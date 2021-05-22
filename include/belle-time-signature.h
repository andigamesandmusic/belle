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
@name Time Signature Construction

Functions that implement construction of simple and complex time signatures.
@{
*/

mica::Concept TimeSignatureNumeral(count i);

#ifdef BELLE_IMPLEMENTATION
mica::Concept TimeSignatureNumeral(count i)
{
  switch(i)
  {
    case 0: return mica::TimeSignature0;
    case 1: return mica::TimeSignature1;
    case 2: return mica::TimeSignature2;
    case 3: return mica::TimeSignature3;
    case 4: return mica::TimeSignature4;
    case 5: return mica::TimeSignature5;
    case 6: return mica::TimeSignature6;
    case 7: return mica::TimeSignature7;
    case 8: return mica::TimeSignature8;
    case 9: return mica::TimeSignature9;
    default:;
  }
   return mica::Undefined;
}

bool IsInSMuFLCodepointRange(unicode c)
{
  return c > 0xE000 and c < 0xF900;
}

Pointer<Path> TimeSignatureConstructFormula(String FormulaText,
  Pointer<const Font> NotationFont, Font::Style LetterTextStyle)
{
  const number OuterPartGap = 0.2f;
  List<List<String> > Formula = TimeSignatureParseFormula(FormulaText);
  Pointer<Path> FormulaPath; FormulaPath.New();
  number OuterGroupX = 0.f;
  for(count i = 0; i < Formula.n(); i++)
  {
    Pointer<Path> OuterGroupPart;
    OuterGroupPart.New();
    for(count j = 0; j < Formula[i].n(); j++)
    {
      Pointer<Path> InnerPart = TimeSignatureConstructFormulaPart(
        Formula[i][j], NotationFont, LetterTextStyle);
      OuterGroupPart->Append(*InnerPart,
        Affine::Translate(Vector(
          -InnerPart->Bounds().Width() / 2.f - InnerPart->Bounds().Left(),
          Formula[i].n() == 1 ? 0.f : (j == 0 ? 1.f : -1.f))));
    }
    FormulaPath->Append(*OuterGroupPart, Affine::Translate(Vector(
      OuterGroupX - OuterGroupPart->Bounds().Left(), 0.f)));
    OuterGroupX += OuterGroupPart->Bounds().Width() + OuterPartGap;
  }
  return FormulaPath;
}

Pointer<Path> TimeSignatureConstructFormulaPart(String FormulaPartText,
  Pointer<const Font> NotationFont, Font::Style LetterTextStyle)
{
  const number InnerPartGap = 0.2f;
  if(not NotationFont) return Pointer<Path>().New();
  Pointer<const Typeface> Notation = NotationFont->GetTypeface(Font::Notation);

  Pointer<const Typeface> Letter = NotationFont->GetTypeface(LetterTextStyle);
  number LetterYOffset = 0.f;
  number LetterScale   = 1.f;
  {
    if(Letter)
      if(Pointer<const Glyph> ComparisonGlyph = Letter->LookupGlyph('0'))
        LetterYOffset = -ComparisonGlyph->Bounds().Center().y,
        LetterScale = 2.f / ComparisonGlyph->Bounds().Height();
    if(not (LetterScale > 0.f))
      LetterScale = 1.f;
  }

  Pointer<Path> FormulaPart; FormulaPart.New();

  number XOffset = 0.f;
  for(count i = 0; i < FormulaPartText.c(); i++)
  {
    unicode Character = FormulaPartText.cth(i);
    Path p;
    if(IsInSMuFLCodepointRange(Character) and Notation)
    {
      if(Pointer<const Glyph> g = Notation->LookupGlyph(Character))
        p.Append(*g, Affine::Translate(Vector(-g->Bounds().Left(), 0.f)));
    }
    else if(Letter)
    {
      if(Pointer<const Glyph> g = Letter->LookupGlyph(Character))
        p.Append(*g,
          Affine::Scale(LetterScale) *
          Affine::Translate(Vector(-g->Bounds().Left(), LetterYOffset)));
    }
    if(!p.Bounds().IsEmpty())
    {
      FormulaPart->Append(p, Affine::Translate(Vector(
        XOffset - p.Bounds().Left(), 0.f)));
      XOffset += p.Bounds().Width() + InnerPartGap;
    }
  }
  return FormulaPart;
}

void TimeSignatureEngrave(Music::ConstNode IslandNode,
  Music::ConstNode Token)
{
  //Get the notation typeface.
  Pointer<Value::ConstReference> H =
    IslandNode->Label.GetState("HouseStyle", "Global").Object();
  Pointer<const Font> NotationFont = H->Get()["NotationFont"].ConstObject();
  if(!NotationFont) return;
  Pointer<const Typeface> NotationTypeface =
    NotationFont->GetTypeface(Font::Notation);
  if(!NotationTypeface) return;

  //Get the island stamp.
  Pointer<Stamp> IslandStamp = IslandNode->Label.Stamp().Object();
  mica::Concept v = Token->Get(mica::Value);
  if(v == mica::RegularTimeSignature or
    v == mica::SingleNumberTimeSignature)
  {
    count Numerator = count(numerator(mica::UUIDv4(Token->Get(mica::Beats))));
    count Denominator = count(denominator(Token->Get(mica::NoteValue)));

    Vector NumeratorPosition, DenominatorPosition;
    if(v == mica::SingleNumberTimeSignature)
      NumeratorPosition = Vector();
    else
      DenominatorPosition = -(NumeratorPosition = Vector(0.f, 1.f)),
      TimeSignatureEngraveRegularNumeral(IslandNode, NotationTypeface,
        Denominator, Value(IslandNode),
        DenominatorPosition, mica::Undefined, Token);

    TimeSignatureEngraveRegularNumeral(IslandNode, NotationTypeface,
      Numerator, Value(IslandNode),
      NumeratorPosition, mica::Undefined, Token);
  }
  else if(v == mica::CommonTime || v == mica::CutTime)
  {
    Pointer<const Glyph> GlyphSymbol =
      SMuFLGlyphFromTypeface(NotationTypeface, v);
    Pointer<const Path> PathSymbol = GlyphSymbol;

    //Create stamp for the symbol.
    Vector SymbolTranslate(-PathSymbol->Bounds().Center().x, 0.f);
    IslandStamp->Add()->p = PathSymbol;
    IslandStamp->z()->a = Affine::Translate(SymbolTranslate);
    IslandStamp->z()->Context = Token;
  }
  else if(v == mica::OpenTimeSignature)
  {
    Pointer<const Glyph> GlyphSymbol =
      SMuFLGlyphFromTypeface(NotationTypeface, v);
    Vector SymbolTranslate(-GlyphSymbol->Bounds().Center().x, 0.f);
    IslandStamp->Add()->p = GlyphSymbol;
    IslandStamp->z()->a = Affine::Translate(SymbolTranslate);
    IslandStamp->z()->Context = Token;
  }
  else if(v == mica::AdditiveTimeSignature)
    IslandStamp->Add()->p = TimeSignatureConstructFormula(
      Token->Get("Formula"), NotationFont, Font::Bold);
}

void TimeSignatureEngraveRegularNumeral(Music::ConstNode IslandNode,
  Pointer<const Typeface> NotationTypeface, count Numeral,
  Value Grouping, Vector Offset,
  mica::Concept Context, Music::ConstNode AssociatedTokenContext)
{
  (void)Grouping;
  (void)Context;
  Pointer<Value::ConstReference> H =
    IslandNode->Label.GetState("HouseStyle", "Global").Object();
  Pointer<Stamp> IslandStamp = IslandNode->Label.Stamp().Object();
  Array<Pointer<const Glyph> > NumberGlyphs;
  Array<number> NumberWidths;
  for(count i = 0; i <= 9; i++)
  {
    NumberGlyphs.Add() = SMuFLGlyphFromTypeface(
      NotationTypeface, TimeSignatureNumeral(i));
    NumberWidths.Add() = NumberGlyphs.z()->Bounds().Width();
  }

  List<count> NumeralDigits;
  {
    count Num = Numeral;
    while(Num > 0)
      NumeralDigits.Prepend(Num % 10), Num /= 10;
  }

  const number NumberGap = 0.2f;
  number NumeralWidth = -NumberGap;
  for(Counter i; i.z(NumeralDigits); i++)
    NumeralWidth += NumberWidths[NumeralDigits[i]] + NumberGap;

  number CenteringOffset = 0.f;
  for(Counter i; i.z(NumeralDigits); i++)
  {
    count Digit = NumeralDigits[i];
    Pointer<const Glyph> DigitGlyph = NumberGlyphs[Digit];
    Vector Translate =
      Offset + Vector(-NumeralWidth / 2.f + CenteringOffset, 0.f);
    CenteringOffset += NumberWidths[Digit];
    IslandStamp->Add()->p = DigitGlyph;
    IslandStamp->z()->a = Affine::Translate(Translate);
    IslandStamp->z()->Context = AssociatedTokenContext;
  }
}

List<List<String> > TimeSignatureParseFormula(String Formula)
{
  /*
  Format uses semicolon to distinguish outer groups and pipe to separate
  numerator from denominator. Characters that have corresponding stylized
  glyphs in SMuFL are automatically mapped to those characters (per the
  TimeSignatureSubstituteForSymbol method).

  Examples:
  4|4
  4+3|4
  4+3|4;+;1|8
  4+3|4;+;1|8;x;2
  (;4+3|4;);+;1|8;x;2
  (;4+(3x2)|4;);+;1|8;x;2
  (;(4+3,2i)|4;+;1|8;);x;1.23xA
  */

  //First remove irrelevant whitespace.
  Formula.Replace("\n", "");
  Formula.Replace("\r", "");
  Formula.Replace("\t", "");
  Formula.Replace(" ", "");

  List<String> OuterGroups = Formula.Tokenize(";", true);
  List<List<String> > OuterAndInnerGroups;
  for(count i = 0; i < OuterGroups.n(); i++)
  {
    List<String> InnerGroups = OuterGroups[i].Tokenize("|");
    List<String> InnerGroupsRewritten;
    for(count j = 0; j < InnerGroups.n() and j < 2; j++)
      InnerGroupsRewritten.Add() =
        TimeSignatureSubstituteForSMuFLSymbolInString(
          InnerGroups[j], InnerGroups.n() == 1);
    if(InnerGroupsRewritten.n())
      OuterAndInnerGroups.Add() = InnerGroupsRewritten;
  }
  return OuterAndInnerGroups;
}

unicode TimeSignatureSubstituteForSMuFLSymbol(
  unicode x, bool Outer)
{
  bool Inner = not Outer;
  return
    x == unicode('0')                                          ?
      SMuFLCodepoint(mica::TimeSignature0)                     :
    x == unicode('1')                                          ?
      SMuFLCodepoint(mica::TimeSignature1)                     :
    x == unicode('2')                                          ?
      SMuFLCodepoint(mica::TimeSignature2)                     :
    x == unicode('3')                                          ?
      SMuFLCodepoint(mica::TimeSignature3)                     :
    x == unicode('4')                                          ?
      SMuFLCodepoint(mica::TimeSignature4)                     :
    x == unicode('5')                                          ?
      SMuFLCodepoint(mica::TimeSignature5)                     :
    x == unicode('6')                                          ?
      SMuFLCodepoint(mica::TimeSignature6)                     :
    x == unicode('7')                                          ?
      SMuFLCodepoint(mica::TimeSignature7)                     :
    x == unicode('8')                                          ?
      SMuFLCodepoint(mica::TimeSignature8)                     :
    x == unicode('9')                                          ?
      SMuFLCodepoint(mica::TimeSignature9)                     :
    x == unicode('+') and Outer                                ?
      SMuFLCodepoint(mica::TimeSignaturePlus)                  :
    x == unicode('+') and Inner                                ?
      SMuFLCodepoint(mica::TimeSignaturePlusSmall)             :
    x == unicode('/')                                          ?
      SMuFLCodepoint(mica::TimeSignatureFractionalSlash)       :
    x == unicode('=')                                          ?
      SMuFLCodepoint(mica::TimeSignatureEquals)                :
    x == unicode('-')                                          ?
      SMuFLCodepoint(mica::TimeSignatureMinus)                 :
    x == unicode('x')                                          ?
      SMuFLCodepoint(mica::TimeSignatureMultiply)              :
    x == unicode('(') and Outer                                ?
      SMuFLCodepoint(mica::TimeSignatureParenthesisLeft)       :
    x == unicode('(') and Inner                                ?
      SMuFLCodepoint(mica::TimeSignatureParenthesisLeftSmall)  :
    x == unicode(')') and Outer                                ?
      SMuFLCodepoint(mica::TimeSignatureParenthesisRight)      :
    x == unicode(')') and Inner                                ?
      SMuFLCodepoint(mica::TimeSignatureParenthesisRightSmall) :
    x == unicode(0xBC)                                         ?
      SMuFLCodepoint(mica::TimeSignatureFractionQuarter)       :
    x == unicode(0xBD)                                         ?
      SMuFLCodepoint(mica::TimeSignatureFractionHalf)          :
    x == unicode(0xBE)                                         ?
      SMuFLCodepoint(mica::TimeSignatureFractionThreeQuarters) :
    x == unicode(0x2153)                                       ?
      SMuFLCodepoint(mica::TimeSignatureFractionThird)         :
    x == unicode(0x2154)                                       ?
      SMuFLCodepoint(mica::TimeSignatureFractionTwoThirds)     :
    x == unicode('X')                                          ?
      SMuFLCodepoint(mica::TimeSignatureX)                     :
    x == unicode('~')                                          ?
      SMuFLCodepoint(mica::OpenTimeSignature)                : x;
}

String TimeSignatureSubstituteForSMuFLSymbolInString(const String& In,
  bool Outer)
{
  String Out;
  for(count i = 0; i < In.c(); i++)
    Out << TimeSignatureSubstituteForSMuFLSymbol(In.cth(i), Outer);
  return Out;
}

#endif
///@}
bool IsInSMuFLCodepointRange(unicode c);
Pointer<Path> TimeSignatureConstructFormula(String FormulaText,
  Pointer<const Font> NotationFont, Font::Style LetterTextStyle);
Pointer<Path> TimeSignatureConstructFormulaPart(String FormulaPartText,
  Pointer<const Font> NotationFont, Font::Style LetterTextStyle);
void TimeSignatureEngrave(Music::ConstNode IslandNode,
  Music::ConstNode Token);
void TimeSignatureEngraveRegularNumeral(Music::ConstNode IslandNode,
  Pointer<const Typeface> NotationTypeface, count Numeral,
  Value Grouping, Vector Offset, mica::Concept Context,
  Music::ConstNode AssociatedTokenContext);
List<List<String> > TimeSignatureParseFormula(String Formula);
unicode TimeSignatureSubstituteForSMuFLSymbol(
  unicode x, bool Outer);
String TimeSignatureSubstituteForSMuFLSymbolInString(const String& In,
  bool Outer);
