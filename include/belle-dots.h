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

void AccumulateRange(Value& x, Value y);
Value RangeOfChords(const Value& ChordsAsClusters);
Value RangeOfDots(const Value& Chords);
Value DotSpaceLocationsBetween(count Lowest, count Highest);
Value DotLocationsWithinExtendedChordSpan(Value Range);
Value DotLocationsWithinChordSpan(Value Range);
Value DotLocationsForChords(const Value& ChordsAsClusters);
Value AssignDotLocationsForChords(const Value& ChordsAsClusters);
void EngraveDots(Music::ConstNode Island, const Value& ChordsAsClusters,
  Stamp& Chord);
void EngraveDotsFromList(Music::ConstNode Island, const Value& Locations,
  number StartX, Stamp& Chord);

#ifdef BELLE_IMPLEMENTATION
void AccumulateRange(Value& x, Value y)
{
  x["Highest"] = x["Highest"].IsNil() ? y["Highest"].AsCount() :
    Max(x["Highest"].AsCount(), y["Highest"].AsCount());
  x["Lowest"] = x["Lowest"].IsNil() ? y["Lowest"].AsCount() :
    Min(x["Lowest"].AsCount(), y["Lowest"].AsCount());
}

Value RangeOfChords(const Value& ChordsAsClusters)
{
  Value v;
  for(count i = 0; i < ChordsAsClusters.n(); i++)
    AccumulateRange(v, ChordsAsClusters[i].a().a()["Range"]);
  return v;
}

Value RangeOfDots(const Value& Chords)
{
  Value v = RangeOfChords(Chords);
  count Notes = 0;
  for(count i = 0; i < Chords.n(); i++)
    for(count j = 0; j < Chords[i].n(); j++)
      for(count k = 0; k < Chords[i][j].n(); k++, Notes++)
        v["Highest"] == Chords[i][j][k]["StaffPosition"] ?
          v["Top"][Chords[i][j][k]["StemDirection"]] = true : Value(),
        v["Lowest"]  == Chords[i][j][k]["StaffPosition"] ?
          v["Bottom"][Chords[i][j][k]["StemDirection"]] = true : Value();
  v["Notes"] = Notes;
  return v;
}

Value DotSpaceLocationsBetween(count Lowest, count Highest)
{
  Value Locations;
  for(count i = Highest; i >= Lowest; i -= 2)
    Locations.Add() = i;
  return Locations;
}

Value DotLocationsWithinExtendedChordSpan(Value Range)
{
  count Lowest  = Range["Lowest"].AsCount();
  count Highest = Range["Highest"].AsCount();
  count LowerExtension  = Range["Notes"].AsCount() == 1 ? 0 : -2;
  count HigherExtension = 2;
  return DotSpaceLocationsBetween(Lowest + (1 - (Lowest % 2)) + LowerExtension,
    Highest - (1 - (Highest % 2)) + HigherExtension);
}

Value DotLocationsWithinChordSpan(Value Range)
{
  count Lowest = Range["Lowest"].AsCount();
  count Highest = Range["Highest"].AsCount();
  return DotSpaceLocationsBetween(
    Lowest - (1 - (Lowest  % 2)), Highest + (1 - (Highest % 2)));
}

Value DotLocationsForChords(const Value& ChordsAsClusters)
{
  Value Range = RangeOfDots(ChordsAsClusters);
  Value Locations = DotLocationsWithinExtendedChordSpan(Range);
  /*Do or do not, there is no try: if it is not possible to assign a dot for
  every note, then we give up on the extended span and just plot the dots within
  the chord span rather than trying to fit as many dots in as we can (BB56).*/
  if(Range["Notes"].AsCount() > Locations.n())
    Locations = DotLocationsWithinChordSpan(Range);
  return Locations;
}

Value AssignDotLocationsForChords(const Value& ChordsAsClusters)
{
  Value Locations = DotLocationsForChords(ChordsAsClusters);
  Value Positions = StaffPositionListFromChordClusters(ChordsAsClusters);
  Value Assignments;
  for(count i = 0; i < Positions.n(); i++)
  {
    count Position = Positions[i]["StaffPosition"].AsCount();
    count Dots = Positions[i]["Dots"].AsCount();
    Value LowestAbove, HighestAbove;
    for(count j = 0; j < Locations.n(); j++)
    {
      if(not Locations[j].IsNil())
      {
        count Location = Locations[j].AsCount();
        LowestAbove = Location >= Position and (LowestAbove.IsNil() or
          Location < LowestAbove.AsCount()) ? Value(Location) : LowestAbove;
        HighestAbove = Location <= Position and (HighestAbove.IsNil() or
          Location > HighestAbove.AsCount()) ? Value(Location) : HighestAbove;
      }
    }
    count Assigned;
    mica::Concept StemDirection = mica::Concept(Positions[i]["StemDirection"]);
    if(StemDirection == mica::Up and LowestAbove.IsInteger())
      Assigned = LowestAbove.AsCount();
    else if(StemDirection == mica::Down and HighestAbove.IsInteger())
      Assigned = HighestAbove.AsCount();
    else
      Assigned = HighestAbove.IsInteger() ? HighestAbove.AsCount() :
        LowestAbove.AsCount();

    if(Dots and Assigned != Nothing<count>())
    {
      for(count j = 0; j < Locations.n(); j++)
        if(Locations[j].AsCount() == Assigned)
          Locations[j].Clear();
      Assignments.Add()["StaffPosition"] = Assigned,
        Assignments.z()["Dots"] = Dots,
        Assignments.z()["Note"] = Positions[i]["Note"];
    }
  }
  return Assignments;
}

void EngraveDotsFromList(Music::ConstNode Island, const Value& Locations,
  number StartX, Stamp& Chord)
{
  const number RhythmicDotNoteheadDistance =
    HouseStyle::GetValue(Island, "RhythmicDotNoteheadDistance").AsNumber();
  const number RhythmicDotSpacing =
    HouseStyle::GetValue(Island, "RhythmicDotSpacing").AsNumber();
  const number RhythmicDotSize =
    HouseStyle::GetValue(Island, "RhythmicDotSize").AsNumber();
  Pointer<Path> DotPath;
  Shapes::AddCircle(*DotPath.New(), Vector(), RhythmicDotSize);
  for(count i = 0; i < Locations.n(); i++)
  {
    for(count j = 0, n = Locations[i]["Dots"].AsCount(); j < n; j++)
    {
      Chord.Add()->p = DotPath;
      Vector DotLocation(StartX + RhythmicDotNoteheadDistance +
        number(j) * RhythmicDotSpacing, +Locations[i]["StaffPosition"] / 2.f);
      Chord.z()->a = Affine::Translate(DotLocation);
    }
  }
}

void EngraveDots(Music::ConstNode Island, const Value& ChordsAsClusters,
  Stamp& Chord)
{
  EngraveDotsFromList(Island, AssignDotLocationsForChords(ChordsAsClusters),
    Chord.Bounds().IsEmpty() ? number(0.f) : Chord.Bounds().Right(), Chord);
}
#endif
