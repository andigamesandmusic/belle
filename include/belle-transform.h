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

#ifndef BELLE_CORE_TRANSFORM_H
#define BELLE_CORE_TRANSFORM_H

namespace BELLE_NAMESPACE
{
  /**Stores a planar affine (common) transformation matrix or CTM. The matrix
  stores combinations of translation, scaling, and rotation as a set of six
  values. Matrix multiplication is not commutative, so the order makes a
  difference. The perspective of the matrix is in reference to the object, not
  the camera viewpoint or world. For example, a point (1, 1) scaled by a 2, will
  become (2, 2) and not (0.5, 0.5). Each consecutive operation is done with
  respect to the object axis, so translating (1, 1) by (1, 1) and scaling by 2
  results in (3, 3) whereas scaling and then transforming would lead to (4, 4).
  Generally, the order of translate, scale, and rotate is the most intuitive in
  that it places an object a given location, with a given scale, and a given
  rotation.*/

  struct Affine
  {
    //Coefficients
    number a; //   Matrix
    number b; // .        .
    number c; // | a  b  0|
    number d; // | c  d  0|
    number e; // | e  f  1|
    number f; // '        '

    //------------------//
    //Basic Constructors//
    //------------------//

    ///Creates a default matrix at the origin with a unit scale.
    Affine()
    {
      a = 1; b = 0; c = 0;
      d = 1; e = 0; f = 0;
    }

    ///Constructor via coefficients.
    Affine(number a_, number b_, number c_,
      number d_, number e_, number f_) :
      a(a_), b(b_), c(c_), d(d_), e(e_), f(f_) {}

    //------//
    //Output//
    //------//

    operator String () const
    {
      String s;
      s >> "[" << a << ", " << b << ", " << c << ", " << d << ", " << e <<
        ", " << f << "]";
      return s;
    }

    //-----------------------//
    //Transformation Matrices//
    //-----------------------//

    ///Returns an affine matrix consisting of a unit transformation (in = out).
    Affine static Unit()
    {
      return Affine();
    }

    ///Returns an affine matrix consisting of a translation.
    Affine static Translate(Vector TranslateBy)
    {
      return Affine(1, 0, 0, 1, TranslateBy.x, TranslateBy.y);
    }

    ///Returns an affine matrix consisting of a linear scale operation.
    Affine static Scale(number ScaleBy)
    {
      return Affine(ScaleBy, 0, 0, ScaleBy, 0, 0);
    }

    ///Returns an affine matrix consisting of a vector scale operation.
    Affine static Scale(Vector ScaleBy)
    {
      return Affine(ScaleBy.x, 0, 0, ScaleBy.y, 0, 0);
    }

    ///Returns an affine matrix consisting of a rotation operation in radians.
    Affine static Rotate(number RotateBy)
    {
      number CosT = Cos(RotateBy);
      number SinT = Sin(RotateBy);

      return Affine(CosT, SinT, -SinT, CosT, 0, 0);
    }

    ///Returns a translated, scale, and rotated (in that order) affine matrix.
    Affine static TranslateScaleRotate(Vector TranslateBy,
      number ScaleBy = 1.f, number RotateBy = 0.f)
    {
      return Translate(TranslateBy) * Scale(ScaleBy) * Rotate(RotateBy);
    }

    ///Returns a translated, scale, and rotated (in that order) affine matrix.
    Affine static TranslateScaleRotate(Vector TranslateBy,
      Vector ScaleBy, number RotateBy = 0.f)
    {
      return Translate(TranslateBy) * Scale(ScaleBy) * Rotate(RotateBy);
    }

    //----------------------------//
    //Multiplication and Inversion//
    //----------------------------//

    /**Multiplies this transform with another and returns the result. Note
    that the given matrix is premultiplied with the current one. In other
    words New X Old (matrix multiplication is not commutative).*/
    Affine operator * (const Affine& Other) const
    {
      //Read in this matrix to local variables.
      number this_a = a, this_b = b, this_c = c;
      number this_d = d, this_e = e, this_f = f;

      //Read in other matrix to local variables.
      number Other_a = Other.a, Other_b = Other.b, Other_c = Other.c;
      number Other_d = Other.d, Other_e = Other.e, Other_f = Other.f;

      //Do matrix multiplication.
      Affine M;

      M.a = Other_a * this_a + Other_b * this_c;
      M.b = Other_a * this_b + Other_b * this_d;

      M.c = Other_c * this_a + Other_d * this_c;
      M.d = Other_c * this_b + Other_d * this_d;

      M.e = Other_e * this_a + Other_f * this_c + this_e;
      M.f = Other_e * this_b + Other_f * this_d + this_f;

      return M;
    }

    /**Calculates the inverse of the transform (unary minus). If the matrix is
    not invertible, then a zero matrix is returned.*/
    Affine operator - () const
    {
      /*
      Matrix Inversion Algorithm:
      .         .
      | a' b' 0 |                |   d        -b      0  |
      | c' d' 0 |  =   1 / DET * |  -c         a      0  |
      | e' f' 1 |                |cf - de  -af + be  DET |
      '         '
         with DET  =  a * d - c * b
      */

      //Calculate the determinant.
      number Determinant = CalculateDeterminant();

      //Make sure the matrix is invertible. If not return a null matrix.
      if(Limits<number>::IsZero(Determinant))
        return Affine(0, 0, 0, 0, 0, 0);

      //Precalculate the determinant inverse.
      number DeterminantInverse = 1.f / Determinant;

      //Do the matrix inversion.
      Affine M;

      M.a = d * DeterminantInverse;
      M.b = -b * DeterminantInverse;

      M.c = -c * DeterminantInverse;
      M.d = a * DeterminantInverse;

#if 1
      M.e = (c * f - d * e) * DeterminantInverse;
      M.f = (-a * f + b * e) * DeterminantInverse;
#else
      //Alternative optimization using precalculated inverse determinant:
      M.e = -M.c * f  - M.a * e;
      M.f = -M.d * f  - M.b * e;
#endif

      return M;
    }

    ///Stores the multiplication of the left matrix with the right.
    void operator *= (const Affine& Other)
    {
      *this = *this * Other;
    }

    ///Returns the left matrix multiplied by the inverse of right.
    Affine operator / (const Affine& Other) const
    {
      return *this * -Other;
    }

    ///Stores the left matrix multiplied by the inverse of right.
    void operator /= (const Affine& Other)
    {
      *this = *this * -Other;
    }

    //----------//
    //Comparison//
    //----------//

    ///Returns whether the transform exactly matches another.
    bool operator == (const Affine& Other) const
    {
      return Limits<number>::IsEqual(a, Other.a) &&
             Limits<number>::IsEqual(b, Other.b) &&
             Limits<number>::IsEqual(c, Other.c) &&
             Limits<number>::IsEqual(d, Other.d) &&
             Limits<number>::IsEqual(e, Other.e) &&
             Limits<number>::IsEqual(f, Other.f);
    }

    ///Returns whether the transform does not exactly match another.
    bool operator != (const Affine& Other) const
    {
      return !(*this == Other);
    }

    //--------------------//
    //Point Transformation//
    //--------------------//

    ///Transforms a vector point using the matrix.
    Vector operator << (const Vector& Untransformed) const
    {
      number x = Untransformed.x;
      number y = Untransformed.y;

      return Vector(a * x + c * y + e, b * x + d * y + f);
    }

    ///Untransforms a vector point using the matrix.
    Vector operator >> (const Vector& Transformed) const
    {
      return -(*this) << Transformed;
    }

    ///Transforms a rectangle using the matrix.
    Box operator << (const Box& Untransformed) const
    {
      Box Transformed;
      Transformed += *this << Untransformed.BottomLeft();
      Transformed += *this << Untransformed.TopLeft();
      Transformed += *this << Untransformed.TopRight();
      Transformed += *this << Untransformed.BottomRight();
      Transformed.Order();
      return Transformed;
    }

    ///Untransforms a rectangle using the matrix.
    Box operator >> (const Box& Transformed) const
    {
      return -(*this) << Transformed;
    }

    //----------//
    //Properties//
    //----------//

    ///Returns the determinant of the matrix.
    number CalculateDeterminant() const
    {
      return a * d - b * c;
    }

    ///Returns whether the matrix is invertible.
    bool IsInvertible() const
    {
      number Determinant = CalculateDeterminant();
      return Limits<number>::Bounded(a) and
             Limits<number>::Bounded(b) and
             Limits<number>::Bounded(c) and
             Limits<number>::Bounded(d) and
             Limits<number>::Bounded(e) and
             Limits<number>::Bounded(f) and
             Limits<number>::Bounded(Determinant) and
             Limits<number>::IsNotZero(Determinant);
    }

    /**Determines if there is a rotation that is not a multiple of 90 degrees.
    Assumes that the matrix does not have a skewing operation. The detection of
    rotation is approximate due to floating point error, so the method is not
    exact.*/
    bool IsRotated(number Delta = 1.0e-12f) const
    {
      number Normalized_b = Abs(b) /
        Sqrt(Abs(CalculateDeterminant()));
      return !(Chop(Normalized_b, Delta) == 0.0 ||
        Chop(Normalized_b - 1.0f, Delta) == 0.0);
    }

    ///Gets the matrix of affine transformation.
    BELLE_NAMESPACE::Matrix<number> Matrix() const
    {
      BELLE_NAMESPACE::Matrix<number> M(3, 3);
      M(0, 0) = a;   M(0, 1) = c;   M(0, 2) = e;
      M(1, 0) = b;   M(1, 1) = d;   M(1, 2) = f;
      M(2, 0) = 0.0; M(2, 1) = 0.0; M(2, 2) = 1.0;
      return M;
    }
  };

  ///Use List::Push() and List::Pop() to add and remove transformations.
  class AffineStack : public List<Affine>
  {
    public:

    /**Collapses a range of affine matrices into a single matrix. This method
    effectively translates one space into another. Often Forwards() or
    Backwards() are useful shortcuts to translate between the whole set of
    spaces.*/
    Affine Collapse(count Start, count End) const
    {
      //Fix bound problems as necessary.
      Start = Min(Max(Start,
        count(0)), n() - count(1));
      End = Min(Max(End,
        count(0)), n() - count(1));

      //Begin with identity matrix.
      Affine M;

      if(Start < End)
      {
        //Traverse forward and compute matrix.
        for(count i = Start + 1; i <= End; i++)
          M *= ith(i);
      }
      else if(End < Start)
      {
        //Traverse backwards and compute matrix.
        for(count i = Start; i >= End + 1; i--)
          M /= ith(i);
      }

      return M;
    }

    ///Creates a matrix to transform from the beginning space to the end space.
    Affine Forwards() const
    {
      return Collapse(0, n() - 1);
    }

    ///Creates a matrix to transform from the end space to the beginning space.
    Affine Backwards() const
    {
      return -Forwards();
    }

    ///Default constructor begins with an identity matrix.
    AffineStack()
    {
      //Push identity matrix to the stack.
      Push(Affine::Unit());
    }
  };
}
#endif
