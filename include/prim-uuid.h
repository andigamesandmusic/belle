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

#ifndef PRIM_INCLUDE_UUIDV4_H
#define PRIM_INCLUDE_UUIDV4_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///UUIDv4 generation, storage, import, and export.
  class UUIDv4
  {
    ///Random number generator.
    Random& RandomSequence()
    {
      static Random r;
      return r;
    }

    ///Lookup table for hexadecimal to digit conversion.
    static const byte HexMap[256];

    ///Converts a byte into two hexadecimal digits.
    static void ToHex(byte Data, ascii& High, ascii& Low)
    {
      ascii DataHigh = ascii(Data >> 4);
      ascii DataLow = ascii(Data % 16);
      High = (DataHigh < 10 ? DataHigh + '0' : (DataHigh - 10) + 'a');
      Low = (DataLow < 10 ? DataLow + '0' : (DataLow - 10) + 'a');
    }

    ///Simultaneously use 16 octet and double 64-bit word representation.
    union
    {
      byte   Octet [16];
      uint64 HighLow[2];
    };

    ///Imports the UUIDv4 from a string.
    void Import(const ascii* UUIDv4String)
    {
      //Convert to byte string to make array indexing safer.
      const byte* s = reinterpret_cast<const byte*>(UUIDv4String);

      //Initialize the UUIDv4 in case of failure.
      HighLow[0] = HighLow[1] = 0;

      //Check the string to make sure it is a valid UUIDv4.
      if(not IsUUIDv4(UUIDv4String))
        return;

      //The UUIDv4 is guaranteed to be valid now so import it using the hex map.
      Octet[0]  = byte((HexMap[s[ 0]] << 4) + HexMap[s[ 1]]);
      Octet[1]  = byte((HexMap[s[ 2]] << 4) + HexMap[s[ 3]]);
      Octet[2]  = byte((HexMap[s[ 4]] << 4) + HexMap[s[ 5]]);
      Octet[3]  = byte((HexMap[s[ 6]] << 4) + HexMap[s[ 7]]);
      Octet[4]  = byte((HexMap[s[ 9]] << 4) + HexMap[s[10]]);
      Octet[5]  = byte((HexMap[s[11]] << 4) + HexMap[s[12]]);
      Octet[6]  = byte((HexMap[s[14]] << 4) + HexMap[s[15]]);
      Octet[7]  = byte((HexMap[s[16]] << 4) + HexMap[s[17]]);
      Octet[8]  = byte((HexMap[s[19]] << 4) + HexMap[s[20]]);
      Octet[9]  = byte((HexMap[s[21]] << 4) + HexMap[s[22]]);
      Octet[10] = byte((HexMap[s[24]] << 4) + HexMap[s[25]]);
      Octet[11] = byte((HexMap[s[26]] << 4) + HexMap[s[27]]);
      Octet[12] = byte((HexMap[s[28]] << 4) + HexMap[s[29]]);
      Octet[13] = byte((HexMap[s[30]] << 4) + HexMap[s[31]]);
      Octet[14] = byte((HexMap[s[32]] << 4) + HexMap[s[33]]);
      Octet[15] = byte((HexMap[s[34]] << 4) + HexMap[s[35]]);
    }

    public:

    ///Returns the UUIDv4 styled as a 32-character lowercase hash.
    String AsHash() const
    {
      String s = String(*this).ToLower();
      s.Replace("-", "");
      return s;
    }

    /**Checks to see whether the string is in the canonical UUIDv4 format. Both
    lowercase and uppercase (and mixtures of both) of letters A-F are allowed. A
    UUIDv4 should look like 00112233-4455-6677-8899-aabbccddeeff, or in other
    words: 8 digits, hyphen, 4 digits, hyphen, 4 digits, hyphen, 4 digits,
    hyphen, 12 digits, and a null-terminator. This method is designed to be safe
    so that even if random strings are passed in, it will fail within testing
    the first 37 characters.*/
    static bool IsUUIDv4(const ascii* UUIDv4String)
    {
      //Convert to byte string to make array indexing safer.
      const byte* s = reinterpret_cast<const byte*>(UUIDv4String);

      //Go through each character and stop at the first sign of trouble.
      for(count i = 0; i < 36; i++)
      {
        //Ensure that the hyphens are where they are supposed to be.
        if(i == 8 or i == 13 or i == 18 or i == 23)
        {
          if(s[i] != '-')
            return false;
        }
        //Ensure that the other characters are digits. See HexMap definition.
        else if(HexMap[s[i]] >= 16)
          return false;
      }

      /*Ensure the string is the proper length without calculating the string
      length in case it is huge (must fail if not proper at the first invalid
      character). If the 36th character is not a null-terminator, the string is
      invalid.*/
      if(s[36])
        return false;

      //UUIDv4 string is valid.
      return true;
    }

    ///Returns the high word.
    inline uint64 High() const
    {
      uint64 h = HighLow[0];
      Endian::ConvertToBigEndian(h);
      return h;
    }

    ///Returns the low word.
    inline uint64 Low() const
    {
      uint64 l = HighLow[1];
      Endian::ConvertToBigEndian(l);
      return l;
    }

    ///Sets the high word.
    inline void High(uint64 h)
    {
      Endian::ConvertToBigEndian(h);
      HighLow[0] = h;
    }

    ///Sets the low word.
    inline void Low(uint64 l)
    {
      Endian::ConvertToBigEndian(l);
      HighLow[1] = l;
    }

    ///Clears the UUIDv4.
    inline void Clear()
    {
      HighLow[0] = HighLow[1] = 0;
    }

    ///Gets the string of the UUIDv4.
    operator String () const
    {
      String s = "00000000-0000-0000-0000-000000000000";
      ascii* d = const_cast<ascii*>(s.Merge());
      ToHex(Octet [0],  d[0],  d[1]);
      ToHex(Octet [1],  d[2],  d[3]);
      ToHex(Octet [2],  d[4],  d[5]);
      ToHex(Octet [3],  d[6],  d[7]);
      ToHex(Octet [4],  d[9], d[10]);
      ToHex(Octet [5], d[11], d[12]);
      ToHex(Octet [6], d[14], d[15]);
      ToHex(Octet [7], d[16], d[17]);
      ToHex(Octet [8], d[19], d[20]);
      ToHex(Octet [9], d[21], d[22]);
      ToHex(Octet[10], d[24], d[25]);
      ToHex(Octet[11], d[26], d[27]);
      ToHex(Octet[12], d[28], d[29]);
      ToHex(Octet[13], d[30], d[31]);
      ToHex(Octet[14], d[32], d[33]);
      ToHex(Octet[15], d[34], d[35]);
      return s;
    }

    ///Gets the hex string of the high part of the UUIDv4.
    String HighHex() const
    {
      String s = "0000000000000000";
      ascii* d = const_cast<ascii*>(s.Merge());
      ToHex(Octet [0],  d[0],  d[1]);
      ToHex(Octet [1],  d[2],  d[3]);
      ToHex(Octet [2],  d[4],  d[5]);
      ToHex(Octet [3],  d[6],  d[7]);
      ToHex(Octet [4],  d[8],  d[9]);
      ToHex(Octet [5], d[10], d[11]);
      ToHex(Octet [6], d[12], d[13]);
      ToHex(Octet [7], d[14], d[15]);
      return s;
    }

    ///Gets the hex string of the high part of the UUIDv4.
    String LowHex() const
    {
      String s = "0000000000000000";
      ascii* d = const_cast<ascii*>(s.Merge());
      ToHex(Octet [8],  d[0],  d[1]);
      ToHex(Octet [9],  d[2],  d[3]);
      ToHex(Octet[10],  d[4],  d[5]);
      ToHex(Octet[11],  d[6],  d[7]);
      ToHex(Octet[12],  d[8],  d[9]);
      ToHex(Octet[13], d[10], d[11]);
      ToHex(Octet[14], d[12], d[13]);
      ToHex(Octet[15], d[14], d[15]);
      return s;
    }

    ///Generates a random UUIDv4 (version 4).
    void Generate()
    {
      High(RandomSequence().NextUnsignedInt64());
      Low( RandomSequence().NextUnsignedInt64());
      Octet[6] = (Octet[6] & 0x4f) | 0x40;
      Octet[8] = (Octet[8] & 0xbf) | 0x80;
    }

    ///Generates an MD5 hashed UUIDv4 (version 3) from a namespace and value.
    void Generate(const UUIDv4& Namespace, const String& Value)
    {
      uint64 Words[2] = {Namespace.High(), Namespace.Low()};
      Endian::ConvertToBigEndian(Words[0]);
      Endian::ConvertToBigEndian(Words[1]);
      String s(reinterpret_cast<const byte*>(&Words[0]), 16);
      s << Value;
      String h = MD5::Hex(s);
      h.Insert("-", 20);
      h.Insert("-", 16);
      h.Insert("-", 12);
      h.Insert("-",  8);
      *this = UUIDv4(h);
      Octet[6] = (Octet[6] & 0x3f) | 0x30;
      Octet[8] = (Octet[8] & 0xbf) | 0x80;
    }

    ///Generates a random UUIDv4.
    UUIDv4()
    {
      Generate();
    }

    ///Generates an MD5 hashed UUIDv4 from a namespace and a value.
    UUIDv4(const UUIDv4& Namespace, const ascii* Value)
    {
      Generate(Namespace, Value);
    }

    ///Generates an MD5 hashed UUIDv4 from a namespace and a value.
    UUIDv4(const UUIDv4& Namespace, const String& Value)
    {
      Generate(Namespace, Value);
    }

    ///Initializes a UUIDv4 from a string.
    UUIDv4(const ascii* s)
    {
      Import(s);
    }

    ///Initializes UUIDv4 from another UUIDv4.
    UUIDv4(const UUIDv4& Other)
    {
      High(Other.High());
      Low(Other.Low());
    }

    ///Initializes UUIDv4 from two 64-bit integers.
    UUIDv4(uint64 h, uint64 l)
    {
      High(h);
      Low(l);
    }

    ///Returns an empty zeroed out UUIDv4.
    static UUIDv4 Empty()
    {
      return UUIDv4(uint64(0), uint64(0));
    }

    ///Copys a UUIDv4 from one object to another.
    UUIDv4& operator = (const UUIDv4& Other)
    {
      HighLow[0] = Other.HighLow[0];
      HighLow[1] = Other.HighLow[1];
      return *this;
    }

    ///Imports a UUIDv4 from a string.
    UUIDv4& operator = (const ascii* s)
    {
      Import(s);
      return *this;
    }

    ///Less-than comparison operator
    inline bool operator < (const UUIDv4& Other) const
    {
      if(High() < Other.High())
        return true;
      else if(High() == Other.High())
        return Low() < Other.Low();
      return false;
    }

    ///Equals comparison operator
    inline bool operator == (const UUIDv4& Other) const
    {
      return High() == Other.High() and Low() == Other.Low();
    }

    ///Not-equals comparison operator
    inline bool operator != (const UUIDv4& Other) const
    {
      return High() != Other.High() or Low() != Other.Low();
    }

    ///Greater-than comparison operator
    inline bool operator > (const UUIDv4& Other) const
    {
      if(High() > Other.High())
        return true;
      else if(High() == Other.High())
        return Low() > Other.Low();
      return false;
    }

    ///Less-than-or-equals comparison operator
    inline bool operator <= (const UUIDv4& Other) const
    {
      return (*this < Other) or (*this == Other);
    }

    ///Greater-than-or-equals comparison operator
    inline bool operator >= (const UUIDv4& Other) const
    {
      return (*this > Other) or (*this == Other);
    }
  };

  template <> inline Nothing<PRIM_NAMESPACE::UUIDv4>::operator UUIDv4() const
  {
    return UUIDv4::Empty();
  }

#ifdef PRIM_COMPILE_INLINE
  /*Lookup table to map ASCII byte codes to hexadecimal digits. Anything that is
  a hexadecimal digit is mapped to its corresponding value, and anything else is
  mapped to 16, indicating it is not a valid hexadecimal digit.*/
  const byte UUIDv4::HexMap[256] = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 16, 16, 16, 16, 16, 16, 10, 11, 12,
  13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 10, 11, 12, 13, 14, 15, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16};
#endif
}
#endif
