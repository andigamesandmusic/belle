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

#ifndef PRIM_INCLUDE_PLANAR_H
#define PRIM_INCLUDE_PLANAR_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  //-----------------------//
  //Vector Type-Definitions//
  //-----------------------//

  ///Planar number coordinate
  typedef Complex<number> Vector;

  ///Planar integer coordinate
  typedef Complex<integer> VectorInt;

  //-----//
  //Boxes//
  //-----//

  /**Stores a rectangle as a pair of opposite vectors. A non-empty rectangle
  is defined to be one that has two non-empty vectors. An ordered rectangle is
  a non-empty rectangle with ordered coordinates such that a is the bottom-left
  corner and b is the top-right corner.*/
  template <class T>
  class BoxT
  {
    public:

    ///Location of the first of two opposite corners of a rectangle.
    Complex<T> a;

    ///Location of the second of two opposite corners of a rectangle.
    Complex<T> b;

    //------------//
    //Constructors//
    //------------//

    //Default constructor creates an invalid rectangle.
    BoxT() : a(Complex<T>::Empty()), b(Complex<T>::Empty()) {}

    ///Creates a collapsed rectangle at a single point.
    BoxT(Complex<T> p) : a(p), b(p) {}

    ///Creates a rectangle from a pair of minimum and maximum vectors.
    BoxT(Complex<T> p1, Complex<T> p2) :
      a(p1), b(p2) {}

    ///Creates a rectangle from the coordinates of its four sides.
    BoxT(T x1, T y1, T x2, T y2) :
      a(x1, y1), b(x2, y2) {}

    //------//
    //String//
    //------//

    ///Returns a string version of the rectangle.
    operator String () const
    {
      String s;
      if(IsEmpty())
        s << "(Empty)";
      else
        s << "(" << a << ", " << b << ")";
      return s;
    }

    //---------//
    //Emptiness//
    //---------//

    /**Returns true if the rectangle is empty. A rectangle is empty if it is
    uninitialized (one or both points are empty). Boxes with zero area are
    not considered empty.*/
    inline bool IsEmpty() const
    {
      return a.IsEmpty() or b.IsEmpty();
    }

    /**Clears a rectangle by reinitializing its coordinates to empty. The
    rectangle returns to an invalid and non-ordered state.*/
    inline void Clear()
    {
      a = b = Complex<T>::Empty();
    }

    //-----//
    //Order//
    //-----//

    /**Ensures that the coordinates are in ascending order. IsOrdered() will
    still return false if either the width or height is zero.*/
    void Order()
    {
      Ascending(a.x, b.x);
      Ascending(a.y, b.y);
    }

    /**Returns whether or not the two points a and b are in ascending order.
    The rectangle must also be non-empty (have non-zero area) to be ordered.*/
    bool IsOrdered() const
    {
      return a.x < b.x and a.y < b.y;
    }

    //----------//
    //Dimensions//
    //----------//

    /**Returns the positive difference of the x-coordinates. An empty
    rectangle may have a non-zero width.*/
    inline T Width() const
    {
      return Abs(b.x - a.x);
    }

    /**Returns the positive difference of the y-coordinates. An empty
    rectangle may have a non-zero height.*/
    inline T Height() const
    {
      return Abs(b.y - a.y);
    }

    ///Returns the positive size of the rectangle.
    inline Complex<T> Size() const
    {
      return Complex<T>(Width(), Height());
    }

    /**Returns the positive or negative difference of the x-coordinates. An
    empty rectangle may have a non-zero width.*/
    inline T DeltaX() const
    {
      return b.x - a.x;
    }

    /**Returns the positive or negative difference of the y-coordinates. An
    empty rectangle may have a non-zero height.*/
    inline T DeltaY() const
    {
      return b.y - a.y;
    }

    /**Returns the difference of the rectangle coordinates. Either dimension may
    be negative depending on how the coordinates are ordered.*/
    inline Complex<T> DeltaSize() const
    {
      return Complex<T>(b - a);
    }

    ///Returns the postive area of the rectangle.
    inline T Area() const
    {
      return Abs(DeltaX() * DeltaY());
    }

    /**Returns the positive perimeter of the rectangle. An empty rectangle
    may have a non-zero perimeter if one of its sides has non-zero length.*/
    inline T Perimeter() const
    {
      return (Width() + Height()) * T(2);
    }

    ///Returns the coordinate of the left side.
    inline T Left() const {return Min(a.x, b.x);}

    ///Returns the coordinate of the bottom side.
    inline T Bottom() const {return Min(a.y, b.y);}

    ///Returns the coordinate of the right side.
    inline T Right() const {return Max(a.x, b.x);}

    ///Returns the coordinate of the top side.
    inline T Top() const {return Max(a.y, b.y);}

    ///Returns the coordinate of the bottom-left corner.
    inline Complex<T> BottomLeft() const
    {
      return Complex<T>(Left(), Bottom());
    }

    ///Returns the coordinate of the bottom-right corner.
    inline Complex<T> BottomRight() const
    {
      return Complex<T>(Right(), Bottom());
    }

    ///Returns the coordinate of the top-left corner.
    inline Complex<T> TopLeft() const
    {
      return Complex<T>(Left(), Top());
    }

    ///Returns the coordinate of the top-right corner.
    inline Complex<T> TopRight() const
    {
      return Complex<T>(Right(), Top());
    }

    ///Returns the coordinate of the center.
    inline Complex<T> Center() const
    {
      return Complex<T>((a.x + b.x) / T(2), (a.y + b.y) / T(2));
    }

    ///Sets the coordinate of the left side.
    inline void Left(T NewLeft)
    {
      a.x = NewLeft;
    }

    ///Sets the coordinate of the bottom side.
    inline void Bottom(T NewBottom)
    {
      a.y = NewBottom;
    }

    ///Sets the coordinate of the right side.
    inline void Right(T NewRight)
    {
      b.x = NewRight;
    }

    ///Sets the coordinate of the top side.
    inline void Top(T NewTop)
    {
      b.y = NewTop;
    }

    ///Sets the coordinate of the bottom-left corner.
    inline void BottomLeft(Complex<T> p)
    {
      a = p;
    }

    ///Sets the coordinate of the bottom-right corner.
    inline void BottomRight(Complex<T> p)
    {
      b.x = p.x;
      a.y = p.y;
    }

    ///Sets the coordinate of the top-left corner.
    inline void TopLeft(Complex<T> p)
    {
      a.x = p.x;
      b.y = p.y;
    }

    ///Sets the coordinate of the top-right corner.
    inline void TopRight(Complex<T> p)
    {
      b = p;
    }

    ///Returns whether a particular point is contained by the rectangle.
    inline bool Contains(Complex<T> p) const
    {
      return p.x >= Left() and p.y >= Bottom() and
             p.x < Right() and p.y < Top();
    }

    ///Returns whether another rectangle is contained by the rectangle.
    inline bool Contains(const BoxT& r) const
    {
      return Contains(r.a) and Contains(r.b);
    }

    /**Subtracts an amount to a and adds an equivalent amount to b. For positive
    values, the rectangle is guaranteed to always gets bigger, regardless of
    the ordering of the corners. For negative values with magnitude less than
    both the width and height of the rectangle, the rectangle gets smaller, and
    for more negative values the result is undefined.*/
    void Dilate(T Amount)
    {
      T SgnOfdx =  Sign(DeltaX());
      T SgnOfdy = Sign(DeltaY());
      T SgnOfdxTimesAmount = SgnOfdx * Amount;
      T SgnOfdyTimesAmount = SgnOfdy * Amount;

      a.x -= SgnOfdxTimesAmount;
      b.x += SgnOfdxTimesAmount;
      a.y -= SgnOfdyTimesAmount;
      b.y += SgnOfdyTimesAmount;
    }

    /**Computes the intersection of two rectangles. If either rectangle is
    empty then an empty rectangle is returned.*/
    static BoxT Intersection(BoxT r1, BoxT r2)
    {
      if(r1.IsEmpty() or r2.IsEmpty())
        return BoxT(); //Return an empty rectangle.

      r1.Order();
      r2.Order();

      T tblx = r1.a.x;
      T oblx = r2.a.x;
      T tbly = r1.a.y;
      T obly = r2.a.y;
      T ttrx = r1.b.x;
      T otrx = r2.b.x;
      T ttry = r1.b.y;
      T otry = r2.b.y;

      T nblx = T(0);
      T nbly = T(0);
      T ntrx = T(0);
      T ntry = T(0);

      //X-dimension
      if(tblx <= oblx and oblx <= ttrx)
      {
        nblx = oblx;
        if(otrx <= ttrx)
        {
          //Case: OTHER is completely contained by THIS.
          ntrx = otrx;
        }
        else
        {
          //Case: Only OTHER's minimum is contained by THIS.
          ntrx = ttrx;
        }
      }
      else if(tblx <= otrx and otrx <= ttrx)
      {
        //Case: Only OTHER's maximum is contained by THIS.
        nblx = tblx;
        ntrx = otrx;
      }
      else if(oblx <= tblx and tblx <= otrx and oblx <= ttrx and ttrx <= otrx)
      {
        //Case: THIS is completely contained by OTHER.
        nblx = tblx;
        ntrx = ttrx;
      }
      else
      {
        //Case: No intersection.
        return BoxT();
      }

      //Y-dimension
      if(tbly <= obly and obly <= ttry)
      {
        nbly = obly;
        if(otry <= ttry)
        {
          //Case: OTHER is completely contained by THIS.
          ntry = otry;
        }
        else
        {
          //Case: Only OTHER's minimum is contained by THIS.
          ntry = ttry;
        }
      }
      else if(tbly <= otry and otry <= ttry)
      {
        //Case: Only OTHER's maximum is contained by THIS.
        nbly = tbly;
        ntry = otry;
      }
      else if(obly <= tbly and tbly <= otry and obly <= ttry and ttry <= otry)
      {
        //Case: THIS is completely contained by OTHER.
        nbly = tbly;
        ntry = ttry;
      }
      else
      {
        //Case: No intersection.
        return BoxT();
      }

      return BoxT(Complex<T>(nblx, nbly), Complex<T>(ntrx, ntry));
    }

    /*Finds the union of two rectangles, or bounding rectangle. If one is empty,
    the other is returned. If both are empty the result is an empty rectangle.*/
    static BoxT Union(BoxT r1, BoxT r2)
    {
      bool r1Empty = r1.IsEmpty();
      bool r2Empty = r2.IsEmpty();

      r1.Order();
      r2.Order();

      if(r1Empty and r2Empty)
        return BoxT(); //Return an empty rectangle.
      else if(r1Empty)
        return r2;
      else if(r2Empty)
        return r1;

      T tblx = r1.a.x;
      T oblx = r2.a.x;
      T tbly = r1.a.y;
      T obly = r2.a.y;
      T ttrx = r1.b.x;
      T otrx = r2.b.x;
      T ttry = r1.b.y;
      T otry = r2.b.y;

      Complex<T> a(Min(tblx, oblx), Min(tbly, obly));
      Complex<T> b(Max(ttrx, otrx), Max(ttry, otry));

      return BoxT(a, b);
    }

    /*Finds the bounding rectangle of a rectangle and a point. This method does
    not check to ensure that the rectangle is valid beforehand, so it is
    possible to do a union of a collapsed rectangle of zero area and another
    vector to get the bounding rectangle of the two points.*/
    static BoxT Union(BoxT r1, Complex<T> p1)
    {
      if(r1.IsEmpty())
        return BoxT(p1, p1);

      T tblx = r1.a.x;
      T tbly = r1.a.y;
      T ttrx = r1.b.x;
      T ttry = r1.b.y;
      T ox = p1.x;
      T oy = p1.y;

      Complex<T> a(Min(tblx, ox), Min(tbly, oy));
      Complex<T> b(Max(ttrx, ox), Max(ttry, oy));

      return BoxT(a, b);
    }

    /**Finds the intersection of two rectangles. The intersection is a
    commutative operation (the order of arguments does not matter).*/
    BoxT operator - (const BoxT& Other) const
    {
      return Intersection(*this, Other);
    }

    /**Finds the union of two rectangles. The union is a commutative
    operation (the order of arguments does not matter).*/
    BoxT operator + (const BoxT& Other) const
    {
      return Union(*this, Other);
    }

    ///Calculates the union of a rectangle and a point.
    BoxT operator + (const Complex<T>& Other) const
    {
      return Union(*this, Other);
    }

    ///Intersects this rectangle with another.
    BoxT operator -= (const BoxT& Other)
    {
      return (*this) = (*this) - Other;
    }

    ///Unions this rectangle with another.
    BoxT operator += (const BoxT& Other)
    {
      return (*this) = (*this) + Other;
    }

    ///Unions this rectangle with a point.
    BoxT operator += (const Complex<T>& Other)
    {
      return (*this) = (*this) + Other;
    }

    ///Determines if this rectangle has the same vertices as another.
    bool operator == (const BoxT& Other) const
    {
      BoxT r1 = *this, r2 = Other;
      return r1.a == r2.a and r1.b == r2.b;
    }

    public:

    ///Indicates a side of the rectangle.
    enum Side
    {
      LeftSide,
      RightSide,
      BottomSide,
      TopSide
    };

    private:

    ///Gets the requested side as another rectangle.
    static BoxT GetSide(const BoxT& R, Side S)
    {
      switch(S)
      {
        case LeftSide:   return BoxT(R.BottomLeft(),  R.TopLeft());
        case RightSide:  return BoxT(R.BottomRight(), R.TopRight());
        case BottomSide: return BoxT(R.BottomLeft(),  R.BottomRight());
        case TopSide:
        break;
      }
      return BoxT(R.TopLeft(), R.TopRight());
    }

    ///Gets the opposite of the requested side as another rectangle.
    static BoxT GetOppositeSide(const BoxT& R, Side S)
    {
      switch(S)
      {
        case LeftSide:   return BoxT(R.BottomRight(), R.TopRight());
        case RightSide:  return BoxT(R.BottomLeft(),  R.TopLeft());
        case BottomSide: return BoxT(R.TopLeft(),     R.TopRight());
        case TopSide:
        break;
      }
      return BoxT(R.BottomLeft(), R.BottomRight());
    }

    ///Gets the most extreme number in the direction of the side.
    static T Extreme(T a, T b, Side S)
    {
      switch(S)
      {
        //Looking for the lowest value of the two.
        case LeftSide:
        case BottomSide:
        return Min(a, b);

        //Looking for the highest value of the two.
        case TopSide:
        case RightSide:
        break;
      }
      return Max(a, b);
    }

    ///Gets the baseline coordinate of the given side.
    static T& Baseline(Complex<T>& v, Side S)
    {
      switch(S)
      {
        //The baseline for left and right segments is the y-coordinate.
        case LeftSide:
        case RightSide:
        return v.y;

        //The baseline for top and bottom segments is the x-coordinate.
        case TopSide:
        case BottomSide:
        break;
      }
      return v.x;
    }

    ///Gets the offset coordinate of the given side.
    static T& Offset(Complex<T>& v, Side S)
    {
      switch(S)
      {
        //The offset for left and right segments is the x-coordinate.
        case LeftSide:
        case RightSide:
        return v.x;

        //The offset for top and bottom segments is the y-coordinate.
        case TopSide:
        case BottomSide:
        break;
      }
      return v.y;
    }

    ///Gets a sided vector.
    static Complex<T> SidedVector(T BaselineValue, T OffsetValue, Side S)
    {
      Complex<T> v;
      Baseline(v, S) = BaselineValue;
      Offset(v, S) = OffsetValue;
      return v;
    }

    ///Inserts the rectangle side into the segmented hull list.
    static void InsertSide(List<Complex<T> >& L, BoxT R, Side S)
    {
      T SegmentToAddStart = Baseline(R.a, S);
      T SegmentToAddEnd   = Baseline(R.b, S);
      Complex<T> SegmentToAddOffset = GetSide(R, S).a;

      for(count i = 0; i < L.n() - 1; i++)
      {
        T SegmentStart = Baseline(L[i], S);
        T SegmentEnd   = Baseline(L[i + 1], S);

        //If the segment being added is out-of-range then skip this segment.
        if(not (SegmentToAddStart < SegmentEnd and
          SegmentToAddEnd > SegmentStart))
            continue;

        //Get the inner segment contained by the segment in question.
        T InnerSegmentStart = Max(SegmentToAddStart, SegmentStart);
        T InnerSegmentEnd   = Min(SegmentToAddEnd, SegmentEnd);

        bool StartEqual =
          Limits<T>::IsEqual(InnerSegmentStart, SegmentStart);
        bool EndEqual =
          Limits<T>::IsEqual(InnerSegmentEnd, SegmentEnd);

        T InnerSegmentValue = Offset(SegmentToAddOffset, S);
        T SegmentValue = Offset(L[i], S);
        T ExtremeValue = Extreme(SegmentValue, InnerSegmentValue, S);

        if(Limits<T>::IsEqual(SegmentValue, ExtremeValue))
          continue;

        if(StartEqual and EndEqual)
          Offset(L[i], S) = ExtremeValue;
        else if(StartEqual and not EndEqual)
        {
          Offset(L[i], S) = ExtremeValue;
          L.InsertAfter(SidedVector(InnerSegmentEnd, SegmentValue, S), i++);
        }
        else if(not StartEqual and EndEqual)
          L.InsertAfter(SidedVector(InnerSegmentStart, ExtremeValue, S), i++);
        else
        {
          L.InsertAfter(SidedVector(InnerSegmentStart, ExtremeValue, S), i++);
          L.InsertAfter(SidedVector(InnerSegmentEnd, SegmentValue, S), i++);
        }
      }
    }

    ///Returns whether the side is a vertical one.
    static bool IsVertical(Side S) {return S == LeftSide or S == RightSide;}

    ///Returns whether the side is a horizontal one.
    static bool IsHorizontal(Side S) {return !IsVertical(S);}

    public:

    ///Converts the hull to an array of one-dimensional rectangles.
    static Array<BoxT> HullAsBoxes(const List<Complex<T> >& Hull,
      Side S)
    {
      Array<BoxT> Boxes(Hull.n() - 1);
      for(count i = 0; i < Boxes.n(); i++)
        (Boxes[i] = BoxT(Hull[i], Vector(Hull[i + IsHorizontal(S)].x,
          Hull[i + IsVertical(S)].y))).Order();
      return Boxes;
    }

    ///Merges two hulls together to form a single hull.
    static List<Complex<T> > MergeHulls(const List<Complex<T> >& A,
      const List<Complex<T> >& B, Side S)
    {
      Array<BoxT> AllBoxes;
      AllBoxes.Append(HullAsBoxes(A, S));
      AllBoxes.Append(HullAsBoxes(B, S));
      return SegmentedHull(AllBoxes, S);
    }

    ///Returns the segmented hull of a given side of a set of rectangles.
    static List<Complex<T> > SegmentedHull(const Array<BoxT>& Boxes,
      Side S)
    {
      List<Complex<T> > L;
      if(not Boxes.n())
        return L;

      //Get the bounding box of all the rectangles.
      BoxT Bounds;
      for(count i = 0; i < Boxes.n(); i++)
        Bounds += Boxes[i];

      //Create the first line segment as the baseline of the bounds.
      {
        BoxT x = GetOppositeSide(Bounds, S);
        L.Add() = x.a;
        L.Add() = x.b;
      }

      //Insert each rectangle side into the segment hull list.
      for(count i = 0; i < Boxes.n(); i++)
        InsertSide(L, Boxes[i], S);

      //Remove redundant offsets.
      for(count i = L.n() - 3; i >= 0; i--)
        if(Limits<T>::IsEqual(Offset(L[i], S), Offset(L[i + 1], S)))
          L.Remove(i + 1);

      //Adjust the last offset to that of the previous segment for consistency.
      Offset(L.z(), S) = Offset(L.z(1), S);

      return L;
    }

    ///Gets closest two segmented hulls can be placed approached from a side.
    static Complex<T> OffsetToPlaceOnSide(const List<Complex<T> >& Anchor_,
      const List<Complex<T> >& Mover_, typename BoxT::Side S)
    {
      List<Complex<T> > Anchor = Anchor_;
      List<Complex<T> > Mover  = Mover_;

      bool DeltaExists = false;
      T FinalDelta = 0.f;
      for(count i = 0; i < Mover.n() - 1; i++)
      {
        T MoverStart = Baseline(Mover[i], S);
        T MoverEnd   = Baseline(Mover[i + 1], S);
        for(count j = 0; j < Anchor.n() - 1; j++)
        {
          T AnchorStart = Baseline(Anchor[j], S);
          T AnchorEnd   = Baseline(Anchor[j + 1], S);

          //If the segments do not overlap, then no offset consideration needed.
          if(MoverEnd <= AnchorStart or MoverStart >= AnchorEnd)
            continue;

          T Delta = Offset(Anchor[j], S) - Offset(Mover[i], S);
          if(not DeltaExists)
          {
            FinalDelta = Delta;
            DeltaExists = true;
          }
          else
            FinalDelta = Extreme(Delta, FinalDelta, S);
        }
      }
      Complex<T> Result;
      Offset(Result, S) = FinalDelta;
      return Result;
    }
  };

  ///Planar rectangle with number coordinates
  typedef BoxT<number> Box;

  ///Planar rectangle with integer coordinates
  typedef BoxT<integer> BoxInt;

  //-----//
  //Lines//
  //-----//

  ///Line segment represented by two coordinates
  class Line
  {
    public:

    ///First coordinate
    Vector a;

    ///Second coordinate
    Vector b;

    ///Creates an empty line with both coordinates at the origin.
    Line() {}

    ///Creates a line from four individual coordinate values.
    Line(number x1, number y1, number x2, number y2) :
      a(x1, y1), b(x2, y2) {}

    ///Creates a line from two vector coordinate values.
    Line(Vector p1, Vector p2) : a(p1), b(p2) {}

    ///Returns the length of the segment ab.
    number Length() const
    {
      return Distance(a.x, a.y, b.x, b.y);
    }

    ///Returns the angle in radians from a to b.
    number Angle() const
    {
      return (b - a).Ang();
    }

    /**Calculates the horizontal intersection at a particular y and stores in x.
    Returns whether or not an intersection was found. The interval is closed, so
    an intersection with endpoints will be reported.*/
    inline bool HorizontalIntersection(number& x, number y = 0.0f)
    {
      //Fetch the segment into local variables.
      number p1x = a.x, p2x = b.x;
      number p1y = a.y, p2y = b.y;

      //Test the endpoints first since they are excluded by the following tests.
      if(Limits<number>::IsEqual(p1y, y))
      {
        x = p1x;
        return true;
      }
      else if(Limits<number>::IsEqual(p2y, y))
      {
        x = p2x;
        return true;
      }

      /*Translate the problem into a segment intersection with the line y = 1.
      Calculating at y = 0 is ill-behaved because of floating-point denormals.
      On the other hand the behavior around y = 1 is well defined with the
      three points around 1.0 being {1 - e / 2, 1.0, 1 + e} where e is the
      epsilon value of the floating-point number type. Since the intersection
      is only calculated if the y values are on opposite sides of 1.0, a stable
      calculation is guaranteed.*/
      number dy = -y + 1.0f;
      p1y += dy;
      p2y += dy;

      //Check whether there is an intersection.
      if(not (p1y > 1.0f and p2y < 1.0f) and not (p2y > 1.0f and p1y < 1.0f))
      {
        x = 0;
        return false;
      }

      /*Line is defined as:
        x = p1x + t * (p2x - p1x)
        y = p1y + t * (p2y - p1y)
        where t = (0, 1).

      The following division is safe to do because of the above check.*/
      number t = (p1y - 1.0f) / (p1y - p2y);

      //Calculate the x-intersection.
      x = p1x + t * (p2x - p1x);

      return true;
    }

    /**Calculates the vertical intersection at a particular x and stores in y.
    Returns whether or not an intersection was found. The interval is closed, so
    an intersection with endpoints will be reported.*/
    inline bool VerticalIntersection(number& y, number x = 0.0f)
    {
      //Fetch the segment into local variables.
      number p1x = a.x, p2x = b.x;
      number p1y = a.y, p2y = b.y;

      //Test the endpoints first since they are excluded by the following tests.
      if(Limits<number>::IsEqual(p1x, x))
      {
        y = p1y;
        return true;
      }
      else if(Limits<number>::IsEqual(p2x, x))
      {
        y = p2y;
        return true;
      }

      /*Translate the problem into a segment intersection with the line x = 1.
      Calculating at x = 0 is ill-behaved because of floating-point denormals.
      On the other hand the behavior around x = 1 is well defined with the
      three points around 1.0 being {1 - e / 2, 1.0, 1 + e} where e is the
      epsilon value of the floating-point number type. Since the intersection
      is only calculated if the x values are on opposite sides of 1.0, a stable
      calculation is guaranteed.*/
      number dx = -x + 1.0f;
      p1x += dx;
      p2x += dx;

      //Check whether there is an intersection.
      if(not (p1x > 1.0f and p2x < 1.0f) and not (p2x > 1.0f and p1x < 1.0f))
      {
        y = 0;
        return false;
      }

      /*Line is defined as:
        x = p1x + t * (p2x - p1x)
        y = p1y + t * (p2y - p1y)
        where t = (0, 1).

      The following division is safe to do because of the above check.*/
      number t = (p1x - 1.0f) / (p1x - p2x);

      //Calculate the y-intersection.
      y = p1y + t * (p2y - p1y);

      return true;
    }

    /**Horizontal distance from point to line segment traveling left. If the
    point would not intersect if it traveled at angle -Pi (left), then -1.0 is
    returned.*/
    inline number HorizontalDistance(Vector v)
    {
      number IntersectionX;
      if(HorizontalIntersection(IntersectionX, v.y))
      {
        number Distance = v.x - IntersectionX;
        return Distance > 0.0f ? Distance : 0.0;
      }
      return 0.0;
    }

    /**Clips the line over the interval y = [y1, y2]. The order does not matter.
    If the y-interval of the clipper does not intersect with the y-interval of
    the original line, then the original line is returned.*/
    inline Line ClipVertical(number y1, number y2)
    {
      //Get local copy of source line.
      number Clippedx1 = a.x, Clippedx2 = b.x;
      number Clippedy1 = a.y, Clippedy2 = b.y;

      //Sort ascending by y value.
      if(Clippedy1 > Clippedy2)
      {
        Swap(Clippedx1, Clippedx2);
        Swap(Clippedy1, Clippedy2);
      }
      if(y1 > y2)
        Swap(y1, y2);

      //Clip by y1 and y2.
      if(Clippedy1 < y1 and y1 < Clippedy2)
      {
        HorizontalIntersection(Clippedx1, y1);
        Clippedy1 = y1;
      }

      if(Clippedy1 < y2 and y2 < Clippedy2)
      {
        HorizontalIntersection(Clippedx2, y2);
        Clippedy2 = y2;
      }

      //Undo the sort when creating the line copy.
      return Line(Clippedx1, Clippedy1, Clippedx2, Clippedy2);
    }

    ///Makes a line segment parallel to this one with the same length.
    Line MakeParallelLine(number Displacement) const
    {
      number LineAngle = (b - a).Ang();
      number DisplacementAngle = LineAngle + HalfPi<number>();
      Vector DisplacementVector;
      DisplacementVector.Polar(DisplacementAngle);
      DisplacementVector *= Displacement;
      return Line(a + DisplacementVector, b + DisplacementVector);
    }

    /*An asymptote-less line-line intersection algorithm that works equally
    well for all configurations of lines by using polar coordinates.*/
    Vector GetPolarLineIntersection(const Line& OtherLine) const
    {
      //Make copies of the parameters so we can modify them.
      Vector Start1 = a, End1 = b, Start2 = OtherLine.a, End2 = OtherLine.b;

      //Rotate the system such that line 1 is horizontal.
      number InitialAngle = (b - a).Ang();
      Start1.Ang(Start1.Ang() - InitialAngle);
      End1.Ang(End1.Ang() - InitialAngle);
      Start2.Ang(Start2.Ang() - InitialAngle);
      End2.Ang(End2.Ang() - InitialAngle);

      /*At this point Start1.y = End1.y; transpose such that line 1 becomes
      the x-axis.*/
      Vector InitialTransposition = Vector(0, Start1.y);
      Start1 -= InitialTransposition;
      End1 -= InitialTransposition;
      Start2 -= InitialTransposition;
      End2 -= InitialTransposition;

      //Now solve for x-axis intersection of line 2.
      Vector Delta = End2 - Start2;

      if(Limits<number>::IsZero(Delta.x) and Limits<number>::IsZero(Delta.y))
        return Vector(0, 0); //Line is a point.
      else if(Limits<number>::IsZero(Delta.y))
        return Vector(0, 0); //Lines are perfectly parallel.

      //Initialize intersection to origin; then just find x-component.
      Vector Intersection(0, 0);

      //For lines that are more horizontal than vertical...
      if(Abs(Delta.y) < Abs(Delta.x))
      {
        /*
        Solve x-axis intersection using:
          0 = (dy/dx)x + b
          x = -b / (dy/dx) at y = 0
        with:
          b = y - (dy/dx)x
        */
        number dyOverdx = Delta.y / Delta.x;
        number b_ = Start2.y - dyOverdx * Start2.x;
        Intersection.x = -b_ / dyOverdx;
      }
      else //for lines that are as or more vertical than horizontal...
      {
        /*
        Solve x-axis intersection directly:
          x = (dx/dy)y + c
        with:
          c = x - (dx/dy)y
        */
        number dxOverdy = Delta.x / Delta.y;
        number c = Start2.x - dxOverdy * Start2.y;
        Intersection.x = c;
      }

      //Undo the transformations from the beginning.
      Intersection += InitialTransposition;
      Intersection.Ang(Intersection.Ang() + InitialAngle);

      return Intersection;
    }

    /**Calculates a line-line intersection quickly. The method assumes that
    a single intersection exists.*/
    Vector GetLineIntersection(const Line& OtherLine) const
    {
      // a1 * x + b1 * y + c1 = 0 is line 1
      number a1 = b.y - a.y;
      number b1 = a.x - b.x;
      number c1 = b.x * a.y - a.x * b.y;

      // a2 * x + b2 * y + c2 = 0 is line 2
      number a2 = OtherLine.b.y - OtherLine.a.y;
      number b2 = OtherLine.a.x - OtherLine.b.x;
      number c2 = OtherLine.b.x * OtherLine.a.y -
        OtherLine.a.x * OtherLine.b.y;

      number Determinant = a1 * b2 - a2 * b1;
      if(Limits<number>::IsZero(Abs(Determinant)))
      {
        //Lines do not intersect, or they are the same line.
        return Vector(0, 0);
      }

      number x = (b1 * c2 - b2 * c1) / Determinant;
      number y = (a2 * c1 - a1 * c2) / Determinant;

      Vector Intersection(x, y);

      return Intersection;
    }

    /**Returns the clockwise order of three points. Returns 1 if the points
    are clockwise; returns -1 if the points are counterclockwise; returns 0
    if the points are collinear.*/
    static inline bool DetermineClockwiseOrder(
      Vector p0, Vector p1, Vector p2)
    {
      return (p2.y - p0.y) * (p1.x - p0.x) < (p1.y - p0.y) * (p2.x - p0.x);
    }

    ///Determines whether or not this line intersects with another one.
    inline bool Intersects(const Line& OtherLine) const
    {
      number Pax = a.x, Pay = a.y, Pbx = b.x, Pby = b.y;
      number Qax = OtherLine.a.x, Qay = OtherLine.a.y,
        Qbx = OtherLine.b.x, Qby = OtherLine.b.y;

      number Pbx_Pax = Pbx - Pax;
      number Qax_Pax = Qax - Pax;
      number Qbx_Pax = Qbx - Pax;
      number Pby_Pay = Pby - Pay;
      number Qay_Pay = Qay - Pay;
      number Qby_Pay = Qby - Pay;
      number Qax_Pbx = Qax - Pbx;
      number Qbx_Pbx = Qbx - Pbx;
      number Qay_Pby = Qay - Pby;
      number Qby_Pby = Qby - Pby;

      bool Result = (Qby_Pay * Qax_Pax < Qay_Pay * Qbx_Pax) !=
      (Qby_Pby * Qax_Pbx < Qay_Pby * Qbx_Pbx) and
      (Qay_Pay * Pbx_Pax < Pby_Pay * Qax_Pax) !=
      (Qby_Pay * Pbx_Pax < Pby_Pay * Qbx_Pax);

      return Result;
    }

    //Returns minimum distance from the point to this line.
    number MinimumDistance(Vector p) const
    {
      number t = (p - a).Dot(b - a) / Square((a - b).Mag());
      return a == b or t < 0.f ? (p - a).Mag() :
                       t > 1.f ? (p - b).Mag() :
                                 (p - (a + (b - a) * t)).Mag();
    }
  };

  //-------------//
  //Bezier Curves//
  //-------------//

  ///Represents a cubic-spline Bezier curve by its polynomial coefficients.
  class Bezier
  {
    public:

    /*Coefficients describing the curve. The curve is defined as a polynomial
    for each dimension:
      f(t){x, y} = {A*t^3 + B*t^2 + C*t + D, E*t^3 + F*t^2 + G*t + H}
    */

    number a;
    number b;
    number c;
    number d;
    number e;
    number f;
    number g;
    number h;

    /**Normally a Bezier curve is defined between the points 0 <= t <= 1. Of
    course the actual curve is valid at all t. Trim allows you to select a new
    finite domain, so that a different selection of the curve can
    be transposed to the effective domain between zero and one. The
    selection can be greater than or less than the original domain.*/
    void Trim(number tZero, number tOne)
    {
      //Compute the new range.
      number newDelta = tOne - tZero;
      number a1 = newDelta;
      number b1 = tZero;

      /*Substituting in a function f(x) = a * x + b into the parametric curve,
      solves for the new constants. This is a generalization of de Casteljau's
      algorithm that bisects the curve.*/
      number a2 = a1 * a1;
      number a3 = a2 * a1;

      number b2 = b1 * b1;
      number b3 = b2 * b1;

      number newA = a3 * a;
      number newB = 3.0f * a2 * b1 * a + a2 * b;
      number newC = 3.0f * a1 * b2 * a + 2.0f * a1 * b1 * b + a1 * c;
      number newD = b3 * a + b2 * b + b1 * c + d;

      number newE = a3 * e;
      number newF = 3.0f * a2 * b1 * e + a2 * f;
      number newG = 3.0f * a1 * b2 * e + 2.0f * a1 * b1 * f + a1 * g;
      number newH = b3 * e + b2 * f + b1 * g + h;

      //Make the assignments.
      a = newA;
      b = newB;
      c = newC;
      d = newD;

      e = newE;
      f = newF;
      g = newG;
      h = newH;
    }

    /**Creates a cubic Bezier from the requested control points. The curve
    begins at p0 and ends at p3. The points p1 and p2 are intermediate points
    of influence which typically do not lie on curve.*/
    void SetControlPoints(Vector p0, Vector p1, Vector p2, Vector p3)
    {
      a = p3.x - p0.x + 3.f * (p1.x - p2.x);
      b = (p2.x - 2.f * p1.x + p0.x) * 3.f;
      c = (p1.x - p0.x) * 3.f;
      d = p0.x;
      e = p3.y - p0.y + 3.f * (p1.y - p2.y);
      f = (p2.y - 2.f * p1.y + p0.y) * 3.f;
      g = (p1.y - p0.y) * 3.f;
      h = p0.y;
    }

    /**Creates a cubic Bezier from two anchors and just one control point. The
    curve begins at p0 and ends at p2. The point p1 is an intermediate point of
    influence that typically does not lie on the curve.*/
    void SetControlPoints(Vector p0, Vector p1, Vector p2)
    {
      a = 0;
      b = p2.x - 2.f * p1.x + p0.x;
      c = (p1.x - p0.x) * 2.f;
      d = p0.x;
      e = 0;
      f = p2.y - 2.f * p1.y + p0.y;
      g = (p1.y - p0.y) * 2.f;
      h = p0.y;
    }

    ///Returns the control points of the curve.
    void GetControlPoints(Vector& p0, Vector& p1, Vector& p2, Vector& p3)
    {
      p0.x = d;
      p1.x = d + c / 3.f;
      p2.x = d + number(2) / number(3) * c + b / 3.f;
      p3.x = d + c + b + a;
      p0.y = h;
      p1.y = h + g / 3.f;
      p2.y = h + number(2) / number(3) * g + f / 3.f;
      p3.y = h + g + f + e;
    }

    /**Attempts to return a bezier curve parallel to this one. The start and
    end control points may not be the same.*/
    Bezier MakeQuasiParallelCurve(number Displacement)
    {
      Vector p0, p1, p2, p3;
      Vector q0, q1, q2, q3;
      GetControlPoints(p0, p1, p2, p3);
      number StartNormalAngle = (p1 - p0).Ang() + HalfPi<number>();
      number EndNormalAngle = (p3 - p2).Ang() + HalfPi<number>();
      Vector StartNormalVector, EndNormalVector;
      StartNormalVector.Polar(StartNormalAngle);
      EndNormalVector.Polar(EndNormalAngle);
      q0 = p0 + StartNormalVector * Displacement;
      q3 = p3 + EndNormalVector * Displacement;
      number Scale = Line(q0, q3).Length() / Line(p0, p3).Length();
      q1 = q0 + (p1 - p0) * Scale;
      q2 = q3 + (p2 - p3) * Scale;
      Bezier bezier;
      bezier.SetControlPoints(q0, q1, q2, q3);
      return bezier;
    }

    ///Returns the coordinate of the cubic spline for some t.
    Vector Value(number t)
    {
      Vector v;

      number t2 = t * t;
      number t3 = t * t2;

      v.x = a * t3 + b * t2 + c * t + d;
      v.y = e * t3 + f * t2 + g * t + h;

      return v;
    }

    /**Finds the curve's vertical tangents by computing the derivative of the
    x-polynomial and finding the zeroes.*/
    count VerticalTangents(number& Root1, number& Root2)
    {
      number a_ = 3.f * a;
      number b_ = 2.f * b;
      number c_ = Bezier::c;

      count NumberOfRoots = Roots(a_, b_, c_, Root1, Root2);

      //Assure that we only get roots in the 0 to 1 range of the Bezier curve.
      if(NumberOfRoots == 2)
      {
        if(Root1 >= 0.f and Root1 <= 1.f)
        {
          if(Root2 >= 0.f and Root2 <= 1.f)
            return 2;
          else
          {
            Root2 = 0.f;
            return 1;
          }
        }
        else if(Root2 >= 0.f and Root2 <= 1.f)
        {
          Root1 = Root2;
          Root2 = 0.f;
          return 1;
        }
        else
        {
          Root1 = Root2 = 0.f;
          return 0;
        }
      }
      else if(NumberOfRoots == 1)
      {
        if(Root1 >= 0.f and Root1 <= 1.f)
          return 1;
        else
        {
          Root1 = 0.f;
          return 0;
        }
      }
      return 0;
    }

    /**Finds the curve's horizontal tangents by computing the derivative of the
    y-polynomial and finding the zeroes.*/
    count HorizontalTangents(number& Root1, number& Root2)
    {
      number a_ = 3.f * Bezier::e;
      number b_ = 2.f * Bezier::f;
      number c_ = Bezier::g;

      count NumberOfRoots = Roots(a_, b_, c_, Root1, Root2);

      //Assure that we only get roots in the 0 to 1 range of the Bezier curve.
      if(NumberOfRoots == 2)
      {
        if(Root1 >= 0.f and Root1 <= 1.f)
        {
          if(Root2 >= 0.f and Root2 <= 1.f)
            return 2;
          else
          {
            Root2 = 0.f;
            return 1;
          }
        }
        else if(Root2 >= 0.f and Root2 <= 1.f)
        {
          Root1 = Root2;
          Root2 = 0.f;
          return 1;
        }
        else
        {
          Root1 = Root2 = 0.f;
          return 0;
        }
      }
      else if(NumberOfRoots == 1)
      {
        if(Root1 >= 0.f and Root1 <= 1.f)
          return 1;
        else
        {
          Root1 = 0.f;
          return 0;
        }
      }
      return 0;
    }

    /**Fits a Bezier curve to a quadrant of an ellipse. Just four cubic splines
    can approximate any ellipse to an error of just one part per thousand,
    which is really convenient for drawing circles that just need to look like
    circles.*/
    void Ellipse(Vector Origin, Vector Scale, number Rotation,
      count Quadrant, bool IsCounterClockwise)
    {
      /*This is the Bezier circle constant which is the distance the
      perpendicular influence points must be from anchors in order to produce a
      curve that is most like a circle or ellipse arc.*/
      const number BezierCircleConstant = BezierCircle<number>();

      number Width = Scale.x * 0.5f;
      number Height = Scale.y * 0.5f;

      Vector p0;
      Vector p1;
      Vector p2;
      Vector p3;

      Quadrant = count(Mod(Quadrant, count(4)));
      if(Quadrant == 0)
        Quadrant = 4;

      if(Quadrant == 3 or Quadrant == 4)
      {
        Rotation += Pi<number>();
        Quadrant -= 2;
      }

      if(Quadrant == 1)
      {
        p0.Polar(Rotation, Width);
        p3.Polar(Rotation + HalfPi<number>(), Height);

        p1.Polar(Rotation + HalfPi<number>(), Height * BezierCircleConstant);
        p1 += p0;

        p2.Polar(Rotation, Width * BezierCircleConstant);
        p2 += p3;
      }
      else if(Quadrant == 2)
      {
        p0.Polar(Rotation + HalfPi<number>(), Height);
        p3.Polar(Rotation + Pi<number>(), Width);

        p1.Polar(Rotation + Pi<number>(), Width * BezierCircleConstant);
        p1 += p0;

        p2.Polar(Rotation + HalfPi<number>(), Height * BezierCircleConstant);
        p2 += p3;
      }

      p0 += Origin;
      p1 += Origin;
      p2 += Origin;
      p3 += Origin;

      if(IsCounterClockwise)
        SetControlPoints(p0, p1, p2, p3);
      else
        SetControlPoints(p3, p2, p1, p0);
    }

    ///Finds a y-root using a simple iterative bisection method.
    number FindSimpleYRoot(number tBisectStart = 0,
      number tBisectEnd = 1.f)
    {
      count i = 0;
      const number Tolerance = number(0.00001);
      const count MaximumSteps = 100;
      while(tBisectEnd - tBisectStart > Tolerance and i < MaximumSteps)
      {
        i++;
        number v0 = Value(tBisectStart).y;
        number tBisect = (tBisectStart + tBisectEnd) * 0.5f;
        number v1 = Value(tBisect).y;
        number v2 = Value(tBisectEnd).y;
        if((v0 < v1 and v0 <= 0.f and v1 >= 0.f) or
          (v0 >= v1 and v1 <= 0.f and v0 >= 0.f))
            tBisectEnd = tBisect;
        else if((v1 < v2 and v1 <= 0.f and v2 >= 0.f) or
          (v1 >= v2 and v2 <= 0.f and v1 >= 0.f))
            tBisectStart = tBisect;
        else
          return 0.f;
      }
      return (tBisectStart + tBisectEnd) * 0.5f;
    }

    ///Finds the intersection of a line and the curve.
    number FindLineIntersection(Line l)
    {
      number Rotation = -l.Angle();

      /*Rotate and vertically translate the line and curve such that the line
      is horizontal and use Newton's method to solve for the y-root.*/
      Vector p0, p1, p2, p3;
      GetControlPoints(p0, p1, p2, p3);
      l.a.Ang(l.a.Ang() + Rotation);
      l.b.Ang(l.b.Ang() + Rotation);
      p0.Ang(p0.Ang() + Rotation);
      p1.Ang(p1.Ang() + Rotation);
      p2.Ang(p2.Ang() + Rotation);
      p3.Ang(p3.Ang() + Rotation);

      number Translation = -l.a.y;
      p0.y += Translation;
      p1.y += Translation;
      p2.y += Translation;
      p3.y += Translation;

      Bezier b_root; b_root.SetControlPoints(p0, p1, p2, p3);
      return b_root.FindSimpleYRoot();
    }

    ///Splits a Bezier curve into two parts.
    void SplitBezier(Bezier& Left, Bezier& Right)
    {
      Left = *this;
      Right = *this;
      Left.Trim(0, 0.5f);
      Right.Trim(0.5f, 1.0f);
    }

    ///Calculates the arc length of the curve.
    number CalculateArcLength(number Tolerance = 0.001f)
    {
      Vector v0, v1, v2, v3;
      GetControlPoints(v0, v1, v2, v3);
      number PolygonShapeLength = (v0 - v1).Mag() + (v1 - v2).Mag() +
        (v2 - v3).Mag();
      number ChordLength = (v0 - v3).Mag();
      number CurrentError = PolygonShapeLength - ChordLength;
      if(CurrentError > Tolerance)
      {
        Bezier Left, Right;
        SplitBezier(Left, Right);
        return Left.CalculateArcLength(Tolerance) +
          Right.CalculateArcLength(Tolerance);
      }
      else
        return PolygonShapeLength * 0.5f + ChordLength * 0.5f;
    }

    ///Calculates the length of a section of the curve.
    number Length(number Point1 = 1.0f, number Point2 = 0.0f,
      number Tolerance = 0.001f)
    {
      Ascending(Point1, Point2);
      Bezier SubArc = *this; SubArc.Trim(Point1, Point2);
      return SubArc.CalculateArcLength(Tolerance);
    }
  };

  //--------//
  //Ellipses//
  //--------//

  ///Computes information related to rotated ellipses.
  class Ellipse
  {
    public:

    /**Computes the vertical tangent of an ellipse. The inputs are the lengths
    of the semimajor and semiminor axises (in other words the 'horizontal
    radius' and the 'vertical radius').*/
    static inline Vector VerticalTangent(number a, number b, number Rotation)
    {
      //Compute the angle t at which the vertical tangent takes place.
      number cos_theta = Cos(Rotation);
      number sin_theta = Sin(Rotation);

      number a_cos_theta = a * cos_theta;
      number a_sin_theta = a * sin_theta;
      number b_cos_theta = b * cos_theta;
      number b_sin_theta = b * sin_theta;

      number t = ArcTan(-b_sin_theta / a_cos_theta);

      /*Apply the matrix rotation algorithm to determine the Cartesian
      components.*/
      number cos_t = Cos(t);
      number sin_t = Sin(t);

      Vector v(a_cos_theta * cos_t - b_sin_theta * sin_t,
           a_sin_theta * cos_t + b_cos_theta * sin_t);

      return v;
    }

    /**Computes the horizontal tangent of an ellipse. The inputs are lengths of
    the semimajor and semiminor axises (in other words the 'horizontal radius'
    and the 'vertical radius').*/
    static inline Vector HorizontalTangent(number a, number b,
      number Rotation)
    {
      //Compute the angle t at which the horizontal tangent takes place.
      number cos_theta = Cos(Rotation);
      number sin_theta = Sin(Rotation);

      number a_cos_theta = a * cos_theta;
      number a_sin_theta = a * sin_theta;
      number b_cos_theta = b * cos_theta;
      number b_sin_theta = b * sin_theta;

      number t = ArcTan(b_cos_theta / a_sin_theta);

      /*Apply the matrix rotation algorithm to determine the Cartesian
      components.*/
      number cos_t = Cos(t);
      number sin_t = Sin(t);

      Vector v(a_cos_theta * cos_t - b_sin_theta * sin_t,
           a_sin_theta * cos_t + b_cos_theta * sin_t);

      return v;
    }

    /**Finds the intersection of a vertical line and ellipse quadrant one. The
    algorithm converts the ellipse to a spline and uses a binary search to look
    for the intersection.*/
    static number VerticalIntersection(number a, number b,
      number Rotation, number DistanceFromRightVerticalTangent)
    {
      //Aliases used in this method
      number CurrentA = a, CurrentB = b, CurrentTheta = Rotation,
        CurrentResult = 0.f;

      //Simply use the origin as the center.
      Vector o;

      //Scale the ellipse.
      Vector s(CurrentA * 2.f, CurrentB * 2.f);

      //Create a cubic spline based off of the input information.
      Bezier c;
      c.Ellipse(o, s, CurrentTheta, 1, true);

      //Compute the vertical tangent.
      Vector i;
      i = VerticalTangent(CurrentA, CurrentB, CurrentTheta);
      number lx = i.x - DistanceFromRightVerticalTangent;

      //Set the conditions for exiting.
      const number ErrorAcceptability = number(0.001);
      const count MaxSteps = 30;

      //Set the initial search bounds.
      number BoundLeft = 0.f;
      number BoundRight = 1.f;
      number BoundMiddle = 0.f;

      //Do a binary subdivision search to quickly hone in.
      for(count Steps = 0; Steps < MaxSteps; Steps++)
      {
        BoundMiddle = (BoundRight + BoundLeft) / 2.f;
        Vector MiddleValue = c.Value(BoundMiddle);

        number mx = Abs(MiddleValue.x);
        number SearchError = Abs(mx - lx);

        //If the error tolerance is acceptable, leave.
        if(SearchError < ErrorAcceptability)
          break;

        //Depending on which side was closer, adjust the boundaries.
        if(MiddleValue.x < lx)
          BoundRight = BoundMiddle;
        else
          BoundLeft = BoundMiddle;
      }

      //Cache the result.
      CurrentResult = BoundMiddle;

      //Return the result.
      return CurrentResult;
    }

    /**A helper method for retrieving a elliptical cubic spline. The parameters
    are the same as the FromEllipse method of the Bezier class, but the
    advantage here is that a new cubic spline is returned, where as in the
    other method, you must already have a Bezier object declared.*/
    static Bezier ToSpline(Vector& Origin, Vector& Scale,
      number Rotation, count Quadrant, bool IsCounterClockwise)
    {
      Bezier c;
      c.Ellipse(Origin, Scale, Rotation, Quadrant, IsCounterClockwise);
      return c;
    }
  };

  //--------//
  //Polygons//
  //--------//

  ///Array of vertices.
  class PolygonShape : public Array<Vector>
  {
    ///Sortable vector for the three-penny convex hull finding algorithm.
    class RadialSortingVector : public Vector
    {
      public:

      //Cached angle to make sorting faster.
      number Angle;

      bool operator < (const RadialSortingVector& Other) const
      {
        return Angle < Other.Angle;
      }

      bool operator > (const RadialSortingVector& Other) const
      {
        return Angle > Other.Angle;
      }

      bool operator == (const RadialSortingVector& Other) const
      {
        return Limits<number>::IsEqual(Angle, Other.Angle);
      }
    };

    public:

    static number FindPerpendicularDistance(Vector p, Vector p1, Vector p2)
    {
      number Result;
      number Slope;
      number Intercept;
      if(Limits<number>::IsZero(p1.x - p2.x))
        Result = Abs(p.x - p1.x);
      else
      {
        Slope = (p2.y - p1.y) / (p2.x - p1.x);
        Intercept = p1.y - (Slope * p1.x);
        Result = Abs(Slope * p.x - p.y + Intercept) / Sqrt(Power(Slope, 2) + 1);
      }
      return Result;
    }

    static PolygonShape Simplify(PolygonShape Points, number Epsilon)
    {
      //Based on RDP algorithm presented by Marius Karthaus
      //http://karthaus.nl/rdp

      if(Points.n() < 3)
        return Points;

      Vector FirstPoint = Points[0];
      Vector LastPoint = Points[Points.n() - 1];
      count Index = -1;
      number MaxDistance = 0;
      for(count i = 1; i < Points.n() - 1; i++)
      {
        number CurrentDistance = FindPerpendicularDistance(Points[i],
          FirstPoint, LastPoint);
        if(CurrentDistance > MaxDistance)
        {
          MaxDistance = CurrentDistance;
          Index = i;
        }
      }

      PolygonShape Simplified;
      if(MaxDistance > Epsilon)
      {
        PolygonShape Left, Right;
        for(count i = 0; i <= Index; i++)
          Left.Add() = Points[i];
        for(count i = Index; i < Points.n(); i++)
          Right.Add() = Points[i];

        PolygonShape LeftSimplified = Simplify(Left, Epsilon);
        PolygonShape RightSimplified = Simplify(Right, Epsilon);

        for(count i = 0; i < LeftSimplified.n() - 1; i++)
          Simplified.Add() = LeftSimplified[i];
        for(count i = 0; i < RightSimplified.n(); i++)
          Simplified.Add() = RightSimplified[i];
      }
      else
      {
        Simplified.Add() = FirstPoint;
        Simplified.Add() = LastPoint;
      }
      return Simplified;
    }

    ///Clears the polygon.
    void Clear()
    {
      Array<Vector>::Clear();
    }

    ///Determines whether or not this polygon intersects the outline of another.
    bool IntersectsOutline(const PolygonShape& Other, Vector OtherDisplacement =
      Vector(0, 0)) const
    {
      Vector p1, p2, q1, q2;
      count p_n = n(), q_n = Other.n();
      for(count i = 0; i < p_n; i++)
      {
        p1 = ith(i);
        p2 = ith(i ? i - 1 : p_n - 1);
        for(count j = 0; j < q_n; j++)
        {
          q1 = Other.ith(j) + OtherDisplacement;
          q2 = Other.ith(j ? j - 1 : q_n - 1) + OtherDisplacement;
          Line P(p1, p2), Q(q1, q2);
          if(P.Intersects(Q))
            return true;
        }
      }
      return false;
    }

    private:

    //Determine quadrant of polygon point relative to test point.
    static count PIPQuadrant(Vector p, Vector t)
    {
      return p.x > t.x ? (p.y > t.y ? 0 : 3) : (p.y > t.y ? 1 : 2);
    }

    /*Determine x-intercept of a polygon edge with a horizontal line at the
    y-value of the test point.*/
    static number PIPXIntercept(Vector p1, Vector p2, number y)
    {
      return p2.x - ((p2.y - y) * ((p1.x - p2.x) / (p1.y - p2.y)));
    }

    //Adjust quadrant delta
    static count PIPAdjustedDelta(count Delta, Vector a, Vector b, Vector p)
    {
      number xi = PIPXIntercept(a, b, p.y);
      return Delta ==  3 or (Delta ==  2 and xi > p.x) ? Delta - 4 :
             Delta == -3 or (Delta == -2 and xi > p.x) ? Delta + 4 : Delta;
    }

    public:

    //Determines if a test point is inside of the polygon.
    bool PointInside(Vector v) const
    {
      Vector a, b;
      count Angle = 0;
      for(count i = 0; i < n(); i++)
      {
        a = ith(i), b = ith((i + 1) % n());
        Angle += PIPAdjustedDelta(PIPQuadrant(b, v) - PIPQuadrant(a, v),
          a, b, v);
      }

      //Complete 360 degrees (angle of +4 or -4) means inside.
      return Angle == 4 or Angle == -4;
      //return Angle & 4;  //Alternatively: odd number of windings rule.
      //return Angle != 0; //Alternatively: non-zero winding rule.
    }

    /*Finds the minimum distance of the point to the polygon. If the point is
    inside the polygon, then the distance is zero (i.e. considers polygon to be
    filled).*/
    number MinimumDistance(Vector v) const
    {
      number m = 0.f;
      for(count i = 0; i < n(); i++)
      {
        number d = Line(ith(i), ith((i + 1) % n())).MinimumDistance(v);
        if(d < m or not (m > 0.f))
          m = d;
      }
      return PointInside(v) ? 0.f : m;
    }

    /**Determines the order of three points. The result can be as clockwise (1),
    counterclockwise (-1), or collinear (0).*/
    static inline count DeterminePointOrder(Vector p0, Vector p1, Vector p2)
    {
      number p1x_p0x = p1.x - p0.x;
      number p2x_p0x = p2.x - p0.x;
      number p1y_p0y = p1.y - p0.y;
      number p2y_p0y = p2.y - p0.y;

      number Left  = p2y_p0y * p1x_p0x;
      number Right = p1y_p0y * p2x_p0x;

      /*This would be better if the above values were normalized so that the
      range was more predictable. #magicnumber*/
      const number Tolerance = 1.0e-6f;

      if(Abs(Left - Right) < Tolerance)
        return 0;
      else if(Left < Right)
        return 1;
      else
        return -1;
    }

    ///Calculates the perimeter of the polygon.
    number Perimeter() const
    {
      number Sum = 0.0;
      for(count i = 0; i < n() - 1; i++)
        Sum += Square(ith(i).x - ith(i + 1).x) +
          Square(ith(i).y - ith(i + 1).y);
      Sum += Square(ith(0).x - ith(n() - 1).x) +
        Square(ith(0).y - ith(n() - 1).y);
      return Sqrt(Sum);
    }

    /**Creates a new polygon consisting of the convex hull of this polygon. The
    algorithm employed is the three-penny convex hull finder.*/
    void CreateConvexHull(PolygonShape& NewPolygonShape) const
    {
      //Make sure there are at least three points to work with.
      if(n() < 3)
      {
        if(n())
          NewPolygonShape = *this;
        else
          NewPolygonShape.Clear();
        return;
      }

      //Copy polygon array into temporary list.
      Sortable::List<RadialSortingVector> L;
      {
        //First find left-most point.
        count this_n = n();
        count LeftMostIndex = 0;
        number LeftMostX = a().x;
        for(count i = 1; i < this_n; i++)
        {
          number CurrentX = ith(i).x;
          if(CurrentX < LeftMostX)
          {
            LeftMostIndex = i;
            LeftMostX = CurrentX;
          }
        }

        /*Now add the points with the left-most point first. This guarantees
        that the first three points in the list will be convex.*/
        for(count i = LeftMostIndex; i < this_n; i++)
          static_cast<Vector&>(L.Add()) = ith(i);

        for(count i = 0; i <= LeftMostIndex; i++) //Use <= adds redundant point.
          static_cast<Vector&>(L.Add()) = ith(i);

        ///Determine the beginning point order.
        Vector L_0 = L[0];
        while(DeterminePointOrder(L_0, L[1], L[2]) == 0)
          L.Remove(1);

        //Determine the angles of the points so that they can be sorted.
        RadialSortingVector v = L[0];
        count L_n_1 = L.n() - 1;
        for(count i = 1; i < L_n_1; i++)
        {
          number Angle = v.Ang(L[i]);
          if(Angle > Pi<number>())
            Angle -= TwoPi<number>();
          L[i].Angle = Angle;
        }

        //Force the first point to be first in the list after the sort.
        L[0].Angle = -Pi<number>();

        //Force the last point to be last in the list after the sort.
        L[L_n_1].Angle = Pi<number>();

        //Sort the vector list radially.
        L.Quicksort();
      }

      //Apply the three-penny convex hull algorithm.
      for(count i = 0; i < L.n() - 2; /*Note: conditional increment*/)
      {
        Vector p0 = L[i], p1 = L[i + 1], p2 = L[i + 2];
        if(DeterminePointOrder(p0, p1, p2) != -1)
        {
          //Remove the middle point since since it causes a concavity.
          L.Remove(i + 1);

          /*Since the point at i + 2 is now the point at i + 1, the algorithm
          needs to back up to the sequence at [i - 1, i, i + 1] or the sequence
          [0, 1, 2], whichever is greater.*/
          if(i > 0)
            i--;
        }
        else
          i++;
      }

      //Copy results into new polygon.
      {
        NewPolygonShape.Clear();
        count L_n = L.n() - 1; //Remove redundant tail point.
        NewPolygonShape.n(L_n);
        for(count i = 0; i < L_n; i++)
          NewPolygonShape[i] = L[i];
      }
    }
  };
}
#endif
