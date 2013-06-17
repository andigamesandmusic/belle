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

#ifndef BELLE_MODERN_HOUSE_H
#define BELLE_MODERN_HOUSE_H

namespace BELLE_NAMESPACE { namespace modern
{
  /**House style for engraving. All constants used in the engraving process are
  stored here. The class can also be inherited to provide custom constants.*/
  struct House
  {
    ///Height of a single space in inches
    prim::number SpaceHeight;
    
    ///Distance between staves
    prim::number StaffDistance;
    
    ///Default stem height when stems are not explicitly adjusted
    prim::number DefaultStemHeight;
    
    ///Maximum number of dots to consider typesetting on a note
    prim::count MaxDotsToConsider;
    
    ///Staff line thickness as fraction of space height
    prim::number StaffLineThickness;
    
    ///Notehead width as a proportion of width to height
    prim::number NoteheadWidth;
    
    ///Notehead angle
    prim::number NoteheadAngle;
    
    ///Notehead precise width when taking into account angle
    prim::number NoteheadWidthPrecise;
    
    ///Note stem proportion
    prim::number StemWidth;
    
    ///Note stem cap height
    prim::number StemCapHeight;
    
    ///Note stem height in line spaces.
    prim::count StemHeight;
        
    ///Extra hanging width of ledger line end closest to stem
    prim::number LedgerLineExtraInner;

    ///Extra hanging width of ledger line end furthest from stem
    prim::number LedgerLineExtraOuter;
    
    ///Proportional amount to scrunch spacing on ledger lined notes
    prim::number LedgerLineScrunch;
    
    ///Amount of optional gap between ledger lined notes and staff notes
    prim::number LedgerLineGap;
    
    ///Amount of extra spacing in between accidentals
    prim::number AccidentalExtraSpacing;
    
    ///Size of rhythmic dots
    prim::number RhythmicDotSize;
    
    ///Distance from end of notehead to rhythmic dot
    prim::number RhythmicDotNoteheadDistance;
    
    ///Spacing between rhythmic dots
    prim::number RhythmicDotSpacing;
    
    ///Thickness of barlines
    prim::number BarlineThickness;
    
    ///Proportional size of a non initial clef.
    prim::number NonInitialClefSize;
    
    ///Default set of reasonable style choices
    void Default()
    {
      SpaceHeight = 0.05;
      StaffDistance = 12.0;
      DefaultStemHeight = 7.5;//3.5;
      MaxDotsToConsider = 4;
      StaffLineThickness = 0.14;
      BarlineThickness = 0.2;
      NoteheadWidth = 1.4;
      NoteheadAngle = 20.0 * prim::Deg;
      NoteheadWidthPrecise = prim::planar::Ellipse::VerticalTangent(
        NoteheadWidth, 1.0f, NoteheadAngle).x;
      StemWidth = 0.14;
      StemCapHeight = 0.8;
      StemHeight = 7;
      LedgerLineExtraInner = 0.25;
      LedgerLineExtraOuter = 0.20;
      LedgerLineScrunch = 0.96;
      LedgerLineGap = 0.1;
      AccidentalExtraSpacing = 0.1;
      RhythmicDotSize = 0.4;
      RhythmicDotNoteheadDistance = 1.1;
      RhythmicDotSpacing = 1.0;
      NonInitialClefSize = 0.8;
    }

    ///Overload and add custom constant definitions.
    virtual void Custom() {}
    
    ///Constructor to apply default settings and then any custom settings.
    House()
    {
      Default();      
      Custom();
    }
    
    ///Virtual destructor
    virtual ~House() {}
  };
}}
#endif
