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

#ifndef PRIM_INCLUDE_COMPLEX_H
#define PRIM_INCLUDE_COMPLEX_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///Stores a complex number (or a point on a plane).
  template <class T>
  class Complex
  {
    public:

    ///Stores the value for the real part (or x-coordinate).
    T x;

    ///Stores the value for the imaginary part (or y-coordinate).
    T y;

    //-------//
    //Aliases//
    //-------//

    ///Alias for x-coordinate
    inline T& i() {return x;}

    ///Alias for y-coordinate
    inline T& j() {return y;}

    ///Alias for x-coordinate
    inline T& a() {return x;}

    ///Alias for y-coordinate
    inline T& b() {return y;}

    //-------------//
    //Const Aliases//
    //-------------//

    ///Alias for x-coordinate
    inline const T& i() const {return x;}

    ///Alias for y-coordinate
    inline const T& j() const {return y;}

    ///Alias for x-coordinate
    inline const T& a() const {return x;}

    ///Alias for y-coordinate
    inline const T& b() const {return y;}

    //---------//
    //Emptiness//
    //---------//

    ///Returns an empty complex number (set to NaN).
    static inline Complex<T> Empty()
    {
      return Complex<T>(Limits<T>::QuietNaN(), Limits<T>::QuietNaN());
    }

    ///Returns whether the complex number is empty.
    inline bool IsEmpty() const
    {
      return Limits<T>::IsNaN(x) or Limits<T>::IsNaN(y);
    }

    //---------------------------//
    //Cartesian-Polar Conversions//
    //---------------------------//

    /**Stores the cartesian equivalent of the given angle and magnitude. The
    units are in radians. Note that specifying a negative magnitude will cause
    the angle to increase by Pi. This method loses less precision than by
    calling Mag and Ang separately.*/
    inline void Polar(T Angle, T Magnitude = 1.f)
    {
      x = T(Cos(Angle) * Magnitude);
      y = T(Sin(Angle) * Magnitude);
    }

    ///Returns the magnitude (distance from the origin) of the x-y pair.
    inline T Mag() const
    {
      return Abs(T(x), T(y));
    }

    ///Changes the magnitude keeping the angle the same.
    inline void Mag(T Magnitude)
    {
      T Angle = Ang();
      Polar(Angle, Magnitude);
    }

    ///Computes dot product of this and another vector.
    inline T Dot(Complex<T> Other)
    {
      return x * Other.x + y * Other.y;
    }

    ///Returns the angle of the x-y pair relative to the origin.
    inline T Ang() const
    {
      return Angle(x, y);
    }

    ///Changes the angle of the x-y pair keeping the magnitude the same.
    inline void Ang(T Angle)
    {
      T Magnitude = Mag();
      Polar(Angle, Magnitude);
    }

    ///Returns the magnitude distance to another x-y pair.
    inline T Mag(Complex<T> Other) const
    {
      return Distance(T(x), T(y), T(Other.x), T(Other.y));
    }

    ///Returns the angle between this x-y pair and another.
    inline T Ang(Complex<T> Other) const
    {
      Complex<T> Delta(Other.x - x, Other.y - y);
      return Delta.Ang();
    }

    //--------------//
    //Multiplication//
    //--------------//

    ///Multiplies by a scalar and returns the result.
    inline Complex operator * (T Scalar) const
    {
      return Complex(x * Scalar, y * Scalar);
    }

    /**Complex multiplies and returns the result. The formula is: (a + bi)(c
    + di) = (ac - bd) + (bc + ad)i.*/
    inline Complex operator * (Complex Other) const
    {
      return Complex(x * Other.x - y * Other.y, x * Other.y + y * Other.x);
    }

    ///Multiplies by another number, then stores and returns result.
    inline Complex operator *= (T Scalar)
    {
      return *this = *this * Scalar;
    }

    ///Complex multiplies, and then stores and returns result.
    inline Complex operator *= (Complex Other)
    {
      return *this = *this * Other;
    }

    //--------//
    //Division//
    //--------//

    ///Divides by a number and returns the result.
    inline Complex operator / (T Divisor) const
    {
      return Complex(x / Divisor, y / Divisor);
    }

    /**Complex divides and returns the result. The formula is: (a + bi)/(c
    + di) = ((ac + bd) + (bc - ad)i) / (c^2 + d^2).*/
    inline Complex operator / (Complex Other) const
    {
      T Divisor = (Square(Other.x) + Square(Other.y));

      return Complex((x * Other.x + y * Other.y) / Divisor,
        (x * Other.y - y * Other.x) / Divisor);
    }

    ///Divides by another number, and then stores and returns result.
    inline Complex operator /= (T Divisor)
    {
      return *this = *this / Divisor;
    }

    ///Complex divides, and then stores and returns result.
    inline Complex operator /= (Complex Other)
    {
      return *this = *this / Other;
    }

    //------------------------//
    //Addition and Subtraction//
    //------------------------//

    ///Adds another x-y pair to this one and returns the result.
    inline Complex operator + (Complex Other) const
    {
      return Complex(x + Other.x, y + Other.y);
    }

    ///Substracts another x-y pair to this one and returns the result.
    inline Complex operator - (Complex Other) const
    {
      return Complex(x - Other.x, y - Other.y);
    }

    ///Adds another x-y pair to this one and stores the result.
    inline Complex operator += (Complex Other)
    {
      x += Other.x;
      y += Other.y;
      return *this;
    }

    ///Substracts another x-y pair to this one and stores the result.
    inline Complex operator -= (Complex Other)
    {
      x -= Other.x;
      y -= Other.y;
      return *this;
    }

    //-----//
    //Unary//
    //-----//

    ///Returns a negated x-y pair.
    inline Complex operator - () const {return Complex(-x, -y);}

    ///Returns the x-y pair.
    inline Complex operator + () const {return Complex(x, y);}

    //----------//
    //Assignment//
    //----------//

    ///Assignment operator copies another x-y pair into this one.
    inline Complex operator = (Complex Other)
    {
      x = Other.x;
      y = Other.y;
      return *this;
    }

    //-----------//
    //Equivalence//
    //-----------//

    ///Equivalence comparison operator.
    inline bool operator == (Complex Other) const
    {
      return Limits<T>::IsEqual(x, Other.x) and
        Limits<T>::IsEqual(y, Other.y);
    }

    ///Non-equivalence comparison operator.
    inline bool operator != (Complex Other) const
    {
      return Limits<T>::IsNotEqual(x, Other.x) or
        Limits<T>::IsNotEqual(y, Other.y);
    }

    //------------//
    //Constructors//
    //------------//

    ///Creates a complex number with a real and imaginary part (or coordinate).
    Complex(T x_, T y_) : x(x_), y(y_) {}

    ///Copy constructor copies another complex number to this one.
    Complex(const Complex& Other)
    {
      x = Other.x;
      y = Other.y;
    }

    ///Creates a complex number at the origin.
    Complex() : x(T(0)), y(T(0)) {}

    ///Casts this coordinate type into a different coordinate type.
    template <class U>
    operator Complex<U> () const
    {
      return Complex<U>(U(x), U(y));
    }
  };
}
#endif
