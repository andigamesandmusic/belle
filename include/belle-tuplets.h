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

void CollectAllTuplets(Music::ConstNode t, Array<Music::ConstNode>& a,
  Value& Levels, count Depth);
void CreateTupletBracket(Vector a, Vector b, String t, bool Above,
  bool Bracket, Pointer<const Font> f, Pointer<Stamp> s);
void EngraveTupletBracket(Music::ConstNode Tuplet, count Level);
void EngraveTupletBrackets(Music::ConstNode Chord);
void EngraveAllTupletBrackets(Pointer<const Music> M);
Music::ConstNode FirstChordInTuplet(Music::ConstNode t);
Music::Node GetLastChordInTuplet(Pointer<Music> M,
  Music::ConstNode TupletBeginning);
bool IsChordBeginningTupletStructure(Music::ConstNode n);
bool IsNodePartOfTuplet(Music::ConstNode n);
bool IsTupletNode(Music::ConstNode n);
Music::ConstNode LastChordInTuplet(Music::ConstNode t);
Ratio TotalTupletScalar(const Array<Ratio>& Scalars);
Array<Ratio> TupletScalarsOfChord(Music::ConstNode n);
Array<Music::ConstNode> TupletBeginningsAtChord(Music::ConstNode n);
Array<Music::ConstNode> TupletEndingsForBeginningsAtChord(Music::ConstNode n);
Array<Music::ConstNode> TupletStrandOfNode(Music::ConstNode n);
MusicLabel TupletTag(Music::ConstNode n);
String TupletTagChord(Music::ConstNode ChordToken);
MusicLabel TupletTagOfAncestor(Music::ConstNode n);

#ifdef BELLE_IMPLEMENTATION

void CollectAllTuplets(Music::ConstNode t, Array<Music::ConstNode>& a,
  Value& Levels, count Depth)
{
  Array<Music::ConstNode> s = TupletStrandOfNode(t->Next(TupletTag(t)));
  for(count i = 1; i < s.n(); i++)
    if(IsTupletInfo(s[i]))
      CollectAllTuplets(s[i], a, Levels, Depth + 1);
  if(IsTupletInfo(t))
    a.Add() = t, Levels.Add() = Depth;
}

void CreateTupletBracket(Vector a, Vector b, String t, bool Above,
  bool Bracket, Pointer<const Font> f, Pointer<Stamp> s)
{
  Pointer<Path> p2;
  Painter::Draw(*p2.New(), t, *f, 72.f * 2.f, Font::Italic);
  Text Layout(*f, Font::Italic, 72.f * 2.f, 1000.f);
  {
    Layout.ImportStringToWords(t);
    Layout.DetermineLineBreaks();
    Layout.Typeset();
    Painter::Draw(Layout, *p2.New());
  }
  (void)t;
  number BraceHeight = 0.75f;
  number Thickness = 0.1f;
  number TextWidth = p2->Bounds().Width() + 1.2f;
  Pointer<Path> p;
  p.New();
  Box R;
  if(Above)
  {
    R = Box(a, b + Vector(0, BraceHeight));
    if(Bracket)
    {
      Shapes::AddLine(*p, R.BottomLeft(), R.TopLeft(), Thickness,
        true, true, true, 0.5f);
      Shapes::AddLine(*p, R.TopLeft(), (R.TopLeft() + R.TopRight()) / 2.f -
        Vector(TextWidth / 2.f, 0.f), Thickness);
      Shapes::AddLine(*p, (R.TopLeft() + R.TopRight()) / 2.f +
        Vector(TextWidth / 2.f, 0.f), R.TopRight(), Thickness);
      Shapes::AddLine(*p, R.BottomRight(), R.TopRight(), Thickness,
        true, true, true, 0.5f);
    }
    Vector v((R.TopLeft() + R.TopRight()) / 2.f - p2->Bounds().Center());
    p->Append(*p2, Affine::Translate(v));
  }
  else
  {
    R = Box(a, b - Vector(0, BraceHeight));
    if(Bracket)
    {
      Shapes::AddLine(*p, R.TopLeft(), R.BottomLeft(), Thickness,
        true, true, true, 0.5f);
      Shapes::AddLine(*p, R.BottomLeft(),
        (R.BottomLeft() + R.BottomRight()) / 2.f -
        Vector(TextWidth / 2.f, 0.f), Thickness);
      Shapes::AddLine(*p, (R.BottomLeft() + R.BottomRight()) / 2.f +
        Vector(TextWidth / 2.f, 0.f), R.BottomRight(), Thickness);
      Shapes::AddLine(*p, R.TopRight(), R.BottomRight(), Thickness,
        true, true, true, 0.5f);
    }
    Vector v((R.BottomLeft() + R.BottomRight()) / 2.f - p2->Bounds().Center());
    p->Append(*p2, Affine::Translate(v));
  }
  s->Add()->p = p;
}

void EngraveTupletBracket(Music::ConstNode Tuplet, count Level)
{
  (void)Level;
  Music::ConstNode StartChord = FirstChordInTuplet(Tuplet);
  Music::ConstNode EndChord = LastChordInTuplet(Tuplet);
  Music::ConstNode StartIsland = IslandOfToken(StartChord);
  Music::ConstNode EndIsland = IslandOfToken(EndChord);
  mica::Concept Placement = Tuplet->Label.Get(mica::Placement);

  bool Above;
  bool Bracket;
  if(Placement == mica::Beam)
  {
    Above = mica::Concept(StartIsland->Label.GetState(
      "PartState", "Chord")[StartChord]["StemDirection"]) == mica::Up;
    Bracket = false;
  }
  else
    Above = Placement != mica::Below, Bracket = true;

  Box Bounds = UnionOfStampBounds(StartIsland, EndIsland) +
    Box(Vector(0.f, -2.f), Vector(0.f, 2.f));

  Vector StartPoint, EndPoint;
  number Distance = Bracket ? 1.5f : 0.5f;
  StartPoint.y = Above ?
    Bounds.Top() + Distance :// * number(Level + 1) :
    Bounds.Bottom() - Distance;// * number(Level + 1);
  EndPoint.y = StartPoint.y;
  StartPoint.x = -0.5f;
  EndPoint.x = EndIsland->Label.GetState(
    "IslandState", "TypesetX").AsNumber() -
    StartIsland->Label.GetState(
    "IslandState", "TypesetX").AsNumber() + 0.5f;
  Pointer<Stamp> StampStart = StartIsland->Label.Stamp().Object();
  bool FullRatio = Tuplet->Label.Get(mica::FullRatio) != mica::Undefined;
  String r = FullRatio ? String(Ratio(Tuplet->Label.Get(mica::Value))) :
    String(Ratio(Tuplet->Label.Get(mica::Value)).Numerator());
  if(FullRatio)
    r.Replace("/", ":");
  CreateTupletBracket(StartPoint, EndPoint, r, Above, Bracket,
    FontFromIsland(StartIsland), StampStart);
  StampStart->z()->Context = Tuplet;
}

void EngraveAllTupletBrackets(Pointer<const Music> M)
{
  Array<Music::ConstNode> a = System::GetIslands(M);
  for(count i = 0; i < a.n(); i++)
  {
    Array<Music::ConstNode> Tokens = a[i]->Children(MusicLabel(mica::Token));
    for(count j = 0; j < Tokens.n(); j++)
      if(IsChord(Tokens[j]))
        EngraveTupletBrackets(Tokens[j]);
  }
}

void EngraveTupletBrackets(Music::ConstNode Chord)
{
  if(not IsChordBeginningTupletStructure(Chord))
    return;

  Array<Music::ConstNode> t;
  Value Levels;
  CollectAllTuplets(TupletBeginningsAtChord(Chord).z(), t, Levels, 0);
  for(count i = 0; i < t.n(); i++)
    EngraveTupletBracket(t[i], Levels[i].AsCount());
}

Music::ConstNode FirstChordInTuplet(Music::ConstNode t)
{
  Music::ConstNode r;
  while(IsTupletInfo(t))
    r = t->Next(TupletTag(t)), t = r;
  return IsChord(r) ? r : Music::ConstNode();
}

Music::Node GetLastChordInTuplet(Pointer<Music> M,
  Music::ConstNode TupletBeginning)
{
  MusicLabel TupletEdge;
  TupletEdge.Set(mica::Type) = mica::Tuplet;
  Music::ConstNode x = TupletBeginning;
  Music::ConstNode LastChord;

  while(x and x->Get(mica::Type) == mica::Tuplet)
  {
    TupletEdge.Set("Tag") = x->Get("Tag");
    x = x->Series(TupletEdge).z();
  }

  return M->Promote(x);
}

bool IsChordBeginningTupletStructure(Music::ConstNode n)
{
  Array<Music::ConstNode> a = TupletBeginningsAtChord(n);
  return a.n() and not a.z()->Previous(MusicLabel(mica::Tuplet));
}

bool IsNodePartOfTuplet(Music::ConstNode n)
{
  return (IsChord(n) or IsTupletNode(n)) and
    n->Previous(MusicLabel(mica::Tuplet));
}

bool IsTupletNode(Music::ConstNode n)
{
  return n and n->Get(mica::Type) == mica::Tuplet;
}

Music::ConstNode LastChordInTuplet(Music::ConstNode t)
{
  return IsTupletInfo(t) ? t->Last(TupletTag(t)) : Music::ConstNode();
}

Ratio TotalTupletScalar(const Array<Ratio>& Scalars)
{
  Ratio r = 1;
  for(count i = 0; i < Scalars.n(); i++)
    r *= Scalars[i];
  return r;
}

Array<Ratio> TupletScalarsOfChord(Music::ConstNode n)
{
  Array<Ratio> Scalars;
  if(IsChord(n))
  {
    while(Music::ConstNode t = n->First(TupletTagOfAncestor(n)))
    {
      if(t->Next(TupletTagOfAncestor(n)))
        Scalars.Add() = t->Label.Get(mica::Value), n = t;
      else
        break;
    }
  }
  return Scalars;
}

Array<Music::ConstNode> TupletStrandOfNode(Music::ConstNode n)
{
  return n->Series(TupletTagOfAncestor(n));
}

Array<Music::ConstNode> TupletBeginningsAtChord(Music::ConstNode n)
{
  Array<Music::ConstNode> Tuplets;
  Music::ConstNode t;
  if(IsChord(n))
    while(n and (t = n->Previous(TupletTagOfAncestor(n))))
      n = (t->Get(mica::Type) == mica::Tuplet and
        t->Next(TupletTagOfAncestor(n)) and not
        t->Previous(TupletTagOfAncestor(n))) ? Tuplets.Add() = t :
        Music::ConstNode();
  return Tuplets;
}

Array<Music::ConstNode> TupletEndingsForBeginningsAtChord(Music::ConstNode n)
{
  Array<Music::ConstNode> a = TupletBeginningsAtChord(n);
  for(count i = 0; i < a.n(); i++)
    a[i] = LastChordInTuplet(a[i]);
  return a;
}

MusicLabel TupletTagOfAncestor(Music::ConstNode n)
{
  return IsNodePartOfTuplet(n) ? n->Previous(
    MusicLabel(mica::Tuplet), true)->Label : MusicLabel(mica::Type);
}

String TupletTagChord(Music::ConstNode ChordToken)
{
  if(not IsChord(ChordToken)) return "";
  if(ChordToken->Previous(MusicLabel(mica::Tuplet)))
  {
    Music::ConstEdge TupletPreviousEdge =
      ChordToken->Previous(MusicLabel(mica::Tuplet), true);
    return TupletPreviousEdge->Get("Tag");
  }
  return "";
}

MusicLabel TupletTag(Music::ConstNode n)
{
  MusicLabel r(mica::Type);
  if(IsTupletInfo(n))
    r.Set(mica::Type) = mica::Tuplet,
    r.Set("Tag") = n->Label.Get("Tag");
  return r;
}

#endif
