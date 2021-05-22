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
@name Flags

Functions that implement flag engraving
@{
*/

#ifdef BELLE_IMPLEMENTATION

Pointer<const Glyph> FlagGlyph(Pointer<const Font> f, Ratio Duration,
  mica::Concept StemDirection)
{
  return SMuFLGlyph(f, mica::map(mica::SMuFL, StemDirection, mica::item(
    mica::Flags, int64(FlagsGivenDuration(Duration) - 1))));
}

bool DurationHasFlag(Ratio Duration)
{
  return UndottedDuration(Duration) <= Ratio(1, 8);
}

bool DurationHasStem(Ratio Duration)
{
  return UndottedDuration(Duration) <= Ratio(1, 2);
}

bool DurationHasStemOnly(Ratio Duration)
{
  return DurationHasStem(Duration) and not DurationHasFlag(Duration);
}

void EngraveFlag(Music::ConstNode Island, const Value &Chord,
  Ratio Duration, Vector Offset, mica::Concept StemDirection)
{
  (void)Chord;
  if(DurationHasFlag(Duration))
  {
    Pointer<Stamp> IslandStamp = StampForIsland(Island);
    Pointer<const Glyph> Flag = FlagGlyph(FontFromIsland(Island), Duration,
      StemDirection);
    IslandStamp->Add()->p = Flag;
    IslandStamp->z()->a = Affine::Translate(Offset);
  }
}

#endif

///@}

//Declarations
Pointer<const Glyph> FlagGlyph(Pointer<const Font> f, Ratio Duration,
  mica::Concept StemDirection);
void EngraveFlag(Music::ConstNode Island, const Value &Chord, Ratio Duration,
  Vector Offset, mica::Concept StemDirection);
bool DurationHasFlag(Ratio Duration);
bool DurationHasStem(Ratio Duration);
bool DurationHasStemOnly(Ratio Duration);
