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

#ifndef BELLE_MODERN_METER_H
#define BELLE_MODERN_METER_H

#include "Directory.h"

namespace BELLE_NAMESPACE { namespace modern
{
  ///Algorithms for typesetting a meter.
  struct Meter
  {
    ///Engrave the different forms of meters.
    static void Engrave(Directory& d, Stamp& s, graph::MusicNode mt)
    {
      if(!mt) return;
      
      mica::Concept v = mt->Get(mica::Value);
      if(v != mica::CommonTime && v != mica::CutTime)
      {
        prim::count Num = integer(mt->Get(mica::Beats));
        prim::count Den = denominator(mt->Get(mica::NoteValue));
        s.Add().p2 = d.Symbol(0x0030 + Num);
        s.z().a = Affine::Translate(
            prim::planar::Vector(0.0, 0.0)) * Affine::Scale(4.0);
        s.z().n = mt;
        s.Add().p2 = d.Symbol(0x0030 + Den);
        s.z().a = Affine::Translate(
            prim::planar::Vector(0.0, -2.0)) * Affine::Scale(4.0);
        s.z().n = mt;
      }
      else
      {
        prim::count C = (v == mica::CommonTime ? 76 : 77);
        s.Add().p2 = d.Symbol(C);
        s.z().a = Affine::Scale(4.0);
        s.z().n = mt;
      }
    }
  };
}}
#endif
