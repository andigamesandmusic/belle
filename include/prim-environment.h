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

#ifndef PRIM_INCLUDE_ENVIRONMENT_H
#define PRIM_INCLUDE_ENVIRONMENT_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

///Exposes all the core classes in prim.
namespace PRIM_NAMESPACE
{
  /**Inspects the current build environment.
  - Platform:
    - PRIM_ENVIRONMENT_WINDOWS
      - PRIM_ENVIRONMENT_WINDOWS_32
      - PRIM_ENVIRONMENT_WINDOWS_64
    - PRIM_ENVIRONMENT_UNIX_LIKE
      - PRIM_ENVIRONMENT_APPLE
      - PRIM_ENVIRONMENT_BSD
      - PRIM_ENVIRONMENT_GNULINUX
      - PRIM_ENVIRONMENT_ANDROID
  - Data Model:
    - PRIM_ENVIRONMENT_ILP32
    - PRIM_ENVIRONMENT_LP64
    - PRIM_ENVIRONMENT_LLP64
  - Endianness:
    - PRIM_ENVIRONMENT_BIG_ENDIAN
    - PRIM_ENVIRONMENT_LITTLE_ENDIAN
  */
  class Environment
  {
    public:

    ///Prints all environment definitions in effect to diagnose porting issues.
    static void Print();

    ///Returns whether the operating system is Windows.
    static inline bool Windows()
    {
#ifdef PRIM_ENVIRONMENT_WINDOWS
      return true;
#else
      return false;
#endif
    }

    ///Returns whether the operating system is 32-bit Windows.
    static inline bool Windows32()
    {
#ifdef PRIM_ENVIRONMENT_WINDOWS_32
      return true;
#else
      return false;
#endif
    }

    ///Returns whether the operating system is 64-bit Windows.
    static inline bool Windows64()
    {
#ifdef PRIM_ENVIRONMENT_WINDOWS_64
      return true;
#else
      return false;
#endif
    }

    ///Returns whether the operating system is Unix-like (*nix).
    static inline bool UnixLike()
    {
#ifdef PRIM_ENVIRONMENT_UNIX_LIKE
      return true;
#else
      return false;
#endif
    }

    ///Returns whether the operating system is one of Apple's.
    static inline bool Apple()
    {
#ifdef PRIM_ENVIRONMENT_APPLE
      return true;
#else
      return false;
#endif
    }

    ///Returns whether the operating system is BSD.
    static inline bool BSDUnix()
    {
#ifdef PRIM_ENVIRONMENT_BSD
      return true;
#else
      return false;
#endif
    }

    ///Returns whether the operating system is GNU/Linux.
    static inline bool GNULinux()
    {
#ifdef PRIM_ENVIRONMENT_GNULINUX
      return true;
#else
      return false;
#endif
    }

    ///Returns whether the operating system is Android.
    static inline bool Android()
    {
#ifdef PRIM_ENVIRONMENT_ANDROID
      return true;
#else
      return false;
#endif
    }

    ///Returns whether the data model is ILP32.
    static inline bool ILP32()
    {
#ifdef PRIM_ENVIRONMENT_ILP32
      return true;
#else
      return false;
#endif
    }

    ///Returns whether the data model is LP64.
    static inline bool LP64()
    {
#ifdef PRIM_ENVIRONMENT_LP64
      return true;
#else
      return false;
#endif
    }

    ///Returns whether the data model is LLP64.
    static inline bool LLP64()
    {
#ifdef PRIM_ENVIRONMENT_LLP64
      return true;
#else
      return false;
#endif
    }

    ///Returns whether the data model is big-endian.
    static inline bool BigEndian()
    {
#ifdef PRIM_ENVIRONMENT_BIG_ENDIAN
      return true;
#else
      return false;
#endif
    }

    ///Returns whether the data model is little-endian.
    static inline bool LittleEndian()
    {
#ifdef PRIM_ENVIRONMENT_LITTLE_ENDIAN
      return true;
#else
      return false;
#endif
    }
  };

  //---------------------//
  //Low-Level Console Out//
  //---------------------//

  /**This function is meant for low-level logging and is platform-specific. It
  is placed here so that it can be available immediately after the environment
  is determined.*/
  void Log(const char* x);
}

#ifdef PRIM_COMPILE_INLINE

namespace PRIM_NAMESPACE
{
  void Environment::Print()
  {
    Log(" Compiler-Detected Environment ");
    Log("-------------------------------");
    #ifdef PRIM_ENVIRONMENT_WINDOWS
    Log("PRIM_ENVIRONMENT_WINDOWS");
    #endif
    #ifdef PRIM_ENVIRONMENT_WINDOWS_32
    Log("PRIM_ENVIRONMENT_WINDOWS_32");
    #endif
    #ifdef PRIM_ENVIRONMENT_WINDOWS_64
    Log("PRIM_ENVIRONMENT_WINDOWS_64");
    #endif
    #ifdef PRIM_ENVIRONMENT_UNIX_LIKE
    Log("PRIM_ENVIRONMENT_UNIX_LIKE");
    #endif
    #ifdef PRIM_ENVIRONMENT_APPLE
    Log("PRIM_ENVIRONMENT_APPLE");
    #endif
    #ifdef PRIM_ENVIRONMENT_BSD
    Log("PRIM_ENVIRONMENT_BSD");
    #endif
    #ifdef PRIM_ENVIRONMENT_GNULINUX
    Log("PRIM_ENVIRONMENT_GNULINUX");
    #endif
    #ifdef PRIM_ENVIRONMENT_ANDROID
    Log("PRIM_ENVIRONMENT_ANDROID");
    #endif
    #ifdef PRIM_ENVIRONMENT_ILP32
    Log("PRIM_ENVIRONMENT_ILP32");
    #endif
    #ifdef PRIM_ENVIRONMENT_LP64
    Log("PRIM_ENVIRONMENT_LP64");
    #endif
    #ifdef PRIM_ENVIRONMENT_LLP64
    Log("PRIM_ENVIRONMENT_LLP64");
    #endif
    #ifdef PRIM_ENVIRONMENT_BIG_ENDIAN
    Log("PRIM_ENVIRONMENT_BIG_ENDIAN");
    #endif
    #ifdef PRIM_ENVIRONMENT_LITTLE_ENDIAN
    Log("PRIM_ENVIRONMENT_LITTLE_ENDIAN");
    #endif
  }
}

#ifdef PRIM_ENVIRONMENT_ANDROID
#include <android/log.h>
#endif

namespace PRIM_NAMESPACE
{
  void Log(const char* x)
  {
  #ifdef PRIM_ENVIRONMENT_ANDROID
    __android_log_print(ANDROID_LOG_INFO, "prim", "%s", x);
  #else
    std::cout << "prim:" << x << std::endl;
  #endif
  }
}

#endif
#endif
