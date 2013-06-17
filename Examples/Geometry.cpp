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

#define PRIM_WITH_TIMER
#define BELLE_COMPILE_INLINE
#include "Belle.h"

int main()
{
  using namespace belle::graph;
  IslandGrid g;
  Geometry p;
  for(prim::count i = 0; i < 4; i++)
  {
    switch(i)
    {
      case 0: g.MakeOssiaAndStaffChangeTest(); break;
      case 1: g.MakeCrossTest(); break;
      case 2: g.MakeSpiralTest(); break;
      case 3: g.MakeInstantTest(); break;
    }
    prim::Timer t;
    t.Start();
    p.Parse(g);
    prim::c >> "Parse Time: " << (t.Stop() * 1000.0) << " ms";
    prim::c >> "" >> "Part Assignment:";
    g.PrintIDs(true);
    prim::c >> "" >> "Instant Assignment:";
    g.PrintIDs(false);
    prim::c >> "" >> "Part Extents:";
    
    for(prim::count i = 0; i < p.GetNumberOfParts(); i++)
      prim::c >> "Part " << i << ": " << p.GetPartRange(i);
    
    prim::c >> "" >> "Instant Part Membership:";
    for(prim::count i = 0; i < p.GetNumberOfInstants(); i++)
    {
      prim::List<prim::count> l;
      p.GetPartListForInstant(i, l);
      prim::c >> "Instant " << i << ": " << l <<
        (p.IsInstantComplete(i) ? " (complete) " : " ");
    }
    //g.WriteToFile("test");
    //Utility::OpenGraphVisualization(g);
    //break;
    prim::c++;
  }
  TransitiveMapping::Explain();
  return prim::c.Finish();
}
