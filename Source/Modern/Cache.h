/*
  ==============================================================================

  Copyright 2007-2013 William Andrew Burnson. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY WILLIAM ANDREW BURNSON ''AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
  EVENT SHALL WILLIAM ANDREW BURNSON OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  ------------------------------------------------------------------------------

  This file is part of Belle, Bonne, Sage --
    The 'Beautiful, Good, Wise' C++ Vector-Graphics Library for Music Notation 

  ==============================================================================
*/

#ifndef BELLE_MODERN_CACHE_H
#define BELLE_MODERN_CACHE_H

#include "House.h"

namespace BELLE_NAMESPACE { namespace modern
{
  /**Stores frequently-constructed paths. Since paths are stored not only in
  the library's native format, but also in parallel with third-party target
  formats (for example, JUCE), having paths already constructed and available
  means they can be painted immediately.*/
  struct Cache : public prim::Array<Path*>
  {
    ///Enumeration of objects with cached paths.
    enum ReusablePaths
    {
      QuarterNoteNoStem,
      QuarterNoteStemUp,
      QuarterNoteStemDown,
      HalfNoteNoStem,
      HalfNoteStemUp,
      HalfNoteStemDown,
      WholeNote,
      LedgerLineOneColumnStemUp,
      LedgerLineOneColumnStemDown,
      LedgerLineTwoColumnStemUp,
      LedgerLineTwoColumnStemDown,
      AccidentalDoubleFlat,
      AccidentalFlat,
      AccidentalNatural,
      AccidentalSharp,
      AccidentalDoubleSharp,
      RhythmicDot,
      CachedStamps
    };
    
    ///Caches all object paths.
    void Create(const House& h, const Typeface& t)
    {
      using namespace prim;
      using namespace prim::planar;
      
      //Create paths to store the objects.
      Array<Path*>& a = *this;      
      a.n(CachedStamps);
      for(count i = 0; i < a.n(); i++)
        a[i] = new Path;
      
      //QuarterNoteNoStem
      Shapes::Music::AddQuarterNote(*a[QuarterNoteNoStem], Vector(), 1.0, false,
        0, 0, h.NoteheadAngle, h.NoteheadWidth);
      
      //QuarterNoteStemUp
      Shapes::Music::AddQuarterNote(*a[QuarterNoteStemUp], Vector(), 1.0, true,
        h.DefaultStemHeight, 0, h.NoteheadAngle, h.NoteheadWidth);

      //QuarterNoteStemDown
      Shapes::Music::AddQuarterNote(*a[QuarterNoteStemDown], Vector(), 1.0,
        true, -h.DefaultStemHeight, 0, h.NoteheadAngle, h.NoteheadWidth);
      
      //HalfNoteNoStem
      Shapes::Music::AddHalfNote(*a[HalfNoteNoStem], Vector(), 1.0, false,
        0, 0, h.NoteheadAngle, h.NoteheadWidth);
      
      //HalfNoteStemUp
      Shapes::Music::AddHalfNote(*a[HalfNoteStemUp], Vector(), 1.0, true,
        h.DefaultStemHeight, 0, h.NoteheadAngle, h.NoteheadWidth);
      
      //HalfNoteStemDown
      Shapes::Music::AddHalfNote(*a[HalfNoteStemDown], Vector(), 1.0, true,
        -h.DefaultStemHeight, 0, h.NoteheadAngle, h.NoteheadWidth);
      
      //WholeNote
      Shapes::Music::AddWholeNote(*a[WholeNote], Vector(), 1.0);
      
      //LedgerLineOneColumnStemUp
      {
        Vector Left(-h.NoteheadWidthPrecise / 2. - h.LedgerLineExtraOuter, 0.);
        Vector Right(h.NoteheadWidthPrecise / 2. + h.LedgerLineExtraInner, 0.);
        Shapes::AddLine(*a[LedgerLineOneColumnStemUp],
          Left, Right, h.StaffLineThickness);
      }
      
      //LedgerLineOneColumnStemDown
      {
        Vector Left(-h.NoteheadWidthPrecise / 2. - h.LedgerLineExtraInner, 0.);
        Vector Right(h.NoteheadWidthPrecise / 2. + h.LedgerLineExtraOuter, 0.);
        Shapes::AddLine(*a[LedgerLineOneColumnStemDown],
          Left, Right, h.StaffLineThickness);
      }
      
      //LedgerLineTwoColumnStemUp
      {
        Vector Left(-h.NoteheadWidthPrecise / 2. - h.LedgerLineExtraOuter, 0.);
        Vector Right(3. * h.NoteheadWidthPrecise / 2. + h.LedgerLineExtraOuter,
          0.);
        Right.x -= h.StemWidth;
        Shapes::AddLine(*a[LedgerLineTwoColumnStemUp],
          Left, Right, h.StaffLineThickness);
      }
      
      //LedgerLineTwoColumnStemDown
      {
        Vector Left(-3. * h.NoteheadWidthPrecise / 2. - h.LedgerLineExtraOuter,
          0.);
        Vector Right(h.NoteheadWidthPrecise / 2. + h.LedgerLineExtraOuter,
          0.);
        Left.x += h.StemWidth;
        Shapes::AddLine(*a[LedgerLineTwoColumnStemDown],
          Left, Right, h.StaffLineThickness);
      }
      
      /*
      AccidentalDoubleFlat
      AccidentalFlat
      AccidentalNatural
      AccidentalSharp
      AccidentalDoubleSharp
      */
      {
        prim::count g = 0x0045;
        Affine s = Affine::Scale(4.0);
        *a[AccidentalDoubleFlat] =
          Path(*t.LookupGlyph(g + 0), s);
        *a[AccidentalFlat] =
          Path(*t.LookupGlyph(g + 1), s);
        *a[AccidentalNatural] =
          Path(*t.LookupGlyph(g + 2), s);
        *a[AccidentalSharp] =
          Path(*t.LookupGlyph(g + 3), s);
        *a[AccidentalDoubleSharp] =
          Path(*t.LookupGlyph(g + 4), s);
      }
      
      //RhythmicDot
      Shapes::AddCircle(*a[RhythmicDot], Vector(), h.RhythmicDotSize);
    }    
    ///Destructor deletes the cached paths.
    ~Cache() {ClearAndDeleteAll();}
  };
}}
#endif
