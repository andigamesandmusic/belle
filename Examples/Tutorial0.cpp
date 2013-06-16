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

//------------------------------------------------------------------------------

/*
Tutorial 0: Fundamentals and prim.cc (read through carefully)

This tutorial explains how Belle, Bonne, Sage is included and compiled. It also
shows features of the prim.cc library on which Belle depends. It does not show
any features specific to Belle, Bonne, Sage.

On Mac/Linux you can build and run from the terminal using:
Scripts/MakeAndRun Tutorial0

For more information related to building, see the README.
*/

//------------------------------------------------------------------------------

/*Step 1: Including Belle, Bonne, Sage
There are two main rules to including Belle, Bonne Sage:

1. Include Belle.h in each .cpp that needs it. Here ../Belle... is used
because the file happens to be in the parent directory; however, the relative
path will differ depending on how your project has been set up.

/Belle.h includes the whole library which is located in /Source and its
subdirectories.

2. #define BELLE_COMPILE_INLINE in one and only one .cpp file, such as
the main .cpp file or the file that contains int main(), above the
Belle.h include.

If you look at the files in /Source you will notice something which may look a
little peculiar at first: there are no .cpp files. This is done primarily for
organizational reasons. By keeping the function definitions (bodies) with the
function declarations, one may quickly inspect a method to see what it does. For
large classes this can make some files unruly, so it is highly recommended that
you use the Doxygen-generated reference material to get an overview of a class.

While most of the code is perfectly happy to keep definition with declaration,
there are times when this is impossible, for example due to mutual dependencies.
Therefore, some code is blocked off as "inline compile":
#ifdef BELLE_COMPILE_INLINE
//code for declarations which are not with definitions
#endif

The _COMPILE_INLINE paradigm is also used in the prim.cc and MICA sublibraries,
however, Belle will automatically trigger the _COMPILE_INLINE switch for these
sublibraries.
*/
#define BELLE_COMPILE_INLINE
#include "Belle.h"

//------------------------------------------------------------------------------

/*Step 2: Namespaces
Belle, Bonne, Sage uses the prim.cc (http://prim.cc/doc) library for all of its
primitive objects. prim.cc is like a dialect of C++ built for rapid prototyping,
and tries to provide an organized, high-level interface to do the most common
programming tasks.

prim.cc and Belle use namespaces to prevent symbol leakage and name collisions.
In fact, prim.cc is designed in such a way that including prim.h will not bring
any symbols into the global namespace (except for the prim namespace), and so
the global namespace is free of all pollution caused by the C++ standard
libraries. Belle inherits this philosophy.

For small projects like this tutorial, it is perfectly fine to use the
'using namespace ...;' keyword to simply bring the relevant namespaces into the
global namespace. For larger projects, especially those including other
libraries, you will either want to not use 'using namespace', or create a
synthetic namespace that contains everything.

For example:
namespace bbs
{
  using namespace prim;
  using namespace prim::planar;
  using namespace belle;
  using namespace belle::painters;
}

Below is an explanation of what each namespace does:
*/

/*The core prim.cc library (http://prim.cc/doc/namespaceprim.html). It contains
fundamental classes like Array, Console, Complex, File, Graph, List, Matrix,
Memory, Pointer, Random, Ratio(nal), Serial, and String.*/
using namespace prim;

/*The planar namespace in prim contains classes relevant to two-dimensional
surfaces: Vector(Int), Rectangle(Int), Bezier, Ellipse, Line, Polygon.*/
using namespace prim::planar;

/*The core belle namespace.
It contains classes relevant to drawing such as Affine, Canvas, Color, Font,
Painter, Path, Portfolio, Shapes, Text.*/
using namespace belle;

//Belle has output painters which are rendering targets such as PDF and JUCE.
using namespace belle::painters;

//------------------------------------------------------------------------------

/*Step 3: int main() -- as always your program starts here, but notice that we
do not need the standard library (cout, cin, string, vector, etc.) since most of
the commonly used functionality is already in prim.*/

//This program shows how prim can be used.
int main()
{
  /*The following objects all belong to prim:: or prim::planar:: (for Vector).
  The 'using namespace' statements above make them unnecessary.*/
  
  //----------//
  //Data Types//
  //----------//
  {
    //There are two fundamental abstract types which are used pervasively:
    count a = 1; //Signed integer set to int32/int64 depending on pointer size
    number b = 3.4; //Floating-point set to float64 by default
    
    //The count is used anywhere counting is needed, for example in for-loops:
    for(count i = 0; i < 10; i++)
    {
    }
    
    //The number is used wherever floating-point calculation is done.
    number x = Cos(Pi / 2.0);
    
    /*Note that count and number are roughly equivalent to the intention of
    'int' and 'double'. However, 'count' stores more like a 'long' (or
    'long long' in 64-bit Windows).*/
    
    /*The count is not intended to be used for integer math. Instead use
    integer, which always uses the largest available int type.*/
    integer y = 123456789;
    y *= 987654321;
    
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
    a = 0; b = 0; d = 0; e = 0; f = 0; g = 0; h = 0; i = 0; j = 0;
    k = 0; l = 0; m = 0; n = 0; o = 0; p = 0; q = 0; r = 0; x = 0;
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
  c >> "Hello " << "World!";
  
  //Adds a blank line to the output (and creates a fortuitous pun).
  c++;

  /*Editorial note: it may seem a little screwy using >> and << for append if
  you are used to the standard streams. However, the upshot is you do not need
  to constantly terminate with end lines, and in this developer's mind, breaking
  with convention significantly improves the readability of most string code.*/
  
  //Consoles and strings are overloaded to take most built-in data types.
  c >> 5;
  c >> 1.4;
  c >> Ratio(4, 8);
  c >> Vector(3.4, 4.5);
  c >> Array<number>();
  c >> List<number>();
  c >> Cos(Pi);
  c >> Sqrt(9.0);
  
  /*Note that Vector is just short for Complex<number> (and VectorInt for
  Complex<integer>). Vector is completely unrelated to the C++ standard
  library's notion of vector which is like a prim Array. Vector comes from
  prim::planar:: and so it is always two-dimensional.*/
  
  //The decimal precision of each String object can be controlled:
  c >> Pi; //Default precision (5 decimal digits)
  c.Precision(10);
  c >> Pi; //10 decimal digits
  c.Precision(2);
  c >> Pi; //2 decimal digits
  
  //----------------//
  //Lists and Arrays//
  //----------------//
  
  /*Lists and Arrays are templated types which means they are declared with an
  element type in angle brackets.*/
  Array<number> A;
  A.Add(10.0); //Pass the new value into...
  A.Add() = 20.0; //...or assign to returned reference, whichever you prefer.
  A.Add() = 30.0;
  c >> A;
  
  List<number> L;
  L.Add() = 40.0; //Lists always assign the new value to the returned reference.
  L.Add() = 50.0;
  L.Add() = 60.0;
  c >> L;
  
  /*Lists and Arrays use n() to get the number of elements and a() and z() to
  get the first and last elements.*/
  c >> "Array: " << A.n() << " elements: " << A.a() << ", ..., " << A.z();
  c >> "List : " << L.n() << " elements: " << L.a() << ", ..., " << L.z();
  
  //Iterating over an array or list typically looks like:
  for(count i = 0; i < A.n(); i++)
    c >> i << ": " << A[i];

  /*This is most of what you need to know to intuit code written in prim. To use
  it yourself, you would want to go to http://prim.cc/doc and examine the
  classes and methods described there.*/

  //Finish the console output.
  c.Finish();
  
  return 0;
}
