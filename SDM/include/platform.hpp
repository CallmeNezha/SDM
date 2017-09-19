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

#ifndef X_PLATFORM_H_HEADER_GUARD
#define X_PLATFORM_H_HEADER_GUARD

#define X_PLATFORM_OSX     1
#define X_PLATFORM_WIN     0

#define X_COMPILER_CLANG   1
#define X_COMPILER_MSVC    0


#if X_PLATFORM_WIN
#     define X_DEBUG_OUTPUT(_out)   OutputDebugStringA(_out);
#elif X_PLATFORM_OSX
#else
#     define X_DEBUG_OUTPUT(_out)   printf(_out);
#endif


#endif //X_PLATFORM_H_HEADER_GUARD
