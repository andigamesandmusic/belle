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

#ifndef PRIM_INCLUDE_CONFIGURATION_H
#define PRIM_INCLUDE_CONFIGURATION_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

/*For all of the below macro settings, either change them here or pass them to
the compiler as preprocessor macros. Be very careful to ensure that the same
configuration of prim is being compiled for a given app wherever prim is used.*/

/*                        Definition of prim namespace

Override the prim namespace.
*/
#ifndef PRIM_NAMESPACE
#define PRIM_NAMESPACE prim
#endif

/*                           Definition of integer

Override the main integer type used by the library here. This can also be
passed in as a compiler define, but make sure it gets passed to all files that
use prim.h in the build.
*/
#ifndef PRIM_INTEGER
#define PRIM_INTEGER int64  //CUSTOMIZE: Use int32 or int64
#endif

/*                           Definition of number

Override the main floating-point typeused by the library here. This can also be
passed in as a compiler define, but make sure it gets passed to all files that
use prim.h in the build.
*/
#ifndef PRIM_NUMBER
#define PRIM_NUMBER float64 //CUSTOMIZE: Use float32, float64, or float80
#endif

/*                            Definition of count

If necessary override the type of the count here, though generally it is best
to leave it to the default, which matches to the size of a pointer. This might
be overriden as an int for type-compatibility with code that uses int for the
main counting type. This can also be passed in as a compiler define, but make
sure it gets passed to all files that use prim.h in the build.
*/
//#define PRIM_COUNT int    //CUSTOMIZE: Use int, int32, or int64

/*                        Member Padding and Alignment

Disables explicit padding. Turning this on may save space but could produce
warnings about padding.
*/
//#define PRIM_NO_PADDING

/*                            Internals Debugging

Enables internal debug and diagnositic messages that are normally left
completely out of the build. Currently used by String.
*/
//#define PRIM_DEBUG_INTERNAL

/*                            std::thread (C++11)

Use std::thread instead of POSIX threads when the thread module is activated.
*/
//#define PRIM_THREAD_USE_STD_THREAD

/*                                C++11 Support

Enable C++11 features in prim.
*/
//#define PRIM_11

/*                          Cryptographic Randomness

Use /dev/random for Random::SystemNoise() and for seeding Random.
*/
//#define PRIM_USE_DEV_RANDOM

/*                                  Modules

Enable platform-specific modules with PRIM_WITH_<MODULENAME>. See below for a
complete list of available modules. Some of these may need configuration or may
have incomplete implementations on some platforms. Refer to the module source
file for more information.
*/
//Set all the modules at once:
//#define PRIM_WITH_MODULES

//Or set them individually:
//#define PRIM_WITH_AES
//#define PRIM_WITH_DIRECTORY
//#define PRIM_WITH_FFT
//#define PRIM_WITH_JAVASCRIPT_JSC
//#define PRIM_WITH_JAVASCRIPT_V8
//#define PRIM_WITH_MEMORY_MAP
//#define PRIM_WITH_MIDI
//#define PRIM_WITH_PYTHON
//#define PRIM_WITH_RSA
//#define PRIM_WITH_SHELL
//#define PRIM_WITH_THREAD
//#define PRIM_WITH_TIMER

//............................................................................//

//Intermodule dependencies
#if defined(PRIM_WITH_DIRECTORY) and not defined(PRIM_WITH_SHELL)
#define PRIM_WITH_SHELL
#endif

#if defined(PRIM_WITH_PYTHON) and not defined(PRIM_WITH_SHELL)
#define PRIM_WITH_SHELL
#endif

#if defined(PRIM_WITH_SHELL) and not defined(PRIM_WITH_TIMER)
#define PRIM_WITH_TIMER
#endif

#if defined(PRIM_WITH_JAVASCRIPT_JSC) and not defined(PRIM_WITH_QUEUE)
#define PRIM_WITH_QUEUE
#endif

#if defined(PRIM_WITH_JAVASCRIPT_V8) and not defined(PRIM_WITH_QUEUE)
#define PRIM_WITH_QUEUE
#endif

#if defined(PRIM_WITH_QUEUE) and not defined(PRIM_WITH_THREAD)
#define PRIM_WITH_THREAD
#endif

#if defined(PRIM_WITH_QUEUE) and not defined(PRIM_WITH_TIMER)
#define PRIM_WITH_TIMER
#endif

//If all modules are requested then set the remaining module defines.
#ifdef PRIM_WITH_MODULES
#define PRIM_WITH_AES
#define PRIM_WITH_DIRECTORY
#define PRIM_WITH_FFT
#define PRIM_WITH_MEMORY_MAP
#define PRIM_WITH_MIDI
#define PRIM_WITH_PYTHON
#define PRIM_WITH_QUEUE
#define PRIM_WITH_RSA
#define PRIM_WITH_SHELL
#define PRIM_WITH_THREAD
#define PRIM_WITH_TIMER
#undef PRIM_WITH_MODULES
#endif

#endif
