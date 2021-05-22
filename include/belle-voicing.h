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

///@name Voicing
///@{

///Returns whether two chords parent islands are connected in order partwise.
bool AreChordsOrderedPartwise(Music::ConstNode BeforeChord,
  Music::ConstNode AfterChord);

///Returns whether two chords parent islands are connected in order partwise.
bool AreChordsOrderedInstantwise(Music::ConstNode BeforeChord,
  Music::ConstNode AfterChord);

///Gets the next chord following a beam link.
Music::ConstNode NextChordByBeam(Music::ConstNode x);

///Gets the previous chord following a beam link.
Music::ConstNode PreviousChordByBeam(Music::ConstNode x);

///Gets the next chord by voice, following either a voice or beam link.
Music::ConstNode NextChordByVoice(Music::ConstNode x);

///Gets the previous chord by voice, following either a voice or beam link.
Music::ConstNode PreviousChordByVoice(Music::ConstNode x);

///Returns whether the chord is part of a voice strand.
bool IsChordInVoiceStrand(Music::ConstNode x);

///Returns whether the chord begins a voice.
bool ChordBeginsVoice(Music::ConstNode x);

///Returns whether the chord ends a voice.
bool ChordEndsVoice(Music::ConstNode x);

///Finds the first chord of the voice strand.
Music::ConstNode FirstChordOfVoiceStrand(Music::ConstNode x);

///Finds the last chord of the voice strand.
Music::ConstNode LastChordOfVoiceStrand(Music::ConstNode x);

///Collects all the chords in a voice strand.
Array<Music::ConstNode> ChordsInVoiceStrand(Music::ConstNode x);

/**Counts the number of voices in the island. Does not count voices that are
passing through.*/
count NumberOfVoicesInIsland(Music::ConstNode x);

///Gets an array of the voices that are beginning in the given island.
Array<Music::ConstNode> ChordsBeginningVoicesInIsland(Music::ConstNode x);

///Returns whether all of chords in the island are beginning a voice strand.
bool IslandBeginsMultivoiceRegion(Music::ConstNode x);

///Gets an array of the last chords of a voice strand given an array of chords.
Array<Music::ConstNode> LastChordsOfVoiceStrand(
  const Array<Music::ConstNode>& Chords);

///Finds all the voice strands from a common starting point.
void FindAllVoiceStrands(Music::ConstNode StartPoint,
  List<Array<Music::ConstNode> >& VoiceStrands);

///Returns the voice strands as a single value.
Value VoiceStrandsAsValue(const List<Array<Music::ConstNode> >& VoiceStrands);

///Returns the first instant in the voice strands.
count FirstInstantInVoiceStrands(
  const List<Array<Music::ConstNode> >& VoiceStrands);

///Returns the last instant in the voice strands.
count LastInstantInVoiceStrands(
  const List<Array<Music::ConstNode> >& VoiceStrands);

///@}

namespace internals
{
  ///Returns whether the strand indicated by its first chord is unique.
  bool VoiceStrandIsUnique(Music::ConstNode FirstChordOfStrand,
    List<Array<Music::ConstNode> >& VoiceStrands);

  ///Searches the strand of chords for new strands.
  void SearchStrandForNewVoices(const Array<Music::ConstNode>& Strand,
    List<Array<Music::ConstNode> >& VoiceStrands);

  ///Adds any new voice strands off of a given island.
  void AddNewVoiceStrands(Music::ConstNode IslandToConsume,
    List<Array<Music::ConstNode> >& VoiceStrands);

  ///Finds all the voice strands off an array of beginning strands.
  void FindAllVoiceStrandsInChords(
    const Array<Music::ConstNode>& BeginningVoices,
    List<Array<Music::ConstNode> >& VoiceStrands);
}

#ifdef BELLE_IMPLEMENTATION
bool AreChordsOrderedPartwise(Music::ConstNode BeforeChord,
  Music::ConstNode AfterChord)
{
  return IsChord(BeforeChord) and IsChord(AfterChord) and
    IslandOfToken(BeforeChord)->Label.GetState("PartID").AsCount() <
    IslandOfToken(AfterChord)->Label.GetState("PartID").AsCount();
}

bool AreChordsOrderedInstantwise(Music::ConstNode BeforeChord,
  Music::ConstNode AfterChord)
{
  return IsChord(BeforeChord) and IsChord(AfterChord) and
    IslandOfToken(BeforeChord)->Label.GetState("InstantID").AsCount() <
    IslandOfToken(AfterChord)->Label.GetState("InstantID").AsCount();
}

Music::ConstNode NextChordByBeam(Music::ConstNode x)
{
  Music::ConstNode y = IsChord(x) ? x->Next(MusicLabel(mica::Beam)) :
    Music::ConstNode();
  return y;
  //return AreChordsOrderedPartwise(x, y) ? y : Music::ConstNode();
}

Music::ConstNode PreviousChordByBeam(Music::ConstNode x)
{
  Music::ConstNode y = IsChord(x) ? x->Previous(MusicLabel(mica::Beam)) :
    Music::ConstNode();
  return y;
  //return AreChordsOrderedPartwise(y, x) ? y : Music::ConstNode();
}

Music::ConstNode NextChordByVoice(Music::ConstNode x)
{
  Music::ConstNode Result;
  if(IsChord(x))
  {
    if(Music::ConstNode NextByVoice = x->Next(MusicLabel(mica::Voice)))
      Result = NextByVoice;
    else if(Music::ConstNode NextByBeam = NextChordByBeam(x))
      Result = NextByBeam;
  }
  return Result;
}

Music::ConstNode PreviousChordByVoice(Music::ConstNode x)
{
  Music::ConstNode Result;
  if(IsChord(x))
  {
    if(Music::ConstNode NextByVoice = x->Previous(MusicLabel(mica::Voice)))
      Result = NextByVoice;
    else if(Music::ConstNode NextByBeam = PreviousChordByBeam(x))
      Result = NextByBeam;
  }
  return Result;
}

bool IsChordInVoiceStrand(Music::ConstNode x)
{
  return NextChordByVoice(x) || PreviousChordByVoice(x);
}

bool ChordBeginsVoice(Music::ConstNode x)
{
  return IsChord(x) && !PreviousChordByVoice(x);
}

bool ChordEndsVoice(Music::ConstNode x)
{
  return IsChord(x) && !NextChordByVoice(x);
}

Music::ConstNode FirstChordOfVoiceStrand(Music::ConstNode x)
{
  x = IsChord(x) ? x : Music::ConstNode();
  while(Music::ConstNode Previous = PreviousChordByVoice(x))
    x = Previous;
  return x;
}

Music::ConstNode LastChordOfVoiceStrand(Music::ConstNode x)
{
  x = IsChord(x) ? x : Music::ConstNode();
  while(Music::ConstNode Next = NextChordByVoice(x))
    x = Next;
  return x;
}

Array<Music::ConstNode> ChordsInVoiceStrand(Music::ConstNode x)
{
  Array<Music::ConstNode> Collector;
  x = FirstChordOfVoiceStrand(x);
  while(x)
    Collector.Add() = x, x = NextChordByVoice(x);
  return Collector;
}

count NumberOfVoicesInIsland(Music::ConstNode x)
{
  count Voices = 0;
  if(IsIsland(x))
  {
    Array<Music::ConstNode> Chords = x->Children(MusicLabel(mica::Token));
    for(count i = 0; i < Chords.n(); i++)
      Voices += IsChord(Chords[i]);
  }
  return Voices;
}

Array<Music::ConstNode> ChordsBeginningVoicesInIsland(Music::ConstNode x)
{
  Array<Music::ConstNode> BeginningVoices;
  if(IsIsland(x))
  {
    Array<Music::ConstNode> Chords = x->Children(MusicLabel(mica::Token));
    for(count i = 0; i < Chords.n(); i++)
      if(ChordBeginsVoice(Chords[i]))
        BeginningVoices.Add() = Chords[i];
  }
  return BeginningVoices;
}

bool IslandBeginsMultivoiceRegion(Music::ConstNode x)
{
  bool IslandBeginsMultivoice = false;
  if(IsIsland(x))
  {
    Array<Music::ConstNode> Chords = x->Children(MusicLabel(mica::Token));
    IslandBeginsMultivoice = Chords.n();
    for(count i = 0; i < Chords.n(); i++)
      if(!ChordBeginsVoice(Chords[i]))
        IslandBeginsMultivoice = false;
  }
  return IslandBeginsMultivoice;
}

Array<Music::ConstNode> LastChordsOfVoiceStrand(
  const Array<Music::ConstNode>& Chords)
{
  Array<Music::ConstNode> EndingChords(Chords.n());
  for(count i = 0; i < Chords.n(); i++)
    EndingChords[i] = LastChordOfVoiceStrand(Chords[i]);
  return EndingChords;
}

namespace internals
{
  bool VoiceStrandIsUnique(Music::ConstNode FirstChordOfStrand,
    List<Array<Music::ConstNode> >& VoiceStrands)
  {
    bool StrandUnique = true;
    for(count i = 0; StrandUnique && i < VoiceStrands.n(); i++)
      if(VoiceStrands[i].a() == FirstChordOfStrand)
        StrandUnique = false;
    return StrandUnique;
  }

  void SearchStrandForNewVoices(const Array<Music::ConstNode>& Strand,
    List<Array<Music::ConstNode> >& VoiceStrands)
  {
    for(count i = 0; i < Strand.n(); i++)
      FindAllVoiceStrands(IslandOfToken(Strand[i]), VoiceStrands);
  }

  void AddNewVoiceStrands(Music::ConstNode IslandToConsume,
    List<Array<Music::ConstNode> >& VoiceStrands)
  {
    Array<Music::ConstNode> Strand = ChordsInVoiceStrand(IslandToConsume);
    if(VoiceStrandIsUnique(Strand.a(), VoiceStrands))
    {
      VoiceStrands.Add() = Strand;
      SearchStrandForNewVoices(Strand, VoiceStrands);
    }
  }

  void FindAllVoiceStrandsInChords(
    const Array<Music::ConstNode>& BeginningVoices,
    List<Array<Music::ConstNode> >& VoiceStrands)
  {
    for(count i = 0; i < BeginningVoices.n(); i++)
      AddNewVoiceStrands(BeginningVoices[i], VoiceStrands);
  }
}

void FindAllVoiceStrands(Music::ConstNode StartPoint,
  List<Array<Music::ConstNode> >& VoiceStrands)
{
  internals::FindAllVoiceStrandsInChords(
    ChordsBeginningVoicesInIsland(StartPoint), VoiceStrands);
}

Value VoiceStrandsAsValue(const List<Array<Music::ConstNode> >& VoiceStrands)
{
  Value v;
  for(count i = 0; i < VoiceStrands.n(); i++)
    for(count j = 0; j < VoiceStrands[i].n(); j++)
      v[i][j] = VoiceStrands[i][j];
  return v;
}

count FirstInstantInVoiceStrands(
  const List<Array<Music::ConstNode> >& VoiceStrands)
{
  count Index = -1;
  if(VoiceStrands.n() && VoiceStrands.a().n())
  {
    Index = VoiceStrands.a().a();
    for(count i = 1; i < VoiceStrands.n(); i++)
      Index = Min(Index, InstantIDOfIsland(IslandOfToken(VoiceStrands[i].a())));
  }
  return Index;
}

count LastInstantInVoiceStrands(
  const List<Array<Music::ConstNode> >& VoiceStrands)
{
  count Index = -1;
  if(VoiceStrands.n() && VoiceStrands.a().n())
  {
    Index = VoiceStrands.a().z();
    for(count i = 1; i < VoiceStrands.n(); i++)
      Index = Max(Index, InstantIDOfIsland(IslandOfToken(VoiceStrands[i].z())));
  }
  return Index;
}

#endif
