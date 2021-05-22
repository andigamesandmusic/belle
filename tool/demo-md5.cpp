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
  String s;

  s = "The quick brown fox jumps over the lazy dog";
  C::Out() >> "'" << s << "'";
  C::Out() >> "Calculated: " << MD5::Hex(s);
  C::Out() >> "Actual:     " << "9e107d9d372bb6826bd81d3542a419d6";
  C::Out()++;

  s = "";
  C::Out() >> "'" << s << "'";
  C::Out() >> "Calculated: " << MD5::Hex(s);
  C::Out() >> "Actual:     " << "d41d8cd98f00b204e9800998ecf8427e";
  C::Out()++;

  s = "a";
  C::Out() >> "'" << s << "'";
  C::Out() >> "Calculated: " << MD5::Hex(s);
  C::Out() >> "Actual:     " << "0cc175b9c0f1b6a831c399e269772661";
  C::Out()++;

  s = "abc";
  C::Out() >> "'" << s << "'";
  C::Out() >> "Calculated: " << MD5::Hex(s);
  C::Out() >> "Actual:     " << "900150983cd24fb0d6963f7d28e17f72";
  C::Out()++;

  s = "message digest";
  C::Out() >> "'" << s << "'";
  C::Out() >> "Calculated: " << MD5::Hex(s);
  C::Out() >> "Actual:     " << "f96b697d7cb7938d525a2f31aaf161d0";
  C::Out()++;

  s = "abcdefghijklmnopqrstuvwxyz";
  C::Out() >> "'" << s << "'";
  C::Out() >> "Calculated: " << MD5::Hex(s);
  C::Out() >> "Actual:     " << "c3fcd3d76192e4007dfb496cca67e13b";
  C::Out()++;

  s = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  C::Out() >> "'" << s << "'";
  C::Out() >> "Calculated: " << MD5::Hex(s);
  C::Out() >> "Actual:     " << "d174ab98d277d9f5a5611c2c9f419d9f";
  C::Out()++;

  s = "123456789012345678901234567890123456789012345678901234567890123456"
    "78901234567890";
  C::Out() >> "'" << s << "'";
  C::Out() >> "Calculated: " << MD5::Hex(s);
  C::Out() >> "Actual:     " << "57edf4a22be3c955ac49da2e2107b67a";
  return AutoRelease<Console>();
}
