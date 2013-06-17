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

/*Include Belle, Bonne, Sage and compile it in this .cpp file. See the previous
tutorials for an explanation.*/
#define BELLE_COMPILE_INLINE
#include "Belle.h"

int main()
{
  //Helper namespaces
  using namespace prim;
  using namespace belle;
  using namespace belle::graph;
  
  //Create an empty music graph.
  Music g;
  
  //----------------//
  //Treble Clef Part//
  //----------------//

  /*Create barline, clef, key signature, and time signature tokens. Each token
  is created and added to its own island (which is a container for tokens).*/
  MusicNode b = g.CreateAndAddBarline();
  MusicNode c = g.CreateAndAddClef(mica::TrebleClef);
  MusicNode k = g.CreateAndAddKeySignature(mica::TwoSharps, mica::Major);
  MusicNode t = g.CreateAndAddTimeSignature(3, prim::Ratio(1, 4));
  
  //Link the clef island from the barline island
  g.Connect(b, c)->Set(mica::Type) = mica::Partwise;
  
  //Link the key signature island from the clef island.
  g.Connect(c, k)->Set(mica::Type) = 
    mica::Partwise;
  
  //Link the time signature island from the key signature island.
  g.Connect(k, t)->Set(mica::Type) = mica::Partwise;
  
  /*Create a quarter-note chord and link its island partwise from the time
  signature island.*/
  MusicNode chord = g.CreateChord(prim::Ratio(3, 4));
  g.CreateAndAddNote(chord, mica::D4);
  g.CreateAndAddNote(chord, mica::FSharp4);
  MusicNode chordIsland = g.AddChordToNewIsland(chord);
  g.Connect(t, chordIsland)->Set(mica::Type) = mica::Partwise;

  //Create a final barline.
  MusicNode f = g.CreateAndAddBarline(mica::EndBarline);
  g.Connect(chordIsland, f)->Set(mica::Type) = mica::Partwise;
  //--------------//
  //Bass Clef Part//
  //--------------//

  /*Create barline, clef, key signature, and time signature tokens. Each token
  is created and added to its own island (which is a container for tokens).*/
  MusicNode b2 = g.CreateAndAddBarline();
  MusicNode c2 = g.CreateAndAddClef(mica::BassClef);
  MusicNode k2 = g.CreateAndAddKeySignature(mica::TwoSharps, mica::Major);
  MusicNode t2 = g.CreateAndAddTimeSignature(3, prim::Ratio(1, 4));
  
  //Link the clef island from the barline island
  g.Connect(b2, c2)->Set(mica::Type) = mica::Partwise;
  
  //Link the key signature island from the clef island.
  g.Connect(c2, k2)->Set(mica::Type) = mica::Partwise;
  
  //Link the time signature island from the key signature island.
  g.Connect(k2, t2)->Set(mica::Type) = mica::Partwise;
  
  /*Create a quarter-note chord and link its island partwise from the time
  signature island.*/
  MusicNode chord2 = g.CreateChord(prim::Ratio(3, 4));
  g.CreateAndAddNote(chord2, mica::D3);
  g.CreateAndAddNote(chord2, mica::A3);
  MusicNode chordIsland2= g.AddChordToNewIsland(chord2);
  g.Connect(t2, chordIsland2)->Set(mica::Type) = mica::Partwise;
  
  //Create a final barline.
  MusicNode f2 = g.CreateAndAddBarline(mica::EndBarline);
  g.Connect(chordIsland2, f2)->Set(mica::Type) = mica::Partwise;

  //-------------------------------//
  //Link Parts Together Instantwise//
  //-------------------------------//
  g.Connect(b, b2)->Set(mica::Type) = mica::Instantwise;
  g.Connect(c, c2)->Set(mica::Type) = mica::Instantwise;
  g.Connect(k, k2)->Set(mica::Type) = mica::Instantwise;
  g.Connect(t, t2)->Set(mica::Type) = mica::Instantwise;
  g.Connect(chordIsland, chordIsland2)->Set(mica::Type) = mica::Instantwise;
  g.Connect(f, f2)->Set(mica::Type) = mica::Instantwise;

  /*Export the graph, reimport the graph (as a test of serialization), and
  show a visualization of it.*/
  Music h;
  h.ImportXML(g.ExportXML());
  Utility::OpenGraphVisualization(h);
  prim::c >> h.ExportXML();
  prim::c >> h;
  return prim::c.Finish();
}
