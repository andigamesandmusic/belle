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

#ifndef BELLE_CORE_PATH_H
#define BELLE_CORE_PATH_H

#include "belle-transform.h"

namespace BELLE_NAMESPACE
{
  class Instruction
  {
    friend class Typeface;

    ///The instruction type (move-to, line-to, cubic-to, close path)
    count Type;

    ///Data points for curves.
    Vector Point0, Point1, Point2;

    //Enumeration of construction types
    enum ConstructionType
    {
      MoveTo = 1,
      LineTo,
      CubicTo,
      ClosePath
    };

    //Returns whether the number is reasonable as a coordinate value.
    bool NumberIsReasonable(number x)
    {
      static const number ReasonableRange = 1000000.0;
      return x > -ReasonableRange and x < ReasonableRange;
    }

    //Returns whether the vector is reasonable as a coordinate.
    bool VectorIsReasonable(const Vector& v)
    {
      return NumberIsReasonable(v.x) and NumberIsReasonable(v.y);
    }

    public:

    ///First control point of a cubic.
    inline Vector Control1() const {return Point0;}

    ///Second control point of a cubic.
    inline Vector Control2() const {return Point1;}

    ///Final point of a line, cubic or move.
    inline Vector End() const {return Point2;}

    ///First control point of a cubic.
    inline Vector& Control1() {return Point0;}

    ///Second control point of a cubic.
    inline Vector& Control2() {return Point1;}

    ///Final point of a line, cubic or move.
    inline Vector& End() {return Point2;}

    ///Returns whether or not this instruction is a MoveTo.
    inline bool IsMove() const {return Type == MoveTo;}

    ///Returns whether or not this instruction is a LineTo.
    inline bool IsLine() const {return Type == LineTo;}

    ///Returns whether or not this instruction is a CubicTo.
    inline bool IsCubic() const {return Type == CubicTo;}

    ///Returns whether or not this instruction is a ClosePath.
    inline bool IsClosing() const {return Type == ClosePath;}

    ///Returns whether or not this instruction has an endpoint.
    inline bool HasEnd() const {return Type != ClosePath;}

    ///Returns whether or not this instruction has control points.
    inline bool HasControls() const {return Type == CubicTo;}

    ///Checks the instruction to see that it has reasonable coordinate values.
    void Check()
    {
      bool IsReasonable = true;
      if(HasEnd())
        IsReasonable = VectorIsReasonable(End());
      if(HasControls())
        IsReasonable = IsReasonable and VectorIsReasonable(Control1()) and
          VectorIsReasonable(Control2());
      if(not IsReasonable)
      {
        C::Error() >> "Warning: detected dubious path instruction:";
        if(HasControls())
        {
          C::Error() >> "Control 1: " << Control1();
          C::Error() >> "Control 2: " << Control2();
        }
        C::Error() >> "End:       " << End();
      }
    }

    ///Default constructor creates a close path instruction.
    Instruction() : Type(ClosePath) {}

    /**Constructor for MoveTo or LineTo. If StartNewPath is true, then MoveTo
    is used. If not, then LineTo is used.*/
    Instruction(Vector p1, bool StartNewPath = false)
    {
      Type = (StartNewPath ? MoveTo : LineTo);
      End() = p1;
      Check();
    }

    ///Constructor for CubicTo.
    Instruction(Vector ControlPoint1,
      Vector ControlPoint2, Vector EndPoint) :
      Type(CubicTo)
    {
      Control1() = ControlPoint1;
      Control2() = ControlPoint2;
      End() = EndPoint;
      Check();
    }

    ///Constructor for CubicTo via Bezier.
    Instruction(Bezier& BezierCurve) : Type(CubicTo)
    {
      Vector Start;
      BezierCurve.GetControlPoints(Start, Control1(), Control2(), End());
      Check();
    }

    ///Constructs a transformed copy of an instruction.
    Instruction(const Instruction& i, Affine a)
    {
      Type = i.Type;
      Point0 = a << i.Point0;
      Point1 = a << i.Point1;
      Point2 = a << i.Point2;
      Check();
    }

    ///Convert the instruction to a string.
    operator String() const
    {
      String s;
      if(IsMove())
        s << "M " << End();
      else if(IsLine())
        s << "L " << End();
      else if(IsCubic())
        s << "C " << Control1() << " " << Control2() << " " << End();
      else if(IsClosing())
        s << "Z";
      return s;
    }
  };

  class Path : public Value::Base
  {
    Array<Instruction> Instructions;
    Array<PolygonShape> CachedOutline;
    Box BoundingBox;

#if defined(JUCE_VERSION)
    juce::Path JUCEPath;
    friend struct JUCE;
#endif
    virtual void VtableEmit();

    public:

    ///Default constructor
    Path() {}

    ///Constructs a transformed copy of a path.
    Path(const Path& p, Affine a)
    {
      Append(p, a);
    }

    ///Starts a new path.
    void StartPath(Vector p)
    {
      Add(Instruction(p, true));
    }

    ///Appends a line to the path.
    void AddLine(Vector p)
    {
      Add(Instruction(p));
    }

    /**Appends a quadratic curve to the path. Note, that this method actually
    appends an identical cubic curve since the path instructions do not store
    quadratic arcs.*/
    void AddQuadratic(Vector q1, Vector q2)
    {
      Bezier b;
      b.SetControlPoints(Instructions.z().End(), q1, q2);
      Vector p0, p1, p2, p3;
      b.GetControlPoints(p0, p1, p2, p3);
      Add(Instruction(p1, p2, p3));
    }

    ///Appends a cubic curve to the path.
    void AddCubic(Vector p0, Vector p1, Vector p2)
    {
      Add(Instruction(p0, p1, p2));
    }

    ///Returns the array of instructions in this path.
    operator String() const
    {
      return Instructions;
    }

    ///Appends a transformed copy of a path.
    void Append(const Path& p, Affine a = Affine::Unit())
    {
      for(count i = 0; i < p.Instructions.n(); i++)
        Add(Instruction(p.Instructions[i], a));
    }

    ///Appends a transformed copy of a polygon.
    void Append(const PolygonShape& p, Affine a = Affine::Unit())
    {
      //Must have at least 3 vertices to be valid.
      if(p.n() < 3) return;

      //Create a MoveTo followed by (n - 1) LineTo's.
      for(count i = 0; i < p.n(); i++)
        Add(Instruction(a << p[i], !i));

      //Close the polygon in the subpath.
      Add(Instruction());
    }

    ///Appends a transformed copy of a polygon array.
    void Append(const Array<PolygonShape>& p,
      Affine a = Affine::Unit())
    {
      for(count j = 0; j < p.n(); j++)
      {
        //Must have at least 3 vertices to be valid.
        if(p[j].n() < 3) continue;

        //Create a MoveTo followed by (n - 1) LineTo's.
        for(count i = 0; i < p[j].n(); i++)
          Add(Instruction(a << p[j][i], !i));

        //Close the polygon in the subpath.
        Add(Instruction());
      }
    }

    ///Appends a transformed convex hull of a path.
    void AppendConvex(const Path& p, Affine a = Affine::Unit())
    {
      PolygonShape EntireHull;
      for(count i = 0; i < p.Outline().n(); i++)
      {
        PolygonShape Hull = p.Outline()[i];
        for(count j = 0; j < Hull.n(); j++)
          EntireHull.Add() = Hull[j];
      }
      PolygonShape ConvexHull;
      EntireHull.CreateConvexHull(ConvexHull);
      Append(ConvexHull, a);
    }

    ///Ensures all subpaths finish with a close path instruction.
    void CloseAllSubpaths()
    {
      Path NewPath;
      bool PreviousWasClose = true;
      for(count i = 0; i < Instructions.n(); i++)
      {
        if(Instructions[i].IsMove() && !PreviousWasClose)
          NewPath.Add(Instruction());
        NewPath.Add(Instructions[i]);
        PreviousWasClose = Instructions[i].IsClosing();
      }
      if(!PreviousWasClose)
        NewPath.Add(Instruction());
      *this = NewPath;
    }

    /**Reverses the clockwise order of the points in the path. This will not
    affect how the path renders in isolation, but can affect how it unions and
    intersects with other sibling paths.*/
    void Reverse()
    {
      /*Close subpaths first so that the reverse can expect move-tos to be
      balanced by close-paths.*/
      CloseAllSubpaths();

      /*Create a vector of the maximum possible size (if all instructions were
      cubic curves). This is actually greater than the maximum practical size,
      but great enough in case the instructions array is filled incorrectly.*/
      Array<Vector> Points(Instructions.n() * 3);

      //Get a reversed point array.
      for(count i = Instructions.n() - 1, p = 0; i >= 0; i--)
      {
        //Store the point data.
        Instruction d = Instructions[i];
        if(d.IsMove() || d.IsLine())
          Points[p++] = d.End();
        else if(d.IsCubic())
        {
          Points[p++] = d.End();
          Points[p++] = d.Control2();
          Points[p++] = d.Control1();
        }
      }

      /*Create a new path from the reversed point array following the original
      instructions.*/
      Path NewPath;
      for(count i = Instructions.n() - 1, p = 0; i >= 0; i--)
      {
        Instruction d = Instructions[i];
        if(d.IsClosing())
        {
          //Close-path becomes a move-to
          NewPath.Add(Instruction(Points[p++], true));
        }
        else if(d.IsMove())
        {
          //Move-to becomes close-path.
          NewPath.Add(Instruction());
        }
        else if(d.IsLine())
        {
          //Add a line.
          NewPath.Add(Instruction(Points[p++]));
        }
        else if(d.IsCubic())
        {
          //Add a cubic.
          NewPath.Add(Instruction(Points[p], Points[p + 1], Points[p + 2]));
          p += 3;
        }
      }

      //Assign new path to the current one.
      *this = NewPath;
    }

    ///Adds instruction and updates bounding box, polygon, and native base path.
    void Add(const Instruction& i)
    {
      Vector e = i.End();
      Vector c1 = i.Control1();
      Vector c2 = i.Control2();

      Instructions.Add() = i;

      if(i.IsMove())
      {
        CachedOutline.Add().Add() = e;
        BoundingBox = BoundingBox + e;
      }
      else if(i.IsLine())
      {
        CachedOutline.z().Add() = e;
        BoundingBox = BoundingBox + e;
      }
      else if(i.HasControls())
      {
        CachedOutline.z().Add() = c1;
        CachedOutline.z().Add() = c2;
        CachedOutline.z().Add() = e;
        BoundingBox = BoundingBox + c1 + c2 + e;
      }

      //Do work for native path base classes.
#if defined(JUCE_VERSION)
      if(i.IsMove())
        JUCEPath.startNewSubPath((float)e.x, (float)e.y);
      else if(i.IsLine())
        JUCEPath.lineTo((float)e.x, (float)e.y);
      else if(i.IsCubic())
        JUCEPath.cubicTo((float)c1.x, (float)c1.y, (float)c2.x, (float)c2.y,
          (float)e.x, (float)e.y);
      else if(i.IsClosing())
        JUCEPath.closeSubPath();
#endif
    }

    ///Scales the path in place.
    void Transform(Affine Transformation)
    {
      for(count i = 0; i < Instructions.n(); i++)
        Instructions[i] = Instruction(Instructions[i], Transformation);
      for(count i = 0; i < CachedOutline.n(); i++)
        for(count j = 0; j < CachedOutline[i].n(); j++)
          CachedOutline[i][j] = Transformation << CachedOutline[i][j];
    }

    ///Retrieves the i-th path construction.
    inline const Instruction& operator [] (count i) const
    {
      return Instructions[i];
    }

    ///Retrieves the i-th path construction.
    inline const Instruction& ith(count i) const
    {
      return Instructions[i];
    }

    ///Retrieves the number of path constructions.
    inline count n() const
    {
      return Instructions.n();
    }

    ///Retrieves the first path construction.
    inline const Instruction& a() const
    {
      return Instructions.a();
    }

    ///Retrieves the last path construction.
    inline const Instruction& z(count ItemsFromEnd = 0) const
    {
      return Instructions.z(ItemsFromEnd);
    }

    ///Retrieves the current end point.
    Vector End()
    {
      if(n())
        return z().End();
      else
        return Vector();
    }

    ///Retrieves the cached polygon outline of this path.
    inline const Array<PolygonShape>& Outline() const
    {
      return CachedOutline;
    }

    ///Retrieves the cached bounding box for this path.
    inline Box Bounds() const
    {
      return BoundingBox;
    }

    ///Retrieves the bounding box of the transformed path box.
    Box Bounds(const Affine& Transformation) const
    {
      return Transformation << BoundingBox;
    }

    ///Gets an array of bounds for each path segment.
    Array<Box> BoundsOfCurves(const Affine& Transformation =
      Affine::Unit()) const
    {
      Array<Box> R;
      Vector MovePoint;
      Vector Anchor;

      for(count i = 0; i < n(); i++)
      {
        Instruction in = ith(i);
        if(in.IsMove())
          Anchor = MovePoint = in.End();
        else if(in.IsLine())
        {
          R.Add() = Box(Anchor, in.End());
          Anchor = in.End();
        }
        else if(in.IsClosing())
          R.Add() = Box(Anchor, MovePoint);
        else if(in.IsCubic())
        {
          R.Add() = Box(Anchor, in.Control1()) +
            Box(in.Control2(), in.End());
          Anchor = in.End();
        }
      }

      for(count i = 0; i < R.n(); i++)
      {
        Box r = R[i];
        R[i] = Box(Transformation << r.a, Transformation << r.b);
        R[i].Order();
      }

      return R;
    }

    ///Retrieves the rectangular polygon box of the transformed path's box.
    PolygonShape BoundsPolygonShape(const Affine& Transformation) const
    {
      PolygonShape p;
      p.n(4);
      p[0] = Transformation << BoundingBox.BottomLeft();
      p[1] = Transformation << BoundingBox.TopLeft();
      p[2] = Transformation << BoundingBox.TopRight();
      p[3] = Transformation << BoundingBox.BottomRight();
      return p;
    }

    /**Fits a polygon of points to a path using the Catmull-Rom algorithm.
    The alpha value must be between 0 and 1 inclusively.*/
    static Path FitPointsWithCatmullRomSpline(
      const PolygonShape& Points, bool Closed, number Alpha)
    {
      Path p;
      if(not (Alpha >= 0.f and Alpha <= 1.f) or Points.n() < 2);
      else if(Points.n() == 2)
          p.Add(Instruction(Points.a(), true)),
          p.Add(Instruction(Points.z()));
      else if(Points.n() == 3)
          p.Add(Instruction(Points[0], true)),
          p.Add(Instruction(Points[1], Points[1], Points[2]));
      else
      {
        count StartIndex = Closed ? 0 : 1;
        count EndIndex = Closed ? Points.n() : Points.n() - 2;
        for(count i = StartIndex; i < EndIndex; i++)
        {
          Vector p0, p1, p2, p3;
          count Next_i      = (i + 1) % Points.n();
          count NextNext_i  = (Next_i + 1) % Points.n();
          count Prev_i      = i - 1 < 0 ? Points.n() - 1 : i - 1;

          p0 = Points[Prev_i];
          p1 = Points[i];
          p2 = Points[Next_i];
          p3 = Points[NextNext_i];

          number d1 = (p1 - p0).Mag();
          number d2 = (p2 - p1).Mag();
          number d3 = (p3 - p2).Mag();

          Vector b1, b2;
          b1 = p2 * Power(d1, Alpha * 2.f);
          b1 = b1 - (p0 * Power(d2, Alpha * 2.f));
          b1 = (b1 + (p1 * (2.f * Power(d1, 2.f * Alpha) + 3 *
            Power(d1, Alpha) * Power(d2, Alpha) + Power(d2, 2.f * Alpha))));
          b1 = (b1 * 1.f / (3.f * Power(d1, Alpha) *
            (Power(d1, Alpha) + Power(d2, Alpha))));

          b2 = (p1 * Power(d3, 2.f * Alpha));
          b2 = (b2 - (p3 * Power(d2, 2.f * Alpha)));
          b2 = (b2 + (p2 * (2.f * Power(d3, 2.f * Alpha) + 3.f *
            Power(d3, Alpha) * Power(d2, Alpha) + Power(d2, 2.f * Alpha))));
          b2 = (b2 * 1.0 / (3.f * Power(d3, Alpha) *
            (Power(d3, Alpha) + Power(d2, Alpha))));

          if(i == StartIndex) p.Add(Instruction(p1, true));
          p.Add(Instruction(b1, b2, p2));
        }
        if(Closed) p.Add(Instruction());
      }
      return p;
    }

    ///Fits a polygon of points to a path using the Hermite spline algorithm.
    static Path FitPointsWithHermiteSpline(
      const PolygonShape& Points, bool Closed)
    {
      Path p;
      if(Points.n() >= 2)
      {
        count CurveCount = (Closed ? Points.n() : Points.n() - 1);
        for(count i = 0; i < CurveCount; i++)
        {
          Vector CurrentPoint, PreviousPoint, NextPoint, EndPoint;
          CurrentPoint = Points[i];
          if(not i) p.Add(Instruction(CurrentPoint, true));

          count Next_i = (i + 1) % Points.n();
          count Previous_i = (i - 1 + Points.n()) % Points.n();

          PreviousPoint = Points[Previous_i];
          NextPoint = Points[Next_i];
          EndPoint = NextPoint;

          number x, y;
          if(Closed or i > 0)
            x = (NextPoint.x - PreviousPoint.x) / 2.f,
            y = (NextPoint.y - PreviousPoint.y) / 2.f;
          else
            x = (NextPoint.x - CurrentPoint.x) / 2.f,
            y = (NextPoint.y - CurrentPoint.y) / 2.f;

          Vector ControlPoint1;
          ControlPoint1.x = CurrentPoint.x + x / 3.f,
          ControlPoint1.y = CurrentPoint.y + y / 3.f;

          CurrentPoint = Points[Next_i];

          Next_i = (Next_i + 1) % Points.n();
          Previous_i = i;

          PreviousPoint = Points[Previous_i];
          NextPoint = Points[Next_i];

          if(Closed or i < CurveCount - 1)
            x = (NextPoint.x - PreviousPoint.x) / 2.f,
            y = (NextPoint.y - PreviousPoint.y) / 2.f;
          else
            x = (CurrentPoint.x - PreviousPoint.x) / 2.f,
            y = (CurrentPoint.y - PreviousPoint.y) / 2.f;

          Vector ControlPoint2;
          ControlPoint2.x = CurrentPoint.x - x / 3.f,
          ControlPoint2.y = CurrentPoint.y - y / 3.f;
          p.Add(Instruction(ControlPoint1, ControlPoint2, EndPoint));
        }
        if(Closed)
          p.Add(Instruction());
      }
      return p;
    }

    ///Returns a polygon of points approximating the curve.
    PolygonShape ApproximateAsPolygonShape(count PointsPerCurve)
    {
      PolygonShape p;
      Vector p0, p1, p2, p3;
      for(count i = 0; i < n(); i++)
      {
        Instruction in = ith(i);
        if(in.IsMove() or in.IsLine())
        {
          p3 = in.End();
          p.Add() = p3;
        }
        else if(in.IsCubic())
        {
          p1 = in.Control1();
          p2 = in.Control2();
          p3 = in.End();
          Bezier b;
          b.SetControlPoints(p0, p1, p2, p3);
          for(count j = 1; j <= PointsPerCurve; j++)
            p.Add() = b.Value(number(j) / number(PointsPerCurve));
        }
        p0 = p3;
      }
      return p;
    }
  };
}
#endif
