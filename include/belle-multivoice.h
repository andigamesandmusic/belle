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

///Engrave the chord as a box.
void EngraveAsBox(Music::ConstNode IslandNode);

void SortTokensByVoiceStrandID(Array<Music::ConstNode>& Tokens);

///Engraves the chord onto the stamp.
void EngraveMultivoice(Music::ConstNode Island,
  Array<Music::ConstNode> Tokens);

///Gets the notehead information from a series of tokens.
Value NoteheadInformationForTokens(const Array<Music::ConstNode>& Tokens);
Value NoteheadInformationForChord(Music::ConstNode Token, Value MultichordInfo);
Value NoteheadInformationForNote(Music::ConstNode Note, Value MultichordInfo);
void EngraveMultichordBrace(Music::ConstNode Island, number MaxOffset);
mica::Concept PartStateAccidentalForStaffPosition(const Value& PartState,
  count StaffPosition, mica::Concept Accidental);
mica::Concept PitchOfNote(Music::ConstNode x);
mica::Concept DiatonicPitchOfNote(Music::ConstNode x);
mica::Concept AccidentalOfNote(Music::ConstNode x);
count StaffPositionOfNote(Music::ConstNode x);
number SpacePositionOfNote(Music::ConstNode x);
Ratio AssumePositiveAndGetGreatestPowerOfTwoLessThan(Ratio r);
Ratio HighestDottableDuration();
Ratio UndottedDuration(Ratio r);
count DurationDots(Ratio r);
Ratio DotScale(count Dots);
Ratio DottedDuration(Ratio r, count Dots);
bool IsSingleDuration(Ratio r);
mica::Concept StandardNoteheadOfDuration(Ratio r);
mica::Concept NoteheadOfNote(Music::ConstNode x);
mica::Concept ActiveClefOfIsland(Music::ConstNode x);
Value RangeOfChord(Music::ConstNode x);

count StrandIDOfChord(Music::ConstNode x);
Value PartStateOfChord(Music::ConstNode x);
Value PartStateInformationOfChord(Music::ConstNode x);

bool AccidentalLowerThan(mica::Concept x, mica::Concept y);
bool AccidentalEqualTo(mica::Concept x, mica::Concept y);
bool AccidentalHigherThan(mica::Concept x, mica::Concept y);

bool NoteLowerThan(Music::ConstNode x, Music::ConstNode y);
bool NoteEqualTo(Music::ConstNode x, Music::ConstNode y);
bool NoteHigherThan(Music::ConstNode x, Music::ConstNode y);

bool ValueNoteLowerThan(Value x, Value y);
bool ValueNoteEqualTo(Value x, Value y);
bool ValueNoteHigherThan(Value x, Value y);

void SortNotesByPitch(Array<Music::ConstNode>& Notes);
void SortValueNotesByPitchHighestToLowest(Value& Notes);
void SortValueNotesByPitchLowestToHighest(Value& Notes);
mica::Concept StemDirectionOfChord(Music::ConstNode x);

bool ChordsOverlap(const Value& UpperNotes, const Value& LowerNotes);
bool ChordsMayShareNotes(const Value& UpperNotes, const Value& LowerNotes);
bool ChordsOppose(const Value& UpperNotes, const Value& LowerNotes);
bool ChordsHaveAdjacentNotes(const Value& UpperNotes, const Value& LowerNotes);
bool ChordsAreCombinable(const Value& UpperNotes, const Value& LowerNotes);
bool IsChordPair(Music::ConstNode First, Music::ConstNode Second);
Ratio TiedDuration(Music::ConstNode Note);

#ifdef BELLE_IMPLEMENTATION

void EngraveMultivoice(Music::ConstNode Island,
  Array<Music::ConstNode> Tokens)
{
  //C::Out() >> "*******************";
  //C::Out() >> "EngraveMultivoice()";
  SortTokensByVoiceStrandID(Tokens);
  Vector Offset;
  Stamp LedgerLines;
  Value Stems;
  number MaxOffset = 0.f;
  for(count i = 0; i < Tokens.n(); i++)
  {
#if 0
    C::Out() >> "--------";
    C::Out() >> "Chord: " << Tokens[i]->Label;
    C::Out() >> "Part of Tuplet: " << IsNodePartOfTuplet(Tokens[i]);
    C::Out() >> "Tuplet Scalars: " << TupletScalarsOfChord(Tokens[i]);
    C::Out() >> "Total Tuplet Scalar: " << TotalTupletScalar(
      TupletScalarsOfChord(Tokens[i]));
    C::Out() >> "Tuplet Tag: " << TupletTagOfAncestor(Tokens[i]);
    C::Out() >> "Tuplet Strand: " << TupletStrandOfNode(Tokens[i]);
    C::Out() >> "Tuplet Chord Start: " << TupletBeginningsAtChord(Tokens[i]);
    C::Out() >> "Tuplet Chord End:   " <<
      TupletEndingsForBeginningsAtChord(Tokens[i]);
    C::Out() >> "Tuplet Structure Start: " <<
      IsChordBeginningTupletStructure(Tokens[i]);
#endif
    Value nc;
    //C::Out() >> "---------------";
    bool ChordIsRest = IsRest(Tokens[i]);
    Music::ConstNode Chord1, Chord2;
    //ChordIsRest = false;
    Value MultichordInfo;
    if(i + 1 < Tokens.n() and IsChordPair(Tokens[i], Tokens[i + 1]))
    {
      //C::Out() >> "Engraving pair:";
      MultichordInfo["MultichordGrouping"] =
        ChordsMayShareNotes(
          NoteheadInformationForChord(Tokens[i], Value()),
          NoteheadInformationForChord(Tokens[i + 1], Value())) ?
        "Shared" : "Multi";
      nc[0] = CreateNoteClusters(NoteheadInformationForChord(Tokens[i],
        MultichordInfo));
      nc[1] = CreateNoteClusters(NoteheadInformationForChord(Tokens[i + 1],
        MultichordInfo));
      Chord1 = Tokens[i], Chord2 = Tokens[i + 1];
      i++;
    }
    else if(ChordIsRest)
    {
      MultichordInfo["MultichordGrouping"] = "Single";
      //C::Out() >> "Engraving rest:";
      EngraveRest(Island, Tokens[i]);
      Chord1 = Tokens[i];
    }
    else
    {
      //C::Out() >> "Engraving single chord:";
      MultichordInfo["MultichordGrouping"] = "Single";
      nc[0] = CreateNoteClusters(NoteheadInformationForChord(Tokens[i],
        MultichordInfo));
      Chord1 = Tokens[i];
    }
    //C::Out() >> "...............";

    if(not ChordIsRest)
    {
      Offset = AccumulateMultichord(Island, nc);
      MaxOffset = Max(MaxOffset, Offset.x);
      //C::Out() >> JSON::Export(nc);
      LedgerLines.Add()->p = LedgerLinePathForMultichord(Island, nc);
      LedgerLines.z()->a = Affine::Translate(Offset);
      AccumulateStemInformationForMultichord(nc, Stems, Offset);

      if(Chord1)
        EngraveArticulations(Chord1, Offset);
      if(Chord2)
        EngraveArticulations(Chord2, Offset);
    }
  }
  StampForIsland(Island)->AccumulateGraphics(LedgerLines);
  EngraveStems(Island, Stems, false);
  EngraveMultichordBrace(Island, MaxOffset);
}

void EngraveMultichordBrace(Music::ConstNode Island, number MaxOffset)
{
  Box Bounds = StampForIsland(Island)->Bounds();
  number BraceHeight = 0.75f;
  number Thickness = 0.1f;
  if(MaxOffset > 3.f)
  {
    Pointer<Path> p = new Path;
    Box R;
    if(Bounds.Top() > -Bounds.Bottom())
    {
      R = Box(Bounds.TopLeft(),
        Bounds.TopRight() + Vector(0, BraceHeight));
      Shapes::AddLine(*p, R.BottomLeft(), R.TopLeft(), Thickness,
        true, true, true, 0.5f);
      Shapes::AddLine(*p, R.TopLeft(), R.TopRight(), Thickness);
      Shapes::AddLine(*p, R.BottomRight(), R.TopRight(), Thickness,
        true, true, true, 0.5f);
    }
    else
    {
      R = Box(Bounds.BottomLeft() - Vector(0, BraceHeight),
        Bounds.BottomRight());
      Shapes::AddLine(*p, R.TopLeft(), R.BottomLeft(), Thickness,
        true, true, true, 0.5f);
      Shapes::AddLine(*p, R.BottomLeft(), R.BottomRight(), Thickness);
      Shapes::AddLine(*p, R.TopRight(), R.BottomRight(), Thickness,
        true, true, true, 0.5f);
    }
    StampForIsland(Island)->Add()->p = p;
  }
}

///Engrave the chord as a box.
void EngraveAsBox(Music::ConstNode IslandNode)
{
  //Get the island stamp.
  Pointer<Stamp> IslandStamp = IslandNode->Label.Stamp().Object();

  Pointer<Path> TemporaryObject = new Path;
  Shapes::AddBox(*TemporaryObject, Box(Vector(-1.0f, -1.0f),
    Vector(1.0f, 1.0f)));
  IslandStamp->Add()->p = TemporaryObject;
}

Value PartStateOfChord(Music::ConstNode x)
{
  Music::ConstNode Island = IslandOfToken(x);
  return IsIsland(Island) ? Island->Label.GetState("PartState") : Value();
}

Value PartStateInformationOfChord(Music::ConstNode x)
{
  Music::ConstNode Island = IslandOfToken(x);
  return IsIsland(Island) ? Island->Label.GetState("PartState", "Chord")[x] :
    Value();
}

count StrandIDOfChord(Music::ConstNode x)
{
  return PartStateInformationOfChord(x)["Voicing"]["StrandID"].AsCount();
}

void SortTokensByVoiceStrandID(Array<Music::ConstNode>& Tokens)
{
  for(count i = 0; i < Tokens.n() - 1; i++)
    for(count j = i + 1; j < Tokens.n(); j++)
      if(StrandIDOfChord(Tokens[i]) > StrandIDOfChord(Tokens[j]))
        Tokens[i].Swap(Tokens[j]);
}

bool ChordsOppose(const Value& UpperNotes, const Value& LowerNotes)
{
  return mica::Concept(UpperNotes.a()["StemDirection"]) == mica::Up and
         mica::Concept(LowerNotes.a()["StemDirection"]) == mica::Down;
}

bool ChordsHaveAdjacentNotes(const Value& UpperNotes, const Value& LowerNotes)
{
  bool HasAdjacentNotes = false;
  for(count i = 0; not HasAdjacentNotes and i < UpperNotes.n(); i++)
    for(count j = 0; not HasAdjacentNotes and j < LowerNotes.n(); j++)
      if(Abs(UpperNotes[i]["StaffPosition"].AsCount() -
        LowerNotes[j]["StaffPosition"].AsCount()) == 1)
          HasAdjacentNotes = true;
  return HasAdjacentNotes;
}

bool ChordsOverlap(const Value& UpperNotes, const Value& LowerNotes)
{
  return UpperNotes[0]["StaffPosition"].AsCount() <=
    LowerNotes[LowerNotes.n() - 1]["StaffPosition"].AsCount();
}

bool ChordsMayShareNotes(const Value& UpperNotes, const Value& LowerNotes)
{
  Value Upper = UpperNotes[0], Lower = LowerNotes[0];
  return UpperNotes.n() == 1 and LowerNotes.n() == 1          and
    Upper["StaffPosition"] == Lower["StaffPosition"]          and
    Upper["DiatonicPitch"] == Lower["DiatonicPitch"]          and
    Upper["Accidental"]    == Lower["Accidental"]             and
    Upper["Dots"]          == Lower["Dots"]                   and
    Upper["Duration"].AsRatio() < Ratio(1, 2)                 and
    Lower["Duration"].AsRatio() < Ratio(1, 2)                 and
    not (UndottedDuration(Upper["Duration"]) == Ratio(1, 4)
     and UndottedDuration(Lower["Duration"]) == Ratio(1, 2))  and
    not (UndottedDuration(Upper["Duration"]) == Ratio(1, 2)
     and UndottedDuration(Lower["Duration"]) == Ratio(1, 4));
}

bool ChordsAreCombinable(const Value& UpperNotes, const Value& LowerNotes)
{
  return ChordsOppose(UpperNotes, LowerNotes)            and
     not ChordsHaveAdjacentNotes(UpperNotes, LowerNotes) and
    (not ChordsOverlap(UpperNotes, LowerNotes) or
      ChordsMayShareNotes(UpperNotes, LowerNotes));
}

bool IsChordPair(Music::ConstNode First, Music::ConstNode Second)
{
  Value UpperNotes = NoteheadInformationForChord(First, Value());
  Value LowerNotes = NoteheadInformationForChord(Second, Value());
  return UpperNotes.n() and LowerNotes.n() and not IsRest(First) and
    not IsRest(Second) ? ChordsAreCombinable(UpperNotes, LowerNotes) : false;
}

mica::Concept PartStateAccidentalForStaffPosition(const Value& PartState,
  count StaffPosition, mica::Concept Accidental)
{
  const Value& Altered = PartState["Accidentals"]["Altered"];
  mica::Concept Result;
  for(count i = Altered.n() - 1; i >= 0 and mica::undefined(Result); i--)
    if(Altered[i]["StaffPosition"].AsCount() == StaffPosition and
      (Accidental == mica::Undefined || Accidental == (mica::Concept)Altered[i]["Accidental"]) and
      Altered[i]["MeasuresAgo"].AsCount() == 0)
        Result = Altered[i]["Accidental"];
  return Result;
}

Value NoteheadInformationForNote(Music::ConstNode Note, Value MultichordInfo)
{
  Value NoteInfo = MultichordInfo;
  NoteInfo["Accidental"]    = PartStateAccidentalForStaffPosition(
    PartStateOfChord(ChordOfNote(Note)), StaffPositionOfNote(Note),
    AccidentalOfNote(Note));
  NoteInfo["Chord"]         = ChordOfNote(Note);
  NoteInfo["DiatonicPitch"] = DiatonicPitchOfNote(Note);
  NoteInfo["Dots"]          = DurationDots(IntrinsicDurationOfNote(Note));
  NoteInfo["Duration"]      = IntrinsicDurationOfNote(Note);
  NoteInfo["Note"]          = Note;
  NoteInfo["Notehead"]      = NoteheadOfNote(Note);
  NoteInfo["Range"]         = RangeOfChord(ChordOfNote(Note));
  NoteInfo["RhythmicDuration"] = RhythmicDurationOfChord(ChordOfNote(Note));
  NoteInfo["StaffPosition"] = StaffPositionOfNote(Note);
  NoteInfo["StemHasBeam"]   = PartStateInformationOfChord(
    ChordOfNote(Note))["StemHasBeam"];
  NoteInfo["StemDirection"] = StemDirectionOfChord(ChordOfNote(Note));
  NoteInfo["StrandID"]      = StrandIDOfChord(ChordOfNote(Note));

  Array<Ratio> Tuplets = TupletScalarsOfChord(ChordOfNote(Note));
  for(count i = 0; i < Tuplets.n(); i++)
    NoteInfo["Tuplets"][i] = Tuplets[i];
  NoteInfo["TupletScalar"] = TotalTupletScalar(Tuplets);
  return NoteInfo;
}

Value NoteheadInformationForChord(Music::ConstNode Token, Value MultichordInfo)
{
  Value Info;
  Array<Music::ConstNode> Notes = NotesOfChord(Token);
  SortNotesByPitch(Notes);
  for(count i = 0; i < Notes.n(); i++)
    Info.Add() = NoteheadInformationForNote(Notes[i], MultichordInfo);
  return Info;
}

Value NoteheadInformationForTokens(const Array<Music::ConstNode>& Tokens)
{
  Value Info;
  for(count i = 0; i < Tokens.n(); i++)
    Info.Add() = NoteheadInformationForChord(Tokens[i], Value());
  return Info;
}

mica::Concept PitchOfNote(Music::ConstNode x)
{
  return IsNote(x) ? x->Get(mica::Value) : mica::Concept();
}

mica::Concept DiatonicPitchOfNote(Music::ConstNode x)
{
  return mica::map(PitchOfNote(x), mica::DiatonicPitch);
}

mica::Concept AccidentalOfNote(Music::ConstNode x)
{
  return mica::map(PitchOfNote(x), mica::Accidental);
}

count StaffPositionOfNote(Music::ConstNode x)
{
  count StaffPosition = 0;
  if(IsNote(x))
  {
    mica::Concept Clef = ActiveClefOfIsland(IslandOfToken(ChordOfNote(x)));
    mica::Concept DiatonicPitch = DiatonicPitchOfNote(x);
    mica::Concept FallbackStaffPosition = x->Get(mica::StaffPosition);
    mica::Concept PitchedStaffPosition = map(DiatonicPitch, Clef);
    if(mica::integer(PitchedStaffPosition))
      StaffPosition = count(mica::numerator(PitchedStaffPosition));
    else if(mica::integer(FallbackStaffPosition))
      StaffPosition = count(mica::numerator(FallbackStaffPosition));
  }
  return StaffPosition;
}

number SpacePositionOfNote(Music::ConstNode x)
{
  return number(StaffPositionOfNote(x)) / 2.f;
}

Ratio AssumePositiveAndGetGreatestPowerOfTwoLessThan(Ratio r)
{
  Ratio Value = 1;
  while(Value < r) Value *= 2;
  while(Value > r) Value /= 2;
  return Value;
}

Ratio HighestDottableDuration()
{
  /*Longas and maximas can not be dotted, since their actual duration is
  determined by the mode of the music, which could be in two or three.*/
  return Ratio(4, 1);
}

Ratio UndottedDuration(Ratio r)
{
  return
    r > HighestDottableDuration() ? r :
    r > 0 ? AssumePositiveAndGetGreatestPowerOfTwoLessThan(r) : Ratio(0);
}

count DurationDots(Ratio r)
{
  const count MaxDotsToConsider = 4;

  count Dots = 0;
  Ratio Base = UndottedDuration(r), DotBase(1, 2);
  if(Base != r and Base > 0)
    for(count i = 1; not Dots and i <= MaxDotsToConsider; i++, DotBase /= 2)
      if(Base * (Ratio(2) - DotBase) == r)
        Dots = i;
  if(r > HighestDottableDuration())
    Dots = 0;
  return Dots;
}

Ratio DotScale(count Dots)
{
  count n = 2, d = 1;
  for(count j = 0; j < Min(Dots, count(4)); j++)
    n *= 2, d *= 2;
  return Ratio(n - 1, d);
}

Ratio DottedDuration(Ratio r, count Dots)
{
  return UndottedDuration(r) * DotScale(Dots);
}

bool IsSingleDuration(Ratio r)
{
  return r == DottedDuration(UndottedDuration(r), DurationDots(r));
}

mica::Concept StandardNoteheadOfDuration(Ratio r)
{
  return r < Ratio( 1, 2) ? mica::BlackNotehead       :
         r < Ratio( 1, 1) ? mica::HalfNotehead        :
         r < Ratio( 2, 1) ? mica::WholeNotehead       :
         r < Ratio( 4, 1) ? mica::DoubleWholeNotehead :
         r < Ratio( 8, 1) ? mica::LongaNotehead       :
         r < Ratio(32, 1) ? mica::MaximaNotehead      : mica::Undefined;
}

mica::Concept NoteheadOfNote(Music::ConstNode x)
{
  return IsNote(x) ? StandardNoteheadOfDuration(IntrinsicDurationOfNote(x)) :
    mica::Concept();
}

mica::Concept ActiveClefOfIsland(Music::ConstNode x)
{
  return IsIsland(x) ? x->Label.GetState("PartState", "Clef", "Active") :
    mica::Concept();
}

Value RangeOfChord(Music::ConstNode x)
{
  Value v;
  Array<Music::ConstNode> Notes = NotesOfChord(x);
  for(count i = 0; i < Notes.n(); i++)
  {
    count p = StaffPositionOfNote(Notes[i]);
    v["Lowest"] = v["Lowest"].IsNil() ? p : Min(v["Lowest"].AsCount(), p);
    v["Highest"] = v["Highest"].IsNil() ? p : Max(v["Highest"].AsCount(), p);
  }
  v.Prune();
  return v;
}

bool AccidentalLowerThan(mica::Concept x, mica::Concept y)
{
  mica::Concept xc = mica::index(mica::Accidentals, x);
  mica::Concept yc = mica::index(mica::Accidentals, y);
  return (mica::integer(xc) and mica::integer(yc)) ?
    numerator(xc) < numerator(yc) : xc < yc;
}

bool AccidentalEqualTo(mica::Concept x, mica::Concept y)
{
  return x == y;
}

bool AccidentalHigherThan(mica::Concept x, mica::Concept y)
{
  return not AccidentalEqualTo(x, y) and not AccidentalLowerThan(x, y);
}

bool ValueNoteLowerThan(Value x, Value y)
{
  count xp = x["StaffPosition"].AsCount(), yp = y["StaffPosition"].AsCount();
  return xp == yp ?
    AccidentalLowerThan(x["Accidental"], y["Accidental"]) : xp < yp;
}

bool NoteLowerThan(Music::ConstNode x, Music::ConstNode y)
{
  count xp = StaffPositionOfNote(x), yp = StaffPositionOfNote(y);
  return xp == yp ?
    AccidentalLowerThan(AccidentalOfNote(x), AccidentalOfNote(y)) : xp < yp;
}

bool ValueNoteEqualTo(Value x, Value y)
{
  return x == y;
}

bool NoteEqualTo(Music::ConstNode x, Music::ConstNode y)
{
  return PitchOfNote(x) == PitchOfNote(y);
}

bool ValueNoteHigherThan(Value x, Value y)
{
  return not ValueNoteEqualTo(x, y) and not ValueNoteLowerThan(x, y);
}

bool NoteHigherThan(Music::ConstNode x, Music::ConstNode y)
{
  return not NoteEqualTo(x, y) and not NoteLowerThan(x, y);
}

void SortNotesByPitch(Array<Music::ConstNode>& Notes)
{
  for(count i = 0; i < Notes.n() - 1; i++)
    for(count j = i + 1; j < Notes.n(); j++)
      if(NoteHigherThan(Notes[i], Notes[j]))
        Notes[i].Swap(Notes[j]);
}

void SortValueNotesByPitchHighestToLowest(Value& Notes)
{
  for(count i = 0; i < Notes.n() - 1; i++)
    for(count j = i + 1; j < Notes.n(); j++)
      if(ValueNoteLowerThan(Notes[i], Notes[j]))
        Swap(Notes[i], Notes[j]);
      else if(ValueNoteEqualTo(Notes[i], Notes[j]))
        Swap(Notes[j], Notes.z()), Notes.n(Notes.n() - 1);
}

void SortValueNotesByPitchLowestToHighest(Value& Notes)
{
  for(count i = 0; i < Notes.n() - 1; i++)
    for(count j = i + 1; j < Notes.n(); j++)
      if(ValueNoteHigherThan(Notes[i], Notes[j]))
        Swap(Notes[i], Notes[j]);
}

mica::Concept StemDirectionOfChord(Music::ConstNode x)
{
  Music::ConstNode Island = IslandOfToken(x);
  return IsIsland(Island) ? Island->Label.GetState(
    "PartState", "Chord")[x]["StemDirection"] : mica::Concept();
}

Ratio TiedDuration(Music::ConstNode Note)
{
  Ratio Duration;
  if(IsNote(Note))
  {
    Duration = 0;
    if(not Note->Previous(Music::Label(mica::Tie)))
    {
      Array<Music::ConstNode> TiedNotes = Note->Series(Music::Label(mica::Tie));
      for(count t = 0; t < TiedNotes.n(); t++)
        Duration += RhythmicDurationOfChord(
          TiedNotes[t]->Previous(Music::Label(mica::Note)));
    }
  }
  return Duration;
}
#endif
