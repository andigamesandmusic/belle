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

#define BELLE_COMPILE_INLINE
#include "Belle.h"

//Use some namespaces to help with scoping.
using namespace prim;
using namespace belle;
using namespace belle::painters;
using namespace prim::planar;

//An example of deriving a Portfolio and Canvas.
struct Score : public Portfolio
{
  struct Page : public Canvas
  {
    virtual void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      number NoteSize = 0.3f;
      number AngleIncrement = 0.8f;
      number CurrentAngle = 0;
      number NoteIncrease = 0.01f;
      number Radius = 1.0f;
      number RadiusIncrease = 0.2f;
      for(count i = 0; i < 20; i++)
      {
        Vector Position(CurrentAngle);
        Position *= Radius;
        
        Path p;
        if(i % 3 == 0)
          Shapes::Music::AddHalfNote(p, Vector(0,0), NoteSize);
        else if(i % 3 == 2)
          Shapes::Music::AddQuarterNote(p, Vector(0,0), NoteSize);
        else
          Shapes::Music::AddWholeNote(p, Vector(0,0), NoteSize);

        Painter.Translate(Vector(8.5f, 11.0f) / 2.0f);
        
        Painter.Scale(0.8f);
        Painter.Translate(Position);
        
        Painter.Rotate(CurrentAngle);
        Painter.SetStroke(Colors::black, 0.01f);
        Painter.Draw(p);

        Painter.Revert(4);
        Radius += RadiusIncrease;
        NoteSize += NoteIncrease;
        CurrentAngle += NoteSize + AngleIncrement;
      }
    }
  };
};

int main()
{
  //Create a score.
  Score MyScore;

  //Add a page to the score.
  MyScore.Canvases.Add() = new Score::Page;

  //Set to letter landscape.
  MyScore.Canvases.z()->Dimensions = Paper::Letter;

  //Set the PDF-specific properties.
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "Spiral.pdf";
  
  //Write the score to PDF.
  MyScore.Create<PDF>(PDFSpecificProperties);

  //Note success to console window.
  c >> "Successfully wrote '" << PDFSpecificProperties.Filename << "'.";

  //Add a blank line to the output.
  c++;
  
  return 0;
}
