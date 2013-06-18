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

#ifndef BELLE_MODERN_KEYSIGNATURE_H
#define BELLE_MODERN_KEYSIGNATURE_H

#include "Directory.h"

#include "State.h"

namespace BELLE_NAMESPACE { namespace modern
{
  ///Algorithms for typesetting a key signature.
  struct KeySignature
  {
    ///Engraves the different forms of key signatures.
    static void Engrave(Directory& d, Stamp& s, graph::MusicNode kt)
    {
      if(!kt) return;
      mica::Concept k = kt->Get(mica::Value);
      prim::count n = Utility::GetNumberOfAccidentals(k);
      const Path* Accidental =
        d.Symbol((mica::map(k, mica::Accidental) == mica::Flat) ? 70 : 72);
      for(prim::count i = 0; i < n; i++)
      {
        s.Add().p2 = Accidental;
        s.z().a = Affine::Translate(prim::planar::Vector((prim::number)i * 1.0,
          (prim::number)(Utility::GetAccidentalPosition(k, d.s.ActiveClef, i)) /
          2.0)) * Affine::Scale(4.0);
        s.z().n = kt;
      }
    }
  };
}}
#endif
