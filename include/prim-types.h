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

#ifndef PRIM_INCLUDE_TYPES_H
#define PRIM_INCLUDE_TYPES_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

/*Standardizes the library intrinsic types according to their purpose. The
purpose of using standardized data types is to make prim portable and
understandable. This file attempts to be sensitive to many different platforms,
but specialized platforms may need attention to associate the correct types.*/

/*
.............................................................
.                    Standard Data Models                   .
.............................................................
. Model .  char . short . int . long . long long . pointers .
.............................................................
. ILP32 .   1   .   2   .  4  .  4   .     8     .    4     .
. LP64  .   1   .   2   .  4  .  8   .     8     .    8     .
. LLP64 .   1   .   2   .  4  .  4   .     8     .    8     .
. ILP64 .   1   .   2   .  8  .  8   .     8     .    8     .
.............................................................

Note that short is the same as short int, long as long int, and long long as
long long int.

More information on cross-platform issues and data models:
http://en.wikipedia.org/wiki/64-bit
http://www.unix.org/version2/whatsnew/lp64_wp.html

In this header file the data models are referred to by number:

#  Data Model  System
-  ----------  ------
1  ILP32       Most 32-bit systems
2  LP64        Linux/Mac OS X 64-bit systems
3  LLP64       Windows 64-bit systems
4  ILP64       Rare and untested; needs clarification for 32-bit types
*/

namespace PRIM_NAMESPACE
{
  //------------------------//
  //Fixed Byte Integer Types//
  //------------------------//

  ///Unsigned 8-bit integer
  typedef unsigned char uint8;

  ///Signed 8-bit integer
  typedef char int8;

  ///Unsigned 16-bit integer
  typedef unsigned short uint16;

  ///Signed 16-bit integer
  typedef short int16;

  ///Unsigned 32-bit integer
  typedef unsigned int uint32;

  ///Signed 32-bit integer
  typedef int int32;

  /*While a long long is 8-bytes for all data models, avoid compiling with the
  long long type when long is sufficient due to portability concerns. For
  example, long long requires the C99 extensions in GNU g++.*/
#ifdef PRIM_ENVIRONMENT_LP64
  ///Signed 64-bit integer
  typedef long int64;

  ///Unsigned 64-bit integer
  typedef unsigned long uint64;
#else
  //Note: long long is a C++11 type
  typedef long long int64;
  typedef unsigned long long uint64;
#endif

  //--------------------//
  //Floating-Point Types//
  //--------------------//

  ///32-bit single precision floating point type
  typedef float float32;

  ///64-bit double precision floating point type
  typedef double float64;

  /**80-bit float for compilers supporting x87 extended precision. Support
  for this type can vary a great deal from platform to platform.*/
  typedef long double float80;

  //-------------------------//
  //Counting and Memory Types//
  //-------------------------//

  //Set size of count to be the size of a pointer by default.
#if !defined(PRIM_COUNT) and defined(PRIM_ENVIRONMENT_ILP32)
  #define PRIM_COUNT int32
#elif !defined(PRIM_COUNT) and not defined(PRIM_ENVIRONMENT_ILP32)
  #define PRIM_COUNT int64
#endif

  ///Signed counting integer the size of a pointer
  typedef PRIM_COUNT count;

#ifdef PRIM_ENVIRONMENT_ILP32
  /**Unsigned integer the size of a pointer. Only low-level memory operations
  make use of this type.*/
  typedef uint32 uintptr;
#else
  typedef uint64 uintptr;
#endif
  //------------------//
  //Mathematical Types//
  //------------------//

  ///Data type for doing general-purpose math with integers.
  typedef PRIM_INTEGER integer;

  /**Data type for doing general-purpose math with floating-point numbers. When
  doing math with literal floating-points, there are two recommended approaches
  depending on the type of number being used. Some literals such as 1.0, 123.0,
  1.75, can be multiplied by a small power of two to create an integer. These
  are exact literals and if they can be specified exactly using a float, they
  should be written as, for example, 1.f, 123.f, and 1.75f respectively. For
  example:
  \code
  number x;
  ...
  x = x * 123.f
  \endcode

  It is important to note that even if number is a double, the highest precision
  calculation will still be used since 123.f will first be upconverted to a
  double with no loss of accuracy (since it is already exact).

  For literals that have a different value between their float and double
  representations, for example, 1.2345, the best thing to do is write the
  number as a double and explicitly cast to the number type:
  \code
  number x;
  ...
  x = x * (number)1.2345;
  \endcode

  The above best practice helps when writing code in which number may be a
  float or a double depending on the configuration of prim.*/
  typedef PRIM_NUMBER number;

  ///Returns the actual mantissa precision of a float type.
  template <typename FloatT> count Precision()
  {
    FloatT Original = FloatT(1);
    FloatT Delta = FloatT(1);
    count BitsPrecision = 0;
    while(Original + Delta != Original)
    {
      Delta /= FloatT(2);
      BitsPrecision++;
    }

    return BitsPrecision;
  }

  //--------------------//
  //ASCII Character Type//
  //--------------------//

  ///Signed eight-bit integer used for storing low-level char strings.
  typedef int8 ascii;

  //----------------------//
  //Unicode Character Type//
  //----------------------//

  ///Four-byte character code capable of representing any Unicode codepoint.
  typedef uint32 unicode;

  //----------------//
  //Binary Data Type//
  //----------------//

  ///Unsigned eight-bit integer used for storing low-level binary data.
  typedef uint8 byte;

  //--------------//
  //Data Alignment//
  //--------------//

  ///Use this class to pad the previous member to the word-size boundary.
  template<class T, class PadTo = void*>
  class Pad
  {
    char Dummy [sizeof(PadTo) - (sizeof(T) % sizeof(PadTo))];
    public:
    Pad() {(void)Dummy;}
  };

  /*The following macro can be appended to a member declaration that pads with
  the given type. This is explicit padding and helps to indicate in the code
  exactly where padding will occur. It can be disabled with PRIM_NO_PADDING to
  allow the compiler to make these decisions automatically, potentially
  producing more efficient code. In particular, the padding is wasteful when the
  the previous is member is already aligned.*/
#ifndef PRIM_NO_PADDING
  #define PRIM_PAD_TOKEN_PASTE(x, y) x ## y
  #define PRIM_PAD_TOKEN_PASTE_INDIRECT(x, y) PRIM_PAD_TOKEN_PASTE(x, y)
  #define PRIM_PAD(T) Pad<T> PRIM_PAD_TOKEN_PASTE_INDIRECT(Padding_, __LINE__);
#else
  #define PRIM_PAD(T)
#endif
}

//---------//
//Constants//
//---------//

namespace PRIM_NAMESPACE
{
  /**Portable limits on primitive data types. Note these are only guaranteed to
  be meaningful for the built-in prim types.*/
  template<class T>
  class Limits
  {
    public:

    ///Returns the size of the type in bits.
    static count Bits();

    ///Returns the size of the type in bytes.
    static count Bytes();

    /**Returns the smallest expressible real number. For floating-point numbers
    this returns the smallest positive normalized value (not counting denormal
    numbers). For integral numbers this returns the most negative number.*/
    static T Min();

    ///Returns the largest expressible real number.
    static T Max();

    ///Returns the value with the highest bit set.
    static T HighBit();

    ///Returns whether the type is integral. If not, it is floating point.
    static bool Integral();

    ///Returns whether the type is floating point. If not, it is integral.
    static bool Floating();

    ///Returns whether the type is signed.
    static bool IsSigned();

    ///Returns whether the type has a quiet NaN.
    static bool HasQuietNaN();

    /**Returns the value qNaN (non-signaling not-a-number). Do not use this
    value in an == expression to test for NaN. Instead use the IsNaN() method.
    For integral types, even though HasQuietNaN is false, a NaN will be emulated
    using the lowest negative number.*/
    static T QuietNaN();

    /**Tests whether the value is a NaN. Note that this is the only portable
    way to test for NaN as NaN == NaN is false.*/
    static bool IsNaN(T x);

    private:

    ///Tests whether a floating-point type is NaN.
    static bool IsFloatingNaN(T x);

    public:

    ///Returns whether the type is bounded (not an infinity or NaN).
    static bool Bounded(T f);

    ///Returns whether the type is unbounded (is an infinity or NaN).
    static bool Unbounded(T f);

    ///Returns the value zero in the given type.
    static T Zero();

    ///Returns the value one in the given type.
    static T One();

    ///Safely tests whether value is zero.
    static bool IsZero(T f);

    ///Safely tests whether value is not zero.
    static bool IsNotZero(T f);

    ///Safely tests whether two values are equal.
    static bool IsEqual(const T& f1, const T& f2) {return f1 == f2;}

    ///Safely tests whether two values are not equal.
    static bool IsNotEqual(const T& f1, const T& f2) {return !IsEqual(f1, f2);}

    ///Safely tests whether two values are bitwise equal.
    static bool IsBitwiseEqual(T f1, T f2);

    ///Safely tests whether two values are bitwise not-equal.
    static bool IsBitwiseNotEqual(T f1, T f2);

    ///Returns whether the type has infinity.
    static bool HasInfinity();

    ///Returns infinity. For integral types it returns the maximum.
    static T Infinity();

    ///Returns negative infinity. For integral types it returns the least.
    static T NegativeInfinity();
  };

  template <> inline bool Limits<float>::IsNaN(float x)
  {
    return Limits<float>::IsFloatingNaN(x);
  }

  template <> inline bool Limits<double>::IsNaN(double x)
  {
    return IsFloatingNaN(x);
  }

  template <> inline bool Limits<long double>::IsNaN(long double x)
  {
    return IsFloatingNaN(x);
  }

  template <> inline bool Limits<float32>::IsEqual(
    const float32& f1, const float32& f2)
  {
    if(IsFloatingNaN(f1))
      return IsFloatingNaN(f2);
    else if(IsZero(f1))
      return IsZero(f2);
    return IsBitwiseEqual(f1, f2);
  }

  template <> inline bool Limits<float64>::IsEqual(
    const float64& f1, const float64& f2)
  {
    if(IsFloatingNaN(f1))
      return IsFloatingNaN(f2);
    else if(IsZero(f1))
      return IsZero(f2);
    return IsBitwiseEqual(f1, f2);
  }

  template <> inline bool Limits<float80>::IsEqual(
    const float80& f1, const float80& f2)
  {
    if(IsFloatingNaN(f1))
      return IsFloatingNaN(f2);
    else if(IsZero(f1))
      return IsZero(f2);
    return IsBitwiseEqual(f1, f2);
  }

  /**Class that returns a zero-initialized const reference object. This class
  assumes that T will be valid if its memory is zero-initialized without the
  actual constructor called and that there are no mutable members in T.*/
  template <class T>
  class ZeroInitialized
  {
    public:
    static const T& Object()
    {
      /*#voodoo Create a region of memory that is as large as the type and stays
      zero (due to the return type being const). Callers of the method, must
      guarantee that zero-initialized memory for T is a valid state (without the
      actual constructor called) and that there are no mutable members in T.
      Note that in C++ statics are *guaranteed* to be zero-initialized first.
      See: §3.6.2/1.*/
      static byte EmptyField[sizeof(T)];
      return *reinterpret_cast<const T*>(EmptyField);
    }
  };

  /**Class that returns the notion of nothing for a given type. For integer
  types it returns the most negative number, for bool it returns false, for
  float types it returns NaN, for classes it returns an object created through
  its default constructor.*/
  template<class T>
  class Nothing
  {
    public:

    ///Returns the value of nothing for the given type.
    operator T() const
    {
      T DefaultValue;
      return DefaultValue;
    }

    ///Safely tests whether the other value is nothing.
    bool operator == (const T& Other) const
    {
      return T(Nothing<T>()) == Other;
    }

    ///Safely tests whether the other value is not nothing.
    bool operator != (const T& Other) const
    {
      return !(*this == Other);
    }
  };

  ///Yields the null pointer for a given type.
  template<class T>
  class Nothing <T*>
  {
    public:

    ///Returns the null pointer for the given type.
    operator T* () const
    {
      T* DefaultValue = 0;
      return DefaultValue;
    }

    ///Safely tests whether the other value is nothing.
    bool operator == (const T*& Other) const
    {
      return static_cast<T*>(Nothing<T*>()) == Other;
    }

    ///Safely tests whether the other value is not nothing.
    bool operator != (const T*& Other) const
    {
      return !(*this == Other);
    }
  };

  /*Provides the reverse comparison so that the Nothing operator == will be
  used instead of the regular T == operator. This is especially important for
  the float types which require special testing for NaN.*/
  template <class T> bool operator == (const T& lhs, const Nothing<T>& rhs)
  {
    return rhs == lhs;
  }

  template <class T> bool operator != (const T& lhs, const Nothing<T>& rhs)
  {
    return rhs != lhs;
  }

  //Set default values for Nothing<T> class.
  template <> inline Nothing<bool>::operator bool() const {return false;}
  template <> inline Nothing<uint8>::operator uint8() const
    {return Limits<uint8>::Min();}
  template <> inline Nothing<int8>::operator int8() const
    {return Limits<int8>::Min();}
  template <> inline Nothing<uint16>::operator uint16() const
    {return Limits<uint16>::Min();}
  template <> inline Nothing<int16>::operator int16() const
    {return Limits<int16>::Min();}
  template <> inline Nothing<uint32>::operator uint32() const
    {return Limits<uint32>::Min();}
  template <> inline Nothing<int32>::operator int32() const
    {return Limits<int32>::Min();}
#ifndef PRIM_ENVIRONMENT_LP64
  //Also provide definitions for the unused long type.
  template <> inline Nothing<unsigned long>::operator unsigned long() const
    {return Limits<unsigned long>::Min();}
  template <> inline Nothing<long>::operator long() const
    {return Limits<long>::Min();}
#endif
  template <> inline Nothing<uint64>::operator uint64() const
    {return Limits<uint64>::Min();}
  template <> inline Nothing<int64>::operator int64() const
    {return Limits<int64>::Min();}
  template <> inline Nothing<float32>::operator float32() const
    {return Limits<float32>::QuietNaN();}
  template <> inline bool Nothing<float32>::operator == (const float32& Other)
    const {return Limits<float32>::IsNaN(Other);}
  template <> inline Nothing<float64>::operator float64() const
    {return Limits<float64>::QuietNaN();}
  template <> inline bool Nothing<float64>::operator == (const float64& Other)
    const {return Limits<float64>::IsNaN(Other);}
  template <> inline Nothing<float80>::operator float80() const
    {return Limits<float80>::QuietNaN();}
  template <> inline bool Nothing<float80>::operator == (const float80& Other)
    const {return Limits<float80>::IsNaN(Other);}
}

#ifdef PRIM_COMPILE_INLINE
namespace PRIM_NAMESPACE
{
  template <class T> inline count Limits<T>::Bits()
  {
    return count(sizeof(T) * 8);
  }

  template <class T> inline count Limits<T>::Bytes()
  {
    return count(sizeof(T));
  }

  template <class T> inline T Limits<T>::Min()
  {
    return std::numeric_limits<T>::min();
  }

  template <class T> inline T Limits<T>::Max()
  {
    return std::numeric_limits<T>::max();
  }

  template <class T> inline T Limits<T>::HighBit()
  {
    if(Limits<T>::Integral())
    {
      if(Limits<T>::IsSigned())
        return Limits<T>::Min();
      else
        return Limits<T>::Max() / T(2) + T(1);
    }
    else
      return T(0);
  }

  template <class T> inline bool Limits<T>::Integral()
  {
    return std::numeric_limits<T>::is_integer;
  }

  template <class T> inline bool Limits<T>::Floating()
  {
    return !Limits<T>::Integral();
  }

  template <class T> inline bool Limits<T>::IsSigned()
  {
    return std::numeric_limits<T>::is_signed;
  }

  template <class T> inline bool Limits<T>::HasQuietNaN()
  {
    return std::numeric_limits<T>::has_quiet_NaN;
  }

  template <class T> inline T Limits<T>::QuietNaN()
  {
    return Limits<T>::HasQuietNaN() ? std::numeric_limits<T>::quiet_NaN() :
      Limits<T>::Min();
  }

  template <class T> inline bool Limits<T>::IsFloatingNaN(T x)
  {
    /* #voodoo: NaN comparison that has been tested and known to work better
    than other NaN testers. There should be a better way to do this though.

    For example, it might be better to detect __FAST_MATH__ and only run the
    below when that is turned on and for IEEE-compliant compiles simply use:

    return !(x > (T)0.0) and not (x < (T)1.0);
    */

    /* Portable warning-free NaN test:
      * Does not emit warning with -Wfloat-equal (no float comparisons)
      * Works for IEEE 754 compliant floating-point representations
      * Works with -ffast-math (floating-point optimization)
      * Only calls to standard library is memset and memcmp via <cstring>
      * Also works for extended precision long double
    */

    /*Initialize all bits including those used for alignment to zero. This sets
    all the values to positive zero but does not clue fast math optimizations as
    to the value of the variables.*/
    T z[4];
    memset(z, 0, sizeof(z)); //z[0] =  0.0, z[1 to 3] initialized to 0.0
    z[1] = T(-double(z[0])); //z[1] = -0.0
    z[2] = x;                //z[2] =  x
    z[3] = z[0] / z[2];      //z[3] =  0.0 / x

    /*Rationale for following test:
      * x is 0 or -0                                --> z[2] = 0, z[3] = NaN
      * x is a negative or positive number          --> z[3] = 0
      * x is a negative or positive denormal number --> z[3] = 0
      * x is negative or positive infinity          --> z[3] = 0
        (IEEE 754 guarantees that 0 / inf is zero)
      * x is a NaN                                  --> z[3] = NaN != 0.
    */

    //Do a bitwise comparison test for positive and negative zero.
    bool z2IsZero = memcmp(&z[2], &z[0], sizeof(T)) == 0 or
                    memcmp(&z[2], &z[1], sizeof(T)) == 0;

    bool z3IsZero = memcmp(&z[3], &z[0], sizeof(T)) == 0 or
                    memcmp(&z[3], &z[1], sizeof(T)) == 0;

    //If the input is bitwise zero or negative zero, then it is not NaN.
    return !z2IsZero and not z3IsZero;
  }

  template <class T> inline bool Limits<T>::IsNaN(T x)
  {
    if(Limits<T>::Integral())
    {
      T Reference = Limits<T>::QuietNaN();
      return Limits<T>::IsBitwiseEqual(x, Reference);
    }
    return false;
  }

  template <class T> inline bool Limits<T>::Bounded(T f)
  {
    if(Limits<T>::HasInfinity())
      return !Limits<T>::IsNaN(f) and
        !Limits<T>::IsBitwiseEqual(f, Limits<T>::Infinity()) and
        !Limits<T>::IsBitwiseEqual(f, Limits<T>::NegativeInfinity());
    else
      return !Limits<T>::IsNaN(f);
  }

  template <class T> inline bool Limits<T>::Unbounded(T f)
  {
    return !Limits<T>::Bounded(f);
  }

  template <class T> inline T Limits<T>::Zero()
  {
    return T(0);
  }

  template <class T> inline T Limits<T>::One()
  {
    return T(1);
  }

  template <class T> inline bool Limits<T>::IsZero(T f)
  {
    if(Limits<T>::Integral())
      return Limits<T>::IsBitwiseEqual(f, T(0));
    else
      return Limits<T>::IsBitwiseEqual(f, T(-0.)) or
        Limits<T>::IsBitwiseEqual(f, T(0));
  }

  template <class T> inline bool Limits<T>::IsNotZero(T f)
  {
    return !Limits<T>::IsZero(f);
  }

  template <class T> inline bool Limits<T>::IsBitwiseEqual(T f1, T f2)
  {
    return memcmp(&f1, &f2, sizeof(T)) == 0;
  }

  template <class T> inline bool Limits<T>::IsBitwiseNotEqual(T f1, T f2)
  {
    return !Limits<T>::IsBitwiseEqual(f1, f2);
  }

  template <class T> inline bool Limits<T>::HasInfinity()
  {
    return std::numeric_limits<T>::has_infinity;
  }

  template <class T> inline T Limits<T>::Infinity()
  {
    return Limits<T>::HasInfinity() ? std::numeric_limits<T>::infinity() :
      Limits<T>::Max();
  }

  template <class T> inline T Limits<T>::NegativeInfinity()
  {
    return Limits<T>::HasInfinity() ?
      T(-double(std::numeric_limits<T>::infinity())) :
      Limits<T>::Min();
  }

  //Explicitly instantiate the templates.
  template class Limits<uint8>;
  template class Limits<int8>;
  template class Limits<uint16>;
  template class Limits<int16>;
  template class Limits<uint32>;
  template class Limits<int32>;
  template class Limits<uint64>;
  template class Limits<int64>;
  template class Limits<float32>;
  template class Limits<float64>;
  template class Limits<float80>;
}
#endif
#endif
