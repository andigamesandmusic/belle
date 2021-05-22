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
@name Harmonization

Functions that help the harmonization filter.
@{
*/
Music::ConstNode FindLongestNoteDurationInIsland(Music::ConstNode Island);
Pointer<Geometry> GetRhythmicOnsetInfo(Pointer<Music> M,
  List<Array<Music::ConstNode> >& NodeMatrix,
  Matrix<Ratio>& RhythmMatrix,
  Tree<Music::ConstNode, VectorInt>& NodeToIndexLookup);
void Harmonize(Pointer<Music> M,
  List<List<mica::Concept> >HarmonizedPitches, List<Ratio> NoteValue,
  count StaffNumber);
Music::Node HelperAddChordToIsland(Music& M, Music::Node Island,
  Ratio NoteValue);
Music::Node HelperAddIslandBefore(Music& M, Music::Node IslandAfter);
Music::Node HelperAddIslandBetween(Music& M, Music::Node IslandAfter,
  Music::Node IslandBefore);
Music::Node HelperAddIslandToEnd(Music& M, Music::Node IslandBefore);
Music::Node HelperAddNotestoIsland(Pointer<Music> M,
  Music::Node Island, Ratio NoteValue, List<mica::Concept> HarmonizedPitches);
bool MultipleVoiceCheck(Music::Node Root);
void RemoveAllVoices(Music& M, Music::Node Root);
void TieConnectNotes(Music& M, Music::Node Chord2, Music::Node Chord1);
void VoiceConnect(Music& M, List<Music::Node> HarmonizedChords,
  Music::Node ChordToken);
void VoiceTogetherSingleVoice(Music& M, Music::Node Root);

#ifdef BELLE_IMPLEMENTATION

///Given an island, find the chord with the longest duration.
Music::ConstNode FindLongestNoteDurationInIsland(Music::ConstNode Island)
{
  if(not IsIsland(Island)) return Music::ConstNode();
  Array<Music::ConstNode> IslandChords =
    Island->Children(MusicLabel(mica::Token));
  Ratio MaximumNoteValue = -1;
  count Counter = -1;

  for(count i = 0; i < IslandChords.n(); i++)
  {
    if(not IsChord(IslandChords[i])) return Music::ConstNode();
    if(RhythmicDurationOfChord(IslandChords[i]) > MaximumNoteValue)
    {
      MaximumNoteValue = RhythmicDurationOfChord(IslandChords[i]);
      Counter = i;
    }
  }
  if(Counter == -1) return Music::ConstNode();
  return IslandChords[Counter];
}

/**Return the parsed geomtery of the music graph. Also initialize the
node matrix, rhythm matrix and node-to-index symbol table.*/
Pointer<Geometry> GetRhythmicOnsetInfo(Pointer<Music> M,
  List<Array<Music::ConstNode> >& NodeMatrix,
  Matrix<Ratio>& RhythmMatrix,
  Tree<Music::ConstNode, VectorInt>& NodeToIndexLookup)
{
  NodeMatrix.RemoveAll();
  RhythmMatrix.Clear();
  NodeToIndexLookup.RemoveAll();
  Array<Music::Node> AllNodes = M->Nodes();
  for(count i = 0; i < AllNodes.n(); i++)
    AllNodes[i]->Label.SetState().Clear();
  if(not M or not M->Root())
    return Pointer<Geometry>();
  Pointer<Geometry> G = System::MutableGeometry(M);
  if(not G)
    return Pointer<Geometry>();
  G->Parse(*M);
  if(not G->GetNumberOfParts() or not G->GetNumberOfInstants())
    return Pointer<Geometry>();
  IslandState::Accumulate(M);
  AccumulatePartState(M);
  InstantState::Accumulate(M);
  Rhythm::CreateRhythmOrderedRegion(G, NodeMatrix);
  RhythmMatrix = Rhythm::GetUnpackedRhythmicOnsets(NodeMatrix);
  for(count i = 0; i < NodeMatrix.n(); i++)
    for(count j = 0; j < NodeMatrix[i].n(); j++)
      NodeToIndexLookup.Set(NodeMatrix[i][j]) =
        VectorInt(integer(i), integer(j));

  return G;
}

/**Harmonizes the specified staff with the given pitches and rhythmic
sequence.*/
void Harmonize(Pointer<Music> M,
  List<List<mica::Concept> >HarmonizedPitches, List<Ratio> NoteValue,
  count StaffNumber)
{
  if(not M) return;
  if(HarmonizedPitches.n() == 0) return;
  if(NoteValue.n() == 0) return;
  if(NoteValue.n() == 1)
  {
    Ratio OnlyNoteValue = NoteValue.a();
    for(count i = 1; i < HarmonizedPitches.n(); i++)
      NoteValue.Push(OnlyNoteValue);
  }

  Pointer<Geometry> G = System::MutableGeometry(M);
  G->Parse(*M);
  Array<Music::ConstNode> IslandBeginnings = G->GetPartBeginnings();
  if(StaffNumber < 1 or StaffNumber > IslandBeginnings.n())
    return;
  count NewJ = StaffNumber - 1;
  Music::Node BeginningIsland = M->Promote(IslandBeginnings[NewJ]);

  List<Array<Music::ConstNode> > NodeMatrix;
  Matrix<Ratio> RhythmMatrix;
  Tree<Music::ConstNode, VectorInt> NodeToIndexLookup;
  List<Music::Node> HarmonizedChords;
  Ratio BeatsAdded = 0;
  Ratio PreviousBarOnset = 0;
  Ratio CurrentBarOnset = 0;
  Ratio CumulativeRhythm = Ratio(0,0);
  Music::Node PreviousChord = Music::Node();
  Music::Node PreviousHarmonizedChord = Music::Node();
  mica::Concept TokenKind;
  count ChordCount = 0;
  count RhythmCount = 0;

  GetRhythmicOnsetInfo(M, NodeMatrix, RhythmMatrix, NodeToIndexLookup);
  VoiceTogetherSingleVoice(*M, BeginningIsland);

  for(count i = 0; i < NodeMatrix.n(); i++)
  {
    if (NodeMatrix[i][NewJ])
    {
      Array<Music::ConstNode> Tokens =
        NodeMatrix[i][NewJ]->Children(MusicLabel(mica::Token));
      if(Tokens.n() > 0)
        TokenKind = Tokens.a()->Get(mica::Kind);
    }

    else continue;

    if(TokenKind == mica::Barline and PreviousChord)
      CumulativeRhythm += RhythmicDurationOfChord(PreviousChord);

    else if (RhythmMatrix(NewJ, i).IsDeterminate())
    {
      PreviousChord =
        M->Promote(FindLongestNoteDurationInIsland(NodeMatrix[i][NewJ]));
      CumulativeRhythm = RhythmMatrix(NewJ, i);
    }

    /*If the music is on the last island in the system or in a bar with no
    musical content then make sure the previous measure is terminated
    correctly. i.e.the last note is changed to a note value equal to the number
    of remaining beats in the measure.*/
    if((!NodeMatrix[i][NewJ]->Next(MusicLabel(mica::Partwise)) or
    (PreviousIslandIsBarline(NodeMatrix[i][NewJ]) and
      TokenKind == mica::Barline))
      and CumulativeRhythm.IsDeterminate() and PreviousHarmonizedChord)
    {
      PreviousBarOnset = CurrentBarOnset;

      if(CumulativeRhythm - PreviousBarOnset < BeatsAdded)
      {
        Ratio Remainder = BeatsAdded -
          (CumulativeRhythm - PreviousBarOnset);

        Ratio PreviousNoteValue =
          RhythmicDurationOfChord(PreviousHarmonizedChord);

        M->Promote(PreviousHarmonizedChord)->Set(mica::NoteValue) =
          mica::Concept(PreviousNoteValue - Remainder);
      }
    }

    if(PreviousIslandIsBarline(NodeMatrix[i][NewJ]) and
      TokenKind != mica::Barline)
    {
      PreviousBarOnset = CurrentBarOnset;
      if (RhythmMatrix(NewJ, i).IsDeterminate())
        CurrentBarOnset = RhythmMatrix(NewJ, i);

      /*If the number of beats added in the previous measure exceeds
      the total number of beats in that measure, then there is a carry
      over into the current measure.*/
      if(CurrentBarOnset - PreviousBarOnset < BeatsAdded
        and PreviousHarmonizedChord)
      {
        HarmonizedChords.RemoveAll();

        Ratio Remainder = BeatsAdded -
          (CurrentBarOnset - PreviousBarOnset);

        count OldChordCount = ChordCount;
        if(--ChordCount < 0) ChordCount = HarmonizedPitches.n() - 1;
        Music::Node ChordToken = HelperAddNotestoIsland(M,
          M->Promote(NodeMatrix[i][NewJ]), Remainder,
            HarmonizedPitches[ChordCount]);
        ChordCount = OldChordCount;

        VoiceConnect(*M, HarmonizedChords, ChordToken);
        HarmonizedChords.Push(ChordToken);

        mica::Concept PreviousNoteValue =
          PreviousHarmonizedChord->Get(mica::NoteValue);

        M->Promote(PreviousHarmonizedChord)->Set(mica::NoteValue) =
          mica::Concept(Ratio(PreviousNoteValue) - Remainder);

        TieConnectNotes(*M, PreviousHarmonizedChord, ChordToken);
        PreviousHarmonizedChord = ChordToken;

        BeatsAdded = Remainder;
      }

       else BeatsAdded = 0;
    }

    /*Upon encountering an empty measure, treat the music as if it is just
    beginning.*/
    if(PreviousIslandIsBarline(NodeMatrix[i][NewJ]) and
      TokenKind == mica::Barline)
    {
      HarmonizedChords.RemoveAll();
      BeatsAdded = 0;
      CumulativeRhythm = Ratio(0,0);
      PreviousChord = Music::Node();
      PreviousHarmonizedChord = Music::Node();
      continue;
    }

    if(CumulativeRhythm.IsDeterminate())
    {
      /*If the rhythmic onset of the current chord relative to the
      beginning of the measure equals the number of beats added
      to that measure, then the next harmonized chord belongs to the current
      island.*/
      if(CumulativeRhythm - CurrentBarOnset == BeatsAdded)
      {
        /*Voice the harmonized chords together by barline and do not
        add the harmonized chord to the barline island.*/
        if(TokenKind == mica::Barline)
        {
          if(HarmonizedChords.n())
            PreviousHarmonizedChord = HarmonizedChords.z();
          HarmonizedChords.RemoveAll();
          continue;
        }

        Music::Node ChordToken = HelperAddNotestoIsland(M,
          M->Promote(NodeMatrix[i][NewJ]), NoteValue[RhythmCount],
          HarmonizedPitches[ChordCount]);

        if(++ChordCount == HarmonizedPitches.n())
          ChordCount = 0;

        VoiceConnect(*M, HarmonizedChords, ChordToken);
        HarmonizedChords.Push(ChordToken);

        PreviousHarmonizedChord = ChordToken;

        BeatsAdded += NoteValue[RhythmCount];
        if(++RhythmCount == NoteValue.n())
          RhythmCount = 0;
      }

      /*If the rhythmic onset of the current chord relative to the
      beginning of the measure is greater than the number of beats added
      to that measure then the next harmonized chord must lie in between the
      current island and the previous island in the graph.*/
      else if(CumulativeRhythm - CurrentBarOnset > BeatsAdded)
      {
        Music::Node Island =
          HelperAddIslandBefore(*M, M->Promote(NodeMatrix[i][NewJ]));

        Music::Node ChordToken = HelperAddNotestoIsland(M,
          Island, NoteValue[RhythmCount], HarmonizedPitches[ChordCount]);
        if(++ChordCount == HarmonizedPitches.n())
          ChordCount = 0;

        VoiceConnect(*M, HarmonizedChords, ChordToken);
        HarmonizedChords.Push(ChordToken);

        PreviousHarmonizedChord = ChordToken;

        BeatsAdded += NoteValue[RhythmCount];
        if(++RhythmCount == NoteValue.n())
          RhythmCount = 0;

        /*Even though there exists a previous chord, trick the program into
        thinking there is no previous chord so that at the next iteration
        cumulative rhythm is not updated.*/
        if(TokenKind == mica::Barline) PreviousChord = Music::Node();
        i--;
      }
    }
  }
  MusicXMLBeaming(M);
}

///Adds a chord with the specified note value onto the given island.
Music::Node HelperAddChordToIsland(Music& M, Music::Node Island,
  Ratio NoteValue)
{
  if(not IsIsland(Island)) return Music::Node();
    Music::Node ChordToken = M.Add();
  ChordToken->Set(mica::Type) = mica::Token;
  ChordToken->Set(mica::Kind) = mica::Chord;
  ChordToken->Set(mica::NoteValue) = mica::Concept(NoteValue);
  M.Connect(Island, ChordToken)->Set(mica::Type) = mica::Token;
  return ChordToken;
}

///Adds an island before the specified island.
Music::Node HelperAddIslandBefore(Music& M, Music::Node IslandAfter)
{
  if(not IsIsland(IslandAfter)) return Music::Node();
  Music::Node IslandBefore = IslandAfter->Previous(MusicLabel(mica::Partwise));
  Music::Node IslandBetween;
  if(IslandBefore)
  {
    IslandBetween = M.Add();
    IslandBetween->Set(mica::Type) = mica::Island;
    IslandBetween->Set("StaffConnects") = "true";
    M.Disconnect(IslandAfter->Previous(MusicLabel(mica::Partwise), true));
    M.Connect(IslandBefore, IslandBetween)->Set(mica::Type) = mica::Partwise;
    M.Connect(IslandBetween, IslandAfter)->Set(mica::Type) = mica::Partwise;
  }

  return IslandBetween;
}

///Adds an island beteween the specified islands.
Music::Node HelperAddIslandBetween(Music& M, Music::Node IslandAfter,
  Music::Node IslandBefore)
{
  if(not IsIsland(IslandAfter)) return Music::Node();
  if(not IsIsland(IslandBefore)) return Music::Node();
  Music::Node IslandBetween;
  if(IslandBefore)
  {
    IslandBetween = M.Add();
    IslandBetween->Set(mica::Type) = mica::Island;
    IslandBetween->Set("StaffConnects") = "true";
    M.Disconnect(IslandAfter->Previous(MusicLabel(mica::Partwise), true));
    M.Connect(IslandBefore, IslandBetween)->Set(mica::Type) = mica::Partwise;
    M.Connect(IslandBetween, IslandAfter)->Set(mica::Type) = mica::Partwise;
  }

  return IslandBetween;
}

/**Adds an island after the specified island but assumes that the added island
will be the last island in the graph.*/
Music::Node HelperAddIslandToEnd(Music& M, Music::Node IslandBefore)
{
  if(not IsIsland(IslandBefore)) return Music::Node();
  Music::Node IslandEnd;
  IslandEnd = M.Add();
  IslandEnd->Set(mica::Type) = mica::Island;
  IslandEnd->Set("StaffConnects") = "true";
  M.Connect(IslandBefore, IslandEnd)->Set(mica::Type) = mica::Partwise;

  return IslandEnd;
}

/**Adds a chord with the given note value to the specified island. Then adds the
specified notes to this chord.*/
Music::Node HelperAddNotestoIsland(Pointer<Music> M,
  Music::Node Island, Ratio NoteValue, List<mica::Concept> HarmonizedPitches)
{
  Music::Node ChordToken =
    HelperAddChordToIsland(*M, Island, NoteValue);

  for(count j = 0; j < HarmonizedPitches.n(); j++)
    HelperAddNoteToChord(*M, ChordToken, HarmonizedPitches[j]);

  return ChordToken;
}

/**Checks to see if there is a at least one island with multiple chord tokens
after the given root node.*/
bool MultipleVoiceCheck(Music::Node Root)
{
  Music::Node x = Root;
  for(; x; x = x->Next(MusicLabel(mica::Partwise)))
    if(x->Children(MusicLabel(mica::Token)).n() > 1) return true;

  return false;
}

///Removes all voice edges starting from the given root node.
void RemoveAllVoices(Music& M, Music::Node Root)
{
  Music::Node Island = Root;
  for(; Island; Island = Island->Next(MusicLabel(mica::Partwise)))
  {
    Music::Node ChordToken = Island->Next(MusicLabel(mica::Token));
    if(not IsChord(ChordToken)) continue;
    M.Disconnect(ChordToken->Next(MusicLabel(mica::Voice), true));
  }
}

///Connects all possible notes between the two given chords.
void TieConnectNotes(Music& M, Music::Node Chord1, Music::Node Chord2)
{
  if(not IsChord(Chord1) or not IsChord(Chord2)) return;
  Array<Music::Node> Children1 = Chord1->Children(MusicLabel(mica::Note));
  Array<Music::Node> Children2 = Chord2->Children(MusicLabel(mica::Note));
  for(count i = 0; i < Children1.n(); i++)
    for(count j = 0; j < Children2.n(); j++)
      if(Children1[i]->Get(mica::Value) == Children2[j]->Get(mica::Value))
        M.Connect(Children1[i], Children2[j])->Set(mica::Type) = mica::Tie;
}

///Connects the last chord in the list of chords with the specified chord.
void VoiceConnect(Music& M, List<Music::Node> HarmonizedChords,
  Music::Node ChordToken)
{
  if(!HarmonizedChords.n() or not ChordToken) return;
  M.Connect(HarmonizedChords.z(), ChordToken)->Set(mica::Type) = mica::Voice;
}

///Voice connects all single voice sections starting from the given root node.
void VoiceTogetherSingleVoice(Music& M, Music::Node Root)
{
  Music::Node Island = Root;
  (void) M;
  for(; Island; Island = Island->Next(MusicLabel(mica::Partwise)))
  {
    Music::Node ChordToken = Island->Next(MusicLabel(mica::Token));
    Music::Node NextChordToken = NextChordExistIncludingRest(Island);

    if(not IsChord(ChordToken) or
      not IsChord(NextChordToken) or
      ChordToken->Next(MusicLabel(mica::Voice)) or
      NextChordToken->Previous(MusicLabel(mica::Voice)) or
      ChordToken->Next(MusicLabel(mica::Beam)) or
      NextChordToken->Previous(MusicLabel(mica::Beam)))
      continue;

    M.Connect(ChordToken, NextChordToken)->Set(mica::Type) = mica::Voice;
  }
}

#endif
///@}
