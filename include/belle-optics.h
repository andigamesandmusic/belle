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

#ifndef BELLE_CORE_OPTICS_H
#define BELLE_CORE_OPTICS_H

namespace BELLE_NAMESPACE
{
  struct Optics
  {
    /**Calculates the conservative distance two paths must be to not collide.
    This calculation is based on their bounding boxes and is useful for
    determining a starting point for a finer optical-based collision detection
    algorithm.*/
    static number CalculateMinimumNonCollidingDistance(
      const Path& p, const Path& q)
    {
      Box r = p.Bounds();
      Box s = q.Bounds();

      return (Vector(r.Width(), r.Height()).Mag() +
        Vector(s.Width(), s.Height()).Mag()) / 2.f;
    }

    /**Recursively bisects for the closest non-colliding distance of two paths.
    The first path is the stationary anchor, the other is the floater which
    moves on a line from the origin to the polar coordinate consisting of an
    angle and a minimum non-colliding distance. The latter should be calculated
    with CalculateMinimumNonCollidingDistance. If left zero, it will be
    automatically calculated.*/
    static number CalculateClosestNonCollidingDistanceAtAngle(
      const Path& Anchor, const Path& Floater,
      number ThetaRadians, Vector AnchorCenter,
      number MinimumNonCollidingDistance = 0.f,
      number FloaterScale = 1.f)
    {
      ///Calculate a conservative starting distance.
      if(Limits<number>::IsZero(MinimumNonCollidingDistance))
        MinimumNonCollidingDistance =
          CalculateMinimumNonCollidingDistance(Anchor, Floater);

      //Get outlines.
      const Array<PolygonShape>& Anchors = Anchor.Outline();
      const Array<PolygonShape>& Floaters = Floater.Outline();

      //Check each subpath against each other subpath.
      number FurthestSafeDistance = 0.f;
      for(count a = 0; a < Anchors.n(); a++)
      {
        for(count f = 0; f < Floaters.n(); f++)
        {
          number d = CalculateClosestNonCollidingDistanceAtAngle(
            Anchors[a], Floaters[f], ThetaRadians, AnchorCenter,
            MinimumNonCollidingDistance, FloaterScale);
          FurthestSafeDistance = Max(FurthestSafeDistance, d);
        }
      }
      return FurthestSafeDistance;
    }

    /**Iteratively looks for the closest non-colliding distance of two polygons.
    The first polygon is the stationary anchor, and the other is the floater
    which moves on a line from the origin to the polar coordinate consisting of
    an angle and a minimum non-colliding distance. The latter should be
    calculated with CalculateMinimumNonCollidingDistance.*/
    static number CalculateClosestNonCollidingDistanceAtAngle(
      const PolygonShape& Anchor, const PolygonShape& Floater,
      number ThetaRadians, Vector AnchorCenter,
      number MinimumNonCollidingDistance, number FloaterScale = 1.f)
    {
      //Create the line on which the floater travels.
      Vector Near = AnchorCenter;
      Vector Far;
      Far.Polar(ThetaRadians);
      Far = (Far * MinimumNonCollidingDistance) + AnchorCenter;

      /*Determine the angle of approach so the system can be rotated to make an
      approach in the direction of Pi.*/
      number ApproachAngle = (Far - Near).Ang();
      Affine AnchorAffine = Affine::Rotate(-ApproachAngle);
      Affine FloaterAffine = (AnchorAffine * Affine::Translate(Far)) *
        Affine::Scale(FloaterScale);

      //Create arrays of transformed lines.
      Array<Line> AnchorLines;
      Array<Line> FloaterLines;

      AnchorLines.n(Anchor.n());
      FloaterLines.n(Floater.n());

      for(count i = 0; i < Anchor.n(); i++)
      {
        count i_1 = (i ? i - 1 : Anchor.n() - 1);
        AnchorLines[i] = Line(AnchorAffine << Anchor[i],
          AnchorAffine << Anchor[i_1]);
      }
      for(count i = 0; i < Floater.n(); i++)
      {
        count i_1 = (i ? i - 1 : Floater.n() - 1);
        FloaterLines[i] = Line(FloaterAffine << Floater[i],
          FloaterAffine << Floater[i_1]);
      }

      //Search for the minimum distance.
      number ClosestDistance = MinimumNonCollidingDistance;
      String c;
      for(count i = 0; i < AnchorLines.n(); i++)
      {
        Line& l = AnchorLines[i];
        for(count j = 0; j < FloaterLines.n(); j++)
        {
          Line f = FloaterLines[j].ClipVertical(l.a.y, l.b.y);
          number d_a = l.HorizontalDistance(f.a);
          number d_b = l.HorizontalDistance(f.b);
          if(d_a > 0.f)
            ClosestDistance = Min(d_a, ClosestDistance);
          if(d_b > 0.f)
            ClosestDistance = Min(d_b, ClosestDistance);
        }
      }

      //Return the best distance of the objects.
      return MinimumNonCollidingDistance - ClosestDistance;

#if 0
      //Iteratively search for the closest distance along the line.
      for(count i = 0; i < Iterations; i++)
      {
        //Create a new distance midpoint to check.
        Vector Mid = Near + (Far - Near) * ApproachValue;
        number MidDistance =
          NearDistance + (FarDistance - NearDistance) * ApproachValue;

        //Test for intersection.
        if(!Anchor.IntersectsOutline(Floater, Mid))
        {
          //Move closer.
          Far = Mid;
          BestDistance = FarDistance = MidDistance;
        }
        else
        {
          //Move further away.
          Near = Mid;
          NearDistance = MidDistance;
        }
      }

      //Return best distance found.
      return BestDistance;
#endif
    }
  };
}
#endif
