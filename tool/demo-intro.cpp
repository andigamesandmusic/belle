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

//------------------------------------------------------------------------------

/*
Intro: Fundamentals of prim

This tutorial explains how prim is included and compiled.
*/

//------------------------------------------------------------------------------

/*Step 1: Including prim
1. Include prim.h in each .cpp that needs it.

2. #define PRIM_COMPILE_INLINE in one and only one .cpp file, such as
the main .cpp file or the file that contains int main(), above the
prim.h include.
*/
#define PRIM_COMPILE_INLINE
#include "prim.h"

//------------------------------------------------------------------------------

//Step 2: Namespaces ('using namespace' here for convenience)

/*The core prim library contains fundamental classes like Array, Console,
Complex, File, Graph, List, Matrix, Memory, Pointer, Random, Ratio(nal),
Serial, and String.*/
using namespace prim;

//------------------------------------------------------------------------------

/*Step 3: int main() -- as always your program starts here, but notice that we
do not need the standard library (cout, cin, string, vector, etc.) since most of
the commonly used functionality is already in prim.*/

//This program shows how prim can be used.
int main()
{
  /*The following objects all belong to the prim namespace. The
  using namespace statement above makes scoping them unnecessary.*/

  //----------//
  //Data Types//
  //----------//
  {
    //There are two fundamental abstract types which are used pervasively:
    count a = 1; //Signed integer set to int32/int64 depending on pointer size
    number b = number(3.4); //Floating-point set to float64 by default

    //The count is used anywhere counting is needed, for example in for-loops:
    for(count i = 0; i < 10; i++)
    {
    }

    //The number is used wherever floating-point calculation is done.
    number x = Cos(Pi<number>() / 2.f);

    /*Note that count and number are roughly equivalent to the intention of
    'int' and 'double'. However, 'count' stores more like a 'long' (or
    'long long' in 64-bit Windows).*/

    /*The count is not intended to be used for integer math. Instead use
    integer, which always uses the largest available int type.*/
    integer y = 123456789;

    //For char and unsigned char use ascii and byte instead:
    ascii d = 'd';
    const ascii* e = "string";
    byte f = 0xFF;

    //For Unicode code-points use unicode:
    unicode g = 0x0416; //Cyrillic 'Zhe'

    //You can always specify word size exactly using the familiar forms:
    uint8 h;
    int8 i;
    uint16 j;
    int16 k;
    uint32 l;
    int32 m;
    uint64 n;
    int64 o;
    float32 p;
    float64 q;
    float80 r;

    /*While not a built-in type, the Ratio and Complex<T> class are also useful
    prim types that are fully overloaded so that they can be used like built-in
    types.*/
    Ratio s(3, 9); //Auto-reduces to 1/3
    s.Numerator(); //Gets numerator
    s.Denominator(); //Gets denominator

    Complex<number> t(1.0, 2.0); //Represents 1 + 2i
    Complex<number> u(-1.0, 2.0);
    t = t + u; //Complex addition
    t = t * u; //Complex multiplication
    t = t / u; //Complex division
    t = t - u; //Complex subtraction
    t.Mag(); //Gets magnitude
    t.Ang(); //Gets angle

    //Hide some warnings about unused parameters (ignore this).
    (void)a; (void)b; (void)d; (void)e; (void)f; (void)g; (void)h; (void)i;
    (void)j; (void)k; (void)l; (void)m; (void)n; (void)o; (void)p; (void)q;
    (void)r; (void)x; (void)y;
  }

  //------------------//
  //String and Console//
  //------------------//

  /*Note that Console is a subclass of String and so everything here equally
  applies to String. For example, uncommenting the following will simply direct
  the values into a String object (and silence the console output).*/
  //String c;

  /*Print-out with global Console object prim::c. Note << means "append" and >>
  means "append on a new line".*/
  C::Out() >> "Hello " << "World!";

  //Adds a blank line to the output (and creates a fortuitous pun).
  C::Out()++;

  /*Editorial note: it may seem a little screwy using >> and << for append if
  you are used to the standard streams. However, the upshot is you do not need
  to constantly terminate with end lines, and in this developer's mind, breaking
  with convention significantly improves the readability of most string code.*/

  //Consoles and strings are overloaded to take most built-in data types.
  C::Out() >> 5;
  C::Out() >> 1.4;
  C::Out() >> Ratio(4, 8);
  C::Out() >> Vector(number(3.4), number(4.5));
  C::Out() >> Array<number>();
  C::Out() >> List<number>();
  C::Out() >> Cos(Pi<number>());
  C::Out() >> Sqrt(9.0);

  /*Note that Vector is just short for Complex<number> (and VectorInt for
  Complex<integer>). Vector is completely unrelated to the C++ standard
  library's notion of vector which is like a prim Array. Vector comes from
  prim:: and so it is always two-dimensional.*/

  //The decimal precision of each String object can be controlled:
  C::Out() >> Pi<number>(); //Default precision (5 decimal digits)
  C::Out().Precision(10);
  C::Out() >> Pi<number>(); //10 decimal digits
  C::Out().Precision(2);
  C::Out() >> Pi<number>(); //2 decimal digits

  //----------------//
  //Lists and Arrays//
  //----------------//

  /*Lists and Arrays are templated types which means they are declared with an
  element type in angle brackets.*/
  Array<number> A;
  A.Add(10.0); //Pass the new value into...
  A.Add() = 20.0; //...or assign to returned reference, whichever you prefer.
  A.Add() = 30.0;
  C::Out() >> A;

  List<number> L;
  L.Add() = 40.0; //Lists always assign the new value to the returned reference.
  L.Add() = 50.0;
  L.Add() = 60.0;
  C::Out() >> L;

  /*Lists and Arrays use n() to get the number of elements and a() and z() to
  get the first and last elements.*/
  C::Out() >> "Array: " << A.n() << " elements: " << A.a() <<
    ", ..., " << A.z();
  C::Out() >> "List : " << L.n() << " elements: " << L.a() <<
    ", ..., " << L.z();

  //Iterating over an array or list typically looks like:
  for(count i = 0; i < A.n(); i++)
    C::Out() >> i << ": " << A[i];

  //Add another blank line so that the terminal begins on a new line upon quit.
  return AutoRelease<Console>();
}
