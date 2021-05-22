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

///@name General
///@{

///Returns a value referenced off of another.
Pointer<const Value> GetReferencedValue(const Value& x);

///Assigns a pointer to value to a given parent.
void AssignValuePointer(Value& Parent, Pointer<const Value> Reference);

///Assigns a pointer to copy of the value to a given parent.
void AssignValuePointerAsReference(Value& Parent, const Value& ValueToCopy);

///Returns whether the node is a chord token.
bool IsChord(Music::ConstNode x);

///Returns whether the node is a token.
bool IsToken(Music::ConstNode x);

///Returns whether the node is an island.
bool IsIsland(Music::ConstNode x);

///Returns whether the node is a note.
bool IsNote(Music::ConstNode x);

///Returns whether the node is a float.
bool IsFloat(Music::ConstNode x);

///Returns whether the node is an expression float.
bool IsExpression(Music::ConstNode x);

///Returns whether the node is a barline token.
bool IsBarline(Music::ConstNode x);

///Returns whether the node is a clef token.
bool IsClef(Music::ConstNode x);

///Returns whether the node is a key signature token.
bool IsKeySignature(Music::ConstNode x);

///Returns whether the node is a time signature token.
bool IsTimeSignature(Music::ConstNode x);

///Returns whether the node is a pedal marking float.
bool IsPedalMarking(Music::ConstNode x);

///Returns whether the node is an initial pedal marking float.
bool IsInitialPedalMarking(Music::ConstNode x);

///Returns whether the node is a tuplet info node.
bool IsTupletInfo(Music::ConstNode x);

///Returns the island that parents the given token.
Music::ConstNode IslandOfToken(Music::ConstNode x);

///Returns the token that parents the given note.
Music::ConstNode ChordOfNote(Music::ConstNode x);

///Returns the island that parents the given note.
Music::ConstNode IslandOfNote(Music::ConstNode x);

///Returns the notes of the chord.
Array<Music::ConstNode> NotesOfChord(Music::ConstNode x);

///Returns the floats linked to the node.
Array<Music::ConstNode> FloatsOfNode(Music::ConstNode x);

///Returns the floats linked to the node.
Array<Music::Node> FloatsOfNode(Music::Node x);

///Returns the expressions linked to the node.
Array<Music::ConstNode> ExpressionsOfNode(Music::ConstNode x);

///Returns the tokens of the island.
Array<Music::ConstNode> TokensOfIsland(Music::ConstNode x);

///Returns the chords of the island.
Array<Music::ConstNode> ChordsOfIsland(Music::ConstNode x);

///Returns whether a note has an outgoing tie.
bool NoteHasOutgoingTie(Music::ConstNode x);

///Returns whether a note has an incoming tie.
bool NoteHasIncomingTie(Music::ConstNode x);

///Returns whether any in an array of notes have outgoing ties.
bool NotesHaveOutgoingTies(const Array<Music::ConstNode>& Notes);

///Returns whether any in an array of notes have incoming ties.
bool NotesHaveIncomingTies(const Array<Music::ConstNode>& Notes);

///Returns whether a chord has notes with any outgoing ties.
bool ChordHasOutgoingTies(Music::ConstNode x);

///Returns whether a chord has notes with any outgoing ties.
bool ChordHasIncomingTies(Music::ConstNode x);

///Returns whether a note has a partial incoming tie.
bool NoteHasPartialIncomingTie(Music::ConstNode x);

///Returns whether a note has a partial outgoing tie.
bool NoteHasPartialOutgoingTie(Music::ConstNode x);

/**Gets the rhythmic duration of the chord. If the chord is a grace note or is
otherwise non-durational, then zero is returned.*/
Ratio RhythmicDurationOfChord(Music::ConstNode x);

/**Gets the intrinsic (visual) duration of the chord.*/
Ratio IntrinsicDurationOfChord(Music::ConstNode x);

/**Gets the intrinsic (visual) duration of the note.*/
Ratio IntrinsicDurationOfNote(Music::ConstNode x);

///Returns the rhythmic duration of the island taking into account voicing.
Ratio RhythmicDurationOfIsland(Music::ConstNode x);

///Gets the geometry-assigned part ID of the island.
count PartIDOfIsland(Music::ConstNode x);

///Gets the geometry-assigned instant ID of the island.
count InstantIDOfIsland(Music::ConstNode x);

///Unlinks unnecessary instantwise edges.
void UnlinkUnnecessaryInstantwiseEdges(Music& G);

///Gets the active key signature accidental for the current note if it exists.
mica::Concept ActiveKeySignatureAccidentalForNote(Music::ConstNode Note);

///Gets the active measure-wise accidental for the note if it exists.
mica::Concept ActiveMeasureAccidentalForNote(Music::ConstNode Note);

///Gets the actual sounding pitch of the note based on the part state.
mica::Concept ActualPitchOfNote(Music::ConstNode Note);

///Returns whether the island contains a barline token.
bool IslandHasBarline(Music::ConstNode Island);

///Returns whether the island contains a clef token.
bool IslandHasClef(Music::ConstNode Island);

///Returns whether the island contains a key signature token.
bool IslandHasKeySignature(Music::ConstNode Island);

///Returns whether the island contains one or more chords.
bool IslandHasChords(Music::ConstNode Island);

///Returns the stamp for the given island if it has one.
Pointer<Stamp> StampForIsland(Music::ConstNode Island);

///Returns the stamp bounds for the given island if any.
Box StampBoundsForIsland(Music::ConstNode Island);

///Returns whether chord stem direction is up in part state or nil if unknown.
Value ChordStemIsUpInPartState(Music::ConstNode Chord);

///Returns the horizontal position of the island within staff in space units.
number TypesetXOfIsland(Music::ConstNode Island);

///Returns horizontal delta between two islands.
number TypesetDistanceToIsland(Music::ConstNode Left, Music::ConstNode Right);

///Returns the edge for the outgoing tie of a note.
Music::ConstEdge OutgoingTieOfNote(Music::ConstNode Note);

Music::ConstNode NextIslandByPart(Music::ConstNode Island);
Music::ConstNode NextIslandByInstant(Music::ConstNode Island);
Music::ConstNode PreviousIslandByPart(Music::ConstNode Island);
Music::ConstNode PreviousIslandByInstant(Music::ConstNode Island);

///@}

#ifdef BELLE_IMPLEMENTATION

Pointer<const Value> GetReferencedValue(const Value& Parent)
{
  Pointer<const Value::ConstReference> Ref = Parent.ConstObject();
  return Ref ? Ref->GetPointer() : Pointer<const Value>();
}

void AssignValuePointer(Value& Parent, Pointer<const Value> Reference)
{
  Parent = new Value::ConstReference(Reference);
}

void AssignValuePointerAsReference(Value& Parent, const Value& ValueToCopy)
{
  AssignValuePointer(Parent, new Value(ValueToCopy));
}

bool IsChord(Music::ConstNode x)
{
  return IsToken(x) and x->Get(mica::Kind) == mica::Chord and
    IsIsland(IslandOfToken(x));
}

bool IsToken(Music::ConstNode x)
{
  return x and x->Get(mica::Type) == mica::Token;
}

bool IsIsland(Music::ConstNode x)
{
  return x and x->Get(mica::Type) == mica::Island;
}

bool IsNote(Music::ConstNode x)
{
  return x and x->Get(mica::Type) == mica::Note;
}

bool IsFloat(Music::ConstNode x)
{
  return x and x->Get(mica::Type) == mica::Float;
}

bool IsExpression(Music::ConstNode x)
{
  return IsFloat(x) and x->Get(mica::Kind) == mica::Expression;
}

bool IsPedalMarking(Music::ConstNode x)
{
  return IsFloat(x) and x->Get(mica::Kind) == mica::PedalMarking;
}

bool IsInitialPedalMarking(Music::ConstNode x)
{
  return IsPedalMarking(x) and not x->Previous(MusicLabel(mica::Span));
}

bool IsTupletInfo(Music::ConstNode x)
{
  return x and x->Get(mica::Type) == mica::Tuplet;
}

Music::ConstNode IslandOfToken(Music::ConstNode x)
{
  return IsToken(x) ? x->Previous(MusicLabel(mica::Token)) : Music::ConstNode();
}

Music::ConstNode ChordOfNote(Music::ConstNode x)
{
  return IsNote(x) ? x->Previous(MusicLabel(mica::Note)) : Music::ConstNode();
}

Music::ConstNode IslandOfNote(Music::ConstNode x)
{
  return IslandOfToken(ChordOfNote(x));
}

Array<Music::ConstNode> NotesOfChord(Music::ConstNode x)
{
  return IsChord(x) ?
    x->Children(MusicLabel(mica::Note)) : Array<Music::ConstNode>();
}

Array<Music::ConstNode> TokensOfIsland(Music::ConstNode x)
{
  return IsIsland(x) ?
    x->Children(MusicLabel(mica::Token)) : Array<Music::ConstNode>();
}

Array<Music::ConstNode> ChordsOfIsland(Music::ConstNode x)
{
  const Array<Music::ConstNode> Tokens = TokensOfIsland(x);
  Array<Music::ConstNode> Chords;
  for(count i = 0; i < Tokens.n(); i++)
    if(IsChord(Tokens[i]))
      Chords.Add() = Tokens[i];
  return Chords;
}

Array<Music::ConstNode> FloatsOfNode(Music::ConstNode x)
{
  Array<Music::ConstNode> Floats = x ?
    x->Children(MusicLabel(mica::Float)) : Array<Music::ConstNode>();
  return Floats;
}

Array<Music::Node> FloatsOfNode(Music::Node x)
{
  Array<Music::Node> Floats = x ?
    x->Children(MusicLabel(mica::Float)) : Array<Music::Node>();
  return Floats;
}

Array<Music::ConstNode> ExpressionsOfNode(Music::ConstNode x)
{
  Array<Music::ConstNode> Floats = FloatsOfNode(x);
  Array<Music::ConstNode> Expressions;
  for(count i = 0; i < Floats.n(); i++)
    if(IsExpression(Floats[i]))
      Expressions.Add() = Floats[i];
  return Expressions;
}

Ratio IntrinsicDurationOfChord(Music::ConstNode x)
{
  return IsChord(x) ? x->Get(mica::NoteValue).ratio() : Ratio(0);
}

Ratio IntrinsicDurationOfNote(Music::ConstNode x)
{
  return IntrinsicDurationOfChord(ChordOfNote(x));
}

bool NoteHasOutgoingTie(Music::ConstNode x)
{
  return IsNote(x) and x->Next(MusicLabel(mica::Tie));
}

bool NoteHasIncomingTie(Music::ConstNode x)
{
  return IsNote(x) and x->Previous(MusicLabel(mica::Tie));
}

bool NotesHaveOutgoingTies(const Array<Music::ConstNode>& Notes)
{
  for(count i = 0; i < Notes.n(); i++)
    if(NoteHasOutgoingTie(Notes[i]))
      return true;
  return false;
}

bool NotesHaveIncomingTies(const Array<Music::ConstNode>& Notes)
{
  for(count i = 0; i < Notes.n(); i++)
    if(NoteHasIncomingTie(Notes[i]))
      return true;
  return false;
}

bool NoteHasPartialIncomingTie(Music::ConstNode x)
{
  return IsNote(x) and x->Get(mica::PartialTieIncoming) !=
    mica::Undefined;
}

bool NoteHasPartialOutgoingTie(Music::ConstNode x)
{
  return IsNote(x) and x->Get(mica::PartialTieOutgoing) !=
    mica::Undefined;
}

bool ChordHasOutgoingTies(Music::ConstNode x)
{
  return NotesHaveOutgoingTies(NotesOfChord(x));
}

bool ChordHasIncomingTies(Music::ConstNode x)
{
  return NotesHaveIncomingTies(NotesOfChord(x));
}

Ratio RhythmicDurationOfChord(Music::ConstNode x)
{
  return IsChord(x) ? x->Get(mica::NoteValue).ratio() /
    TotalTupletScalar(TupletScalarsOfChord(x)) : Ratio(0);
}

Ratio RhythmicDurationOfIsland(Music::ConstNode x)
{
  Array<Music::ConstNode> Chords = ChordsOfIsland(x);
  Ratio Duration;
  if(Chords.n())
  {
    Duration = x->Label.GetState("PartState", "Voicing", "Duration").AsRatio();
    if(not (Duration > 0))
      Duration = RhythmicDurationOfChord(Chords.a());
  }
  return Duration > 0 ? Duration : 0;
}

count PartIDOfIsland(Music::ConstNode x)
{
  return IsIsland(x) ? x->Label.GetState("PartID").AsCount() : -1;
}

count InstantIDOfIsland(Music::ConstNode x)
{
  return IsIsland(x) ? x->Label.GetState("InstantID").AsCount() : -1;
}

void UnlinkUnnecessaryInstantwiseEdges(Music& G)
{
  Sortable::Array<Music::Node> N = G.Nodes();
  for(count i = 0; i < N.n(); i++)
    if(IsIsland(N[i]) and N[i]->Next(MusicLabel(mica::Instantwise)) and
      ChordsOfIsland(N[i]).n() > 0)
        G.Disconnect(N[i]->Next(MusicLabel(mica::Instantwise), true));
}

mica::Concept ActiveKeySignatureAccidentalForNote(Music::ConstNode Note)
{
  mica::Concept Result;
  if(Music::ConstNode Island = IslandOfNote(Note))
  {
    mica::Concept ActiveKeySignature =
      Island->Label.GetState("PartState", "KeySignature", "Active");
    for(count i = 0; i < mica::length(ActiveKeySignature); i++)
    {
      mica::Concept Alteration = mica::item(ActiveKeySignature, i);
      mica::Concept AlteredAccidental =
        mica::map(Alteration, mica::Accidental);
      mica::Concept AlteredLetter =
        mica::map(Alteration, mica::Letter);
      if(AlteredLetter == mica::map(Note->Get(mica::Value), mica::Letter))
        Result = AlteredAccidental;
    }
  }
  return Result;
}

mica::Concept ActiveMeasureAccidentalForNote(Music::ConstNode Note)
{
  mica::Concept Result;
  if(Music::ConstNode Island = IslandOfNote(Note))
  {
    const Value& Alterations =
      Island->Label.GetState("PartState", "Accidentals", "Altered");
    Value Keys = Alterations.Keys();
    for(count i = 0; i < Keys.n(); i++)
    {
      Value Val = Alterations[Keys[i]];
      if(mica::Concept(Val["DiatonicPitch"]) ==
        mica::map(Note->Get(mica::Value), mica::DiatonicPitch) and
        Val["MeasuresAgo"].AsCount() == 0)
          Result = Val["Accidental"];
    }
  }
  return Result;
}

mica::Concept ActualPitchOfNote(Music::ConstNode Note)
{
  mica::Concept Result;
  if(IsNote(Note))
  {
    mica::Concept KeyAccidental = ActiveKeySignatureAccidentalForNote(Note);
    mica::Concept MeasureAccidental = ActiveMeasureAccidentalForNote(Note);
    mica::Concept PitchAccidental = mica::map(Note->Get(mica::Value),
      mica::Accidental);
    mica::Concept OverrideAccidental = Note->Get(mica::Accidental);
    mica::Concept DiatonicPitch = mica::map(Note->Get(mica::Value),
      mica::DiatonicPitch);

    mica::Concept ActualAccidental = PitchAccidental;
    if(KeyAccidental != mica::Undefined and
        OverrideAccidental == mica::Accidental)
      ActualAccidental = KeyAccidental;
    if(MeasureAccidental != mica::Undefined and
        OverrideAccidental == mica::Accidental)
      ActualAccidental = MeasureAccidental;
    if(OverrideAccidental != mica::Undefined and
      OverrideAccidental != mica::Accidental)
        ActualAccidental = OverrideAccidental;

    Result = mica::map(DiatonicPitch, ActualAccidental);
  }
  return Result;
}

bool IsBarline(Music::ConstNode x)
{
  return IsToken(x) and x->Get(mica::Kind) == mica::Barline;
}

bool IsClef(Music::ConstNode x)
{
  return IsToken(x) and x->Get(mica::Kind) == mica::Clef;
}

bool IsKeySignature(Music::ConstNode x)
{
  return IsToken(x) and x->Get(mica::Kind) == mica::KeySignature;
}

bool IsTimeSignature(Music::ConstNode x)
{
  return IsToken(x) and x->Get(mica::Kind) == mica::TimeSignature;
}

bool IslandHasChords(Music::ConstNode Island)
{
  return ChordsOfIsland(Island).n();
}

bool IslandHasBarline(Music::ConstNode Island)
{
  Array<Music::ConstNode> Tokens = TokensOfIsland(Island);
  bool FoundBarline = false;
  for(Counter i; i.z(Tokens) and not FoundBarline; i++)
    FoundBarline = IsBarline(Tokens[i]);
  return FoundBarline;
}

bool IslandHasClef(Music::ConstNode Island)
{
  Array<Music::ConstNode> Tokens = TokensOfIsland(Island);
  bool FoundClef = false;
  for(Counter i; i.z(Tokens) and not FoundClef; i++)
    FoundClef = IsClef(Tokens[i]);
  return FoundClef;
}

bool IslandHasKeySignature(Music::ConstNode Island)
{
  Array<Music::ConstNode> Tokens = TokensOfIsland(Island);
  bool FoundKeySignature = false;
  for(Counter i; i.z(Tokens) and not FoundKeySignature; i++)
    FoundKeySignature = IsKeySignature(Tokens[i]);
  return FoundKeySignature;
}

Pointer<Stamp> StampForIsland(Music::ConstNode Island)
{
  return IsIsland(Island) ? Pointer<Stamp>(Island->Label.Stamp().Object()) :
    Pointer<Stamp>();
}

Box StampBoundsForIsland(Music::ConstNode Island)
{
  Box R;
  if(Pointer<Stamp> S = StampForIsland(Island))
    R = S->GetAbstractBoundsWithoutSpans();
  return R;
}

Value ChordStemIsUpInPartState(Music::ConstNode Chord)
{
  Music::ConstNode Island = IslandOfToken(Chord);
  Value v;
  if(Island and IsChord(Chord))
    v = mica::Concept(Island->Label.GetState(
      "PartState", "Chord")[Chord]["StemDirection"]) == mica::Up;
  return v;
}

number TypesetXOfIsland(Music::ConstNode Island)
{
  Value v;
  if(IsIsland(Island))
    v = Island->Label.GetState("IslandState", "TypesetX");
  return v.IsNumber() ? v.AsNumber() : number(0.f);
}

Music::ConstEdge OutgoingTieOfNote(Music::ConstNode Note)
{
  Music::ConstEdge e;
  if(IsNote(Note))
    e = Note->Next(MusicLabel(mica::Tie), true);
  return e;
}

Music::ConstNode NextIslandByPart(Music::ConstNode Island)
{
  return IsIsland(Island) ? Island->Next(MusicLabel(mica::Partwise)) :
    Music::ConstNode();
}

Music::ConstNode NextIslandByInstant(Music::ConstNode Island)
{
  return IsIsland(Island) ? Island->Next(MusicLabel(mica::Instantwise)) :
    Music::ConstNode();
}

Music::ConstNode PreviousIslandByPart(Music::ConstNode Island)
{
  return IsIsland(Island) ? Island->Previous(MusicLabel(mica::Partwise)) :
    Music::ConstNode();
}

Music::ConstNode PreviousIslandByInstant(Music::ConstNode Island)
{
  return IsIsland(Island) ? Island->Previous(MusicLabel(mica::Instantwise)) :
    Music::ConstNode();
}

number TypesetDistanceToIsland(Music::ConstNode Left, Music::ConstNode Right)
{
  return TypesetXOfIsland(Right) - TypesetXOfIsland(Left);
}
#endif
