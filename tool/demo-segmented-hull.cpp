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
  {
    Array<Box> R;
    R.Add() = Box(Vector(0.0, 2.0), Vector(10.0, 4.0));
    R.Add() = Box(Vector(4.0, 0.0), Vector( 6.0, 6.0));

    C::Out() >> "Box array:";
    C::Out() >> R;
    C::Out()++;
    C::Out() >> "Top side:";
    C::Out() >> Box::SegmentedHull(R, Box::TopSide);
    C::Out() >> "Left side:";
    C::Out() >> Box::SegmentedHull(R, Box::LeftSide);
    C::Out() >> "Bottom side:";
    C::Out() >> Box::SegmentedHull(R, Box::BottomSide);
    C::Out() >> "Right side:";
    C::Out() >> Box::SegmentedHull(R, Box::RightSide);
  }

  {
    List<Vector> A, B;
    A.Add() = Vector( 5.0,  0.0);
    A.Add() = Vector(10.0,  5.0);
    A.Add() = Vector( 5.0, 10.0);
    A.Add() = Vector( 5.0, 15.0);

    B.Add() = Vector( 2.0,  0.0);
    B.Add() = Vector(10.0,  6.0);
    B.Add() = Vector( 2.0,  8.0);
    B.Add() = Vector( 2.0, 15.0);

    C::Out()++;
    C::Out() >> "Anchor: " << A;
    C::Out() >> "Mover:  " << B;
    C::Out()++;
    C::Out() >> "Offset to place top:    ";
    C::Out() << Box::OffsetToPlaceOnSide(A, B, Box::TopSide);
    C::Out() >> "Offset to place left:   ";
    C::Out() << Box::OffsetToPlaceOnSide(A, B, Box::LeftSide);
    C::Out() >> "Offset to place bottom: ";
    C::Out() << Box::OffsetToPlaceOnSide(A, B, Box::BottomSide);
    C::Out() >> "Offset to place right:  ";
    C::Out() << Box::OffsetToPlaceOnSide(A, B, Box::RightSide);
  }

  {
    List<Vector> A, B;
    A.Add() = Vector( 0.0,  0.0);
    A.Add() = Vector( 5.0,  5.0);
    A.Add() = Vector( 0.0, 10.0);
    A.Add() = Vector( 0.0, 15.0);

    B.Add() = Vector( 2.0,  3.0);
    B.Add() = Vector( 2.0, 12.0);

    C::Out()++;
    C::Out() >> "Anchor: " << A;
    C::Out() >> "Mover:  " << B;
    C::Out() >> "Merged: " << Box::MergeHulls(A, B, Box::RightSide);
  }

  return AutoRelease<Console>();
}
