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

#ifndef BELLE_ENGRAVER_SPACING_H
#define BELLE_ENGRAVER_SPACING_H

namespace BELLE_NAMESPACE
{
  class Spacing
  {
    public:

    ///Gets the intended width of the system in spaces from the root node.
    static number GetSystemWidth(Music::ConstNode Root)
    {
      Value v = Root ?
        Root->Label.GetState("System", "WidthInSpaces") : Value();
      return v.IsNumber() ? v.AsNumber() : 0.f;
    }

    static Box GetAdditionalBounds(Music::ConstNode Island,
      number HorizontalOffset)
    {
      Box AdditionalBounds;
      if(IsIsland(Island) and Island->Next(MusicLabel(mica::MeasureRest)))
      {
        Music::ConstEdge MeasureRest = Island->Next(
          MusicLabel(mica::MeasureRest), true);
        Vector Left(HorizontalOffset, -1.f);
        Vector Right(HorizontalOffset, 1.f);
        Right += mica::integer(MeasureRest->Get(mica::Value)) ?
          Vector(12.f, 0.f) : Vector(8.f, 0.f);
        AdditionalBounds = Box(Left, Right);
      }
      return AdditionalBounds;
    }

    static void UpdateBordersForStamp(Music::ConstNode Island,
      Pointer<Stamp> IslandStamp, Box::Side S, number HorizontalOffset,
      List<Vector>& Borders)
    {
      if(IslandStamp)
      {
        Array<Box> OpticalBounds = IslandStamp->GetGraphicBounds(
          Affine::Translate(Vector(HorizontalOffset, 0.f)));
        Box AdditionalBounds =
          GetAdditionalBounds(Island, HorizontalOffset);
        if(not AdditionalBounds.IsEmpty())
          OpticalBounds.Add() = AdditionalBounds;
        Borders = Box::SegmentedHull(OpticalBounds, S);
      }
    }

    static Array<List<Vector> > GetInstantBorders(
      const Array<Music::ConstNode>& Instant, Box::Side S,
      number HorizontalOffset)
    {
      Array<List<Vector> > InstantBorders(Instant.n());
      for(count Part = 0; Part < Instant.n(); Part++)
      {
        if(Music::ConstNode Island = Instant[Part])
        {
          UpdateBordersForStamp(Island, Island->Label.Stamp().Object(),
            S, HorizontalOffset, InstantBorders[Part]);
        }
      }
      return InstantBorders;
    }

    static void OffsetInstantBorders(Array<List<Vector> >& InstantBorders,
      number Offset)
    {
      for(count i = 0; i < InstantBorders.n(); i++)
        for(count j = 0; j < InstantBorders[i].n(); j++)
          InstantBorders[i][j] += Vector(Offset, 0.f);
    }

    static void AppendInstantBorders(Array<List<Vector> >& Anchor,
      Array<List<Vector> > Incoming)
    {
      for(count i = 0; i < Incoming.n(); i++)
      {
        if(Incoming[i].n())
          Anchor[i] = Box::MergeHulls(Anchor[i], Incoming[i],
            Box::RightSide);
      }
    }

    static void OffsetAndAppendInstantBorders(Array<List<Vector> >& Anchor,
      Array<List<Vector> > Incoming, number Offset)
    {
      OffsetInstantBorders(Incoming, Offset);
      AppendInstantBorders(Anchor, Incoming);
    }

    static number GetClosestInstantOffset(const Array<List<Vector> >& Anchor,
      const Array<List<Vector> >& Mover)
    {
      number MaximumOffset = Limits<number>::NegativeInfinity();

      for(count i = 0; i < Anchor.n(); i++)
        if(Anchor[i].n() && Mover[i].n())
          MaximumOffset = Max(MaximumOffset, Box::OffsetToPlaceOnSide(
            Anchor[i], Mover[i], Box::RightSide).x);

      return MaximumOffset > 0.f ? MaximumOffset : 0.f;
    }

    static number TokenSpringConstant(Music::ConstNode LeftToken,
      Music::ConstNode RightToken)
    {
      mica::Concept LeftKind = LeftToken->Label.Get(mica::Kind);
      mica::Concept RightKind = RightToken->Label.Get(mica::Kind);

      Music::ConstNode LeftIsland = IslandOfToken(LeftToken);
      Music::ConstNode RightIsland = IslandOfToken(RightToken);

      Ratio LeftDuration = RhythmicDurationOfIsland(LeftIsland);

      number Result = SpringSystem::RodLikeCoefficient();
      if(LeftKind  == mica::Chord and (RightKind == mica::Chord or
        RightKind == mica::Barline or RightKind == mica::Clef))
      {

        Result = Power(1.f / LeftDuration.To<number>(), number(0.8f));
        if(RightKind == mica::Barline)
          Result *= 2.f;
      }

#if 1
      //Deprecated: centering for whole rests.
      Ratio RightDuration = RhythmicDurationOfIsland(RightIsland);
      if((IsRest(RightToken) and RightDuration == 1 and LeftKind != mica::Chord)
        or (IsRest(LeftToken) and LeftDuration == 1 and
          RightKind != mica::Chord))
            Result = 2.0f;
#endif

      //Measure rests
      if(IsIsland(LeftIsland) and
        LeftIsland->Next(MusicLabel(mica::MeasureRest)))
          Result = 1.f;

      return Result;
    }

    static number TokenMinimumDistance(mica::Concept A, mica::Concept B,
      Ratio AOnset, Ratio BOnset, Music::ConstNode LeftIsland)
    {
      Value MinimumDistance = HouseStyle::GetValue(
        LeftIsland, "MinimumDistances", Value(A), Value(B));
      number FinalDistance = MinimumDistance.IsNumber() ?
        MinimumDistance.AsNumber() : 0.f;

      //Add special case for distance between front matter and first chord.
      if(AOnset.IsEmpty() && BOnset == Ratio(0, 1))
      {
        FinalDistance = HouseStyle::GetValue(LeftIsland, "MinimumDistances",
          Value("FrontMatterToChord")).AsNumber();
      }

      //Add special case for island that has ties.
      if(Utility::IslandChordsHaveTies(LeftIsland))
      {
        FinalDistance += HouseStyle::GetValue(LeftIsland, "MinimumDistances",
          Value("TiedChord")).AsNumber();
      }

      return FinalDistance;
    }

    static number IslandAdjacencyMetric(Music::ConstNode A, Music::ConstNode B,
      Ratio AOnset, Ratio BOnset, const String& Category)
    {
      if(!A || !B)
        return 0.f;

      Array<Music::ConstNode> ATokens = A->Children(MusicLabel(mica::Token));
      Array<Music::ConstNode> BTokens = B->Children(MusicLabel(mica::Token));

      if(!ATokens.n() || !BTokens.n())
        return 0.f;

      Music::ConstNode AToken = ATokens.a();
      Music::ConstNode BToken = BTokens.a();

      number Result = 0.f;
      if(Category == "MinimumDistances")
      {
        mica::Concept AType = AToken->Label.Get(mica::Kind);
        mica::Concept BType = BToken->Label.Get(mica::Kind);
        Result = TokenMinimumDistance(AType, BType, AOnset, BOnset, A);
      }
      else if(Category == "SpringConstants")
        Result = TokenSpringConstant(AToken, BToken);

      return Result;
    }

    static Matrix<number> CalculateMinimumDistances(
      const List<Array<Music::ConstNode> >& RhythmOrderedRegion,
      const Matrix<Ratio>& Onsets)
    {
      const count PartCount = RhythmOrderedRegion.a().n();
      Matrix<number> Distances(PartCount, RhythmOrderedRegion.n());
      Distances.Zero();
      for(count Instant = 1; Instant < RhythmOrderedRegion.n(); Instant++)
      {
        for(count Part = 0; Part < PartCount; Part++)
          Distances(Part, Instant) = IslandAdjacencyMetric(
            RhythmOrderedRegion[Instant - 1][Part],
            RhythmOrderedRegion[Instant][Part], Onsets(Part, Instant - 1),
            Onsets(Part, Instant), "MinimumDistances");
      }
      return Distances;
    }

    static void SetSpring(SpringSystem& S, count PartIndex,
      SpringSystem::Node Tail, SpringSystem::Node Head,
      Music::ConstNode PreviousIsland, Music::ConstNode CurrentIsland,
      number SpringWidth)
    {
      if(PreviousIsland && CurrentIsland)
      {
        number SpringConstant = IslandAdjacencyMetric(
          PreviousIsland, CurrentIsland, Ratio(), Ratio(), "SpringConstants");
        S.Connect(Tail, Head)->Label.SetSpring(PartIndex, SpringConstant,
          SpringWidth);
      }
    }

    static Array<number> SolveSpringNetwork(
      const List<Array<Music::ConstNode> >& Instants,
      const Array<number>& MinimumOffsets, number DesiredWidth)
    {
      SpringSystem S;
      Array<SpringSystem::Node> Nodes(Instants.n());
      for(count i = 0; i < Nodes.n(); i++)
        Nodes[i] = S.Add();

      count PartCount = Instants.a().n();
      for(count Instant = 1; Instant < Instants.n(); Instant++)
      {
        for(count Part = 0; Part < PartCount; Part++)
        {
          Music::ConstNode Island = Instants[Instant][Part], PreviousIsland;
          count PreviousInstant = Instant;
          while(Island and not PreviousIsland and PreviousInstant > 0)
            PreviousIsland = Instants[--PreviousInstant][Part];

          number Distance = MinimumOffsets[Instant] -
            MinimumOffsets[PreviousInstant];

          if(PreviousIsland)
            SetSpring(S, Part, Nodes[PreviousInstant], Nodes[Instant],
              PreviousIsland, Island, Distance);
        }
      }

      S.Solve(DesiredWidth);

      Array<number> Solution(Nodes.n());
      for(count i = 0; i < Solution.n(); i++)
        Solution[i] = Nodes[i]->Label.CalculatedX();

      if(Solution.n() >= 2 and Solution.z() < DesiredWidth / 2.f)
      {

        C::Error() >> "There was a problem solving the spring spacing network.";
        C::Error() >> "Elements will be equally spaced as a debugging stopgap.";
        for(count i = 0; i < Solution.n(); i++)
          Solution[i] = number(i) / number(Solution.n() - 1) * DesiredWidth;
      }
      return Solution;
    }

    static Value MinimumSpaceInstantsUsingBorders(Music::ConstNode Root,
      Pointer<const Geometry> G, Value& InstantSpacing)
    {
      Value v;

      //Clear the instant spacing value.
      InstantSpacing.NewArray();

      //Create a rhythm-ordered region version of the system.
      List<Array<Music::ConstNode> > RhythmOrderedRegion;
      Rhythm::CreateRhythmOrderedRegion(G, RhythmOrderedRegion);
      Matrix<Ratio> Onsets = Rhythm::GetUnpackedRhythmicOnsets(
        RhythmOrderedRegion);

      Array<number> TypesetX(RhythmOrderedRegion.n());

      /*Create the leading edge for the first instant, which does not need to
      take into account any past instants.*/
      Array<List<Vector> > LeadingEdge =
        GetInstantBorders(RhythmOrderedRegion.a(), Box::RightSide,
        TypesetX.a() = 0.f);

      Matrix<number> MinimumDistances = CalculateMinimumDistances(
        RhythmOrderedRegion, Onsets);

      count PartCount = RhythmOrderedRegion.a().n();

      for(count Instant = 1; Instant < RhythmOrderedRegion.n(); Instant++)
      {
        //Add the minimum padding.
        for(count Part = 0; Part < PartCount; Part++)
          for(count i = 0; i < LeadingEdge[Part].n(); i++)
            LeadingEdge[Part][i].x += MinimumDistances(Part, Instant);

        //Find the closest this instant may be placed next to the leading edge.
        Array<List<Vector> > InstantBordersLeft = GetInstantBorders(
          RhythmOrderedRegion[Instant], Box::LeftSide, 0.f);
        number Offset = GetClosestInstantOffset(
          LeadingEdge, InstantBordersLeft);

        //Prevent rhythmic order breaches.
        if(Rhythm::IsInstantRhythmicallyAdjacent(Onsets, Instant))
          Offset = Max(Offset, TypesetX[Instant - 1] + 1.5f);

        TypesetX[Instant] = Offset;
        Array<List<Vector> > InstantBordersRight = GetInstantBorders(
          RhythmOrderedRegion[Instant], Box::RightSide,
          Offset);

        AppendInstantBorders(LeadingEdge, InstantBordersRight);
      }
      v["MinimumSpacingWidthInSpaces"] = TypesetX.z();

      //Do spring spacing.
      Array<number> SpringSolution = SolveSpringNetwork(
        RhythmOrderedRegion, TypesetX, GetSystemWidth(Root));
      TypesetX = SpringSolution;

      for(count Instant = 0; Instant < RhythmOrderedRegion.n(); Instant++)
      {
        //Place this instant right after the previous one.
        number InstantX = TypesetX[Instant];
        for(count Part = 0; Part < RhythmOrderedRegion[Instant].n(); Part++)
        {
          if(Music::ConstNode Island = RhythmOrderedRegion[Instant][Part])
          {
            Island->Label.SetState("IslandState", "TypesetX") = InstantX;
            count InstantID = Island->Label.GetState("InstantID").AsCount();
            count PartID = Island->Label.GetState("PartID").AsCount();
            Value& InstantData = InstantSpacing[InstantID];
            InstantData["TypesetX"] = InstantX;
            InstantData["InstantID"] = InstantID;
            InstantData["PartIDs"].Add() = PartID;
            InstantData["Nodes"].Add() = Island;
            //InstantData["Duration"] = ...;
            //InstantData["Bounds"] = InstantBounds;
          }
        }
      }

      number FinalWidth = 0.f;
      {
        for(count i = 0; i < LeadingEdge.n(); i++)
          for(count j = 0; j < LeadingEdge[i].n(); j++)
            FinalWidth = Max(FinalWidth, LeadingEdge[i][j].x);
      }
      v["SpringSpacingWidthInSpaces"] = FinalWidth;
      return v;
    }

    static number MinimumSpaceInstants(Pointer<const Geometry> G,
      Value& InstantSpacing)
    {
      //Clear the instant spacing value.
      InstantSpacing.NewArray();

      //Create a rhythm-ordered region version of the system.
      List<Array<Music::ConstNode> > RhythmOrderedRegion;
      Rhythm::CreateRhythmOrderedRegion(G, RhythmOrderedRegion);

      //Space each instant back to back.
      number TypesetX = 0;
      number FinalWidth = 0;
      for(count Instant = 0; Instant < RhythmOrderedRegion.n(); Instant++)
      {
        //Determine the instant bounds.
        Box InstantBounds;
        for(count Part = 0; Part < RhythmOrderedRegion[Instant].n(); Part++)
        {
          if(Music::ConstNode Island = RhythmOrderedRegion[Instant][Part])
          {
            Pointer<Stamp> S = Island->Label.Stamp().Object();
            if(S)
              InstantBounds += S->GetAbstractBounds();
          }
        }

        /*If the first instant, set the initial instant cursor so that the
        instant will center at x = 0.*/
        if(!Instant)
          TypesetX = InstantBounds.Left();

        number InstantX = TypesetX - InstantBounds.Left();

        //Place this instant right after the previous one.
        for(count Part = 0; Part < RhythmOrderedRegion[Instant].n(); Part++)
        {
          if(Music::ConstNode Island = RhythmOrderedRegion[Instant][Part])
          {
            Island->Label.SetState("IslandState", "TypesetX") = InstantX;
            count InstantID = Island->Label.GetState("InstantID").AsCount();
            count PartID = Island->Label.GetState("PartID").AsCount();
            Value& InstantData = InstantSpacing[InstantID];
            InstantData["TypesetX"] = InstantX;
            InstantData["Bounds"] = InstantBounds;
            InstantData["InstantID"] = InstantID;
            InstantData["PartIDs"].Add() = PartID;
            //InstantData["Duration"] = ...;
            InstantData["Nodes"].Add() = Island;
          }
        }

        //Move the instant cursor over.
        TypesetX += InstantBounds.Width();
        FinalWidth = TypesetX - InstantBounds.Right();
      }

      return FinalWidth;
    }
  };
}
#endif
