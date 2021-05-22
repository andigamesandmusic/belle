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

#ifndef PRIM_INCLUDE_ENCODING_BASE64_H
#define PRIM_INCLUDE_ENCODING_BASE64_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

//Note: this class is meant to be embedded in the Encoding class.

///Encoding and decoding for Base64 according to RFC 4648.
class Base64
{
  public:

  /**Decodes a Base64 stream according to the RFC 4648 specification. In
  particular it follows strict rules about parsing a stream and rejects any
  non-canonical Base64 string. Returns whether or not the decode was
  successful.*/
  static bool Decode(const String& s, Array<byte>& Out)
  {
    return Decode(reinterpret_cast<const byte*>(s.Merge()), s.n(), Out);
  }

  /**Decodes a Base64 stream according to the RFC 4648 specification. In
  particular it follows strict rules about parsing a stream and rejects any
  non-canonical Base64 string. Returns whether or not the decode was
  successful.*/
  static bool Decode(const String& s, String& Out)
  {
    Array<byte> OutArray;
    bool Result = Decode(reinterpret_cast<const byte*>(s.Merge()), s.n(),
      OutArray);
    Out.Clear();
    Out.Append(&OutArray.a(), OutArray.n());
    return Result;
  }

  /**Decodes a Base64 stream according to the RFC 4648 specification. In
  particular it follows strict rules about parsing a stream and rejects any
  non-canonical Base64 string. Returns whether or not the decode was
  successful.*/
  static bool Decode(const byte* Data, count Length, Array<byte>& Out)
  {
    static const byte Lookup[256] = //A-Z, a-z, 0-9, +, /
    { //MD5 checksum of data lines: 534957c8233c60ebaf526d10056bf6d4
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 62, 65, 65, 65, 63,
      52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 65, 65, 65, 64, 65, 65,
      65,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
      15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 65, 65, 65, 65, 65,
      65, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
      41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65
    };

    //Ratio for conversion (4 octets to 3 bytes of data).
    static const count Down = 4;
    static const count Up   = 3;

    //Clear the output array.
    Out.Clear();

    //If the input data is invalid return immediately.
    if(not Data or Length <= 0)
      return false;

    /*If the data length has not been padded to a multiple of 4 bytes, reject
    the string.*/
    if(Length % Down)
      return false;

    //Set up the data pointers.
    const byte*  InData       = Data;
    const byte*  InDataEnding = InData + Length - Down;
    const byte*  InDataEnd    = InData + Length;
    byte x[Down], y[Up];

    //Get the true length (assume data stream is valid, enforce later).
    count UnpaddedLength = Length / Down * Up;
    count Padding = 0;
    if(InData[Length - 2] == 0x3d) //0x3d is the = character.
    {
      Padding = 1;
      UnpaddedLength -= 2;
    }
    else if(InData[Length - 1] == 0x3d)
    {
      Padding = 2;
      UnpaddedLength -= 1;
    }

    if(not Padding)
      InDataEnding = InDataEnd;

    //Resize the array.
    Out.n(UnpaddedLength);
    byte* OutData = &Out.a();

    //Loop through the unpadded data.
    while(InData < InDataEnding)
    {
      //Get the values.
      x[0] = Lookup[InData[0]];
      x[1] = Lookup[InData[1]];
      x[2] = Lookup[InData[2]];
      x[3] = Lookup[InData[3]];

      //Values above 6-bits are invalid.
      if((x[0] | x[1] | x[2] | x[3]) & 0xc0)
        return Out.Clear();

      /*w w w w w w x x x x x x y y y y y y z z z z z z
        a a a a a a a a b b b b b b b b c c c c c c c c*/

      //Decode the stream.
      OutData[0] = byte(( x[0]        << 2) + (x[1] >> 4));
      OutData[1] = byte(((x[1] & 0xf) << 4) + (x[2] >> 2));
      OutData[2] = byte(((x[2] & 0x3) << 6) +  x[3]);

      //Increase the data pointers.
      InData  += Down;
      OutData += Up;
    }

    //If the data does not require padding, the output array is complete.
    if(InData >= InDataEnd)
      return true;

    //Compute remaining bytes of padded data.

    //Get the values.
    x[0] = Lookup[InData[0]];
    x[1] = Lookup[InData[1]];
    x[2] = Lookup[InData[2]];
    x[3] = Lookup[InData[3]];

    //Zero out the padding characters.
    if(Padding == 1) //1: xx== -> y
    {
      if(x[2] != 64 or x[3] != 64)
        return Out.Clear();
      x[2] = x[3] = 0;
    }
    else             //2: xxx= -> yy
    {
      if(x[3] != 64)
        return Out.Clear();
      x[3] = 0;
    }

    //Values above 6-bits are invalid.
    if((x[0] | x[1] | x[2] | x[3]) & 0xc0)
      return Out.Clear();

    //Decode the stream.
    y[0] = byte(( x[0]        << 2) + (x[1] >> 4));
    y[1] = byte(((x[1] & 0xf) << 4) + (x[2] >> 2));
    y[2] = byte(((x[2] & 0x3) << 6) +  x[3]);

    //Last byte must be zero.
    if(y[2] != 0)
      return Out.Clear();

    //There will be at least one extra byte.
    OutData[0] = y[0];

    //Second to last byte must be zero for 1 byte padding, otherwise use it.
    if(Padding == 1 and y[1] != 0) //1: xx== -> y
      return Out.Clear();
    else if(Padding == 2)         //2: xxx= -> yy
      OutData[1] = y[1];

    //Decode was successful.
    return true;
  }

  ///Encodes a string as a Base64 string.
  static void Encode(const String& Data, String& Out)
  {
    Encode(reinterpret_cast<const byte*>(Data.Merge()), Data.n(), Out);
  }

  ///Encodes a byte array as a Base64 string.
  static void Encode(const Array<byte>& Data, String& Out)
  {
    Encode(&Data.a(), Data.n(), Out);
  }

  //Encodes a byte array fragment as a Base64 string.
  static void Encode(const byte* Data, count Length, String& Out)
  {
    Out.Clear();
    if(not Data or Length <= 0)
      return;

    //Ratio for conversion (3 bytes of data to 4 octets).
    static const count Down = 3;
    static const count Up   = 4;

    count PaddedCharacters = Length % Down;
    const byte* InData = Data;
    const byte* InDataEnd = InData + Length;
    const byte* InDataEnding = InDataEnd - PaddedCharacters;
    byte x[Down], y[Up];

    count OutLength = (Length - PaddedCharacters) / Down * Up;
    if(PaddedCharacters)
      OutLength += Up;

    Array<byte> OutDataArray;
    OutDataArray.n(OutLength);
    OutDataArray.Zero();
    byte* OutData = &OutDataArray.a();

    static const byte Lookup[64] =
    { //MD5 checksum of data lines: eb3ab664a60feb7c52dadf461d78134a
       65,  66,  67,  68,  69,  70,  71,  72,
       73,  74,  75,  76,  77,  78,  79,  80,
       81,  82,  83,  84,  85,  86,  87,  88,
       89,  90,  97,  98,  99, 100, 101, 102,
      103, 104, 105, 106, 107, 108, 109, 110,
      111, 112, 113, 114, 115, 116, 117, 118,
      119, 120, 121, 122,  48,  49,  50,  51,
       52,  53,  54,  55,  56,  57,  43,  47
    };

    while(InData < InDataEnding)
    {
      //Load in the data.
      x[0] = InData[0];
      x[1] = InData[1];
      x[2] = InData[2];

      /*Decode the data:
        a a a a a a a a b b b b b b b b c c c c c c c c
        w w w w w w x x x x x x y y y y y y z z z z z z*/
      y[0] = byte(  x[0]        >> 2);
      y[1] = byte(((x[0] & 0x3) << 4) + (x[1] >> 4));
      y[2] = byte(((x[1] & 0xf) << 2) + (x[2] >> 6));
      y[3] = byte(  x[2] & 0x3f);

      OutData[0] = Lookup[y[0]];
      OutData[1] = Lookup[y[1]];
      OutData[2] = Lookup[y[2]];
      OutData[3] = Lookup[y[3]];

      //Increase the data pointers.
      InData  += Down;
      OutData += Up;
    }

    if(InData < InDataEnd)
    {
      x[0] = InData[0];
      x[1] = 0;
      if(PaddedCharacters == 2)
        x[1] = InData[1];

      /*Decode the data:
        a a a a a a a a b b b b b b b b c c c c c c c c
        w w w w w w x x x x x x y y y y y y z z z z z z*/
      y[0] = byte(  x[0]        >> 2);
      y[1] = byte(((x[0] & 0x3) << 4) + (x[1] >> 4));
      y[2] = byte(((x[1] & 0xf) << 2));

      if(PaddedCharacters == 1) //1:  y -> xx==
      {
        OutData[0] = Lookup[y[0]];
        OutData[1] = Lookup[y[1]];
        OutData[2] = 0x3d;
        OutData[3] = 0x3d;
      }
      else                      //2: yy -> xxx=
      {
        OutData[0] = Lookup[y[0]];
        OutData[1] = Lookup[y[1]];
        OutData[2] = Lookup[y[2]];
        OutData[3] = 0x3d;
      }
    }
    Out.Append(&OutDataArray.a(), OutDataArray.n());
  }
};
#endif
