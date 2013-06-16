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

#ifndef BELLE_CORE_PATH_H
#define BELLE_CORE_PATH_H

#include "Transform.h"

namespace BELLE_NAMESPACE
{
  class Instruction
  {
    friend class Typeface;
    
    ///The instruction type (move-to, line-to, cubic-to, close path)
    prim::count Type;
    
    ///Data points for curves.
    mutable prim::planar::Vector Point[3];
    
    //Enumeration of construction types
    enum ConstructionType
    {
      MoveTo = 1,
      LineTo,
      CubicTo,
      ClosePath
    };
    
    public:
    
    ///First control point of a cubic.
    inline prim::planar::Vector& Control1() const
    {
      return Point[0];
    }
    
    ///Second control point of a cubic.
    inline prim::planar::Vector& Control2() const
    {
      return Point[1];
    }
    
    ///Final point of a line, cubic or move.
    inline prim::planar::Vector& End() const
    {
      return Point[2];
    }
    
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
    
    ///Default constructor creates a close path instruction.
    Instruction() : Type(ClosePath) {}
    
    /**Constructor for MoveTo or LineTo. If StartNewPath is true, then MoveTo
    is used. If not, then LineTo is used.*/
    Instruction(prim::planar::Vector p1, bool StartNewPath = false)
    {
      Type = (StartNewPath ? MoveTo : LineTo);
      End() = p1;
    }
    
    ///Constructor for CubicTo.
    Instruction(prim::planar::Vector ControlPoint1,
      prim::planar::Vector ControlPoint2, prim::planar::Vector EndPoint) :
      Type(CubicTo)
    {
      Control1() = ControlPoint1;
      Control2() = ControlPoint2;
      End() = EndPoint;
    }
    
    ///Constructor for CubicTo via Bezier.
    Instruction(prim::planar::Bezier& BezierCurve) : Type(CubicTo)
    {
      prim::planar::Vector Start;
      BezierCurve.GetControlPoints(Start, Control1(), Control2(), End());
    }
    
    ///Constructs a transformed copy of an instruction.
    Instruction(const Instruction& i, Affine a)
    {
      Type = i.Type;
      for(prim::count j = 0; j < 3; j++)
        Point[j] = a << i.Point[j];
    }
  };

#if defined(JUCE_VERSION)
  //Inherit native path base classes.
  class Path : public juce::Path
#else
  ///Vector path object
  class Path
#endif
  {
    prim::Array<Instruction> Instructions;
    prim::Array<prim::planar::Polygon> CachedOutline;
    prim::planar::Rectangle BoundingBox;
    
    public:
    
    ///Default constructor
    Path() {}
    
    ///Constructs a transformed copy of a path.
    Path(const Path& p, Affine a)
    {
      Append(p, a);
    }
    
    ///Appends a transformed copy of a path.
    void Append(const Path& p, Affine a = Affine::Unit())
    {
      for(prim::count i = 0; i < p.Instructions.n(); i++)
        Add(Instruction(p.Instructions[i], a));      
    }
    
    ///Appends a transformed copy of a polygon.
    void Append(const prim::planar::Polygon& p, Affine a = Affine::Unit())
    {
      //Must have at least 3 vertices to be valid.
      if(p.n() < 3) return;

      //Create a MoveTo followed by (n - 1) LineTo's.
      for(prim::count i = 0; i < p.n(); i++)
        Add(Instruction(a << p[i], !i));
      
      //Close the polygon in the subpath.
      Add(Instruction());
    }
    
    ///Appends a transformed copy of a polygon array.
    void Append(const prim::Array<prim::planar::Polygon>& p,
      Affine a = Affine::Unit())
    {
      for(prim::count j = 0; j < p.n(); j++)
      {
        //Must have at least 3 vertices to be valid.
        if(p[j].n() < 3) continue;
  
        //Create a MoveTo followed by (n - 1) LineTo's.
        for(prim::count i = 0; i < p[j].n(); i++)
          Add(Instruction(a << p[j][i], !i));
        
        //Close the polygon in the subpath.
        Add(Instruction());
      }
    }
    
    ///Appends a transformed convex hull of a path.
    void AppendConvex(const Path& p, Affine a = Affine::Unit())
    {
      for(prim::count i = 0; i < p.Outline().n(); i++)
      {
        prim::planar::Polygon Hull;
        p.Outline()[i].CreateConvexHull(Hull);
        Append(Hull, a);
      }
    }
    
    ///Adds instruction and updates bounding box, polygon, and native base path.
    void Add(const Instruction& i)
    {
      prim::planar::Vector e = i.End();
      prim::planar::Vector c1 = i.Control1();
      prim::planar::Vector c2 = i.Control2();
      
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
        startNewSubPath((float)e.x, (float)e.y);
      else if(i.IsLine())
        lineTo((float)e.x, (float)e.y);
      else if(i.IsCubic())
        cubicTo((float)c1.x, (float)c1.y, (float)c2.x, (float)c2.y,
          (float)e.x, (float)e.y);
      else if(i.IsClosing())
        closeSubPath();
#endif
    }
    
    ///Retrieves the i-th path construction.
    inline const Instruction& operator [] (prim::count i) const
    {
      return Instructions[i];
    }
    
    ///Retrieves the i-th path construction.
    inline const Instruction& ith(prim::count i) const
    {
      return Instructions[i];
    }
    
    ///Retrieves the number of path constructions.
    inline prim::count n() const
    {
      return Instructions.n();
    }
    
    ///Retrieves the first path construction.
    inline const Instruction& a() const
    {
      return Instructions.a();
    }
    
    ///Retrieves the last path construction.
    inline const Instruction& z(prim::count ItemsFromEnd = 0) const
    {
      return Instructions.z(ItemsFromEnd);
    }
    
    ///Retrieves the current end point.
    prim::planar::Vector End()
    {
      if(n())
        return z().End();
      else
        return prim::planar::Vector();
    }
    
    ///Retrieves the cached polygon outline of this path.
    inline const prim::Array<prim::planar::Polygon>& Outline() const
    {
      return CachedOutline;
    }
    
    ///Retrieves the cached bounding box for this path.
    inline prim::planar::Rectangle Bounds() const
    {
      return BoundingBox;
    }
    
    ///Retrieves the bounding box of the transformed path box.
    prim::planar::Rectangle Bounds(const Affine& Transformation) const
    {
      prim::planar::Rectangle rt;
      rt = rt + (Transformation << BoundingBox.BottomLeft());
      rt = rt + (Transformation << BoundingBox.TopLeft());
      rt = rt + (Transformation << BoundingBox.TopRight());
      rt = rt + (Transformation << BoundingBox.BottomRight());
      return rt;
    }
    
    ///Retrieves the rectangular polygon box of the transformed path's box.
    prim::planar::Polygon BoundsPolygon(const Affine& Transformation) const
    {
      prim::planar::Polygon p;
      p.n(4);
      p[0] = Transformation << BoundingBox.BottomLeft();
      p[1] = Transformation << BoundingBox.TopLeft();
      p[2] = Transformation << BoundingBox.TopRight();
      p[3] = Transformation << BoundingBox.BottomRight();
      return p;
    }
  };
}
#endif
