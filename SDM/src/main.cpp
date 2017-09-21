/*
 SDM ::
 
 Copyright 2017 ZiJian Jiang
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include <opencv2/opencv.hpp>
#include <cmath>

#include "config.hpp"
#include "x.hpp"
#include "ximgproc.hpp"
#include "iodata.hpp"

#include "superviseddescent/superviseddescent.hpp"
#include "superviseddescent/regressors.hpp"
#include "superviseddescent/verbose_solver.hpp"

const std::string faceDetector = "/Volumes/Workbench/thirdparty/opencv-3.2.0/data/haarcascades/haarcascade_frontalface_alt2.xml";
const cv::Vec3f RED{0,0,255};
const cv::Vec3f GREEN{0,255,0};
const cv::Vec3f CYAN{255,128,0};

double norm(const rcr::Landmark<cv::Vec2f>& prediction, const rcr::Landmark<cv::Vec2f>& groundtruth)
{
  return cv::norm(prediction.coordinates, groundtruth.coordinates, cv::NORM_L2);
};

cv::Mat elementwise_norm(const rcr::LandmarkCollection<cv::Vec2f>& prediction, const rcr::LandmarkCollection<cv::Vec2f>& groundtruth)
{
  assert(prediction.size() == groundtruth.size());
  cv::Mat result(1, prediction.size(), CV_32FC1); // a row with each entry a norm
  for (std::size_t i = 0; i < prediction.size(); ++i) {
    result.at<float>(i) = norm(prediction[i], groundtruth[i]);
  }
  return result;
};

///
cv::Mat calculate_normalised_landmark_errors(cv::Mat predictions, cv::Mat groundtruth, std::vector<std::string> model_landmarks, std::vector<std::string> right_eye_identifiers, std::vector<std::string> left_eye_identifiers)
{
  assert(predictions.rows == groundtruth.rows && predictions.cols == groundtruth.cols);
  cv::Mat normalised_errors;
  for (int r = 0; r < predictions.rows; ++r) {
    auto pred = rcr::to_landmark_collection(predictions.row(r), model_landmarks);
    auto gt = rcr::to_landmark_collection(groundtruth.row(r), model_landmarks);
    // calculates the element-wise norm, normalised with the IED:
    cv::Mat landmark_norms = elementwise_norm(pred, gt).mul(1.0f / rcr::get_ied(pred, right_eye_identifiers, left_eye_identifiers));
    normalised_errors.push_back(landmark_norms);
  }
  return normalised_errors;
};

///
void drawLandmarks(cv::Mat& _img, const rcr::LandmarkCollection<cv::Vec2f>& _lmks, const cv::Vec3f& _color, bool _drawId = false, float _r = 2.f)
{
  for (uint32_t ii = 0; ii < _lmks.size(); ++ii)
  {
    auto pt = _lmks[ii].coordinates;
    cv::circle(_img, X::toPoint(pt), _r, _color, _r / 2.f);
    if(_drawId) cv::putText(_img, std::to_string(ii), X::toPoint(pt), cv::FONT_ITALIC, _r / 4.f, cv::Scalar(180,180,180), 1, CV_AA);
  }
}

///
void drawLandmarks(cv::Mat& _img, const cv::Mat& _lmks, const cv::Vec3f& _color, bool _drawId = false, float _r = 4.f)
{
  for (uint32_t ii = 0; ii < _lmks.cols / 2; ++ii)
  {
    auto pt = cv::Vec2f(_lmks.at<float>(0, ii), _lmks.at<float>(0, ii + _lmks.cols / 2));
    cv::circle(_img, X::toPoint(pt), _r, _color, _r / 2.f);
    if(_drawId) cv::putText(_img, std::to_string(ii), X::toPoint(pt), cv::FONT_ITALIC, _r / 4.f, cv::Scalar(180,180,180), 1, CV_AA);
  }
}


///
boost::optional<cv::Rect> validFace(const std::vector<cv::Rect>& _detectedFaces, const rcr::LandmarkCollection<cv::Vec2f>& _lmks)
{
  rcr::LandmarkCollection<cv::Vec2f> key_lmks(_lmks.size());
  std::vector<cv::Rect> valid_face(_detectedFaces.size());
  std::vector<cv::Rect>::iterator vf_iter;
  rcr::LandmarkCollection<cv::Vec2f>::iterator kl_iter;
  
  if (_detectedFaces.empty())
    goto none;
  
  kl_iter = std::copy_if(_lmks.begin(), _lmks.end(), key_lmks.begin(),
               [](const rcr::Landmark<cv::Vec2f>& _v)
                  { return _v.name == "144" || _v.name == "124" || _v.name == "95"; } );
  key_lmks.resize(std::distance(key_lmks.begin(), kl_iter) ); //reduce size. This is much more efficient then push_back.
  
  vf_iter = std::copy_if(_detectedFaces.begin(), _detectedFaces.end(), valid_face.begin(),
               [&key_lmks](const cv::Rect& _box)
                  { return std::all_of(key_lmks.begin(), key_lmks.end(), [&_box](const rcr::Landmark<cv::Vec2f>& _v){ return _box.contains(X::toPoint(_v.coordinates)) ; } ) ; } );
  valid_face.resize(std::distance(valid_face.begin(), vf_iter) );
  
  if (1 != valid_face.size())
    goto none;
  
  return *valid_face.begin();
  
none:
  return boost::optional<cv::Rect>();
}

///
int32_t train(SDM::HelenIO& _helen)
{
  
  using namespace superviseddescent;
  
  std::vector<uint32_t> train_ids;
  std::vector<cv::Rect> train_faces;
  std::vector<cv::Mat>  train_x_gt_normlized;
  
  cv::Mat  x0; // initialize of mean face landmarks
  cv::Mat  x_gt; // ground truth for training
  std::vector<cv::Mat>  training_imgs;
  
  float perturb_t_mu = 0.f;
  float perturb_t_sigma = 0.04f;
  float perturb_s_mu = 1.f;
  float perturb_s_sigma = 0.04f;
  
  uint16_t num_perturbations = 0; // = 10 perturbations + 1 original = 11 total
  
  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<> dist_t(perturb_t_mu, perturb_t_sigma);
  std::normal_distribution<> dist_s(perturb_s_mu, perturb_s_sigma);
  
  cv::CascadeClassifier face_cascade;
  if (!face_cascade.load(faceDetector))
  {
    throw std::runtime_error("OpenCV cascade face detector not loaded!");
    return X::kExitFailure;
  }
  
  // Run the face detector and obtain the initial estimate x_0 using the mean landmarks.
  for (uint32_t ii = 0; ii < _helen.getData().size(); ++ii)
  {
    auto& img = _helen.getData()[ii];
    auto& lmk = _helen.getLandmarks()[ii];
    
    std::vector<cv::Rect> detected_faces;
    face_cascade.detectMultiScale(img, detected_faces, 1.2, 2, 0, cv::Size(50, 50));
    auto face = validFace(detected_faces, lmk);
    if (!face)
    {
      X_NOOP(
            auto d_img = img.clone();
            drawLandmarks(d_img, lmk, {0, 0, 255} );
            cv::imshow("d_img", d_img);
            cv::waitKey(0);
            )
      continue;
    }
    
    X_NOOP(
          auto d_img = img.clone();
          cv::rectangle(d_img, face.get(), {0, 255, 0} );
          drawLandmarks(d_img, lmk, {0, 255, 0} );
          cv::imshow("d_img", d_img);
          cv::waitKey(0);
          )
    
    // Here we save training info
    train_ids.emplace_back(ii);
    train_faces.emplace_back(face.get() );
  }
  
  // Get all faces normlized landmarks.
  for (uint32_t ii = 0; ii < train_ids.size(); ++ii)
  {
    // calculate mean
    auto box = train_faces[ii];
    auto pts  = _helen.getLandmarks()[ train_ids[ii] ];
    
    float x = box.x + box.width / 2.f;
    float y = box.y + box.height / 2.f;
    
    cv::Mat normlized(1, 194*2, CV_32F);
    for (uint32_t jj = 0; jj < 194; ++jj)
    {
      auto pt = pts[jj].coordinates;
      float dx = (pt[0] - x) / box.width;
      float dy = (pt[1] - y) / box.height;
      normlized.at<float>(0, jj)     = dx;
      normlized.at<float>(0, jj+194) = dy;
    }
    train_x_gt_normlized.emplace_back(normlized);
  }
  
  // Double check.
  if (train_ids.size() != train_faces.size() || train_faces.size() != train_x_gt_normlized.size() )
  {
    throw std::runtime_error("Dimensions not matched. Something is wrong here.");
    return X::kExitFailure;
  }
  
  cv::Mat mean = std::accumulate(train_x_gt_normlized.begin(), train_x_gt_normlized.end(), cv::Mat::zeros(1, 194*2, CV_32F) )
                  / (float)train_x_gt_normlized.size();
  
  for (uint32_t ii = 0; ii < train_ids.size(); ++ii)
  {
    auto& img = _helen.getData()[train_ids[ii] ];
    auto& normlized_lmk = train_x_gt_normlized[ii];
    auto& face = train_faces[ii];
    
    auto lmk_gt = rcr::align_mean(normlized_lmk, face);
    auto lmk_mean = rcr::align_mean(mean, face);
    
    x0.push_back(lmk_mean);
    x_gt.push_back(lmk_gt);
    training_imgs.emplace_back(img);
    
    /// debug only
    X_MACRO_DEBUG(
                  auto d_img = img.clone();
                  cv::rectangle(d_img, face, GREEN);
                  )

    for (uint32_t pp = 0 ; pp < num_perturbations; ++pp)
    {
      cv::Rect tmp_pert = X::perturb(face, dist_t(gen), dist_t(gen), dist_s(gen) );
      
      /// debug only
      X_MACRO_DEBUG(cv::rectangle(d_img, tmp_pert, {255,0,0} ); )
      X_MACRO_DEBUG(drawLandmarks(d_img, rcr::align_mean(mean, tmp_pert), GREEN, false, 2.f); )
      
      x0.push_back(rcr::align_mean(mean, tmp_pert) );
      x_gt.push_back(lmk_gt);
      
      training_imgs.emplace_back(img);
    }
    X_NOOP(
                  drawLandmarks(d_img, lmk_gt, CYAN);
                  drawLandmarks(d_img, lmk_mean, RED);
                  cv::imshow("d_img", X::scaleImg(d_img, 800) );
                  cv::waitKey(0);
                  )
  }
  
  X_TRACE("Kept %d images out of %d", training_imgs.size() / (num_perturbations+1), _helen.getData().size())
  
  
  // Create 3 regularised linear regressors in series:
  std::vector<LinearRegressor<VerbosePartialPivLUSolver>> regressors;
  regressors.emplace_back(LinearRegressor<VerbosePartialPivLUSolver>(Regulariser(Regulariser::RegularisationType::MatrixNorm, 1.5f, false)));
  regressors.emplace_back(LinearRegressor<VerbosePartialPivLUSolver>(Regulariser(Regulariser::RegularisationType::MatrixNorm, 1.5f, false)));
  regressors.emplace_back(LinearRegressor<VerbosePartialPivLUSolver>(Regulariser(Regulariser::RegularisationType::MatrixNorm, 1.5f, false)));
  regressors.emplace_back(LinearRegressor<VerbosePartialPivLUSolver>(Regulariser(Regulariser::RegularisationType::MatrixNorm, 1.5f, false)));
  
  std::vector<std::string> model_landmarks;
  model_landmarks.resize(194);
  {
    int n = 1;
    std::generate(model_landmarks.begin(), model_landmarks.end(), [&n]{ return std::to_string(n++); });
  }
  std::vector<std::string> right_eye_ids { std::to_string(_helen.getRightEye().inCorner), std::to_string(_helen.getRightEye().outCorner) };
  std::vector<std::string> left_eye_ids { std::to_string(_helen.getLeftEye().inCorner), std::to_string(_helen.getLeftEye().outCorner) };
  
  SupervisedDescentOptimiser<LinearRegressor<VerbosePartialPivLUSolver>, rcr::InterEyeDistanceNormalisation> supervised_descent_model(regressors, rcr::InterEyeDistanceNormalisation(model_landmarks, right_eye_ids, left_eye_ids));

  std::vector<rcr::HoGParam> hog_params{ { VlHogVariant::VlHogVariantUoctti, 5, 11, 4, 1.0f },{ VlHogVariant::VlHogVariantUoctti, 5, 10, 4, 0.7f },{ VlHogVariant::VlHogVariantUoctti, 5, 8, 4, 0.4f },{ VlHogVariant::VlHogVariantUoctti, 5, 6, 4, 0.25f } }; // 3 /*numCells*/, 12 /*cellSize*/, 4 /*numBins*/
  assert(hog_params.size() == regressors.size());
  rcr::HogTransform hog(training_imgs, hog_params, model_landmarks, right_eye_ids, left_eye_ids);
  
  // Train the model. We'll also specify an optional callback function:
  std::cout << "Training the model, printing the residual after each learned regressor: " << std::endl;
  // Note: Rename to landmark_error_callback and put in the library?
  auto print_residual = [&x_gt, &model_landmarks, &right_eye_ids, &left_eye_ids](const cv::Mat& current_predictions) {
    std::cout << "NLSR train: " << cv::norm(current_predictions, x_gt, cv::NORM_L2) / cv::norm(x_gt, cv::NORM_L2) << std::endl;
    
    cv::Mat normalised_error = calculate_normalised_landmark_errors(current_predictions, x_gt, model_landmarks, right_eye_ids, left_eye_ids);
    std::cout << "Normalised LM-error train: " << cv::mean(normalised_error)[0] << std::endl;
  };
  
  supervised_descent_model.train(x_gt, x0, cv::Mat(), hog, print_residual);
  
  // Save the learned model:
  
  fs::path outputfile("/Volumes/Workbench/mylab/Training-Data/Helen_Small/helen_SDM_model.bin");
  rcr::detection_model learned_model(supervised_descent_model, mean, model_landmarks, hog_params, right_eye_ids, left_eye_ids);
  try {
    rcr::save_detection_model(learned_model, outputfile.string());
  }
  catch (const cereal::Exception& e) {
    std::cout << e.what() << std::endl;
  }
  X_TRACE("Training finished...")
  return X::kExitSuccess;
}

int main(int argc, char** argv)
{
  auto helen = SDM::HelenIO("/Volumes/Workbench/mylab/Training-Data/Helen_Small/helen", "/Volumes/Workbench/mylab/Training-Data/Helen_Small/annotation");
  
//  for (uint32_t kk = 0; kk < helen.getData().size(); ++kk)
//  {
//    auto img = helen.getData()[kk].clone();
//    auto lmk = helen.getLandmarks()[kk];
//    
//    for (uint32_t ii = 0; ii < lmk.size(); ++ii)
//    {
//      auto pt = lmk[ii].coordinates;
//      cv::circle(img, toPoint(pt), 2.f, {255.f, 0.f, 0.f}, 1.f);
//      cv::putText(img, std::to_string(ii), toPoint(pt), cv::FONT_ITALIC,.5, cv::Scalar(180,180,180),1, CV_AA);
//    }
//    cv::imshow("img", X::scaleImg(img, 1280));
//    cv::waitKey(0);
//  }
  
  train(helen);
  
  return X::kExitSuccess;
}
