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

#ifndef PRIM_INCLUDE_UNICODE_H
#define PRIM_INCLUDE_UNICODE_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///Utility class for Unicode and text operations
  class Unicode
  {
    public:

    ///Returns the Unicode codepoint for a replacement character.
    static unicode ReplacementCharacter()
    {
      return unicode(0xfffd);
    }

    /**Returns the hex digit value of the character. If the character is not a
    hex digit then 0x10 (16) is returned. Both upper and lowercase are accepted
    for the digits a-f (10-15).*/
    static byte HexDigitValue(unicode Codepoint)
    {
      static const byte HexMap['g'] =
      {
        16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 16, 16, 16, 16, 16, 16,
        16, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
        16, 10, 11, 12, 13, 14, 15
      };
      return Codepoint < 'g' ? HexMap[Codepoint & 0xff] : 16;
    }

    /**Returns whether the character is a valid hex digit. Both upper and
    lowercase are accepted for the digits a-f (10-15).*/
    static bool IsHexDigitValue(unicode Codepoint)
    {
      return HexDigitValue(Codepoint) < 16;
    }

    ///Returns whether the codepoint is a lead surrogate.
    static bool IsLeadSurrogate(unicode Codepoint)
    {
      return Codepoint >= 0xd800 and Codepoint <= 0xdbff;
    }

    ///Returns whether the codepoint is a lead surrogate.
    static bool IsTrailSurrogate(unicode Codepoint)
    {
      return Codepoint >= 0xdc00 and Codepoint <= 0xdfff;
    }

    ///Returns the combined codepoint of a surrogate.
    static unicode FromSurrogatePair(unicode Lead, unicode Trail)
    {
      if(not IsLeadSurrogate(Lead) or not IsTrailSurrogate(Trail))
        return ReplacementCharacter();

      return (Trail - 0xdc00) + ((Lead - 0xd800) << 10) + 0x10000;
    }

    ///Returns whether the codepoint is expressed as a surrogate pair in UTF-16.
    static bool NeedsSurrogatePair(unicode Codepoint)
    {
      return Codepoint >= 0x10000;
    }

    /**Returns the lead surrogate of a codepoint. If codepoint is not expressed
    with surrogate pairs, then the replacement character is returned.*/
    static unicode ToLeadSurrogate(unicode Codepoint)
    {
      if(not NeedsSurrogatePair(Codepoint))
        return ReplacementCharacter();

      return ((Codepoint - 0x10000) >> 10) + 0xd800;
    }

    /**Returns the tail surrogate of a codepoint. If codepoint is not expressed
    with surrogate pairs, then the replacement character is returned.*/
    static unicode ToTailSurrogate(unicode Codepoint)
    {
      if(not NeedsSurrogatePair(Codepoint))
        return ReplacementCharacter();

      return ((Codepoint - 0x10000) & 0x3ff) + 0xdc00;
    }
  };
}
#endif
