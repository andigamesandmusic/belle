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

int main()
{
  /*Make a bunch of UUIDv4s (these are not valid UUIDv4s since they do not have
  the type information embedded, so do not use them.)*/

  //Test importing UUIDv4 strings.
  String Good1 = "00e60c24-6dfc-11e1-b23c-1f66cb788b8c";
  String Good2 = "00E60C24-6DFC-11E1-B23C-1F66CB788B8C";
  String Good3 = "00e60c24-6dfc-11e1-B23C-1F66CB788B8C";
  String Bad1  = "00e60c24-6dfc-11e1-b23c-1f66cb788b8c2";
  String Bad2  = "00e60c24-6dfc-11e1-b23c-1f66cb788b";
  String Bad3  = "00e60c24-6dfcx11e1-b23c-1f66cb788b8c";
  C::Out() >> "Valid UUIDv4 Conversions:";
  C::Out() >> Good1 >> UUIDv4(Good1);
  C::Out()++;
  C::Out() >> Good2 >> UUIDv4(Good2);
  C::Out()++;
  C::Out() >> Good3 >> UUIDv4(Good3);
  C::Out()++;
  C::Out() >> "Invalid UUIDv4 Conversions:";
  C::Out() >> Bad1 >> UUIDv4(Bad1);
  C::Out()++;
  C::Out() >> Bad2 >> UUIDv4(Bad2);
  C::Out()++;
  C::Out() >> Bad3 >> UUIDv4(Bad3);
  C::Out()++;

  //Test sorting UUIDv4s.
  Sortable::Array<UUIDv4> a;
  a.n(20);
  a.Sort();
  C::Out() >> "Sorted UUIDv4s:";
  for(count i = 0; i < a.n(); i++)
    C::Out() >> a[i];

  C::Out()++;
  C::Out() >> "MD5 UUIDv4:";
  C::Out() >> Good1 << " + " << "foo";
  C::Out() >> "Calculated: " <<
    UUIDv4("00e60c24-6dfc-11e1-b23c-1f66cb788b8c", "foo");
  C::Out() >> "Reference : dc0b557a-d84b-33c1-a936-05ec762cd4a0";
  C::Out() >> Good1 << " + " << "foobar";
  C::Out() >> "Calculated: " <<
    UUIDv4("00e60c24-6dfc-11e1-b23c-1f66cb788b8c", "foobar");
  C::Out() >> "Reference : bbc2142d-e63f-3ded-8631-56e3ae0266f2";

  return AutoRelease<Console>();
}
