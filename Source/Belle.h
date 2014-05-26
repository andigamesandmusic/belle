/*
  ==============================================================================

  Copyright 2007-2013 William Andrew Burnson. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY WILLIAM ANDREW BURNSON ''AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
  EVENT SHALL WILLIAM ANDREW BURNSON OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  ------------------------------------------------------------------------------

  This file is part of Belle, Bonne, Sage --
    The 'Beautiful, Good, Wise' C++ Vector-Graphics Library for Music Notation 

  ==============================================================================
*/

#ifndef BELLE_LIBRARY
#define BELLE_LIBRARY

//-----------------//
//Default Namespace//
//-----------------//

//The belle namespace can be overridden as necessary.
#ifndef BELLE_NAMESPACE
#define BELLE_NAMESPACE belle
#endif

//----------------------//
//Prerequisite Locations//
//----------------------//

/*Default locations to prim.cc and MICA (change these if necessary or push them
as preprocessor defines to the compiler).*/
#ifndef BELLE_PRIM_LOCATION
#define BELLE_PRIM_LOCATION "Modules/prim.cc/Source/prim.h"
#endif

#ifndef BELLE_MICA_H_LOCATION
#define BELLE_MICA_H_LOCATION "Modules/MICA/Ports/en-cpp/mica.h"
#endif

#ifndef BELLE_MICA_CPP_LOCATION
#define BELLE_MICA_CPP_LOCATION "Modules/MICA/Ports/en-cpp/mica.cpp"
#endif

//-------------//
//Prerequisites//
//-------------//

//The prim.cc library -- https://github.com/burnson/prim.cc
#ifndef PRIM_CC_LIBRARY
#define PRIM_WITH_SHELL
#ifdef BELLE_COMPILE_INLINE
#  define PRIM_COMPILE_INLINE
#endif
#include BELLE_PRIM_LOCATION
#endif

//The MICA library --  https://github.com/burnson/MICA
#ifndef MICA_H
#ifdef BELLE_COMPILE_INLINE
#  include BELLE_MICA_CPP_LOCATION
#endif
#include BELLE_MICA_H_LOCATION
#endif

//----//
//Core//
//----//

#include "Core/Core.h"

//-----//
//Graph//
//-----//

#include "Graph/Graph.h"

//------//
//Modern//
//------//

#include "Modern/Modern.h"

//--------//
//Painters//
//--------//

#include "Painters/Painters.h"

//--------//
//FreeType//
//--------//

/*FreeType2 requires a little bit of configuration so it is optional.

To use:
1) Add include path to freetype2 (i.e. -I/usr/local/include/freetype2)
2) Link to freetype (i.e. -lfreetype)
3) Define BELLE_WITH_FREETYPE

For example:
-I/usr/local/include/freetype2 -lfreetype -DBELLE_WITH_FREETYPE

Currently Scripts/GetFreeTypeArguments determines this for POSIX systems.*/ 
#include "Modules/FreeType/FreeType.h"

#endif
