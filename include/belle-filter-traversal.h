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
@name Traversals

Functions that the filters use to access various tokens within the graph.
@{
*/
VectorInt FindFirstChordInBarline(Music::ConstNode Island,
  const List<Array<Music::ConstNode> >& NodeMatrix,
  const Tree<Music::ConstNode, VectorInt>& NodeToIndexLookup);
Music::ConstNode FindFirstPitch(Music::ConstNode Island);
Music::ConstNode FindTimeSignature(const Music& M);
Music::ConstNode GetBarlineBetweenIslands(const Music& M,
  Music::ConstNode Island1, Music::ConstNode Island2);
Music::ConstNode GetPreviousChordFromIsland(Music::ConstNode Island);
Music::Node NextChordExist(Music::Node Island);
Music::Node NextChordExistIncludingRest(Music::Node Island);
bool OnlyRestInBetweenChords(Music::ConstNode Chord1,
  Music::ConstNode Chord2);
Music::Node PreviousChordExist(Music::Node Island);
Music::Node PreviousChordExistIncludingRest(Music::Node Island);
Music::ConstNode PreviousIslandIsBarline(Music::ConstNode Island);

#ifdef BELLE_IMPLEMENTATION

/**Given an island in a measure, find the first chord in that measure and
return the indices associated with its corresponding island in the
Node matrix.*/
VectorInt FindFirstChordInBarline(Music::ConstNode Island,
  const List<Array<Music::ConstNode> >& NodeMatrix,
  const Tree<Music::ConstNode, VectorInt>& NodeToIndexLookup)
{
  VectorInt Index = NodeToIndexLookup.Get(Island);
  count i = count(Index.i());
  count j = count(Index.j());
  count FirstChordI = -1;

  for(; i >= 0 and NodeMatrix[i][j]; i--)
  {
    Array<Music::ConstNode> CurrentTokens =
      NodeMatrix[i][j]->Children(MusicLabel(mica::Token));
    Music::ConstNode CurrentToken = CurrentTokens.a();
    if(CurrentToken)
    {
      if(CurrentToken->Get(mica::Kind) == mica::Barline)
        break;

      if(IsChord(CurrentToken))
        FirstChordI = i;
    }
  }
  return VectorInt(integer(FirstChordI), integer(j));
}

/**Find the island corresponding to the first pitch starting from the given
 island.*/
Music::ConstNode FindFirstPitch(Music::ConstNode Island)
{
  bool Found = false;
  for(; !Found and Island; Island = Island->Next(MusicLabel(mica::Partwise)))
    if(Music::ConstNode y = Island->Next(MusicLabel(mica::Token)))
      Found = (y->Get(mica::Kind) == mica::Chord) and (!IsRest(y));
  if (Island)
    return Island->Previous(MusicLabel(mica::Partwise));
  else return Music::ConstNode();
}

///Find the first time signature in the music.
Music::ConstNode FindTimeSignature(const Music& M)
{
  Music::ConstNode Token = Music::ConstNode();
  for(Music::ConstNode x = M.Root(); x; x = x->Next(MusicLabel(mica::Partwise)))
    if((Token = x->Next(MusicLabel(mica::Token))))
      if(Token->Get(mica::Kind) == mica::TimeSignature)
        return Token;
  return Token;
}

///Get the barline node between the two islands.
Music::ConstNode GetBarlineBetweenIslands(const Music& M,
  Music::ConstNode Island1, Music::ConstNode Island2)
{
  if(not Island1) return Music::ConstNode();
  if(not Island2) return Music::ConstNode();

  Array<Music::ConstNode> IslandsFrom1 = M.Series(Island1,
    MusicLabel(mica::Partwise));
  if(not IslandsFrom1.Contains(Island2))
    return Music::ConstNode();

  Music::ConstNode x = Music::ConstNode();
  for(x = Island1; x != Island2; x = x->Next(MusicLabel(mica::Partwise)))
  {
    Music::ConstNode CurrentToken;
    if((CurrentToken = x->Next(MusicLabel(mica::Token))))
      if(CurrentToken->Get(mica::Kind) == mica::Barline)
        break;
  }

  if (x == Island2) return Music::ConstNode();
  return x;
}

///Traverse backwards from a given island until the graph hits a chord token.
Music::ConstNode GetPreviousChordFromIsland(Music::ConstNode IslandCurrent)
{
  Music::ConstNode Island = Music::ConstNode();
  if (!(Island = IslandCurrent->Previous(MusicLabel(mica::Partwise))))
    return Music::ConstNode();

  for(; Island; Island = Island->Previous(MusicLabel(mica::Partwise)))
  {
    Music::ConstNode TokenPrevious = Island->Next(MusicLabel(mica::Token));
    if(IsChord(TokenPrevious) and !IsRest(TokenPrevious)) return TokenPrevious;
  }

  return Music::ConstNode();
}

///Checks to see if there is a a chord containing notes directly afterwards.
Music::Node NextChordExist(Music::Node Island)
{
  Music::Node IslandNext = Island->Next(MusicLabel(mica::Partwise));
  if(!IslandNext or !IsIsland(IslandNext)) return Music::Node();

  Music::Node TokenNext = IslandNext->Next(MusicLabel(mica::Token));
  if(IsChord(TokenNext) and !IsRest(TokenNext)) return TokenNext;
  return Music::Node();
}

///Checks to see if there is a chord directly afterwards.
Music::Node NextChordExistIncludingRest(Music::Node Island)
{
  Music::Node IslandNext = Island->Next(MusicLabel(mica::Partwise));
  if(!IsIsland(IslandNext)) return Music::Node();

  Music::Node TokenNext = IslandNext->Next(MusicLabel(mica::Token));
  if(IsChord(TokenNext)) return TokenNext;
  return Music::Node();
}

/**Assumes the passage is already voice connected. Checks to see if there are
any rests between the given two chords. No rests will return true.*/
bool OnlyRestInBetweenChords(Music::ConstNode Chord1,
  Music::ConstNode Chord2)
{
  if(not (IsChord(Chord1) and IsChord(Chord2))) return false;
  Array<Music::ConstNode> ChordsFrom1 =
    Chord1->Series(MusicLabel(mica::Voice), false);
  if(not ChordsFrom1.Contains(Chord2)) return false;

  for(count i = 1; i < ChordsFrom1.n(); i++)
   {
     if(ChordsFrom1[i] == Chord2) break;
     if(not IsRest(ChordsFrom1[i])) return false;
   }
  return true;
}

/**Checks to see if there is a chord containing notes directly before the
current chord.*/
Music::Node PreviousChordExist(Music::Node Island)
{
  Music::Node IslandPrevious = Island->Previous(MusicLabel(mica::Partwise));
  if(!IslandPrevious or !IsIsland(IslandPrevious)) return Music::Node();

  Music::Node TokenPrevious = IslandPrevious->Next(MusicLabel(mica::Token));
  if(IsChord(TokenPrevious) and !IsRest(TokenPrevious)) return TokenPrevious;
  return Music::Node();
}

///Checks to see if there is a a chord directly before the current chord.
Music::Node PreviousChordExistIncludingRest(Music::Node Island)
{
  Music::Node IslandPrevious = Island->Previous(MusicLabel(mica::Partwise));
  if(!IslandPrevious or !IsIsland(IslandPrevious)) return Music::Node();

  Music::Node TokenPrevious = IslandPrevious->Next(MusicLabel(mica::Token));
  if(IsChord(TokenPrevious)) return TokenPrevious;
  return Music::Node();
}

/**Checks to see if the island directly before the given island contains a
barline token.*/
Music::ConstNode PreviousIslandIsBarline(Music::ConstNode Island)
{
  if(IsIsland(Island))
    if(Music::ConstNode IslandPrevious =
      Island->Previous(MusicLabel(mica::Partwise)))
        if(Music::ConstNode TokenPrevious =
          IslandPrevious->Next(MusicLabel(mica::Token)))
            if(TokenPrevious->Get(mica::Kind) == mica::Barline)
              return TokenPrevious;
  return Music::ConstNode();
}
#endif
///@}
