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

///@name System wrap
///@{
List<Pointer<Music> > WrapBreakGraph(Pointer<const Music> M,
  Value PotentialBreaks, List<VectorInt> Distribution);
void WrapBreakTies(Pointer<Music> M, Pointer<const class Geometry> G,
  count FirstInstant, count LastInstant);
List<number> WrapCalculateBreakWidths(Value PotentialBreaks);
List<VectorInt> WrapCalculateOptimalBreaks(Value PotentialBreaks,
  number FirstLineWidth, number RemainingLineWidths, number CostPower);
Value WrapCreateBreak(count Instant, Music::ConstNode Island, number TypesetX);
List<VectorInt> WrapDistributeMeasures(List<number> MeasureWidths,
  number FirstLineWidth, number RemainingLineWidths, number CostPower);
count WrapFindInstantOfLastHeaderItem(Pointer<const Music> M);
Value WrapPotentialBreaks(Pointer<const Music> M);
///@}

#ifdef BELLE_IMPLEMENTATION

List<Pointer<Music> > WrapBreakGraph(Pointer<const Music> M,
  Value PotentialBreaks, List<VectorInt> Distribution)
{
  List<Pointer<Music> > SeparatedGraphs;
  const String Original = M->ExportXML();
  for(Counter d; d.z(Distribution); d++)
  {
    Pointer<Music> Copy;
    Copy.New()->ImportXML(Original);
    if(System::MutableGeometry(Copy)->Parse(*Copy))
    {
      Pointer<const class Geometry> G = System::Geometry(Copy);
      count BreakLeft = count(Distribution[d].i());
      count BreakRight = count(Distribution[d].j()) + 1;
      count SelectionFirstItem =
        PotentialBreaks[BreakLeft]["Instant"].AsCount();
      count SelectionLastItem =
        PotentialBreaks[BreakRight]["Instant"].AsCount();
      count HeaderLastItem = WrapFindInstantOfLastHeaderItem(M);
      WrapBreakTies(Copy, G, SelectionFirstItem, SelectionLastItem);

      if(count(d) > 0)
      {
        //Partwise connect header to beginning of selection.
        for(count i = 0; i < G->GetNumberOfParts(); i++)
        {
          Music::Node Left = Copy->Promote(
            G->LookupIsland(i, HeaderLastItem));
          Music::Node Right = Copy->Promote(
            G->LookupIsland(i, SelectionFirstItem));
          Right = Right ? Right->Next(MusicLabel(mica::Partwise)) :
            Music::Node();
          if(IsIsland(Left) and IsIsland(Right))
            Copy->Connect(Left, Right)->Set(mica::Type) = mica::Partwise;
        }

        //Remove section between header and selection.
        for(count i = HeaderLastItem + 1; i <= SelectionFirstItem; i++)
          for(count j = 0; j < G->GetNumberOfParts(); j++)
            RemoveIsland(Copy, G->LookupIsland(j, i));
      }

      //Save the original instant ID and part ID in the wrapped section.
      for(count i = 0; i < G->GetNumberOfParts(); i++)
        for(count j = SelectionFirstItem; j <= SelectionLastItem; j++)
          if(Music::Node n = Copy->Promote(G->LookupIsland(i, j)))
            n->Set("OriginalInstantID") =
              n->Label.GetState("InstantID").AsString(),
            n->Set("OriginalPartID") =
              n->Label.GetState("PartID").AsString();

      //Remove section after selection.
      if(count(d) != Distribution.n() - 1)
        for(count i = SelectionLastItem + 1; i < G->GetNumberOfInstants(); i++)
          for(count j = 0; j < G->GetNumberOfParts(); j++)
            RemoveIsland(Copy, G->LookupIsland(j, i));
      SeparatedGraphs.Add(Copy);
    }
  }
  return SeparatedGraphs;
}

void WrapBreakTies(Pointer<Music> M, Pointer<const class Geometry> G,
  count FirstInstant, count LastInstant)
{
  Phrasing::EngraveTies(M);
  for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
  {
    for(count Instant = FirstInstant; Instant <= LastInstant; Instant++)
    {
      Music::ConstNode Island = G->LookupIsland(Part, Instant);
      Array<Music::ConstNode> Chords = ChordsOfIsland(Island);
      for(count c = 0; c < Chords.n(); c++)
      {
        Array<Music::ConstNode> Notes = NotesOfChord(Chords[c]);
        for(count n = 0; n < Notes.n(); n++)
        {
          Music::ConstNode Note = Notes[n];
          if(NoteHasIncomingTie(Note))
          {
            Music::ConstNode PreviousNote =
              Note->Previous(MusicLabel(mica::Tie));
            if(Music::ConstNode Previous = IslandOfNote(PreviousNote))
              if(Previous->Label.GetState("InstantID").AsCount() < FirstInstant)
                M->Promote(Note)->Set(mica::PartialTieIncoming) =
                  mica::Concept(Previous->GetState(
                  "IslandState", "TieDirections")[PreviousNote]);
          }
          if(NoteHasOutgoingTie(Note))
          {
            if(Music::ConstNode Next =
              IslandOfNote(Note->Next(MusicLabel(mica::Tie))))
              if(Next->Label.GetState("InstantID").AsCount() > LastInstant)
                M->Promote(Note)->Set(mica::PartialTieOutgoing) =
                  mica::Concept(Island->GetState(
                  "IslandState", "TieDirections")[Note]);
          }
        }
      }
    }
  }
}

class WrapCostLabel : public GraphTLabel<String>
{
  public:
  number Cost() {return Get("Cost").ToNumber();}
  bool EdgeEquivalent(const GraphTLabel<String>& L) {(void)L; return true;}
};

List<number> WrapCalculateBreakWidths(Value PotentialBreaks)
{
  List<number> MeasureWidths;
  for(Counter i; i.y(PotentialBreaks); i++)
    MeasureWidths.Add() =
      +PotentialBreaks[i + 1]["TypesetX"] - +PotentialBreaks[i]["TypesetX"];
  return MeasureWidths;
}

List<VectorInt> WrapCalculateOptimalBreaks(Value PotentialBreaks,
  number FirstLineWidth, number RemainingLineWidths, number CostPower)
{
  List<number> MeasureWidths = WrapCalculateBreakWidths(PotentialBreaks);
  List<VectorInt> MeasureBreaks = WrapDistributeMeasures(MeasureWidths,
    FirstLineWidth, RemainingLineWidths, CostPower);
  return MeasureBreaks;
}

Value WrapCreateBreak(count Instant, Music::ConstNode Island, number TypesetX)
{
  Value v;
  v["Instant"] = Instant;
  v["Island"] = Island;
  v["TypesetX"] = TypesetX;
  return v;
}

List<VectorInt> WrapDistributeMeasures(List<number> MeasureWidths,
  number FirstLineWidth, number RemainingLineWidths, number CostPower)
{
  typedef GraphT<WrapCostLabel> CostGraph;
  typedef Pointer<GraphT<WrapCostLabel>::Object> CostNode;
  typedef Pointer<const GraphT<WrapCostLabel>::Object> ConstCostNode;

  List<VectorInt> Distribution;
  if(MeasureWidths.n() > 0 and MeasureWidths > 0.f and
    FirstLineWidth > 0.f and RemainingLineWidths > 0.f)
  {
    CostGraph G;

    Array<CostNode> Breakpoints;
    for(count i = 0; i <= MeasureWidths.n(); i++)
      Breakpoints.Add() = G.Add(), Breakpoints.z()->Set("Measure") = i;

    for(count i = 0; i < MeasureWidths.n(); i++)
    {
      number CurrentWidth = i ? RemainingLineWidths : FirstLineWidth;
      number Length = 0.f;
      for(count j = i; j < MeasureWidths.n() and
        Length <= CurrentWidth; j++)
      {
        if((Length += MeasureWidths[j]) <= CurrentWidth)
        {
          number x = CurrentWidth - Length;
          G.Connect(Breakpoints[i], Breakpoints[j + 1])->Set("Cost") =
            Power(x, CostPower);
        }
      }
    }

    {
      List<ConstCostNode> ShortestPath = G.ShortestPath(
        Breakpoints.a(), Breakpoints.z(), WrapCostLabel());
      count Left = 0;
      for(count i = 1; i < ShortestPath.n(); i++)
      {
        count Right = count(ShortestPath[i]->Get("Measure").ToNumber());
        Distribution.Add(VectorInt(integer(Left), integer(Right) - 1));
        Left = Right;
      }
    }
  }
  return Distribution;
}

count WrapFindInstantOfLastHeaderItem(Pointer<const Music> M)
{
  count LastHeaderItem = -1;
  if(Pointer<const class Geometry> g = System::Geometry(M))
  {
    bool EndOfHeaderFound = false;
    for(count i = 0; not EndOfHeaderFound and i < g->GetNumberOfInstants(); i++)
    {
      if(not g->IsInstantComplete(i))
        EndOfHeaderFound = true;
      else
        for(count j = 0; j < g->GetNumberOfParts(); j++)
          if(Music::ConstNode x = g->LookupIsland(j, i))
            if(not IslandHasBarline(x) and not IslandHasClef(x) and not
              IslandHasKeySignature(x))
                EndOfHeaderFound = true;

      if(not EndOfHeaderFound) LastHeaderItem = i;
    }
  }
  return LastHeaderItem;
}

Value WrapPotentialBreaks(Pointer<const Music> M)
{
  number SystemSpaceHeight = +System::Get(M.Const())["HeightOfSpace"];
  Value Breakpoints;
  Pointer<const class Geometry> g = System::Geometry(M);
  for(count i = 0; i < g->GetNumberOfInstants(); i++)
  {
    Music::ConstNode Breakpoint;
    for(count j = 0; j < g->GetNumberOfParts() and not Breakpoint; j++)
      if(Music::ConstNode x = g->LookupIsland(j, i))
        if(not i or i == g->GetNumberOfInstants() - 1 or IslandHasBarline(x))
          Breakpoint = x;
    if(Breakpoint)
      Breakpoints.Add() = WrapCreateBreak(i, Breakpoint, SystemSpaceHeight *
        +Breakpoint->Label.GetState("IslandState", "TypesetX"));
  }
  return Breakpoints;
}
#endif
