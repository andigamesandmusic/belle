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

///@name Part State
///@{

///Gets the minimum duration from the voicing leading-edge in a part state.
Ratio GetMinimumDurationFromVoicingLeadingEdge(const Value& VoicingLeadingEdge);

///Accumulates part state for the system.
void AccumulatePartState(Pointer<const Music> M);

///Returns the starting instant ID of the given voice region.
count VoiceRegionStartInstant(const Value& PartStateValue);

///Returns the starting instant ID of the given voice region.
count VoiceRegionEndInstant(const Value& PartStateValue);

///Gets the voice height of the chord.
number VoiceHeightOfChord(Music::ConstNode Chord);

///@}

namespace internals
{
  /**Accumulates all state relevant to a part. Before this method is called,
  The accumulator copies the part state from the partwise-previous island and
  this method updates the values that change.*/
  void AccumulatePartStateForIsland(Music::ConstNode IslandNode);
  void AccumulateVoiceStateForIsland(Music::ConstNode IslandNode);

  ///Accumulates part state from the beginning of the part.
  void AccumulatePartStateFromPartBeginning(Music::ConstNode Island);
  void AccumulateVoiceStateFromPartBeginning(Music::ConstNode Island);

  ///Accumulates part state given the system geometry.
  void AccumulatePartStateForGeometry(Pointer<const Geometry> G);

  ///Accumulates part state given the system geometry.
  void AccumulateVoiceStateForGeometry(Pointer<const Geometry> G);

  ///Gets the overall staff-position height of each voice strand.
  Value HeightOfVoiceStrands(List<Array<Music::ConstNode> >& VoiceStrands);

  void SortVoiceStrandsByHeight(List<Array<Music::ConstNode> >& VoiceStrands);

  ///Initializes the leading-edge information for the part state voicing.
  void InitializeVoicingLeadingEdge(
    const List<Array<Music::ConstNode> >& VoiceStrands,
    Value& VoicingLeadingEdge);

  void BeginVoiceStrandInPartState(Music::ConstNode IslandNode,
    Value& PartStateValue);

  void UpdateVoicingState(Music::ConstNode IslandNode, Value& PartStateValue);

  void UpdateVoicingLeadingEdge(Value& PartStateValue);

  void UpdateVoicingDuration(Value& PartStateValue);
  void ApplyVoiceStrandInformationForChord(Music::ConstNode TokenNode,
    Value& PartStateValue);
  void UpdateStemDirection(Music::ConstNode Chord, mica::Concept StemDirection);
  void UpdateStemDirectionFromStaffPosition(Music::ConstNode Chord);
  void UpdateStemDirectionsByStrandID(
    const List<Array<Music::ConstNode> >& VoiceStrands);
}

#ifdef BELLE_IMPLEMENTATION

Ratio GetMinimumDurationFromVoicingLeadingEdge(const Value& VoicingLeadingEdge)
{
  Ratio MinimumDuration = 0;
  for(count i = 0; i < VoicingLeadingEdge.n(); i++)
  {
    Ratio Existing = VoicingLeadingEdge[i].AsRatio();
    if(!Existing.IsEmpty() and Existing > 0 and
      (MinimumDuration == 0 or Existing < MinimumDuration))
        MinimumDuration = Existing;
  }
  return MinimumDuration;
}

void AccumulatePartState(Pointer<const Music> M)
{
  if(M and M->Root())
  {
    Pointer<const Geometry> G = M->Root()->Label.GetState(
      "System", "Geometry").ConstObject();
    internals::AccumulatePartStateForGeometry(G);
    internals::AccumulateVoiceStateForGeometry(G);
  }
}

count VoiceRegionStartInstant(const Value& PartStateValue)
{
  return PartStateValue["Voicing"]["First"].AsCount();
}

count VoiceRegionEndInstant(const Value& PartStateValue)
{
  return PartStateValue["Voicing"]["Last"].AsCount();
}

number VoiceHeightOfChord(Music::ConstNode Chord)
{
  Value Range = RangeOfChord(Chord);
  return (Range["Lowest"].AsNumber() + Range["Highest"].AsNumber()) / number(2);
}

namespace internals
{
  void ApplyVoiceStrandInformationForChord(Music::ConstNode TokenNode,
    Value& PartStateValue)
  {
    Pointer<const Value> VoicingStrands = GetReferencedValue(
      PartStateValue["Voicing"]["Strands"]);

    if(TokenNode and VoicingStrands)
    {
      Value& VoicingInformation = PartStateValue["Chord"][TokenNode]["Voicing"];

      count StrandID = -1;
      for(count i = 0; i < (*VoicingStrands).n(); i++)
        for(count j = 0; j < (*VoicingStrands)[i].n(); j++)
          if((*VoicingStrands)[i][j].ConstObject() == TokenNode)
            VoicingInformation["StrandID"] = StrandID = i,
            VoicingInformation["IndexInStrand"] = j;

      if(StrandID >= 0)
        PartStateValue["Voicing"]["LeadingEdge"][StrandID] =
          RhythmicDurationOfChord(TokenNode);
    }
  }

  void UpdateVoicingState(Music::ConstNode IslandNode, Value& PartStateValue)
  {
    if(IslandBeginsMultivoiceRegion(IslandNode))
      internals::BeginVoiceStrandInPartState(IslandNode, PartStateValue);
    else
    {
      count Current = InstantIDOfIsland(IslandNode);
      if(VoiceRegionStartInstant(PartStateValue) < Current and
        Current <= VoiceRegionEndInstant(PartStateValue))
          UpdateVoicingLeadingEdge(PartStateValue);
      else
        PartStateValue["Voicing"].Clear();
    }
  }

  Value HeightOfVoiceStrands(
    List<Array<Music::ConstNode> >& VoiceStrands)
  {
    Value VoiceHeights;
    for(count i = 0; i < VoiceStrands.n(); i++)
    {
      number Total = 0;
      number NumberOfChords = 0;
      for(count j = 0; j < VoiceStrands[i].n(); j++)
        if(not IsRest(VoiceStrands[i][j]))
          Total += VoiceHeightOfChord(VoiceStrands[i][j]), NumberOfChords++;
      VoiceHeights.Add() = NumberOfChords > 0 ? Total / NumberOfChords :
        number(0);
    }
    return VoiceHeights;
  }

  void InitializeVoicingLeadingEdge(
    const List<Array<Music::ConstNode> >& VoiceStrands,
    Value& VoicingLeadingEdge)
  {
    VoicingLeadingEdge.NewArray();
    for(count i = 0; i < VoiceStrands.n(); i++)
      VoicingLeadingEdge[i] = Ratio(0, 1);
  }

  void SortVoiceStrandsByHeight(List<Array<Music::ConstNode> >& VoiceStrands)
  {
    Value v = HeightOfVoiceStrands(VoiceStrands);
    for(count i = 0; i < VoiceStrands.n() - 1; i++)
      for(count j = i + 1; j < VoiceStrands.n(); j++)
        if(v[i] < v[j])
          Swap(v[i], v[j]), VoiceStrands.Swap(i, j);
  }

  void UpdateStemDirection(Music::ConstNode Chord, mica::Concept StemDirection)
  {
    Music::ConstNode Island = IslandOfToken(Chord);
    if(IsIsland(Island) and IsChord(Chord))
      Island->Label.SetState("PartState", "Chord")[Chord]["StemDirection"] =
        StemDirection;
  }

  void UpdateStemDirectionFromStaffPosition(Music::ConstNode Chord)
  {
    Music::ConstNode Island = IslandOfToken(Chord);
    if(IsIsland(Island) and IsChord(Chord))
      Island->Label.SetState("PartState", "Chord")[Chord]["StemDirection"] =
        Island->Label.GetState(
          "PartState", "Chord")[Chord]["StemDirectionSingleVoice"];
  }

  void UpdateStemDirectionsByStrandID(
    const List<Array<Music::ConstNode> >& VoiceStrands)
  {
    if(VoiceStrands.n() == 1)
      for(count j = 0; j < VoiceStrands.a().n(); j++)
        UpdateStemDirectionFromStaffPosition(VoiceStrands.a()[j]);
    else
      for(count i = 0; i < VoiceStrands.n(); i++)
        for(count j = 0; j < VoiceStrands[i].n(); j++)
          UpdateStemDirection(VoiceStrands[i][j],
            i % 2 == 0 ? mica::Up : mica::Down);
  }

  void BeginVoiceStrandInPartState(Music::ConstNode IslandNode,
    Value& PartStateValue)
  {
    List<Array<Music::ConstNode> > VoiceStrands;
    FindAllVoiceStrands(IslandNode, VoiceStrands);

    SortVoiceStrandsByHeight(VoiceStrands);
    UpdateStemDirectionsByStrandID(VoiceStrands);

    PartStateValue["Voicing"]["Heights"] = HeightOfVoiceStrands(VoiceStrands);

    AssignValuePointerAsReference(PartStateValue["Voicing"]["Strands"],
      VoiceStrandsAsValue(VoiceStrands));

    PartStateValue["Voicing"]["First"] =
      FirstInstantInVoiceStrands(VoiceStrands);
    PartStateValue["Voicing"]["Last"] =
      LastInstantInVoiceStrands(VoiceStrands);

    InitializeVoicingLeadingEdge(VoiceStrands,
      PartStateValue["Voicing"]["LeadingEdge"]);
  }

  void AccumulatePartStateForIsland(Music::ConstNode IslandNode)
  {
    Array<Music::ConstNode> Tokens = TokensOfIsland(IslandNode);
    if(Tokens.n())
    {
      /*Save the previous staff state before merging in the incoming island
      staff state so that they can be compared later to look for changes.*/
      IslandNode->Label.SetState("PartState", "PreviousStaff") =
        IslandNode->Label.GetState("PartState", "Staff");

      //Merge in the current island-staff state.
      IslandNode->Label.SetState("PartState", "Staff").Merge(
        IslandNode->Label.GetState("IslandState", "Staff"));

      //Accumulate state for the particular type of token.
      Value& PartStateValue = IslandNode->Label.SetState("PartState");

      /*The chord state from the previous island needs to be cleared. Chord
      state is the exception to the part-state copy-to-next rule.*/
      PartStateValue["PreviousChord"] = PartStateValue["Chord"];
      PartStateValue["Chord"].Clear();

      for(count i = 0; i < Tokens.n(); i++)
      {
        Music::ConstNode TokenNode = Tokens[i];
        mica::Concept Kind = TokenNode->Label.Get(mica::Kind);
        if(Kind == mica::Clef)
          AccumulateClefStateForPart(TokenNode, PartStateValue);
        else if(Kind == mica::KeySignature)
          AccumulateKeySignatureStateForPart(TokenNode, PartStateValue);
        else if(Kind == mica::Barline)
          AccumulateBarlineStateForPart(TokenNode, PartStateValue);
        else if(Kind == mica::TimeSignature)
          AccumulateTimeSignatureStateForPart(TokenNode, PartStateValue);
        else if(Kind == mica::Chord)
          AccumulateChordStateForPart(TokenNode,
            IslandNode->Label.SetState());
      }
    }
  }

  void AccumulateVoiceStateForIsland(Music::ConstNode IslandNode)
  {
    Array<Music::ConstNode> Tokens = TokensOfIsland(IslandNode);
    if(Tokens.n())
    {
      Value& PartStateValue = IslandNode->Label.SetState("PartState");
      UpdateVoicingState(IslandNode, PartStateValue);
      for(count i = 0; i < Tokens.n(); i++)
        if(Tokens[i]->Label.Get(mica::Kind) == mica::Chord)
          ApplyVoiceStrandInformationForChord(Tokens[i], PartStateValue);
      UpdateVoicingDuration(PartStateValue);
    }
  }

  void UpdateVoicingDuration(Value& PartStateValue)
  {
    if(PartStateValue.Contains(Value("Voicing")))
      PartStateValue["Voicing"]["Duration"] =
        GetMinimumDurationFromVoicingLeadingEdge(
        PartStateValue["Voicing"]["LeadingEdge"]);
  }

  void UpdateVoicingLeadingEdge(Value& PartStateValue)
  {
    Value& LeadingEdge = PartStateValue["Voicing"]["LeadingEdge"];
    Ratio Minimum = GetMinimumDurationFromVoicingLeadingEdge(LeadingEdge);
    for(count i = 0; i < LeadingEdge.n(); i++)
      if(LeadingEdge[i].AsRatio() > 0)
        LeadingEdge[i] = LeadingEdge[i].AsRatio() - Minimum;
  }

  void AccumulatePartStateFromPartBeginning(Music::ConstNode Island)
  {
    Value PreviousPartState;
    for(Music::ConstNode n = Island; n; n = n->Next(MusicLabel(mica::Partwise)))
    {
      n->Label.SetState("PartState") = PreviousPartState;
      AccumulatePartStateForIsland(n);
      PreviousPartState = n->Label.GetState("PartState");
    }
  }

  void AccumulateVoiceStateFromPartBeginning(Music::ConstNode Island)
  {
    Value PreviousVoiceState;
    for(Music::ConstNode n = Island; n; n = n->Next(MusicLabel(mica::Partwise)))
    {
      n->Label.SetState("PartState", "Voicing") = PreviousVoiceState;
      AccumulateVoiceStateForIsland(n);
      PreviousVoiceState = n->Label.GetState("PartState", "Voicing");
    }
  }

  void AccumulatePartStateForGeometry(Pointer<const Geometry> G)
  {
    for(count i = 0; G and i < G->GetNumberOfParts(); i++)
      for(count j = 0; j < G->GetNumberOfInstants(); j++)
        if(Music::ConstNode n = G->LookupIsland(i, j))
          if(not n->Previous(MusicLabel(mica::Partwise)))
            AccumulatePartStateFromPartBeginning(G->LookupIsland(i, j));
  }

  void AccumulateVoiceStateForGeometry(Pointer<const Geometry> G)
  {
    for(count i = 0; G and i < G->GetNumberOfParts(); i++)
      for(count j = 0; j < G->GetNumberOfInstants(); j++)
        if(Music::ConstNode n = G->LookupIsland(i, j))
          if(not n->Previous(MusicLabel(mica::Partwise)))
            AccumulateVoiceStateFromPartBeginning(G->LookupIsland(i, j));
  }
}

#endif
