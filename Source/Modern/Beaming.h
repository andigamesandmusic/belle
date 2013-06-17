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

#ifndef BELLE_MODERN_BEAMING_H
#define BELLE_MODERN_BEAMING_H

#include "Directory.h"

namespace BELLE_NAMESPACE {

/**The "traditional Western music notation" dialect will instead be called
"modern" in the sense of: "denoting the form of a language that is currently
used, as opposed to any earlier form." Modern refers not only to the recent
innovations in notation, but includes the culmination of the notational language
as we know it.*/
namespace modern
{
  //Forward declaration.
  class Beam;
  
  class Stem
  {
    protected:
      //Cached info
      mutable prim::count LevelsAbove;
      mutable prim::count LevelsBelow;
      
    public:
      /**Describes the direction of the stem. If true, the stem will go from a
      note up to the beam. If false, the stem will go from the note down to the
      beam.*/
      bool StemUp;
      
      ///Describes the number of extra beams besides the 8th beam.
      prim::count ExtraLevels;
      
      ///The positive distance to the next stem (if there is one; if not, zero).
      prim::number UnitsNextStemDistance;
      
      /**The location of the current stem. This is an intermediate value used
      internally.*/
      prim::planar::Vector UnitsStemLocation;
      
      ///If true, then no extra beams will connect to the left of this stem.
      bool BeginsGroup;
      
      ///Default constructor zeroes fields.
      Stem() : StemUp(false), ExtraLevels(0), UnitsNextStemDistance(0), 
        BeginsGroup(false) {}
      
      ///Constructor to initialize fields.
      Stem(bool StemUp, prim::count ExtraLevels,
        prim::number UnitsNextStemDistance, bool BeginsGroup = false) :
        StemUp(StemUp), ExtraLevels(ExtraLevels),
        UnitsNextStemDistance(UnitsNextStemDistance), BeginsGroup(BeginsGroup)
        {}
        
    friend class Beam;
  };
  
  class Beam
  {
    protected:
    ///The highest level observed by Prepare().
    mutable prim::count HighestLevel;
    
    ///The lowest level observed by Prepare().
    mutable prim::count LowestLevel;
    
    public:
    ///An array of stems which contain information about how to beam the levels.
    prim::Array<Stem> Stems;
    
    ///The baseline of the primary beam.
    prim::planar::Line UnitsBaseline;
    
    ///The thickness of each beam.
    prim::number UnitsLevelThickness;
    
    /**The positive distance of beams from one level to the next. The distance
    is measured from center of the beams at any common value on the construction
    plane's x-axis.*/
    prim::number UnitsLevelSpacing;
    
    ///Width of the flag that juts out of an unconnected beam.
    prim::number UnitsFlagWidth;
    
    ///Constructor to create a number of stems.
    Beam(prim::count NumberOfStems)
    {
      Stems.n(NumberOfStems);
    }
    
    ///Default constructor initializes with no stems.
    Beam()
    {
    }
    
    void Prepare()
    {
      prim::count Stems_n = Stems.n();
      
      //Make sure there are enough stems to continue.
      if(Stems_n < 2)
        return;
      
      //Make sure that all the level values are positive numbers.
      for(prim::count i = 0; i < Stems_n; i++)
      {
        Stem& ithStem = Stems[i];
        ithStem.ExtraLevels =
          (prim::count)prim::Abs((prim::integer)ithStem.ExtraLevels);
      }
      
      //-----------------------//
      //Algorithm: Beam Sharing//
      //-----------------------//
      
      //Determine the beam levels.
      for(prim::count i = 0; i < Stems_n; i++)
      {
        Stem& s = Stems[i];
        if(i == 0 || s.BeginsGroup) /*Beginning of group does not depend on
        prior beams since beams are not shared.*/
        {
          if(s.StemUp) //Current stem goes from note up to beam.
          {
            s.LevelsAbove = 0;
            s.LevelsBelow = -s.ExtraLevels;
          }
          else //Current stem goes from note down to beam.
          {
            s.LevelsAbove = s.ExtraLevels;
            s.LevelsBelow = 0;
          }
        }
        else
        {
          Stem& p = Stems[i - 1];
          if(s.StemUp) //Current stem goes from note up to beam.
          {
            if(p.LevelsAbove == 0) //All previous beams were below primary beam.
            {
              s.LevelsAbove = 0;
              s.LevelsBelow = -s.ExtraLevels;
            }
            else //Some previous beams were above.
            {
              if(s.ExtraLevels <= p.LevelsAbove)
              {
                if(!p.StemUp)
                {
                  /*Start at primary beam and share as many below as possible,
                  then come above the primary beam to share the rest.*/
                  if(s.ExtraLevels <= -p.LevelsBelow)
                  {
                    s.LevelsAbove = 0;
                    s.LevelsBelow = -s.ExtraLevels;
                  }
                  else
                  {
                    s.LevelsAbove = p.LevelsBelow + s.ExtraLevels;
                    s.LevelsBelow = p.LevelsBelow;
                  }
                }
                else
                {
                  /*Start at primary beam and share as many above as possible,
                  then dip below the primary beam to share the rest.*/
                  if(s.ExtraLevels <= p.LevelsAbove)
                  {
                    s.LevelsAbove = s.ExtraLevels;
                    s.LevelsBelow = 0;
                  }
                  else
                  {
                    s.LevelsAbove = p.LevelsAbove;
                    s.LevelsBelow = p.LevelsAbove - s.ExtraLevels;
                  }
                }
              }
              else
              {
                //Share beams above the primary first, and then the rest below.
                s.LevelsAbove = p.LevelsAbove;
                s.LevelsBelow = p.LevelsAbove - s.ExtraLevels;
              }
            }
          }
          else //Current stem goes from note down to beam.
          {
            if(p.LevelsBelow == 0) //All previous beams were above primary beam.
            {
              s.LevelsAbove = s.ExtraLevels;
              s.LevelsBelow = 0;
            }
            else //Some previous beams were below.
            {
              if(s.ExtraLevels <= -p.LevelsBelow)
              {
                if(p.StemUp)
                {
                  /*Start at primary beam and share as many above as possible,
                  then dip below the primary beam to share the rest.*/
                  if(s.ExtraLevels <= p.LevelsAbove)
                  {
                    s.LevelsAbove = s.ExtraLevels;
                    s.LevelsBelow = 0;
                  }
                  else
                  {
                    s.LevelsAbove = p.LevelsAbove;
                    s.LevelsBelow = p.LevelsAbove - s.ExtraLevels;
                  }
                }
                else
                {
                  /*Start at primary beam and share as many below as possible,
                  then come above the primary beam to share the rest.*/
                  if(s.ExtraLevels <= -p.LevelsBelow)
                  {
                    s.LevelsAbove = 0;
                    s.LevelsBelow = -s.ExtraLevels;
                  }
                  else
                  {
                    s.LevelsAbove = p.LevelsBelow + s.ExtraLevels;
                    s.LevelsBelow = p.LevelsBelow;
                  }
                }
              }
              else
              {
                //Share beams below the primary first, and then the rest above.
                s.LevelsAbove = p.LevelsBelow + s.ExtraLevels;
                s.LevelsBelow = p.LevelsBelow;
              }
            }
          }
        }
      }
      
      //Update the highest and lowest beam levels found.
      HighestLevel = 0;
      LowestLevel = 0;
      for(prim::count i = 0; i < Stems_n; i++)
      {
        prim::MakeAtLeast(HighestLevel, Stems[i].LevelsAbove);
        prim::MakeAtMost(LowestLevel, Stems[i].LevelsBelow);
      }
    }
    
    bool HasBeam(prim::count LeftStemIndex, prim::count Level) const
    {
      //If out of range then there can be no beam.
      if(LeftStemIndex < 0 || LeftStemIndex >= Stems.n() - 1)
        return false;
      
      /*If the primary beam, then it is automatically beamed without further
      consideration.*/
      if(Level == 0)
        return true;
      
      //Get a reference to the current and next stems.
      const Stem& s = Stems.ith(LeftStemIndex);
      const Stem& n = Stems.ith(LeftStemIndex + 1);
      if(prim::IsBetween(Level, s.LevelsBelow, s.LevelsAbove) &&
        prim::IsBetween(Level, n.LevelsBelow, n.LevelsAbove))
        return true; //The two adjacent stems share a beam.
      else
        return false; //The adjacent stems do not share a beam.
    }
    
    bool HasLeftFlag(prim::count StemIndex, prim::count Level) const
    {
      /*Algorithm note: left flags are preferred over right flags when there is
      no other visual cue that would indicate one direction or the other.*/
      
      //Eighth-level is always beamed.
      if(Level == 0)
        return false;

      //No left flags on the first stem or non-existent ones.
      if(StemIndex <= 0 || StemIndex >= Stems.n())
        return false;

      //Get a reference to the stem.
      const Stem& p = Stems.ith(StemIndex - 1);
      const Stem& s = Stems.ith(StemIndex);
      
      //If the first in a group, then there is automatically no left flag.
      if(s.BeginsGroup)
        return false;
        
      //If beamed to the next stem, then a flag might be unnecessary.
      if(HasBeam(StemIndex, Level))
      {
        if(s.StemUp)
        {
          //See if there are any left flags below this level on this stem.
          if(Level >= 0 || Level == s.LevelsBelow || 
            !HasLeftFlag(StemIndex, Level - 1))
              return false;
        }
        else
        {
          //See if there are any left flags above this level on this stem.
          if(Level <= 0 || Level == s.LevelsAbove || 
            !HasLeftFlag(StemIndex, Level + 1))
              return false;
        }
      }
      
      //Make sure that the level is actually in use for this stem.
      if(!prim::IsBetween(Level, s.LevelsBelow, s.LevelsAbove))
        return false;
      
      //Detect beam sharing as the final decision.
      if(prim::IsBetween(Level, p.LevelsBelow, p.LevelsAbove))
        return false; //Flag unnecessary since beam can be shared.
      else
        return true; //Flag necessary since beam can not be shared.
    }
    
    bool HasRightFlag(prim::count StemIndex, prim::count Level) const
    {
      /*Right flag is a fallback in case there is neither a left flag or a beam,
      yet, the level exists.*/
      
      /*See if the stem index is in range. Also last stem can not have right
      flag.*/
      if(StemIndex < 0 || StemIndex >= Stems.n() - 1)
        return false;
      
      //Get a reference to the current and next stems.
      const Stem& s = Stems.ith(StemIndex);
      //const Stem& n = Stems.ith(StemIndex + 1);
      
      /*If the level is valid and this is neither left flagged or beamed, then
      this stem is right flagged.*/
      if(prim::IsBetween(Level, s.LevelsBelow, s.LevelsAbove) &&
        !HasLeftFlag(StemIndex, Level) && !HasBeam(StemIndex, Level)
         && !HasBeam(StemIndex - 1, Level))
          return true;
      else
        return false; //Right flag not applicable or not necessary.
    }
    
    /**Returns a number, whose sign indicates the overall direction of the stems.
    If all stems point up from the notes, returns a positive number. If all stems
    point down, returns a negative number. If the direction is mixed or there are
    no stems, returns zero.*/
    prim::count GetGeneralStemDirection() const
    {
      bool HasUpStems = false;
      bool HasDownStems = false;
      
      for(prim::count i = Stems.n() - 1; i >= 0; i--)
      {
        if(Stems[i].StemUp)
          HasUpStems = true;
        else
          HasDownStems = true;
      }
      
      if(HasUpStems && HasDownStems)
        return 0;
      else if(HasUpStems)
        return 1;
      else if(HasDownStems)
        return -1;
      else
        return 0;
    }
  
    ///Paints a single beamed segment as a parallelogram.
    void PaintBeamSegment(Painter* p, prim::planar::Line l,
      prim::number Height)
    {
      prim::number HalfHeight = Height * 0.5f;
      
      prim::planar::Vector tl(l.a.x, l.a.y + HalfHeight),
        bl(l.a.x, l.a.y - HalfHeight),
        tr(l.b.x, l.b.y + HalfHeight),
        br(l.b.x, l.b.y - HalfHeight);
        
      Path Parallelogram;
      Parallelogram.Add(Instruction(tl, true));
      Parallelogram.Add(Instruction(tr));
      Parallelogram.Add(Instruction(br));
      Parallelogram.Add(Instruction(bl));
      
      p->Draw(Parallelogram);
    }
    
    ///Paints the beam specified by the structure.
    void Paint(Painter* p)
    {
      //Update the information.
      Prepare();
      
      //No beams to paint if beam slant is vertical.
      if(UnitsBaseline.a.x == UnitsBaseline.b.x)
        return;
      
      //Calculate the slope of the beam.
      prim::number Slope = (UnitsBaseline.b.y - UnitsBaseline.a.y) / 
        (UnitsBaseline.b.x - UnitsBaseline.a.x);
      
      prim::number x = UnitsBaseline.a.x;
      prim::number y = UnitsBaseline.a.y;
      
      for(prim::count i = 0; i < Stems.n(); i++)
      {
        Stem& Stem = Stems[i];
        
        //Draw ticks for now instead of stems.
        //...
        /*if(Stem.StemUp)
        {
          p->DrawLine(prim::planar::Vector(x, y + UnitsLevelThickness * 0.5f +
            (prim::number)Stem.LevelsAbove * UnitsLevelSpacing),
            prim::planar::Vector(x, y - 0.4f), 0.01f);
        }
        else
        {
          p->DrawLine(prim::planar::Vector(x, y - UnitsLevelThickness * 0.5f +
            (prim::number)Stem.LevelsBelow * UnitsLevelSpacing),
            prim::planar::Vector(x, y + 0.4f), 0.01f);
        }*/
        //...
        
        for(prim::count j = LowestLevel; j <= HighestLevel; j++)
        {
          prim::number UnitsLevelY = y + UnitsLevelSpacing * (prim::number)j;
          
          if(HasLeftFlag(i, j))
          {
            //Create flag on left.
            prim::planar::Line l(x - UnitsFlagWidth,
              UnitsLevelY - UnitsFlagWidth * Slope, x, UnitsLevelY);
            PaintBeamSegment(p, l, UnitsLevelThickness);
          }
          if(HasRightFlag(i, j))
          {
            //Create flag on right.
            prim::planar::Line l(x, UnitsLevelY,
              x + UnitsFlagWidth, UnitsLevelY + UnitsFlagWidth * Slope);
            PaintBeamSegment(p, l, UnitsLevelThickness);
          }
          if(HasBeam(i, j))
          {
            //Create beam.
            prim::planar::Line l(x, UnitsLevelY, x + Stem.UnitsNextStemDistance,
              UnitsLevelY + Stem.UnitsNextStemDistance * Slope);
            PaintBeamSegment(p, l, UnitsLevelThickness);
          }
          /*
          else
          {
            //There is no beam at this level.
          }
          */
        }

        prim::number delta = Stem.UnitsNextStemDistance;
        x += delta;
        y += delta * Slope;
      }
    }
  };
}}
#endif
