/*
 Xmath is a C++11 math toolset.
 
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


#ifndef X_MATH_H_HEADER_GUARD
#define X_MATH_H_HEADER_GUARD

#include <cmath>
#include <algorithm>
#include <x.hpp>

namespace X
{
  template<typename T>
  std::vector<T> range(T a, T b)
  {
    std::vector<T> v;
    v.reserve(abs(b - a));
    for (T i = a; i != b; a > b ? i-- : i++)
    {
      v.emplace_back(i);
    }
    return v;
  }
  
  
}


#endif //X_MATH_H_HEADER_GUARD
