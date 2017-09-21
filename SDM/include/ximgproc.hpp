/*
 XImgProc :: X image process
 
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

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifndef X_IMGPROC_H_HEADER_GUARD
#define X_IMGPROC_H_HEADER_GUARD

/// Debug macro
# ifndef X_PRINTMAT
# define X_PRINTMAT(_m) \
std::cout << "cv::Mat " << #_m << std::endl; \
std::cout << _m << std::endl << std::endl;
# endif //X_PRINTMAT(_m)

# if X_DEBUG
# define X_MACRO_DEBUG(x) x
# else
# define X_MACRO_DEBUG(x)
# endif //X_MACRO_DEBUG

namespace X
{
  ///
  inline cv::Point toPoint(const cv::Vec2f& _pt)
  {
    return cv::Point((int32_t)_pt[0], (int32_t)_pt[1]);
  }
  
  ///
  inline cv::Mat scaleImg(const cv::Mat& _img, uint16_t _width)
  {
    cv::Mat scaled;
    uint16_t width = fmin(_img.cols, _width);
    uint16_t height = width * (float)_img.rows / (float)_img.cols;
    cv::resize(_img, scaled, cv::Size(width, height));
    return scaled;
  }
  
  ///
  cv::Rect perturb(const cv::Rect& _box, float _transRatioX, float _transRatioY, float _scale = 1.f)
  {
    auto tx_pixel = _transRatioX * _box.width;
    auto ty_pixel = _transRatioY * _box.height;
    
    auto perturbed_width = _box.width * _scale;
    auto perturbed_height = _box.height * _scale;
    
    cv::Rect perturteb_box(
      _box.x + (_box.width - perturbed_width) / 2.f + tx_pixel,
      _box.y + (_box.height - perturbed_height) / 2.f + ty_pixel,
      perturbed_width,
      perturbed_height
    );
    return perturteb_box;
  }
  
}


#endif //X_IMGPROC_H_HEADER_GUARD

