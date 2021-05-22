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
@name Beaming

Functions that help the beaming filter.
*/
void BeamByVoiceStrands(
  Pointer<Music> M,
  const List<Array<Music::ConstNode> >& VoiceStrands,
  count i,
  const Tree<Music::ConstNode, VectorInt>& NodeToIndexLookup,
  const Matrix<Ratio>& RhythmMatrix,
  Ratio CurrentBarOnset,
  Ratio SumOfDivisions,
  const List<Ratio>& Divisions,
  Ratio NewNoteValue,
  Ratio& BeatsTraversed,
  Ratio& SyncopationBeginning,
  List<List<Music::Node> >& SyncopatedSections,
  count& k,
  const Ratio Beats);
void BeamChange(Music& M, const List<Ratio>& Divisions, Ratio NewNoteValue,
  Ratio NewBeats, List<Music::Node> TimeSignatureRange);
void BeamChordsTogether(Music& M, List<Music::Node> Chords);
void BeamInnerTuplets(Pointer<Music> M, Music::ConstNode TupletToken,
  Tree<String, List<Music::Node> >& TupletBeams);
void BeamSyncopatedPassages(Music& M,
  List<List<Music::Node> >& SyncopatedSections);
Music::Node BeamTuplets(Pointer<Music> M, Music::ConstNode TupletToken,
  List<Music::Node>& TupletBeamedChords);
void ClearAllBeams(Pointer<Music> M, Music::Node BeginningToken,
  Music::Node EndingToken);
void ClearAllBeams(Pointer<Music> M, count BeginningBarline,
  count EndingBarline);
void ClearAllBeams(Pointer<Music> M,
  List<Music::Node> NodePair);
void CombineAndReorderChordLists(List<Music::Node>& ChordList1,
  List<Music::Node>& ChordList2, bool Reverse);
bool IsBeamed(Music::Node Chord);
bool IsChordBeginningOfBeamGroup(Music::ConstNode Chord);
bool IsValidBeamingRhythm(Music::ConstNode Chord);
void MultivoiceBeamChange(Pointer<Music> M,
  const List<List<Music::Node> >& SelectedTokens);
void RemoveBeam(Music& M, Music::Node ChordToken);
void RemoveBeams(Music& M, const Array<Music::Node>& BeamedChords);
void RemoveBeams(Music& M, const List<Music::Node>& BeamedChords);
List<Music::Node> ReorderChordsInList(List<Music::Node>& ChordList,
  bool Reverse);
List<Ratio> TupletDivisions(const List<Ratio>& Divisions,
  Ratio NewNoteValue, Ratio Beats, Ratio SumOfDivisions,
  Ratio BeatsTraversed, Ratio& TupletNoteValue,
  Ratio& TupletSumOfDivisions, count& TupletCounter, count Counter);

#ifdef BELLE_IMPLEMENTATION

///Beam the voice strand in the voice strand array specified by the index i.
void BeamByVoiceStrands(
  Pointer<Music> M,
  const List<Array<Music::ConstNode> >& VoiceStrands,
  count i,
  const Tree<Music::ConstNode, VectorInt>& NodeToIndexLookup,
  const Matrix<Ratio>& RhythmMatrix,
  Ratio CurrentBarOnset,
  Ratio SumOfDivisions,
  const List<Ratio>& Divisions,
  Ratio NewNoteValue,
  Ratio& BeatsTraversed,
  Ratio& SyncopationBeginning,
  List<List<Music::Node> >& SyncopatedSections,
  count& k,
  const Ratio Beats)
{
  Ratio TupletNoteValue = 0;
  Ratio TupletSumOfDivisions;
  count TupletCounter = 0;
  List<Ratio> TupletDivisionsList;
  TupletDivisionsList = TupletDivisions(Divisions, NewNoteValue,
      Beats, SumOfDivisions, BeatsTraversed, TupletNoteValue,
      TupletSumOfDivisions, TupletCounter, k);

  //Two seperate stacks for tuplets and beamed chords
  List<Music::Node> BeamedChords;
  List<Music::Node> TupletBeamedChords;

  for(count j = 0; j <= VoiceStrands[i].n(); j++)
  {
    Music::ConstNode Chord = Music::ConstNode();
    if(j == VoiceStrands[i].n())
    {
      BeatsTraversed = GetBeatsTraversed(RhythmMatrix, NodeToIndexLookup,
        CurrentBarOnset, M->Promote(VoiceStrands[i][j -  1])) +
        RhythmicDurationOfChord(VoiceStrands[i][j -  1]);
    }
    else
    {
      Chord = VoiceStrands[i][j];
      BeatsTraversed = GetBeatsTraversed(RhythmMatrix, NodeToIndexLookup,
        CurrentBarOnset, M->Promote(Chord));
    }

    //Updating potentially new tuplet divisions
    if(BeatsTraversed >= TupletSumOfDivisions * TupletNoteValue)
    {
      //If there were TupletBeamedChords
      if(TupletBeamedChords.n())
      {
        if(BeatsTraversed > TupletSumOfDivisions * TupletNoteValue)
        {
          BeamChordsTogether(*M, BeamedChords);
          BeamedChords.RemoveAll();
          TupletBeamedChords.RemoveAll();
        }

        else
        {
          CombineAndReorderChordLists(BeamedChords, TupletBeamedChords, true);
          BeamChordsTogether(*M, BeamedChords);
          BeamedChords.RemoveAll();
          TupletBeamedChords.RemoveAll();
        }
      }

      while(TupletSumOfDivisions * TupletNoteValue <= BeatsTraversed and
      ++TupletCounter < TupletDivisionsList.n())
        TupletSumOfDivisions += TupletDivisionsList[TupletCounter];
    }

    if(BeatsTraversed >= SumOfDivisions * NewNoteValue)
    {
      BeamChordsTogether(*M, BeamedChords);
      BeamedChords.RemoveAll();
      while(SumOfDivisions * NewNoteValue <= BeatsTraversed &&
        ++k < Divisions.n())
        SumOfDivisions += Divisions[k];
    }

    /*If the music is on the token directly after the last VoiceStrand chord
    there can be no further syncopated or tuplet passages in the Voice Strand*/
    if(IsChord(Chord))
    {
      if(not OnOffBeat(BeatsTraversed, SumOfDivisions, Divisions,
        NewNoteValue))
      {
        SyncopatedSections.Push(List<Music::Node>());
        SyncopationBeginning =
          Ratio(Chord->Get(mica::NoteValue));
      }

      else
      {
        Ratio Length = Ratio(Chord->Get(mica::NoteValue));
        if(!IsRest(Chord))
        {
          if(IsValidSyncopatedRhythm(Length, SyncopationBeginning,
            NewNoteValue))
              SyncopatedSections.z().Push(M->Promote(Chord));
        }
        else
          SyncopationBeginning = 0;
      }

      Music::Node TraversedChord = Music::Node();
      if((TraversedChord = BeamTuplets(M, Chord, TupletBeamedChords)))
      {
        j = VoiceStrands[i].Search(TraversedChord);
        continue;
      }

      if(!IsRest(Chord))
        BeamedChords.Prepend(M->Promote(Chord));
    }
  }
  BeamSyncopatedPassages(*M, SyncopatedSections);
}

/**Beams single voice passages. Does not do syncopation detection.
The function is used primarily within the time signature filter.*/
void BeamChange(Music& M, const List<Ratio>& Divisions, Ratio NewNoteValue,
  Ratio NewBeats, List<Music::Node> TimeSignatureRange)
{
  List<Music::Node> BeamedChords;
  Ratio BeatsTraversed = 0;
  count i = 0;
  Ratio SumOfDivisions = Divisions[i];

  Music::Node BeginningIsland =
    TimeSignatureRange.a()->Previous(MusicLabel(mica::Token));
  Music::Node EndingIsland = Music::Node();
  if(TimeSignatureRange.z())
    EndingIsland =
      TimeSignatureRange.z()->Previous(MusicLabel(mica::Token));

  for(Music::Node x = BeginningIsland; x and
    x != EndingIsland; x = x->Next(MusicLabel(mica::Partwise)))
  {
    Music::Node CurrentToken;
    if((CurrentToken = x->Next(MusicLabel(mica::Token))))
    {
      if(BeatsTraversed >= SumOfDivisions * NewNoteValue)
      {
        BeamChordsTogether(M, BeamedChords);
        BeamedChords.RemoveAll();
        while(SumOfDivisions * NewNoteValue <= BeatsTraversed &&
          ++i < Divisions.n())
          SumOfDivisions += Divisions[i];
      }

      if(CurrentToken->Get(mica::Kind) == mica::Barline ||
        BeatsTraversed == NewNoteValue * NewBeats)
      {
        BeatsTraversed = 0;
        i = 0;
        SumOfDivisions = Divisions[i];
        BeamedChords.RemoveAll();
        continue;
      }

      if(IsChord(CurrentToken))
      {
        if(!IsRest(CurrentToken))
          BeamedChords.Prepend(CurrentToken);
        BeatsTraversed = BeatsTraversed +
          Ratio(CurrentToken->Get(mica::NoteValue));
      }
    }
  }
}

/**Beam the list of chords together. Assume that the chords have
already been voice connected.*/
void BeamChordsTogether(Music& M, List<Music::Node> Chords)
{
  Music::Node CurrentChord;
  while(Chords.n())
  {
    CurrentChord = Chords.Pop();
    if(Chords.n() and IsValidBeamingRhythm(CurrentChord) and
      IsValidBeamingRhythm(Chords.z()) and
      (not CurrentChord->Next(MusicLabel(mica::Beam))) and
      OnlyRestInBetweenChords(CurrentChord, Chords.z()))
      M.Connect(CurrentChord, Chords.z())->Set(mica::Type) = mica::Beam;
  }
}

/**Beam together the specified list of syncopated sections. Each syncopated
section is specified by its opening and ending chords.*/
void BeamSyncopatedPassages(Music& M,
  List<List<Music::Node> >& SyncopatedSections)
{
  for(count i = 0; i < SyncopatedSections.n(); i++)
  {
    if(SyncopatedSections[i].n() == 0) continue;
    Ratio NoteValue = Ratio(SyncopatedSections[i].a()->Get(mica::NoteValue));
    if(NoteValue >= Ratio(1, 4))
      continue;

    Music::Node IslandBeginning =
      SyncopatedSections[i].a()->Previous(MusicLabel(mica::Token));
    Music::Node IslandEnd =
      SyncopatedSections[i].z()->Previous(MusicLabel(mica::Token));
    Music::Node IslandPrevious =
      IslandBeginning->Previous(MusicLabel(mica::Partwise));
    Music::Node IslandNext = IslandEnd->Next(MusicLabel(mica::Partwise));

    Array<Music::Node> Beginning;
    Array<Music::Node> Ending;
    if(IslandPrevious)
      Beginning = IslandPrevious->Children(MusicLabel(mica::Token));
    if(IslandNext)
      Ending = IslandNext->Children(MusicLabel(mica::Token));

    Music::Node FirstChord;
    if(Beginning.n() == 1)
      FirstChord = Beginning.a();
    else
      FirstChord =
        SyncopatedSections[i].a()->Previous(MusicLabel(mica::Voice));

    if(Ratio(FirstChord->Get(mica::NoteValue)) * 3 == NoteValue)
      continue;

    SyncopatedSections[i].Prepend(Beginning.a());
    RemoveBeams(M, SyncopatedSections[i]);

    Music::Node End;
    if(Ending.n() == 1)
      End = Ending.a();
    else if(Ending.n() > 1)
      End = SyncopatedSections[i].z()->Next(MusicLabel(mica::Voice));
    if(End)
      SyncopatedSections[i].Push(End);

    List<Music::Node> Syncopation;
    for(count j = SyncopatedSections[i].n() - 1; j >= 0; j--)
      Syncopation.Push(SyncopatedSections[i][j]);

    BeamChordsTogether(M, Syncopation);
  }
}

/**Beam all nested tuplets starting from a given tuplet node or chord. Will
store all tuplet chords in a symbol table with key = Tag and
Value = a List of Chords.*/
void BeamInnerTuplets(Pointer<Music> M, Music::ConstNode TupletToken,
  Tree<String, List<Music::Node> >& TupletBeams)
{
  Music::ConstNode PreviousTupletChord;
  if(IsChord(TupletToken))
  {
    String Tag = TupletTagChord(TupletToken);
    if(not IsNodePartOfTuplet(TupletToken)) return;
    if(not IsRest(TupletToken))
      TupletBeams.Set(Tag).Prepend(M->Promote(TupletToken));
    if(not IsChord(TupletToken->Next(MusicLabel(mica::Tuplet))))
      BeamChordsTogether(*M, TupletBeams.Set(Tag));
    return;
  }

  MusicLabel TupletEdge;
  String Tag = TupletToken->Get("Tag");
  TupletEdge.Set(mica::Type) = mica::Tuplet;
  TupletEdge.Set("Tag") = Tag;

  TupletBeams.Set(Tag, List<Music::Node>());
  Array<Music::ConstNode> TupletSequence = TupletToken->Series(TupletEdge);

  for(count i = 1; i < TupletSequence.n(); i++)
    BeamInnerTuplets(M, TupletSequence[i], TupletBeams);
}

/**Assumes that the geometry of the graph has already been parsed. Beams the
tuplet starting from a given chord. Returns the last chord in the tuplet
and a list of all the chords in that tuplet in order of their placement
on the graph.*/
Music::Node BeamTuplets(Pointer<Music> M, Music::ConstNode Chord,
  List<Music::Node>& TupletBeamedChords)
{
  if(not Chord->Previous(MusicLabel(mica::Tuplet))) return Music::Node();
  Music::ConstNode x = Chord;
  Music::ConstNode TupletBeginning;
  for(; x; x = x->Previous(MusicLabel(mica::Tuplet)))
    TupletBeginning = x;

  Tree<String, List<Music::Node> > TupletBeams;
  BeamInnerTuplets(M, TupletBeginning, TupletBeams);

  Tree<String, List<Music::Node> >::Iterator TupletIt;
  Tree<count, Music::Node> OrderedChords;

  for(TupletIt.Begin(TupletBeams); TupletIt.Iterating(); TupletIt.Next())
  {
    List<Music::Node> NoteList = TupletIt.Value();
    for(count i = 0; i < NoteList.n(); i++)
    {
      Music::Node Island = NoteList[i]->Previous(MusicLabel(mica::Token));
      if(Island)
        OrderedChords.Set(Island->Label.GetState("InstantID").AsCount(),
          NoteList[i]);
    }
  }

  Tree<count, Music::Node>::Iterator NodeIt;
  for(NodeIt.Begin(OrderedChords); NodeIt.Iterating(); NodeIt.Next())
    TupletBeamedChords.Prepend(NodeIt.Value());

  return GetLastChordInTuplet(M, TupletBeginning);
}

/**ClearAllBeams will not only remove all beams in the given passage
and replace each beam edge with a voice edge, it will also voice connect
all single voice sections as a side effect.*/
void ClearAllBeams(Pointer<Music> M, Music::Node BeginningToken,
  Music::Node EndingToken)
{
  Music::Node BeginningIsland;
  if(BeginningToken)
  BeginningIsland = BeginningToken->Previous(MusicLabel(mica::Token));

  /*Voice connect all single voices passages as a side effect
  of the function*/
  VoiceTogetherSingleVoice(*M, BeginningIsland);
  Music::Node EndingIsland = Music::Node();
  if(EndingToken)
    EndingIsland = EndingToken->Previous(MusicLabel(mica::Token));

  for(Music::Node x = BeginningIsland; x != EndingIsland;
      x = x->Next(MusicLabel(mica::Partwise)))
  {
    if(IslandBeginsMultivoiceRegion(x))
    {
      List<Array<Music::ConstNode> > VoiceStrands;
      FindAllVoiceStrands(x, VoiceStrands);
      for(count j = 0; j < VoiceStrands.n(); j++)
      {
        count k = 0;
        while(k < VoiceStrands[j].n())
        {
          Music::Node ChordToken = M->Promote(VoiceStrands[j][k]);
          Array<Music::Node> BeamingSeries =
            ChordToken->Series(MusicLabel(mica::Beam));
          RemoveBeams(*M, BeamingSeries);

          k += BeamingSeries.n();
        }
      }
    }
  }
}

/**ClearAllBeams will not only remove all beams in the given passage
and replace each beam edge with a voice edge, it will also voice connect
all single voice sections as a side effect.*/
void ClearAllBeams(Pointer<Music> M, count BeginningBarline,
  count EndingBarline)
{
  List<List<Music::Node> > SelectedTokens =
    BarlineAddress(M, BeginningBarline, EndingBarline);
  for(count i = 0; i < SelectedTokens.n(); i++)
    ClearAllBeams(M, SelectedTokens[i].a(), SelectedTokens[i].z());
}

/**ClearAllBeams will not only remove all beams in the given passage
and replace each beam edge with a voice edge, it will also voice connect
all single voice sections as a side effect.*/
void ClearAllBeams(Pointer<Music> M,
  List<Music::Node> NodePair)
{
  ClearAllBeams(M, NodePair.a(), NodePair.z());
}

/**Combines two list of chords into a single list of orderd chords.*/
void CombineAndReorderChordLists(List<Music::Node>& ChordList1,
  List<Music::Node>& ChordList2, bool Reverse)
{
  for(count i = 0; i < ChordList2.n(); i++)
  {
    if(not IsChord(ChordList2[i])) return;
    ChordList1.Push(ChordList2[i]);
  }

  ChordList1 = ReorderChordsInList(ChordList1, Reverse);
}

///Checks to see if chord is beamed.
bool IsBeamed(Music::Node Chord)
{
  return Chord->Next(MusicLabel(mica::Beam))
    or Chord->Previous(MusicLabel(mica::Beam));
}

///Checks to see if the specified chord is the beginning of a beam group.
bool IsChordBeginningOfBeamGroup(Music::ConstNode Chord)
{
  if(not IsChord(Chord) or IsRest(Chord)) return false;
  if(Chord->Next(MusicLabel(mica::Beam)) and
    not Chord->Previous(MusicLabel(mica::Beam)))
    return true;
  return false;
}

/**Checks to see if the rhythm of the given chord is a valid beaming rhythm.
A valid beaming rhythm is any note value less than a quarter note. If the Chord
is a rest then it is unbeamable.*/
bool IsValidBeamingRhythm(Music::ConstNode Chord)
{
  if(IsChord(Chord) && !IsRest(Chord) &&
    Ratio(Chord->Get(mica::NoteValue)) < Ratio(1,4))
      return true;
  return false;
}

/**Given a list of pairs of starting and ending nodes corresponding to the
different time signature ranges, beam the voice strands within each range.*/
void MultivoiceBeamChange(Pointer<Music> M,
  const List<List<Music::Node> >& SelectedTokens)
{
  List<Array<Music::ConstNode> > NodeMatrix;
  Matrix<Ratio> RhythmMatrix;
  Tree<Music::ConstNode, VectorInt> NodeToIndexLookup;
  Pointer<Geometry> G =
    GetRhythmicOnsetInfo(M, NodeMatrix, RhythmMatrix, NodeToIndexLookup);

  for(count a = 0; a < SelectedTokens.n(); a++)
  {
    Ratio NewNoteValue, NewBeats;
    GetTimeSignatureData(SelectedTokens[a].a(), NewNoteValue, NewBeats);
    List<Ratio> Divisions = CommonDivisions(SelectedTokens[a].a());
    if(Divisions.n() == 0) return;

    Music::Node BeginningIsland =
      SelectedTokens[a].a()->Previous(MusicLabel(mica::Token));

    Music::Node EndingIsland = Music::Node();
    if(SelectedTokens[a].z())
      EndingIsland = SelectedTokens[a].z()->Previous(MusicLabel(mica::Token));

    for(Music::Node x = BeginningIsland; x != EndingIsland;
        x = x->Next(MusicLabel(mica::Partwise)))
    {
      if(IslandBeginsMultivoiceRegion(x))
      {
        List<Array<Music::ConstNode> > VoiceStrands;
        FindAllVoiceStrands(x, VoiceStrands);
        VectorInt FirstChordIndex = FindFirstChordInBarline(x, NodeMatrix,
          NodeToIndexLookup);

        Ratio CurrentBarOnset;
        if(FirstChordIndex.i() != -1)
          CurrentBarOnset =
            RhythmMatrix(count(FirstChordIndex.j()),
              count(FirstChordIndex.i()));
        else
          return;

        for(count i = 0; i < VoiceStrands.n(); i++)
        {
          Music::ConstNode FirstIsland =
            VoiceStrands[i].a()->Previous(MusicLabel(mica::Token));
          VectorInt Index = NodeToIndexLookup.Get(FirstIsland);
          Ratio StartingRhythmicOnset =
            RhythmMatrix(count(Index.j()), count(Index.i()));

          Ratio BeatsTraversed = StartingRhythmicOnset - CurrentBarOnset;

          count k = 0;
          Ratio SumOfDivisions = Divisions[k];
          while(SumOfDivisions * NewNoteValue <= BeatsTraversed &&
            ++k < Divisions.n())
            SumOfDivisions += Divisions[k];

          Ratio SyncopationBeginning = 0;
          List<List<Music::Node> > SyncopatedSections;

          BeamByVoiceStrands(M, VoiceStrands, i, NodeToIndexLookup,
            RhythmMatrix, CurrentBarOnset, SumOfDivisions, Divisions,
            NewNoteValue, BeatsTraversed, SyncopationBeginning,
            SyncopatedSections, k, NewBeats);
        }
      }
    }
  }
}

///Remove the outgoing beam from the given chord.
void RemoveBeam(Music& M, Music::Node ChordToken)
{
  if(not IsChord(ChordToken)) return;
  Music::Node NextChordToken = ChordToken->Next(MusicLabel(mica::Beam));
  if(not IsChord(NextChordToken)) return;
  M.Disconnect(ChordToken->Next(MusicLabel(mica::Beam), true));
  if(not ChordToken->Next(MusicLabel(mica::Voice)))
    M.Connect(ChordToken, NextChordToken)->Set(mica::Type) = mica::Voice;
}

///Remove the beams in the given array of chords.
void RemoveBeams(Music& M, const Array<Music::Node>& BeamedChords)
{
  for(count i = 0; i < BeamedChords.n(); i++)
    RemoveBeam(M, BeamedChords[i]);
}

///Remove the beams in the given list of chords.
void RemoveBeams(Music& M, const List<Music::Node>& BeamedChords)
{
  for(count i = 0; i < BeamedChords.n(); i++)
    RemoveBeam(M, BeamedChords[i]);
}

/**Reorder the given list of chords according to their placment on the graph,
i.e. chords earlier in the graph will appear at the beginning of the list.*/
List<Music::Node> ReorderChordsInList(List<Music::Node>& ChordList,
  bool Reverse)
{
  Tree<count, Music::Node> OrderedChords;
  List<Music::Node> ReorderdChords;
  for(count i = 0; i < ChordList.n(); i++)
  {
    if(not IsChord(ChordList[i])) return List<Music::Node>();
    Music::Node Island = ChordList[i]->Previous(MusicLabel(mica::Token));
    if(Island)
      OrderedChords.Set(Island->Label.GetState("InstantID").AsCount(),
        ChordList[i]);
  }

  Tree<count, Music::Node>::Iterator NodeIt;
  for(NodeIt.Begin(OrderedChords); NodeIt.Iterating(); NodeIt.Next())
  {
    if(Reverse)
      ReorderdChords.Prepend(NodeIt.Value());
    else
      ReorderdChords.Push(NodeIt.Value());
  }
  return ReorderdChords;
}

/**Create a new set of beat divisions that prevent tuplets from being beamed
incorrectly.*/
List<Ratio> TupletDivisions(const List<Ratio>& Divisions,
  Ratio NewNoteValue, Ratio Beats, Ratio SumOfDivisions,
  Ratio BeatsTraversed, Ratio& TupletNoteValue,
  Ratio& TupletSumOfDivisions, count& TupletCounter, count Counter)
{
  /*If the note value of the Time Signature is greater than or equal to
  a half note then tuplet divisions = quarter notes * (Beats * 2)*/
  if(NewNoteValue >= Ratio(1, 2))
  {
    TupletNoteValue = Ratio(1, 4);
    List<Ratio> TupletDivisions;
    for(Ratio i = 0; i < Beats * 2; i++)
      TupletDivisions.Push(1);
    TupletSumOfDivisions = TupletDivisions[TupletCounter];
    while(TupletSumOfDivisions * TupletNoteValue < BeatsTraversed and
      ++TupletCounter < TupletDivisions.n())
      TupletSumOfDivisions += TupletDivisions[TupletCounter];

    return TupletDivisions;
  }
  TupletNoteValue = NewNoteValue;
  TupletSumOfDivisions = SumOfDivisions;
  TupletCounter = Counter;
  return Divisions;
}

#endif
///@}
