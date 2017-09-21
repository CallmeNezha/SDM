/*
 X ::
 
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

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "xmath.hpp"

TEST_CASE( "Return range of numbers", "[X::range]" )
{
  SECTION( "testing forward range" )
  {
    auto range = X::range<uint32_t>(1, 5);
    REQUIRE( (range == std::vector<uint32_t>{1, 2, 3, 4}) );
  }
  SECTION( "testing backward range" )
  {
    auto range = X::range<int32_t>(1, -3);
    REQUIRE( (range == std::vector<int32_t>{1, 0, -1, -2}) );
  }
}


