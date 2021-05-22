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

///@name Articulations
///@{
void EngraveArticulations(Music::ConstNode Chord, Vector ChordOffset);
///@}

#ifdef BELLE_IMPLEMENTATION
void EngraveArticulations(Music::ConstNode Chord, Vector ChordOffset)
{
  if(not IsChord(Chord))
    return;

  Music::ConstNode Island = IslandOfToken(Chord);
  Pointer<Stamp> IslandStamp = StampForIsland(Island);

  mica::Concept ArticulationPlacement = Chord->Get(mica::ArticulationPlacement);
  bool Above = ArticulationPlacement != mica::Below;
  Value StemState = Beaming::StemState(Island, Chord);
  if (ArticulationPlacement == mica::Undefined)
    Above = not StemState["StemUp"].AsBoolean();

  Vector StemEnd = Beaming::StemEndPoint(StemState);

  count Articulations = count(mica::length(mica::Articulations));
  for(count i = 0; i < Articulations; i++)
  {
    mica::Concept Articulation = mica::item(mica::Articulations, i);
    if(Chord->Get(Articulation) != mica::Undefined)
    {
      if(Pointer<const Glyph> g =
        SMuFLGlyphFromCodepoint(FontFromIsland(Island), unicode(mica::numerator(
          mica::map(
            mica::SMuFL,
            Articulation,
            Above ? mica::Above : mica::Below)))))
      {
        Vector Position;
        Position.x = -g->Bounds().Center().x;
        Position.y = Above ?
          Max((IslandStamp->Bounds().Top() - g->Bounds().Bottom()),
            Max(number(2), StemEnd.y)) :
          Min((IslandStamp->Bounds().Bottom() - g->Bounds().Top()),
            Min(number(-2), StemEnd.y));

        Position.y += Above ? 0.3f : -0.3f;
        Position += ChordOffset;
        IslandStamp->Add()->p = g;
        IslandStamp->z()->a = Affine::Translate(Position);
      }
    }
  }
}
#endif
