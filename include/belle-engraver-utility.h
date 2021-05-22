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

#ifndef BELLE_ENGRAVER_ENGRAVERUTILITY_H
#define BELLE_ENGRAVER_ENGRAVERUTILITY_H

namespace BELLE_NAMESPACE
{
  ///Context-less static conversion methods
  class EngraverUtility
  {
    public:

    ///Gets the base notehead value without dots applied.
    static Ratio GetUndottedValue(Ratio c)
    {
      //Round down to nearest power-of-two ratio or inverse ratio.
      Ratio Value = 1;
      while(Value < c)
        Value *= 2;
      while(Value > c)
        Value /= 2;
      return Value;
    }

    ///Determines the number of dots for a rhythm.
    static count CountDots(Ratio Duration, count MaxDotsToConsider)
    {
      Ratio Base = GetUndottedValue(Duration);
      if(Base == Duration)
        return 0;
      Ratio t = 1;
      for(count i = 1; i <= MaxDotsToConsider; i++)
      {
        t *= Ratio(1, 2);
        Ratio c = Base * (Ratio(2) - t);
        if(c == Duration)
          return i;
      }
      return -1; //Unexpected rhythm encountered
    }

    ///Determines the number of flags for a rhythm.
    static count CountFlags(Ratio Duration)
    {
      Ratio Base = GetUndottedValue(Duration);
      count Flags = 0;
      while(Base < Ratio(1, 4))
      {
        Flags++;
        Base *= 2;
      }
      return Flags;
    }

    ///Gets the notehead for the given note and rhythm.
    static Pointer<const Path> GetNotehead(Music::ConstNode Island, Ratio r)
    {
      Ratio h = GetUndottedValue(r);
      if(h <= Ratio(1, 4))
        return HouseStyle::GetCached(Island, "QuarterNoteNoStem");
      else if(h == Ratio(1, 2))
        return HouseStyle::GetCached(Island, "HalfNoteNoStem");
      else
        return HouseStyle::GetCached(Island, "WholeNote");
    }

    static mica::Concept GetLineSpace(count i)
    {
      return mica::Concept(Ratio(i, 1));
    }

    static count GetLineSpaceIndex(mica::Concept LineSpace)
    {
      return count(numerator(LineSpace));
    }

    ///Gets the line/space of the top line for a given number of staff lines.
    static count GetTopLine(count StaffLines)
    {
      return StaffLines - 1;
    }

    ///Gets the line/space of the bottom line for a given number of staff lines.
    static count GetBottomLine(count StaffLines)
    {
      return -StaffLines + 1;
    }

    ///Rounds the current line-space up to the next space.
    static count RoundUpToNextSpace(count s, count StaffLines)
    {
      if(StaffLines % 2)
        return (s % 2) ? s : s + 1;
      else
        return (s % 2) ? s + 1 : s;
    }

    ///Converts a line space into vertical position.
    static number GetLineSpacePosition(number s)
    {
      return s / 2.f;
    }

    ///Gets the number of accidentals in a given key signature.
    static count GetNumberOfAccidentals(mica::Concept KeySignature)
    {
      return count(Abs(numerator(index(mica::KeySignatures,
        mica::NoAccidentals, KeySignature))));
    }

    ///Gets the cleffed-position of an accidental in a key signature.
    static count GetAccidentalPosition(mica::Concept KeySignature,
      mica::Concept Clef, count i)
    {
      /*Get the sequence of accidental positions for the combination of clef
      and accidental.*/
      mica::Concept s = map(Clef, map(KeySignature, mica::Accidental));
      return count(numerator(item(s, i)));
    }
  };
}
#endif
