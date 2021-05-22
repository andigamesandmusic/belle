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
@name Time Signature Manipulation

Functions that help the Time Signature manipulation filter.
@{
*/
void ChangeTimeSignature(Pointer<Music> M, Ratio TimeSignatureChange,
  const List<Ratio> Divisions);
void CombineTies(Pointer<Music> M, const List<Ratio>& Divisions,
  Ratio NewNoteValue, Ratio NewBeats,
  List<List<Music::Node> >& SyncopatedSections,
  List<Music::Node> TimeSignatureRange);
List<Ratio> CommonDivisions(Music::Node TimeSignature);
Music::Node ExceptionDuration(Pointer<Music> M, List<Music::Node> TiedNotes,
  Ratio& BeatsTraversed, const List<Ratio>& Divisions, Ratio& SumOfDivisions,
  Ratio NewNoteValue, count& Counter, Ratio& SyncopationBeginning,
  List<List<Music::Node> >& SyncopatedSections);
Music::Node ExceptionOnBeat(Pointer<Music> M, List<Music::Node>& TiedNotes,
  Ratio& BeatsTraversed, const List<Ratio>& Divisions, Ratio& SumOfDivisions,
  Ratio NewNoteValue, count& Counter);
Music::Node ExceptionSyncopation(Pointer<Music> M, List<Music::Node>& TiedNotes,
  Ratio& BeatsTraversed, const List<Ratio>& Divisions, Ratio& SumOfDivisions,
  Ratio NewNoteValue, count& Counter, Ratio& SyncopationBeginning,
  List<List<Music::Node> >& SyncopatedSections);
Ratio GetBeatsTraversed(const Matrix<Ratio>& RhythmMatrix,
  const Tree<Music::ConstNode, VectorInt>& NodeToIndexLookup,
  Ratio CurrentBarOnset, Music::Node Chord);
void GetTimeSignatureData(Music::ConstNode CurrentTimeSignature,
  Ratio& CurrentNoteValue, Ratio& CurrentBeats);
bool IsValidLongRhythm(Ratio BeatsTraversed, const List<Ratio>& Divisions,
  Ratio NoteValue);
List<Music::Node> IsValidMergeRhythm(count& Counter,
  const List<Music::Node>& TiedSequence);
bool IsValidSyncopatedRhythm(Ratio Length, Ratio& SyncopationBeginning,
  Ratio NoteValue);
bool IsValidTimeSignatureManipulation(Music::Node TimeSignature,
  Ratio TimeSignatureChange, const List<Ratio>& Divisions);
Music::Node MergeNotes(Pointer<Music> M, List<Music::Node> TiedSequence);
Music::Node MergeTieSequence(Pointer<Music> M,
  List<Music::Node>& TiedNotes, Ratio& BeatsTraversed, Ratio& SumOfDivisions,
  Ratio NewNoteValue, const List<Ratio>& Divisions, count& Counter,
  Ratio& SyncopationBeginning, List<List<Music::Node> >& SyncopatedSections);
void NoteValueChange(Pointer<Music> M, const List<Ratio>& Divisions,
  Ratio NewNoteValue, Ratio NewBeats,
  List<Music::Node> TimeSignatureRange);
bool OnOffBeat(Ratio BeatsTraversed, const Ratio& SumOfDivisions,
  const List<Ratio>& Divisions, Ratio NewNoteValue);
void RemoveIsland(Pointer<Music> M, Music::ConstNode Island);
Music::Node SplitChord(Pointer<Music> M, Music::Node ChordToken,
  Ratio BeatsTraversed, Ratio Beat);
void SplitIntoValidChordRatios(Ratio Value, Array<Ratio>& RatioArray);
void SplitRemainingChord(Pointer<Music> M, Ratio Value,
  Music::Node CurrentToken, Music::Node RemainingChord);
void TieNotesTogether(Music& M, const Array<Music::Node>& Notes1,
  const Array<Music::Node>& Notes2);
List<Music::Node> TieSequenceUntilBarline(Pointer<Music> M,
  const List<Music::Node>& TiedSequence);

#ifdef BELLE_IMPLEMENTATION

/**Changing the time signature. Multiples all time signatures in the score
by the specified ratio to ensure that the time signature change will maintain
the original number of beats. If no beat divisions are given then the default
divisions will be used.*/
void ChangeTimeSignature(Pointer<Music> M, Ratio TimeSignatureChange,
  List<Ratio> Divisions)
{
  UnlinkUnnecessaryInstantwiseEdges(*M);

  Pointer<Geometry> G = System::MutableGeometry(M);
  G->Parse(*M);
  Array<Music::ConstNode> IslandBeginnings = G->GetPartBeginnings();
  List<List<List<Music::Node> > > SystemRanges;

  //Finding the various ranges of Time Signatures
  for(count i = 0; i < IslandBeginnings.n(); i++)
    SystemRanges.Push(SystemTimeSignatureRanges(M,
      IslandBeginnings[i]));

  List<List<Music::Node> > TimeSignatureSystemRanges;

  //Checking validity of time signature manipulations
  for(count i = 0; i < SystemRanges.n(); i++)
    for(count j = 0; j < SystemRanges[i].n(); j++)
    {
      if(!IsValidTimeSignatureManipulation(SystemRanges[i][j].a(),
        TimeSignatureChange, Divisions))
      continue;
      TimeSignatureSystemRanges.Push(SystemRanges[i][j]);
    }

  Music::Node TimeSignature;
  Ratio CurrentNoteValue;
  Ratio CurrentBeats;

  for(count i = 0; i < TimeSignatureSystemRanges.n();i++)
  {
    Music::Node CurrentTimeSignature;
    if(not (CurrentTimeSignature = TimeSignatureSystemRanges[i].a())) continue;

    GetTimeSignatureData(CurrentTimeSignature, CurrentNoteValue, CurrentBeats);
    TimeSignature = M->Promote(CurrentTimeSignature);

    //Changing the Time Signatures
    Ratio NewNoteValue = CurrentNoteValue / TimeSignatureChange;
    Ratio NewBeats = CurrentBeats * TimeSignatureChange;

    if(TimeSignature->Get(mica::Value) == mica::CommonTime)
      TimeSignature->Set(mica::Value) = mica::RegularTimeSignature;

    if(TimeSignature->Get(mica::Value) == mica::CutTime)
      TimeSignature->Set(mica::Value) = mica::RegularTimeSignature;

    TimeSignature->Set(mica::NoteValue) = mica::Concept(NewNoteValue);
    TimeSignature->Set(mica::Beats) = mica::Concept(NewBeats);

    if(!Divisions.n())
      Divisions = CommonDivisions(TimeSignature);

    if(!Divisions.n())
      return;

    List<List<Music::Node> > SyncopatedSections;

    ClearAllBeams(M, TimeSignatureSystemRanges[i]);
    NoteValueChange(M, Divisions, NewNoteValue, NewBeats,
      TimeSignatureSystemRanges[i]);
    CombineTies(M, Divisions, NewNoteValue, NewBeats, SyncopatedSections,
      TimeSignatureSystemRanges[i]);
    RemoveAllVoices(*M,
      TimeSignatureSystemRanges[i].a()->Previous(MusicLabel(mica::Token)));
    VoiceTogetherSingleVoice(*M,
      TimeSignatureSystemRanges[i].a()->Previous(MusicLabel(mica::Token)));
    BeamChange(*M, Divisions, NewNoteValue, NewBeats,
      TimeSignatureSystemRanges[i]);
    BeamSyncopatedPassages(*M, SyncopatedSections);
  }
}

/**Combines all unnecessary tied chords in the specified range. All tied chords
that lie within a beat division are deemed unnecessary. The function also
detects any syncopated sequences in the specified range.*/
void CombineTies(Pointer<Music> M, const List<Ratio>& Divisions,
  Ratio NewNoteValue, Ratio NewBeats,
  List<List<Music::Node> >& SyncopatedSections,
  List<Music::Node> TimeSignatureRange)
{
  Ratio BeatsTraversed = 0;
  count i = 0;
  Ratio SumOfDivisions = Divisions[i];
  Ratio SyncopationBeginning = 0;

  Music::Node BeginningIsland =
    TimeSignatureRange.a()->Previous(MusicLabel(mica::Token));
  Music::Node EndingIsland = Music::Node();
  if(TimeSignatureRange.z())
    EndingIsland = TimeSignatureRange.z()->Previous(MusicLabel(mica::Token));

  for(Music::Node x = BeginningIsland; x and
    x != EndingIsland; x = x->Next(MusicLabel(mica::Partwise)))
  {
    Music::Node CurrentToken;
    if((CurrentToken = x->Next(MusicLabel(mica::Token))))
    {
      if(BeatsTraversed == 0)
        if(IsChord(CurrentToken) and !IsRest(CurrentToken))
        {
          SyncopatedSections.Push(List<Music::Node>());
          SyncopationBeginning = Ratio(CurrentToken->Get(mica::NoteValue));
        }

      if(BeatsTraversed == SumOfDivisions * NewNoteValue)
      {
        if(IsChord(CurrentToken) and !IsRest(CurrentToken))
        {
          SyncopatedSections.Push(List<Music::Node>());
          SyncopationBeginning = Ratio(CurrentToken->Get(mica::NoteValue));
        }

        if(++i < Divisions.n())
          SumOfDivisions += Divisions[i];
      }

      if(CurrentToken->Get(mica::Kind) == mica::Barline ||
        BeatsTraversed == NewNoteValue * NewBeats)
      {
        BeatsTraversed = 0;
        i = 0;
        SumOfDivisions = Divisions[i];
        continue;
      }

      if(IsChord(CurrentToken))
      {
        List<Music::Node> TiedNotes = TieCheck(CurrentToken);

        if(TiedNotes.n() == 0)
        {
          if(OnOffBeat(BeatsTraversed, SumOfDivisions, Divisions, NewNoteValue))
          {
            Ratio Length = Ratio(CurrentToken->Get(mica::NoteValue));
            if(!IsRest(CurrentToken))
            {
              if(IsValidSyncopatedRhythm(Length, SyncopationBeginning,
                NewNoteValue))
                  SyncopatedSections.z().Push(CurrentToken);
            }
            else
              SyncopationBeginning = 0;
          }

          BeatsTraversed = BeatsTraversed +
            Ratio(CurrentToken->Get(mica::NoteValue));
          continue;
        }

        TiedNotes = TieSequenceUntilBarline(M, TiedNotes);

        x = MergeTieSequence(M, TiedNotes, BeatsTraversed,
          SumOfDivisions, NewNoteValue, Divisions, i, SyncopationBeginning,
          SyncopatedSections);
      }
    }
  }
}

///The default beat divisions for the given time signature.
List<Ratio> CommonDivisions(Music::Node TimeSignature)
{
  if(not TimeSignature) return List<Ratio>();
  Ratio Beats;
  Ratio NoteValue;
  GetTimeSignatureData(TimeSignature, NoteValue, Beats);
  List<Ratio> Divisions;

  //2/2 or 2/4
  if(Beats == 2)
    for(count i = 0; i < 2; i++)
      Divisions.Push(1);
  else if(NoteValue == Ratio(1, 2))
    for(Ratio i = 0; i < Beats; i++)
      Divisions.Push(1);
  //3/4
  else if(Beats == 3 and (NoteValue == Ratio(1, 4)))
    for(count i = 0; i < 3; i++)
      Divisions.Push(1);
  //4/4
  else if(Beats == 4 and NoteValue == Ratio(1,4))
    for(count i = 0; i < 4; i++)
      Divisions.Push(1);
  //6/4
  else if(Beats == 6 and NoteValue == Ratio(1,4))
    for(count i = 0; i < 2; i++)
      Divisions.Push(3);
  //4/8
  else if(Beats == 4 and NoteValue == Ratio(1, 8))
    for(count i = 0; i < 2; i++)
      Divisions.Push(2);
  //3/8
  else if(Beats == 3 and (NoteValue == Ratio(1, 8) or NoteValue == Ratio(1,16)))
    Divisions.Push(3);
  //6/8
  else if(Beats == 6 && (NoteValue == Ratio(1, 8) or NoteValue == Ratio(1,16)))
    for(count i = 0; i < 2; i++)
      Divisions.Push(3);
  //9/8
  else if(Beats == 9 && (NoteValue == Ratio(1, 8) or NoteValue == Ratio(1,16)))
    for(count i = 0; i < 3; i++)
      Divisions.Push(3);
  //12/8
  else if(Beats == 12 && (NoteValue == Ratio(1, 8) or NoteValue == Ratio(1,16)))
    for(count i = 0; i < 4; i++)
      Divisions.Push(3);
  //15/8
  else if(Beats == 15 and NoteValue == Ratio(1,8))
    for(count i = 0; i < 5; i++)
      Divisions.Push(3);
  else if(NoteValue == Ratio(1, 4))
    for(Ratio i = 0; i < Beats; i++)
      Divisions.Push(1);
  //Ambiguous time signatures
  else
  {
    Ratio Division = 2;
    for(; Division <= Beats; Division +=2)
      Divisions.Push(2);

    if(Division - Beats == 1 and Divisions.n() > 0)
      Divisions.z()++;
  }

  return Divisions;
}

/**Checks to see whether the given tied sequence of notes should be combined
even though they do not lie within a beat division.*/
Music::Node ExceptionDuration(Pointer<Music> M, List<Music::Node> TiedNotes,
  Ratio& BeatsTraversed, const List<Ratio>& Divisions, Ratio& SumOfDivisions,
  Ratio NewNoteValue, count& Counter, Ratio& SyncopationBeginning,
  List<List<Music::Node> >& SyncopatedSections)
{
  Ratio PreviousRatio = Divisions.a();
  for(count i = 1; i < Divisions.n(); i++)
  {
    if(PreviousRatio != Divisions[i])
      return Music::Node();
    PreviousRatio = Divisions[i];
  }

  if(not OnOffBeat(BeatsTraversed, SumOfDivisions, Divisions, NewNoteValue))
    return ExceptionOnBeat(M, TiedNotes, BeatsTraversed, Divisions,
      SumOfDivisions, NewNoteValue, Counter);

  else
    return ExceptionSyncopation(M, TiedNotes, BeatsTraversed, Divisions,
      SumOfDivisions, NewNoteValue, Counter, SyncopationBeginning,
      SyncopatedSections);
}

/**If the tied sequence starts on a beat, then combine as many chords in the
tie sequence as the time signature permits.*/
Music::Node ExceptionOnBeat(Pointer<Music> M, List<Music::Node>& TiedNotes,
  Ratio& BeatsTraversed, const List<Ratio>& Divisions, Ratio& SumOfDivisions,
  Ratio NewNoteValue, count& Counter)
{
  count ExceptionCounter = 0;
  Ratio Offset = BeatsTraversed;
  Ratio BeatsInSequence = BeatsTraversed;
  Ratio SumInSequence = SumOfDivisions;
  count Count = Counter;

  for(count i = 0; i < TiedNotes.n(); i++)
  {
    if(BeatsInSequence == SumInSequence * NewNoteValue)
      if(++Count < Divisions.n())
        SumInSequence += Divisions[Count];

    BeatsInSequence += Ratio(TiedNotes[i]->Get(mica::NoteValue));

    if(IsValidLongRhythm(BeatsInSequence - Offset, Divisions, NewNoteValue))
      ExceptionCounter = i;
  }

  if(ExceptionCounter == 0) return Music::Node();

  List<Music::Node> NewTiedList;

  for(count j = 0; j <= ExceptionCounter; j++)
  {
    if(BeatsTraversed == SumOfDivisions * NewNoteValue)
      if(++Counter < Divisions.n())
        SumOfDivisions += Divisions[Counter];

    BeatsTraversed += Ratio(TiedNotes[j]->Get(mica::NoteValue));
  }

  for(count j = 0; j <= ExceptionCounter; j++)
    NewTiedList.Push(TiedNotes[j]);

  Music::Node NewChord = MergeNotes(M, NewTiedList);

  return NewChord->Previous(MusicLabel(mica::Token));
}

/**If the tied sequence starts on an off beat, combine all the tied chords
if the new chord will belong to a syncopated sequence. If not, then check
to see if the tied sequence can create a valid dotted rhythm. This is to
account for cases where a dotted quarter note is be preceded by its
corresponding eighth note.*/
Music::Node ExceptionSyncopation(Pointer<Music> M, List<Music::Node>& TiedNotes,
  Ratio& BeatsTraversed, const List<Ratio>& Divisions, Ratio& SumOfDivisions,
  Ratio NewNoteValue, count& Counter, Ratio& SyncopationBeginning,
  List<List<Music::Node> >& SyncopatedSections)
{
    Ratio Offset = BeatsTraversed;

    for(count i = 0; i < TiedNotes.n(); i++)
    {
      if(BeatsTraversed == SumOfDivisions * NewNoteValue)
        if(++Counter < Divisions.n())
          SumOfDivisions += Divisions[Counter];

      BeatsTraversed += Ratio(TiedNotes[i]->Get(mica::NoteValue));
    }

    if(IsValidSyncopatedRhythm(BeatsTraversed - Offset, SyncopationBeginning,
      NewNoteValue))
    {
      Music::Node NewChord = MergeNotes(M, TiedNotes);
      SyncopatedSections.z().Push(NewChord);
      return NewChord->Previous(MusicLabel(mica::Token));
    }

    return Music::Node();
}

///Calculates the amount of beats traversed in the measure
Ratio GetBeatsTraversed(const Matrix<Ratio>& RhythmMatrix,
  const Tree<Music::ConstNode, VectorInt>& NodeToIndexLookup,
  Ratio CurrentBarOnset, Music::Node Chord)
{
  if(not Chord) return Ratio(0, 0);
  VectorInt RhythmIndex =
      NodeToIndexLookup.Get(Chord->Previous(MusicLabel(mica::Token)));
  return RhythmMatrix(count(RhythmIndex.j()),
    count(RhythmIndex.i())) - CurrentBarOnset;
}

///Returns the note value and beats associated with the time signature.
void GetTimeSignatureData(Music::ConstNode TimeSignature,
  Ratio& NoteValue, Ratio& Beats)
{
  Beats = NoteValue = Ratio();
  if(not TimeSignature)
    return;

  if(TimeSignature->Get(mica::Value) == mica::CommonTime)
  {
    NoteValue = Ratio(1, 4);
    Beats = 4;
  }

  else if(TimeSignature->Get(mica::Value) == mica::CutTime)
  {
    NoteValue = Ratio(1, 2);
    Beats = 2;
  }

  else
  {
    NoteValue = Ratio(TimeSignature->Get(mica::NoteValue));
    Beats = Ratio(TimeSignature->Get(mica::Beats));
  }
}

/**Checks to see if the note value is a valid "long" duration. If the
music is in simple meter, then any note value greater than a dotted
quarter is a valid "long" duration. If the music is in compound meter
then only multiples of a dotted quarter are valid "long" durations.*/
bool IsValidLongRhythm(Ratio Length, const List<Ratio>& Divisions,
  Ratio NoteValue)
{
  if(not IsSingleDuration(Length)) return false;

  if(NoteValue == Ratio(1, 4) or NoteValue == Ratio(1, 2))
    if(Length >= Ratio(3, 8))
      return true;

  Ratio WholeDivisions = 0;
  if(NoteValue == Ratio(1, 8))
  {
    for(count i = 0; i < Divisions.n(); i++)
    {
      WholeDivisions += Divisions[i];
      if(Length == (WholeDivisions * NoteValue))
        return true;
    }
  }

  return false;
}

/**If combining the tie sequence creates a chord with a non-existent
note value, return a shortened list of chords whose combined note value is
valid.*/
List<Music::Node> IsValidMergeRhythm(count& Counter,
  const List<Music::Node>& TiedSequence)
{
  Ratio Sum = 0;
  for(count i = 0; i < TiedSequence.n(); i++)
  {
    Sum += Ratio(TiedSequence[i]->Get(mica::NoteValue));
    if(IsSingleDuration(Sum))
      Counter = i;
  }

  List<Music::Node> NewTiedlist;
  for(count i = 0; i <= Counter; i++)
    NewTiedlist.Push(TiedSequence[i]);

  return NewTiedlist;
}

/**Checks to see if the speicifed note value belongs to a syncopated sequence.*/
bool IsValidSyncopatedRhythm(Ratio Length, Ratio& SyncopationBeginning,
  Ratio NoteValue)
{
  if(not IsSingleDuration(Length)) return false;
  if(SyncopationBeginning * 2 == Length and SyncopationBeginning < Ratio(1, 4))
    return true;

  if(NoteValue == Ratio(1, 4) or NoteValue == Ratio(1, 2))
  {
    if(SyncopationBeginning * 2 == Length)
      return true;
    if(SyncopationBeginning * 3 == Length)
      return true;
  }

  SyncopationBeginning = 0;
  return false;
}

/**Checks to see if the user has specified a valid time signature manipulation:
1) The sum of beat divisions equals the total number of beats in the time
signature. 2) Multiplying the time signature by the specifed ratio maintains
validity of the time signature.*/
bool IsValidTimeSignatureManipulation(Music::Node CurrentTimeSignature,
  Ratio TimeSignatureChange, const List<Ratio>& Divisions)
{
  if(!CurrentTimeSignature) return false;

  Ratio CurrentNoteValue;
  Ratio CurrentBeats;

  GetTimeSignatureData(CurrentTimeSignature, CurrentNoteValue, CurrentBeats);

  Ratio NewNoteValue = CurrentNoteValue / TimeSignatureChange;
  Ratio NewBeats = CurrentBeats * TimeSignatureChange;

  if(!NewBeats.IsWhole())
    return false;

  if(NewNoteValue != UndottedDuration(NewNoteValue) || NewNoteValue > 1)
    return false;

  Ratio Sum = 0;

  if(!Divisions.n()) return true;

  for(count i = 0; i < Divisions.n(); i++)
    Sum += Divisions[i];

  if(Sum != CurrentBeats * CurrentNoteValue / NewNoteValue)
    return false;

  return true;
}

/**Combining all the notes in the tie sequence. Make sure to maintain
incoming and outgoing ties from the tie sequence.*/
Music::Node MergeNotes(Pointer<Music> M, List<Music::Node> TiedSequence)
{
  if(TiedSequence.n() == 0) return Music::Node();
  if(TiedSequence.n() == 1) return TiedSequence.a();

  Array<Music::Node> ChildrenChord1 =
    TiedSequence.a()->Children(MusicLabel(mica::Note));

  Array<Music::Node> ChildrenChord2 =
    TiedSequence.z()->Children(MusicLabel(mica::Note));

  List<mica::Concept> Notes;
  for(count i = 0; i < ChildrenChord1.n(); i++)
    Notes.Push(ChildrenChord1[i]->Get(mica::Value));

  Ratio Sum = 0;
  for(count i = 0; i < TiedSequence.n(); i++)
    Sum += Ratio(TiedSequence[i]->Get(mica::NoteValue));

  Array<Music::Node> TiedNotesPrevious;
  Array<Music::Node> TiedNotesNext;

  for(count i = 0; i < ChildrenChord1.n(); i++)
  {
    Music::Node x;
    if((x = ChildrenChord1[i]->Previous(MusicLabel(mica::Tie))))
    {
      TiedNotesPrevious.Push(x);
      M->Disconnect(ChildrenChord1[i]->Previous(MusicLabel(mica::Tie), true));
    }
  }

  for(count i = 0; i < ChildrenChord2.n(); i++)
  {
    Music::Node x;
    if((x = ChildrenChord2[i]->Next(MusicLabel(mica::Tie))))
    {
      TiedNotesNext.Push(x);
      M->Disconnect(ChildrenChord2[i]->Next(MusicLabel(mica::Tie), true));
    }
  }
  Music::Node LastIslandInSequence =
    TiedSequence.z()->Previous(MusicLabel(mica::Token));
  Music::Node IslandAfter;
  if(LastIslandInSequence)
    IslandAfter =
      LastIslandInSequence->Next(MusicLabel(mica::Partwise));

  Music::Node FirstIslandInSequence =
    TiedSequence.a()->Previous(MusicLabel(mica::Token));
  Music::Node IslandBefore;
  if(FirstIslandInSequence)
    IslandBefore =
      FirstIslandInSequence->Previous(MusicLabel(mica::Partwise));

  for(count i = 0; i < TiedSequence.n(); i++)
    RemoveIsland(M, TiedSequence[i]->Previous(MusicLabel(mica::Token)));

  Music::Node IslandBetween;
  if(IslandAfter)
    IslandBetween =
      HelperAddIslandBetween(*M, IslandAfter, IslandBefore);
  else
    IslandBetween = HelperAddIslandToEnd(*M, IslandBefore);

  Music::Node NewChord =
    HelperAddNotestoIsland(M, IslandBetween, Sum, Notes);

  Array<Music::Node> NewNotes =
    NewChord->Children(MusicLabel(mica::Note));

  TieNotesTogether(*M, TiedNotesPrevious, NewNotes);
  TieNotesTogether(*M, NewNotes, TiedNotesNext);

  return NewChord;
}

/**Combine the chords in the specified tie sequence accounting for
any exceptions and invalid rhythms that the merge might yield.*/
Music::Node MergeTieSequence(Pointer<Music> M, List<Music::Node>& TiedNotes,
  Ratio& BeatsTraversed, Ratio& SumOfDivisions, Ratio NewNoteValue,
  const List<Ratio>& Divisions, count& Counter, Ratio& SyncopationBeginning,
  List<List<Music::Node> >& SyncopatedSections)
{
  if(not M) return Music::Node();
  count i = 0;

  Ratio OldBeatsTraversed = BeatsTraversed;
  Ratio OldSumOfDivision = SumOfDivisions;
  count OldCounter = Counter;

  Music::Node DurationExceptionExists = Music::Node();

  DurationExceptionExists = ExceptionDuration(M, TiedNotes, BeatsTraversed,
    Divisions, SumOfDivisions, NewNoteValue, Counter, SyncopationBeginning,
    SyncopatedSections);

  if(DurationExceptionExists) return DurationExceptionExists;

  BeatsTraversed = OldBeatsTraversed;
  SumOfDivisions = OldSumOfDivision;
  Counter = OldCounter;
  Ratio BeatsInSequence = BeatsTraversed;
  Ratio NextDivision = SumOfDivisions * NewNoteValue;

  while(BeatsInSequence != NextDivision && i < TiedNotes.n())
  {
    BeatsInSequence += Ratio(TiedNotes[i]->Get(mica::NoteValue));
    i++;
  }

  List<Music::Node> NewTiedList;

  if(BeatsInSequence == NextDivision)
    for(count j = 0; j < i; j++)
      NewTiedList.Push(TiedNotes[j]);

  else if(i == TiedNotes.n())
    NewTiedList = TiedNotes;

  else return Music::Node();

  count Index = 0;
  NewTiedList = IsValidMergeRhythm(Index, NewTiedList);
  for(count j = 0; j <= Index; j++)
    BeatsTraversed += Ratio(NewTiedList[j]->Get(mica::NoteValue));

  Music::Node NewChord = MergeNotes(M, NewTiedList);
  return NewChord->Previous(MusicLabel(mica::Token));
}

/**Iterate through the specified range, TimeSignatureRange, splitting up
any chords that exceed a beat division.*/
void NoteValueChange(Pointer<Music> M, const List<Ratio>& Divisions,
  Ratio NewNoteValue, Ratio NewBeats,
  List<Music::Node> TimeSignatureRange)
{
  Ratio BeatsTraversed = 0;
  count i = 0;
  Ratio SumOfDivisions = Divisions[i];
  Music::Node PreviousChord;

  Music::Node BeginningIsland =
    TimeSignatureRange.a()->Previous(MusicLabel(mica::Token));
  Music::Node EndingIsland = Music::Node();
  if(TimeSignatureRange.z())
    EndingIsland = TimeSignatureRange.z()->Previous(MusicLabel(mica::Token));

  for(Music::Node x = BeginningIsland; x and
    x != EndingIsland; x = x->Next(MusicLabel(mica::Partwise)))
  {
    Music::Node CurrentToken;
    if((CurrentToken = x->Next(MusicLabel(mica::Token))))
    {
      if(BeatsTraversed == SumOfDivisions * NewNoteValue)
      {
        if(++i < Divisions.n())
          SumOfDivisions += Divisions[i];
      }

      if(BeatsTraversed > SumOfDivisions * NewNoteValue)
      {
        while(SumOfDivisions * NewNoteValue <= BeatsTraversed &&
          ++i < Divisions.n())
        {
          if(BeatsTraversed > SumOfDivisions * NewNoteValue)
            PreviousChord = SplitChord(M, PreviousChord, BeatsTraversed,
                              SumOfDivisions * NewNoteValue);
          SumOfDivisions += Divisions[i];
        }

        Music::Node Remaining2 = PreviousChordExistIncludingRest(x);
        Music::Node RemainingIsland2 =
          Remaining2->Previous(MusicLabel(mica::Token));
        Music::Node Remaining1 =
          PreviousChordExistIncludingRest(RemainingIsland2);

        Ratio Remainder2 = Ratio(Remaining2->Get(mica::NoteValue));
        Ratio Remainder1 = Ratio(Remaining1->Get(mica::NoteValue));
        SplitRemainingChord(M, Remainder2, CurrentToken, Remaining2);
        SplitRemainingChord(M, Remainder1, Remaining2, Remaining1);

      }

      if(CurrentToken->Get(mica::Kind) == mica::Barline ||
        BeatsTraversed == NewNoteValue * NewBeats)
      {
        BeatsTraversed = 0;
        i = 0;
        SumOfDivisions = Divisions[i];
        PreviousChord = Music::Node();
        continue;
      }

      if(IsChord(CurrentToken))
      {
        PreviousChord = CurrentToken;
        BeatsTraversed = BeatsTraversed +
          Ratio(CurrentToken->Get(mica::NoteValue));
      }
    }
  }
}

///Checks to see if the music is on an off beat.
bool OnOffBeat(Ratio BeatsTraversed, const Ratio& SumOfDivisions,
  const List<Ratio>& Divisions, Ratio NewNoteValue)
{
  return BeatsTraversed != 0 and
    BeatsTraversed != (SumOfDivisions * NewNoteValue -
      NewNoteValue * Divisions.a());
}

///Remove the island from the graph.
void RemoveIsland(Pointer<Music> M, Music::ConstNode Island)
{
  Array<Music::ConstNode> Chords = ChordsOfIsland(Island);
  for(Counter i; i.z(Chords); i++)
    RemoveAllNotes(*M, M->Promote(Chords[i])),
    M->Remove(M->Promote(Chords[i]));
  Array<Music::ConstNode> Tokens = TokensOfIsland(Island);
  for(Counter i; i.z(Tokens); i++)
    M->Remove(M->Promote(Tokens[i]));
  M->Remove(M->Promote(Island));
}

/**If the chord is sustained over a beat division, split the chord into two
such that the second chord lies on the beat division. */
Music::Node SplitChord(Pointer<Music> M, Music::Node ChordToken,
  Ratio BeatsTraversed, Ratio Beat)
{
  if(not IsChord(ChordToken)) return Music::Node();
  Ratio Remainder = BeatsTraversed - Beat;
  Array<Music::Node> CurrentNotes;

  if(!IsRest(ChordToken))
    CurrentNotes = ChordToken->Children(MusicLabel(mica::Note));

  Ratio NoteValue = Ratio(ChordToken->Get(mica::NoteValue));

  Array<Music::Node> TiedPreviousNotes;
  Array<Music::Node> TiedNextNotes;
  List<mica::Concept> Notes;

  if(!IsRest(ChordToken))
  {
    for(count i = 0; i < CurrentNotes.n(); i++)
      Notes.Push(CurrentNotes[i]->Get(mica::Value));

    for(count i = 0; i < CurrentNotes.n(); i++)
      if(CurrentNotes[i]->Previous(MusicLabel(mica::Tie)))
      {
        TiedPreviousNotes.Push(
          CurrentNotes[i]->Previous(MusicLabel(mica::Tie)));
        M->Disconnect(CurrentNotes[i]->Previous(MusicLabel(mica::Tie), true));
      }

    for(count i = 0; i < CurrentNotes.n(); i++)
    {
      if(CurrentNotes[i]->Next(MusicLabel(mica::Tie)))
      {
        TiedNextNotes.Push(CurrentNotes[i]->Next(MusicLabel(mica::Tie)));
        M->Disconnect(CurrentNotes[i]->Next(MusicLabel(mica::Tie), true));
      }
    }
  }
  ChordToken->Set(mica::NoteValue) = mica::Concept(Remainder);

  Music::Node CurrentIsland = ChordToken->Previous(MusicLabel(mica::Token));
  Music::Node IslandBefore = HelperAddIslandBefore(*M, CurrentIsland);

  Music::Node PreviousChord =
    HelperAddNotestoIsland(M, IslandBefore, (NoteValue - Remainder), Notes);

  if(!IsRest(ChordToken))
    TieConnectNotes(*M, PreviousChord, ChordToken);

  Array<Music::Node> PreviousNotes =
    PreviousChord->Children(MusicLabel(mica::Note));

  if(!IsRest(ChordToken))
  {
    TieNotesTogether(*M, TiedPreviousNotes, PreviousNotes);
    TieNotesTogether(*M, CurrentNotes, TiedNextNotes);
  }

  return ChordToken;
}

///Splits up the note values until all the ratios are valid rhythms.
void SplitIntoValidChordRatios(Ratio Value, Array<Ratio>& RatioArray)
{
  Ratio NearestValidRhythm;
  while(!IsSingleDuration(Value) && Value > 0)
  {
    count Den = count(Value.Denominator());
    if(Den % 2 != 0 and Den != 1) return;
    NearestValidRhythm = UndottedDuration(Value);
    Value -= NearestValidRhythm;
    RatioArray.Add(NearestValidRhythm);
  }
  RatioArray.Add(Value);
}

/**Takes the given chord and splits the chord up until each chord has a valid
note value.*/
void SplitRemainingChord(Pointer<Music> M, Ratio Value,
  Music::Node CurrentToken, Music::Node RemainingChord)
{
  Array<Ratio> RatioArray;
  SplitIntoValidChordRatios(Value, RatioArray);
  if(RatioArray.n() == 1) return;

  Array<Music::Node> CurrentNotes =
    RemainingChord->Children(MusicLabel(mica::Note));

  Array<Music::Node> TiedPreviousNotes;
  Array<Music::Node> TiedNextNotes;
  List<mica::Concept> Notes;

  for(count i = 0; i < CurrentNotes.n(); i++)
    Notes.Push(CurrentNotes[i]->Get(mica::Value));

  for(count i = 0; i < CurrentNotes.n(); i++)
  {
    Music::Node x;
    if((x = CurrentNotes[i]->Previous(MusicLabel(mica::Tie))))
    {
      TiedPreviousNotes.Push(x);
      M->Disconnect(CurrentNotes[i]->Previous(MusicLabel(mica::Tie), true));
    }
  }

  for(count i = 0; i < CurrentNotes.n(); i++)
  {
    Music::Node x;
    if((x = CurrentNotes[i]->Next(MusicLabel(mica::Tie))))
    {
      TiedNextNotes.Push(x);
      M->Disconnect(CurrentNotes[i]->Next(MusicLabel(mica::Tie), true));
    }
  }

  Music::Node CurrentIsland = CurrentToken->Previous(MusicLabel(mica::Token));
  Music::Node RemainingIsland =
    RemainingChord->Previous(MusicLabel(mica::Token));
  Music::Node PreviousIsland =
    RemainingIsland->Previous(MusicLabel(mica::Partwise));

  RemoveIsland(M, RemainingIsland);
  M->Connect(PreviousIsland, CurrentIsland)->Set(mica::Type) = mica::Partwise;

  Music::Node x = CurrentIsland;
  Music::Node LastChord = Music::Node();
  Music::Node CurrentChord;
  List<Music::Node> ListOfAddedChords;

  for(count i = RatioArray.n() - 1; i >= 0; i--)
  {
    Music::Node AddedIsland = HelperAddIslandBefore(*M, x);
    CurrentChord =
      HelperAddNotestoIsland(M, AddedIsland, RatioArray[i], Notes);
    ListOfAddedChords.Prepend(CurrentChord);
    if(LastChord)
      TieConnectNotes(*M, CurrentChord, LastChord);
    LastChord = CurrentChord;
    x = AddedIsland;
  }

  Array<Music::Node> LastAddedIslandNotes =
    ListOfAddedChords.z()->Children(MusicLabel(mica::Note));
  Array<Music::Node> FirstAddedIslandNotes =
    ListOfAddedChords.a()->Children(MusicLabel(mica::Note));

  TieNotesTogether(*M, TiedPreviousNotes, FirstAddedIslandNotes);
  TieNotesTogether(*M, LastAddedIslandNotes, TiedNextNotes);
}

///Ties together any two notes that are the same in the two note arrays.
void TieNotesTogether(Music& M, const Array<Music::Node>& Notes1,
  const Array<Music::Node>& Notes2)
{
  for(count i = 0; i < Notes1.n(); i++)
    for(count j = 0; j < Notes2.n(); j++)
      if((Notes1[i]->Get(mica::Value)) == (Notes2[j]->Get(mica::Value)))
        M.Connect(Notes1[i], Notes2[j])->Set(mica::Type) = mica::Tie;
}

///If the tie sequence extends past a barline, then only tie upto that barline.
List<Music::Node> TieSequenceUntilBarline(Pointer<Music> M,
  const List<Music::Node>& TiedSequence)
{
  count i = 0;
  List<Music::Node> NewTiedSequence;
  for(; i < TiedSequence.n() - 1; i++)
  {
    Music::Node Island = TiedSequence[i]->Previous(MusicLabel(mica::Token));
    Music::Node NextIslandInSequence =
      TiedSequence[i + 1]->Previous(MusicLabel(mica::Token));

    if(GetBarlineBetweenIslands(*M, Island, NextIslandInSequence))
      break;
  }

  if(i == TiedSequence.n() - 1) return TiedSequence;

  for(count j = 0; j <= i; j++)
    NewTiedSequence.Push(TiedSequence[j]);

  return NewTiedSequence;
}

#endif
///@}
