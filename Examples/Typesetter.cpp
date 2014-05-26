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

//Helper namespaces
using namespace prim;
using namespace belle;
using namespace belle::graph;
using namespace belle::painters;

void CreateTest(belle::graph::Music& g)
{
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
}

//Some constants used for the manual layout.
prim::number SpaceHeight = 0.05;
prim::number StaffDistance = 10.0;
prim::number SystemWidth = 6.5;
prim::number SystemWidthSpaces = SystemWidth / SpaceHeight;

//An example of deriving a Portfolio and Canvas.
struct Score : public Portfolio
{
  //Font used by the typesetter
  Font ScoreFont;

  //Music graph to typeset
  graph::Music Music;
  
  //House style
  modern::House HouseStyle;
  
  //Notation cache
  modern::Cache Cache;
  
  //Piece object
  modern::Piece Piece;
  
  //Array of systems produced by the piece
  List<modern::System> Systems;

  ///Default constructor
  Score()
  {
    CreateTest(Music);
    
    {
      Array<byte> Data;
      if(File::Read("./Joie.bellefont", Data)) {}
      else if(File::Read("../Resources/Joie.bellefont", Data)) {}
      else if(File::Read("./Resources/Joie.bellefont", Data)) {}
      else
      {
        c >> "Path to GentiumBasic bellefonts could not be determined.";
        return;
      }
      ScoreFont.Add(Font::Special1)->ImportFromArray(&Data.a());
    }
      
    //Get the notation typeface.
    const Typeface& NotationTypeface = *ScoreFont.GetTypeface(Font::Special1);
    
    //Create the cache.
    Cache.Create(HouseStyle, NotationTypeface);
    
    //Initialize the piece.
    Piece.Initialize(&Music, HouseStyle, Cache, NotationTypeface, ScoreFont);
    
    //Typeset the islands.
    Piece.Typeset(true);
    
    //Create the systems.
    Piece.CreateSystems(Systems, SystemWidthSpaces, SystemWidthSpaces);
    for(count i = 0; i < Systems.n(); i++)
      Systems[i].CalculateSpacing(Music, StaffDistance,
        (i < Systems.n() - 1 ? SystemWidthSpaces : 0.0));
  }
  
  //Page of the score
  struct Page : public Canvas
  {
    //Page paint method
    virtual void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      Score& s = dynamic_cast<Score&>(Portfolio);
      prim::planar::Vector BottomLeft = prim::planar::Vector(1.0,
        Dimensions.y - 1.0);
      for(count i = 0; i < s.Systems.n(); i++)
      {
        BottomLeft -= prim::planar::Vector(0.0, s.Systems[i].Bounds.Height() *
          SpaceHeight);
        s.Systems[i].Paint(Painter, BottomLeft, SpaceHeight);
      }
    }
  };
};

int main()
{
  c++;
  c>>"NOTE: this typesetter example makes use of the Belle proto-engraver.";
  c>>"This engraver can not do anything meaningful in terms of printing music;";
  c>>"rather, it was only created to determine how graphs may be useful as a";
  c>>"data structure for music. As such, the following code directories are";
  c>>"considered deprecated:";
  c++;
  c>>"  Belle/Source/Graph";
  c>>"  Belle/Source/Modern";
  c++;
  c++;

  //Create a score.
  Score MyScore;

  //Add a page to the score.
  MyScore.Canvases.Add() = new Score::Page;

  //Set the PDF-specific properties.
  PDF::Properties PDFSpecificProperties("Typesetter.pdf");

  //Write the score to PDF.
  MyScore.Create<PDF>(PDFSpecificProperties);

  //Note success to console window.
  c >> "Successfully wrote '" << PDFSpecificProperties.Filename << "'.";

  //Add a blank line to the output.
  c++;
  
  return 0;
}
