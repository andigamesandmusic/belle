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

#ifndef BELLE_ENGRAVER_SYSTEM_H
#define BELLE_ENGRAVER_SYSTEM_H

namespace BELLE_NAMESPACE
{
  ///Represents a single system of music.
  class System
  {
    public:

    /**Engraves a system according to the given house style. Engraving is the
    process of constructing symbol glyphs and determining their placement. To
    actually paint them to a canvas, the Paint() method is used.*/
    static Value Engrave(Pointer<const Music> M)
    {
      Value v;
      if(!M or not MutableGeometry(M)->Parse(*M)) return v;
      IslandState::Accumulate(M);
      AccumulatePartState(M);
      InstantState::Accumulate(M);
      Island::EngraveIslands(M, GetHouseStyle(M));
      v = SpaceJustify(M);
      MeasureRestEngraveAll(M);
      Phrasing::EngraveTies(M);
      Beaming::EngraveBeams(M);
      Phrasing::EngraveSlurs(M);
      EngraveAllTupletBrackets(M);
      EngraveFloats(M);
      EngraveOctaveTranspositions(M);
      EngravePedalMarkings(M);
      MarkStaffEndsIfNecessary(M);
      return v;
    }

    static void MarkStaffEndsIfNecessary(Pointer<const Music> M)
    {
      Pointer<const class Geometry> G = System::Geometry(M);
      Array<Music::ConstNode> NodesToTry;
      for(count i = 0; i < G->GetNumberOfParts(); i++)
      {
        NodesToTry.Add() = G->LookupIsland(i, 0);
        NodesToTry.Add() = G->LookupIsland(i, G->GetNumberOfInstants() - 1);
      }
      for(Counter i; i.z(NodesToTry); i++)
      {
        Music::ConstNode I = NodesToTry[i];
      }
    }

    static Value SpaceJustify(Pointer<const Music> M)
    {
      Value SpacingResult =
        Spacing::MinimumSpaceInstantsUsingBorders(M->Root(), Geometry(M),
        Get(M)["InstantSpacing"]);

      number SpaceHeight = +Get(M)["HeightOfSpace"];
      number MinimumWidth = +SpacingResult["MinimumSpacingWidthInSpaces"];
      bool WithMinimumWidth =
        Get(M)["JustifyWithAtLeastMinimumWidth"].AsBoolean();
      number MinimumWidthScale = +Get(M)["MinimumWidthScale"];
      bool UseMinimumWidthOnly = MinimumWidthScale > 0.f;
      if(MinimumWidth > +Get(M)["WidthInSpaces"] or UseMinimumWidthOnly)
      {
        if(UseMinimumWidthOnly)
          MinimumWidth *= MinimumWidthScale;
        else
          C::Out() >> "Note: minimum system width (" <<
            MinimumWidth << " spaces) exceeds justified system width (" <<
            +Get(M)["WidthInSpaces"] << " spaces)";
        if(WithMinimumWidth or UseMinimumWidthOnly)
        {
          SetDimensions(M, MinimumWidth * SpaceHeight, SpaceHeight,
            WithMinimumWidth);
          SpacingResult = Spacing::MinimumSpaceInstantsUsingBorders(
            M->Root(), Geometry(M), Get(M)["InstantSpacing"]);
        }
      }

      Value v;
      {
        v["EngravedSpaceWidthInSpaces"] = Get(M)["WidthInSpaces"];
        v["EngravedSpaceWidth"] = Get(M)["Width"];
        v["MinimumSpacingWidthInSpaces"] = MinimumWidth;
        v["MinimumSpacingWidth"] = MinimumWidth * SpaceHeight;
      }
      return v;
    }

    static Value& Get(Music::ConstNode Root)
    {
      return Root->Label.SetState("System").NewTreeIfEmpty();
    }

    static Value& Get(Pointer<const Music> M)
    {
      if(!M or !M->Root())
        C::Error() >> "Error: getting system properties of empty graph.";
      return Get(M->Root());
    }

    static Pointer<class Geometry> MutableGeometry(Pointer<const Music> M)
    {
      if(!M || !M->Root()) return Pointer<class Geometry>().New();

      return Get(M)["Geometry"].NewObjectIfEmpty<class Geometry>();
    }

    static Pointer<const class Geometry> Geometry(Pointer<const Music> M)
    {
      return MutableGeometry(M).Const();
    }

    static void SetHouseStyle(Pointer<const Music> M, Pointer<Value> HouseStyle)
    {
      if(!M || !M->Root() || !HouseStyle) return;
      Get(M)["HouseStyle"] = new Value::ConstReference(HouseStyle);
    }

    static void SetDimensions(Pointer<const Music> M, number SystemWidth,
      number SpaceHeight, bool JustifyWithAtLeastMinimumWidth,
      number MinimumWidthScale = 0.f)
    {
      if(!M || !M->Root()) return;
      Value& SystemProperties = Get(M);
      SystemProperties["Width"] = SystemWidth;
      SystemProperties["HeightOfSpace"] = SpaceHeight;
      SystemProperties["WidthInSpaces"] = SystemWidth / SpaceHeight;
      SystemProperties["JustifyWithAtLeastMinimumWidth"] =
        JustifyWithAtLeastMinimumWidth;
      SystemProperties["MinimumWidthScale"] = MinimumWidthScale;
    }

    static void SetLabel(Pointer<const Music> M, String LabelText)
    {
      if(!M || !M->Root()) return;
      Value& SystemProperties = Get(M);
      SystemProperties["Label"] = LabelText;
    }

    static String GetLabel(Pointer<const Music> M)
    {
      if(!M || !M->Root()) return "";
      Value& SystemProperties = Get(M);
      if(SystemProperties["Label"].IsString())
        return SystemProperties["Label"].AsString();
      return "";
    }

    static Pointer<const Value> GetHouseStyle(Pointer<const Music> M)
    {
      if(!M || !M->Root())
        return Pointer<const Value>();

      if(Pointer<const Value::ConstReference> x =
        Get(M)["HouseStyle"].ConstObject())
          return x->GetPointer();
      return Pointer<const Value>();
    }

    //Paints the system.
    static bool Paint(Pointer<const Music> M, Painter& Painter, Vector Location)
    {
      if(!M || !M->Root())
        return false;
      if(not Geometry(M)->GetNumberOfParts() or
          not Geometry(M)->GetNumberOfInstants())
        return false;

      Get(M)["PaintedBounds"] = Placement::Paint(Painter, M->Root(),
        Get(M)["InstantSpacing"], Location);

      if(Get(M)["PaintedBounds"]["Staves"].IsNil())
        return false;

      if(String LabelText = GetLabel(M))
      {
        Vector LabelOffset = GetPaintedSystemBounds(M).TopLeft();
        LabelOffset.y += 0.1;
        Pointer<Value::ConstReference> H =
          M->Root()->Label.GetState("HouseStyle", "Global").Object();
        Pointer<const Font> NotationFont =
          H->Get()["NotationFont"].ConstObject();
        ScopedAffine LabelAffine(Painter, Affine::Translate(LabelOffset));
        Painter.Draw(LabelText, *NotationFont, 10.f, Font::Regular);
      }

      return true;
    }

    ///Get the painted bounds of just the staff objects.
    static Array<Box> GetPaintedStaffBounds(Pointer<const Music> M)
    {
      Array<Box> a;
      if(!M || !M->Root())
        return a;

      Value v = Get(M)["PaintedBounds"]["StaffBounds"];
      a.n(v.n());
      for(count i = 0; i < v.n(); i++)
        a[i] = v[i].AsBox();
      return a;
    }

    ///Gets the staff-line offset information of each staff in the system.
    static Value GetStaffLineOffsets(Pointer<const Music> M)
    {
      if(!M || !M->Root()) return Value();
      return Get(M)["PaintedBounds"]["Staves"];
    }

    ///Gets the brace and bracket ids.
    static Value GetBracketIds(Pointer<const Music> M)
    {
      if(!M || !M->Root()) return Value();
      return Get(M)["PaintedBounds"]["Ids"];
    }

    ///Get the painted bounds of the system.
    static Box GetPaintedSystemBounds(Pointer<const Music> M)
    {
      Array<Box> a = GetPaintedStaffBounds(M);
      Box r;
      for(count i = 0; i < a.n(); i++)
        r += a[i];
      return r;
    }

    ///Returns the number of staves in the system.
    static count StaffCount(Pointer<const Music> M)
    {
      if(!M || !M->Root()) return 0;

      if(Pointer<const class Geometry> G = Geometry(M))
        return G->GetNumberOfParts();

      return 0;
    }

    ///Returns an array of all the islands in the system.
    static Array<Music::ConstNode> GetIslands(Pointer<const Music> M)
    {
      Array<Music::ConstNode> Islands;
      if(M)
        if(Pointer<const class Geometry> G = Geometry(M))
          for(count i = 0; i < G->GetNumberOfParts(); i++)
            for(count j = 0; j < G->GetNumberOfInstants(); j++)
              if(Music::ConstNode Island = G->LookupIsland(i, j))
                Islands.Add() = Island;
      return Islands;
    }

    ///Returns an array of all the islands in the system.
    static Array<Music::Node> GetIslandsMutable(Pointer<Music> M)
    {
      Array<Music::ConstNode> ConstIslands = GetIslands(M.Const());
      Array<Music::Node> Islands;
      for(count i = 0; i < ConstIslands.n(); i++)
        Islands.Add() = M->Promote(ConstIslands[i]);
      return Islands;
    }

    ///Sets metadata to space the staves within the system.
    static void SpaceStaves(Pointer<Music> M, number SpacesBetweenStaves)
    {
      MusicIterator Iterator;
      Iterator.Start(M.Const());
      while(Iterator.NextIsland())
      {
        Music::ConstNode nc = Iterator.Island();
        Music::Node n = M->Promote(nc);
        count Part = 0;
        Music::ConstNode np = nc;
        while((np = np->Previous(MusicLabel(mica::Instantwise))))
          Part++;
        n->Set("StaffConnects") = "true";
        n->Set("StaffLines") = "5";
        n->Set("StaffOffset") = String(Part * -SpacesBetweenStaves);
        n->Set("StaffScale") = "1.0";
      }
    }

    ///Perform simple system vertical spacing.
    static List<number> SpaceSystems(List<Pointer<const Music> > Systems,
      number yPositionOfBottomStaff, number yPositionOfTopStaff,
      number SpaceHeight, number SpacesStaffToStaffDistance,
      number SpacesMinimumSystemToSystem, number SpacesMaximumSystemToSystem)
    {
      if(!Systems.n())
        return List<number>();
      //Calculate the space-wise height available.
      number HeightAvailableInSpaces = (yPositionOfTopStaff -
        yPositionOfBottomStaff) / SpaceHeight;

      //First space the staves within the systems and get the system heights.
      List<number> SystemHeights;
      for(count i = 0; i < Systems.n(); i++)
      {
        //Get the staff count of the system and determine the system
        Pointer<const Music> M = Systems[i];

        //Count the number of staves and return if a system has no staves.
        count Staves = Max(StaffCount(M), count(1));
        if(!Staves)
          return List<number>();

        SystemHeights.Add() = number(Staves - 1) * SpacesStaffToStaffDistance;

        /*Starting at zero and using decreasing y-position, produce evenly
        spaced y-offsets.*/
        number yOffset = 0.f;
        Music::ConstNode m;
        for(m = M->Root(); m; m = m->Next(MusicLabel(mica::Instantwise)))
        {
          if(Limits<number>::IsNaN(yOffset))
            yOffset = 0.f;
          m->Label.SetState("IslandState", "Staff", "Offset") = yOffset;
          yOffset -= SpacesStaffToStaffDistance;
        }
      }

      //Calculate the total system height.
      number TotalSystemHeight = 0.f;
      for(count i = 0; i < SystemHeights.n(); i++)
        TotalSystemHeight += SystemHeights[i];

      //Check to see whether the unpadded systems exceed available space.
      if(TotalSystemHeight > HeightAvailableInSpaces)
        return List<number>(); //System spacing failed.

      /*Calculate the spaced system positions starting with the first system at
      the top of the page.*/
      List<number> SystemPositions;
      SystemPositions.Add() = yPositionOfTopStaff;

      //If just one system, then it will simply print at the top of the page.
      if(Systems.n() <= 1)
        return SystemPositions;

      //Calculate the range of total system-padded spacing heights.
      number PaddingNumber = number(Systems.n() - 1);
      number MinimumSpacingHeight = TotalSystemHeight +
        PaddingNumber * SpacesMinimumSystemToSystem;
      number MaximumSpacingHeight = TotalSystemHeight +
        PaddingNumber * SpacesMaximumSystemToSystem;

      //Check to see if the minimum spacing overflows the page.
      if(MinimumSpacingHeight > HeightAvailableInSpaces)
        return List<number>(); //System spacing failed.

      {
        /*Check to see if the maximum spacing underflows the page, otherwise
        adjust the padding so that the last staff of the last system meets up
        with the desired position of the bottom staff.*/
        number Padding;
        if(MaximumSpacingHeight <= HeightAvailableInSpaces)
          Padding = SpacesStaffToStaffDistance; //Revert to the minimum.
        else
        {
          Padding = (HeightAvailableInSpaces - TotalSystemHeight) /
            number(Systems.n() - 1);
        }

        //Calculate system positions.
        number yOffset = 0;
        for(count i = 1; i < Systems.n(); i++)
        {
          yOffset -= SystemHeights[i - 1] + Padding;
          SystemPositions.Add() = yPositionOfTopStaff + yOffset * SpaceHeight;
        }
      }

      //Return the system positions.
      return SystemPositions;
    }

    ///Debug graph.
    static void DebugGraph(Pointer<const Music> M)
    {
      Music::ConstNode m, n;
      for(m = M->Root(); m; m = m->Next(MusicLabel(mica::Instantwise)))
        for(n = m; n; n = n->Next(MusicLabel(mica::Partwise)))
          C::Out() >> JSON::Export(n->Label.GetState("PartState"));
    }

    ///Visualizes a system of engraved stamps.
    void Visualize(Pointer<const Music> M, Painter& Painter,
      number Scale = 0.03f, number Spacing = 10.f)
    {
      ScopedAffine AffineScale(Painter, Affine::Scale(Scale));
      Path p;
      const number IslandSize = Spacing / 2.f;
      Shapes::AddCircle(p, Vector(), IslandSize);

      Music::ConstNode m, n;
      number i = 0.f, j = 0.f;
      for(m = M->Root(); m; m = m->Next(MusicLabel(mica::Instantwise)))
      {
        for(n = m; n; n = n->Next(MusicLabel(mica::Partwise)))
        {
          ScopedAffine AffinePosition(Painter, Affine::Translate(Vector(i, j)));
          Painter.SetStroke(Colors::Gray(), 0.1f);
          Painter.Draw(p);
          if(Pointer<Stamp> s = n->Stamp().Object())
            s->Paint(Painter);
          i += Spacing;
        }
        i = 0.f;
        j -= Spacing;
      }
    }
  };
}
#endif
