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

//Font which will be loaded.
Font MyFont;

//An example of deriving a Portfolio and Canvas.
struct Score : public Portfolio
{
  struct Page : public Canvas
  {
    virtual void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      //Some normal text first.
      Painter.Translate(Vector(1.0, 10.0));
      Painter.Draw("Hello, world!", MyFont);
      Painter.Revert();
      
      //Tale of Two Cities -- in changing size, style, and color!
      Text t(MyFont, Font::Regular, 12.0, 6.0);
      
      String s = "It was the best of times, it was the worst of times, "
        "it was the age of wisdom, it was the age of foolishness, it was "
        "the epoch of belief, it was the epoch of incredulity, it was the "
        "season of Light, it was the season of Darkness, it was the spring "
        "of hope, it was the winter of despair, we had everything before us, "
        "we had nothing before us, we were all going direct to heaven, we "
        "were all going direct the other way - in short, the period was so "
        "far like the present period, that some of its noisiest authorities "
        "insisted on its being received, for good or for evil, in the "
        "superlative degree of comparison only.";
      
      t.ImportStringToWords(s);
      
      count k = 0;
      Random r(123);
      for(count i = 0; i < t.Words.n(); i++)
      {
        Word& w = t.Words[i];
        for(count j = 0; j < w.n(); j++, k++)
        {
          Character& l = w[j];
          l.PointSize = j + 9;
          l.Style = (Font::Style)((k / 5) % 4);
          l.FaceColor = Color(r.Between(0.0, 0.5), r.Between(0.0, 0.5),
            r.Between(0.0, 0.5));
        }
      }
      
      t.DetermineLineBreaks();
      t.Typeset();
      Painter.Translate(Vector(1.0, 6.0));
      Painter.Draw(t);
      Painter.Revert();
    }
  };
};

String DetermineFontPath()
{
  String Dummy;
  if(File::Read("./GentiumBasicRegular.bellefont", Dummy))
    return "./";
  else if(File::Read("../Resources/GentiumBasicRegular.bellefont", Dummy))
    return "../Resources/";
  else if(File::Read("./Resources/GentiumBasicRegular.bellefont", Dummy))
    return "./Resources/";
  else
    c >> "Path to GentiumBasic bellefonts could not be determined.";
  return "";
}

void LoadFonts()
{
  //Find the font path.
  String Path = DetermineFontPath();
  if(!Path)
    return;
  
  //Queue up some font files to load.
  String Regular = Path; Regular << "GentiumBasicRegular.bellefont";
  String Bold = Path; Bold << "GentiumBasicBold.bellefont";
  String Italic = Path; Italic << "GentiumBasicItalic.bellefont";
  String BoldItalic = Path; BoldItalic << "GentiumBasicBoldItalic.bellefont";
  
  //Load some typefaces into the font.
  Array<byte> a;
  if(File::Read(Regular, a))
    MyFont.Add(Font::Regular)->ImportFromArray(&a.a());
  if(File::Read(Bold, a))
    MyFont.Add(Font::Bold)->ImportFromArray(&a.a());
  if(File::Read(Italic, a))
    MyFont.Add(Font::Italic)->ImportFromArray(&a.a());
  if(File::Read(BoldItalic, a))
    MyFont.Add(Font::BoldItalic)->ImportFromArray(&a.a());
}

int main()
{
  //Load the fonts.
  LoadFonts();
  
  //Create a score.
  Score MyScore;

  //Add a page to the score.
  MyScore.Canvases.Add() = new Score::Page;

  //Set to letter landscape.
  MyScore.Canvases.z()->Dimensions = Paper::Letter;

  //Set the PDF-specific properties.
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "Text.pdf";
  
  //Write the score to PDF.
  MyScore.Create<PDF>(PDFSpecificProperties);

  //Note success to console window.
  c >> "Successfully wrote '" << PDFSpecificProperties.Filename << "'.";

  //Add a blank line to the output.
  c++;
  
  return 0;
}
