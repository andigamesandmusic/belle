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
@name Selection

Functions that return particular regions in the graph.
@{
*/
List<List<Music::Node> > BarlineAddress(Pointer<Music> M,
 count BeginningBarline, count EndingBarline);
Music::Node FindFirstInstantConnection(Music::Node Island);
void InitializeSelectedPassage(Music& M, Music::Node Beginning, Music::Node End,
  Music::Node& Root, Music::Node& NextEnd);
bool IsValidSelectedPassage(Music& M, Music::Node Beginning, Music::Node End);
List<List<Music::Node> > SystemTimeSignatureRanges(Pointer<Music> M,
  Music::ConstNode BeginningIsland);

#ifdef BELLE_IMPLEMENTATION

/**Returns the pair of memory addresses for the beginning and ending barline
numbers. The code assumes 0 indexing. Only return an empty list if
the specified beginning barline is greater than the ending barline. If
either BeginningBarline or EndingBarline does not belong to the graph then
return a list of containing first token on every system and a null reference.*/
List<List<Music::Node> >BarlineAddress(Pointer<Music> M,
  count BeginningBarline, count EndingBarline)
{
  if(BeginningBarline > EndingBarline) return List<List<Music::Node> >();
  Pointer<Geometry> G = System::MutableGeometry(M);
  G->Parse(*M);
  Array<Music::ConstNode> IslandBeginnings = G->GetPartBeginnings();
  List<List<Music::Node> > BarlinePairs;

  for(count i = 0; i < IslandBeginnings.n(); i++)
  {
    count BarlineCounter = 0;
    BarlinePairs.Push(List<Music::Node>());
    Music::ConstNode FirstToken;
    count TokenCount = 0;

    for(Music::ConstNode x = IslandBeginnings[i]; x;
        x = x->Next(MusicLabel(mica::Partwise)))
    {
      Music::ConstNode CurrentToken;
      if((CurrentToken = x->Next(MusicLabel(mica::Token))))
      {
        if(TokenCount == 0) FirstToken = CurrentToken;
        TokenCount++;
        if(CurrentToken->Get(mica::Kind) == mica::Barline)
        {
          if(BarlineCounter == BeginningBarline)
            BarlinePairs.z().Push(M->Promote(CurrentToken));

          if(BarlineCounter == EndingBarline)
          {
            BarlinePairs.z().Push(M->Promote(CurrentToken));
            break;
          }
          BarlineCounter++;
        }
      }
    }
    for(count j = 0; j < BarlinePairs.n(); j++)
    {
      if(BarlinePairs[j].n() < 2)
      {
        BarlinePairs[j].RemoveAll();
        BarlinePairs[j].Push(M->Promote(FirstToken));
        BarlinePairs[j].Push(Music::Node());
      }
    }
  }
  return BarlinePairs;
}

///Finds the first instant connection from the given island.
Music::Node FindFirstInstantConnection(Music::Node Island)
{
  while(Island)
  {
    if(Island->Next(MusicLabel(mica::Instantwise), true))
      return Island->Next(MusicLabel(mica::Instantwise));
    Island = Island->Next(MusicLabel(mica::Partwise));
  }

  return Music::Node();
}

/**Sets Root equal to the specified Beginning. Sets NextEnd to the node after
End.*/
void InitializeSelectedPassage(Music& M, Music::Node Beginning, Music::Node End,
  Music::Node& Root, Music::Node& NextEnd)
{
  if(Beginning) Root = Beginning;
  else Root = M.Root();

  if(End) NextEnd = End->Next(MusicLabel(mica::Partwise));
  else NextEnd = Music::Node();
}

///A selected passage is valid if both Beginning and End belong to the graph.
bool IsValidSelectedPassage(Music& M, Music::Node Beginning, Music::Node End)
{
  bool ValidBeginning = !Beginning or M.Belongs(Beginning);
  bool ValidEnd = !End or M.Belongs(End);

  return ValidBeginning and ValidEnd;
}

/**Returns a list of memory address pairs. Each pair contains the nodes
corresponding to a time signature and the next time signature. If
only one time signature in the system exists, then the pair will contain the
address of that time signature and a null reference.*/
List<List<Music::Node> > SystemTimeSignatureRanges(Pointer<Music> M,
  Music::ConstNode BeginningIsland)
{
  List<List<Music::Node> > TimeSignatureRanges;
  for(Music::ConstNode x = BeginningIsland; x;
    x = x->Next(MusicLabel(mica::Partwise)))
  {
    Music::ConstNode CurrentToken;
    if((CurrentToken = x->Next(MusicLabel(mica::Token))))
      if(CurrentToken->Get(mica::Kind) == mica::TimeSignature)
      {
        if(TimeSignatureRanges.n() != 0)
          TimeSignatureRanges.z().Push(M->Promote(CurrentToken));

        TimeSignatureRanges.Push(List<Music::Node>());
        TimeSignatureRanges.z().Push(M->Promote(CurrentToken));
      }
  }
  if(TimeSignatureRanges.n() > 0 and TimeSignatureRanges.z().n() == 1)
    TimeSignatureRanges.z().Push(Music::Node());

  return TimeSignatureRanges;
}

#endif
///@}
