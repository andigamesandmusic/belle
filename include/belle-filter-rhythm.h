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
@name Rhytmic Manipulation

Functions that allow for the dotting and undotting of passages.
@{
*/
void AssumeDottify(Music::Node Chord1, Music::Node Chord2);
void AssumeUndottify(Music::Node DottedChord, Music::Node MatchChord);
void AssumeTupletize(Pointer<Music> M, Music::Node BeginningChord,
  Music::Node EndChord, Ratio Length, String Tag);
bool BeamGroupTest(Music::Node Chord1, Music::Node Chord2);
void BackwardsUndot(Tree<count, Pointer<List<Music::Node> > >& DottedListTable,
  Music::Node Beginning);
void DottedRhythmicManipulation(Music& M, Ratio Initial, Ratio Base,
  bool DotType, bool Staff2, Music::Node Beginning, Music::Node End);
bool IsCorrespondingRhythm(Music::Node currentChord, Music::Node DottedChord);
bool IsChordTuplet(Music::ConstNode ChordToken);
bool IsValidDottedRhythm(Music::Node Chord, Ratio Initial, Ratio Base);
bool IsValidRhythmForChange(Ratio noteValue, Ratio Initial, Ratio Base);
void UndotRhythmicManipulation(Music& M, Ratio Initial, Ratio Base,
  bool Staff2, Music::Node Beginning, Music::Node End);

#ifdef BELLE_IMPLEMENTATION

///Dot comes first. Assumes that Chord1 and Chord2 are valid chords.
void AssumeDottify(Music::Node Chord1, Music::Node Chord2)
{
  Ratio Chord1Value = Ratio(Chord1->Get(mica::NoteValue));
  Ratio Chord2Value = Ratio(Chord2->Get(mica::NoteValue));

  Chord1->Set(mica::NoteValue) = mica::Concept(Chord1Value / 2 * 3);
  Chord2->Set(mica::NoteValue) = mica::Concept(Chord2Value / 2);
}

///Undotting the dotted pair given by DottedChord and MatchChord.
void AssumeUndottify(Music::Node DottedChord, Music::Node MatchChord)
{
  Ratio UndotValue = UndottedDuration(Ratio(DottedChord->Get(mica::NoteValue)));
  Ratio MatchChordValue = Ratio(MatchChord->Get(mica::NoteValue));

  DottedChord->Set(mica::NoteValue) = mica::Concept(UndotValue);
  MatchChord->Set(mica::NoteValue) = mica::Concept(MatchChordValue * 2);
}

/**Assume that the beginning and ending chord are in the same voice. If the
assumption is satisfied, then make all the chords between the beginning
chord and last chord, inclusive, into a tuplet with the specified note value
and tag.*/
void AssumeTupletize(Pointer<Music> M, Music::Node BeginningChord,
  Music::Node EndChord, Ratio Length, String Tag)
{
  Music::Node x = BeginningChord;
  for(; x and x != EndChord; x = x->Next(MusicLabel(mica::Voice)))
  {
    Music::Node NextChord = x->Next(MusicLabel(mica::Voice));
    if(not NextChord) continue;
    M->Connect(x, NextChord)->Set(mica::Type) = mica::Tuplet;
    Music::Edge TupletEdge = x->Next(MusicLabel(mica::Tuplet), true);
    TupletEdge->Set("Tag") = Tag;
  }

  Music::Node TupletTag = M->Add();
  TupletTag->Set(mica::Type) = mica::Tuplet;
  TupletTag->Set("Tag") = Tag;
  TupletTag->Set(mica::Value) = mica::Concept(Length);
  M->Connect(TupletTag, BeginningChord)->Set(mica::Type) = mica::Tuplet;
  Music::Edge TupletEdge = TupletTag->Next(MusicLabel(mica::Tuplet), true);
  TupletEdge->Set("Tag") = Tag;
}

/**The beam group test passes if either 1) both Chord 1 and Chord 2 are beamed
and they are in the same beam group, or 2) neither Chord is beamed.*/
bool BeamGroupTest(Music::Node Chord1, Music::Node Chord2)
{
  bool Chord1Beamed = IsBeamed(Chord1);
  bool Chord2Beamed = IsBeamed(Chord2);
  bool SameBeamGroup = Chord1->Series(MusicLabel(mica::Beam)).Contains(Chord2);

  return (Chord1Beamed and Chord2Beamed and SameBeamGroup)
    or (!Chord1Beamed and !Chord2Beamed);
}

/**Undotting all remaining rhythms, going backwards. Every iteration starts
at the leftover dotted rhythms on the stack. These rhythms could not find
corresponding rhythms going forward so their coresponding rhythms must exist
going backwards.*/
void BackwardsUndot(Tree<count, Pointer<List<Music::Node> > >& DottedListTable,
  Music::Node Beginning)
{
  Pointer<List<Music::Node> > DotList;
  Music::Node PreviousBeginning;

  if (Beginning) PreviousBeginning =
    Beginning->Previous(MusicLabel(mica::Partwise));
  else PreviousBeginning = Music::Node();

  for(count Measure = DottedListTable.n() - 1; Measure >= 0; Measure--)
  {
    DotList = DottedListTable.Get(Measure);
    while(DotList->n() > 0)
    {
      Music::Node Dot = DotList->Pop();
      Music::Node Island = Dot->Previous(MusicLabel(mica::Token));
      Ratio OldDot = Ratio(Dot->Get(mica::NoteValue));

      while(Island and (Ratio(Dot->Get(mica::NoteValue)) != OldDot / 3 * 2)
        and Island != PreviousBeginning)
      {
        Music::Node CurrentToken = Island->Next(MusicLabel(mica::Token));
        if(CurrentToken and CurrentToken->Get(mica::Kind) == mica::Barline)
          break;
        if(IsChord(CurrentToken) and !IsRest(CurrentToken))
          if(IsCorrespondingRhythm(CurrentToken, Dot)
             and BeamGroupTest(CurrentToken, Dot))
          AssumeUndottify(Dot, CurrentToken);

        Island = Island->Previous(MusicLabel(mica::Partwise));
      }
    }
  }
}

/**Dot the passage. If DotType is true, then have the dot come first.
If false, have the dot come second. If Staff2 is true, then dot the second
staff. If false, dot the first staff. Beginning and end refer to the
first and last islands that the changes will apply to.*/
void DottedRhythmicManipulation(Music& M, Ratio Initial, Ratio Base,
  bool DotType, bool Staff2, Music::Node Beginning, Music::Node End)
{
  Music::Node Island;
  Music::Node Root;
  Music::Node x = Music::Node();
  Music::Node NextEnd;

  if(!IsValidSelectedPassage(M, Beginning, End)) return;
  InitializeSelectedPassage(M, Beginning, End, Root, NextEnd);

  if (Staff2) Island = FindFirstInstantConnection(Root);
  else Island = Root;

  while(Island and Island != End and Island != NextEnd)
  {
    Music::Node CurrentToken = Island->Next(MusicLabel(mica::Token));
    if(IsChord(CurrentToken) and !IsRest(CurrentToken) and
      !IsChordTuplet(CurrentToken))
    {
      Ratio NoteValue1 = RhythmicDurationOfChord(CurrentToken);
      Music::Node NextChord;
      if(IsValidRhythmForChange(NoteValue1, Initial, Base)
        and (NextChord = NextChordExist(Island)))
      {
        Ratio NoteValue2 = RhythmicDurationOfChord(NextChord);
        if(!IsChordTuplet(NextChord)
          and (IsValidRhythmForChange(NoteValue2, Initial, Base)
          and (NoteValue2 == NoteValue1))
          and BeamGroupTest(CurrentToken, NextChord))
        {
          if(DotType) AssumeDottify(CurrentToken, NextChord);
          if(!DotType) AssumeDottify(NextChord, CurrentToken);
          Music::Node NextIsland = NextChord->Previous(MusicLabel(mica::Token));
          Island = (NextIsland)->Next(MusicLabel(mica::Partwise));
          continue;
        }
      }
    }

    Island = Island->Next(MusicLabel(mica::Partwise));
  }
}

/**Checks to see if the current chord and the dotted chord are a dotted pair.
Dotted chord must be a dotted rhythm.*/
bool IsCorrespondingRhythm(Music::Node CurrentChord, Music::Node DottedChord)
{
  Ratio CurrentChordValue = Ratio(CurrentChord->Get(mica::NoteValue));
  Ratio DottedChordValue = Ratio(DottedChord->Get(mica::NoteValue));
  return CurrentChordValue * 3 == DottedChordValue;
}

///Checks to see if the ChordToken is part of a tuplet.
bool IsChordTuplet(Music::ConstNode ChordToken)
{
  if(not IsChord(ChordToken)) return false;
  return  IntrinsicDurationOfChord(ChordToken) !=
    RhythmicDurationOfChord(ChordToken);
}

/**Checks to see if the chord is a valid dotted rhythm: 1) is a dotted rhythm
(not double dotted), 2) the undotted version of the rhythm is between the
initial and base values.*/
bool IsValidDottedRhythm(Music::Node Chord, Ratio Initial, Ratio Base)
{
  Ratio DotDuration = Ratio(Chord->Get(mica::NoteValue));
  Ratio Undot = UndottedDuration(DotDuration);
  return ((numerator(Chord->Get(mica::NoteValue)) % 3 == 0)
    and (Undot <= Initial) and (Undot > Base));
}

/**Checks to see if the rhythm is valid for manipulation, namely if it is
divisible by 2 and is between initial and base.*/
bool IsValidRhythmForChange(Ratio Notevalue, Ratio Initial, Ratio Base)
{
  return (Notevalue == UndottedDuration(Notevalue))
    and (Notevalue <= Initial) and (Notevalue > Base);
}

///Removes all dots from the given passage. (Does not remove double dots)
void UndotRhythmicManipulation(Music& M, Ratio Initial, Ratio Base, bool Staff2,
  Music::Node Beginning, Music::Node End)
{
  Music::Node x = Music::Node();
  Music::Node Root;
  Music::Node NextEnd;
  count Counter = 0;

  if(!IsValidSelectedPassage(M, Beginning, End)) return;
  InitializeSelectedPassage(M, Beginning, End, Root, NextEnd);

  Tree<count, Pointer<List<Music::Node> > > DottedListTable;

  DottedListTable.Set(Counter, new List<Music::Node>());
  Pointer<List<Music::Node> > DotList = DottedListTable.Get(Counter);
  Counter++;

  if (Staff2) x = FindFirstInstantConnection(Root);
  else x = Root;

  //First iteration: undotting all the rhythms going forward.
  for(; x and x != NextEnd; x = x->Next(MusicLabel(mica::Partwise)))
  {
    Music::Node CurrentToken = x->Next(MusicLabel(mica::Token));

    if(CurrentToken and CurrentToken->Get(mica::Kind) == mica::Barline)
    {
      DottedListTable.Set(Counter, new List<Music::Node>());
      DotList = DottedListTable.Get(Counter);
      Counter++;
    }

    //Undot the appropriate rhythms or push dotted rhythms onto list.
    if(IsChord(CurrentToken) and !IsRest(CurrentToken) and
      !IsChordTuplet(CurrentToken))
    {
      if(DotList->n()!= 0)
        if(IsCorrespondingRhythm(CurrentToken, DotList->z())
            and (BeamGroupTest(DotList->z(), CurrentToken)))
        AssumeUndottify(DotList->Pop(), CurrentToken);

      if(IsValidDottedRhythm(CurrentToken, Initial, Base))
        DotList->Push(CurrentToken);
    }
  }

  //Finish undotting the passage.
  BackwardsUndot(DottedListTable, Beginning);
}
#endif
///@}
