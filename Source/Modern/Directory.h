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

#ifndef BELLE_MODERN_DIRECTORY_H
#define BELLE_MODERN_DIRECTORY_H

#include "Cache.h"
#include "House.h"
#include "Stamp.h"
#include "Utility.h"

namespace BELLE_NAMESPACE { namespace modern
{
  //Forward declaration
  struct State;

  /**Index with references to the other class objects. Instead of creating one
  large typesetting class, the index contains references to all of its
  components.*/
  struct Directory
  {
    //Objects and references in the directory.
    State& s;
    graph::Music& m;
    const House& h;
    const Cache& c;
    const Typeface& t;
    const Font& f;
    
    ///Constructor initializes the references to each object.
    Directory(State& s, graph::Music& m, const House& h,
      const Cache& c, const Typeface& t, const Font& f) : s(s), m(m), h(h),
      c(c), t(t), f(f) {}

    ///Retrieves a cached path.
    inline const Path* Cached(prim::count i) {return c[i];}
    
    ///Retrieves a symbol from a font.
    inline const Path* Symbol(prim::count i) {return t.LookupGlyph(i);}
  };
  
  ///Class to inherit a directory reference.
  struct DirectoryHandler
  {
    ///Reference to directory.
    Directory& d;
    
    ///Constructor to set directory.
    DirectoryHandler(Directory& d) : d(d) {}
  };
}}
#endif
