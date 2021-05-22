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

#ifndef BELLE_ENGRAVER_BEAMING_H
#define BELLE_ENGRAVER_BEAMING_H

namespace BELLE_NAMESPACE
{
  //Forward declaration.
  class Beam;

  class Stem
  {
    public:
      //Cached info
      mutable count LevelsAbove;
      mutable count LevelsBelow;

      /**Describes the direction of the stem. If true, the stem will go from a
      note up to the beam. If false, the stem will go from the note down to the
      beam.*/
      bool StemUp; PRIM_PAD(bool)

      ///Describes the number of extra beams besides the 8th beam.
      count ExtraLevels;

      ///The positive distance to the next stem (if there is one; if not, zero).
      number UnitsNextStemDistance;

      /**The location of the current stem. This is an intermediate value used
      internally.*/
      Vector UnitsStemLocation;

      /**The level of group division. If < 0 it is beamed all the way through.
      Otherwise 0 = divides at 1/8, 1 = divides at 1/16, etc.*/
      count DivisionLevel;

      /** Chord to which the stem corresponds; may be set by a constructor. */
      Music::ConstNode Chord;

      ///Default constructor zeroes fields.
      Stem() : StemUp(false), ExtraLevels(0), UnitsNextStemDistance(0),
        DivisionLevel(-1) {}

      ///Constructor to set chord and zero fields.
      Stem(Music::ConstNode c) : StemUp(false), ExtraLevels(0),
          UnitsNextStemDistance(0),
        DivisionLevel(-1), Chord(c) {}

      ///Constructor to initialize fields.
      Stem(bool StemUp_, count ExtraLevels_,
        number UnitsNextStemDistance_, count DivisionLevel_ = -1) :
        StemUp(StemUp_), ExtraLevels(ExtraLevels_),
        UnitsNextStemDistance(UnitsNextStemDistance_),
        DivisionLevel(DivisionLevel_) {}

    friend class Beam;
  };

  class Beam
  {
    protected:
    ///The highest level observed by Prepare().
    mutable count HighestLevel;

    ///The lowest level observed by Prepare().
    mutable count LowestLevel;

    public:
    ///An array of stems which contain information about how to beam the levels.
    Array<Stem> Stems;

    ///The baseline of the primary beam.
    Line UnitsBaseline;

    ///The thickness of each beam.
    number UnitsLevelThickness;

    /**The positive distance of beams from one level to the next. The distance
    is measured from center of the beams at any common value on the construction
    plane's x-axis.*/
    number UnitsLevelSpacing;

    ///Width of the flag that juts out of an unconnected beam.
    number UnitsFlagWidth;

    ///Width of stem.
    number StemWidth;

    ///Constructor to create a number of stems.
    Beam(const Array<Music::ConstNode>& ChordsInBeam)
    {
      for(count i = 0; i < ChordsInBeam.n(); i++)
        Stems.Add(Stem(ChordsInBeam[i]));
    }

    ///Default constructor initializes with no stems.
    Beam()
    {
    }

    void Prepare()
    {
      count Stems_n = Stems.n();

      //Make sure there are enough stems to continue.
      if(Stems_n < 2)
        return;

      //Make sure that all the level values are positive numbers.
      for(count i = 0; i < Stems_n; i++)
      {
        Stem& ithStem = Stems[i];
        ithStem.ExtraLevels = Abs(count(ithStem.ExtraLevels));
      }

      //-----------------------//
      //Algorithm: Beam Sharing//
      //-----------------------//

      //Determine the beam levels.
      for(count i = 0; i < Stems_n; i++)
      {
        Stem& s = Stems[i];
        if(not i or (s.DivisionLevel >= 0)) /*Beginning of group does not depend
        on prior beams since beams are not shared.*/
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
            if(not p.LevelsAbove) //All previous beams were below primary beam.
            {
              s.LevelsAbove = 0;
              s.LevelsBelow = -s.ExtraLevels;
            }
            else //Some previous beams were above.
            {
              if(s.ExtraLevels <= p.LevelsAbove)
              {
                if(not p.StemUp)
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
            if(not p.LevelsBelow) //All previous beams were above primary beam.
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
      for(count i = 0; i < Stems_n; i++)
      {
        MakeAtLeast(HighestLevel, Stems[i].LevelsAbove);
        MakeAtMost(LowestLevel, Stems[i].LevelsBelow);
      }
    }

    bool HasBeam(count LeftStemIndex, count Level) const
    {
      //If out of range then there can be no beam.
      if(LeftStemIndex < 0 or LeftStemIndex >= Stems.n() - 1)
        return false;

      /*If the primary beam, then it is automatically beamed without further
      consideration.*/
      if(not Level)
        return true;

      //Get a reference to the current and next stems.
      const Stem& s = Stems.ith(LeftStemIndex);
      const Stem& n = Stems.ith(LeftStemIndex + 1);

      //Do not beam this level if going across a beam division.
      if(n.DivisionLevel >= 0 and Abs(Level) > n.DivisionLevel)
        return false;

      if(IsBetween(Level, s.LevelsBelow, s.LevelsAbove) and
        IsBetween(Level, n.LevelsBelow, n.LevelsAbove))
        return true; //The two adjacent stems share a beam.
      else
        return false; //The adjacent stems do not share a beam.
    }

    bool HasLeftFlag(count StemIndex, count Level) const
    {
      /*Algorithm note: left flags are preferred over right flags when there is
      no other visual cue that would indicate one direction or the other.*/

      //Eighth-level is always beamed.
      if(not Level)
        return false;

      //No left flags on the first stem or non-existent ones.
      if(StemIndex <= 0 or StemIndex >= Stems.n())
        return false;

      //Get a reference to the stem.
      const Stem& p = Stems.ith(StemIndex - 1);
      const Stem& s = Stems.ith(StemIndex);

      /*If the first in the beam group or on a beaming division there can be
      no left flag.*/
      if(s.DivisionLevel >= 0)
        return false;

      //If beamed to the next stem, then a flag might be unnecessary.
      if(HasBeam(StemIndex, Level))
      {
        if(s.StemUp)
        {
          //See if there are any left flags below this level on this stem.
          if(Level >= 0 or Level == s.LevelsBelow or
            not HasLeftFlag(StemIndex, Level - 1))
              return false;
        }
        else
        {
          //See if there are any left flags above this level on this stem.
          if(Level <= 0 or Level == s.LevelsAbove or
            not HasLeftFlag(StemIndex, Level + 1))
              return false;
        }
      }

      //Make sure that the level is actually in use for this stem.
      if(not IsBetween(Level, s.LevelsBelow, s.LevelsAbove))
        return false;

      //Detect beam sharing as the final decision.
      if(IsBetween(Level, p.LevelsBelow, p.LevelsAbove))
        return false; //Flag unnecessary since beam can be shared.
      else
        return true; //Flag necessary since beam can not be shared.
    }

    bool HasRightFlag(count StemIndex, count Level) const
    {
      /*Right flag is a fallback in case there is neither a left flag or a beam,
      yet, the level exists.*/

      /*See if the stem index is in range. Also last stem can not have right
      flag.*/
      if(StemIndex < 0 or StemIndex >= Stems.n() - 1)
        return false;

      //Get a reference to the current and next stems.
      const Stem& s = Stems.ith(StemIndex);
      //const Stem& n = Stems.ith(StemIndex + 1);

      /*If the level is valid and this is neither left flagged or beamed, then
      this stem is right flagged.*/
      if(IsBetween(Level, s.LevelsBelow, s.LevelsAbove) and
        not HasLeftFlag(StemIndex, Level) and not HasBeam(StemIndex, Level)
         and not HasBeam(StemIndex - 1, Level))
          return true;
      else
        return false; //Right flag not applicable or not necessary.
    }

    /**Returns a number, whose sign indicates the overall direction of the
    stems. If all stems point up from the notes, returns a positive number.
    If all stems point down, returns a negative number. If the direction is
    mixed or there are no stems, returns zero.*/
    count GetGeneralStemDirection() const
    {
      bool HasUpStems = false;
      bool HasDownStems = false;

      for(count i = Stems.n() - 1; i >= 0; i--)
      {
        if(Stems[i].StemUp)
          HasUpStems = true;
        else
          HasDownStems = true;
      }

      if(HasUpStems and HasDownStems)
        return 0;
      else if(HasUpStems)
        return 1;
      else if(HasDownStems)
        return -1;
      else
        return 0;
    }

    ///Paints a single beamed segment as a parallelogram.
    void PaintBeamSegment(Pointer<Path> p, Line l,
      number Height)
    {
      if(l.a.x > l.b.x)
        Swap(l.a, l.b);
      if(not (l.a.x < l.b.x))
        return;
      number Angle = HalfPi<number>() - Vector(l.b - l.a).Ang();
      number ActualHeight = Height / Sin(Angle);

      number HalfHeight = ActualHeight * 0.5f;

      Vector tl(l.a.x, l.a.y + HalfHeight),
        bl(l.a.x, l.a.y - HalfHeight),
        tr(l.b.x, l.b.y + HalfHeight),
        br(l.b.x, l.b.y - HalfHeight);

      Path Parallelogram;
      Parallelogram.Add(Instruction(bl, true));
      Parallelogram.Add(Instruction(br));
      Parallelogram.Add(Instruction(tr));
      Parallelogram.Add(Instruction(tl));
      Parallelogram.Add(Instruction());

      p->Append(Parallelogram);
    }

    number GetSlope() const
    {
      return (UnitsBaseline.b.y - UnitsBaseline.a.y) /
        (UnitsBaseline.b.x - UnitsBaseline.a.x);
    }

    Line GetLowestBaseline()
    {
      //Update the information.
      Prepare();
      Line InteriorBaseline = UnitsBaseline;
      InteriorBaseline.a.y = UnitsBaseline.a.y - UnitsLevelThickness / 2.f +
        UnitsLevelSpacing * number(LowestLevel);
      InteriorBaseline.b.y += InteriorBaseline.a.y - UnitsBaseline.a.y;
      return InteriorBaseline;
    }

    Line GetHighestBaseline()
    {
      //Update the information.
      Prepare();
      Line InteriorBaseline = UnitsBaseline;
      InteriorBaseline.a.y = UnitsBaseline.a.y + UnitsLevelThickness / 2.f +
        UnitsLevelSpacing * number(HighestLevel);
      InteriorBaseline.b.y += InteriorBaseline.a.y - UnitsBaseline.a.y;
      return InteriorBaseline;
    }

    ///Paints the beam specified by the structure.
    void Paint(Pointer<Path> p)
    {
      //Update the information.
      Prepare();

      //No beams to paint if beam slant is vertical.
      if(Limits<number>::IsEqual(UnitsBaseline.a.x, UnitsBaseline.b.x))
        return;

      //Calculate the slope of the beam.
      number Slope = GetSlope();

      number x = UnitsBaseline.a.x;
      number y = UnitsBaseline.a.y;

      for(count i = 0; i < Stems.n(); i++)
      {
        Stem& Stem = Stems[i];
        for(count j = LowestLevel; j <= HighestLevel; j++)
        {
          number UnitsLevelY = y + UnitsLevelSpacing * number(j);

          if(HasLeftFlag(i, j))
          {
            //Create flag on left.
            Line l(x - UnitsFlagWidth,
              UnitsLevelY - UnitsFlagWidth * Slope, x, UnitsLevelY);
            PaintBeamSegment(p, l, UnitsLevelThickness);
          }
          if(HasRightFlag(i, j))
          {
            //Create flag on right.
            Line l(x, UnitsLevelY,
              x + UnitsFlagWidth, UnitsLevelY + UnitsFlagWidth * Slope);
            PaintBeamSegment(p, l, UnitsLevelThickness);
          }
          if(HasBeam(i, j))
          {
            //Create beam.
            Line l(x, UnitsLevelY, x + Stem.UnitsNextStemDistance,
              UnitsLevelY + Stem.UnitsNextStemDistance * Slope);
            PaintBeamSegment(p, l, UnitsLevelThickness);

            if(i + 1 < Stems.n() and not HasBeam(i + 1, j))
            {
              Vector CenteringOffset;
              if(i == Stems.n() - 2)
                CenteringOffset = -Vector(StemWidth / 2.f, 0);
            }
          }
        }

        number delta = Stem.UnitsNextStemDistance;
        x += delta;
        y += delta * Slope;
      }
    }
  };

  class Beaming
  {
    public:
    static void ProjectBeam(Vector& First, Vector& Last, number Angle)
    {
      Last.y = First.y + Tan(Angle) * (Last - First).x;
    }

    static bool BeamShouldBeHorizontal(const List<count>& StaffPositions,
      bool StemUp)
    {
      count First = StaffPositions.a();
      count Last = StaffPositions.z();
      count MaxNote = Max(First, Last);
      count MinNote = Min(First, Last);
      bool Horizontal = false;
      for(count i = 1; not Horizontal and i < StaffPositions.n() - 1; i++)
        if((StemUp and StaffPositions[i] >= MaxNote) or
          (not StemUp and StaffPositions[i] <= MinNote))
            Horizontal = true;
      return Horizontal;
    }

    static void NormalizeBeam(Vector& First, Vector& Last, bool StemUp,
      const List<count>& StaffPositions)
    {
      number CurrentAngle = (Last - First).Ang();
      if(CurrentAngle > Pi<number>())
        CurrentAngle -= TwoPi<number>();

      CurrentAngle *= 0.5f;
      const number MaxAngle = 10.f * Deg<number>();
      number ClampedAngle = Clip(CurrentAngle, -MaxAngle, MaxAngle);

      /*Beaming slope possibilities are discretized to help with visual
      consistency and output stability.*/
      //ClampedAngle = Truncate(ClampedAngle, 1.f * Deg<number>());

      if(BeamShouldBeHorizontal(StaffPositions, StemUp))
        ClampedAngle = 0.f;

      if((ClampedAngle > 0.f) == StemUp)
        ProjectBeam(Last, First, ClampedAngle);
      else
        ProjectBeam(First, Last, ClampedAngle);
    }

    static Value& StemState(Music::ConstNode Island, Music::ConstNode Chord)
    {
      return Island->Label.SetState("PartState", "Chord")[Chord]["Stem"];
    }

    static Vector StemEndPoint(const Value& Stem)
    {
      if(Stem["StemUp"])
        return Vector(+Stem["X"], +Stem["EndY"] + +Stem["Height"]) +
          Stem["ChordOffset"].AsVector();
      else
        return Vector(+Stem["X"], +Stem["EndY"] - +Stem["Height"]) +
          Stem["ChordOffset"].AsVector();
    }

    static Vector StemStartPoint(const Value& Stem)
    {
      return Vector(+Stem["X"], +Stem["StartY"]) +
        Stem["ChordOffset"].AsVector();
    }

    static void AdjustBeamHeightToAvoidCollisions(Vector& Start, Vector& End,
      bool StemUp, const List<Vector>& CollisionPoints)
    {
      Line Slant(Start, End);
      number Delta = 0.f;
      for(count i = 0; i < CollisionPoints.n(); i++)
      {
        number y = 0.f;
        if(Slant.VerticalIntersection(y, CollisionPoints[i].x))
        {
          if(StemUp)
            Delta = Max(Delta, CollisionPoints[i].y - y);
          else
            Delta = Min(Delta, CollisionPoints[i].y - y);
        }
      }
      Start.y += Delta;
      End.y += Delta;

      //Lock the beam to specific staff position.
      if(StemUp)
      {
        Start.y = Ceiling(Start.y * 2.f) / 2.f;
        End.y = Ceiling(End.y * 2.f) / 2.f;
      }
      else
      {
        Start.y = Floor(Start.y * 2.f) / 2.f;
        End.y = Floor(End.y * 2.f) / 2.f;
      }
    }

    static void EngraveBeam(Music::ConstNode t)
    {
      //Get the set of chords in the beam.
      Array<Music::ConstNode> ChordsInBeam = t->Series(MusicLabel(mica::Beam));

      //Get the respective islands for the chords in the beam.
      Array<Music::ConstNode> IslandsInBeam;
      for(count i = 0; i < ChordsInBeam.n(); i++)
        IslandsInBeam.Add() = IslandOfToken(ChordsInBeam[i]);

      //Accumulate any intermediate islands not in the beam.
      Array<Music::ConstNode> IslandsNotInBeam;
      for(count i = 0; i < IslandsInBeam.n() - 1; i++)
      {
        Music::ConstNode Next = IslandsInBeam[i]->Next(
          MusicLabel(mica::Partwise));
        while(Next and Next != IslandsInBeam[i + 1])
        {
          IslandsNotInBeam.Add() = Next;
          Next = Next->Next(MusicLabel(mica::Partwise));
        }
      }

      //Calculate the maximal bounds of the intermediate objects.
      Box NonBeamIslandBounds;
      for(count i = 0; i < IslandsNotInBeam.n(); i++)
      {
        Music::ConstNode Island = IslandsNotInBeam[i];
        Pointer<Stamp> s = Island->Label.Stamp().Object();
        NonBeamIslandBounds += s->Bounds();
      }

      //Calculate the slant.
      number StartX = IslandsInBeam.a()->Label.GetState(
        "IslandState", "TypesetX");
      number EndX = IslandsInBeam.z()->Label.GetState(
        "IslandState", "TypesetX");

      if(not Limits<number>::Bounded(StartX) or
         not Limits<number>::Bounded(EndX))
      {
        C::Error() >>
          "Error: could not engrave beam due to missing spacing information";
        return;
      }

      Value& StartStem = StemState(IslandsInBeam.a(), ChordsInBeam.a());
      Value& EndStem = StemState(IslandsInBeam.z(), ChordsInBeam.z());

      List<count> StaffPositions;
      for(count i = 0; i < ChordsInBeam.n(); i++)
        StaffPositions.Add() =
          count(+StemState(IslandsInBeam[i], ChordsInBeam[i])["StartY"] * 2.f);

      bool StemUp = StartStem["StemUp"];
      Vector Start = Vector(StartX, 0) + StemEndPoint(StartStem);
      Vector End = Vector(EndX, 0) + StemEndPoint(EndStem);
      if(not Limits<number>::Bounded(Start.x) or
        not Limits<number>::Bounded(Start.y) or
        not Limits<number>::Bounded(End.x) or
        not Limits<number>::Bounded(End.y))
      {
        C::Error() >>
          "Error: could not engrave beam due to bad stem information";
        return;
      }

      NormalizeBeam(Start, End, StemUp, StaffPositions);

      //Get list of points to consider for collision.
      number BeamStemDisplaceThreshold = (0.5f + 0.25f + 0.5f);
      Vector CollisionDisplace(0, BeamStemDisplaceThreshold);
      List<Vector> CollisionPoints;
      {
        for(count i = 0; i < ChordsInBeam.n(); i++)
        {
          CollisionPoints.Add() =
            Vector(+IslandsInBeam[i]->Label.GetState(
            "IslandState", "TypesetX"), 0) + StemEndPoint(
            StemState(IslandsInBeam[i], ChordsInBeam[i]));
        }
        for(count i = 0; i < IslandsNotInBeam.n(); i++)
        {
          Music::ConstNode Island = IslandsNotInBeam[i];
          Pointer<Stamp> s = Island->Label.Stamp().Object();
          Box r = s->Bounds();
          Vector Delta = Vector(+Island->Label.GetState(
            "IslandState", "TypesetX"), 0);
          if(StemUp)
            CollisionPoints.Add() = r.TopLeft() + Delta + CollisionDisplace,
            CollisionPoints.Add() = r.TopRight() + Delta + CollisionDisplace;
          else
            CollisionPoints.Add() = r.BottomLeft() + Delta - CollisionDisplace,
            CollisionPoints.Add() = r.BottomRight() + Delta - CollisionDisplace;
        }
      }

      AdjustBeamHeightToAvoidCollisions(Start, End, StemUp, CollisionPoints);

      //Adjust beam ends to be flush with stem.
      number StemWidth = +Property(IslandsInBeam.a(), "StemWidth");
      {
        if(StemUp)
          Start.x -= StemWidth;
        else
          End.x += StemWidth;
      }

      //Set some rendering properties.
      Vector IslandDelta = Vector(StartX, 0);
      Beam BeamGroup(ChordsInBeam);
      BeamGroup.UnitsFlagWidth = 1.3f;
      BeamGroup.UnitsLevelSpacing = 0.75f;
      BeamGroup.UnitsLevelThickness = 0.5f;
      BeamGroup.StemWidth = StemWidth;
      for(count i = 0; i < IslandsInBeam.n(); i++)
      {
        BeamGroup.Stems[i].ExtraLevels = Max(FlagsGivenDuration(
          IntrinsicDurationOfChord(ChordsInBeam[i])) - 1, count(0));
        BeamGroup.Stems[i].StemUp = StemUp;
        Ratio BeamDivision = Ratio(
          ChordsInBeam[i]->Label.Get(mica::BeamDivision));
        if(not i)
          BeamGroup.Stems[i].DivisionLevel = 0;
        else if(not BeamDivision.IsEmpty() and BeamDivision > Ratio(0) and
          BeamDivision <= Ratio(1, 8))
            BeamGroup.Stems[i].DivisionLevel =
              FlagsGivenDuration(BeamDivision) - 1;
      }
      BeamGroup.UnitsBaseline = Line(Start - IslandDelta, End - IslandDelta);

      //Figure out the beam height.
      number BeamHeight = Abs(BeamGroup.GetLowestBaseline().a.y -
        BeamGroup.GetHighestBaseline().a.y);

      BeamHeight = Max(BeamHeight - BeamStemDisplaceThreshold, number(0));
      if(StemUp)
        Start.y += BeamHeight, End.y += BeamHeight;
      else
        Start.y -= BeamHeight, End.y -= BeamHeight;
      BeamGroup.UnitsBaseline = Line(Start - IslandDelta, End - IslandDelta);

      //Engrave the stems.
      Array<number> XPositions(ChordsInBeam.n());
      for(count i = 0; i < ChordsInBeam.n(); i++)
      {
        Line Slant(Start, End);
        Value& Stem = StemState(IslandsInBeam[i], ChordsInBeam[i]);
        Vector CurrentEndPoint = Vector(+IslandsInBeam[i]->Label.GetState(
          "IslandState", "TypesetX"), 0) + StemEndPoint(Stem);
        number y = 0.f;
        Slant.VerticalIntersection(y, CurrentEndPoint.x);
        if(StemUp)
          Stem["Height"] = +Stem["Height"] + (y - CurrentEndPoint.y);
        else
          Stem["Height"] = +Stem["Height"] - (y - CurrentEndPoint.y);
        Value Stems;
        if(not i)
          XPositions[i] = Start.x;
        else if(i == ChordsInBeam.n() - 1)
          XPositions[i] = End.x;
        else
        {
          if(StemUp)
            XPositions[i] = CurrentEndPoint.x - StemWidth / 2.f;
          else
            XPositions[i] = CurrentEndPoint.x + StemWidth / 2.f;
        }

        Stems.Add() = Stem;
        EngraveStems(IslandsInBeam[i], Stems, true);
      }

      //Calculate intermediate stem positions.
      for(count i = 1; i < IslandsInBeam.n(); i++)
        BeamGroup.Stems[i - 1].UnitsNextStemDistance =
          XPositions[i] - XPositions[i - 1];

      Pointer<Path> p = new Path;
      Pointer<Stamp> IslandToDrawTo = IslandsInBeam.a()->Label.Stamp().Object();
      BeamGroup.Paint(p);

      //Draw the beam path.
      IslandToDrawTo->Add()->p = p;
      IslandToDrawTo->z()->Spans = true;
      IslandToDrawTo->z()->Context = ChordsInBeam.a()->Next(
        MusicLabel(mica::Beam), true);
    }

    static void EngraveBeams(Pointer<const Music> M)
    {
      if(not M) return;
      Music::ConstNode m, n;
      for(m = M->Root(); m; m = m->Next(MusicLabel(mica::Instantwise)))
      {
        for(n = m; n; n = n->Next(MusicLabel(mica::Partwise)))
        {
          Array<Music::ConstNode> Chords =
            n->Children(MusicLabel(mica::Token));
          for(count c = 0; c < Chords.n(); c++)
            if(Chords[c]->Next(MusicLabel(mica::Beam)) and
              not Chords[c]->Previous(MusicLabel(mica::Beam)))
                EngraveBeam(Chords[c]);
        }
      }
    }
  };
}
#endif
