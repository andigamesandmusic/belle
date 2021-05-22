/*
  ==============================================================================

  Copyright 2007-2013, 2017 William Andrew Burnson
  Copyright 2013-2016 Robert Taub

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

  ==============================================================================
*/

#ifndef PRIM_INCLUDE_PLATFORM_H
#define PRIM_INCLUDE_PLATFORM_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

//The big picture: Windows or Unix-like?
#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32) || defined(_WIN64)
//Windows XP and later (32-bit or 64-bit)
#define PRIM_ENVIRONMENT_WINDOWS
#include <ciso646>
#else
//GNU/Linux, BSD, Mac OS X, iOS, Android and variants
#define PRIM_ENVIRONMENT_UNIX_LIKE
#endif

//Among Windows: 32-bit or 64-bit?
#if defined(PRIM_ENVIRONMENT_WINDOWS) and defined(_WIN64)
//64-bit Windows
#define PRIM_ENVIRONMENT_WINDOWS_64
#elif defined(PRIM_ENVIRONMENT_WINDOWS)
//32-bit Windows
#define PRIM_ENVIRONMENT_WINDOWS_32
#endif

//Is it an Apple platform?
#if defined(__APPLE__)
//Mac OS X (iMac, MacBook variants, Mac mini) or iOS (iPhone and iPad)
#define PRIM_ENVIRONMENT_APPLE
#endif

//Is it a GNU/Linux variant?
#if defined(__linux__)
#define PRIM_ENVIRONMENT_GNULINUX
#endif

//Is it a BSD variant?
#if defined(BSD)
#define PRIM_ENVIRONMENT_BSD
#endif

//Is it an Android OS?
#if defined(__ANDROID__)
#define PRIM_ENVIRONMENT_ANDROID
#endif

//What is the data model?
#if defined(PRIM_ENVIRONMENT_WINDOWS_64)
//Model LLP64: int = long = 4 bytes, long long = 8 bytes, pointers = 8 bytes
#define PRIM_ENVIRONMENT_LLP64
#elif defined (__LP64__) or defined (_LP64)
//Model LP64: int = 4 bytes, long = long long = 8 bytes, pointers = 8 bytes
#define PRIM_ENVIRONMENT_LP64
#else
//Model ILP32: int = long = 4 bytes, long long = 8 bytes, pointers = 4 bytes
#define PRIM_ENVIRONMENT_ILP32
#endif

//What is the endianness?
#if defined(PRIM_ENVIRONMENT_WINDOWS) or defined(__LITTLE_ENDIAN__)
  //Little-endian flag detected
  #define PRIM_ENVIRONMENT_LITTLE_ENDIAN
#elif defined(__BYTE_ORDER__) and defined(__ORDER_LITTLE_ENDIAN__)
  //Little-endian detected by byte order flag
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  #define PRIM_ENVIRONMENT_LITTLE_ENDIAN
  #endif
#endif
#ifndef PRIM_ENVIRONMENT_LITTLE_ENDIAN
#define PRIM_ENVIRONMENT_BIG_ENDIAN
#endif

#endif
