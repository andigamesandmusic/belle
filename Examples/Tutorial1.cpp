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

//------------------------------------------------------------------------------

/*
Tutorial 1: Drawing simple graphics manually in Belle

This tutorial explains the graphics abstraction used by Belle, Bonne, Sage. It
assumes familiarity with prim as seen in Tutorial 0.

On Mac/Linux you can build and run from the terminal using:
Scripts/MakeAndRun Tutorial1

For more information related to building, see the README.
*/

//------------------------------------------------------------------------------

//Include Belle, Bonne, Sage and compile it in this .cpp file.
#define BELLE_COMPILE_INLINE
#include "Belle.h"

//These were discussed in Tutorial 0.
using namespace prim;
using namespace prim::planar;

/*The core belle namespace.
It contains classes relevant to drawing such as Affine, Canvas, Color, Font,
Painter, Path, Portfolio, Shapes, Text.*/
using namespace belle;

//Belle has output painters which are rendering targets such as PDF and JUCE.
using namespace belle::painters;

//------------------------------------------------------------------------------

/*In Belle there are three fundamental abstract data types for graphics:
Portfolio, Canvas, and Painter.

The Portfolio contains a list of Canvases and can be thought of as a document
with multiple pages. The user of the library must at least subclass
Canvas, and implement the Paint virtual method. If the user needs the
Portfolio to store any information relevant to the whole document to be accessed
during the Paint, then the Portfolio should also be subclassed.

The Painter is a device-independent vector graphics object and could represent
file or screen-based output.

This example will show how to subclass both Portfolio and Canvas and how to use
the PDF and SVG painters.
*/

//Subclass Score from belle::Portfolio
struct Score : public Portfolio
{
  //An array of rectangles to paint.
  Array<Rectangle> RectanglesToPaint;

  /*Subclass Page from belle::Canvas. Note that Page is a class inside
  a class, so it is really a Score::Page; however, it is not necessary to do it
  this way. It just logically groups the Page class with the Score to which it
  pertains.*/
  struct Page : public Canvas
  {
    //This method gets called once per canvas.
    void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      /*Since we need access to the Score (as opposed to base class Portfolio)
      in order to draw the rectangles, we can forward the paint call to a custom
      paint method which uses a Score& instead of a Portfolio&.*/
      Paint(Painter, dynamic_cast<Score&>(Portfolio));
    }
    
    //Custom paint method with score.
    void Paint(Painter& Painter, Score& Score)
    {
      //Print which page is being painted.
      c >> "Painting page: " << Painter.GetPageNumber();
      
      //Paint each rectangle in the rectangle array.
      for(count i = 0; i < Score.RectanglesToPaint.n(); i++)
      {
        /*Create an empty path. A path is a vector graphics object containing
        a list of core instructions: move-to (start new path), line-to,
        cubic-to (Bezier curve), and close-path. Generally, multiple subpaths
        are interpreted by the rendering targets according to the zero-winding
        rule.*/
        Path p;
        
        /*Add the rectangle shape to the path. The Shapes class contains several
        primitive building methods.*/
        Shapes::AddRectangle(p, Score.RectanglesToPaint[i]);
        
        //Alternate green fill with blue stroke.
        if(i % 2 == 0)
          Painter.SetFill(Colors::green);
        else
          Painter.SetStroke(Colors::blue, 0.01);
        
        //Draw the path, separating the fills and strokes by page.
        if(i % 2 == Painter.GetPageNumber())
          Painter.Draw(p);
      }
    }
  };
};

//This program creates a couple of pages with some rectangles using Belle.
int main()
{
  //Step 1: Create a score, add some pages, and give it some information.
  
  //Instantiate a score.
  Score MyScore;

  //Add a portrait page to the score.
  MyScore.Canvases.Add() = new Score::Page;
  MyScore.Canvases.z()->Dimensions = Paper::Portrait(Paper::Letter);
  
  //Add a landscape page to the score.
  MyScore.Canvases.Add() = new Score::Page;
  MyScore.Canvases.z()->Dimensions = Paper::Landscape(Paper::Letter);
  
  /*Add some rectangles for the score to paint. Note this is just a custom
  member that was created to demonstrate how to pass information to the painter.
  There is nothing intrinsic to the Score about painting rectangles.*/
  const number GeometricConstant = 1.2;
  for(number i = 0.01; i < 8.0; i *= GeometricConstant)
    MyScore.RectanglesToPaint.Add() = Rectangle(Vector(i, i), Vector(i, i) *
      GeometricConstant);
  
  //Step 2a: Draw the score to PDF.
  
  /*Set the PDF-specific properties, for example, the output filename. If no
  filename is set, then the contents of the PDF file end up in
  PDF::Properties::Output.*/
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "Tutorial1.pdf";
  
  /*Write the score to PDF. Note how in Belle, the Canvas Paint() method is
  never called directly. Instead a portfolio creates a render target which then
  calls back the paint method on each canvas. This is an extension of the
  device-independent graphics paradigm.*/
  MyScore.Create<PDF>(PDFSpecificProperties);

  //Print the name of the output file.
  c >> "Wrote PDF to '" << PDFSpecificProperties.Filename << "'.";
  
  /*Step 2b: Here is the same thing except using the SVG renderer. Since SVG is
  an image format, the result will be a sequence of files.*/
  
  /*Set the SVG-specific properties, for example, the output filename prefix. If
  no filename is set, then the contents of the SVG file end up in the
  SVG::Properties::Output array.*/
  SVG::Properties SVGSpecificProperties;
  SVGSpecificProperties.FilenameStem = "Tutorial1-";
  
  //Write the score to SVG.
  MyScore.Create<SVG>(SVGSpecificProperties);

  //Note the name of the output file to console window.
  c >> "Wrote SVGs to '" << SVGSpecificProperties.FilenameStem << "*.svg'.";

  //Finish the console output.
  c.Finish();
  
  return 0;
}
