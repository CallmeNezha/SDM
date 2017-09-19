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

#ifndef SDM_IODATA_H_HEADER_GUARD
#define SDM_IODATA_H_HEADER_GUARD


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>


#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include <cassert>

#include "rcr/landmark.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace pt = boost::property_tree;


namespace SDM
{
  
  using LandmarksInfo = std::pair<std::string, rcr::LandmarkCollection<cv::Vec2f>>;
  
  LandmarksInfo readHelenLandmarks(const std::string& _filename)
  {
    rcr::LandmarkCollection<cv::Vec2f> landmarks;
    landmarks.reserve(194);
    std::ifstream file(_filename);
    if (false == file.is_open())
    {
      throw std::runtime_error(std::string("Could not open landmark file: " + _filename));
    }
    
    std::string name;
    std::getline(file, name);
    name = name.substr(0, name.size() - 1);
    
    rcr::Landmark<cv::Vec2f> landmark;
    
    std::string line;
    std::string dot;
    uint32_t lmk_id = 1;
    while(std::getline(file, line))
    {
      std::stringstream line_stream(line);
      landmark.name = std::to_string(lmk_id);
      if(!(line_stream >> landmark.coordinates[0] >> dot >> landmark.coordinates[1]))
        throw std::runtime_error(std::string("Landmark format error while parsing the line " + line));
      // From the iBug website:
      // "Please note that the re-annotated data for this challenge are saved in the Matlab convention of 1 being
      // the first index, i.e. the coordinates of the top left pixel in an image are x=1, y=1."
      // ==> So we shift every point by 1:
      landmark.coordinates[0] -= 1.0f;
      landmark.coordinates[1] -= 1.0f;
      landmarks.emplace_back(landmark);
      ++lmk_id;
    }
    return std::make_pair(name, landmarks);
  }
  
  
  /*!
   Interface class of data io.
   */
  class IData
  {
  public:
    virtual ~IData() {};
    /*!
     Get all images.
     
     @return  Images.
     */
    virtual const std::vector<cv::Mat>& getData() = 0;
    
    /*!
     Get landmarks of corresponding images.
     
     @return  Landmarks.
     */
    virtual const std::vector<rcr::LandmarkCollection<cv::Vec2f>>& getLandmarks() = 0;
    
    /*!
     Get filenames of corresponding images.
     
     @return  Filenames.
     */
    virtual const std::vector<fs::path>& getFilenames() = 0;
     
  };
  
  /*!
   IO implement for read Helen 194 dataset.
   */
  class HelenIO : public IData
  {
  public:
    HelenIO(fs::path _imgDir, fs::path _lmkDir)
    {
      
      std::map<std::string, rcr::LandmarkCollection<cv::Vec2f>>  landmarks;
      
      // Get all the filenames in the given directory:
      fs::directory_iterator end_itr;
      for (fs::directory_iterator i(_imgDir); i != end_itr; ++i)
      {
        if (fs::is_regular_file(i->status()) && i->path().extension() == ".jpg")
          m_filenames.emplace_back(i->path());
      }
      
      // Get all the annotations corresponding to given images.
      for (fs::directory_iterator i(_lmkDir); i != end_itr; ++i)
      {
        if (fs::is_regular_file(i->status()) && i->path().extension() == ".txt")
        {
          LandmarksInfo lmk = readHelenLandmarks(i->path().string());
          landmarks.insert(lmk);
        }
      }
      
      // Check correspondency.
      if (m_filenames.size() != landmarks.size())
      {
        throw std::runtime_error("Image file is not corresponding to landmarks file.");
      }
      
      // Convert map to vector.
      for (uint32_t i = 0; i < m_filenames.size(); ++i)
      {
        m_landmarks.emplace_back(landmarks[ m_filenames[i].string() ]);
      }
    }
    
    
    virtual const std::vector<cv::Mat>& getData()
    {
      if (0 == m_images.size())
      {
        for (auto& file : m_filenames)
          m_images.emplace_back(cv::imread(file.string()));
      }
      return m_images;
    }
    
    virtual const std::vector<rcr::LandmarkCollection<cv::Vec2f>>& getLandmarks()
    {
      return m_landmarks;
    }
    
    virtual const std::vector<fs::path>& getFilenames()
    {
      return m_filenames;
    }
    
  private:
    std::vector<cv::Mat>    m_images;
    std::vector<fs::path>   m_filenames;
    std::vector<rcr::LandmarkCollection<cv::Vec2f>> m_landmarks;
    
  };

}

#endif  //SDM_IODATA_H_HEADER_GUARD
