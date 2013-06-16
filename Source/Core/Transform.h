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
    prim::number a; //   Matrix
    prim::number b; // .        .
    prim::number c; // | a  b  0|
    prim::number d; // | c  d  0|
    prim::number e; // | e  f  1|
    prim::number f; // '        '

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
    Affine(prim::number a, prim::number b, prim::number c,
      prim::number d, prim::number e, prim::number f) :
      a(a), b(b), c(c), d(d), e(e), f(f) {}

    //-----------------------//
    //Transformation Matrices//
    //-----------------------//

    ///Returns an affine matrix consisting of a unit transformation (in = out).
    Affine static Unit()
    {
      return Affine();
    }

    ///Returns an affine matrix consisting of a translation.
    Affine static Translate(prim::planar::Vector TranslateBy)
    {
      return Affine(1, 0, 0, 1, TranslateBy.x, TranslateBy.y);
    }

    ///Returns an affine matrix consisting of a linear scale operation.
    Affine static Scale(prim::number ScaleBy)
    {
      return Affine(ScaleBy, 0, 0, ScaleBy, 0, 0);
    }

    ///Returns an affine matrix consisting of a vector scale operation.
    Affine static Scale(prim::planar::Vector ScaleBy)
    {
      return Affine(ScaleBy.x, 0, 0, ScaleBy.y, 0, 0);
    }

    ///Returns an affine matrix consisting of a rotation operation in radians.
    Affine static Rotate(prim::number RotateBy)
    {
      prim::number CosT = prim::Cos(RotateBy);
      prim::number SinT = prim::Sin(RotateBy);

      return Affine(CosT, SinT, -SinT, CosT, 0, 0);
    }
    
    ///Returns a translated, scale, and rotated (in that order) affine matrix.
    Affine static TranslateScaleRotate(prim::planar::Vector TranslateBy,
      prim::number ScaleBy = (prim::number)1.0,
      prim::number RotateBy = (prim::number)0.0)
    {
      return Translate(TranslateBy) * Scale(ScaleBy) * Rotate(RotateBy);
    }
    
    ///Returns a translated, scale, and rotated (in that order) affine matrix.
    Affine static TranslateScaleRotate(prim::planar::Vector TranslateBy,
      prim::planar::Vector ScaleBy, prim::number RotateBy = (prim::number)0.0)
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
      prim::number this_a = a, this_b = b, this_c = c;
      prim::number this_d = d, this_e = e, this_f = f;

      //Read in other matrix to local variables.
      prim::number Other_a = Other.a, Other_b = Other.b, Other_c = Other.c;
      prim::number Other_d = Other.d, Other_e = Other.e, Other_f = Other.f;

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
      prim::number Determinant = CalculateDeterminant();

      //Make sure the matrix is invertible. If not return a null matrix.
      if(!Determinant)
        return Affine(0, 0, 0, 0, 0, 0);

      //Precalculate the determinant inverse.
      prim::number DeterminantInverse = (prim::number)1.0 / Determinant;

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
      return a == Other.a && b == Other.b && c == Other.c &&
        d == Other.d && e == Other.e && f == Other.f;
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
    prim::planar::Vector operator << (
      const prim::planar::Vector& Untransformed) const
    {
      prim::number x = Untransformed.x;
      prim::number y = Untransformed.y;

      return prim::planar::Vector(a * x + c * y + e, b * x + d * y + f);
    }

    ///Untransforms a vector point using the matrix.
    prim::planar::Vector operator >> (
      const prim::planar::Vector& Transformed) const
    {
      return -(*this) << Transformed;
    }
    
    //----------//
    //Properties//
    //----------//
    
    inline prim::number CalculateDeterminant() const
    {
      return a * d - b * c;
    }
    
    inline bool IsInvertible() const
    {
      //If the determinant is zero, the matrix is not invertible.
      return CalculateDeterminant() != (prim::number)0.0;
    }
    
    /**Determines if there is a rotation that is not a multiple of 90 degrees.
    Assumes that the matrix does not have a skewing operation. The detection of
    rotation is approximate due to floating point error, so the method is not
    exact.*/
    bool IsRotated(prim::number Delta = 1.0e-12) const
    {
      prim::number Normalized_b = prim::Abs(b) / 
        prim::Sqrt(prim::Abs(CalculateDeterminant()));
      return !(prim::Chop(Normalized_b, Delta) == 0.0 ||
        prim::Chop(Normalized_b - 1.0f, Delta) == 0.0);
    }
    
    ///Gets the matrix of affine transformation.
    prim::Matrix<prim::number> Matrix() const
    {
      prim::Matrix<prim::number> M(3, 3);
      M(0, 0) = a;   M(0, 1) = c;   M(0, 2) = e;
      M(1, 0) = b;   M(1, 1) = d;   M(1, 2) = f;
      M(2, 0) = 0.0; M(2, 1) = 0.0; M(2, 2) = 1.0;
      return M;      
    }
  };
  
  ///Use List::Push() and List::Pop() to add and remove transformations.
  struct AffineStack : public prim::List<Affine>
  {
    /**Collapses a range of affine matrices into a single matrix. This method
    effectively translates one space into another. Often Forwards() or
    Backwards() are useful shortcuts to translate between the whole set of
    spaces.*/
    Affine Collapse(prim::count Start, prim::count End) const
    {
      //Fix bound problems as necessary.
      Start = prim::Min(prim::Max(Start,
        (prim::count)0), n() - (prim::count)1);
      End = prim::Min(prim::Max(End,
        (prim::count)0), n() - (prim::count)1);

      //Begin with identity matrix.
      Affine M;

      if(Start < End)
      {
        //Traverse forward and compute matrix.
        for(prim::count i = Start + 1; i <= End; i++)
          M *= ith(i);
      }
      else if(End < Start)
      {
        //Traverse backwards and compute matrix.
        for(prim::count i = Start; i >= End + 1; i--)
          M /= ith(i);
      }

      return M;
    };

    ///Creates a matrix to transform from the beginning space to the end space.
    Affine Forwards() const
    {
      return Collapse(0, n() - 1);
    }

    ///Creates a matrix to transform from the end space to the beginning space.
    Affine Backwards() const
    {
#if 1
      return -Forwards();
#else
      //Alternatively:
      return Collapse(n() - 1, 0);
#endif
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
