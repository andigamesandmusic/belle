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

#ifndef BELLE_ENGRAVER_CACHE_H
#define BELLE_ENGRAVER_CACHE_H

namespace BELLE_NAMESPACE
{
  ///Stores pointers to frequently-constructed paths.
  class Cache
  {
    ///Adds a path to the cache at the given key.
    static Path& AddPathAt(Value& CacheKey, const String& Key)
    {
      CacheKey[Key] = new Path;
      return *Pointer<Path>(CacheKey[Key].Object());
    }

    public:

    ///Initializes the cache from the house style and font.
    static void Initialize(Value& CacheKey, Value& HouseStyleKey,
      Pointer<const Font> FontToUse)
    {
      Value& h = HouseStyleKey;

      //Clear the cache.
      CacheKey.Clear();

      //If no typeface was passed in, then abort leaving the cached empty paths.
      if(!FontToUse)
        return;
      Pointer<const Typeface> ty = FontToUse->GetTypeface(Font::Notation);
      if(!ty)
        return;

      Shapes::Music::AddQuarterNote(AddPathAt(CacheKey, "QuarterNoteNoStem"),
        Vector(), +h["BlackNoteheadSize"], false, 0, 0,
        +h["BlackNoteheadAngle"], +h["BlackNoteheadWidth"]);

      Shapes::Music::AddQuarterNote(AddPathAt(CacheKey, "QuarterNoteStemUp"),
        Vector(), +h["BlackNoteheadSize"], true, h["DefaultStemHeight"], 0,
        h["BlackNoteheadAngle"], h["BlackNoteheadWidth"]);

      Shapes::Music::AddQuarterNote(AddPathAt(CacheKey, "QuarterNoteStemDown"),
        Vector(), +h["BlackNoteheadSize"], true, -h["DefaultStemHeight"], 0,
        h["BlackNoteheadAngle"], h["BlackNoteheadWidth"]);

      Shapes::Music::AddHalfNote(AddPathAt(CacheKey, "HalfNoteNoStem"),
        Vector(), +h["WhiteNoteheadSize"], false, 0, 0,
        h["WhiteNoteheadAngle"], h["WhiteNoteheadWidth"]);

      Shapes::Music::AddHalfNote(AddPathAt(CacheKey, "HalfNoteStemUp"),
        Vector(), +h["WhiteNoteheadSize"], true, h["DefaultStemHeight"], 0,
        h["WhiteNoteheadAngle"], h["WhiteNoteheadWidth"]);

      Shapes::Music::AddHalfNote(AddPathAt(CacheKey, "HalfNoteStemDown"),
        Vector(), +h["WhiteNoteheadSize"], true, -h["DefaultStemHeight"], 0,
        h["WhiteNoteheadAngle"], h["WhiteNoteheadWidth"]);

      Shapes::Music::AddWholeNote(AddPathAt(CacheKey, "WholeNote"), Vector(),
        1.f);

      {
        Vector Left(-h["WhiteNoteheadWidthPrecise"] / 2.f -
          +h["LedgerLineExtraOuter"], 0.f);
        Vector Right(+h["WhiteNoteheadWidthPrecise"] / 2.f +
          +h["LedgerLineExtraInner"], 0.f);
        Shapes::AddLine(AddPathAt(CacheKey, "LedgerLineOneColumnStemUp"),
          Left, Right, +h["StaffLineThickness"] *
          +h["LedgerLineRelativeThickness"]);
      }

      {
        Vector Left(-h["WhiteNoteheadWidthPrecise"] / 2.f -
          +h["LedgerLineExtraInner"], 0.f);
        Vector Right(+h["WhiteNoteheadWidthPrecise"] / 2.f +
          +h["LedgerLineExtraOuter"], 0.f);
        Shapes::AddLine(AddPathAt(CacheKey, "LedgerLineOneColumnStemDown"),
          Left, Right, +h["StaffLineThickness"] *
          +h["LedgerLineRelativeThickness"]);
      }

      {
        Vector Left(-h["WhiteNoteheadWidthPrecise"] / 2.f -
          +h["LedgerLineExtraOuter"], 0.f);
        Vector Right(3.f * +h["WhiteNoteheadWidthPrecise"] / 2.f +
          +h["LedgerLineExtraOuter"], 0.f);
        Right.x += -h["StemWidth"];
        Shapes::AddLine(AddPathAt(CacheKey, "LedgerLineTwoColumnStemUp"),
          Left, Right, +h["StaffLineThickness"] *
          +h["LedgerLineRelativeThickness"]);
      }

      {
        Vector Left(-3.f * +h["WhiteNoteheadWidthPrecise"] / 2.f -
          +h["LedgerLineExtraOuter"], 0.f);
        Vector Right(+h["WhiteNoteheadWidthPrecise"] / 2.f +
          +h["LedgerLineExtraOuter"], 0.f);
        Left.x += +h["StemWidth"];
        Shapes::AddLine(AddPathAt(CacheKey, "LedgerLineTwoColumnStemDown"),
          Left, Right, +h["StaffLineThickness"] *
          +h["LedgerLineRelativeThickness"]);
      }

      AddPathAt(CacheKey, "AccidentalDoubleFlat") =
        Path(*SMuFLGlyphFromTypeface(ty, mica::DoubleFlat));
      AddPathAt(CacheKey, "AccidentalFlat") =
        Path(*SMuFLGlyphFromTypeface(ty, mica::Flat));
      AddPathAt(CacheKey, "AccidentalNatural") =
        Path(*SMuFLGlyphFromTypeface(ty, mica::Natural));
      AddPathAt(CacheKey, "AccidentalSharp") =
        Path(*SMuFLGlyphFromTypeface(ty, mica::Sharp));
      AddPathAt(CacheKey, "AccidentalDoubleSharp") =
        Path(*SMuFLGlyphFromTypeface(ty, mica::DoubleFlat));

      Shapes::AddCircle(AddPathAt(CacheKey, "RhythmicDot"), Vector(),
        h["RhythmicDotSize"]);

      AddPathAt(CacheKey, "TrebleClef") =
        Path(*SMuFLGlyphFromTypeface(ty, mica::GClef));
      AddPathAt(CacheKey, "BassClef") =
        Path(*SMuFLGlyphFromTypeface(ty, mica::FClef));
    }

    ///Grid visualization of the cache in a 1x1 square.
    static void Visualize(Painter& Painter, const Value& CacheKey,
      number InteriorScale = 0.12f)
    {
      //Make sure there is something in the cache.
      if(!CacheKey.n()) return;

      //Calculate the number of columns so that the grid forms a square.
      count Columns = count(Ceiling(Sqrt(number(CacheKey.n()))));

      //Show a grid.
      Path p;
      Shapes::AddGrid(p, Vector(1.f, 1.f),
        VectorInt(integer(Columns), integer(Columns)));
      Painter.Draw(p);

      //Show each item in the cache.
      ScopedAffine g(Painter, Affine::Scale(1.f / number(Columns)));
      Array<Value> CacheKeys;
      CacheKey.EnumerateKeys(CacheKeys);
      for(count i = 0; i < CacheKeys.n(); i++)
      {
        number x = number(i % Columns) + 0.5f;
        number y = number(i / Columns) + 0.5f;
        ScopedAffine a(Painter, Affine::Translate(Vector(x, y)));
        ScopedAffine s(Painter, Affine::Scale(InteriorScale));
        Painter.Draw(CacheKey[CacheKeys[i]].ConstObject());
      }
    }
  };
}
#endif
