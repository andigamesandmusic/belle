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

#ifndef PRIM_INCLUDE_RATIONAL_H
#define PRIM_INCLUDE_RATIONAL_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///Templated class for expressing rational numbers.
  template <class IntegralType>
  class Rational
  {
    protected:

    /**The numerator of the Rational number. It is always expressed in its
    simplest form.*/
    IntegralType n;

    /**The denominator of the Rational number. It is always expressed in its
    simplest form.*/
    IntegralType d;

    ///Rewrites the sign in canonical form where the numerator takes the sign.
    void SimplifySign()
    {
      if(d < 0)
      {
        n = -n;
        d = -d;
      }
    }

    ///Reduces the ratio to the simplest possible terms.
    void SimplifyRatio()
    {
      if(not d) //Indeterminate form
      {
        n = 0;
        return;
      }

      if(not n) //Zero: assume denominator of one for consistency.
      {
        d = 1;
        return;
      }

      IntegralType g = GCD(n, d);
      if(g < 1)
      {
        //GCD calculation failed.
        n = d = 0;
        return;
      }

      while(g > 1)
      {
        n = n / g;
        d = d / g;
        g = GCD(n, d);
      }
    }

    ///Simplifies the sign and ratio.
    void Simplify()
    {
      SimplifySign();
      SimplifyRatio();
    }

    public:

    ///Creates an empty ratio (not zero).
    Rational() : n(0), d(0) {}

    ///Copy constructor for a ratio.
    Rational(const Rational<IntegralType>& Other)
    {
      n = Other.n;
      d = Other.d;
    }

    ///Creates a ratio from a numerator and denominator.
    Rational(IntegralType Numerator, IntegralType Denominator)
    {
      n = Numerator;
      d = Denominator;
      Simplify();
    }

    ///Creates a ratio from a coerced floating-point value.
    Rational(float32 Number, count HighestDenominator, bool PowersOfTwoOnly)
    {
      (*this) = Coerce(number(Number), HighestDenominator, PowersOfTwoOnly);
    }

    ///Creates a ratio from a coerced floating-point value.
    Rational(float64 Number, count HighestDenominator, bool PowersOfTwoOnly)
    {
      (*this) = Coerce(number(Number), HighestDenominator, PowersOfTwoOnly);
    }

    ///Creates a ratio from a coerced floating-point value.
    Rational(float80 Number, count HighestDenominator, bool PowersOfTwoOnly)
    {
      (*this) = Coerce(number(Number), HighestDenominator, PowersOfTwoOnly);
    }

    ///Creates a ratio from a whole number.
    Rational(int32 WholeNumber)
    {
      n = IntegralType(WholeNumber);
      d = 1;
    }

    ///Creates a ratio from a whole number.
    Rational(int64 WholeNumber)
    {
      n = IntegralType(WholeNumber);
      d = 1;
    }

    ///Creates a ratio from a string.
    Rational(const String& s)
    {
      *this = FromString(s);
    }

    ///Returns the numerator.
    IntegralType Numerator() const
    {
      return n;
    }

    ///Returns the denominator.
    IntegralType Denominator() const
    {
      return d;
    }

    ///Coerces a number to a ratio.
    static Rational<IntegralType> Coerce(number Value,
      count HighestDenominator = 1024, bool PowersOfTwoOnly = true)
    {
      integer Best_n = Round(Value), Best_d = 1;
      number Best = Rational<IntegralType>(Best_n, Best_d).To<number>();
      for(integer d = 2; d <= integer(HighestDenominator);
        (PowersOfTwoOnly ? d *= 2 : d++))
      {
        integer n = Round(Value * number(d));
        number Try = number(n) / number(d);
        if(Abs(Try - Value) < Abs(Best - Value))
        {
          Best = Try;
          Best_n = n;
          Best_d = d;
        }
      }
      return Rational<IntegralType>(Best_n, Best_d);
    }

    ///Coerces a ratio to use a lower denominator.
    static Rational<IntegralType> Coerce(Rational<IntegralType> Value,
      count HighestDenominator = 1024, bool PowersOfTwoOnly = true)
    {
      return Coerce(Value.To<number>(), HighestDenominator, PowersOfTwoOnly);
    }

    ///Checks whether the number has a non-zero denominator.
    bool IsDeterminate() const
    {
      return d;
    }

    ///Checks whether the number is whole.
    bool IsWhole() const
    {
      return d == 1;
    }

    ///Checks whether the number is empty.
    bool IsEmpty() const
    {
      return not d;
    }

    ///Calculates the greatest common denominator.
    static IntegralType GCD(IntegralType a, IntegralType b)
    {
      //Test for zeroes.
      if(not a or not b)
        return 0;

      /*The negative-most integer is not permissible since it has no positive
      counterpart. The GCD calculation only works with positive numbers and
      must negate negative numbers to calculate. Therefore, if the negative-most
      integer is used, the GCD will fail to calculate. In this situation, it is
      best to set the ratio to empty.*/
      if(a == Limits<IntegralType>::Min() or b == Limits<IntegralType>::Min())
        return 0;

      //Naturalize the numbers.
      if(a < 0) a = -a;
      if(b < 0) b = -b;

      //Using the iterative Euclidean algorithm.

      /*Note this algorithm has a maximum number of steps no more than 5 times
      the number of digits of the smaller integer. That peaks at about 100 for
      the largest 64-bit integers.*/
      IntegralType t;
      while(b > 0)
      {
        t = b;
        b = a % b;
        a = t;
      }
      return a;
    }

    ///Calculates the least common multiple.
    static IntegralType LCM(IntegralType a, IntegralType b)
    {
      //Test for zeroes.
      if(not a or not b)
        return 0;

      //Naturalize the numbers.
      if(a < 0) a = -a;
      if(b < 0) b = -b;

      //Use the GCD to calculate the LCM.
      return (a * b) / GCD(a, b);
    }

    ///Calculates the remainder of division.
    static Rational<IntegralType> Mod(Rational<IntegralType> a,
      Rational<IntegralType> b)
    {
      IntegralType d2 = LCM(a.d, b.d);
      IntegralType a2 = d2 / a.d;
      IntegralType b2 = d2 / b.d;
      a.n *= a2;
      b.n *= b2;

      //Now a and b have the same denominator.
      IntegralType n2 = a.n % b.n;

      return Rational<IntegralType>(n2, d2);
    }

    //--------------------//
    //Operator definitions//
    //--------------------//

    Rational<IntegralType> operator + (
      const Rational<IntegralType>& Other) const
    {
      if(not d or not Other.d)
        return Rational<IntegralType>();
      IntegralType LCD = LCM(d, Other.d);
      IntegralType d2 = LCD;
      IntegralType n2 = n * (LCD / d) + Other.n * (LCD / Other.d);
      return Rational<IntegralType>(n2, d2);
    }

    Rational<IntegralType> operator + (int Other) const
    {
      return *this + Rational<IntegralType>(IntegralType(Other),
        IntegralType(1));
    }

    Rational<IntegralType> operator + (const String& Other) const
    {
      return *this + Rational<IntegralType>(Other);
    }

    Rational<IntegralType> operator - (
      const Rational<IntegralType>& Other) const
    {
      if(not d or not Other.d)
        return Rational<IntegralType>();
      IntegralType LCD = LCM(d, Other.d);
      IntegralType d2 = LCD;
      IntegralType n2 = n * (LCD / d) - Other.n * (LCD / Other.d);
      return Rational<IntegralType>(n2, d2);
    }

    Rational<IntegralType> operator - (int Other) const
    {
      return *this - Rational<IntegralType>(IntegralType(Other),
        IntegralType(1));
    }

    Rational<IntegralType> operator - (const String& Other) const
    {
      return *this - Rational<IntegralType>(Other);
    }

    Rational<IntegralType> operator * (
      const Rational<IntegralType>& Other) const
    {
      IntegralType n2 = n * Other.n;
      IntegralType d2 = d * Other.d;
      return Rational<IntegralType>(n2, d2);
    }

    Rational<IntegralType> operator * (int Other) const
    {
      return *this * Rational<IntegralType>(IntegralType(Other),
        IntegralType(1));
    }

    Rational<IntegralType> operator * (const String& Other) const
    {
      return *this * Rational<IntegralType>(Other);
    }

    Rational<IntegralType> operator / (
      const Rational<IntegralType>& Other) const
    {
      IntegralType n2 = n * Other.d;
      IntegralType d2 = d * Other.n;
      return Rational<IntegralType>(n2, d2);
    }

    Rational<IntegralType> operator / (int Other) const
    {
      return *this / Rational<IntegralType>(IntegralType(Other),
        IntegralType(1));
    }

    Rational<IntegralType> operator / (const String& Other) const
    {
      return *this / Rational<IntegralType>(Other);
    }

    Rational<IntegralType> operator += (const Rational<IntegralType>& Other)
    {
      *this = *this + Other;
      return *this;
    }

    Rational<IntegralType> operator += (int Other)
    {
      *this = *this + Rational<IntegralType>(IntegralType(Other),
        IntegralType(1));
      return *this;
    }

    Rational<IntegralType> operator += (const String& Other)
    {
      *this = *this + Rational<IntegralType>(Other);
      return *this;
    }

    Rational<IntegralType> operator -= (const Rational<IntegralType>& Other)
    {
      *this = *this - Other;
      return *this;
    }

    Rational<IntegralType> operator -= (int Other)
    {
      *this = *this - Rational<IntegralType>(IntegralType(Other),
        IntegralType(1));
      return *this;
    }

    Rational<IntegralType> operator -= (const String& Other)
    {
      *this = *this - Rational<IntegralType>(Other);
      return *this;
    }

    Rational<IntegralType> operator *= (const Rational<IntegralType>& Other)
    {
      *this = *this * Other;
      return *this;
    }

    Rational<IntegralType> operator *= (int Other)
    {
      *this = *this * Rational<IntegralType>(IntegralType(Other),
        IntegralType(1));
      return *this;
    }

    Rational<IntegralType> operator *= (const String& Other)
    {
      *this = *this * Rational<IntegralType>(Other);
      return *this;
    }

    Rational<IntegralType> operator /= (const Rational<IntegralType>& Other)
    {
      *this = *this / Other;
      return *this;
    }

    Rational<IntegralType> operator /= (int Other)
    {
      *this = *this / Rational<IntegralType>(IntegralType(Other),
        IntegralType(1));
      return *this;
    }

    Rational<IntegralType> operator /= (const String& Other)
    {
      *this = *this / Rational<IntegralType>(Other);
      return *this;
    }

    Rational<IntegralType> operator = (const Rational<IntegralType>& Other)
    {
      n = Other.n;
      d = Other.d;
      return *this;
    }

    Rational<IntegralType> operator = (int Other)
    {
      n = Other;
      d = 1;
      return *this;
    }

    Rational<IntegralType> operator = (const String& Other)
    {
      *this = FromString(Other);
      return *this;
    }

    Rational<IntegralType> operator ++ (int Dummy)
    {
      (void)Dummy;
      n = n + d;
      return *this;
    }

    Rational<IntegralType> operator -- (int Dummy)
    {
      (void)Dummy;
      n = n - d;
      return *this;
    }

    Rational<IntegralType> operator - () const
    {
      return *this * -1;
    }

    bool operator == (const Rational<IntegralType>& Other) const
    {
      return n == Other.n and d == Other.d;
    }

    bool operator == (int Other) const
    {
      return n == Other and d == 1;
    }

    bool operator == (const String& Other) const
    {
      Rational<IntegralType> r(Other);
      return n == r.n and d == r.d;
    }

    bool operator != (const Rational<IntegralType>& Other) const
    {
      return n != Other.n or d != Other.d;
    }

    bool operator != (int Other) const
    {
      return n != Other or d != 1;
    }

    bool operator != (const String& Other) const
    {
      Rational<IntegralType> r(Other);
      return n != r.n or d != r.d;
    }

    bool operator > (const Rational<IntegralType>& Other) const
    {
      Rational<IntegralType> Difference = *this - Other;
      return Difference.n > 0;
    }

    bool operator > (int Other) const
    {
      Rational<IntegralType> Difference = *this -
        Rational<IntegralType>(IntegralType(Other), IntegralType(1));
      return Difference.n > 0;
    }

    bool operator > (const String& Other) const
    {
      Rational<IntegralType> r(Other);
      Rational<IntegralType> Difference = *this - r;
      return Difference.n > 0;
    }

    bool operator >= (const Rational<IntegralType>& Other) const
    {
      Rational<IntegralType> Difference = *this - Other;
      return Difference.n >= 0 and Difference.d;
    }

    bool operator >= (int Other) const
    {
      Rational<IntegralType> Difference = *this -
        Rational<IntegralType>(IntegralType(Other), IntegralType(1));
      return Difference.n >= 0 and Difference.d;
    }

    bool operator >= (const String& Other) const
    {
      Rational<IntegralType> r(Other);
      Rational<IntegralType> Difference = *this - r;
      return Difference.n >= 0 and Difference.d;
    }

    bool operator < (const Rational<IntegralType>& Other) const
    {
      Rational<IntegralType> Difference = *this - Other;
      return Difference.n < 0;
    }

    bool operator < (int Other) const
    {
      Rational<IntegralType> Difference = *this -
        Rational<IntegralType>(IntegralType(Other), IntegralType(1));
      return Difference.n < 0;
    }

    bool operator < (const String& Other) const
    {
      Rational<IntegralType> r(Other);
      Rational<IntegralType> Difference = *this - r;
      return Difference.n < 0;
    }

    bool operator <= (const Rational<IntegralType>& Other) const
    {
      Rational<IntegralType> Difference = *this - Other;
      return Difference.n <= 0 and Difference.d;
    }

    bool operator <= (int Other) const
    {
      Rational<IntegralType> Difference = *this -
        Rational<IntegralType>(IntegralType(Other), IntegralType(1));
      return Difference.n <= 0 and Difference.d;
    }

    bool operator <= (const String& Other) const
    {
      Rational<IntegralType> r(Other);
      Rational<IntegralType> Difference = *this - r;
      return Difference.n <= 0 and Difference.d;
    }

    //-------//
    //Helpers//
    //-------//

    ///Converts a string to x/y format.
    String ToString() const
    {
      if(d == 0) return "0/0";
      else if(n == 0) return "0/1";
      const count LengthOfLargest64BitRatioString = 42;
      /*
      Note: 42 is intentional and equals:
      length of "18446744073709551614/18446744073709551615X")
      where X is the null terminator
      */
      ascii Buffer[LengthOfLargest64BitRatioString] = {0};
      count CurrentPosition = LengthOfLargest64BitRatioString - 2;
      IntegralType dIterator = d;
      IntegralType nIterator = n;
      while(dIterator)
      {
        Buffer[CurrentPosition--] = '0' + ascii(dIterator % 10);
        dIterator /= 10;
      }
      Buffer[CurrentPosition--] = '/';
      if(n > 0)
      {
        while(nIterator)
        {
          Buffer[CurrentPosition--] = '0' + ascii(nIterator % 10);
          nIterator /= 10;
        }
      }
      else
      {
        while(nIterator)
        {
          Buffer[CurrentPosition--] = '0' +
            ascii(-(nIterator - (nIterator / 10) * 10));
          nIterator /= 10;
        }
        Buffer[CurrentPosition--] = '-';
      }
      return &Buffer[++CurrentPosition];
    }

    ///Converts a string into the most common format for that number.
    String ToPrettyString() const
    {
      if(not d) return Constants::NullSet();
      String s;
      s << n;
      if(n and d != 1)
        s << "/" << d;
      return s;
    }

    ///Reads a rational from a string.
    static Rational<IntegralType> FromString(const String& s)
    {
      count Length = s.n();
      IntegralType n = 0, d = 0;
      bool IsPastSlash = false;
      bool IsInvalid = false;
      bool IsNegative = false;

      //If empty string, immediately return.
      if(not Length)
        return Rational<IntegralType>(0, 0);

      //Look for initial negative sign.
      count Start = 0;
      if(s[0] == '-')
      {
        Start = 1;
        IsNegative = true;
      }

      for(count i = Start; i < Length; i++)
      {
        ascii cr = ascii(s[i]);
        if(cr == '/')
        {
          //Invalid use of slash.
          if(not i or i == Length - 1 or IsPastSlash)
          {
            IsInvalid = true;
            break;
          }
          IsPastSlash = true;
        }
        else if(not IsPastSlash)
        {
          if(cr >= 48 and cr <= 57)
          {
            IntegralType Old_n = n;
            if(IsNegative)
            {
              n = n * IntegralType(10) - IntegralType(cr - 48);

              //If numerator became more positive an overflow is occurring.
              if(n > Old_n)
              {
                IsInvalid = true;
                break;
              }
            }
            else
            {
              n = n * IntegralType(10) + IntegralType(cr - 48);

              //If numerator became more negative an overflow is occurring.
              if(n < Old_n)
              {
                IsInvalid = true;
                break;
              }
            }
          }
          else
          {
            //Invalid non-digit numerator.
            IsInvalid = true;
            break;
          }
        }
        else
        {
          if(cr >= 48 and cr <= 57)
          {
            IntegralType Old_d = d;
            d = d * IntegralType(10) + IntegralType(cr - 48);
            //If numerator became more negative an overflow is occurring.
            if(d < Old_d)
            {
              IsInvalid = true;
              break;
            }
          }
          else
          {
            //Invalid non-digit denominator
            IsInvalid = true;
            break;
          }
        }
      }

      if(IsInvalid)
      {
        n = 0;
        d = 0;
      }
      else
      {
        if(not IsPastSlash)
          d = 1;
      }

      return Rational<IntegralType>(n, d);
    }

    ///Automatically casts the rational to a string when needed.
    operator String() const
    {
      return ToPrettyString();
    }

    ///Casts the rational to other types.
    template <class Numeric>
    Numeric To() const
    {
      if(not d)
        return 0; //Indeterminate case
      else
        return Numeric(n) / Numeric(d);
    }
  };

  //Template instantiations
  typedef Rational<int64> Ratio;

  //Math functions for ratios

  /**Chop for ratio returns the input value. This is done because ratios are
  exact and thus do not need to be chopped. This is useful for zero testing
  when the argument could be a floating point or rational number.*/
  Ratio Chop(Ratio x, float32 Delta);

  ///Returns the absolute value.
  Ratio Abs(Ratio x);

#ifdef PRIM_COMPILE_INLINE
  /**Chop for ratio returns the input value. This is done because ratios are
  exact and thus do not need to be chopped. This is useful for zero testing
  when the argument could be a floating point or rational number.*/
  Ratio Chop(Ratio x, float32 Delta) {(void)Delta; return x;}

  ///Returns the absolute value.
  Ratio Abs(Ratio x) {return (x > 0 ? x : -x);}
#endif
}
#endif
