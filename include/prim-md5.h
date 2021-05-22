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

#ifndef PRIM_INCLUDE_MD5_H
#define PRIM_INCLUDE_MD5_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

/*Module Configuration
================================================================================

Works on both little-endian and big-endian processors provided that the
PRIM_ENVIRONMENT_LITTLE_ENDIAN or PRIM_ENVIRONMENT_BIG_ENDIAN define is
correctly set.

================================================================================
End Module Configuration*/

namespace PRIM_NAMESPACE
{
  ///Calculates MD5 checksums.
  class MD5
  {
    public:

    ///Calculates an MD5 checksum on a string of byte data.
    static void Calculate(const byte* Data, int64 Length, uint32 Out[4])
    {
      uint32 g[192] = { //64 magic numbers each at g[0], g[64], and g[128]
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
        0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
        0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
        0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
        0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
        0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
        0, 1,  2,  3, 4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
        1, 6, 11,  0, 5, 10, 15,  4,  9, 14,  3,  8, 13,  2,  7, 12,
        5, 8, 11, 14, 1,  4,  7, 10, 13,  0,  3,  6,  9, 12, 15,  2,
        0, 7, 14, 5, 12,  3, 10,  1,  8, 15,  6, 13,  4, 11,  2,  9};

      uint32 a  = Out[0] = 0x67452301, //Hash state
             b  = Out[1] = 0xefcdab89,
             c  = Out[2] = 0x98badcfe,
             d  = Out[3] = 0x10325476,
             x, w[16];

      int64 q = 1, j, i; //Loop variables

      byte p[128] = {0}; p[0] = 0x80; //Message padding

      int64* p64 = 0;

      while((Length + q) % 64 != 56) //Calculate padding
        p[q++] = 0;
      byte* p64AsByte = &p[q];
      Memory::MemCopy(&p64, &p64AsByte, sizeof(p64));
      int64 Length_8 = Length * 8;
#ifdef PRIM_ENVIRONMENT_BIG_ENDIAN
      Endian::ReverseOrder(Length_8);
#endif
      Memory::MemCopy(p64, &Length_8, sizeof(Length_8));

      for(j = -Length; j < q + 8;)
      {
        for(i = 0; i < 64; i++, j++) //Read in 64-bytes of message
          reinterpret_cast<byte*>(w)[i] = j < 0 ? *Data++ : p[j];
#ifdef PRIM_ENVIRONMENT_BIG_ENDIAN
        for(i = 0; i < 16; i++)
          Endian::ReverseOrder(w[i]);
#endif
        for(i = 0; i < 64; i++) //Calculate chunk hash
        {
          if(i < 16)      x = (b & c) | (~b & d);
          else if(i < 32) x = (d & b) | (~d & c);
          else if(i < 48) x = (b ^ c) ^ d;
          else            x =  c ^ (b | ~d);

          x += a + g[i + 64] + w[g[i + 128]];
          a = d;
          d = c;
          c = b;
          b += (x << g[i]) | (x >> (32 - g[i]));
        }

        a = Out[0] += a; //Sum chunk hash to output hash
        b = Out[1] += b;
        c = Out[2] += c;
        d = Out[3] += d;
      }
#ifdef PRIM_ENVIRONMENT_BIG_ENDIAN
      for(i = 0; i < 4; i++)
        Endian::ReverseOrder(Out[i]);
#endif
    }

    ///Returns a hexadecimal MD5 checksum to a string of byte data.
    static String Hex(const byte* Start, count Length, bool Uppercase = false)
    {
      Array<uint32> Digest;
      Digest.n(4);
      Calculate(Start, Length, &Digest.a());
      Array<byte> HexArray;
      Digest.EncodeAsASCIIHex(HexArray);
      if(not Uppercase)
        for(count i = 0; i < HexArray.n(); i++)
          if(HexArray[i] >= 'A' and HexArray[i] <= 'F')
            HexArray[i] += 'a' - 'A';
      return String(&HexArray.a(), HexArray.n());
    }

    ///Returns a hexadecimal MD5 checksum to an array of byte data.
    static String Hex(const Array<byte>& Data, bool Uppercase = false)
    {
      return Hex(&Data.a(), Data.n(), Uppercase);
    }

    ///Returns a hexadecimal MD5 checksum to a character string.
    static String Hex(const String& DataString, bool Uppercase = false)
    {
      return Hex(reinterpret_cast<const byte*>(DataString.Merge()),
        DataString.n(), Uppercase);
    }
  };
}
#endif
