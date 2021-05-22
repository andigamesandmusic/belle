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
@name Pitch-collapse

Functions that help the pitch-collapse filter.
@{
*/
List<mica::Concept> AssumeAndGetPitchesOnIsland(Music::Node Island);
void ChordPitchCollapse(Music& M, Music::Node ChordToken,
  List<mica::Concept> FirstPitches);
void ChordSlurRemoval(Music& M, Music::Node ChordToken);
Music::Node HelperAddNoteToChord(Music& M, Music::Node ChordToken,
  mica::Concept Pitch);
void PitchCollapse(Music& M, bool Staff2, Music::Node Beginning,
  Music::Node End);
void RemoveAllNotes(Music& M, Music::Node ChordToken);
List<Music::Node> TieCheck(Music::Node ChordToken);
void TiePitchCollapse(Music& M, List<mica::Concept> FirstPitches,
  List<Music::Node>& TieList);

#ifdef BELLE_IMPLEMENTATION
List<mica::Concept> AssumeAndGetPitchesOnIsland(Music::Node Island)
{
  List<mica::Concept> FirstPitches;
  Music::Node ChordToken = Island->Next(MusicLabel(mica::Token));
  Array<Music::Node> Z = ChordToken->Children(MusicLabel(mica::Note));
  for(count i = 0; i < Z.n(); i++)
    FirstPitches.Push(Z[i]->Get(mica::Value));
  return FirstPitches;
}

///Pitch collapse the chord token to the pitches specified by FirstPitches.
void ChordPitchCollapse(Music& M, Music::Node ChordToken,
  List<mica::Concept> FirstPitches)
{
  //Disconnect any slurs.
  ChordSlurRemoval(M, ChordToken);

  RemoveAllNotes(M, ChordToken);

  //Remake the chord from scratch.
  for(count i = 0; i < FirstPitches.n(); i++)
    HelperAddNoteToChord(M, ChordToken, FirstPitches[i]);
}

///Make sure to remove any articulation markings.
void ChordSlurRemoval(Music& M, Music::Node ChordToken)
{
  M.Disconnect(ChordToken->Next(MusicLabel(mica::Slur), true));
}

///Adds the pitch to the chord token.
Music::Node HelperAddNoteToChord(Music& M, Music::Node ChordToken,
  mica::Concept Pitch)
{
  Music::Node Note = M.Add();
  Note->Set(mica::Type) = mica::Note;
  Note->Set(mica::Value) = Pitch;
  M.Connect(ChordToken, Note)->Set(mica::Type) = mica::Note;
  return Note;
}

/**Pitch collapses the music graph. If Staff2 = true, pitch collapse the second
staff. If Beginning is an empty node, then start from the beginning. If
End is an empty node, then finish at the end. */
void PitchCollapse(Music& M, bool Staff2, Music::Node Beginning,
  Music::Node End)
{
  Music::Node NewRoot;
  Music::Node Root;
  Music::Node x = Music::Node();
  Music::Node NextEnd;

  if(!IsValidSelectedPassage(M, Beginning, End)) return;
  InitializeSelectedPassage(M, Beginning, End, Root, NextEnd);

  if(Staff2)
  {
    NewRoot = FindFirstInstantConnection(Root);
    if(NewRoot) x = M.Promote(FindFirstPitch(NewRoot));
  }

  else
    x = M.Promote(FindFirstPitch(Root));

  //If first pitch not found, no point in pitch collapsing.
  if(!x) return;

  List<mica::Concept> FirstPitches = AssumeAndGetPitchesOnIsland(x);

  //Pitch collapse the rest of the notes to that first pitch level.
  while(x and x != NextEnd)
  {
    Music::Node y = x->Next(MusicLabel(mica::Token));

    if(IsChord(y) and !IsRest(y))
    {
      List<Music::Node> TieList = TieCheck(y);
      if(TieList.n())
      {
        TiePitchCollapse(M, FirstPitches, TieList);
        x = (TieList.z())->Previous(MusicLabel(mica::Token));
      }
      //If there is no tie sequence simply pitch collapse the current note.
      else
        ChordPitchCollapse(M, y, FirstPitches);
    }

    x = x->Next(MusicLabel(mica::Partwise));

  }
}

///Removes all the notes from the given chord token.
void RemoveAllNotes(Music& M, Music::Node ChordToken)
{
  if(IsChord(ChordToken))
  {
    //Make the chord into a rest first.
    Array<Music::Node> Children = ChordToken->Children(MusicLabel(mica::Note));
    for(count i = 0; i < Children.n(); i++)
      M.Remove(Children[i]);
  }
}

/**Checks to see if there is a tie sequence starting from the chord specified
by chord notes. Returns the list of chords in the tie sequence, if such
sequence exists. If there is no sequence, return an empty list.*/
List<Music::Node> TieCheck(Music::Node ChordToken)
{
  Array<Music::Node> ChordNotes =
    ChordToken->Children(MusicLabel(mica::Note));
  count NumberNotes = ChordNotes.n();
  List<Music::Node> ListChord;

  //Get the tie sequence for each note.
  Array<Array<Music::Node> > TieSequence(NumberNotes);
  for(count i = 0; i < NumberNotes; i++)
    TieSequence[i] = ChordNotes[i]->Series(MusicLabel(mica::Tie), false);

  //Find the shortest tie sequence.
  count Small = 1;
  count SmallIndex = 0;
  for(count i = 0; i < NumberNotes; i++)
    if(!i or TieSequence[i].n() < Small)
      Small = TieSequence[i].n(),
      SmallIndex = i;

  /*If one of the notes in the chord does not have a tie, then the chord cannot
  be in a tie sequence.*/
  if(Small == 1) return List<Music::Node>();

  /*For this shortest sequence, find the number of notes per chord in that
  tie sequence. If this number is the same, then it is a tie sequence.*/
  Array<Music::Node> SmallSequence = TieSequence[SmallIndex];
  Array<count> ChordSizes(Small);

  //Find the chord sizes.
  for(count i = 0; i < Small; i++)
  {
    Music::Node ChordPrev =
      (SmallSequence[i])->Previous(MusicLabel(mica::Note));
    ListChord.Push(ChordPrev);
    ChordSizes[i] = (ChordPrev->Children(MusicLabel(mica::Note))).n();
    if(ChordSizes[i] != ChordSizes.a()) return List<Music::Node>();
  }

  //If all the tests above pass, then the tie sequence is proper.
  return ListChord;
}

/**Pitch collapses every chord in the tie sequence given by the tie list to the
pitches specified by FirstPitches.*/
void TiePitchCollapse(Music& M, List<mica::Concept> FirstPitches,
  List<Music::Node>& TieList)
{
  for(count i = 0; i < TieList.n(); i++)
    ChordPitchCollapse(M, TieList[i], FirstPitches);

  for(count i = 1; i < TieList.n(); i++)
  {
    Array<Music::Node> z = TieList[i]->Children(MusicLabel(mica::Note));
    Array<Music::Node> zPrevious =
      TieList[i - 1]->Children(MusicLabel(mica::Note));

    for(count j = 0; j < FirstPitches.n(); j++)
        for(count k = 0; k < FirstPitches.n(); k++)
          if((zPrevious[j]->Get(mica::Value)) == (z[k]->Get(mica::Value)))
            M.Connect(zPrevious[j], z[k])->Set(mica::Type) = mica::Tie;
  }
}
#endif
///@}
