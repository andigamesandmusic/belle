/*
  ==============================================================================

  Copyright 2007-2013, 2017 William Andrew Burnson
  Copyright 2013-2016 Robert Taub

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

  ==============================================================================
*/

#ifndef BELLE_ENGRAVER_PLACEMENT_H
#define BELLE_ENGRAVER_PLACEMENT_H

namespace BELLE_NAMESPACE
{
  ///Places stamp graphics on the page by painting them.
  class Placement
  {
    public:

    static Pointer<Path> GlyphByIndex(Pointer<const Typeface> t, count i)
    {
      Pointer<Path> p = new Path;
      if(t && t->LookupGlyph(unicode(i)))
        *p = *t->LookupGlyph(unicode(i));
      return p;
    }

    /**Finds staff brackets on the first instant and paints them. This will take
    into account multiple levels of brace, square bracket, and thin square
    bracket.*/
    static Value PaintStaffBrackets(Painter& Painter, Music::ConstNode Root,
      const Value& Offsets)
    {
      Value Ids;

      //Find all the brackets.
      Array<VectorInt> SquareBrackets, ThinSquareBrackets, Braces;
      Music::ConstNode m;
      for(m = Root; m; m = m->Next(MusicLabel(mica::Instantwise)))
      {
        count mi = m->Label.GetState("PartID").AsCount();
        if(Music::ConstNode b = m->Next(MusicLabel(
          mica::StaffBracket, mica::SquareBracket)))
            SquareBrackets.Add() = VectorInt(integer(mi),
              b->Label.GetState("PartID").AsInteger());
        if(Music::ConstNode b = m->Next(MusicLabel(
          mica::StaffBracket, mica::ThinSquareBracket)))
            ThinSquareBrackets.Add() = VectorInt(integer(mi),
              b->Label.GetState("PartID").AsInteger());
        if(Music::ConstNode b = m->Next(MusicLabel(
          mica::StaffBracket, mica::Brace)))
            Braces.Add() = VectorInt(integer(mi),
              b->Label.GetState("PartID").AsInteger());

        //Initialize bracket and brace IDs to -1 to start.
        Ids["BracketIds"].Add() = -1;
        Ids["BraceIds"].Add() = -1;
      }

      //Determine the staff bracket levels for each island.
      Array<count> BracketLevels(Offsets.n());
      BracketLevels.Zero();
      for(count i = 0, Id = 0; i < SquareBrackets.n(); i++, Id++)
        for(count j = count(SquareBrackets[i].i());
          j <= count(SquareBrackets[i].j()); j++)
            BracketLevels[j]++, Ids["BracketIds"][j] = Id;
      for(count i = 0; i < ThinSquareBrackets.n(); i++)
        for(count j = count(ThinSquareBrackets[i].i());
          j <= count(ThinSquareBrackets[i].j()); j++)
            BracketLevels[j]++;
      for(count i = 0, Id = 0; i < Braces.n(); i++)
        for(count j = count(Braces[i].i()); j <= count(Braces[i].j()); j++)
          BracketLevels[j]++, Ids["BraceIds"][j] = Id;

      //Paint the square brackets.
      for(count i = 0; i < SquareBrackets.n(); i++)
      {
        count TopStaff = count(Min(
          SquareBrackets[i].i(), SquareBrackets[i].j()));
        count BottomStaff = count(Max(
          SquareBrackets[i].i(), SquareBrackets[i].j()));
        number x = HouseStyle::GetValue(Root, "StaffBracketDistance",
          Value("SquareBracket"));
        number yExtent = HouseStyle::GetValue(Root, "SquareBracketExtent");
        number Thickness = HouseStyle::GetValue(Root, "SquareBracketThickness");
        Vector Top(x, Offsets["Top"][TopStaff].AsNumber() + yExtent);
        Vector Bottom(x, Offsets["Bottom"][BottomStaff].AsNumber() - yExtent);

        Path p;
        Shapes::AddLine(p, Top, Bottom, Thickness, true, false, false);

        //Draw the curled ends.
        {
          number Squash = HouseStyle::GetValue(Root, "SquareBracketCurlSquash");
          Vector TopExtent(-Thickness / 2.f, -yExtent / 2.f);
          Vector BottomExtent(-Thickness / 2.f, yExtent / 2.f);
          Vector SquashVector(1.f, Squash);
          {
            Pointer<Value::ConstReference> H =
              Root->Label.GetState("HouseStyle", "Global").Object();
            p.Append(*SMuFLGlyphFromTypeface(
              HouseStyle::GetTypeface(Root), mica::BracketTop),
              Affine::Translate(Top + TopExtent) * Affine::Scale(SquashVector));
            p.Append(*SMuFLGlyphFromTypeface(
              HouseStyle::GetTypeface(Root), mica::BracketBottom),
              Affine::Translate(Bottom + BottomExtent) *
              Affine::Scale(SquashVector));
          }
        }
        Painter.Draw(p);
      }

      //Paint the thin square brackets.
      for(count i = 0; i < ThinSquareBrackets.n(); i++)
      {
        count TopStaff =
          count(Min(ThinSquareBrackets[i].i(), ThinSquareBrackets[i].j()));
        count BottomStaff =
          count(Max(ThinSquareBrackets[i].i(), ThinSquareBrackets[i].j()));
        count ThinSquareBracketLevel = count(Min(Max(BracketLevels[TopStaff],
          BracketLevels[BottomStaff]), count(2))) - 1;
        number x = HouseStyle::GetValue(Root, "StaffBracketDistance",
          Value("ThinSquareBracket"), Value(ThinSquareBracketLevel));
        number Thickness = HouseStyle::GetValue(Root, "StaffLineThickness");
        Vector NE(0.f, Offsets["Top"][TopStaff]);
        Vector SE(0.f, Offsets["Bottom"][BottomStaff]);
        Vector NW = NE, SW = SE;
        NW.x += x;
        SW.x += x;

        Path p;
        Shapes::AddLine(p, NW, NE, Thickness);
        Shapes::AddLine(p, NW, SW, Thickness);
        Shapes::AddLine(p, SW, SE, Thickness);
        Painter.Draw(p);
      }

      //Paint the braces.
      for(count i = 0; i < Braces.n(); i++)
      {
        count TopStaff = count(Min(Braces[i].i(), Braces[i].j()));
        count BottomStaff = count(Max(Braces[i].i(), Braces[i].j()));
        count BraceLevel = count(Max(BracketLevels[TopStaff],
          BracketLevels[BottomStaff])) - 1;
        number x = HouseStyle::GetValue(Root, "StaffBracketDistance",
          Value("Brace"), Value(BraceLevel));
        Vector Top(x, Offsets["Top"][TopStaff]);
        Vector Bottom(x, Offsets["Bottom"][BottomStaff]);

        Path p;
        Shapes::Music::AddBrace(p, (Top + Bottom) / 2.f, (Top - Bottom).y);
        Painter.Draw(p);
      }

      return Ids;
    }

    /**Paints the staff lines to the system. Returns a value table of offsets
    with entries for the top, middle, and bottom of the staff.*/
    static Value PaintStaffLines(Painter& Painter, Music::ConstNode Root,
      number SystemSpaceWidth, Value& Offsets)
    {
      //Clear the offsets.
      Offsets.NewTree();
      Offsets["Top"].NewArray();
      Offsets["Middle"].NewArray();
      Offsets["Bottom"].NewArray();

      //Create staff line positions array.
      Value StaffLinePositions;
      StaffLinePositions.NewArray();

      //Bounds on the staff lines.
      Box PaintedBounds;

      //Staff lines to construct and paint.
      Path StaffLines;

      //Get the current space of the painter.
      Affine CurrentSpace = Painter.CurrentSpace();

      //Iterate through each staff.
      Music::ConstNode m;
      for(m = Root; m; m = m->Next(MusicLabel(mica::Instantwise)))
      {
        /*Calculate the bounds on the left and right stamps to help determine
        the actual horizontal extents of the staff lines.*/
        Pointer<Stamp> LeftStamp = m->Label.Stamp().Object();
        Pointer<Stamp> RightStamp;
        {
          Music::ConstNode n;
          for(n = m; n; n = n->Next(MusicLabel(mica::Partwise)))
            RightStamp = n->Label.Stamp().Object();
        }
#if 0
        /*Used to use these, but sometimes floats extend the staff lines beyond
        the actual barline.*/
        Box LeftBarlineBounds = LeftStamp->GetAbstractBounds();
        Box RightBarlineBounds = RightStamp->GetAbstractBounds();
#endif

        //Calculate the staff line thickness and extent.
        number Thickness = HouseStyle::GetValue(m, "StaffLineThickness");
        number ThicknessExtent = Thickness / 2.f;

        //Calculate the left and right extents of the staff lines.
        number LeftExtent = ThicknessExtent;
        number RightExtent = SystemSpaceWidth - ThicknessExtent;

        //Get the y-offset of the staff.
        number yOffset = m->Label.GetState("IslandState", "Staff", "Offset");
        if(Limits<number>::IsNaN(yOffset))
          yOffset = 0.f;

        //Retrieve the number of staff lines to print.
        integer StaffLineCount = m->Label.GetState(
          "IslandState", "Staff", "Lines");

        //If no staff lines are visible.
        if(StaffLineCount <= 0)
          return Value();

        //Calculate staff height and vertical extents.
        number StaffHeight = number(StaffLineCount - 1);
        number TopLineOffset = StaffHeight / 2.f;
        number BottomLineOffset = -TopLineOffset;

        //Create each staff line.
        Value StaffLineOffsets;
        StaffLineOffsets["Lines"] = StaffLineCount;
        Path CurrentStaffLines;
        count StaffLinesToPaint = count(TopLineOffset - BottomLineOffset);
        for(count i = 0; i <= StaffLinesToPaint; i++)
        {
          number j = BottomLineOffset + number(i);
          Vector a(LeftExtent -
            HouseStyle::GetValue(m, "BarlineThickness").AsNumber() / 2.f,
            j + yOffset);
          Vector b(RightExtent +
            HouseStyle::GetValue(m, "BarlineThickness").AsNumber() / 2.f,
            j + yOffset);
          Shapes::AddLine(CurrentStaffLines, a, b, Thickness);
        }
        StaffLines.Append(CurrentStaffLines);
        Box r = CurrentStaffLines.Bounds(CurrentSpace);
        StaffLineOffsets["StaffLineBounds"] = r;
        StaffLinePositions.Add() = StaffLineOffsets;

        //Store the offsets
        Offsets["Top"].Add() = TopLineOffset + yOffset;
        Offsets["Middle"].Add() = yOffset;
        Offsets["Bottom"].Add() = BottomLineOffset + yOffset;

        //Get the page-space bounds of the staff lines.
        PaintedBounds += StaffLines.Bounds(CurrentSpace);
      }

      //Draw the staff lines to page.
      Painter.Draw(StaffLines);

      //Return the painted bounds of the staff lines.
      return StaffLinePositions;
    }

    ///Paints the system and returns the bounding boxes.
    static Value Paint(Painter& Painter, Music::ConstNode Root,
      Value InstantSpacing, Vector SystemPosition)
    {
      if(!Root) return Value();

      //Bounds on the staff objects.
      Value Bounds;

      //Calculated system width in spaces.
      number SystemSpaceWidth = Root->Label.GetState("System", "WidthInSpaces");
      number SystemSpaceHeight =
        Root->Label.GetState("System", "HeightOfSpace");
      Color DefaultColor = Colors::Black();
      {
        Value SystemDefaultColor =
          Root->Label.GetState("System", "DefaultColor");
        if(not SystemDefaultColor.IsNil())
        {
          DefaultColor.R = float32(+SystemDefaultColor["R"]);
          DefaultColor.G = float32(+SystemDefaultColor["G"]);
          DefaultColor.B = float32(+SystemDefaultColor["B"]);
          DefaultColor.A = float32(+SystemDefaultColor["A"]);
          DefaultColor.Validate();
        }
      }

      //Transform to staff space.
      ScopedAffine a1(Painter, Affine::Translate(SystemPosition));
      ScopedAffine a2(Painter, Affine::Scale(SystemSpaceHeight));

      //Reset the paint color to black.
      Painter.SetFill(DefaultColor);

      //Show the staff lines and get the staff offsets.
      Value Offsets;
      Bounds["Staves"] = PaintStaffLines(Painter, Root, SystemSpaceWidth,
        Offsets);

      //Draw the staff brackets.
      Bounds["Ids"] = PaintStaffBrackets(Painter, Root, Offsets);

      //Paint each of the islands in the instant spacer.
      Bounds["StaffBounds"].NewArray();
      for(count i = 0; i < InstantSpacing.n(); i++)
      {
        const Value& in = InstantSpacing[i];
        Box StaffBounds;
        for(count j = 0; j < in["PartIDs"].n(); j++)
        {
          //Get the y-offset of the staff the island is on.
          number y = Offsets["Middle"][in["PartIDs"][j]];

          //Get the island from the geometry grid.
          Music::ConstNode n = in["Nodes"][j].ConstObject();

          if(n)
          {
            //For this island, get the stamp.
            Pointer<Stamp> s = n->Label.Stamp().Object();

            if(s)
            {
              //Reset the paint color to black.
              Painter.SetFill(DefaultColor);

              //Place the stamp on the page.
              s->a = Affine::Translate(Vector(in["TypesetX"].AsNumber(), y));

              //Paint the stamp.
              s->Paint(Painter);

              //Union staff bounds with the stamp painted bounds.
              Bounds["StaffBounds"][j] = Bounds["StaffBounds"][j].AsBox()
                + s->GetPaintedBounds();

              //Reset the paint color to black.
              Painter.SetFill(DefaultColor);
            }
          }
        }
      }

      //Return painted bounds.
      return Bounds;
    }
  };
}
#endif
