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

#ifndef X_H_HEADER_GUARD
#define X_H_HEADER_GUARD

#include <alloca.h> // alloca
#include <stdarg.h> // va_list
#include <stdint.h> // uint32_t
#include <stdlib.h> // size_t
#include <stddef.h> // ptrdiff_t
#include <cstring>
#include <cstdio>

#include "platform.hpp"
#include "macros.hpp"


/// \c X_COUNTOF calculate plain array \c _x[N] size at compile time. It returns a const variable.
#define X_COUNTOF(_x) sizeof( X::COUNTOF_REQUIRES_ARRAY_ARGUMENT(_x) )

#if X_PLATFORM_WIN
#     define X_DEBUG_OUTPUT(_out)   OutputDebugStringA(_out);
#elif X_PLATFORM_OSX
#     define X_DEBUG_OUTPUT(_out)   printf("%s", _out);
#else
#     define X_DEBUG_OUTPUT(_out)   printf("%s", _out);
#endif


namespace X
{
  const int32_t kExitSuccess = 0; // k for 'count.
  const int32_t kExitFailure = 1;
  
  template<typename Ty>
  inline void xchg(Ty& _a, Ty& _b)
  {
    Ty tmp = _a; _a = _b; _b = tmp;
  }
  
  void xchg(void* _a, void* _b, size_t _numBytes)
  {
    uint8_t* lhs = (uint8_t*)_a;
    uint8_t* rhs = (uint8_t*)_b;
    const uint8_t* end = rhs + _numBytes;
    while (rhs != end)
    {
      xchg(*lhs++, *rhs++);
    }
  }
  
  void memCopyRef(void* _dst, const void* _src, size_t _numBytes)
  {
    uint8_t* dst = (uint8_t*)_dst;
    const uint8_t* end = dst + _numBytes;
    const uint8_t* src = (const uint8_t*)_src;
    while (dst != end)
    {
      *dst++ = *src++;
    }
  }
  
  void memCopy(void* _dst, const void* _src, size_t _numBytes)
  {
    std::memcpy(_dst, _src, _numBytes);
  }
  
  void memCopy(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch, uint32_t _dstPitch)
  {
    const uint8_t* src = (const uint8_t*)_src;
    uint8_t* dst = (uint8_t*)_dst;
    
    for (uint32_t ii = 0; ii < _num; ++ii)
    {
      memCopy(dst, src, _size);
      src += _srcPitch;
      dst += _dstPitch;
    }
  }
  
  ///
  void gather(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch)
  {
    memCopy(_dst, _src, _size, _num, _srcPitch, _size);
  }
  
  ///
  void scatter(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _dstPitch)
  {
    memCopy(_dst, _src, _size, _num, _size, _dstPitch);
  }
  
  void memMoveRef(void* _dst, const void* _src, size_t _numBytes)
  {
    uint8_t* dst = (uint8_t*)_dst;
    const uint8_t* src = (const uint8_t*)_src;
    
    if (_numBytes == 0
        ||  dst == src)
    {
      return;
    }
    
    //	if (src+_numBytes <= dst || end <= src)
    if (dst < src)
    {
      memCopy(_dst, _src, _numBytes);
      return;
    }
    
    for (intptr_t ii = _numBytes-1; ii >= 0; --ii)
    {
      dst[ii] = src[ii];
    }
  }
  
  void memMove(void* _dst, const void* _src, size_t _numBytes)
  {
    std::memmove(_dst, _src, _numBytes);
  }
  
  void memSetRef(void* _dst, uint8_t _ch, size_t _numBytes)
  {
    uint8_t* dst = (uint8_t*)_dst;
    const uint8_t* end = dst + _numBytes;
    while (dst != end)
    {
      *dst++ = char(_ch);
    }
  }
  
  void memSet(void* _dst, uint8_t _ch, size_t _numBytes)
  {
    std::memset(_dst, _ch, _numBytes);
  }
  
  int32_t memCmpRef(const void* _lhs, const void* _rhs, size_t _numBytes)
  {
    const char* lhs = (const char*)_lhs;
    const char* rhs = (const char*)_rhs;
    for (
         ; 0 < _numBytes && *lhs == *rhs
         ; ++lhs, ++rhs, --_numBytes
         )
    {
    }
    
    return 0 == _numBytes ? 0 : *lhs - *rhs;
  }
  
  int32_t memCmp(const void* _lhs, const void* _rhs, size_t _numBytes)
  {
    return std::memcmp(_lhs, _rhs, _numBytes);
  }
  
  

  ///
  int32_t vsnprintf(char* _out, int32_t _max, const char* _format, va_list _argList)
  {
#if X_COMPILER_MSVC
    int32_t len = -1;
    if (NULL != _out)
    {
      va_list argListCopy;
      va_copy(argListCopy, _argList);
      len = ::vsnprintf_s(_out, _max, size_t(-1), _format, argListCopy);
      va_end(argListCopy);
    }
    return -1 == len ? ::_vscprintf(_format, _argList) : len;
#else
    return ::vsnprintf(_out, _max, _format, _argList);
#endif
  }
  
  int32_t snprintf(char* _out, int32_t _max, const char* _format, ...)
  {
    va_list argList;
    va_start(argList, _format);
    int32_t len = vsnprintf(_out, _max, _format, argList);
    va_end(argList);
    return len;
  }
  
  /// Only output information
  void debugPrintfVargs(const char* _format, va_list _argList)
  {
    char temp[8192];
    char* out = temp;
    int32_t len = vsnprintf(out, sizeof(temp), _format, _argList);
    if ((int32_t)sizeof(temp) < len)
    {
      out = (char*)alloca(len + 1);
      len = vsnprintf(out, len, _format, _argList);
    }
    out[len] = '\0';
    X_DEBUG_OUTPUT(out)
  }
  
  /// Output information and __FILE__ __LINE__ information that can easily trace on.
  void traceVargs(const char* _filePath, uint16_t _line, const char* _format, va_list _argList)
  {
    char temp[2048];
    char* out = temp;
    va_list argListCopy;
    va_copy(argListCopy, _argList);
    int32_t len = snprintf(out, sizeof(temp), "%s (%d): ", _filePath, _line);
    int32_t total = len + vsnprintf(out + len, sizeof(temp) - len, _format, argListCopy);
    va_end(argListCopy);
    if ((int32_t)sizeof(temp) < total)
    {
      out = (char*)alloca(total + 1);
      memCopy(out, temp, len);
      vsnprintf(out + len, total - len, _format, _argList);
    }
    out[total] = '\0';
    X_DEBUG_OUTPUT(out)
  }
  
  void trace(const char* _filePath, uint16_t _line, const char* _format, ...)
  {
    va_list argList;
    va_start(argList, _format);
    //debugPrintfVargs(_format, argList);
    traceVargs(_filePath, _line, _format, argList);
    va_end(argList);
  }
  
}



#endif //X_H_HEADER_GUARD
