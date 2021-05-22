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

#ifndef BELLE_CORE_SHAPES_H
#define BELLE_CORE_SHAPES_H

#include "belle-path.h"
#include "belle-abstracts.h"

namespace BELLE_NAMESPACE
{
  class StaffLines
  {
    static void DisplaceLineMeetsCurveOnAngle(number Displacement,
      Line& l_in, Bezier& b_in,
      Line& l_out, Bezier& b_out)
    {
      /*Trivial case: line meets the curve on angle (the slope of the line
      equals (or is extremely close to) the slope of the curve initially).*/
      Vector p0, p1, p2, p3;

      l_out = l_in.MakeParallelLine(Displacement);
      b_out = b_in.MakeQuasiParallelCurve(Displacement);

      //Error correction: force the bezier curve to start where the line stops.
      b_out.GetControlPoints(p0, p1, p2, p3);
      p0 = l_out.b;
      b_out.SetControlPoints(p0, p1, p2, p3);
    }

    static void DisplaceLineMeetsCurveBelowAngle(number Displacement,
      Line& l_in, Bezier& b_in,
      Line& l_out, Line& l2_out,
      Bezier& b_out)
    {
      /*Gap case: There is a gap that needs to be filled by an additional line
      segment that has the same slope as the curve's initial tangent and
      extends from the intersection with the line to the intersection with the
      curve's first point.*/
      Vector p0, p1, p2, p3;

      l_out = l_in.MakeParallelLine(Displacement);
      b_out = b_in.MakeQuasiParallelCurve(Displacement);

      //Calculate the extension line.
      b_out.GetControlPoints(p0, p1, p2, p3);
      Line b_tangent(p0, p1);
      l_out.b = l_out.GetLineIntersection(b_tangent);
      l2_out.a = l_out.b;
      l2_out.b = p0;
    }

    static void DisplaceLineMeetsCurveAboveAngle(number Displacement,
      Line& l_in, Bezier& b_in,
      Line& l_out, Bezier& b_out)
    {
      /*Overlap case: The displaced line overlaps with the displaced curve. This
      problem reduces into finding the intersection of the displaced line and
      curve and making that the join.*/
      Vector p0, p1, p2, p3;

      l_out = l_in.MakeParallelLine(Displacement);
      b_out = b_in.MakeQuasiParallelCurve(Displacement);

      number t_intersection = b_out.FindLineIntersection(l_out);
      l_out.b = b_out.Value(t_intersection);
      b_out.Trim(t_intersection, 1.f);
    }

    static void DisplaceCurveMeetsCurveAndIntersects(number Displacement,
      Bezier& b1_in, Bezier& b2_in,
      Bezier& b1_out, Bezier& b2_out)
    {
      b1_out = b1_in.MakeQuasiParallelCurve(Displacement);
      b2_out = b2_in.MakeQuasiParallelCurve(Displacement);

      count Segmentation = 100;
      for(count i = 0; i < Segmentation; i++)
      {
        number t1 = number(i) / number(Segmentation);
        number t2 = number(i + 1) / number(Segmentation);
        Line TestLine(b1_out.Value(t1), b1_out.Value(t2));
        number t2_intersect = b2_out.FindLineIntersection(TestLine);
        Vector p1 = b2_out.Value(t2_intersect);
        Box r(TestLine.a, TestLine.b);
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
          b2_out.Trim(t2_intersect, 1.f);
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
    static bool DisplaceLineMeetsCurve(number Displacement,
      Line& l_in, Bezier& b_in,
      Line& l_out, Line& l_ext_out,
      Bezier& b_out)
    {
      Vector p0, p1, p2, p3;
      b_in.GetControlPoints(p0, p1, p2, p3);
      Line b_tangent(p0, p1);
      number JoinAngle = l_in.Angle() - b_tangent.Angle();
      if(JoinAngle < -Pi<number>())
        JoinAngle += TwoPi<number>();
      if(JoinAngle >= Pi<number>())
        JoinAngle -= TwoPi<number>();

      if(Limits<number>::IsZero(Chop(JoinAngle, number(0.001))))
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
    static bool DisplaceCurveMeetsLine(number Displacement,
      Bezier& b_in, Line& l_in,
      Bezier& b_out, Line& l_ext_out,
      Line& l_out)
    {
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
    static void DisplaceLineMeetsLine(number Displacement,
      Line& l1_in, Line& l2_in,
      Line& l1_out, Line& l2_out)
    {
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
    static bool DisplaceCurveMeetsCurve(number Displacement,
      Bezier& b1_in, Bezier& b2_in,
      Bezier& b1_out, Line& l1_ext_out,
      Line& l2_ext_out, Bezier& b2_out)
    {
      Vector p0, p1, p2, p3, p4, p5, p6, p7;
      b1_in.GetControlPoints(p0, p1, p2, p3);
      b2_in.GetControlPoints(p4, p5, p6, p7);

      Line b1_tangent(p2, p3);
      Line b2_tangent(p4, p5);
      number JoinAngle = b1_tangent.Angle() - b2_tangent.Angle();
      if(JoinAngle < -Pi<number>())
        JoinAngle += TwoPi<number>();
      if(JoinAngle >= Pi<number>())
        JoinAngle -= TwoPi<number>();

      if(Limits<number>::IsZero(Chop(JoinAngle, number(0.001))))
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
    static void DeriveParallelPath(number Displacement,
      Path& DestinationPath, Path::Component& SourceComponent)
    {
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
    static void AddQuestionMark(Path& p, Affine a)
    {
      Path q;
      q.Add(Instruction(Vector(-0.027f, 0.733f), true));
      q.AddQuadratic(Vector(-0.03f, 0.709f), Vector(-0.033f, 0.686f));
      q.AddQuadratic(Vector(-0.036f, 0.663f), Vector(-0.036f, 0.64f));
      q.AddQuadratic(Vector(-0.036f, 0.568f), Vector(-0.015f, 0.516f));
      q.AddQuadratic(Vector(0.006f, 0.464f), Vector(0.032f, 0.418f));
      q.AddQuadratic(Vector(0.057f, 0.372f), Vector(0.078f, 0.324f));
      q.AddQuadratic(Vector(0.1f, 0.276f), Vector(0.1f, 0.211f));
      q.AddQuadratic(Vector(0.1f, 0.187f), Vector(0.094f, 0.163f));
      q.AddQuadratic(Vector(0.089f, 0.139f), Vector(0.077f, 0.12f));
      q.AddQuadratic(Vector(0.065f, 0.102f), Vector(0.045f, 0.09f));
      q.AddQuadratic(Vector(0.025f, 0.078f), Vector(-0.003f, 0.078f));
      q.AddQuadratic(Vector(-0.062f, 0.078f), Vector(-0.085f, 0.117f));
      q.AddQuadratic(Vector(-0.108f, 0.156f), Vector(-0.108f, 0.208f));
      q.AddQuadratic(Vector(-0.108f, 0.223f), Vector(-0.106f, 0.24f));
      q.AddQuadratic(Vector(-0.103f, 0.256f), Vector(-0.1f, 0.272f));
      q.Add(Instruction(Vector(-0.195f, 0.298f)));
      q.AddQuadratic(Vector(-0.2f, 0.284f), Vector(-0.202f, 0.269f));
      q.AddQuadratic(Vector(-0.204f, 0.254f), Vector(-0.204f, 0.239f));
      q.AddQuadratic(Vector(-0.204f, 0.191f), Vector(-0.186f, 0.155f));
      q.AddQuadratic(Vector(-0.168f, 0.119f), Vector(-0.138f, 0.096f));
      q.AddQuadratic(Vector(-0.108f, 0.072f), Vector(-0.069f, 0.06f));
      q.AddQuadratic(Vector(-0.03f, 0.048f), Vector(0.014f, 0.048f));
      q.AddQuadratic(Vector(0.056f, 0.048f), Vector(0.092f, 0.062f));
      q.AddQuadratic(Vector(0.129f, 0.076f), Vector(0.157f, 0.101f));
      q.AddQuadratic(Vector(0.185f, 0.126f), Vector(0.201f, 0.161f));
      q.AddQuadratic(Vector(0.217f, 0.196f), Vector(0.217f, 0.239f));
      q.AddQuadratic(Vector(0.217f, 0.299f), Vector(0.193f, 0.343f));
      q.AddQuadratic(Vector(0.169f, 0.387f), Vector(0.136f, 0.426f));
      q.AddQuadratic(Vector(0.102f, 0.465f), Vector(0.067f, 0.507f));
      q.AddQuadratic(Vector(0.032f, 0.549f), Vector(0.01f, 0.605f));
      q.AddQuadratic(Vector(-0.012f, 0.665f), Vector(-0.01f, 0.733f));
      q.Add(Instruction());
      q.Add(Instruction(Vector(-0.007f, 0.802f)));
      q.AddQuadratic(Vector(0.024f, 0.802f), Vector(0.046f, 0.824f));
      q.AddQuadratic(Vector(0.068f, 0.845f), Vector(0.068f, 0.877f));
      q.AddQuadratic(Vector(0.068f, 0.909f), Vector(0.046f, 0.93f));
      q.AddQuadratic(Vector(0.024f, 0.952f), Vector(-0.007f, 0.952f));
      q.AddQuadratic(Vector(-0.038f, 0.952f), Vector(-0.06f, 0.93f));
      q.AddQuadratic(Vector(-0.082f, 0.909f), Vector(-0.082f, 0.877f));
      q.AddQuadratic(Vector(-0.082f, 0.845f), Vector(-0.06f, 0.824f));
      q.AddQuadratic(Vector(-0.038f, 0.802f), Vector(-0.007f, 0.802f));
      q.Add(Instruction());
      p.Append(q, a * (Affine::Translate(Vector(0.f, 0.5f)) *
        Affine::Scale(Vector(1.f, -1.f))));
    }

    static void AddSelection(Path& p, const Box& Outer,
      const Box& Inner, number CornerRadius)
    {
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
        if(Limits<number>::IsNotEqual(p1.x, p3.x) &&
           Limits<number>::IsNotEqual(p5.x, p7.x) &&
           Limits<number>::IsNotEqual(p1.y, p3.y) &&
           Limits<number>::IsNotEqual(p5.y, p7.y))
        {
          AddSelection(p, Box(p1, p3),
            Box(p1, p3), CornerRadius);
          AddSelection(p, Box(p5, p7),
            Box(p5, p7), CornerRadius);
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

      Vector r(CornerRadius * 2.f, CornerRadius * 2.f);

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
    static void AddBox(Path& p, const Box& r,
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
    static void AddEllipse(Path& p, Vector Origin,
      Vector Scale, number Rotation,
      bool CounterClockwise = true)
    {
      Bezier b1;
      Bezier b2;
      Bezier b3;
      Bezier b4;

      b1.Ellipse(Origin, Scale, Rotation, 1, CounterClockwise);
      b2.Ellipse(Origin, Scale, Rotation, 2, CounterClockwise);
      b3.Ellipse(Origin, Scale, Rotation, 3, CounterClockwise);
      b4.Ellipse(Origin, Scale, Rotation, 4, CounterClockwise);

      Vector v1, v2, v3, v4;

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
    static void AddCircle(Path& p, Vector Origin,
      number Diameter, bool CounterClockwise = true)
    {
      AddEllipse(p, Origin, Vector(Diameter, Diameter), 0,
        CounterClockwise);
    }

    ///Adds a series of dashed lines.
    static void AddDashedLine(Path& p, Vector Start, Vector End,
      number Thickness, number CapRelativeHeight, number DashWidth,
      number MinimumGapWidth, bool StartWithGap, bool EndWithGap)
    {
      number Length = (End - Start).Mag(), RemainingLength = Length;
      number IdealSpacing = DashWidth + MinimumGapWidth;
      count DashCount = 0, GapCount = 0;

      if(StartWithGap)
        GapCount++, RemainingLength -= MinimumGapWidth;
      if(not EndWithGap)
        GapCount--, RemainingLength += MinimumGapWidth;

      while(RemainingLength >= IdealSpacing)
        DashCount++, GapCount++, RemainingLength -= IdealSpacing;

      if(not GapCount)
      {
        AddLine(p, Start, End, Thickness, true, true, true, CapRelativeHeight);
        return;
      }

      number ActualGapWidth = (Length - number(DashCount) * DashWidth) /
        number(GapCount);

      number ActualSpacing = DashWidth + ActualGapWidth;
      number Offset = StartWithGap ? ActualGapWidth : number(0);

      for(count i = 0; i < DashCount; i++)
      {
        number a = Offset + ActualSpacing * number(i);
        number b = a + DashWidth;
        Vector DashStart = Start + (End - Start) * (a / Length);
        Vector DashEnd   = Start + (End - Start) * (b / Length);
        AddLine(p, DashStart, DashEnd, Thickness, true, true, true,
          CapRelativeHeight);
      }
    }

    /**Adds the outline of a non-zero thickness line to a Path. Additionally
    you can specify whether either the start or end caps are round (elliptical),
    and specify how elliptical the caps are.*/
    static void AddLine(Path& p, Vector Start,
      Vector End, number Thickness,
      bool CounterClockwise = true, bool StartRoundCap = true,
      bool EndRoundCap = true, number CapRelativeHeight = 1.0f)
    {
      if(Thickness <= 0.f)
        return;

      Vector Delta = End - Start;
      number Angle = Delta.Ang();

      Vector t;

      t.Polar(Angle - HalfPi<number>(), Thickness / 2.f);

      Vector p0 = Start + t;
      Vector p1 = End + t;
      Vector p2 = End - t;
      Vector p3 = Start - t;

      Bezier Start1;
      Bezier Start2;
      Bezier End1;
      Bezier End2;

      Vector Scale(Thickness, Thickness * CapRelativeHeight);

      End1.Ellipse(End, Scale, Angle - HalfPi<number>(), 1, CounterClockwise);
      End2.Ellipse(End, Scale, Angle - HalfPi<number>(), 2, CounterClockwise);
      Start1.Ellipse(Start, Scale, Angle - HalfPi<number>(), 3,
        CounterClockwise);
      Start2.Ellipse(Start, Scale, Angle - HalfPi<number>(), 4,
        CounterClockwise);

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
    static void AddBoxFromLines(Path& p, const Box& r,
      number Thickness)
    {
      AddLine(p, r.TopRight(), r.BottomRight(), Thickness);
      AddLine(p, r.BottomRight(), r.BottomLeft(), Thickness);
      AddLine(p, r.BottomLeft(), r.TopLeft(), Thickness);
      AddLine(p, r.TopLeft(), r.TopRight(), Thickness);
    }

    ///Adds rectangular subpaths from outlined lines.
    static void AddXFromLines(Path& p, const Box& r,
      number Thickness)
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
      const number Extent = 5.f;
      const number TotalExtent = Extent + 1.f;
      const number Thickness = 0.02f;
      const number TickWidth = 0.1f;
      Shapes::AddLine(p, Vector(-TotalExtent, 0),
        Vector(TotalExtent, 0), Thickness);
      Shapes::AddLine(p, Vector(0, -TotalExtent),
        Vector(0, TotalExtent), Thickness);

      for(count i = count(-Extent); i <= count(Extent); i++)
      {
        number iNumber = number(i);
        if(Limits<number>::IsZero(iNumber)) continue;
        Shapes::AddLine(p, Vector(-TickWidth, iNumber),
          Vector(TickWidth, iNumber), Thickness);
        Shapes::AddLine(p, Vector(iNumber, -TickWidth),
          Vector(iNumber, TickWidth), Thickness);
      }
    }

    ///Creates a grid.
    static void AddGrid(Path& p, Vector Size, VectorInt Cells, number
      RelativeLineThickness = 0.01f)
    {
      if(Cells.i() < 1 || Cells.j() < 1) return;

      number Thickness = number(Size.x) / Cells.x *
        RelativeLineThickness;

      for(count i = 0; i <= Cells.i(); i++)
      {
        number x = number(i) / number(Cells.i()) * Size.x;
        AddLine(p, Vector(x, 0), Vector(x, Size.y), Thickness);
      }
      for(count j = 0; j <= Cells.j(); j++)
      {
        number y = number(j) / number(Cells.j()) * Size.y;
        AddLine(p, Vector(0, y), Vector(Size.x, y), Thickness);
      }
    }

    ///Shows an axis at the origin with unit square to show the current space.
    static void ShowAxis(belle::Painter& Painter,
      belle::Color ShapeColor = belle::Colors::LightBlue(),
      belle::Color AxisColor = belle::Colors::Black())
    {
      /*To demonstrate affine transformations it helps to show a coordinate
      axis. The Shapes class has a built-in path-maker for an axis with ticks.*/
      belle::Path Axis, Shape;
      belle::Shapes::AddCoordinateAxis(Axis);

      /*Show a simple rectangle using filled (not stroked) lines. This allows
      us to use a fill operation instead of a stroke operation and it also
      provides the outline with rounded corners.*/
      belle::Shapes::AddBoxFromLines(Shape,
        Box(Vector(0, 0), Vector(1, 1)), 0.05f);

      //Draw the axis using the color for the axis.
      Painter.SetFill(AxisColor);
      Painter.Draw(Axis);

      //Draw the shape on top of the axis using the color for the shape.
      Painter.SetFill(ShapeColor);
      Painter.Draw(Shape);

      //Reset the fill color.
      Painter.SetFill(Colors::Black());
    }

    struct Music
    {
      ///Adds subpaths to create a whole note.
      static void AddWholeNote(Path& p, Vector HeadOrigin,
        number Height = 1.f, number RelativeWidth = 1.7f,
        number HollowScale = 0.55f,
        number HollowAngle = 0.75f * Pi<number>())
      {
        Vector HeadScale(Height * RelativeWidth, Height);
        Vector HollowVectorScale = HeadScale;
        HollowVectorScale *= HollowScale;

        AddEllipse(p, HeadOrigin, HeadScale, 0, true);
        AddEllipse(p, HeadOrigin, HollowVectorScale, HollowAngle, false);
      }

      ///Adds subpaths to create a quarter note.
      static Vector AddQuarterNote
      (
        Path&           p,
        Vector          HeadOrigin,
        number          Height                = 1.f,
        bool            MakeSingleOutline     = true,
        number          RelativeStemHeight    = 4.f,
        Vector*         FlagPosition          = 0,
        number          HeadTheta             = 20.f * Deg<number>(),
        number          RelativeWidth         = 1.4f,
        number          RelativeStemThickness = 0.1f,
        number          RelativeStemCapHeight = 0.8f,
        bool            IsHollow              = false,
        number          HollowScale           = 0.48f,
        number          HollowTheta           = 0.4f * HalfPi<number>()
      )
      {
        Vector StemLocation;
        Vector HeadScale(Height * RelativeWidth, Height);
        number StemThickness = RelativeStemThickness;
        number StemHeight = RelativeStemHeight;

        //Create the hollow area if necessary.
        if(IsHollow)
        {
          Vector HollowVectorScale = HeadScale * HollowScale;

          Vector Start;
          Start.Polar(HollowTheta + Pi<number>(), HollowVectorScale.x / 2.f);
          Start += HeadOrigin;

          Vector End;
          End.Polar(HollowTheta, HollowVectorScale.x / 2.f);
          End += HeadOrigin;

          AddLine(p, Start, End, HollowVectorScale.y, false, true, true, 1.f);
        }

        if(!MakeSingleOutline || Abs(StemHeight) < HeadScale.y / 2.f)
        {
          /*Algorithm 1 - Draw an ellipse and overlay the stem as two different
          subpaths. Only draw a line if the height is non-zero, and draw on the
          right if it is positive, and on the left if it is negative.*/

          AddEllipse(p, HeadOrigin, HeadScale, HeadTheta, true);

          if(StemHeight > HeadScale.y / 2.f)
          {
            Vector Start = Ellipse::VerticalTangent(
              HeadScale.x / 2.f, HeadScale.y / 2.f, HeadTheta);

            StemHeight -= Start.y;

            Start.x = HeadOrigin.x + Start.x;
            Start.y = HeadOrigin.y + Start.y;

            StemLocation = Start;

            Start.x -= StemThickness / 2.f;
            Vector End = Start;
            End.y += StemHeight;
            //AddLine(p, Start, End, StemThickness, true, false, true,
            //  RelativeStemCapHeight);

            if(FlagPosition)
              *FlagPosition = Vector(End.x - StemThickness / 2.f, End.y);
          }
          else if(StemHeight < -HeadScale.y / 2.f)
          {
            Vector Start = Ellipse::VerticalTangent(
              HeadScale.x / 2.f, HeadScale.y / 2.f, HeadTheta);

            StemHeight += Start.y;

            Start.x = HeadOrigin.x - Start.x;
            Start.y = HeadOrigin.y - Start.y;

            StemLocation = Start;

            Start.x += StemThickness / 2.f;
            Vector End = Start;
            End.y += StemHeight;
            //AddLine(p, Start, End, StemThickness, true, false, true,
            //  RelativeStemCapHeight);
            if(FlagPosition)
              *FlagPosition = Vector(End.x + StemThickness / 2.f, End.y);
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
            HeadScale.x / 2.f, HeadScale.y / 2.f, HeadTheta, StemThickness);

          //Quadrant 4 intersection
          number t4;
          number Dummy;
          if(!c4.VerticalTangents(t4, Dummy))
            return StemLocation;

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
          if(StemHeight > 0.f)
          {
            p1.Ellipse(HeadOrigin, HeadScale, HeadTheta, 1, true);
            p1.Trim(t1, 1.f); //Trim the curve.
            p2.Ellipse(HeadOrigin, HeadScale, HeadTheta, 2, true);
            p3.Ellipse(HeadOrigin, HeadScale, HeadTheta, 3, true);
            p4.Ellipse(HeadOrigin, HeadScale, HeadTheta, 4, true);
            p4.Trim(0.f,t4); //Trim the curve.

            l5_Start = p4.Value(1.f);
            l5_End = l5_Start;
            l5_End.y = HeadOrigin.y + StemHeight;

            Vector p6origin(l5_End.x - StemThickness / 2.f, l5_End.y);
            Vector p6scale(StemThickness,
              StemThickness * RelativeStemCapHeight);

            p6.Ellipse(p6origin, p6scale, 0.f, 1, true);
            p7.Ellipse(p6origin, p6scale, 0.f, 2, true);

            l8_Start = l5_End;
            l8_Start.x -= StemThickness;
            l8_End = p1.Value(0);

            if(FlagPosition)
              *FlagPosition = l8_Start;
          }
          else
          {
            p1.Ellipse(HeadOrigin, HeadScale, HeadTheta + Pi<number>(), 1,
              true);
            p1.Trim(t1, 1.f); //Trim the curve.
            p2.Ellipse(HeadOrigin, HeadScale, HeadTheta + Pi<number>(), 2,
              true);
            p3.Ellipse(HeadOrigin, HeadScale, HeadTheta + Pi<number>(), 3,
              true);
            p4.Ellipse(HeadOrigin, HeadScale, HeadTheta + Pi<number>(), 4,
              true);
            p4.Trim(0.f, t4); //Trim the curve.

            l5_Start = p4.Value(1.f);
            l5_End = l5_Start;
            l5_End.y = HeadOrigin.y + StemHeight;

            Vector p6origin(l5_End.x + StemThickness / 2.f, l5_End.y);
            Vector p6scale(StemThickness,
              StemThickness * RelativeStemCapHeight);

            p6.Ellipse(p6origin, p6scale, 0.f, 3, true);
            p7.Ellipse(p6origin, p6scale, 0.f, 4, true);

            l8_Start = l5_End;
            l8_Start.x += StemThickness;
            l8_End = p1.Value(0.f);

            if(FlagPosition)
              *FlagPosition = l5_End;
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

        return StemLocation;
      }

      ///Adds subpaths to create a half note.
      static Vector AddHalfNote
      (
        Path&           p,
        Vector          HeadOrigin,
        number          Height                = 1.f,
        bool            MakeSingleOutline     = true,
        number          RelativeStemHeight    = 4.f,
        Vector*         FlagPosition          = 0,
        number          HeadTheta             = 20.f * Deg<number>(),
        number          RelativeWidth         = 1.4f,
        number          RelativeStemThickness = 0.1f,
        number          RelativeStemCapHeight = 0.8f,
        number          HollowScale           = 0.48f,
        number          HollowTheta           = 0.4f * HalfPi<number>()
      )
      {
        return AddQuarterNote(p, HeadOrigin, Height, MakeSingleOutline,
          RelativeStemHeight, FlagPosition, HeadTheta, RelativeWidth,
          RelativeStemThickness, RelativeStemCapHeight, true, HollowScale,
          HollowTheta);
      }

      ///Adds a convex slur to a path.
      static void AddSlur(Path& p, Vector a,
        Vector b, number inSpaceHeight,
        number relativeArchHeight = 0.1f,
        number relativeArchWidth = 0.5f,
        number shMaxThickness = 0.2f,
        number shMinThickness = 0.06f,
        number relativeCapHeight = 3.f)
      {
        number theta = a.Ang(b);
        number perpen = theta + HalfPi<number>();
        number width = a.Mag(b);

        number shArchHeight = relativeArchHeight *
          (Power(width, number(0.8f)) / inSpaceHeight);

        shArchHeight *= inSpaceHeight;
        shMaxThickness *= inSpaceHeight;
        shMinThickness *= inSpaceHeight;

        Vector ghost_a1 = a;
        Vector perpenAngle;
        Vector thetaAngle;
        perpenAngle.Polar(perpen);
        thetaAngle.Polar(theta);
        ghost_a1 += perpenAngle * shArchHeight;

        Vector c = ghost_a1;
        c += thetaAngle * width * (0.5f - relativeArchWidth * 0.5f);

        Vector d = ghost_a1;
        d += thetaAngle * width * (0.5f + relativeArchWidth * 0.5f);

        Vector ghost_a2 = a;
        ghost_a2 += perpenAngle * (shArchHeight + shMaxThickness);

        Vector e = ghost_a2;
        e += thetaAngle * width * (0.5f - relativeArchWidth * 0.5f);

        Vector f = ghost_a2;
        f += thetaAngle * width * (0.5f + relativeArchWidth * 0.5f);

        Vector g = a;
        Vector ac, bd;
        ac.Polar(a.Ang(c) + HalfPi<number>());
        bd.Polar(b.Ang(d) - HalfPi<number>());
        g += ac * shMinThickness;

        Vector h = b;
        h += bd * shMinThickness;

        Vector i = (a + g) * 0.5f;
        Vector j = (b + h) * 0.5f;

        Bezier ellipse_bj;
        Bezier ellipse_jh;
        Bezier ellipse_gi;
        Bezier ellipse_ia;

        Vector ellipse_scale(shMinThickness,
          shMinThickness * relativeCapHeight);
        ellipse_bj.Ellipse(j, ellipse_scale, d.Ang(b) - HalfPi<number>(), 1,
          true);
        ellipse_jh.Ellipse(j, ellipse_scale, d.Ang(b) - HalfPi<number>(), 2,
          true);
        ellipse_gi.Ellipse(i, ellipse_scale, a.Ang(c) + HalfPi<number>(), 1,
          true);
        ellipse_ia.Ellipse(i, ellipse_scale, a.Ang(c) + HalfPi<number>(), 2,
          true);

        Vector bj[4];
        Vector jh[4];
        Vector gi[4];
        Vector ia[4];

        ellipse_bj.GetControlPoints(bj[0], bj[1], bj[2], bj[3]);
        ellipse_jh.GetControlPoints(jh[0], jh[1], jh[2], jh[3]);
        ellipse_gi.GetControlPoints(gi[0], gi[1], gi[2], gi[3]);
        ellipse_ia.GetControlPoints(ia[0], ia[1], ia[2], ia[3]);

        p.Add(Instruction(a, true));
        p.Add(Instruction(c, d, b));
        p.Add(Instruction(bj[1], bj[2], bj[3]));
        p.Add(Instruction(jh[1], jh[2], jh[3]));
        p.Add(Instruction(f, e, g));
        p.Add(Instruction(gi[1], gi[2], gi[3]));
        p.Add(Instruction(ia[1], ia[2], ia[3]));
      }

      ///Adds a grand staff brace to a path.
      static void AddBrace(Path& p,
        Vector Center,
        number Height,
        number RelativeWidth = number(0.1))
      {
        /*We're using a set of control points from a quadratic Bezier
        curve taken from a font program, due to the complexity of the
        shape.*/

        /*The control points here were derived from the Bravura path data for
        glyph 119060 (Brace).*/
        Vector v[67];
        {
          count pt = 0;
          v[pt++] = Vector(0.02f, 0.498f);
          v[pt++] = Vector(0.049f, 0.516f);
          v[pt++] = Vector(0.082f, 0.587f);
          v[pt++] = Vector(0.082f, 0.646f);
          v[pt++] = Vector(0.082f, 0.651f);
          v[pt++] = Vector(0.082f, 0.657f);
          v[pt++] = Vector(0.081f, 0.662f);
          v[pt++] = Vector(0.074f, 0.722f);
          v[pt++] = Vector(0.044f, 0.815f);
          v[pt++] = Vector(0.044f, 0.869f);
          v[pt++] = Vector(0.044f, 0.921f);
          v[pt++] = Vector(0.067f, 0.971f);
          v[pt++] = Vector(0.072f, 0.98f);
          v[pt++] = Vector(0.076f, 0.984f);
          v[pt++] = Vector(0.077f, 0.987f);
          v[pt++] = Vector(0.077f, 0.991f);
          v[pt++] = Vector(0.077f, 0.993f);
          v[pt++] = Vector(0.076f, 0.994f);
          v[pt++] = Vector(0.075f, 0.995f);
          v[pt++] = Vector(0.074f, 0.996f);
          v[pt++] = Vector(0.073f, 0.997f);
          v[pt++] = Vector(0.071f, 0.997f);
          v[pt++] = Vector(0.069f, 0.997f);
          v[pt++] = Vector(0.067f, 0.995f);
          v[pt++] = Vector(0.063f, 0.99f);
          v[pt++] = Vector(0.041f, 0.963f);
          v[pt++] = Vector(0.014f, 0.905f);
          v[pt++] = Vector(0.014f, 0.805f);
          v[pt++] = Vector(0.014f, 0.706f);
          v[pt++] = Vector(0.049f, 0.666f);
          v[pt++] = Vector(0.049f, 0.603f);
          v[pt++] = Vector(0.049f, 0.556f);
          v[pt++] = Vector(0.03f, 0.53f);
          v[pt++] = Vector(0.002f, 0.498f);
          v[pt++] = Vector(0.02f, 0.478f);
          v[pt++] = Vector(0.049f, 0.462f);
          v[pt++] = Vector(0.049f, 0.397f);
          v[pt++] = Vector(0.049f, 0.327f);
          v[pt++] = Vector(0.014f, 0.265f);
          v[pt++] = Vector(0.014f, 0.192f);
          v[pt++] = Vector(0.014f, 0.092f);
          v[pt++] = Vector(0.041f, 0.034f);
          v[pt++] = Vector(0.063f, 0.006f);
          v[pt++] = Vector(0.067f, 0.001f);
          v[pt++] = Vector(0.069f, 0.0f);
          v[pt++] = Vector(0.071f, 0.0f);
          v[pt++] = Vector(0.073f, 0.0f);
          v[pt++] = Vector(0.074f, 0.001f);
          v[pt++] = Vector(0.075f, 0.002f);
          v[pt++] = Vector(0.076f, 0.003f);
          v[pt++] = Vector(0.077f, 0.004f);
          v[pt++] = Vector(0.077f, 0.006f);
          v[pt++] = Vector(0.077f, 0.009f);
          v[pt++] = Vector(0.076f, 0.012f);
          v[pt++] = Vector(0.072f, 0.017f);
          v[pt++] = Vector(0.067f, 0.025f);
          v[pt++] = Vector(0.044f, 0.075f);
          v[pt++] = Vector(0.044f, 0.128f);
          v[pt++] = Vector(0.044f, 0.181f);
          v[pt++] = Vector(0.074f, 0.275f);
          v[pt++] = Vector(0.081f, 0.334f);
          v[pt++] = Vector(0.082f, 0.339f);
          v[pt++] = Vector(0.082f, 0.344f);
          v[pt++] = Vector(0.082f, 0.35f);
          v[pt++] = Vector(0.082f, 0.409f);
          v[pt++] = Vector(0.049f, 0.48f);
          v[pt] = Vector(0.02f, 0.498f);
        }

        //Transform into context space.
        for(count i = 0; i < 67; i++)
        {
          number mult  = Height;
          number multx = RelativeWidth / number(0.1);
          v[i].y -= 0.5f;
          v[i] *= mult;
          v[i].x *= multx;
          v[i] += Center;
        }

        //Create the curve.
        p.Add(Instruction(v[0], true));
        for(count i = 1; i <= 64; i += 3)
          p.Add(Instruction(v[i], v[i + 1], v[i + 2]));
        //p.Add(Instruction());
      }
    };
  };
}
#endif
