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

class JIMS
{
private:
  Vector Origin;
  Vector StaffSize;

  //Constants
  number LineWidth;
  number DashSpacing;
  number DashWidth ;
  number Spaces;
  number SpacesArcRight;
  number SpacesArcLeftOuter;
  number SpacesArcLeftInner;
  number SpacesArcHeight;
  number SpacesInnerMiddleWidth;
  number SpacesOuterMiddleWidth;
  number SpacesCircleOffset;
  number SpacesCircleSize;
  number SpacesLedgerXExtent;
  number SpaceHeight;

private:
  void AddDiamond(Path& p, Vector Position)
  {
    const number SpacesXExtent = 1.0;
    const number SpacesYExtent = 0.5;
    const number LineWidth = 0.1;
    p.Add(Instruction(Position - Vector(SpacesXExtent, 0.0) * LineWidth, true));
    p.Add(Instruction(Position + Vector(0.0, SpacesYExtent) * LineWidth));
    p.Add(Instruction(Position + Vector(SpacesXExtent, 0.0) * LineWidth));
    p.Add(Instruction(Position - Vector(0.0, SpacesYExtent) * LineWidth));
    p.Add(Instruction());
    return;
    p.Add(Instruction(Position -
      Vector(SpacesXExtent - LineWidth, 0.0) * LineWidth, true));
    p.Add(Instruction(Position -
      Vector(0.0, SpacesYExtent - LineWidth) * LineWidth));
    p.Add(Instruction(Position +
      Vector(SpacesXExtent - LineWidth, 0.0) * LineWidth));
    p.Add(Instruction(Position +
      Vector(0.0, SpacesYExtent - LineWidth) * LineWidth));
    p.Add(Instruction());
  }

public:

  JIMS(Vector Origin, Vector StaffSize) : Origin(Origin), StaffSize(StaffSize)
  {
    LineWidth = 0.1;
    DashSpacing = 0.1;
    DashWidth = 0.05;
    Spaces = 6.0;
    SpacesArcRight = 5.0;
    SpacesArcLeftOuter = 1.2;
    SpacesArcLeftInner = 2.0;
    SpacesArcHeight = 2.0;
    SpacesInnerMiddleWidth = 2.5;
    SpacesOuterMiddleWidth = 3.2;
    SpacesCircleOffset = 0.8;
    SpacesCircleSize = 0.5;
    SpacesLedgerXExtent = 0.9;
    SpaceHeight = StaffSize.y / 6.0;
  }

  void DrawStaff(Painter& Painter)
  {
    //Draw the staff lines.
    {
      Path p;
      Shapes::AddLine(p, Origin + Vector(0.0, -StaffSize.y / 2.0),
        Origin + Vector(StaffSize.x, -StaffSize.y / 2.0),
        LineWidth * SpaceHeight);
      Shapes::AddLine(p, Origin + Vector(0.0, StaffSize.y / 2.0),
        Origin + Vector(StaffSize.x, StaffSize.y / 2.0),
        LineWidth * SpaceHeight);
      for(number x = Origin.x; x <= Origin.x + StaffSize.x;
        x += DashSpacing)
      {
        Shapes::AddLine(p, Vector(x, Origin.y),
          Vector(Min(x + DashWidth, Origin.x + StaffSize.x), Origin.y),
          LineWidth * SpaceHeight);
      }
      Painter.Draw(p);
    }

    {
      //Draw the arc
      Vector OuterPoints[4] = {
        Origin + Vector(SpacesArcRight * SpaceHeight, -StaffSize.y / 2.0),
        Origin + Vector(SpacesArcLeftOuter * SpaceHeight,
          -SpacesArcHeight * SpaceHeight),
        Origin + Vector(SpacesArcLeftOuter * SpaceHeight,
          SpacesArcHeight * SpaceHeight),
        Origin + Vector(SpacesArcRight * SpaceHeight, StaffSize.y / 2.0)};

      Vector InnerPoints[4] = {
        Origin + Vector(SpacesArcRight * SpaceHeight, -StaffSize.y / 2.0),
        Origin + Vector(SpacesArcLeftInner * SpaceHeight,
          -SpacesArcHeight * SpaceHeight),
        Origin + Vector(SpacesArcLeftInner * SpaceHeight,
          SpacesArcHeight * SpaceHeight),
        Origin + Vector(SpacesArcRight * SpaceHeight, StaffSize.y / 2.0)};

      Path p;
      p.Add(Instruction(OuterPoints[0], true));
      p.Add(Instruction(OuterPoints[1], OuterPoints[2], OuterPoints[3]));
      p.Add(Instruction(InnerPoints[2], InnerPoints[1], InnerPoints[0]));
      p.Add(Instruction());
      Painter.Draw(p);

      //Draw the interior arc-line connectors
      Path l;
      Shapes::AddLine(l, Origin + Vector(0.0, SpaceHeight),
        Origin + Vector(SpacesInnerMiddleWidth * SpaceHeight, SpaceHeight),
        LineWidth * SpaceHeight);
      Shapes::AddLine(l, Origin + Vector(0.0, -SpaceHeight),
        Origin + Vector(SpacesInnerMiddleWidth * SpaceHeight, -SpaceHeight),
        LineWidth * SpaceHeight);
      Shapes::AddLine(l, Origin + Vector(0.0, SpaceHeight * 2.0),
        Origin + Vector(SpacesOuterMiddleWidth * SpaceHeight,
        SpaceHeight * 2.0), LineWidth * SpaceHeight);
      Shapes::AddLine(l, Origin + Vector(0.0, -SpaceHeight * 2.0),
        Origin + Vector(SpacesOuterMiddleWidth * SpaceHeight,
        -SpaceHeight * 2.0), LineWidth * SpaceHeight);

      Painter.Draw(l);
    }

    //Draw the circles
    {
      List<number> Placements;
      Placements.Add() = -2.0;
      Placements.Add() = -1.0;
      Placements.Add() = -0.5;
      Placements.Add() = 0.5;
      Placements.Add() = 1.5;
      Placements.Add() = 2.5;
      
      Path p;
      for(count i = 0; i < Placements.n(); i++)
      {
        Vector CircleOrigin = Origin +
          Vector(SpacesCircleOffset * SpaceHeight, Placements[i] * SpaceHeight);
        Shapes::AddCircle(p, CircleOrigin, SpacesCircleSize * SpaceHeight);
        Shapes::AddCircle(p, CircleOrigin,
          (SpacesCircleSize - LineWidth * 2.0f) * SpaceHeight, false);
      }
      Painter.Draw(p);
    }

    //Draw the diamonds
    {
      //Stroke-fill diamonds to hollow out.
      Path p;
      AddDiamond(p, Origin +
        Vector(SpacesCircleOffset, 3.0) * SpaceHeight);
      AddDiamond(p, Origin +
        Vector(SpacesCircleOffset, -3.0) * SpaceHeight);
      Painter.SetFillAndStroke(Colors::white, Colors::black, SpaceHeight * LineWidth);
      Painter.Draw(p);
      Painter.SetFill(Colors::black); //Reset color back to black fill
    }
  }

  void DrawLedger(Painter& Painter, count Index, number SpacesXOffset)
  {
    Path p;
    Vector a(SpacesXOffset - SpacesLedgerXExtent, (number)Index / 2.0);
    Vector b(SpacesXOffset + SpacesLedgerXExtent, (number)Index / 2.0);
    Shapes::AddLine(p, Origin + a * SpaceHeight, Origin + b * SpaceHeight,
      LineWidth * SpaceHeight);
    Painter.Draw(p);
  }

  void DrawQuarterNote(Painter& Painter, count Index, number SpacesXOffset,
    String Text = "")
  {
    Path p;
    Vector Position = Origin +
      Vector(SpacesXOffset, (number)Index / 2.0) * SpaceHeight;
    Shapes::Music::AddQuarterNote(p, Position,
      SpaceHeight, true, (Index < 0 ? 3.5 : -3.5));
    Painter.Draw(p);

    Painter.Translate(Position + Vector(0.0, (Index < 0 ? -1.3 : 0.8)) * SpaceHeight +
      Vector(-2.0, 0.0));
    Painter.Draw(Text, MyFont, 12.0, Font::Regular,
      Text::Justifications::Center, 4.0);
    Painter.Revert();
  }
};

class Score : public Portfolio
{
public:
};

class Page : public Canvas
{
public:
  virtual void Paint(Painter& Painter, Portfolio& Portfolio)
  {
    JIMS j(Vector(1.0, 9.0), Vector(6.5, 1.0));
    j.DrawStaff(Painter);
    j.DrawQuarterNote(Painter, -6, 6.0, "Do");
    j.DrawQuarterNote(Painter, -4, 10.0, "Re");
    j.DrawLedger(Painter, -4, 10.0);
    j.DrawQuarterNote(Painter, -2, 14.0, "Mi");
    j.DrawLedger(Painter, -2, 14.0);
    j.DrawQuarterNote(Painter, -1, 18.0, "Fa");
    j.DrawQuarterNote(Painter, 1, 22.0, "So");
    j.DrawQuarterNote(Painter, 3, 26.0, "La");
    j.DrawLedger(Painter, 2, 26.0);
    j.DrawLedger(Painter, 4, 26.0);
    j.DrawQuarterNote(Painter, 5, 30.0, "Ti");
    j.DrawQuarterNote(Painter, 6, 34.0, "Do");
  }
};

String DetermineFontPath()
{
  String Dummy;
  if(File::Read("./Joie.bellefont", Dummy))
    return "./";
  else if(File::Read("../Resources/Joie.bellefont", Dummy))
    return "../Resources/";
  else if(File::Read("./Resources/Joie.bellefont", Dummy))
    return "./Resources/";
  else
    c >> "Path to bellefonts could not be determined.";
  return "";
}

void LoadFonts()
{
  //Find the font path.
  String Path = DetermineFontPath();
  if(!Path)
    return;
  
  //Queue up some font files to load.
  String Notation = Path; Notation << "Joie.bellefont";
  String Regular = Path; Regular << "GentiumBasicRegular.bellefont";
  String Bold = Path; Bold << "GentiumBasicBold.bellefont";
  String Italic = Path; Italic << "GentiumBasicItalic.bellefont";
  String BoldItalic = Path; BoldItalic << "GentiumBasicBoldItalic.bellefont";
  
  //Load some typefaces into the font.
  Array<byte> a;
  if(File::Read(Notation, a))
    MyFont.Add(Font::Special1)->ImportFromArray(&a.a());
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
  MyScore.Canvases.Add() = new Page;

  //Set to letter landscape.
  MyScore.Canvases.z()->Dimensions = Paper::Letter;

  //Set the PDF-specific properties.
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "JIMS.pdf";
  
  //Write the score to PDF.
  MyScore.Create<PDF>(PDFSpecificProperties);

  //Note success to console window.
  c >> "Successfully wrote '" << PDFSpecificProperties.Filename << "'.";

  //Add a blank line to the output.
  c++;
  
  return 0;
}
