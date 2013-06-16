/*
  ==============================================================================

  Copyright 2007-2012 William Andrew Burnson. All rights reserved.

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

  The views and conclusions contained in the software and documentation are
  those of the authors and should not be interpreted as representing official
  policies, either expressed or implied, of William Andrew Burnson.

  ------------------------------------------------------------------------------

  This file is part of Belle, Bonne, Sage --
    The 'Beautiful, Good, Wise' C++ Vector-Graphics Library for Music Notation 

  ==============================================================================
*/

#define BELLEBONNESAGE_COMPILE_INLINE
#include "BelleBonneSage.h"

//Use some namespaces to help with scoping.
using namespace prim;
using namespace bellebonnesage;
using namespace bellebonnesage::painters;
using namespace prim::planar;

//Resource handle to the test image.
Resource TestResource;

//An example of deriving a Portfolio and Canvas.
struct Score : public Portfolio
{
  struct Page : public Canvas
  {
    virtual void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      //Tile the test image on the page.
      for(prim::number i = 0; i <= 8; i++)
      {
        for(prim::number j = 0; j < 11; j++)
        {
          //Translate to the square at (i, j).
          Painter.Translate(Vector(i, j));
          
          //Paint a one-inch block using the resource key.
          Painter.Draw(TestResource, Vector(1.0, 1.0));
          
          //Revert the translation.
          Painter.Revert();
        }
      }
    }
  };
};

int main()
{
  //Create a score.
  Score MyScore;
  
  //Assign a JPEG file to the resource ID.
  PDF::JPEGImage Test(TestResource, "../Resources/Test.jpg");
  
  //Add the JPEG image resource to the score.
  MyScore.AddImageResource(Test);

  //Add a page to the score.
  MyScore.Canvases.Add() = new Score::Page;

  //Set to letter landscape.
  MyScore.Canvases.z()->Dimensions = Paper::Letter;

  //Set the PDF-specific properties.
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "Image.pdf";
  
  //Write the score to PDF.
  MyScore.Create<PDF>(PDFSpecificProperties);

  //Note success to console window.
  c >> "Successfully wrote '" << PDFSpecificProperties.Filename << "'.";

  //Add a blank line to the output.
  c++;
  
  return 0;
}
