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

#ifndef PRIM_INCLUDE_MATH_H
#define PRIM_INCLUDE_MATH_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  //---------//
  //Constants//
  //---------//

  template <class T> T Deg()
  {
    return T(0.017453292519943295769236907684886127134428718885417L);
  }

  template <class T> T E()
  {
    return T(2.7182818284590452353602874713526624977572470937000L);
  }

  template <class T> T Pi()
  {
    return T(3.1415926535897932384626433832795028841971693993751L);
  }

  template <class T> T TwoPi()
  {
    return T(6.2831853071795864769252867665590057683943387987502L);
  }

  template <class T> T HalfPi()
  {
    return T(1.5707963267948966192313216916397514420985846996876L);
  }

  template <class T> T Log2Inv()
  {
    return T(1.4426950408889634073599246810018921374266459541530L);
  }

  /**Returns the best known Bezier circle constant. This constant was calculated
  to minimize the area difference between a Bezier arc and a circle using
  arbitrary precision numerical integration.*/
  template <class T> T BezierCircle()
  {
    return T(0.552009225676999299949201202L);
  }

  //---------------------//
  //Exponential Functions//
  //---------------------//

  ///Returns the exponential of x.
  float32 Exp(float32 x);

  ///Returns the exponential of x.
  float64 Exp(float64 x);

  ///Returns the exponential of x.
  float80 Exp(float80 x);

  ///Returns x to the power y.
  float32 Power(float32 x, float32 y);

  ///Returns x to the power y.
  float64 Power(float64 x, float64 y);

  ///Returns x to the power y.
  float80 Power(float80 x, float80 y);

  ///Returns the natural logarithm of x.
  float32 Log(float32 x);

  ///Returns the natural logarithm of x.
  float64 Log(float64 x);

  ///Returns the natural logarithm of x.
  float80 Log(float80 x);

  ///Returns the logarithm of x to base b.
  float32 Log(float32 b, float32 x);

  ///Returns the logarithm of x to base b.
  float64 Log(float64 b, float64 x);

  ///Returns the logarithm of x to base b.
  float80 Log(float80 b, float80 x);

  ///Returns the base-2 logarithm of x.
  float32 Log2(float32 x);

  ///Returns the base-2 logarithm of x.
  float64 Log2(float64 x);

  ///Returns the base-2 logarithm of x.
  float80 Log2(float80 x);

  ///Returns the base-10 logarithm of x.
  float32 Log10(float32 x);

  ///Returns the base-10 logarithm of x.
  float64 Log10(float64 x);

  ///Returns the base-10 logarithm of x.
  float80 Log10(float80 x);

  ///Returns the square root of x.
  float32 Sqrt(float32 x);

  ///Returns the square root of x.
  float64 Sqrt(float64 x);

  ///Returns the square root of x.
  float80 Sqrt(float80 x);

  //----------------//
  //Bessel Functions//
  //----------------//

  ///Computes the zeroth-order modified Bessel function of the first kind of z.
  template<typename FloatT> FloatT BesselI0(FloatT z)
  {
    /*The BesselI[0, z] function, also known as the zeroth-order modified Bessel
    function of the first kind, is defined by:

            oo                    oo
            __  (z ^ 2 / 4) ^ k   __  / (z / 2) ^ k  \
    I0(z) = \   --------------- = \   | ------------ | ^ 2  =
            /_     (k!) ^ 2       /_  \      k!      /

           k = 0                 k = 0

               oo                              oo
               __  / (z / 2) ^ k  \            __
          1 +  \   | ------------ | ^ 2  = 1 + \   P_z[k] ^ 2, where
               /_  \      k!      /            /_

              k = 1                           k = 1

                                z / 2
          P_z[k] = P_z[k - 1] * -----, P_z[0] = 1
                                  k

    Note that the function is evenly symmetric and I0(0) = 1 is the function
    minimum. The function grows monotonically at an exponential pace. I0(700) is
    above 10^300, and since this is the near the limit of the double floating-
    point arithmetic, the valid domain of this numeric function is [-700, 700].

    The numerical accuracy of the function depends on the value of z chosen, but
    it appears that over the given range 15 decimal digit accuracy is typical in
    double arithmetic. The approach taken is to continue the summation so long
    as it has an effect on the output. When the number added is too small to
    affect the result, the loop exits. Since the formula is well-conditioned,
    there should not be any input in the domain that would not eventually exit.
    Still, in case there is some number that could cause this to occur, a
    conservative value of 1000 summations is used to prevent an infinite loop.
    It was calculated that the maximum number of summations before total
    convergence is 465 in double arithmetic. Note as well that the number of
    summations is essentially proportional to the input z:

    Summations ~= 3 + z * 0.66.

    Note that in the Kaiser window that uses this Bessel function, the domain
    used is [0, beta], where beta is the alpha-pi constant that determines the
    window's sidelobe attenuation. For example, a beta of 20 (i.e., a Kaiser-20
    window) has sidelobe attenuation of about -190 dB.*/

    /*1) The function is evenly symmetric, so to make things simpler, wrap the
    domain so that it is always used positively.*/
    if(z < 0.)
      z = -z;

    //2) Simple case: for z = 0, the result is exactly 1.
    if(Limits<FloatT>::IsZero(z))
      return 1.;

    //Set initial conditions.
    FloatT z_Half = z * 0.5, Result = 1.0, PreviousResult = 0.0,
      Pz_k = 1.0, k = 1.0;

    /*Calculate until the series converges on a single decimal value. This
    happens because at some point a very small number is added to a large number
    and there is no change. This event causes the loop to stop. The loop
    continues so long as the floating-point representation does not change.*/
    while(Limits<FloatT>::IsNotEqual(PreviousResult, Result) and k < 1000.)
    {
      PreviousResult = Result;
      Pz_k = Pz_k * (z_Half / k++);
      Result += Pz_k * Pz_k;
    }
    return Result;
  }

  //-----------------------//
  //Trigonometric Functions//
  //-----------------------//

  ///Returns the sine of x.
  float32 Sin(float32 x);

  ///Returns the sine of x.
  float64 Sin(float64 x);

  ///Returns the sine of x.
  float80 Sin(float80 x);

  ///Returns the cosine of x.
  float32 Cos(float32 x);

  ///Returns the cosine of x.
  float64 Cos(float64 x);

  ///Returns the cosine of x.
  float80 Cos(float80 x);

  ///Returns the tangent of x.
  float32 Tan(float32 x);

  ///Returns the tangent of x.
  float64 Tan(float64 x);

  ///Returns the tangent of x.
  float80 Tan(float80 x);

  ///Returns the sine of x.
  float32 ArcSin(float32 x);

  ///Returns the sine of x.
  float64 ArcSin(float64 x);

  ///Returns the sine of x.
  float80 ArcSin(float80 x);

  ///Returns the cosine of x.
  float32 ArcCos(float32 x);

  ///Returns the cosine of x.
  float64 ArcCos(float64 x);

  ///Returns the cosine of x.
  float80 ArcCos(float80 x);

  ///Returns the cosine of x.
  float32 ArcTan(float32 x);

  ///Returns the cosine of x.
  float64 ArcTan(float64 x);

  ///Returns the cosine of x.
  float80 ArcTan(float80 x);

  ///Returns the sinc of x.
  template<typename FloatT>
  FloatT Sinc(FloatT x)
  {
    if(Limits<FloatT>::IsZero(x))
      return FloatT(1);
    return Sin(x) / x;
  }

  //------------------------------//
  //Floors, Ceilings, and Rounding//
  //------------------------------//

  ///Returns the greatest integer less than or equal to x.
  integer Floor(float32 x);

  ///Returns the greatest integer less than or equal to x.
  integer Floor(float64 x);

  ///Returns the greatest integer less than or equal to x.
  integer Floor(float80 x);

  /**Returns Floor(x / SmallestIncrement) * SmallestIncrement. When used with a
  power-of-two value for SmallestIncrement, this method truncates the
  floating-point mantissa at a place before or after the decimal place.*/
  template<typename FloatT>
  FloatT Truncate(FloatT x, FloatT SmallestIncrement)
  {
    return Floor(x / SmallestIncrement) * SmallestIncrement;
  }

  ///Returns the integer closest to x.
  integer Round(float32 x);

  ///Returns the integer closest to x.
  integer Round(float64 x);

  ///Returns the integer closest to x.
  integer Round(float80 x);

  ///Returns the smallest integer greater than or equal to x.
  integer Ceiling(float32 x);

  ///Returns the smallest integer greater than or equal to x.
  integer Ceiling(float64 x);

  ///Returns the smallest integer greater than or equal to x.
  integer Ceiling(float80 x);

  ///Returns the smallest power-of-two greater than or equal to x.
  float32 PowerOfTwoCeiling(float32 x);

  ///Returns the smallest power-of-two greater than or equal to x.
  float64 PowerOfTwoCeiling(float64 x);

  ///Returns the smallest power-of-two greater than or equal to x.
  float80 PowerOfTwoCeiling(float80 x);

  ///Returns the smallest power-of-two greater than or equal to m.
  int32 PowerOfTwoCeiling(int32 m);

  ///Returns the smallest power-of-two greater than or equal to m.
  int64 PowerOfTwoCeiling(int64 m);

  ///Returns the integer part of x.
  integer IntegerPart(float32 x);

  ///Returns the integer part of x.
  integer IntegerPart(float64 x);

  ///Returns the integer part of x.
  integer IntegerPart(float80 x);

  ///Returns the fractional part of x.
  float32 FractionalPart(float32 x);

  ///Returns the fractional part of x.
  float64 FractionalPart(float64 x);

  ///Returns the fractional part of x.
  float80 FractionalPart(float80 x);

  ///Returns zero for x that are within delta of zero and returns x otherwise.
  float32 Chop(float32 x, float32 Delta);

  ///Returns zero for x that are within delta of zero and returns x otherwise.
  float64 Chop(float64 x, float64 Delta);

  ///Returns zero for x that are within delta of zero and returns x otherwise.
  float80 Chop(float80 x, float80 Delta);

  /**Returns x clipped to be between minimum and maximum values. If x is NaN,
  then the minimum value is returned.*/
  template <class T> T Clip(T x, T MinValue, T MaxValue)
  {
    return x >= MinValue and x <= MaxValue ? x        :
           x > MaxValue                    ? MaxValue : MinValue;
  }

  //---------------------------------//
  //Sign, Absolute Value and Distance//
  //---------------------------------//

  ///Returns -1, 0 or 1 depending on whether x is negative, zero, or positive.
  float32 Sign(float32 x);

  ///Returns -1, 0 or 1 depending on whether x is negative, zero, or positive.
  float64 Sign(float64 x);

  ///Returns -1, 0 or 1 depending on whether x is negative, zero, or positive.
  float80 Sign(float80 x);

  ///Returns -1, 0 or 1 depending on whether m is negative, zero, or positive.
  int32 Sign(int32 m);

  ///Returns -1, 0 or 1 depending on whether m is negative, zero, or positive.
  int64 Sign(int64 m);

  ///Returns the absolute value of x.
  float32 Abs(float32 x);

  ///Returns the absolute value of x.
  float64 Abs(float64 x);

  ///Returns the absolute value of x.
  float80 Abs(float80 x);

  ///Returns the absolute value of m.
  int32 Abs(int32 m);

  ///Returns the absolute value of m.
  int64 Abs(int64 m);

  ///Returns the absolute value of the vector {x, y}.
  float32 Abs(float32 x, float32 y);

  ///Returns the absolute value of the vector {x, y}
  float64 Abs(float64 x, float64 y);

  ///Returns the absolute value of the vector {x, y}
  float80 Abs(float80 x, float80 y);

  ///Returns the argument (principle angle) of the vector {x, y} over (-Pi, Pi].
  float32 Arg(float32 x, float32 y);

  ///Returns the argument (principle angle) of the vector {x, y} over (-Pi, Pi].
  float64 Arg(float64 x, float64 y);

  ///Returns the argument (principle angle) of the vector {x, y} over (-Pi, Pi].
  float80 Arg(float80 x, float80 y);

  ///Returns the angle of the vector {x, y} over [0, 2*Pi).
  float32 Angle(float32 x, float32 y);

  ///Returns the angle of the vector {x, y} over [0, 2*Pi).
  float64 Angle(float64 x, float64 y);

  ///Returns the angle of the vector {x, y} over [0, 2*Pi).
  float80 Angle(float80 x, float80 y);

  ///Returns the distance between the vectors {x1, y1} and {x2, y2}.
  float32 Distance(float32 x1, float32 y1, float32 x2, float32 y2);

  ///Returns the distance between the vectors {x1, y1} and {x2, y2}.
  float64 Distance(float64 x1, float64 y1, float64 x2, float64 y2);

  ///Returns the distance between the vectors {x1, y1} and {x2, y2}.
  float80 Distance(float80 x1, float80 y1, float80 x2, float80 y2);

  //-----------//
  //Polynomials//
  //-----------//

  ///Returns the linear root of ax + b.
  count Roots(number a, number b, number& Root1);

  ///Returns the quadratic root of ax^2 + bx + c.
  count Roots(number a, number b, number c, number& Root1, number& Root2);

  //------//
  //Modulo//
  //------//

  /**Returns the remainder on division of m by n. This modulo is defined to be
  cyclic for negative m, so that Mod(-1, 3) = 2.*/
  int32 Mod(int32 m, int32 n);

  /**Returns the remainder on division of m by n. This modulo is defined to be
  cyclic for negative m, so that Mod(-1, 3) = 2.*/
  int64 Mod(int64 m, int64 n);

  /**Returns the remainder on division of x by y. This modulo is defined to be
  cyclic for negative x, so that Mod(-1.0, 3.0) = 2.0.*/
  float32 Mod(float32 x, float32 y);

  /**Returns the remainder on division of x by y. This modulo is defined to be
  cyclic for negative x, so that Mod(-1.0, 3.0) = 2.0.*/
  float64 Mod(float64 x, float64 y);

  /**Returns the remainder on division of x by y. This modulo is defined to be
  cyclic for negative x, so that Mod(-1.0, 3.0) = 2.0.*/
  float80 Mod(float80 x, float80 y);

  //-------------------------------//
  //Swapping, Minimums and Maximums//
  //-------------------------------//

  ///Swaps two things using a temporary variable.
  template <class T> static void Swap(T& a, T& b)
  {
    T t = a;
    a = b;
    b = t;
  }

  ///Sorts two things in ascending order.
  template <class T> static void Ascending(T& a, T& b)
  {
    if(a > b)
      Swap(a, b);
  }

  ///Sorts two things in descending order.
  template <class T> static void Descending(T& a, T& b)
  {
    if(a < b)
      Swap(a, b);
  }

  ///Returns the greater of two values.
  template <class T> static T Max(const T& a, const T& b)
  {
    return a > b ? a : b;
  }

  ///Returns the lesser of two values.
  template <class T> static T Min(const T& a, const T& b)
  {
    return (a < b ? a : b);
  }

  ///Returns the square of a value.
  template <class T> static T Square(const T& a)
  {
    return a * a;
  }

  ///Returns the cube of a value.
  template <class T> static T Cube(const T& a)
  {
    return a * a * a;
  }

  ///Makes a at least the value of b.
  template <class T> static void MakeAtLeast(T& a, const T& b)
  {
    if(a < b)
      a = b;
  }

  ///Makes a at most the value of b.
  template <class T> static void MakeAtMost(T& a, const T& b)
  {
    if(a > b)
      a = b;
  }

  ///Returns whether c is inclusively between a and b.
  template <class T> static bool IsBetween(const T& c, const T& a,
    const T& b)
  {
    return (b >= a and c >= a and c <= b) or (a > b and c >= b and c <= a);
  }

  //-------//
  //Utility//
  //-------//

  ///Adds b to a if a + b does not exceed maximum. Returns whether sum occurred.
  template <class T> static bool AddIfNotMoreThan(T& a, const T& b,
    const T& Maximum)
  {
    T Sum = a + b;
    return Sum <= Maximum ? a = Sum, true : false;
  }

  template <class T> static T Average(T a, T b)
  {
    Ascending(a, b);
    return a + (b - a) / T(2);
  }

  template <class T> static T Half(T x)
  {
    return x / T(2);
  }

  template <class T> static T Double(T x)
  {
    return x * T(2);
  }

#ifdef PRIM_COMPILE_INLINE
  //---------------------//
  //Exponential Functions//
  //---------------------//

  float32 Exp(float32 x)
  {
    return std::exp(x);
  }

  float64 Exp(float64 x)
  {
    return std::exp(x);
  }

  float80 Exp(float80 x)
  {
    return std::exp(x);
  }

  float32 Power(float32 x, float32 y)
  {
    return std::pow(x, y);
  }

  float64 Power(float64 x, float64 y)
  {
    return std::pow(x, y);
  }

  float80 Power(float80 x, float80 y)
  {
    return std::pow(x, y);
  }

  float32 Log(float32 x)
  {
    return std::log(x);
  }

  float64 Log(float64 x)
  {
    return std::log(x);
  }

  float80 Log(float80 x)
  {
    return std::log(x);
  }

  float32 Log(float32 b, float32 x)
  {
    return std::log(x) / std::log(b);
  }

  float64 Log(float64 b, float64 x)
  {
    return std::log(x) / std::log(b);
  }

  float80 Log(float80 b, float80 x)
  {
    return std::log(x) / std::log(b);
  }

  float32 Log2(float32 x)
  {
    return std::log(x) * Log2Inv<float32>();
  }

  float64 Log2(float64 x)
  {
    return std::log(x) * Log2Inv<float64>();
  }

  float80 Log2(float80 x)
  {
    return std::log(x) * Log2Inv<float80>();
  }

  float32 Log10(float32 x)
  {
    return std::log10(x);
  }

  float64 Log10(float64 x)
  {
    return std::log10(x);
  }

  float80 Log10(float80 x)
  {
    return std::log10(x);
  }

  float32 Sqrt(float32 x)
  {
    return std::sqrt(x);
  }

  float64 Sqrt(float64 x)
  {
    return std::sqrt(x);
  }

  float80 Sqrt(float80 x)
  {
    return std::sqrt(x);
  }

  //-----------------------//
  //Trigonometric Functions//
  //-----------------------//

  float32 Sin(float32 x)
  {
    return std::sin(x);
  }

  float64 Sin(float64 x)
  {
    return std::sin(x);
  }

  float80 Sin(float80 x)
  {
    return std::sin(x);
  }

  float32 Cos(float32 x)
  {
    return std::cos(x);
  }

  float64 Cos(float64 x)
  {
    return std::cos(x);
  }

  float80 Cos(float80 x)
  {
    return std::cos(x);
  }

  float32 Tan(float32 x)
  {
    return std::tan(x);
  }

  float64 Tan(float64 x)
  {
    return std::tan(x);
  }

  float80 Tan(float80 x)
  {
    return std::tan(x);
  }

  float32 ArcSin(float32 x)
  {
    return std::asin(x);
  }

  float64 ArcSin(float64 x)
  {
    return std::asin(x);
  }

  float80 ArcSin(float80 x)
  {
    return std::asin(x);
  }

  float32 ArcCos(float32 x)
  {
    return std::acos(x);
  }

  float64 ArcCos(float64 x)
  {
    return std::acos(x);
  }

  float80 ArcCos(float80 x)
  {
    return std::acos(x);
  }

  float32 ArcTan(float32 x)
  {
    return std::atan(x);
  }

  float64 ArcTan(float64 x)
  {
    return std::atan(x);
  }

  float80 ArcTan(float80 x)
  {
    return std::atan(x);
  }

  //------------------------------//
  //Floors, Ceilings, and Rounding//
  //------------------------------//

  integer Floor(float32 x)
  {
    return integer(std::floor(x));
  }

  integer Floor(float64 x)
  {
    return integer(std::floor(x));
  }

  integer Floor(float80 x)
  {
    return integer(std::floor(x));
  }

  integer Round(float32 x)
  {
    return Floor(x + 0.5f);
  }

  integer Round(float64 x)
  {
    return Floor(x + 0.5);
  }

  integer Round(float80 x)
  {
    return Floor(x + 0.5L);
  }

  integer Ceiling(float32 x)
  {
    return integer(std::ceil(x));
  }

  integer Ceiling(float64 x)
  {
    return integer(std::ceil(x));
  }

  integer Ceiling(float80 x)
  {
    return integer(std::ceil(x));
  }

  float32 PowerOfTwoCeiling(float32 x)
  {
    /* #hack : (long long) -> (float) is problematic on armv7 and causes an
    internal linker error in llvm. Casting to double first.*/
    return Power(2.f, float32(float64(Ceiling(Log2(Abs(x)))))) * Sign(x);
  }

  float64 PowerOfTwoCeiling(float64 x)
  {
    return Power(2., float64(Ceiling(Log2(Abs(x))))) * Sign(x);
  }

  float80 PowerOfTwoCeiling(float80 x)
  {
    return Power(2.L, float80(Ceiling(Log2(Abs(x))))) * Sign(x);
  }

  int32 PowerOfTwoCeiling(int32 m)
  {
    return int32(Power(2., float64(Ceiling(Log2(float64(Abs(m))))))) * Sign(m);
  }

  int64 PowerOfTwoCeiling(int64 m)
  {
    return int64(Power(2., float64(Ceiling(Log2(float64(Abs(m))))))) * Sign(m);
  }

  integer IntegerPart(float32 x)
  {
    if(x > 0.f)
      return Floor(x);
    else
      return Ceiling(x);
  }

  integer IntegerPart(float64 x)
  {
    if(x >= 0.)
      return Floor(x);
    else
      return Ceiling(x);
  }

  integer IntegerPart(float80 x)
  {
    if(x >= 0.L)
      return Floor(x);
    else
      return Ceiling(x);
  }

  float32 FractionalPart(float32 x)
  {
    /* #hack : (long long) -> (float) is problematic on armv7 and causes an
    internal linker error in llvm. Casting to double first.*/
    if(x >= 0.f)
      return x - float32(float64(Floor(x)));
    else
      return x - float32(float64(Ceiling(x)));
  }

  float64 FractionalPart(float64 x)
  {
    if(x >= 0.)
      return x - Floor(x);
    else
      return x - Ceiling(x);
  }

  float80 FractionalPart(float80 x)
  {
    if(x >= 0.L)
      return x - Floor(x);
    else
      return x - Ceiling(x);
  }

  float32 Chop(float32 x, float32 Delta)
  {
    if(x > -Delta and x < Delta)
      return 0.f;
    return x;
  }

  float64 Chop(float64 x, float64 Delta)
  {
    if(x > -Delta and x < Delta)
      return 0.;
    return x;
  }

  float80 Chop(float80 x, float80 Delta)
  {
    if(x > -Delta and x < Delta)
      return 0.L;
    return x;
  }

  //----------------------------------//
  //Sign, Absolute Value, and Distance//
  //----------------------------------//

  float32 Sign(float32 x)
  {
    if(x > 0.f)
      return 1.f;
    else if(x < 0.f)
      return -1.f;

    return 0.f;
  }

  float64 Sign(float64 x)
  {
    if(x > 0.)
      return 1.;
    else if(x < 0.)
      return -1.;

    return 0.;
  }

  float80 Sign(float80 x)
  {
    if(x > 0.L)
      return 1.L;
    else if(x < 0.L)
      return -1.L;

    return 0.L;
  }

  int32 Sign(int32 m)
  {
    if(m > 0)
      return 1;
    else if(m < 0)
      return -1;

    return 0;
  }

  int64 Sign(int64 m)
  {
    if(m > 0)
      return 1;
    else if(m < 0)
      return -1;

    return 0;
  }

  float32 Abs(float32 x)
  {
    return x >= 0.f ? x : -x;
  }

  float64 Abs(float64 x)
  {
    return x >= 0. ? x : -x;
  }

  float80 Abs(float80 x)
  {
    return x >= 0.L ? x : -x;
  }

  int32 Abs(int32 m)
  {
    return m >= 0 ? m : -m;
  }

  int64 Abs(int64 m)
  {
    return m >= 0 ? m : -m;
  }

  float32 Abs(float32 x, float32 y)
  {
    return std::sqrt(x * x + y * y);
  }

  float64 Abs(float64 x, float64 y)
  {
    return std::sqrt(x * x + y * y);
  }

  float80 Abs(float80 x, float80 y)
  {
    return std::sqrt(x * x + y * y);
  }

  float32 Arg(float32 x, float32 y)
  {
    return std::atan2(y, x);
  }

  float64 Arg(float64 x, float64 y)
  {
    return std::atan2(y, x);
  }

  float80 Arg(float80 x, float80 y)
  {
    return std::atan2(y, x);
  }

  float32 Angle(float32 x, float32 y)
  {
    float32 Principle = Arg(x, y);
    if(Principle < 0.f)
      Principle += 2.f * Pi<float32>();
    return Principle;
  }

  float64 Angle(float64 x, float64 y)
  {
    float64 Principle = Arg(x, y);
    if(Principle < 0.)
      Principle += 2. * Pi<float64>();
    return Principle;
  }

  float80 Angle(float80 x, float80 y)
  {
    float80 Principle = Arg(x, y);
    if(Principle < 0.L)
      Principle += 2.L * Pi<float80>();
    return Principle;
  }

  float32 Distance(float32 x1, float32 y1, float32 x2, float32 y2)
  {
    float32 xd = x2 - x1, yd = y2 - y1;
    return std::sqrt(xd * xd + yd * yd);
  }

  float64 Distance(float64 x1, float64 y1, float64 x2, float64 y2)
  {
    float64 xd = x2 - x1, yd = y2 - y1;
    return std::sqrt(xd * xd + yd * yd);
  }

  float80 Distance(float80 x1, float80 y1, float80 x2, float80 y2)
  {
    float80 xd = x2 - x1, yd = y2 - y1;
    return std::sqrt(xd * xd + yd * yd);
  }

  //-----------//
  //Polynomials//
  //-----------//

  count Roots(number a, number b, number& Root1)
  {
    if(Limits<number>::Unbounded(Root1 = -b / a))
    {
      Root1 = Limits<number>::QuietNaN();
      return 0;
    }
    return 1;
  }

  count Roots(number a, number b, number c, number& Root1, number& Root2)
  {
    number DiscriminantSquared = b * b - 4.f * a * c;
    if(DiscriminantSquared < 0.f)
    {
      Root1 = Root2 = 0.f;
      return 0;
    }

    number Discriminant = Sqrt(DiscriminantSquared);
    if(Discriminant > 0.f)
    {
      number t = -0.5f * (b + Sign(b) * Discriminant);
      if(Limits<number>::IsNotZero(t))
      {
        Root1 = c / t;
        if(a < 0.f or a > 0.f)
          Root2 = t / a;
        else
          return 1;
      }
      else
      {
        Root1 = (-b + Discriminant) * 0.5f * a;
        Root2 = (-b - Discriminant) * 0.5f * a;
      }
      Ascending(Root1, Root2);
      return 2;
    }
    else if(Limits<number>::IsZero(Discriminant))
    {
      Root1 = 0.5f * b / a;
      Root2 = 0.f;
      return 1;
    }
    else
    {
      Root1 = Root2 = 0.f;
      return 0;
    }
  }

  //------//
  //Modulo//
  //------//

  int32 Mod(int32 m, int32 n)
  {
    int32 n2 = Abs(n);

    if(n2 == 0)
      return -1;
    if(m < 0)
      return (n2 - (-m % n2)) * Sign(n);

    return (m % n2) * Sign(n);
  }

  int64 Mod(int64 m, int64 n)
  {
    int64 n2 = Abs(n);

    if(n2 == 0)
      return -1;
    if(m < 0)
      return (n2 - (-m % n2)) * Sign(n);

    return (m % n2) * Sign(n);
  }

  float32 Mod(float32 x, float32 y)
  {
    return float32(Mod(float64(x), float64(y)));
  }

  float64 Mod(float64 x, float64 y)
  {
    float64 IntegerPart = 0.f;
    return std::modf(Abs(x / y), &IntegerPart) * y;
  }

  float80 Mod(float80 x, float80 y)
  {
    float80 IntegerPart = 0.f;
    return std::modf(Abs(x / y), &IntegerPart) * y;
  }
#endif
}
#endif
