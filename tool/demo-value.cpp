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

#define PRIM_COMPILE_INLINE
#include "prim.h"

using namespace prim;

/*Example class for object value type. If you overload operator String() then
you'll be able to see the custom string data when a Value is printed.*/
class Foo : public Value::Base
{
  public:
  operator String() {return "foobar";}
  virtual ~Foo();
};

Foo::~Foo() {}

int main()
{
  //Create a new empty value
  Value b;

  //Set the value to an integer.
  b = 123;

  //Test whether the value contains something non-nil.
  if(b)
    C::Out() >> "A: " << "Value is not nil";

  //Get the floating-point value of the value's integer.
  number z = b;
  C::Out() >> "B: " << z;
  C::Out()++;

  //Set the value to a floating-point number.
  b = 123.4567890123456789012345 / 10000000.0;
  C::Out() >> "C: " << b;
  C::Out()++;

  //Clear the value (sets to nil).
  b.Clear();
  C::Out() >> "D: " << b;
  C::Out()++;

  //Set value to a boolean.
  b = false;
  C::Out() >> "E: " << b;
  C::Out()++;

  //Set value to a string.
  b = "xyz";
  C::Out() >> "F: " << b;

  //Set value to a ratio.
  b = Ratio(16, 5);
  C::Out() >> "G: " << b;

  //Notice how ratios are specially encoded in JSON.
  C::Out() >> "H: " << b.ExportJSON();
  C::Out()++;

  //Set value to a vector.
  b = Vector(3.4f, 7.2f);
  C::Out() >> "I: " << b;

  //Notice how vectors are specially encoded in JSON.
  C::Out() >> "J: " << b.ExportJSON();
  C::Out()++;

  //Set value to a rectangle.
  b = Box(Vector(3.f, 4.f), Vector(5.f, 6.7f));
  C::Out() >> "K: " << b;

  //Notice how rectangles are specially encoded in JSON.
  C::Out() >> "L: " << b.ExportJSON();
  C::Out()++;

  //Certain numeric values are coerced to nil, such as NaN and 0/0
  b = number(Nothing<float64>());
  C::Out() >> "M: " << b;
  b = Ratio(0, 0);
  C::Out() >> "N: " << b;
  C::Out()++;

  //If the value is indexed with non-negative integers, it turns into an array.
  b[0] = 1;
  b[2] = 2;
  b[5] = 3;
  C::Out() >> "O: " << b;
  C::Out() >> "O.n(): " << b.n();
  b.n(10);
  C::Out() >> "O: " << b;
  C::Out() >> "O.n(): " << b.n();
  b["now"] = "table";
  C::Out() >> "O: " << b;
  C::Out() >> "O.n(): " << b.n();
  b.n(10);
  C::Out() >> "O: " << b;
  C::Out() >> "O.n(): " << b.n();

  /*If however, you start indexing with things other than non-negative integers,
  the value is cleared and replaced by a tree.*/
  b[-1] = 5;
  b["xyz"] = 4;
  C::Out() >> "P: " << b;
  C::Out()++;

  //Create an empty value.
  Value a;

  //Set the value to an empty tree.
  a.NewTree();

  //Set a[3] to 1.
  a[3] = 1;

  //Set a[4] to 13.
  a[4] = 13;

  //Set a[5] to array with 3rd element set to 0.
  a[5][3] = 0;

  //Change a[5] to a tree (now empty) and set a[5]={"xyz":1}
  a[5]["xyz"] = 1;

  /*Sets a key which is a pointer to an object derived from Value::Base and
  sets the corresponding value to 3.*/
  a[Pointer<Foo>(new Foo)] = 3;

  /*You can do pretty weird things. Keys can be any value, including other
  trees. Cool! The value of the key is deep-copied before being used for
  lookup. Pretty weird, huh?*/
  a[5]["hi"][b] = Pointer<Foo>(new Foo);

  //See what this looks like...
  C::Out() >> "Q: " << a;
  C::Out()++;

  //You can even do horrible partially self-reflexive assignments.
  a[1] = a;
  a[a] = 1;
  a[a] = a;
  C::Out() >> "R: " << a;
  C::Out()++;

  /*(However, you should know that a[a] = a is an O(3^n) operation due to the
  deep-copy of intermediate key values. Ouch!)*/

  //You can also store references to other values.
  Pointer<Value> c_Object = new Value;
  Value& c = *c_Object;
  Value d;
  c = "this";
  d["that"] = new Value::ConstReference(c_Object);
  C::Out() >> "c: " << c;
  C::Out() >> "d: " << d;

  Pointer<Value::ConstReference> vr = d["that"].Object();
  C::Out() >> "d[that]: " << vr->Get();

  C::Out()++;

  //You can treat a value like a property tree.
  Value e, f;
  e["abc"] = 123;
  e["foo"] = "bar";
  e["nested"]["abc"] = 123;
  e["nested"]["foo"] = "bar";
  f["abc"] = "def";
  f["nested"]["abc"] = "def";
  C::Out() >> e;
  C::Out() >> f;
  e.Merge(f);
  C::Out() >> e;

  //Release the console singleton.
  return AutoRelease<Console>();
}
