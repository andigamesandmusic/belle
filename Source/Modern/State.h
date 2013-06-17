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

#ifndef BELLE_MODERN_STATE_H
#define BELLE_MODERN_STATE_H

#include "Chord.h"

namespace BELLE_NAMESPACE { namespace modern
{
  //Ongoing part-wise state for the island engraver.
  struct State
  {
    mica::UUID ActiveClef;
    Chord::State Previous;
    Chord::State Current;
    mica::UUID NextAccidentals[7];
    mica::UUID ActiveAccidentals[7];
    mica::UUID KeyAccidentals[7];
    
    mica::UUID ConsumeAccidental(prim::count s, mica::UUID a)
    {
#if 0 //For testing all accidentals
      return a;
#endif
      //Does not take into account unisons with different accidentals yet.
      mica::UUID Note = mica::map(ActiveClef,
        mica::M(mica::LineSpaces).Item(s, mica::LS0));
      mica::UUID Letter = mica::map(Note, mica::Letter);
      prim::count LetterIndex = mica::index(mica::Letters, Letter);
      
      if(ActiveAccidentals[LetterIndex] == a)
      {
        //Accidental is redundant in scope as it already appeared.
        return mica::Undefined;
      }
      else
      {
        /*Save accidental as part of the current state and emit accidental,
        however save it to a buffer so that accidentals of other octaves have
        a chance to use the same accidental.*/
        NextAccidentals[LetterIndex] = a;
        return a;
      }
    }
    
    void SetKeySignature(mica::UUID k)
    {
      for(prim::count i = 0; i < 7; i++)
        KeyAccidentals[i] = mica::Natural;
      
      mica::UUID AccidentalType = mica::map(mica::Accidental, k);
      prim::count NumberOfAccidentals =
        prim::Abs(mica::index(mica::KeySignatures, k, mica::NoAccidentals));
      
      for(prim::count i = 0; i < NumberOfAccidentals; i++)
      {
        mica::UUID KeySignatureSequence = mica::map(AccidentalType, ActiveClef);
        mica::UUID LineSpace = mica::M(KeySignatureSequence).Item(i);
        mica::UUID Note = mica::map(ActiveClef, LineSpace);
        mica::UUID Letter = mica::map(Note, mica::Letter);
        prim::count LetterIndex = mica::index(mica::Letters, Letter);
        KeyAccidentals[LetterIndex] = AccidentalType;
      }
    }
    
    void AdvanceAccidentalState()
    {
      for(prim::count i = 0; i < 7; i++)
        ActiveAccidentals[i] = NextAccidentals[i];
    }
    
    void ResetActiveAccidentalsToKeySignature()
    {
      for(prim::count i = 0; i < 7; i++)
        NextAccidentals[i] = ActiveAccidentals[i] = KeyAccidentals[i];
    }
    
    State() : ActiveClef(mica::Undefined)
    {
      for(prim::count i = 0; i < 7; i++)
        NextAccidentals[i] = ActiveAccidentals[i] = KeyAccidentals[i] = 
          mica::Undefined;
    }
  };
}}
#endif
