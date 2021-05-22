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
@name Notehead Construction

Functions that implement construction and clustering of noteheads.
@{
*/

#ifdef BELLE_IMPLEMENTATION

///Returns value-array of active accidentals given a set of multichord clusters.
Value AccidentalListFromChordClusters(Music::ConstNode Island,
  const Value& MultichordClusters)
{
  Value v = RawAccidentalListFromChordClusters(Island, MultichordClusters);
  SortValueNotesByPitchHighestToLowest(v);
  return v;
}

///Accumulates a set of multichord clusters onto an island stamp.
Vector AccumulateMultichord(Music::ConstNode Island, Value& MultichordClusters)
{
  Pointer<Stamp> Multichord = EngraveMultichord(Island, MultichordClusters);
  Vector Offset;
  if(not StampForIsland(Island)->IsEmpty())
    Offset = PlaceMultichordNextToExisting(StampForIsland(Island), Multichord);
  StampForIsland(Island)->AccumulateGraphics(*Multichord);
  return Offset;
}

///Adds the accidental for the note to the list if it should be emitted.
void AddAccidentalToListIfNecessary(Music::ConstNode Island,
  const Value& NoteInfo, Value& AccidentalList)
{
  if(ShouldEmitAccidental(Island, NoteInfo["StaffPosition"], NoteInfo["Accidental"]))
    AccidentalList.Add()["Accidental"]  = NoteInfo["Accidental"],
      AccidentalList.z()["StaffPosition"] = NoteInfo["StaffPosition"],
      AccidentalList.z()["Note"] = NoteInfo["Note"];
}

/**Assigns the note columns for a set of clusters for a single chord. For
stem-up and stemless chords, the normal column is 0 and the backnote column
is 1. For stem-down chords, the normal column is 0 and the backnote column
is -1.*/
void AssignNoteColumns(Value& Clusters)
{
  bool StemUp = mica::Concept(Clusters.a().a()["StemDirection"]) == mica::Up;
  bool Stemless = Clusters.a().a()["Duration"].AsRatio() >= 1;
  count FirstColumn = (StemUp or Stemless) ? 0 : -1;
  for(count i = 0; i < Clusters.n(); i++)
    for(count j = 0, n = Clusters[i].n(); j < n; j++)
      Clusters[i][j]["Column"] = n > 1 ?
        FirstColumn + ((j + (not StemUp and n % 2 == 1)) % 2) : 0;
}

/**Constructs a black notehead given the island and note state. The note state
is updated to include the location of the stem.*/
Pointer<Path> BlackNotehead(Music::ConstNode Island, Value& Note)
{
  Pointer<Path> p;
  number ColumnWidth = +Property(Island, "BlackNoteheadWidthPrecise") -
    +Property(Island, "StemWidth");
  Vector StemLocation = Shapes::Music::AddQuarterNote(
    *p.New(),
    Vector(+Note["Column"] * ColumnWidth, +Note["StaffPosition"] / 2.f),
    Property(Island, "BlackNoteheadSize"),
    false,
    mica::Concept(Note["StemDirection"]) == mica::Up ? 1.f : -1.f,
    0,
    Property(Island, "BlackNoteheadAngle"),
    Property(Island, "BlackNoteheadWidth"));
  Note["StemLocation"] = StemLocation;
  return p;
}

///Constructs a notehead given the island and note state.
Pointer<Path> ConstructNotehead(Music::ConstNode Island, Value& Note)
{
  mica::Concept Notehead = mica::Concept(Note["Notehead"]);
  bool StemmedNote = Notehead == mica::BlackNotehead or
    Notehead == mica::HalfNotehead;
  Pointer<Path> p =
    Notehead == mica::BlackNotehead            ?
      BlackNotehead(Island, Note)              :
    Notehead == mica::HalfNotehead             ?
      HalfNotehead(Island, Note)               :
    Notehead == mica::WholeNotehead            ?
      StemlessNotehead(Island, Note, Notehead) :
    Notehead == mica::DoubleWholeNotehead      ?
      StemlessNotehead(Island, Note, Notehead) :
    Notehead == mica::LongaNotehead            ?
      MensuralNotehead(Island, Note, Notehead) :
    Notehead == mica::MaximaNotehead           ?
      MensuralNotehead(Island, Note, Notehead) :
    Unsupported(Vector(+Note["Column"], +Note["StaffPosition"] / 2.f));

  Note["NoteheadWidth"] = p->Bounds().Width() -
    (StemmedNote ? +Property(Island, "StemWidth") : 0.f);
  Note["NoteheadCenter"] = p->Bounds().Center();
  return p;
}

/**Creates an array of clusters. Each cluster contains a staff note list
containing a list of staff notes which are adjacent by unison or second. The
input is a value created by the NoteheadInformationForChord.*/
Value CreateNoteClusters(const Value& Chord)
{
  Value Clusters;
  for(count i = 0; i < Chord.n(); i++)
    (not i or IsClusteredWith(Chord[i], Chord[i - 1]) ?
      Clusters.z().Add() : Clusters.Add().Add()) = Chord[i];
  AssignNoteColumns(Clusters);
  return Clusters;
}

/**Engraves the accidentals to a set of multichord clusters. The result is
engraved to the chord-accumulation stamp instead of the island stamp.*/
void EngraveAccidentals(Music::ConstNode Island, Value& MultichordClusters,
  Stamp& Noteheads)
{
  Value AccidentalList = AccidentalListFromChordClusters(Island,
    MultichordClusters);
  Array<Pointer<const Glyph> > AccidentalPaths =
    AccidentalStackPaths(FontFromIsland(Island), AccidentalList);
  //Modify AccidentalList to include Placement.
  PlacementForAccidentalStack(AccidentalPaths, Noteheads,
    AccidentalList, DefaultOrderForAccidentalStack(AccidentalList.n()));
  PlaceAccidentals(AccidentalList, AccidentalPaths, Noteheads);
}

///Engraves the noteheads to a set of multichord clusters.
void EngraveChordNoteheads(Music::ConstNode Island,
  Value& Clusters, Stamp& Noteheads)
{
  for(count i = 0; i < Clusters.n(); i++)
    for(count j = 0; j < Clusters[i].n(); j++)
      for(count k = 0; k < Clusters[i][j].n(); k++)
        Noteheads.Add()->p = ConstructNotehead(Island, Clusters[i][j][k]),
        Noteheads.z()->Context = Clusters[i][j][k]["Note"].ConstObject();
}

///Accumulates the chord noteheads and accidentals onto the island stamp.
Pointer<Stamp> EngraveMultichord(Music::ConstNode Island,
  Value& MultichordClusters)
{
  Pointer<Stamp> Chord;
  EngraveChordNoteheads(Island, MultichordClusters, *Chord.New());
  EngraveAccidentals(Island, MultichordClusters, *Chord);
  EngraveDots(Island, MultichordClusters, *Chord);
  return Chord;
}

///Returns the active font for the given island.
Pointer<const Font> FontFromIsland(Music::ConstNode Island)
{
  Pointer<Value::ConstReference> H = GlobalHouseStyleFromIsland(Island);
  return H ? Pointer<const Font>(H->Get()["NotationFont"].ConstObject()) :
    Pointer<const Font>();
}

///Returns the global house-style reference cached on the given island.
Pointer<Value::ConstReference> GlobalHouseStyleFromIsland(
  Music::ConstNode Island)
{
  return Pointer<Value::ConstReference>(Island->Label.GetState(
    "HouseStyle", "Global").Object());
}

/**Constructs a half-note notehead given the island and note state. The note
state is updated to include the location of the stem.*/
Pointer<Path> HalfNotehead(Music::ConstNode Island, Value& Note)
{
  Pointer<Path> p;
  number ColumnWidth = +Property(Island, "WhiteNoteheadWidthPrecise") -
    +Property(Island, "StemWidth");
  Vector StemLocation = Shapes::Music::AddHalfNote(
    *p.New(),
    Vector(+Note["Column"] * ColumnWidth, +Note["StaffPosition"] / 2.f),
    Property(Island, "WhiteNoteheadSize"),
    false,
    mica::Concept(Note["StemDirection"]) == mica::Up ? 1.f : -1.f,
    0,
    Property(Island, "WhiteNoteheadAngle"),
    Property(Island, "WhiteNoteheadWidth"));
  Note["StemLocation"] = StemLocation;
  return p;
}

/**Returns whether this note is clustered with another. This occurs
when the notes are either at a distance of a unison or a second.*/
bool IsClusteredWith(const Value& First, const Value& Second)
{
  return Abs(First["StaffPosition"].AsCount() -
    Second["StaffPosition"].AsCount()) <= 1;
}

///Constructs a SMuFL-based mensural notehead such as a longa.
Pointer<Path> MensuralNotehead(Music::ConstNode Island, Value& Note,
  mica::Concept Symbol)
{
  Pointer<Path> p;
  Path g(*SMuFLGlyph(FontFromIsland(Island), Symbol), Affine::Unit());
  number ColumnWidth = g.Bounds().Width() * 0.5f;

  p.New()->Append(g, Affine::Translate(Vector(+Note["Column"] * ColumnWidth -
    g.Bounds().Width() / 2.f,
    +Note["StaffPosition"] / 2.f)));
  return p;
}

///Determines the collision offset to place a multichord from the right.
Vector OffsetToPlaceMultichordOnStamp(Pointer<Stamp> IslandStamp,
  Pointer<Stamp> Multichord)
{
  return Box::OffsetToPlaceOnSide(
    Box::SegmentedHull(IslandStamp->GetGraphicBounds(),
      Box::RightSide),
    Box::SegmentedHull(Multichord->GetGraphicBounds(),
      Box::LeftSide),
    Box::RightSide);
}

///Places a multichord in the given stamp by finding its collision offset.
Vector PlaceMultichordNextToExisting(Pointer<Stamp> IslandStamp,
  Pointer<Stamp> Multichord)
{
  Vector Offset = OffsetToPlaceMultichordOnStamp(IslandStamp, Multichord);
  Vector ExtraGap(0.3f, 0.f);
  return Offset.x > 0 ? Multichord->a = Affine::Translate(Offset + ExtraGap),
    Offset + ExtraGap : Vector();
}

/**Returns the active value of a house-style key at the given island. Any local
value on the island level will override the key's global value.*/
Value Property(Music::ConstNode Island, String Key)
{
  return HouseStyle::GetValue(Island, Key);
}

///Returns a raw (unsorted with duplicates) active accidental value-array.
Value RawAccidentalListFromChordClusters(Music::ConstNode Island,
  const Value& MultichordClusters)
{
  Value v;
  for(count i = 0; i < MultichordClusters.n(); i++)
    for(count j = 0; j < MultichordClusters[i].n(); j++)
      for(count k = 0; k < MultichordClusters[i][j].n(); k++)
        AddAccidentalToListIfNecessary(Island, MultichordClusters[i][j][k], v);
  return v;
}

///Returns whether an accidental at a given staff position should be emitted.
bool ShouldEmitAccidental(Music::ConstNode Island, Value StaffPosition,
  mica::Concept Accidental)
{
  return Island->GetState("PartState", "Chord",
    "AccidentalsToEmit")[StaffPosition][Accidental].AsBoolean();
}

/**Returns a staff-position list from a set of multichord clusters. The stem
direction and dot count are also returned.*/
Value StaffPositionListFromChordClusters(const Value& MultichordClusters)
{
  Value v;
  for(count i = 0; i < MultichordClusters.n(); i++)
    for(count j = 0; j < MultichordClusters[i].n(); j++)
      for(count k = 0; k < MultichordClusters[i][j].n(); k++)
        v.Add()["StaffPosition"] = MultichordClusters[i][j][k]["StaffPosition"],
        v.z()["StemDirection"] = MultichordClusters[i][j][k]["StemDirection"],
        v.z()["Dots"] = MultichordClusters[i][j][k]["Dots"],
        v.z()["Note"] = MultichordClusters[i][j][k]["Note"];
  return v;
}

///Constructs a SMuFL-based stemless notehead such as a whole note.
Pointer<Path> StemlessNotehead(Music::ConstNode Island, Value& Note,
  mica::Concept Symbol)
{
  Pointer<Path> p;
  number ColumnWidth = +Property(Island, "WholeNoteWidth") * 0.85f;
  Path g(*SMuFLGlyph(FontFromIsland(Island), Symbol), Affine::Unit());
  p.New()->Append(g, Affine::Translate(Vector(+Note["Column"] * ColumnWidth,
    +Note["StaffPosition"] / 2.f) - g.Bounds().Center()));
  return p;
}

/**Returns a glyph, currently a question mark, indicating an unsupported symbol.
The glyph is 1x1 and centered around the point given for the position.*/
Pointer<Path> Unsupported(Vector Position)
{
  Pointer<Path> p;
  Shapes::AddQuestionMark(*p.New(), Affine::Translate(Position));
  return p;
}
#endif
///@}

//Declarations
Value AccidentalListFromChordClusters(Music::ConstNode Island,
  const Value& MultichordClusters);
Vector AccumulateMultichord(Music::ConstNode Island, Value& MultichordClusters);
void AddAccidentalToListIfNecessary(Music::ConstNode Island,
  const Value& NoteInfo, Value& List);
void AssignNoteColumns(Value& Clusters);
Pointer<Path> BlackNotehead(Music::ConstNode Island, Value& Note);
Pointer<Path> ConstructNotehead(Music::ConstNode Island, Value& Note);
Value CreateNoteClusters(const Value& Chord);
bool IsClusteredWith(const Value& First, const Value& Second);
void EngraveChordNoteheads(Music::ConstNode Island,
  Value& Clusters, Stamp& Noteheads);
Pointer<Stamp> EngraveMultichord(Music::ConstNode Island,
  Value& MultichordClusters);
void EngraveAccidentals(Music::ConstNode Island, Value& MultichordClusters,
  Stamp& Noteheads);
Pointer<const Font> FontFromIsland(Music::ConstNode Island);
Pointer<Value::ConstReference> GlobalHouseStyleFromIsland(
  Music::ConstNode Island);
Pointer<Path> HalfNotehead(Music::ConstNode Island, Value& Note);
Pointer<Path> MensuralNotehead(Music::ConstNode Island, Value& Note,
  mica::Concept Symbol);
Vector OffsetToPlaceMultichordOnStamp(Pointer<Stamp> IslandStamp,
  Pointer<Stamp> Multichord);
Vector PlaceMultichordNextToExisting(Pointer<Stamp> IslandStamp,
  Pointer<Stamp> Multichord);
Value Property(Music::ConstNode Island, String Name);
Value RawAccidentalListFromChordClusters(Music::ConstNode Island,
  const Value& MultichordClusters);
bool ShouldEmitAccidental(Music::ConstNode Island, Value StaffPosition,
  mica::Concept Accidental);
Value  StaffPositionListFromChordClusters(const Value& MultichordClusters);
Pointer<Path> StemlessNotehead(Music::ConstNode Island, Value& Note,
  mica::Concept Symbol);
Pointer<Path> Unsupported(Vector Position);
