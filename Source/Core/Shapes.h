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

#ifndef BELLE_CORE_SHAPES_H
#define BELLE_CORE_SHAPES_H

#include "Path.h"

namespace BELLE_NAMESPACE
{
  class StaffLines
  {
    static void DisplaceLineMeetsCurveOnAngle(prim::number Displacement,
      prim::planar::Line& l_in, prim::planar::Bezier& b_in,
      prim::planar::Line& l_out, prim::planar::Bezier& b_out)
    {
      /*Trivial case: line meets the curve on angle (the slope of the line
      equals (or is extremely close to) the slope of the curve initially).*/
      prim::planar::Vector p0, p1, p2, p3;

      l_out = l_in.MakeParallelLine(Displacement);
      b_out = b_in.MakeQuasiParallelCurve(Displacement);
      
      //Error correction: force the bezier curve to start where the line stops.
      b_out.GetControlPoints(p0, p1, p2, p3);
      p0 = l_out.b;
      b_out.SetControlPoints(p0, p1, p2, p3);
    }

    static void DisplaceLineMeetsCurveBelowAngle(prim::number Displacement,
      prim::planar::Line& l_in, prim::planar::Bezier& b_in,
      prim::planar::Line& l_out, prim::planar::Line& l2_out,
      prim::planar::Bezier& b_out)
    {
      /*Gap case: There is a gap that needs to be filled by an additional line
      segment that has the same slope as the curve's initial tangent and 
      extends from the intersection with the line to the intersection with the
      curve's first point.*/
      prim::planar::Vector p0, p1, p2, p3;

      l_out = l_in.MakeParallelLine(Displacement);
      b_out = b_in.MakeQuasiParallelCurve(Displacement); 

      //Calculate the extension line.
      b_out.GetControlPoints(p0, p1, p2, p3);
      prim::planar::Line b_tangent(p0, p1);
      l_out.b = l_out.GetLineIntersection(b_tangent);
      l2_out.a = l_out.b;
      l2_out.b = p0;
    }

    static void DisplaceLineMeetsCurveAboveAngle(prim::number Displacement,
      prim::planar::Line& l_in, prim::planar::Bezier& b_in,
      prim::planar::Line& l_out, prim::planar::Bezier& b_out)
    {
      /*Overlap case: The displaced line overlaps with the displaced curve. This
      problem reduces into finding the intersection of the displaced line and 
      curve and making that the join.*/
      prim::planar::Vector p0, p1, p2, p3;

      l_out = l_in.MakeParallelLine(Displacement);
      b_out = b_in.MakeQuasiParallelCurve(Displacement); 

      prim::number t_intersection = b_out.FindLineIntersection(l_out);
      l_out.b = b_out.Value(t_intersection);
      b_out.Trim(t_intersection, 1.0);
    }

    static void DisplaceCurveMeetsCurveAndIntersects(prim::number Displacement,
      prim::planar::Bezier& b1_in, prim::planar::Bezier& b2_in,
      prim::planar::Bezier& b1_out, prim::planar::Bezier& b2_out)
    {
      using namespace prim;
      using namespace planar;

      b1_out = b1_in.MakeQuasiParallelCurve(Displacement);
      b2_out = b2_in.MakeQuasiParallelCurve(Displacement);

      number Segmentation = 100.0;
      for(number i = 0; i < Segmentation; i++)
      {
        number t1 = i / Segmentation;
        number t2 = (i + 1.0) / Segmentation;
        Line TestLine(b1_out.Value(t1), b1_out.Value(t2));
        number t2_intersect = b2_out.FindLineIntersection(TestLine);
        Vector p1 = b2_out.Value(t2_intersect);
        Rectangle r(TestLine.a, TestLine.b);
        if(r.Contains(p1))
        {
          Vector p0 = TestLine.a;
          Vector p2 = TestLine.b;
          number TestLineAngle = -TestLine.Angle();
          p0.Ang(p0.Ang() + TestLineAngle);
          p1.Ang(p1.Ang() + TestLineAngle);
          p2.Ang(p2.Ang() + TestLineAngle);
          number t1_intersect = (p1.x - p0.x) / (p2.x - p0.x) * (t2 - t1) + t1;
          b1_out.Trim(0, t1_intersect);
          b2_out.Trim(t2_intersect, 1.0);
          break;
        }
      }

      //Error correction: ensure curves join at a single point.
      Vector p0, p1, p2, p3, p4, p5, p6, p7;
      b1_out.GetControlPoints(p0, p1, p2, p3);
      b2_out.GetControlPoints(p4, p5, p6, p7);
      p4 = p3;
      b2_out.SetControlPoints(p4, p5, p6, p7);
    }

  public:
    /**Chooses an appropriate algorithm for calculating a displaced line-curve.
    Returns true if the extension line l_ext_out is used.*/
    static bool DisplaceLineMeetsCurve(prim::number Displacement,
      prim::planar::Line& l_in, prim::planar::Bezier& b_in,
      prim::planar::Line& l_out, prim::planar::Line& l_ext_out,
      prim::planar::Bezier& b_out)
    {
      using namespace prim;
      using namespace planar;

      Vector p0, p1, p2, p3;
      b_in.GetControlPoints(p0, p1, p2, p3);
      Line b_tangent(p0, p1);
      number JoinAngle = l_in.Angle() - b_tangent.Angle();
      if(JoinAngle < -Pi)
        JoinAngle += TwoPi;
      if(JoinAngle >= Pi)
        JoinAngle -= TwoPi;

      if(!Chop(JoinAngle, (prim::number)0.001))
      {
        DisplaceLineMeetsCurveOnAngle(Displacement, l_in, b_in, l_out, b_out);
        return false;
      }
      else if((JoinAngle > 0 && Displacement > 0)
        || (JoinAngle < 0 && Displacement <= 0))
      {
        DisplaceLineMeetsCurveBelowAngle(Displacement,
          l_in, b_in, l_out, l_ext_out, b_out);
        return true;
      }
      else
      {
        DisplaceLineMeetsCurveAboveAngle(Displacement,
          l_in, b_in, l_out, b_out);
        return false;
      }
    }

    /**Chooses an appropriate algorithm for calculating a displaced curve-line.
    Returns true if the extension line l_ext_out is used.*/
    static bool DisplaceCurveMeetsLine(prim::number Displacement,
      prim::planar::Bezier& b_in, prim::planar::Line& l_in,
      prim::planar::Bezier& b_out, prim::planar::Line& l_ext_out,
      prim::planar::Line& l_out)
    {
      using namespace prim;
      using namespace planar;

      Vector p0, p1, p2, p3;
      b_in.GetControlPoints(p0, p1, p2, p3);

      Line l_swap(l_in.b, l_in.a);
      Bezier b_swap; b_swap.SetControlPoints(p3, p2, p1, p0);
      bool UseLineExtension = DisplaceLineMeetsCurve(
        -Displacement, l_swap, b_swap, l_out, l_ext_out, b_out);

      b_out.GetControlPoints(p0, p1, p2, p3);
      b_out.SetControlPoints(p3, p2, p1, p0);
      Swap(l_out.a, l_out.b);
      Swap(l_ext_out.a, l_ext_out.b);

      return UseLineExtension;
    }

    ///Chooses an appropriate algorithm for calculating a displaced line-line.
    static void DisplaceLineMeetsLine(prim::number Displacement,
      prim::planar::Line& l1_in, prim::planar::Line& l2_in,
      prim::planar::Line& l1_out, prim::planar::Line& l2_out)
    {
      using namespace prim;
      using namespace planar;

      number JoinAngle = l1_in.Angle() - l2_in.Angle();

      if(Abs(JoinAngle) < 0.001)
      {
        l1_out = l1_in.MakeParallelLine(Displacement);
        l2_out = l2_in.MakeParallelLine(Displacement);

        //Error correction: ensure l1_out and l2_out join at a single point. 
        l2_out.a = l1_out.b;
      }
      else
      {
        l1_out = l1_in.MakeParallelLine(Displacement);
        l2_out = l2_in.MakeParallelLine(Displacement);

        Vector Intersection = l1_out.GetPolarLineIntersection(l2_out);
        l1_out.b = Intersection;
        l2_out.a = Intersection;
      }
    }

    /**Chooses an appropriate algorithm for calculating a displaced curve-curve.
    Returns true if the extension line l_ext_out is used.*/
    static bool DisplaceCurveMeetsCurve(prim::number Displacement,
      prim::planar::Bezier& b1_in, prim::planar::Bezier& b2_in,
      prim::planar::Bezier& b1_out, prim::planar::Line& l1_ext_out,
      prim::planar::Line& l2_ext_out, prim::planar::Bezier& b2_out)
    {
      using namespace prim;
      using namespace planar;

      Vector p0, p1, p2, p3, p4, p5, p6, p7;
      b1_in.GetControlPoints(p0, p1, p2, p3);
      b2_in.GetControlPoints(p4, p5, p6, p7);

      Line b1_tangent(p2, p3);
      Line b2_tangent(p4, p5);
      number JoinAngle = b1_tangent.Angle() - b2_tangent.Angle();
      if(JoinAngle < -Pi)
        JoinAngle += TwoPi;
      if(JoinAngle >= Pi)
        JoinAngle -= TwoPi;

      if(!Chop(JoinAngle, (prim::number)0.001))
      {
        b1_out = b1_in.MakeQuasiParallelCurve(Displacement);
        b2_out = b2_in.MakeQuasiParallelCurve(Displacement);
        
        //Error correction: ensure curves join at a single point.
        b1_out.GetControlPoints(p0, p1, p2, p3);
        b2_out.GetControlPoints(p4, p5, p6, p7);
        p4 = p3;
        b2_out.SetControlPoints(p4, p5, p6, p7);
        return false;
      }
      else if((JoinAngle > 0 && Displacement > 0)
        || (JoinAngle < 0 && Displacement <= 0))
      {
        b1_out = b1_in.MakeQuasiParallelCurve(Displacement);
        b2_out = b2_in.MakeQuasiParallelCurve(Displacement);

        //Have to fill gap with two line extensions.
        b1_out.GetControlPoints(p0, p1, p2, p3);
        b2_out.GetControlPoints(p4, p5, p6, p7);
        Line b1_out_tangent(p2, p3);
        Line b2_out_tangent(p4, p5);
        Vector Intersection = 
          b1_out_tangent.GetLineIntersection(b2_out_tangent);
        l1_ext_out.a = p3;
        l1_ext_out.b = Intersection;
        l2_ext_out.a = Intersection;
        l2_ext_out.b = p4;
        return true;
      }
      else
      {
        DisplaceCurveMeetsCurveAndIntersects(Displacement,
          b1_in, b2_in, b1_out, b2_out);
        return false;
      }
    }
#if 0 //Need to update using new path.
    static void DeriveParallelPath(prim::number Displacement,
      Path& DestinationPath, Path::Component& SourceComponent)
    {
      using namespace prim;
      using namespace planar;

      Path::Component& d = DestinationPath.Components.Add();
      Path::Component& s = SourceComponent;
      
      count s_Curves_n = s.Curves.n() - 2;
      for(count i = 1; i <= s_Curves_n; i++)
      {
        Vector& StartPoint = s.Curves[i - 1].End;
        Path::Curve& LeftCurve = s.Curves[i];
        Path::Curve& RightCurve = s.Curves[i + 1];
        if(!LeftCurve.IsCurve && !RightCurve.IsCurve)
        {
          Line l1(StartPoint, LeftCurve.End), l2(LeftCurve.End, RightCurve.End);
          Line l1_out, l2_out;
          StaffLines::DisplaceLineMeetsLine(Displacement, l1, l2,
            l1_out, l2_out);
          if(i == 1) d.AddCurve(l1_out.a);

          d.AddCurve(l1_out.b);
          if(i == s_Curves_n) d.AddCurve(l2_out.b);
        }
        else if(LeftCurve.IsCurve && !RightCurve.IsCurve)
        {
          Bezier b1; b1.SetControlPoints(StartPoint, LeftCurve.StartControl,
            LeftCurve.EndControl, LeftCurve.End);
          Line l1(LeftCurve.End, RightCurve.End);

          Bezier b_out;
          Line l_ext_out, l_out;
          bool UseExtension = StaffLines::DisplaceCurveMeetsLine(Displacement,
            b1, l1, b_out, l_ext_out, l_out);

          Vector p0, p1, p2, p3;
          b_out.GetControlPoints(p0, p1, p2, p3);
          if(i == 1) d.AddCurve(p0);

          d.AddCurve(b_out);
          if(UseExtension) d.AddCurve(l_ext_out.b);
          if(i == s_Curves_n) d.AddCurve(l_out.b);
        }
        else if(!LeftCurve.IsCurve && RightCurve.IsCurve)
        {
          Line l1(StartPoint, LeftCurve.End);
          Bezier b1; b1.SetControlPoints(LeftCurve.End, RightCurve.StartControl,
            RightCurve.EndControl, RightCurve.End);

          Line l_ext_out, l_out;
          Bezier b_out;
          bool UseExtension = StaffLines::DisplaceLineMeetsCurve(Displacement,
            l1, b1, l_out, l_ext_out, b_out);

          Vector p0, p1, p2, p3;
          b_out.GetControlPoints(p0, p1, p2, p3);
          if(i == 1) d.AddCurve(l_out.a);

          d.AddCurve(l_out.b);
          if(UseExtension) d.AddCurve(l_ext_out.b);
          if(i == s_Curves_n) d.AddCurve(b_out);
        }
        else
        {
          Bezier b1; b1.SetControlPoints(StartPoint, LeftCurve.StartControl,
            LeftCurve.EndControl, LeftCurve.End);
          Bezier b2; b2.SetControlPoints(LeftCurve.End,
            RightCurve.StartControl, RightCurve.EndControl, RightCurve.End);

          Bezier b1_out, b2_out;
          Line l_ext1_out, l_ext2_out;
          bool UseExtension = StaffLines::DisplaceCurveMeetsCurve(Displacement,
            b1, b2, b1_out, l_ext1_out, l_ext2_out, b2_out);

          Vector p0, p1, p2, p3;
          b1_out.GetControlPoints(p0, p1, p2, p3);
          if(i == 1) d.AddCurve(p0);

          d.AddCurve(b1_out);
          if(UseExtension)
          {
            d.AddCurve(l_ext1_out.b);
            d.AddCurve(l_ext2_out.b);
          }
          if(i == s_Curves_n) d.AddCurve(b2_out);
        }
      }
    }
#endif
  };
  
  struct Shapes
  {
    static void AddSelection(Path& p, const prim::planar::Rectangle& Outer,
      const prim::planar::Rectangle& Inner, prim::number CornerRadius)
    {
      using namespace prim;
      using namespace prim::planar;
      
      /*
      ........................6---------5
      . OUTER                 |         |
      0-----------------------7         |
      |       .               .         |
      |       .     INNER     .         |
      |       .               .         |
      |       3-------------------------4
      |       |                   OUTER .
      1-------2..........................
      */
      
      Bezier b[8];
      
      Vector p0(Outer.Left(), Inner.b.y);
      Vector p1(Outer.BottomLeft());
      Vector p2(Inner.a.x, Outer.Bottom());
      Vector p3(Inner.a);
      Vector p4(Outer.Right(), Inner.a.y);
      Vector p5(Outer.TopRight());
      Vector p6(Inner.b.x, Outer.Top());
      Vector p7(Inner.b);
      
      bool MergeTopLine = (p7.x - CornerRadius * 2.0 <= p0.x);
      bool MergeBottomLine = (p3.x + CornerRadius * 2.0 >= p4.x);
      
      if(p7.y <= p3.y && p7.x >= p3.x)
      {
        if(p1.x != p3.x && p5.x != p7.x && p1.y != p3.y && p5.y != p7.y)
        {
          AddSelection(p, prim::planar::Rectangle(p1, p3),
            prim::planar::Rectangle(p1, p3), CornerRadius);
          AddSelection(p, prim::planar::Rectangle(p5, p7),
            prim::planar::Rectangle(p5, p7), CornerRadius);
          return;
        }
        MergeTopLine = MergeBottomLine = true;
      }
      
      if(Outer.a == Inner.a)
        MergeBottomLine = true;

      if(Outer.b == Inner.b)
        MergeTopLine = true;
      
      Vector q1(CornerRadius, CornerRadius);
      Vector q2(-CornerRadius, CornerRadius);
      Vector q3(-CornerRadius, -CornerRadius);
      Vector q4(CornerRadius, -CornerRadius);
     
      Vector r(CornerRadius * 2.0, CornerRadius * 2.0);
      
      if(MergeTopLine) p6.x = p0.x;
      if(MergeBottomLine) p2.x = p4.x;
      
      b[0].Ellipse(p0 - q2, r, 0, 2, true);
      b[1].Ellipse(p1 - q3, r, 0, 3, true);
      b[2].Ellipse(p2 - q4, r, 0, 4, true);
      b[3].Ellipse(p3 - q2, r, 0, 2, false);
      b[4].Ellipse(p4 - q4, r, 0, 4, true);
      b[5].Ellipse(p5 - q1, r, 0, 1, true);
      b[6].Ellipse(p6 - q2, r, 0, 2, true);
      b[7].Ellipse(p7 - q4, r, 0, 4, false);
      
      Vector v1, v2, v3, v4;
      bool First = true;
      for(count i = 0; i < 8; i++)
      {
        if(MergeTopLine && (i == 0 || i == 7)) continue;
        if(MergeBottomLine && (i == 3 || i == 4)) continue;
        
        b[i].GetControlPoints(v1, v2, v3, v4);
        p.Add(Instruction(v1, First));
        p.Add(Instruction(b[i]));
        First = false;
      }
      p.Add(Instruction());
    }
      
    ///Adds a rectangular subpath.
    static void AddRectangle(Path& p, const prim::planar::Rectangle& r,
      bool CounterClockwise = true)
    {
      p.Add(Instruction(r.BottomLeft(), true));
      if(CounterClockwise)
      {
        p.Add(Instruction(r.BottomRight()));
        p.Add(Instruction(r.TopRight()));
        p.Add(Instruction(r.TopLeft()));
      }
      else
      {
        p.Add(Instruction(r.TopLeft()));
        p.Add(Instruction(r.TopRight()));
        p.Add(Instruction(r.BottomRight()));
      }
      p.Add(Instruction());
    }

    ///Adds an elliptical subpath.
    static void AddEllipse(Path& p, prim::planar::Vector Origin,
      prim::planar::Vector Scale, prim::number Rotation,
      bool CounterClockwise = true)
    {
      prim::planar::Bezier b1;
      prim::planar::Bezier b2;
      prim::planar::Bezier b3;
      prim::planar::Bezier b4;

      b1.Ellipse(Origin, Scale, Rotation, 1, CounterClockwise);
      b2.Ellipse(Origin, Scale, Rotation, 2, CounterClockwise);
      b3.Ellipse(Origin, Scale, Rotation, 3, CounterClockwise);
      b4.Ellipse(Origin, Scale, Rotation, 4, CounterClockwise);

      prim::planar::Vector v1, v2, v3, v4;
      
      if(CounterClockwise)
      {
        b1.GetControlPoints(v1, v2, v3, v4);
        p.Add(Instruction(v1, true));
        p.Add(Instruction(b1));
        p.Add(Instruction(b2));
        p.Add(Instruction(b3));
        p.Add(Instruction(b4));
      }
      else
      {
        b4.GetControlPoints(v1, v2, v3, v4);
        p.Add(Instruction(v1, true));
        p.Add(Instruction(b4));
        p.Add(Instruction(b3));
        p.Add(Instruction(b2));
        p.Add(Instruction(b1));
      }
      p.Add(Instruction());
    }

    ///Adds a circular subpath.
    static void AddCircle(Path& p, prim::planar::Vector Origin,
      prim::number Diameter, bool CounterClockwise = true)
    {
      AddEllipse(p, Origin, prim::planar::Vector(Diameter, Diameter), 0,
        CounterClockwise);
    }

    /**Adds the outline of a non-zero thickness line to a Path. Additionally
    you can specify whether either the start or end caps are round (elliptical),
    and specify how elliptical the caps are.*/
    static void AddLine(Path& p, prim::planar::Vector Start,
      prim::planar::Vector End, prim::number Thickness,
      bool CounterClockwise = true, bool StartRoundCap = true,
      bool EndRoundCap = true, prim::number CapRelativeHeight = 1.0f)
    {
      using namespace prim;
      using namespace planar;

      if(Thickness <= 0.0)
        return;
      
      prim::planar::Vector Delta = End - Start;
      prim::number Angle = Delta.Ang();

      prim::planar::Vector t;

      t.Polar(Angle - HalfPi, Thickness / 2.0);

      prim::planar::Vector p0 = Start + t;
      prim::planar::Vector p1 = End + t;
      prim::planar::Vector p2 = End - t;
      prim::planar::Vector p3 = Start - t;

      prim::planar::Bezier Start1;
      prim::planar::Bezier Start2;
      prim::planar::Bezier End1;
      prim::planar::Bezier End2;

      prim::planar::Vector Scale(Thickness, Thickness * CapRelativeHeight);

      End1.Ellipse(End, Scale, Angle - HalfPi, 1, CounterClockwise);
      End2.Ellipse(End, Scale, Angle - HalfPi, 2, CounterClockwise);
      Start1.Ellipse(Start, Scale, Angle - HalfPi, 3, CounterClockwise);
      Start2.Ellipse(Start, Scale, Angle - HalfPi, 4, CounterClockwise);

      p.Add(Instruction(p0, true));

      if(!StartRoundCap && !EndRoundCap)
      {
        if(CounterClockwise)
        {
          p.Add(Instruction(p1));
          p.Add(Instruction(p2));
          p.Add(Instruction(p3));
        }
        else
        {
          p.Add(Instruction(p3));
          p.Add(Instruction(p2));
          p.Add(Instruction(p1));
        }
      }
      else if(EndRoundCap && !StartRoundCap)
      {
        if(CounterClockwise)
        {
          p.Add(Instruction(p1));
          p.Add(Instruction(End1));
          p.Add(Instruction(End2));
          p.Add(Instruction(p3));
        }
        else
        {
          p.Add(Instruction(p3));
          p.Add(Instruction(p2));
          p.Add(Instruction(End2));
          p.Add(Instruction(End1));
        }
      }
      else if(StartRoundCap && !EndRoundCap)
      {
        if(CounterClockwise)
        {
          p.Add(Instruction(p1));
          p.Add(Instruction(p2));
          p.Add(Instruction(p3));
          p.Add(Instruction(Start1));
          p.Add(Instruction(Start2));
        }
        else
        {
          p.Add(Instruction(Start2));
          p.Add(Instruction(Start1));
          p.Add(Instruction(p2));
          p.Add(Instruction(p1));
        }
      }
      else
      {
        if(CounterClockwise)
        {
          p.Add(Instruction(p1));
          p.Add(Instruction(End1));
          p.Add(Instruction(End2));
          p.Add(Instruction(p3));
          p.Add(Instruction(Start1));
          p.Add(Instruction(Start2));
        }
        else
        {
          p.Add(Instruction(Start2));
          p.Add(Instruction(Start1));
          p.Add(Instruction(p2));
          p.Add(Instruction(End2));
          p.Add(Instruction(End1));
        }
      }
      p.Add(Instruction());
    }

    ///Adds rectangular subpaths from outlined lines.
    static void AddRectangleFromLines(Path& p, const prim::planar::Rectangle& r,
      prim::number Thickness)
    {
      AddLine(p, r.TopRight(), r.BottomRight(), Thickness);
      AddLine(p, r.BottomRight(), r.BottomLeft(), Thickness);
      AddLine(p, r.BottomLeft(), r.TopLeft(), Thickness);
      AddLine(p, r.TopLeft(), r.TopRight(), Thickness);
    }
    
    ///Adds rectangular subpaths from outlined lines.
    static void AddXFromLines(Path& p, const prim::planar::Rectangle& r,
      prim::number Thickness)
    {
      AddLine(p, r.BottomLeft(), r.TopRight(), Thickness);
      AddLine(p, r.TopLeft(), r.BottomRight(), Thickness);
    }
    
    /**Helps debug those pesky affine transformations. Create a path and call
    this method on the path. Then each time you call a transform method on the
    painter, draw the axis path. This will provide a visible record of what is
    happening to the affine space.*/
    static void AddCoordinateAxis(Path& p)
    {
      const prim::number Extent = 5.0;
      const prim::number TotalExtent = Extent + 1.0;
      const prim::number Thickness = 0.02;
      const prim::number TickWidth = 0.1;
      Shapes::AddLine(p, prim::planar::Vector(-TotalExtent, 0),
        prim::planar::Vector(TotalExtent, 0), Thickness);
      Shapes::AddLine(p, prim::planar::Vector(0, -TotalExtent),
        prim::planar::Vector(0, TotalExtent), Thickness);
      
      for(prim::number i = -Extent; i <= Extent; i++)
      {
        if(i == 0) continue;
        Shapes::AddLine(p, prim::planar::Vector(-TickWidth, i),
          prim::planar::Vector(TickWidth, i), Thickness);
        Shapes::AddLine(p, prim::planar::Vector(i, -TickWidth),
          prim::planar::Vector(i, TickWidth), Thickness);
      }
    }

    struct Music
    {
      ///Adds subpaths to create a whole note.
      static void AddWholeNote(Path& p, prim::planar::Vector HeadOrigin,
        prim::number SpaceHeight = 1.0, prim::number RelativeWidth = 1.7,
        prim::number HollowScale = 0.55,
        prim::number HollowAngle = 0.75 * prim::Pi)
      {
        using namespace prim;
        using namespace planar;
        
        Vector HeadScale(SpaceHeight * RelativeWidth, SpaceHeight);
        Vector HollowVectorScale = HeadScale;
        HollowVectorScale *= HollowScale;

        AddEllipse(p, HeadOrigin, HeadScale, 0, true);
        AddEllipse(p, HeadOrigin, HollowVectorScale, HollowAngle, false);
      }

      ///Adds subpaths to create a quarter note.
      static void AddQuarterNote
      (
        Path&                 p,
        prim::planar::Vector  HeadOrigin,
        prim::number          SpaceHeight           = 1.0,
        bool                  MakeSingleOutline     = true,
        prim::number          RelativeStemHeight    = 4.0,
        prim::planar::Vector* FlagPosition          = 0,
        prim::number          HeadTheta             = 20.0 * prim::Deg,
        prim::number          RelativeWidth         = 1.4,
        prim::number          RelativeStemThickness = 0.1,
        prim::number          RelativeStemCapHeight = 0.8,
        bool                  IsHollow              = false,
        prim::number          HollowScale           = 0.48,
        prim::number          HollowTheta           = prim::HalfPi * 0.4
      )
      {
        using namespace prim;
        using namespace planar;

        Vector HeadScale(SpaceHeight * RelativeWidth, SpaceHeight);
        number StemThickness = RelativeStemThickness * SpaceHeight;
        number StemHeight = RelativeStemHeight * SpaceHeight;

        //Create the hollow area if necessary.
        if(IsHollow)
        {
          Vector HollowVectorScale = HeadScale * HollowScale;

          Vector Start;
          Start.Polar(HollowTheta + Pi, HollowVectorScale.x / 2.0);
          Start += HeadOrigin;

          Vector End;
          End.Polar(HollowTheta, HollowVectorScale.x / 2.0);
          End += HeadOrigin;

          AddLine(p, Start, End, HollowVectorScale.y, false, true, true, 1.0);
        }

        if(!MakeSingleOutline || Abs(StemHeight) < HeadScale.y / 2.0)
        {
          /*Algorithm 1 - Draw an ellipse and overlay the stem as two different
          subpaths. Only draw a line if the height is non-zero, and draw on the
          right if it is positive, and on the left if it is negative.*/
          
          AddEllipse(p, HeadOrigin, HeadScale, HeadTheta, true);

          if(StemHeight > HeadScale.y / 2.0)
          {
            Vector Start = Ellipse::VerticalTangent(
              HeadScale.x / 2.0, HeadScale.y / 2.0, HeadTheta);

            StemHeight -= Start.y;

            Start.x = HeadOrigin.x + Start.x;
            Start.y = HeadOrigin.y + Start.y;

            Start.x -= StemThickness / 2.0;
            Vector End = Start;
            End.y += StemHeight;
            AddLine(p, Start, End, StemThickness, true, false, true,
              RelativeStemCapHeight);

            if(FlagPosition)
              *FlagPosition = Vector(End.x + StemThickness / 2.0, End.y);
          }
          else if(StemHeight < -HeadScale.y / 2.0)
          {
            Vector Start = Ellipse::VerticalTangent(
              HeadScale.x / 2.0, HeadScale.y / 2.0, HeadTheta);

            StemHeight += Start.y;

            Start.x = HeadOrigin.x - Start.x;
            Start.y = HeadOrigin.y - Start.y;

            Start.x += StemThickness / 2.0;
            Vector End = Start;
            End.y += StemHeight;
            AddLine(p, Start, End, StemThickness, true, false, true,
              RelativeStemCapHeight);
            if(FlagPosition)
              *FlagPosition = Vector(End.x - StemThickness / 2.0, End.y);
          }
        }
        else
        {
          /*Algorithm 2 - Calculate the outline exactly. Somewhat tedious and
          lots of math, but highly worth it for the ability to outline notes.*/

          //Step 1 - Calculate the intersections.
          Bezier c1;
          Bezier c4;

          c1.Ellipse(HeadOrigin, HeadScale, HeadTheta, 1, true);
          c4.Ellipse(HeadOrigin, HeadScale, HeadTheta, 4, true);

          //Quadrant 1 intersection
          number t1 = Ellipse::VerticalIntersection(
            HeadScale.x / 2.0, HeadScale.y / 2.0, HeadTheta, StemThickness);

          //Quadrant 4 intersection
          number t4;
          number Dummy;
          if(!c4.VerticalTangents(t4, Dummy))
            return;

          //Step 2 - Declare the path segment variables.
          Bezier p1;
          Bezier p2;
          Bezier p3;
          Bezier p4;
          Vector l5_Start;
          Vector l5_End;
          Bezier p6;
          Bezier p7;
          Vector l8_Start;
          Vector l8_End;

          /*Step 3 - Create each part of the path, depending on whether the stem
          is on the right or left.*/
          if(StemHeight > 0.0)
          {
            p1.Ellipse(HeadOrigin, HeadScale, HeadTheta, 1, true);
            p1.Trim(t1, 1.0); //Trim the curve.
            p2.Ellipse(HeadOrigin, HeadScale, HeadTheta, 2, true);
            p3.Ellipse(HeadOrigin, HeadScale, HeadTheta, 3, true);
            p4.Ellipse(HeadOrigin, HeadScale, HeadTheta, 4, true);
            p4.Trim(0.0,t4); //Trim the curve.

            l5_Start = p4.Value(1.0);
            l5_End = l5_Start;
            l5_End.y = HeadOrigin.y + StemHeight;

            Vector p6origin(l5_End.x - StemThickness / 2.0, l5_End.y);
            Vector p6scale(StemThickness,
              StemThickness * RelativeStemCapHeight);
            
            p6.Ellipse(p6origin, p6scale, 0.0, 1, true);
            p7.Ellipse(p6origin, p6scale, 0.0, 2, true);

            l8_Start = l5_End;
            l8_Start.x -= StemThickness;
            l8_End = p1.Value(0);

            if(FlagPosition)
              *FlagPosition = l5_End;
          }
          else
          {
            p1.Ellipse(HeadOrigin, HeadScale, HeadTheta + Pi, 1, true);
            p1.Trim(t1,1.0); //Trim the curve.
            p2.Ellipse(HeadOrigin, HeadScale, HeadTheta + Pi, 2, true);
            p3.Ellipse(HeadOrigin, HeadScale, HeadTheta + Pi, 3, true);
            p4.Ellipse(HeadOrigin, HeadScale, HeadTheta + Pi, 4, true);
            p4.Trim(0.0, t4); //Trim the curve.
            
            l5_Start = p4.Value(1.0);
            l5_End = l5_Start;
            l5_End.y = HeadOrigin.y + StemHeight;

            Vector p6origin(l5_End.x + StemThickness / 2.0, l5_End.y);
            Vector p6scale(StemThickness,
              StemThickness * RelativeStemCapHeight);
            
            p6.Ellipse(p6origin, p6scale, 0.0, 3, true);
            p7.Ellipse(p6origin, p6scale, 0.0, 4, true);

            l8_Start = l5_End;
            l8_Start.x += StemThickness;
            l8_End = p1.Value(0.0);

            if(FlagPosition)
              *FlagPosition = l8_Start;
          }

          //Step 4 - Create a sub path.
          p.Add(Instruction(l8_End, true));
          p.Add(Instruction(p1));
          p.Add(Instruction(p2));
          p.Add(Instruction(p3));
          p.Add(Instruction(p4));
          p.Add(Instruction(l5_End));
          p.Add(Instruction(p6));
          p.Add(Instruction(p7));
          p.Add(Instruction(l8_End));
          p.Add(Instruction());
        }
      }

      ///Adds subpaths to create a half note.
      static void AddHalfNote
      (
        Path&                 p,
        prim::planar::Vector  HeadOrigin,
        prim::number          SpaceHeight           = 1.0,
        bool                  MakeSingleOutline     = true,
        prim::number          RelativeStemHeight    = 4.0,
        prim::planar::Vector* FlagPosition          = 0,
        prim::number          HeadTheta             = 20.0 * prim::Deg,
        prim::number          RelativeWidth         = 1.4,
        prim::number          RelativeStemThickness = 0.1,
        prim::number          RelativeStemCapHeight = 0.8,
        prim::number          HollowScale           = 0.48,
        prim::number          HollowTheta           = 0.4 * prim::HalfPi
      )
      {
        AddQuarterNote(p, HeadOrigin, SpaceHeight, MakeSingleOutline,
          RelativeStemHeight, FlagPosition, HeadTheta, RelativeWidth,
          RelativeStemThickness, RelativeStemCapHeight, true, HollowScale,
          HollowTheta);
      }
#if 0 //Needs conversion to new path...
      ///Adds a convex slur to a path.
      static void AddSlur(Path& p, prim::planar::Vector a,
        prim::planar::Vector b, prim::number inSpaceHeight,
        prim::number relativeArchHeight = 0.1f, 
        prim::number relativeArchWidth = 0.5f, 
        prim::number shMaxThickness = 0.2f,
        prim::number shMinThickness = 0.06f,
        prim::number relativeCapHeight = 2.0f)
      {
        using namespace prim;
        using namespace planar;

        number theta = a.Ang(b);
        number perpen = theta + HalfPi;
        number width = a.Mag(b);

        number shArchHeight = relativeArchHeight * (width / inSpaceHeight);
        if(shArchHeight > 2.5f)
          shArchHeight = 2.5f;
        if(shArchHeight < -2.5f)
          shArchHeight = -2.5f;

        shArchHeight *= inSpaceHeight;
        shMaxThickness *= inSpaceHeight;
        shMinThickness *= inSpaceHeight;

        Vector ghost_a1 = a;
        ghost_a1 += Vector(perpen) * shArchHeight;

        Vector c = ghost_a1;
        c += Vector(theta) * width * (0.5f - relativeArchWidth * 0.5f);

        Vector d = ghost_a1;
        d += Vector(theta) * width * (0.5f + relativeArchWidth * 0.5f);

        Vector ghost_a2 = a;
        ghost_a2 += Vector(perpen) * (shArchHeight + shMaxThickness);

        Vector e = ghost_a2;
        e += Vector(theta) * width * (0.5f - relativeArchWidth * 0.5f);

        Vector f = ghost_a2;
        f += Vector(theta) * width * (0.5f + relativeArchWidth * 0.5f);

        Vector g = a;
        g += Vector(a.Ang(c) + HalfPi) * shMinThickness;

        Vector h = b;
        h += Vector(b.Ang(d) - HalfPi) * shMinThickness;

        Vector i = (a + g) * 0.5f;
        Vector j = (b + h) * 0.5f;

        Bezier ellipse_bj;
        Bezier ellipse_jh;
        Bezier ellipse_gi;
        Bezier ellipse_ia;

        Vector ellipse_scale(shMinThickness,
          shMinThickness * relativeCapHeight);
        ellipse_bj.Ellipse(j, ellipse_scale, d.Ang(b) - HalfPi, 1, true);
        ellipse_jh.Ellipse(j, ellipse_scale, d.Ang(b) - HalfPi, 2, true);
        ellipse_gi.Ellipse(i, ellipse_scale, a.Ang(c) + HalfPi, 1, true);
        ellipse_ia.Ellipse(i, ellipse_scale, a.Ang(c) + HalfPi, 2, true);

        Vector bj[4];
        Vector jh[4];
        Vector gi[4];
        Vector ia[4];

        ellipse_bj.GetControlPoints(bj[0],bj[1],bj[2],bj[3]);
        ellipse_jh.GetControlPoints(jh[0],jh[1],jh[2],jh[3]);
        ellipse_gi.GetControlPoints(gi[0],gi[1],gi[2],gi[3]);
        ellipse_ia.GetControlPoints(ia[0],ia[1],ia[2],ia[3]);

        p.AddComponent(a);
        p.AddCurve(c, d, b);
        p.AddCurve(bj[1], bj[2], bj[3]);
        p.AddCurve(jh[1], jh[2], jh[3]);
        p.AddCurve(f, e, g);
        p.AddCurve(gi[1], gi[2], gi[3]);
        p.AddCurve(ia[1], ia[2], ia[3]);
      }

      struct SlurControlPoint
      {
        prim::planar::Vector Start;
        prim::number degStartAngle;
        prim::number shStartDepth;
        prim::number shEndDepth;
        
        SlurControlPoint() : degStartAngle(0),
          shStartDepth(1.0f), shEndDepth(1.0f) {}

        SlurControlPoint(prim::planar::Vector Start, prim::number degStartAngle,
          prim::number shStartDepth = 1.0f, prim::number shEndDepth = 1.0f)
        {
          SlurControlPoint::Start = Start;
          SlurControlPoint::degStartAngle = degStartAngle;
          SlurControlPoint::shStartDepth = shStartDepth;
          SlurControlPoint::shEndDepth = shEndDepth;
        }
      };

      static void AddSlur(Path& p,
        prim::Array<SlurControlPoint>& ControlPoints,
        prim::number inSpaceHeight,
        prim::number shMaxThickness = 0.2f,
        prim::number shMinThickness = 0.06f,
        prim::number relativeCapHeight = 2.0f)
      {
        using namespace prim;
        using namespace planar;

        //Stop unreferenced formal parameter warning.
        inSpaceHeight += relativeCapHeight * 0.0f;

        //Make sure at least two control points are in the list.
        if(ControlPoints.n() < 2)
          return;

        //Begin the new path component.
        p.AddComponent();

        //Cached information
        Vector FirstPoint;

        //Trace upper part of slur hull.
        for(count i = 0; i < ControlPoints.n() - 1; i++)
        {
          SlurControlPoint a = ControlPoints[i];
          SlurControlPoint b = ControlPoints[i + 1];
          
          Vector a_Start = a.Start;
          Vector b_Start = b.Start;

          Vector a_Control = a_Start + Vector(a.degStartAngle * prim::Deg) * 
            a.shStartDepth * inSpaceHeight;
          Vector b_Control = b_Start - Vector(b.degStartAngle * prim::Deg) *
            a.shEndDepth * inSpaceHeight;

          Vector p0, p1, p2, p3;
          p0 = a_Start + Vector((a.degStartAngle + 90.0f) * prim::Deg) *
            shMinThickness * inSpaceHeight * 0.5f;
          p3 = b_Start + Vector((b.degStartAngle + 90.0f) * prim::Deg) *
            shMinThickness * inSpaceHeight * 0.5f;

          p1 = a_Control + Vector((a.degStartAngle + 90.0f) * prim::Deg) *
            shMaxThickness * inSpaceHeight * 0.5f;
          p2 = b_Control + Vector((b.degStartAngle + 90.0f) * prim::Deg) *
            shMaxThickness * inSpaceHeight * 0.5f;

          if(i == 0)
          {
            FirstPoint = p0;
            p.AddCurve(p0);
          }

          p.AddCurve(p1, p2, p3);
        }

        //Add right end cap.
        /*
        Bezier ellipse_r1;
        Bezier ellipse_r2;

        Vector ellipse_scale(shMinThickness,
          shMinThickness * relativeCapHeight);
        ellipse_bj.Ellipse(j, ellipse_scale, d.Ang(b) - math::HalfPi, 1, true);
        ellipse_jh.Ellipse(j, ellipse_scale, d.Ang(b) - math::HalfPi, 2, true);
        ellipse_gi.Ellipse(i, ellipse_scale, a.Ang(c) + math::HalfPi, 1, true);
        ellipse_ia.Ellipse(i, ellipse_scale, a.Ang(c) + math::HalfPi, 2, true);

        Vector bj[4];
        Vector jh[4];
        Vector gi[4];
        Vector ia[4];

        ellipse_bj.GetControlPoints(bj[0],bj[1],bj[2],bj[3]);
        ellipse_jh.GetControlPoints(jh[0],jh[1],jh[2],jh[3]);
        ellipse_gi.GetControlPoints(gi[0],gi[1],gi[2],gi[3]);
        ellipse_ia.GetControlPoints(ia[0],ia[1],ia[2],ia[3]);

        p.AddComponent(a);
        p.AddCurve(c, d, b);
        p.AddCurve(bj[1], bj[2], bj[3]);
        p.AddCurve(jh[1], jh[2], jh[3]);
        */

        //Trace backwards on the lower part of the slur hull.
        for(count i = ControlPoints.n() - 2; i >= 0; i--)
        {
          SlurControlPoint a = ControlPoints[i];
          SlurControlPoint b = ControlPoints[i + 1];
          
          Vector a_Start = a.Start;
          Vector b_Start = b.Start;

          Vector a_Control = a_Start + Vector(a.degStartAngle * prim::Deg) *
            a.shStartDepth * inSpaceHeight;
          Vector b_Control = b_Start - Vector(b.degStartAngle * prim::Deg) *
            a.shEndDepth * inSpaceHeight;

          Vector p0, p1, p2, p3;
            
          p0 = a_Start - Vector((a.degStartAngle + 90.0f) * prim::Deg) *
            shMinThickness * inSpaceHeight * 0.5f;
          p3 = b_Start - Vector((b.degStartAngle + 90.0f) * prim::Deg) *
            shMinThickness * inSpaceHeight * 0.5f;

          p1 = a_Control - Vector((a.degStartAngle + 90.0f) * prim::Deg) *
            shMaxThickness * inSpaceHeight * 0.5f;
          p2 = b_Control - Vector((b.degStartAngle + 90.0f) * prim::Deg) *
            shMaxThickness * inSpaceHeight * 0.5f;

          if(i == ControlPoints.n() - 2)
            p.AddCurve(p3); //Temporary to create right cap.

          p.AddCurve(p2, p1, p0);
        }

        //Add left end cap.
        p.AddCurve(FirstPoint);
      }
#endif
#if 0 //Deprecated
      ///Adds a convex slur to a path.
      static void AddPolygonSlur(Path& p, prim::planar::Vector a,
        prim::planar::Vector b, prim::number inSpaceHeight,
        prim::number relativeArchHeight = 0.1f, 
        prim::number relativeArchWidth = 0.5f, 
        prim::number shMaxThickness = 0.2f,
        prim::number shMinThickness = 0.06f,
        prim::number relativeCapHeight = 2.0f)
      {
        using namespace prim;
        using namespace planar;

        number theta = a.Ang(b);
        number perpen = theta + math::HalfPi;
        number width = a.Mag(b);

        number shArchHeight = relativeArchHeight * (width / inSpaceHeight);
        if(shArchHeight > 2.5f)
          shArchHeight = 2.5f;
        if(shArchHeight < -2.5f)
          shArchHeight = -2.5f;

        shArchHeight *= inSpaceHeight;
        shMaxThickness *= inSpaceHeight;
        shMinThickness *= inSpaceHeight;

        Vector ghost_a1 = a;
        ghost_a1 += Vector(perpen) * shArchHeight;

        Vector c = ghost_a1;
        c += Vector(theta) * width * (0.5f - relativeArchWidth * 0.5f);

        Vector d = ghost_a1;
        d += Vector(theta) * width * (0.5f + relativeArchWidth * 0.5f);

        Vector ghost_a2 = a;
        ghost_a2 += Vector(perpen) * (shArchHeight + shMaxThickness);

        Vector e = ghost_a2;
        e += Vector(theta) * width * (0.5f - relativeArchWidth * 0.5f);

        Vector f = ghost_a2;
        f += Vector(theta) * width * (0.5f + relativeArchWidth * 0.5f);

        Vector g = a;
        g += Vector(a.Ang(c) + math::HalfPi) * shMinThickness;

        Vector h = b;
        h += Vector(b.Ang(d) - math::HalfPi) * shMinThickness;

        Vector i = (a + g) * 0.5f;
        Vector j = (b + h) * 0.5f;

        Bezier ellipse_bj;
        Bezier ellipse_jh;
        Bezier ellipse_gi;
        Bezier ellipse_ia;

        Vector ellipse_scale(shMinThickness,
          shMinThickness * relativeCapHeight);
        ellipse_bj.Ellipse(j, ellipse_scale, d.Ang(b) - math::HalfPi, 1, true);
        ellipse_jh.Ellipse(j, ellipse_scale, d.Ang(b) - math::HalfPi, 2, true);
        ellipse_gi.Ellipse(i, ellipse_scale, a.Ang(c) + math::HalfPi, 1, true);
        ellipse_ia.Ellipse(i, ellipse_scale, a.Ang(c) + math::HalfPi, 2, true);

        Vector bj[4];
        Vector jh[4];
        Vector gi[4];
        Vector ia[4];

        ellipse_bj.GetControlPoints(bj[0],bj[1],bj[2],bj[3]);
        ellipse_jh.GetControlPoints(jh[0],jh[1],jh[2],jh[3]);
        ellipse_gi.GetControlPoints(gi[0],gi[1],gi[2],gi[3]);
        ellipse_ia.GetControlPoints(ia[0],ia[1],ia[2],ia[3]);

        p.AddComponent(a);
        p.AddCurve(c, d, b);
        p.AddCurve(bj[1], bj[2], bj[3]);
        p.AddCurve(jh[1], jh[2], jh[3]);
        p.AddCurve(f, e, g);
        p.AddCurve(gi[1], gi[2], gi[3]);
        p.AddCurve(ia[1], ia[2], ia[3]);
      }
#endif

      ///Adds a grand staff brace to a path.
      static void AddBrace(Path& p,
        prim::planar::Vector Center,
        prim::number Height,
        prim::number RelativeWidth = (prim::number)0.1)
      {
        /*We're using a set of control points from a quadratic Bezier 
        curve taken from a font program, due to the complexity of the 
        shape.*/
        using namespace prim;
        using namespace planar;

        prim::planar::Vector v[44];

        //Even are anchors, odd are control points.
        v[0]  = prim::planar::Vector(12.5f ,  12.5f);
        v[1]  = prim::planar::Vector(25.0f ,  25.0f);
        v[2]  = prim::planar::Vector(87.5f ,  87.5f);
        v[3]  = prim::planar::Vector(150.0f,  150.0f);
        v[4]  = prim::planar::Vector(100.0f,  325.0f);
        v[5]  = prim::planar::Vector(50.0f ,  500.0f);
        v[6]  = prim::planar::Vector(25.0f ,  650.0f);
        v[7]  = prim::planar::Vector(0.0f  ,  800.0f);
        v[8]  = prim::planar::Vector(75.0f ,  925.0f);
        v[9]  = prim::planar::Vector(150.0f,  1050.0f);
        v[10] = prim::planar::Vector(175.0f,  1025.0f);
        v[11] = prim::planar::Vector(200.0f,  1000.0f);
        v[12] = prim::planar::Vector(125.0f,  925.0f);
        v[13] = prim::planar::Vector(50.0f ,  850.0f);
        v[14] = prim::planar::Vector(100.0f,  675.0f);
        v[15] = prim::planar::Vector(150.0f,  500.0f);
        v[16] = prim::planar::Vector(175.0f,  350.0f);
        v[17] = prim::planar::Vector(200.0f,  200.0f);
        v[18] = prim::planar::Vector(175.0f,  125.0f);
        v[19] = prim::planar::Vector(150.0f,  50.0f);
        v[20] = prim::planar::Vector(125.0f,  25.0f);
        v[21] = prim::planar::Vector(100.0f,  0.0f);
        v[22] = prim::planar::Vector(125.0f, -25.0f);
        v[23] = prim::planar::Vector(150.0f, -50.0f);
        v[24] = prim::planar::Vector(175.0f, -125.0f);
        v[25] = prim::planar::Vector(200.0f, -200.0f);
        v[26] = prim::planar::Vector(175.0f, -350.0f);
        v[27] = prim::planar::Vector(150.0f, -500.0f);
        v[28] = prim::planar::Vector(100.0f, -675.0f);
        v[29] = prim::planar::Vector(50.0f , -850.0f);
        v[30] = prim::planar::Vector(125.0f, -925.0f);
        v[31] = prim::planar::Vector(200.0f, -1000.0f);
        v[32] = prim::planar::Vector(175.0f, -1025.0f);
        v[33] = prim::planar::Vector(150.0f, -1050.0f);
        v[34] = prim::planar::Vector(75.0f , -925.0f);
        v[35] = prim::planar::Vector(0.0f  , -800.0f);
        v[36] = prim::planar::Vector(25.0f , -650.0f);
        v[37] = prim::planar::Vector(50.0f , -500.0f);
        v[38] = prim::planar::Vector(100.0f, -325.0f);
        v[39] = prim::planar::Vector(150.0f, -150.0f);
        v[40] = prim::planar::Vector(87.5f , -87.5f);
        v[41] = prim::planar::Vector(25.0f , -25.0f);
        v[42] = prim::planar::Vector(12.5f , -12.5f);
        v[43] = prim::planar::Vector(0.0f  ,  0.0f);

        //Transform into context space.
        for(prim::count i = 0; i < 44; i++)
        {
          prim::number mult  = Height / (prim::number)(2000.0 * 0.95);
          prim::number multx = RelativeWidth / (prim::number)0.1;
          v[i] *= mult;
          v[i].x *= multx;
          v[i] += Center;
        }

        //Create the curve.
        p.Add(Instruction(v[0], true));
        for(prim::count i = 0; i <= 42; i+= 2)
        {
          prim::planar::Bezier b;
          prim::planar::Vector p0 = v[i];
          prim::planar::Vector p1 = v[i + 1];
          prim::planar::Vector p2 =
            v[prim::Mod((prim::integer)i + 2, (prim::integer)44)];
          b.SetControlPoints(p0, p1, p2);
          p.Add(Instruction(b));
        }
        p.Add(Instruction());
      }
    };
  };
}
#endif
