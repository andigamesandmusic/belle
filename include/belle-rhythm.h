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

#ifndef BELLE_ENGRAVER_RHYTHM_H
#define BELLE_ENGRAVER_RHYTHM_H

namespace BELLE_NAMESPACE
{
  class Rhythm
  {
    public:

    static void ShiftNonRhythmicInstantsLeft(
      List<Array<Music::ConstNode> >& InstantRegions)
    {
      count PartCount = InstantRegions.n() ? InstantRegions.a().n() : count(0);
      bool ShiftWasMade = true;
      while(ShiftWasMade)
      {
        ShiftWasMade = false;
        for(count Instant = 1; Instant < InstantRegions.n(); Instant++)
        {
          if(not IsInstantRhythmic(InstantRegions[Instant]) and
            IsInstantRhythmic(InstantRegions[Instant - 1]))
          {
            bool AllowedToShift = true;
            for(count Part = 0; AllowedToShift and Part < PartCount; Part++)
              if(InstantRegions[Instant][Part] and
                InstantRegions[Instant - 1][Part])
                  AllowedToShift = false;

            if(AllowedToShift)
            {
              InstantRegions.Swap(Instant, Instant - 1);
              ShiftWasMade = true;
            }
          }
        }
      }
    }

    static List<VectorInt> GetInstantRegions(Pointer<const Geometry> G)
    {
      List<VectorInt> L;
      for(count i = 0; i < G->GetNumberOfInstants(); i++)
      {
        if(G->IsInstantComplete(i) || !L.n())
          L.Add().i() = integer(i);
        L.z().j() = integer(i);
      }
      return L;
    }

    static bool IsInstantRhythmic(const Array<Music::ConstNode>& Instant)
    {
      bool InstantIsRhythmic = false;
      for(count i = 0; !InstantIsRhythmic && i < Instant.n(); i++)
        if(Utility::IsIslandRhythmic(Instant[i]))
          InstantIsRhythmic = true;
      return InstantIsRhythmic;
    }

    static List<VectorInt> GetRhythmicRegions(
      const List<Array<Music::ConstNode> >& InstantRegions)
    {
      List<VectorInt> L;
      for(count i = 0; i < InstantRegions.n(); i++)
      {
        bool InstantIsRhythmic = IsInstantRhythmic(InstantRegions[i]);
        if(InstantIsRhythmic || !L.n())
          L.Add().i() = integer(i);
        L.z().j() = integer(i);
      }
      return L;
    }

    static List<Array<Music::ConstNode> > GetIslandsForInstantRegion(
      Pointer<const Geometry> G, VectorInt Region)
    {
      List<Array<Music::ConstNode> > L;
      count PartCount = G->GetNumberOfParts();
      for(count Instant = count(Region.i()); Instant <= count(Region.j());
        Instant++)
      {
        L.Add().n(PartCount);
        for(count Part = 0; Part < PartCount; Part++)
          if(Music::ConstNode Island = G->LookupIsland(Part, Instant))
            L.z()[Part] = Island;
      }
      return L;
    }

    static Matrix<Ratio> GetRhythmicOnsets(
      const List<Array<Music::ConstNode> >& InstantRegions)
    {
      List<VectorInt> Regions = GetRhythmicRegions(InstantRegions);
      count PartCount = InstantRegions.n() ? InstantRegions.a().n() : count(0);
      Matrix<Ratio> Onsets(PartCount, Regions.n());
      for(count Part = 0; Part < PartCount; Part++)
      {
        Ratio Onset = 0;
        for(count RegionIndex = 0; RegionIndex < Regions.n(); RegionIndex++)
        {
          count Instant = count(Regions[RegionIndex].i());
          if(Music::ConstNode Island = InstantRegions[Instant][Part])
          {
            Ratio IslandDuration = RhythmicDurationOfIsland(Island);
            if(IslandDuration > 0)
            {
              Onsets(Part, RegionIndex) = Onset;
              Onset += IslandDuration;
            }
          }
        }
      }
      return Onsets;
    }

    static Matrix<Ratio> GetUnpackedRhythmicOnsets(
      const List<Array<Music::ConstNode> >& InstantRegions)
    {
      Matrix<Ratio> PackedOnsets = GetRhythmicOnsets(InstantRegions);
      List<VectorInt> Regions = GetRhythmicRegions(InstantRegions);
      count PartCount = PackedOnsets.m();
      Matrix<Ratio> UnpackedOnsets(PartCount, count(Regions.z().j()) + 1);

      for(count PackedInstant = 0; PackedInstant < Regions.n(); PackedInstant++)
        for(count Part = 0; Part < PartCount; Part++)
          UnpackedOnsets(Part, count(Regions[PackedInstant].i())) =
            PackedOnsets(Part, PackedInstant);

      return UnpackedOnsets;
    }

    static bool IsInstantRhythmic(const Matrix<Ratio>& Onsets, count Instant)
    {
      bool Rhythmic = false;
      if(Instant >= 0 && Instant < Onsets.n())
      {
        for(count Part = 0; !Rhythmic && Part < Onsets.m(); Part++)
          Rhythmic = Onsets(Part, Instant).IsDeterminate();
      }
      return Rhythmic;
    }

    static bool IsInstantRhythmicallyAdjacent(const Matrix<Ratio>& Onsets,
      count Instant)
    {
      return IsInstantRhythmic(Onsets, Instant - 1) &&
        IsInstantRhythmic(Onsets, Instant);
    }

    static Array<Ratio> GetLowestOnsets(const Matrix<Ratio>& M)
    {
      Array<Ratio> LowestOnset(M.n());
      for(count i = 0; i < M.n(); i++)
      {
        Ratio Lowest;
        for(count j = 0; j < M.m(); j++)
        {
          Ratio r = M(j, i);
          if(!r.IsEmpty())
          {
            if(Lowest.IsEmpty() || r < Lowest)
              Lowest = r;
          }
        }
        LowestOnset[i] = Lowest;
      }
      return LowestOnset;
    }

    static count FindFirstOutOfOrderInstant(const Array<Ratio>& LowestOnsets)
    {
      Ratio Current;
      count OutOfOrderInstant = -1;
      for(count i = 0; i < LowestOnsets.n() && OutOfOrderInstant < 0; i++)
      {
        if(!LowestOnsets[i].IsEmpty())
        {
          if(!Current.IsEmpty() && LowestOnsets[i] <= Current)
            OutOfOrderInstant = i;

          Current = LowestOnsets[i];
        }
      }
      return OutOfOrderInstant;
    }

    static count InsertionPositionForOnset(const Array<Ratio>& LowestOnsets,
      count OutOfOrderInstant)
    {
      count InsertionPosition = -1;
      if(OutOfOrderInstant >= 0)
      {
        Ratio Onset = LowestOnsets[OutOfOrderInstant];
        for(count i = 0; i < LowestOnsets.n() && InsertionPosition < 0; i++)
          if(!LowestOnsets[i].IsEmpty() && Onset <= LowestOnsets[i])
            InsertionPosition = i;
      }
      return InsertionPosition;
    }

    static void OrderByRhythm(
      List<Array<Music::ConstNode> >& IslandsInInstantRegion)
    {
      /*First shift non-rhythmic instants over as far to the left as possible.
      This helps to correctly order non-rhythmic instants that occur before the
      first rhythmic instant.*/
      ShiftNonRhythmicInstantsLeft(IslandsInInstantRegion);

      bool Reordering = true;
      while(Reordering)
      {
        List<VectorInt> RhythmicRegions =
          GetRhythmicRegions(IslandsInInstantRegion);
        Matrix<Ratio> Onsets = GetRhythmicOnsets(IslandsInInstantRegion);
        Array<Ratio> LowestOnsets = GetLowestOnsets(Onsets);
        count OutOfOrder = FindFirstOutOfOrderInstant(LowestOnsets);
        count InsertionPoint = InsertionPositionForOnset(LowestOnsets,
          OutOfOrder);
        if(OutOfOrder >= 0 && InsertionPoint >= 0)
        {
          Ratio Source = LowestOnsets[OutOfOrder];
          Ratio Destination = LowestOnsets[InsertionPoint];
          count SourceIndex = count(RhythmicRegions[OutOfOrder].i());
          count SourceItems = count(RhythmicRegions[OutOfOrder].j()) -
            count(RhythmicRegions[OutOfOrder].i()) + 1;
          count DestinationIndex = count(RhythmicRegions[InsertionPoint].i());

          if(Source == Destination)
          {
            //Merge the islands in source instant to destination instant.
            count PartCount = IslandsInInstantRegion.a().n();
            for(count Part = 0; Part < PartCount; Part++)
            {
              if(!IslandsInInstantRegion[DestinationIndex][Part] &&
                IslandsInInstantRegion[SourceIndex][Part])
              {
                IslandsInInstantRegion[DestinationIndex][Part] =
                  IslandsInInstantRegion[SourceIndex][Part];
              }
            }

            //Remove the original source.
            IslandsInInstantRegion.Remove(SourceIndex);
            SourceItems--;
          }
          else
          {
            /*Insert new empty instant, swap with source, and remove empty
            instant.*/
            IslandsInInstantRegion.InsertBefore(Array<Music::ConstNode>(),
              DestinationIndex);
            SourceIndex++;
            IslandsInInstantRegion.Swap(DestinationIndex, SourceIndex);
            IslandsInInstantRegion.Remove(SourceIndex);
            SourceItems--;
          }

          //Now move any remaining non-rhythmic trailing instants.
          while(SourceItems)
          {
            IslandsInInstantRegion.InsertAfter(Array<Music::ConstNode>(),
              DestinationIndex++);
            SourceIndex++;
            IslandsInInstantRegion.Swap(DestinationIndex, SourceIndex);
            IslandsInInstantRegion.Remove(SourceIndex);
            SourceItems--;
          }
        }
        else
        {
          Reordering = false;
        }
      }
    }

    static void CreateRhythmOrderedRegion(Pointer<const Geometry> G,
      List<Array<Music::ConstNode> >& RhythmOrderedRegion)
    {
      //Clear any elements in the output argument.
      RhythmOrderedRegion.RemoveAll();

      //Get a list of complete-instant regions.
      List<VectorInt> InstantRegions = GetInstantRegions(G);

      /*For each complete-instant region, create a rhythm-ordered region and
      append it to the output region.*/
      for(count i = 0; i < InstantRegions.n(); i++)
      {
        //Get the initial unordered region for the section of the geometry.
        List<Array<Music::ConstNode> > IslandsInInstantRegion =
          GetIslandsForInstantRegion(G, InstantRegions[i]);

        //Order the region by rhythm.
        OrderByRhythm(IslandsInInstantRegion);

        //Append each instant of the rhythm-order region to the output region.
        for(count j = 0; j < IslandsInInstantRegion.n(); j++)
          RhythmOrderedRegion.Add() = IslandsInInstantRegion[j];
      }
    }

    static Matrix<Ratio> ForceAlignRhythmMatrix(Matrix<Ratio> RhythmMatrix)
    {
      count Parts = RhythmMatrix.m();
      count Instants = RhythmMatrix.n();
      Array<Ratio> Deltas(Parts);
      for(count Part = 0; Part < Parts; Part++)
        Deltas[Part] = Ratio(0);
      for(count Instant = 0; Instant < Instants; Instant++)
      {
        Ratio Furthest;
        for(count Part = 0; Part < Parts; Part++)
          if(Furthest.IsEmpty() or (not RhythmMatrix(Part, Instant).IsEmpty()
            and (RhythmMatrix(Part, Instant) + Deltas[Part]) > Furthest))
              Furthest = RhythmMatrix(Part, Instant) + Deltas[Part];
        for(count Part = 0; Part < Parts; Part++)
          if(not RhythmMatrix(Part, Instant).IsEmpty())
          {
            Ratio Current = RhythmMatrix(Part, Instant) + Deltas[Part];
            Deltas[Part] += Furthest - Current;
            RhythmMatrix(Part, Instant) += Deltas[Part];
          }
      }
      return RhythmMatrix;
    }

    static Array<Ratio> GetMomentDurations(
      const Matrix<Ratio>& RhythmMatrix,
      const List<Array<Music::ConstNode> >& NodeMatrix)
    {
      count Moments = RhythmMatrix.n();
      count Parts = RhythmMatrix.m();
      Array<Ratio> MomentDurations(Moments);
      for(count i = 0; i < MomentDurations.n(); i++)
      {
        Ratio SmallestDuration;
        Ratio CurrentOnset;
        for(count j = 0; j < Parts and CurrentOnset.IsEmpty(); j++)
          CurrentOnset = RhythmMatrix(j, i);
        for(count j = 0; j < Parts and SmallestDuration.IsEmpty(); j++)
        {
          Ratio DurationOfIsland = RhythmicDurationOfIsland(NodeMatrix[i][j]);
          if(DurationOfIsland.IsDeterminate() and DurationOfIsland > Ratio(0))
            SmallestDuration = DurationOfIsland;
        }
        for(count j = 0; j < Parts and CurrentOnset.IsDeterminate(); j++)
        {
          Ratio DurationToOnset;
          Ratio NextOnset;
          for(count k = i + 1; k < Moments and NextOnset.IsEmpty(); k++)
            NextOnset = RhythmMatrix(j, k);
          if(NextOnset.IsDeterminate())
            DurationToOnset = NextOnset - CurrentOnset;

          if(DurationToOnset.IsDeterminate() and
            ((SmallestDuration.IsDeterminate() and
            DurationToOnset < SmallestDuration) or
            SmallestDuration.IsEmpty()))
              SmallestDuration = DurationToOnset;
        }
        MomentDurations[i] = SmallestDuration;
      }
      return MomentDurations;
    }
  };
}
#endif
