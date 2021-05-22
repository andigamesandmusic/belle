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

/**
@name Stems

Functions that implement stem placement calculations
@{
*/

#ifdef BELLE_IMPLEMENTATION

void AccumulateStemInformationForChord(const Value& Chord,
  Value& StemInformation, Vector Offset)
{
  bool StemUp = mica::Concept(Chord.a().a()["StemDirection"]) == mica::Up;
  Value StartNote = StemUp ? Chord.a().a() : Chord.z().z();
  Value EndNote   = StemUp ? Chord.z().z() : Chord.a().a();

  Value Stem;
  Stem["X"] = StartNote["StemLocation"].AsVector().x;
  Stem["StartY"] = +StartNote["StaffPosition"] / 2.f;
  Stem["EndY"] = +EndNote["StaffPosition"] / 2.f;
  Stem["JoinOffset"] = StartNote["StemLocation"].AsVector().y - +Stem["StartY"];
  Stem["StemUp"] = StemUp;
  Stem["Height"] = StemHeightForPositionAndStemDirection(
    EndNote["StaffPosition"].AsCount(), StemUp, StartNote["Duration"],
    Chord.a().a()["StemHasBeam"]);
  Stem["ChordOffset"] = Offset;
  Stem["Duration"] = StartNote["Duration"];
  Stem["StemHasBeam"] = Chord.a().a()["StemHasBeam"];

  Music::ConstNode ChordNode = Chord.a().a()["Chord"].ConstObject();
  if(IsChord(ChordNode)){
    IslandOfToken(ChordNode)->SetState(
      "PartState", "Chord")[ChordNode]["Stem"] = Stem;
    Stem["Chord"] = Value(ChordNode);
  }

  if(StemmedDuration(StartNote["Duration"].AsRatio()))
    StemInformation.Add() = Stem;
}

void AccumulateStemInformationForMultichord(const Value& Multichord,
  Value& StemInformation, Vector Offset)
{
  for(count i = 0; i < Multichord.n(); i++)
    AccumulateStemInformationForChord(Multichord[i], StemInformation, Offset);
}

number BeamDistance()
{
  return 0.25f;
}

number BeamThickness()
{
  return 0.5f;
}

void EngraveStems(Music::ConstNode Island, const Value& Stems, bool Beamed)
{
  number StemWidth = Property(Island, "StemWidth");
  for(count i = 0; i < Stems.n(); i++)
  {
    Value Stem = Stems[i];
    bool StemUp = Stem["StemUp"].AsBoolean();
    number Direction = StemUp ? 1.f : -1.f;
    number StemAlign = -StemWidth / 2.f * Direction;
    Vector Start(+Stem["X"] + StemAlign,
      +Stem["StartY"] + +Stem["JoinOffset"]);
    Vector End(+Stem["X"] + StemAlign,
      +Stem["EndY"]);
    End.y += +Stem["Height"] * Direction;
    Vector Offset = Stem["ChordOffset"].AsVector();

    if(Beamed == Stem["StemHasBeam"].AsBoolean())
    {
      Pointer<Path> p;
      Shapes::AddLine(*p.New(), Start + Offset, End + Offset, StemWidth,
        true, true, true, 0.5f);
      Pointer<Stamp> IslandStamp = StampForIsland(Island);
      IslandStamp->Add()->p = p;
      Vector StemJoin(-StemWidth / 2.f, 0);

      if(not Beamed)
        EngraveFlag(Island, Stem["Chord"], Stem["Duration"],
          End + Offset + StemJoin,
          Stem["StemUp"].AsBoolean() ? mica::Up : mica::Down);
    }
    else if(not Beamed and Stem["StemHasBeam"].AsBoolean())
    {
      /*Add virtual stem line so that it can be used for collision detection in
      spacing.*/
      StampForIsland(Island)->AddArtificialBounds(Box(Start + Offset,
        End + Offset));
    }
  }
}

number FlagExtensionForDuration(Ratio r)
{
  number Flags = number(FlagsGivenDuration(r));
  return Flags > 2 ? (Flags - 2) * (BeamThickness() + BeamDistance()) : 0;
}

count FlagsGivenDuration(Ratio r)
{
  r = UndottedDuration(r);
  count Flags = 0;
  while(r > 0 and r < Ratio(1, 4))
    r *= 2, Flags++;
  return Flags;
}

number StemHeightForPositionAndStemDirection(count StaffPosition, bool StemUp,
  Ratio Duration, bool IsBeamed)
{
  StaffPosition = (StemUp ? 1 : -1) * StaffPosition;
  return not IsBeamed and UndottedDuration(Duration) <= Ratio(1, 8) ? 3.5f :
    StaffPosition <= -8                         ? number(-StaffPosition) / 2.f :
    StaffPosition >= -7 and StaffPosition <= -1 ? 3.5f  :
    StaffPosition == 0                          ? 3.25f :
    StaffPosition == 1                          ? 3.f   :
    StaffPosition == 2                          ? 2.75f : 2.5f;
}

bool StemmedDuration(Ratio r)
{
  return UndottedDuration(r) <= Ratio(1, 2) or
    UndottedDuration(r) >= Ratio(4, 1);
}

#endif

///@}

//Declarations

void AccumulateStemInformationForChord(const Value& Chord,
  Value& StemInformation, Vector Offset);
void AccumulateStemInformationForMultichord(const Value& Multichord,
  Value& StemInformation, Vector Offset);
number BeamDistance();
number BeamThickness();
void EngraveStems(Music::ConstNode Island, const Value& Stems, bool Beamed);
number FlagExtensionForDuration(Ratio r);
count FlagsGivenDuration(Ratio r);
number StemHeightForPositionAndStemDirection(count StaffPosition, bool StemUp,
  Ratio Duration, bool IsBeamed);
bool StemmedDuration(Ratio r);
