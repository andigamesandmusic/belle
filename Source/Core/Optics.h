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
    static prim::number CalculateMinimumNonCollidingDistance(
      const Path& p, const Path& q)
    {
      using namespace prim;
      using namespace prim::planar;

      Rectangle r = p.Bounds();
      Rectangle s = q.Bounds();
      
      return (Vector(r.Width(), r.Height()).Mag() +
        Vector(s.Width(), s.Height()).Mag()) / 2.0;
    }
    
    /**Recursively bisects for the closest non-colliding distance of two paths.
    The first path is the stationary anchor, the other is the floater which
    moves on a line from the origin to the polar coordinate consisting of an
    angle and a minimum non-colliding distance. The latter should be calculated
    with CalculateMinimumNonCollidingDistance. If left zero, it will be
    automatically calculated.*/
    static prim::number CalculateClosestNonCollidingDistanceAtAngle(
      const Path& Anchor, const Path& Floater,
      prim::number ThetaRadians, prim::planar::Vector AnchorCenter,
      prim::number MinimumNonCollidingDistance = 0,
      prim::number FloaterScale = 1.0)
    {
      ///Calculate a conservative starting distance.
      if(!MinimumNonCollidingDistance)
        MinimumNonCollidingDistance = 
          CalculateMinimumNonCollidingDistance(Anchor, Floater);
      
      //Get outlines.
      const prim::Array<prim::planar::Polygon>& Anchors = Anchor.Outline();
      const prim::Array<prim::planar::Polygon>& Floaters = Floater.Outline();
      
      //Check each subpath against each other subpath.
      prim::number FurthestSafeDistance = 0.0;
      for(prim::count a = 0; a < Anchors.n(); a++)
      {
        for(prim::count f = 0; f < Floaters.n(); f++)
        {
          prim::number d = CalculateClosestNonCollidingDistanceAtAngle(
            Anchors[a], Floaters[f], ThetaRadians, AnchorCenter,
            MinimumNonCollidingDistance, FloaterScale);
          FurthestSafeDistance = prim::Max(FurthestSafeDistance, d);
        }
      }
      return FurthestSafeDistance;
    }

    /**Iteratively looks for the closest non-colliding distance of two polygons.
    The first polygon is the stationary anchor, and the other is the floater
    which moves on a line from the origin to the polar coordinate consisting of
    an angle and a minimum non-colliding distance. The latter should be
    calculated with CalculateMinimumNonCollidingDistance.*/
    static prim::number CalculateClosestNonCollidingDistanceAtAngle(
      const prim::planar::Polygon& Anchor, const prim::planar::Polygon& Floater,
      prim::number ThetaRadians, prim::planar::Vector AnchorCenter,
      prim::number MinimumNonCollidingDistance, prim::number FloaterScale = 1.0)
    {
      //Create the line on which the floater travels.
      prim::planar::Vector Near = AnchorCenter;
      prim::planar::Vector Far(ThetaRadians);
      Far = (Far * MinimumNonCollidingDistance) + AnchorCenter;
      
      /*Determine the angle of approach so the system can be rotated to make an
      approach in the direction of Pi.*/
      prim::number ApproachAngle = (Far - Near).Ang();
      Affine AnchorAffine = Affine::Rotate(-ApproachAngle);
      Affine FloaterAffine = (AnchorAffine * Affine::Translate(Far)) *
        Affine::Scale(FloaterScale);
      
      //Create arrays of transformed lines.
      prim::Array<prim::planar::Line> AnchorLines;
      prim::Array<prim::planar::Line> FloaterLines;
      
      AnchorLines.n(Anchor.n());
      FloaterLines.n(Floater.n());
      
      for(prim::count i = 0; i < Anchor.n(); i++)
      {
        prim::count i_1 = (i ? i - 1 : Anchor.n() - 1);
        AnchorLines[i] = prim::planar::Line(AnchorAffine << Anchor[i],
          AnchorAffine << Anchor[i_1]);
      }
      for(prim::count i = 0; i < Floater.n(); i++)
      {
        prim::count i_1 = (i ? i - 1 : Floater.n() - 1);
        FloaterLines[i] = prim::planar::Line(FloaterAffine << Floater[i],
          FloaterAffine << Floater[i_1]);
      }
      
      //Search for the minimum distance.
      prim::number ClosestDistance = MinimumNonCollidingDistance;
      prim::String c;
      for(prim::count i = 0; i < AnchorLines.n(); i++)
      {
        prim::planar::Line& l = AnchorLines[i];
        for(prim::count j = 0; j < FloaterLines.n(); j++)
        {
          prim::planar::Line f = FloaterLines[j].ClipVertical(l.a.y, l.b.y);
          
          if(prim::number d = l.HorizontalDistance(f.a))
            ClosestDistance = prim::Min(d, ClosestDistance);
            
          if(prim::number d = l.HorizontalDistance(f.b))
            ClosestDistance = prim::Min(d, ClosestDistance);
        }
      }
      
      //Return the best distance of the objects.
      return MinimumNonCollidingDistance - ClosestDistance;
      
#if 0      
      //Iteratively search for the closest distance along the line.
      for(prim::count i = 0; i < Iterations; i++)
      {
        //Create a new distance midpoint to check.
        prim::planar::Vector Mid = Near + (Far - Near) * ApproachValue;
        prim::number MidDistance =
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
