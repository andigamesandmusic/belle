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

#ifndef BELLE_LIBRARY
#define BELLE_LIBRARY

//The belle namespace can be overridden as necessary.
#ifndef BELLE_NAMESPACE
  #define BELLE_NAMESPACE belle
#endif

/*Default locations to prim and MICA (change these if necessary or push them
as preprocessor defines to the compiler).*/

#ifndef BELLE_PRIM_LOCATION
  #define BELLE_PRIM_LOCATION "prim.h"
#endif

#ifndef BELLE_MICA_H_LOCATION
  #define BELLE_MICA_H_LOCATION "mica.h"
#endif

/*
  The BELLE_JUCE_LOCATION preprocessor define specifies the location of the
  JuceLibraryCode/JuceHeader.h relative to this file.
  For information about using and licensing JUCE see: https://www.juce.com
*/
#ifdef BELLE_JUCE_LOCATION
  #define DONT_SET_USING_JUCE_NAMESPACE 1
  #define BELLE_MACRO_STRINGIFY(s) BELLE_MACRO_EXPAND(s)
  #define BELLE_MACRO_EXPAND(s) #s
  #include BELLE_MACRO_STRINGIFY(BELLE_JUCE_LOCATION)
  #undef BELLE_MACRO_STRINGIFY
  #undef BELLE_MACRO_EXPAND
#endif

//The prim library
#ifndef PRIM_LIBRARY
  #define PRIM_WITH_SHELL
  #define PRIM_WITH_MIDI
  #ifdef BELLE_COMPILE_INLINE
    #define PRIM_COMPILE_INLINE
  #endif
  #include BELLE_PRIM_LOCATION
#endif

//The MICA library
#ifndef MICA_H
  #ifdef BELLE_COMPILE_INLINE
    #define MICA_COMPILE_INLINE
  #endif
  #include BELLE_MICA_H_LOCATION
#endif

namespace BELLE_NAMESPACE
{
  /*We are the Borg. Lower your scope and surrender your symbols. We will add
  your biological and technological distinctiveness to our own. Your namespace
  will adapt to service ours. Resistance is futile.*/
  using namespace PRIM_NAMESPACE;
}

//Shows a rectangle around the bounds of each stamp.
//#define BELLE_DEBUG_SHOW_STAMP_BOUNDS

//Show the vertical borders used for collision detection around islands.
//#define BELLE_DEBUG_SHOW_VERTICAL_ISLAND_BORDERS

//Look for problematic numbers during painting.
#define BELLE_DEBUG_OBSERVE_PROBLEMATIC_NUMBERS_DURING_PAINTING

//Stores MICA concept UUIDs as strings instead of high 64 bits.
#define BELLE_DEBUG_STORE_MICA_UUIDV4_AS_STRING

#include "belle-core.h"
#include "belle-painters.h"
#include "belle-freetype.h"
#include "belle-engraver.h"
#include "belle-vtables.h"

#endif
