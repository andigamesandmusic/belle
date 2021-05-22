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

///@name Octave-Transposition
///@{

///Returns the union of all the stamp bounds from the start to end island.
Box UnionOfStampBounds(Music::ConstNode StartIsland,
  Music::ConstNode EndIsland);

///Engraves the octave transpositions on the system.
void EngraveOctaveTransposition(Music::ConstNode StartIsland,
  Music::ConstNode EndIsland, Music::ConstEdge Transposition);

///Engraves the octave transpositions on the system.
void EngraveOctaveTranspositions(Pointer<const Music> M);

///Returns the x-position of the island as determined by the spacer.
number IslandX(Music::ConstNode Island);

///Returns whether the edge is an octave-transposition.
bool IsOctaveTransposition(Music::ConstEdge e);

///Gets the SMuFL codepoint for a given octave-transposition signature.
unicode SMuFLCodepointForOctaveTransposition(String Signature);

///@}
#ifdef BELLE_IMPLEMENTATION

void EngraveOctaveTransposition(Music::ConstNode StartIsland,
  Music::ConstNode EndIsland, Music::ConstEdge Transposition)
{
  const number OctaveTranspositionLineDisplace = 0.1f;
  const number OctaveTranspositionHookBuffer = 1.f;
  const number OctaveTranspositionHookSize = 0.7f;
  number OctaveTranspositionLineWidth =
    HouseStyle::GetValue(StartIsland, "StaffLineThickness");

  if(not IsOctaveTransposition(Transposition))
    return;

  Pointer<const Font> f = FontFromIsland(StartIsland);
  Pointer<Stamp> s = StampForIsland(StartIsland);
  Box r = UnionOfStampBounds(StartIsland, EndIsland);
  bool Above = Transposition->Get(mica::Placement) != mica::Below;
  unicode Signature = SMuFLCodepointForOctaveTransposition(
    Transposition->Get("Text"));
  r.Dilate(0.5f);
  Pointer<const Glyph> g = SMuFLGlyphFromCodepoint(f, Signature);
  Vector SignaturePosition(StartIsland->Label.GetState(
      "IslandState", "TokenBounds").AsBox().Left(), Above ?
      (r.Top() - g->Bounds().Bottom()) :
      (r.Bottom() - g->Bounds().Top()));
  if(StartIsland == EndIsland)
    SignaturePosition.x =
      -(g->Bounds().Width() + OctaveTranspositionHookSize) / number(2);
  s->Add()->p = g;
  s->z()->a = Affine::Translate(SignaturePosition);
  s->z()->Context = Transposition;

  Vector LineStart = SignaturePosition + Vector(g->Bounds().Right(),
    (Above ? g->Bounds().Height() - OctaveTranspositionLineDisplace :
    OctaveTranspositionLineDisplace));
  Vector LineEnd(IslandX(EndIsland) - IslandX(StartIsland) +
    EndIsland->Label.GetState(
      "IslandState", "TokenBounds").AsBox().Right() +
      OctaveTranspositionHookBuffer, LineStart.y);
  Pointer<Path> Line;
  Line.New();
  Vector LineHookStart = LineEnd - Vector(OctaveTranspositionHookSize, 0.f);
  if(LineHookStart.x < LineStart.x)
  {
    LineHookStart = LineStart;
    LineEnd = LineHookStart + Vector(OctaveTranspositionHookSize, 0.f);
  }
  else
  {
    Shapes::AddDashedLine(*Line, LineStart, LineHookStart,
      OctaveTranspositionLineWidth, 0.5f, 0.5f, 0.5f, true, true);
  }
  Shapes::AddLine(*Line, LineHookStart, LineEnd,
    OctaveTranspositionLineWidth);
  Shapes::AddLine(*Line, LineEnd, LineEnd + Vector(0.f,
    number(Above ? -1 : 1) * OctaveTranspositionHookSize),
    OctaveTranspositionLineWidth);
  s->Add()->p = Line;
  s->z()->Context = Transposition;
}

void EngraveOctaveTranspositions(Pointer<const Music> M)
{
  if(!M) return;

  MusicLabel EdgeFilter(mica::Span);
  EdgeFilter.Set(mica::Kind) = mica::OctaveTransposition;

  Music::ConstNode m, n;
  for(m = M->Root(); m; m = m->Next(MusicLabel(mica::Instantwise)))
    for(n = m; n; n = n->Next(MusicLabel(mica::Partwise)))
      if(Music::ConstNode EndIsland = n->Next(EdgeFilter))
        EngraveOctaveTransposition(n, EndIsland, n->Next(EdgeFilter, true));
}

number IslandX(Music::ConstNode Island)
{
  return IsIsland(Island) ? Island->Label.GetState(
    "IslandState", "TypesetX").AsNumber() : number(0);
}

bool IsOctaveTransposition(Music::ConstEdge e)
{
  return e and e->Get(mica::Type) == mica::Span and
    e->Get(mica::Kind) == mica::OctaveTransposition and e->Get("Text").n();
}

unicode SMuFLCodepointForOctaveTransposition(String Signature)
{
  return unicode(
    Signature == "8"    ? SMuFLCodepoint(mica::Ottava) :
    Signature == "8va"  ? SMuFLCodepoint(mica::OttavaAlta) :
    Signature == "8vb"  ? SMuFLCodepoint(mica::OttavaBassa) :
    Signature == "8ba"  ? SMuFLCodepoint(mica::OttavaBassaBa) :
    Signature == "15"   ? SMuFLCodepoint(mica::Quindicesima) :
    Signature == "15ma" ? SMuFLCodepoint(mica::QuindicesimaAlta) :
    Signature == "15mb" ? SMuFLCodepoint(mica::QuindicesimaBassa) :
    Signature == "22"   ? SMuFLCodepoint(mica::Ventiduesima) :
    Signature == "22ma" ? SMuFLCodepoint(mica::VentiduesimaAlta) :
    Signature == "22mb" ? SMuFLCodepoint(mica::VentiduesimaBassa) : 0);
}

Box UnionOfStampBounds(Music::ConstNode StartIsland,
  Music::ConstNode EndIsland)
{
  Box r;
  for(Music::ConstNode m = StartIsland; m;
    m and (m = m->Next(MusicLabel(mica::Partwise))))
  {
    r += StampForIsland(m)->Bounds();
    if(m == EndIsland)
      m = Music::ConstNode();
  }
  return r;
}
#endif
