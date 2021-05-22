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

///@name Pedal Markings
///@{
void EngravePedalMarking(Music::ConstNode Float);
void EngravePedalMarkings(Pointer<const Music> M);
void EngravePedalStack(const Array<Music::ConstNode>& FloatStack);
///@}

#ifdef BELLE_IMPLEMENTATION
void EngravePedalMarking(Music::ConstNode Float)
{
  Array<Music::ConstNode> PedalSequence = Float->Series(MusicLabel(mica::Span));
  Array<Music::ConstNode> IslandSequence;
  for(count i = 0; i < PedalSequence.n(); i++)
    IslandSequence.Add() = OriginOfFloat(PedalSequence[i]);

  //Box r = UnionOfStampBounds(IslandSequence.a(), IslandSequence.z());
  Box r = StampForIsland(IslandSequence.a())->Bounds() +
    StampForIsland(IslandSequence.z())->Bounds();
  Pointer<const Font> StartFont = FontFromIsland(IslandSequence.a());
  Pointer<const Glyph> Ped = SMuFLGlyphFromCodepoint(StartFont,
    PedalSequence.a()->Get(mica::PedalMarking) ==
    mica::DamperPedal ? SMuFLCodepoint(mica::PedalMarkPed) :
      SMuFLCodepoint(mica::PedalMarkRelease));
  {
    Pointer<Stamp> s = StampForIsland(IslandSequence.a());
    Vector Position(-1.5, r.Bottom() - Ped->Bounds().Bottom() -
      Ped->Bounds().Height() - 1.f);
    s->Add()->p = Ped;
    s->z()->a = Affine::Translate(Position);
    s->z()->Context = Float;
  }
  if(IslandSequence.a() != IslandSequence.z())
  {
    Pointer<const Font> f = FontFromIsland(IslandSequence.z());
    Pointer<Stamp> s = StampForIsland(IslandSequence.z());
    Pointer<const Glyph> g = SMuFLGlyphFromCodepoint(f,
      PedalSequence.z()->Get(mica::PedalMarking) ==
      mica::DamperPedal ? SMuFLCodepoint(mica::PedalMarkPed) :
        SMuFLCodepoint(mica::PedalMarkRelease));
    Vector Position(-g->Bounds().Center().x, r.Bottom() - g->Bounds().Bottom() -
      Ped->Bounds().Height() - 1.f);
    s->Add()->p = g;
    s->z()->a = Affine::Translate(Position);
    s->z()->Context = PedalSequence.z();
  }
}

void EngravePedalMarkings(Pointer<const Music> M)
{
  if(!M) return;
  Music::ConstNode m, n;
  for(m = M->Root(); m; m = m->Next(MusicLabel(mica::Instantwise)))
    for(n = m; n; n = n->Next(MusicLabel(mica::Partwise)))
      EngravePedalStack(n->Series(TraverseFloatStack(mica::Below)));
}

void EngravePedalStack(const Array<Music::ConstNode>& FloatStack)
{
  for(count i = 0; i < FloatStack.n(); i++)
  {
    Music::ConstNode Float = FloatStack[i];
    if(IsInitialPedalMarking(Float))
      EngravePedalMarking(Float);
  }
}
#endif
