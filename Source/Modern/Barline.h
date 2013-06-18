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

#ifndef BELLE_MODERN_BARLINE_H
#define BELLE_MODERN_BARLINE_H

#include "Directory.h"

namespace BELLE_NAMESPACE { namespace modern
{
  ///Algorithms for typesetting a barline.
  struct Barline
  {
    ///Engrave the different forms of barline.
    static void Engrave(Directory& d, Stamp& s, graph::MusicNode bt)
    {
      if(!bt) return;
      
      //Distance to the next staff in spaces (should perhaps be passed in).
      prim::number StaffInteriorDistance = Connects(d.m, bt) ?
        (d.h.StaffDistance - 4.0) : 0.0;
      
      if(bt->Get(mica::Value) == mica::StandardBarline)
      {
        Shapes::AddLine(s.Add().p, prim::planar::Vector(0.0, 2.0),
          prim::planar::Vector(0.0, -2.0 - StaffInteriorDistance),
          d.h.BarlineThickness, true, false, false);
      }
      else if(bt->Get(mica::Value) == mica::EndBarline)
      {
        Shapes::AddLine(s.Add().p, prim::planar::Vector(0.0, 2.0),
          prim::planar::Vector(0.0, -2.0 - StaffInteriorDistance),
          d.h.BarlineThickness, true, false, false);
        Shapes::AddLine(s.z().p, prim::planar::Vector(0.8, 2.0),
          prim::planar::Vector(0.8, -2.0 - StaffInteriorDistance),
          d.h.BarlineThickness * 3.0, true, false, false);
      }
      else if(bt->Get(mica::Value) == mica::EndRepeatBarline)
      {
        Shapes::AddLine(s.Add().p, prim::planar::Vector(0.0, 2.0),
          prim::planar::Vector(0.0, -2.0 - StaffInteriorDistance),
          d.h.BarlineThickness, true, false, false);
        Shapes::AddLine(s.z().p, prim::planar::Vector(0.8, 2.0),
          prim::planar::Vector(0.8, -2.0 - StaffInteriorDistance),
          d.h.BarlineThickness * 3.0, true, false, false);
        Shapes::AddCircle(s.z().p, prim::planar::Vector(-0.8, 0.5), 0.5);
        Shapes::AddCircle(s.z().p, prim::planar::Vector(-0.8, -0.5), 0.5);
      }
      else //For as of yet unsupported features
      {
        Shapes::AddLine(s.Add().p, prim::planar::Vector(0.0, 2.0),
          prim::planar::Vector(0.0, -2.0 - StaffInteriorDistance),
          d.h.BarlineThickness, true, false, false);
        
        //Mark in red to show it is being displayed incorrectly.
        s.z().c = Colors::red;
      }
    }
    
    ///Determines whether the barline connects downward.
    static bool Connects(const graph::Music& g, graph::ConstMusicNode bt)
    {
      /*In the future this might instead read for a special barline continue
      span rather than just assuming the connection.*/
      bool BarlineConnects = false;
      {
        graph::ConstMusicNode ParentIsland = g.Previous(bt,
          graph::MusicLabel::Token());
        
        graph::ConstMusicNode NextIsland = g.Next(ParentIsland,
          graph::MusicLabel::Instantwise());
        
        graph::ConstMusicNode NextBarline = g.Next(NextIsland,
          graph::MusicLabel::Token());

        if(NextBarline)
          BarlineConnects = true;
      }
      
      return BarlineConnects;
    }
  };
}}
#endif
