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
@name Search

Functions that assist in score search
@{
*/

#ifdef BELLE_IMPLEMENTATION
number SearchHistogramError(const Value& HistogramA, const Value& HistogramB)
{
  Value HistogramSubtraction;
  Value AKeys = HistogramA.Keys(), BKeys = HistogramB.Keys();
  for(count i = 0; i < AKeys.n(); i++)
    HistogramSubtraction[AKeys[i]] = true;
  for(count i = 0; i < BKeys.n(); i++)
    HistogramSubtraction[BKeys[i]] = true;
  Value SKeys = HistogramSubtraction.Keys();

  number MeanSquaredError = 0.f;
  {
    for(count i = 0; i < SKeys.n(); i++)
    {
      Value k = SKeys[i];
      integer a = HistogramA[k].IsInteger() ? HistogramA[k].AsInteger() :
        integer(0);
      integer b = HistogramB[k].IsInteger() ? HistogramB[k].AsInteger() :
        integer(0);

      MeanSquaredError += Square(number(a) - number(b)) / number(SKeys.n());
    }
  }
  return Sqrt(MeanSquaredError);
}

void SearchHistogramOfStaffPositions(Pointer<const Music> M, Histogram& H)
{
  Pointer<Music> MMutable;
  MMutable.New()->ImportXML(M->ExportXML());
  Array<Music::Node> AllNodes = MMutable->Nodes();
  if(MMutable and MMutable->Root())
  {
    if(Pointer<Geometry> G = System::MutableGeometry(MMutable))
    {
      G->Parse(*MMutable);
      if(G->GetNumberOfParts() and G->GetNumberOfInstants())
      {
        IslandState::Accumulate(MMutable);
        AccumulatePartState(MMutable);
        InstantState::Accumulate(MMutable);
        for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
        {
          for(count Instant = 0; Instant <  G->GetNumberOfInstants(); Instant++)
          {
            Music::ConstNode Island = G->LookupIsland(Part, Instant);
            Array<Music::ConstNode> Chords = ChordsOfIsland(Island);
            for(count i = 0; i < Chords.n(); i++)
            {
              Array<Music::ConstNode> Notes = NotesOfChord(Chords[i]);
              for(count j = 0; j < Notes.n(); j++)
              {
                mica::Concept Pitch = ActualPitchOfNote(Notes[j]);
                mica::Concept Clef = Island->GetState("PartState", "Clef",
                  "Active");
                mica::Concept Position = mica::map(
                  mica::map(Pitch, mica::DiatonicPitch), Clef);
                if(not mica::undefined(Position))
                  H.Increment(Value(mica::numerator(Position)));
              }
            }
          }
        }
      }
    }
  }
}

#endif

///@}

//Declarations

number SearchHistogramError(const Value& HistogramA, const Value& HistogramB);
void SearchHistogramOfStaffPositions(Pointer<const Music> M, Histogram& H);
