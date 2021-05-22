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
@name Expression engraving

Functions that implement construction and engraving of dynamics and expressions
@{
*/

#ifdef BELLE_IMPLEMENTATION

void ComposePathForDynamic(Pointer<const Font> NotationFont,
  const String& Dynamic, Stamp& Destination)
{
  Pointer<const Glyph> g;
  if(NotationFont)
    if(Pointer<const Typeface> t = NotationFont->GetTypeface(Font::Notation))
      g = t->LookupGlyph(SMuFLForDynamic(Dynamic));
  Pointer<Path> Composed;
  if(g)
  {
    Composed.New()->Append(*g);
  }
  Destination.Add()->p = Composed;
}

void ComposePathForExpression(Pointer<const Font> NotationFont,
  const String& Expression, Stamp& Destination, String Style, number PointSize)
{
  Path TextPath;
  Font::Style s =
    Style == "Regular" ? Font::Regular :
    Style == "Bold" ? Font::Bold :
    Style == "Bold Italic" ? Font::BoldItalic :
    Style == "Italic" ? Font::Italic :
    Font::Italic;

  if(not (PointSize >= 1.f and PointSize <= 144.f))
    PointSize = 12.f;

  Text Layout(*NotationFont, s, 72.f * 2.5f * PointSize / 12.f, 1000.f);
  Layout.ImportStringToWords(Expression);
  Layout.DetermineLineBreaks();
  Layout.Typeset();
  Painter::Draw(Layout, TextPath);
  Pointer<Path> Composed;
  Composed.New()->Append(TextPath);
  Destination.Add()->p = Composed;
}

///Engraves expressions linked off of the island.
void EngraveExpression(Music::ConstNode Island, Music::ConstNode Expression)
{
  String ExpressionText = Expression->Get("Text");
  mica::Concept Placement = Expression->Get(mica::Placement);
  Pointer<const Font> NotationFont = FontFromIsland(Island);

  Box r = StampForIsland(Island)->Bounds();
  number BottomY = Min(r.Bottom(), number(-2));
  number TopY = Max(r.Top(), number(2));
  Stamp TemporaryStamp;
  if(IsDynamic(ExpressionText))
    ComposePathForDynamic(NotationFont, ExpressionText, TemporaryStamp);
  else
    ComposePathForExpression(NotationFont, ExpressionText, TemporaryStamp,
      Expression->Get("Style"), Expression->Get("Size").ToNumber());
  Vector PlacementLeftBelow(-TemporaryStamp.Bounds().Left() + r.Left(),
      BottomY - TemporaryStamp.Bounds().Top() - 0.5f);
  Vector PlacementLeftAbove(-TemporaryStamp.Bounds().Left() + r.Left(),
      TopY - TemporaryStamp.Bounds().Bottom() + 0.5f);
  Vector PlacementCenterBelow(-TemporaryStamp.Bounds().Center().x,
      BottomY - TemporaryStamp.Bounds().Top() - 0.5f);
  Vector PlacementCenterAbove(-TemporaryStamp.Bounds().Center().x,
      TopY - TemporaryStamp.Bounds().Bottom() + 0.5f);
  Vector FinalPlacement =
    IsDynamic(ExpressionText) ?
    (Placement == mica::Above ? PlacementCenterAbove : PlacementCenterBelow) :
    (Placement == mica::Above ? PlacementLeftAbove : PlacementLeftBelow);

  TemporaryStamp.a = Affine::Translate(FinalPlacement);
  TemporaryStamp.Context = Expression;
  StampForIsland(Island)->AccumulateGraphics(TemporaryStamp);
}

void EngraveFloats(Pointer<const Music> M)
{
  if(!M) return;

  MusicLabel EdgeFilter(mica::Span);
  EdgeFilter.Set(mica::Kind) = mica::OctaveTransposition;

  Music::ConstNode m, n;
  for(m = M->Root(); m; m = m->Next(MusicLabel(mica::Instantwise)))
    for(n = m; n; n = n->Next(MusicLabel(mica::Partwise)))
      EngraveFloatsOnIsland(n);
}

void EngraveFloatsOnIsland(Music::ConstNode Island)
{
  Array<Music::ConstNode> Chords = ChordsOfIsland(Island);
  for(count i = 0; i < Chords.n(); i++)
  {
    EngraveFloatStack(Island,
      Chords[i]->Series(TraverseFloatStack(mica::Above)));
    EngraveFloatStack(Island,
      Chords[i]->Series(TraverseFloatStack(mica::Below)));
  }
  EngraveFloatStack(Island, Island->Series(TraverseFloatStack(mica::Above)));
  EngraveFloatStack(Island, Island->Series(TraverseFloatStack(mica::Below)));
}

void EngraveFloatStack(Music::ConstNode Island,
  const Array<Music::ConstNode>& FloatStack)
{
  for(count i = 0; i < FloatStack.n(); i++)
  {
    Music::ConstNode Float = FloatStack[i];
    if(IsExpression(Float))
      EngraveExpression(Island, Float);
  }
}

Music::Node HighestInFloatStack(Music::Node Island, mica::Concept Placement)
{
  Music::Node LastInStack = Island, NextInStack;
  while((NextInStack = LastInStack->Next(TraverseFloatStack(Placement))))
    LastInStack = NextInStack;
  return LastInStack;
}

bool IsDynamic(const String& Dynamic)
{
  const ascii* Letter = Dynamic.Merge();
  bool OnlyDynamicLetters = true;
  while(*Letter and OnlyDynamicLetters)
    if(not IsDynamicLetter(*Letter++))
      OnlyDynamicLetters = false;
  return OnlyDynamicLetters;
}

bool IsDynamicLetter(ascii Letter)
{
  return Letter == 'f' or Letter == 'm' or Letter == 'n' or Letter == 'p' or
    Letter == 'r' or Letter == 's' or Letter == 'z';
}

Music::ConstNode OriginOfFloat(Music::ConstNode Float)
{
  Music::ConstNode Current = Float, Previous;
  while((Previous = Current->Previous(MusicLabel(mica::Float))))
    Current = Previous;
  return Current;
}

unicode SMuFLForDynamic(String Dynamic)
{
  return unicode(
    Dynamic == "p"      ?
      SMuFLCodepoint(mica::DynamicMarkPiano) :
    Dynamic == "m"      ?
      SMuFLCodepoint(mica::DynamicMarkMezzo) :
    Dynamic == "f"      ?
      SMuFLCodepoint(mica::DynamicMarkForte) :
    Dynamic == "r"      ?
      SMuFLCodepoint(mica::DynamicMarkRinforzando) :
    Dynamic == "s"      ?
      SMuFLCodepoint(mica::DynamicMarkSforzandoS) :
    Dynamic == "z"      ?
      SMuFLCodepoint(mica::DynamicMarkZ) :
    Dynamic == "n"      ?
      SMuFLCodepoint(mica::DynamicMarkNiente) :
    Dynamic == "pppppp" ?
      SMuFLCodepoint(mica::DynamicMarkPianoissississississimo) :
    Dynamic == "ppppp"  ?
      SMuFLCodepoint(mica::DynamicMarkPianoississississimo) :
    Dynamic == "pppp"   ?
      SMuFLCodepoint(mica::DynamicMarkPianoissississimo) :
    Dynamic == "ppp"    ?
      SMuFLCodepoint(mica::DynamicMarkPianoississimo) :
    Dynamic == "pp"     ?
      SMuFLCodepoint(mica::DynamicMarkPianoissimo) :
    Dynamic == "mp"     ?
      SMuFLCodepoint(mica::DynamicMarkMezzopiano) :
    Dynamic == "mf"     ?
      SMuFLCodepoint(mica::DynamicMarkMezzoforte) :
    Dynamic == "pf"     ?
      SMuFLCodepoint(mica::DynamicMarkPianoforte) :
    Dynamic == "ff"     ?
      SMuFLCodepoint(mica::DynamicMarkFortissimo) :
    Dynamic == "fff"    ?
      SMuFLCodepoint(mica::DynamicMarkFortississimo) :
    Dynamic == "ffff"   ?
      SMuFLCodepoint(mica::DynamicMarkFortissississimo) :
    Dynamic == "fffff"  ?
      SMuFLCodepoint(mica::DynamicMarkFortississississimo) :
    Dynamic == "ffffff" ?
      SMuFLCodepoint(mica::DynamicMarkFortissississississimo) :
    Dynamic == "fp"     ?
      SMuFLCodepoint(mica::DynamicMarkFortepiano) :
    Dynamic == "fz"     ?
      SMuFLCodepoint(mica::DynamicMarkForzando) :
    Dynamic == "sf"     ?
      SMuFLCodepoint(mica::DynamicMarkSforzandoSF) :
    Dynamic == "sfp"    ?
      SMuFLCodepoint(mica::DynamicMarkSforzandoPiano) :
    Dynamic == "sfpp"   ?
      SMuFLCodepoint(mica::DynamicMarkSforzandoPianissimo) :
    Dynamic == "sfz"    ?
      SMuFLCodepoint(mica::DynamicMarkSforzato) :
    Dynamic == "sfzp"   ?
      SMuFLCodepoint(mica::DynamicMarkSforzatoPiano) :
    Dynamic == "sffz"   ?
      SMuFLCodepoint(mica::DynamicMarkSforzatoFortissimo) :
    Dynamic == "rf"     ?
      SMuFLCodepoint(mica::DynamicMarkRinforzandoRF) :
    Dynamic == "rfz"    ?
      SMuFLCodepoint(mica::DynamicMarkRinforzandoRFZ) : 0);
}

MusicLabel TraverseFloatStack(mica::Concept Placement)
{
  MusicLabel t(mica::Float);
  t.Set(mica::Placement) = Placement;
  return t;
}

#endif
///@}

//Declarations
void ComposePathForDynamic(Pointer<const Font> NotationFont,
  const String& Dynamic, Stamp& Destination);
void ComposePathForExpression(Pointer<const Font> NotationFont,
  const String& Expression, Stamp& Destination, String Style, number PointSize);
void EngraveExpression(Music::ConstNode Island, Music::ConstNode Expression);
void EngraveFloats(Pointer<const Music> M);
void EngraveFloatsOnIsland(Music::ConstNode Island);
void EngraveFloatStack(Music::ConstNode Island,
  const Array<Music::ConstNode>& FloatStack);
Music::Node HighestInFloatStack(Music::Node Island, mica::Concept Placement);
bool IsDynamic(const String& Dynamic);
bool IsDynamicLetter(ascii Letter);
Music::ConstNode OriginOfFloat(Music::ConstNode Float);
unicode SMuFLForDynamic(String Dynamic);
MusicLabel TraverseFloatStack(mica::Concept Placement);
