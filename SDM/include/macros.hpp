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


#ifndef X_MACROS_H_HEADER_GUARD
#define X_MACROS_H_HEADER_GUARD


/// Macro routine helper
#define X_MACRO_BLOCK_BEGIN for(;;) {
#define X_MACRO_BLOCK_END   break;  }
#define X_NOOP(...)     X_MACRO_BLOCK_BEGIN X_MACRO_BLOCK_END

/// Condition semantic helper
#define X_UNLIKELY(_x)    (_x)
#define X_LIKELY(_x)      (_x)

#if X_DEBUG
#     define X_CHECK _X_CHECK
#     define X_TRACE _X_TRACE
#else
#     define X_CHECK X_NOOP
#     define X_TRACE X_NOOP
#endif

#define _X_CHECK(_condition, _format, ...) \
                  X_MACRO_BLOCK_BEGIN \
                        if (!_condition) \
                        { \
                              X_TRACE("* CHECK " __format, ##__VA_ARGS__); \
                        } \
                  X_MACRO_BLOCK_END

#define _X_TRACE(_format, ...) \
                  X_MACRO_BLOCK_BEGIN \
                        X::trace(__FILE__, uint16_t(__LINE__), "* X " _format "\n", ##__VA_ARGS__); \
                  X_MACRO_BLOCK_END


#endif //X_MACROS_H_HEADER_GUARD
