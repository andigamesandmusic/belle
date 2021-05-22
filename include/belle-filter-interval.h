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
@name Interval-Transpostion.

The function that interval transposes the given passage.
@{
*/
void IntervalTransposition(Pointer<Music> M, count StaffNumber,
  mica::Concept IntervalTranspose, mica::Concept Direction,
  mica::Concept Octaves);

#ifdef BELLE_IMPLEMENTATION

/**Interval transposes the specified staff by the given direction, interval and
octave transpose.*/
void IntervalTransposition(Pointer<Music> M, count StaffNumber,
  mica::Concept IntervalTranspose, mica::Concept Direction,
  mica::Concept Octaves)
{
  Music::Node x = Music::Node();

  Pointer<Geometry> G = System::MutableGeometry(M);
  G->Parse(*M);
  Array<Music::ConstNode> IslandBeginnings = G->GetPartBeginnings();
  if(StaffNumber < 1 or StaffNumber > IslandBeginnings.n())
    return;

  x = M->Promote(IslandBeginnings[StaffNumber - 1]);

  for(; x; x = x->Next(MusicLabel(mica::Partwise)))
  {
    Array<Music::Node> NextTokens = x->Children(MusicLabel(mica::Token));
    for(count i = 0; i < NextTokens.n() && IsChord(NextTokens.a()); i++)
    {
      Array<Music::Node> Notes =
       NextTokens[i]->Children(MusicLabel(mica::Note));

      for (count j = 0; j < Notes.n(); j++)
      {
        mica::Concept NewPitch = mica::Intervals::transpose(
        Notes[j]->Get(mica::Value), IntervalTranspose, Direction,
          Octaves);
        if(!undefined(NewPitch))
          Notes[j]->Set(mica::Value) = NewPitch;
      }
    }
  }
}
#endif
///@}
