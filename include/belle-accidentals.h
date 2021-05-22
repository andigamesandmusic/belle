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

unicode SMuFLCodepoint(mica::Concept x);
Array<count> DefaultOrderForAccidentalStack(count n);
Pointer<const Glyph> SMuFLGlyphFromTypeface(Pointer<const Typeface> t,
  mica::Concept x);
Pointer<const Glyph> SMuFLGlyphFromCodepoint(Pointer<const Font> f, unicode u);
Pointer<const Glyph> SMuFLGlyph(Pointer<const Font> f, mica::Concept x);
Array<Pointer<const Glyph> > AccidentalStackPaths(Pointer<const Font> f,
  const Value& AccidentalList);
Array<Vector> PlacementForAccidentalStack(
  const Array<Pointer<const Glyph> >& AccidentalPaths,
  const Stamp& Noteheads, Value& AccidentalList,
  const Array<count>& AccidentalStack);
void PlaceAccidentals(const Value &AccidentalList,
  const Array<Pointer<const Glyph> >& AccidentalPaths, Stamp& Noteheads);

#ifdef BELLE_IMPLEMENTATION
Pointer<const Glyph> SMuFLGlyphFromTypeface(Pointer<const Typeface> t,
  mica::Concept x)
{
  return t ? t->LookupGlyph(SMuFLCodepoint(x)).Const() :
    Pointer<const Glyph>();
}

Pointer<const Glyph> SMuFLGlyph(Pointer<const Font> f, mica::Concept x)
{
  return f ? SMuFLGlyphFromTypeface(f->GetTypeface(Font::Notation), x) :
    Pointer<const Glyph>();
}

Pointer<const Glyph> SMuFLGlyphFromCodepoint(Pointer<const Font> f, unicode u)
{
  return (f and f->GetTypeface(Font::Notation)) ?
    f->GetTypeface(Font::Notation)->LookupGlyph(u).Const() :
    Pointer<const Glyph>();
}

unicode SMuFLCodepoint(mica::Concept x)
{
  return mica::integer(x) ? unicode(numerator(x)) :
    unicode(mica::numerator(mica::map(mica::SMuFL, x)));
}

Array<Pointer<const Glyph> > AccidentalStackPaths(Pointer<const Font> f,
  const Value& AccidentalList)
{
  Array<Pointer<const Glyph> > x(AccidentalList.n());
  for(count i = 0; i < x.n(); i++)
    x[i] = SMuFLGlyph(f, AccidentalList[i]["Accidental"]);
  return x;
}

Array<count> DefaultOrderForAccidentalStack(count n)
{
  Array<count> v(n);
  for(count i = 0; i < n; i++)
    v[i] = i % 2 == 0 ? i / 2 : n - 1 - i / 2;
  return v;
}

///Put Array<Vector> in AccidentalList
Array<Vector> PlacementForAccidentalStack(
  const Array<Pointer<const Glyph> >& AccidentalPaths,
  const Stamp& Noteheads, Value& AccidentalList,
  const Array<count>& AccidentalStack)
{
  Array<Vector> Placement(AccidentalStack.n());
  Array<Box> AccumulatingBounds = Noteheads.GetGraphicBounds();

  //Stack accidentals incrementally to the left of the chord.
  for(count i = 0; i < AccidentalStack.n(); i++)
  {
    //Lookup the accidental to place next.
    count PathIndex = AccidentalStack[i];

    //Skip accidental if it is not visible.
    if(!AccidentalPaths[PathIndex]) continue;

    //Calculate the vertical position of the accidental.
    number VerticalPosition =
      EngraverUtility::GetLineSpacePosition(
      AccidentalList[PathIndex]["StaffPosition"].AsCount());

    //Get the bounds of the accidental.
    Array<Box> AccidentalBounds =
      AccidentalPaths[PathIndex]->BoundsOfCurves(
      Affine::Translate(Vector(0.f, VerticalPosition)));

    //Calculate the hulls.
    List<Vector> AccumulatingHull = Box::SegmentedHull(
      AccumulatingBounds, Box::LeftSide);
    List<Vector> AccidentalHull = Box::SegmentedHull(
      AccidentalBounds, Box::RightSide);

    //Calculate the placement offset.
    Vector XOffset = Box::OffsetToPlaceOnSide(AccumulatingHull,
      AccidentalHull, Box::LeftSide);

    //Add just a little bit of extra space.
    XOffset.x -= 0.3f;

    Vector AccidentalPlacement(XOffset.x, VerticalPosition);

    //Update the accidental placement for this accidental.
    Placement[PathIndex] = AccidentalPlacement;

    //Add the bounds of the accidental.
    for(count j = 0; j < AccidentalBounds.n(); j++)
      AccumulatingBounds.Add() = Box(
        AccidentalBounds[j].a + XOffset, AccidentalBounds[j].b + XOffset);

    AccidentalList[PathIndex]["Placement"] = AccidentalPlacement;
  }

  return Placement;
}

void PlaceAccidentals(const Value &AccidentalList,
  const Array<Pointer<const Glyph> >& AccidentalPaths, Stamp& Noteheads)
{
  for(count i = 0; i < AccidentalList.n(); i++)
  {
    Vector AccidentalPlacement = AccidentalList[i]["Placement"].AsVector();
    Pointer<const Glyph> Accidental = AccidentalPaths[i];
    Noteheads.Add()->p = Accidental;
    Noteheads.z()->a = Affine::Translate(AccidentalPlacement);
  }
}
#endif
