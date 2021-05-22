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
#define PRIM_WITH_AES
#define PRIM_WITH_FFT
#define PRIM_WITH_MIDI
#include "prim.h"
using namespace PRIM_NAMESPACE;

static count ChecksRun = 0;
static count ChecksFailed = 0;

template<class U, class V> void EXPECT_EQ(const U& a, const V& b)
{
  ChecksRun++;
  if(not (a == b))
  {
    C::Out() >> "  Failed: " << a << " == " << b;
    ChecksFailed++;
  }
}

template<class U, class V> void EXPECT_LT(const U& a, const V& b)
{
  ChecksRun++;
  if(not (a < b))
  {
    C::Out() >> "  Failed: " << a << " < " << b;
    ChecksFailed++;
  }
}

template<class U, class V> void EXPECT_GT(const U& a, const V& b)
{
  ChecksRun++;
  if(not (a > b))
  {
    C::Out() >> "  Failed: " << a << " > " << b;
    ChecksFailed++;
  }
}

template<class T> void ASSERT_EQ(const T& a, const T& b)
{
  ChecksRun++;
  if(not (a == b))
  {
    C::Out() >> "  Failed: " << a << " == " << b;
    ChecksFailed++;
    C::Out() >> "Aborting.\n";
    exit(1);
  }
}

template<class T> void EXPECT_NEAR(const T& a, const T& b, const T& Delta)
{
  ChecksRun++;
  if(not (Abs(a - b) <= Delta))
  {
    C::Out() >> "  Failed: abs(" << a << " - " << b << ") < " << Delta;
    ChecksFailed++;
  }
}

////////////////////////////////////////////////////////////////////////////////

void TEST_PrimUnitTests_AESRoundtrips();
void TEST_PrimUnitTests_AESRoundtrips()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "AESRoundtrips";
  Random R(12345);
  bool Failed = false;
  for(count i = 0; i <= 100; i++)
  {
    SecureByteArray Key(32);
    SecureByteArray IV(16);
    for(count j = 0; j < 32; j++)
      Key[j] = byte(R.NextInteger());
    for(count j = 0; j < 16; j++)
      IV[j] = byte(R.NextInteger());
    SecureByteArray Plaintext(i);
    for(count j = 0; j < Plaintext.n(); j++)
      Plaintext[j] = byte(R.NextInteger());
    Array<byte> Encrypted;
    SecureByteArray Decrypted;
    AES::EncryptCBC256(Plaintext, Encrypted, Key, IV);
    AES::DecryptCBC256(Encrypted, Decrypted, Key, IV);
    if(Plaintext == Encrypted and Plaintext.n())
      C::Error() >> "Error: Plaintext == Encrypted (!!!)", Failed = true;
    else if(Encrypted == Decrypted and Encrypted.n())
      C::Error() >> "Error: Encrypted == Decrypted (!!!)", Failed = true;
    else if(Decrypted != Plaintext)
      C::Error() >> "Error: Plaintext != Decrypted", Failed = true;
    else if(Encrypted.n() % 16 != 0)
      C::Error() >> "Error: Encrypted.n() not multiple of 16", Failed = true;
    else if(Decrypted.n() != i)
      C::Error() >> "Error: Decrypted.n() != i", Failed = true;
  }
  EXPECT_EQ(false, Failed);
}

void TEST_PrimUnitTests_AESReference();
void TEST_PrimUnitTests_AESReference()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "AESReference";
  String LoremIpsumPlaintext =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
    "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum.";

  String LoremIpsumKey =
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";

  byte LoremIpsumEncrypted[] =
  {
    77, 171, 220, 236, 146, 238, 176, 218, 202, 95, 136, 85, 89, 180,
    90, 200, 73, 194, 112, 122, 27, 7, 132, 183, 197, 149, 103, 61, 197,
    213, 68, 137, 29, 145, 203, 124, 169, 34, 31, 153, 190, 9, 19, 169,
    157, 249, 200, 209, 79, 203, 124, 73, 24, 216, 123, 26, 109, 66, 27,
    109, 213, 24, 180, 6, 80, 241, 115, 83, 178, 59, 0, 161, 138, 208, 91,
    14, 110, 1, 126, 128, 253, 246, 246, 191, 89, 118, 138, 116, 224, 105,
    77, 202, 247, 163, 177, 96, 247, 39, 109, 52, 97, 130, 205, 211, 44,
    19, 68, 167, 136, 33, 173, 5, 221, 118, 16, 86, 43, 235, 206, 195,
    200, 123, 243, 242, 234, 129, 18, 209, 103, 2, 99, 2, 13, 235, 178,
    81, 59, 242, 195, 180, 189, 188, 83, 42, 122, 71, 55, 79, 174, 219,
    195, 255, 198, 22, 75, 81, 50, 243, 198, 179, 158, 140, 199, 204, 254,
    136, 236, 243, 89, 251, 73, 58, 15, 110, 43, 186, 2, 197, 247, 150,
    172, 124, 111, 186, 120, 128, 254, 2, 76, 95, 43, 176, 138, 29, 13,
    166, 139, 127, 241, 113, 54, 6, 109, 123, 150, 224, 209, 143, 136, 28,
    111, 101, 57, 210, 61, 73, 87, 73, 156, 123, 242, 113, 26, 15, 131,
    206, 14, 92, 131, 4, 235, 251, 93, 175, 126, 102, 244, 104, 122, 69,
    51, 47, 109, 0, 87, 193, 17, 95, 231, 101, 132, 97, 92, 106, 254, 222,
    172, 86, 202, 177, 145, 58, 67, 173, 79, 45, 128, 235, 121, 3, 49, 78,
    33, 179, 213, 152, 187, 127, 38, 41, 45, 159, 212, 47, 247, 23, 237,
    57, 62, 138, 217, 24, 100, 15, 250, 127, 173, 70, 136, 241, 219, 143,
    73, 35, 224, 21, 220, 155, 111, 235, 105, 92, 11, 254, 241, 139, 224,
    240, 227, 114, 47, 40, 8, 5, 108, 108, 246, 120, 162, 170, 81, 241, 4,
    240, 57, 238, 37, 194, 46, 65, 149, 25, 171, 98, 15, 5, 91, 41, 24,
    240, 142, 220, 124, 231, 66, 116, 179, 49, 240, 234, 122, 85, 193, 40,
    105, 161, 176, 70, 162, 116, 199, 232, 38, 151, 51, 218, 168, 70, 168,
    200, 197, 208, 179, 192, 240, 236, 1, 1, 107, 137, 57, 1, 128, 70,
    108, 79, 224, 53, 1, 40, 104, 20, 49, 63, 61, 128, 52, 160, 46, 81,
    118, 177, 176, 91, 20, 8, 164, 47, 158, 99, 215, 230, 220, 1, 136,
    194, 235, 14, 58, 35, 83, 97, 51, 246, 213, 25, 153, 172, 225, 42, 31,
    140, 140, 200, 77, 87, 10, 103
  };

  count LoremIpsumEncryptedLength = count(sizeof(LoremIpsumEncrypted));

  /*In a real application you would take measures to prevent this
  information from residing in memory or in a non-secure byte array.*/
  Array<byte> UnsecuredKey = String::Hex(LoremIpsumKey);

  //Import the plaintext a secure byte array.
  SecureByteArray Plaintext(LoremIpsumPlaintext.n());
  for(count i = 0; i < Plaintext.n(); i++)
    Plaintext[i] = LoremIpsumPlaintext[i];

  //Import the key to a secure byte array.
  SecureByteArray Key(UnsecuredKey.n());
  for(count i = 0; i < LoremIpsumKey.n(); i++)
    Key[i] = UnsecuredKey[i];

  //Encrypt plaintext.
  Array<byte> Encrypted;
  AES::EncryptCBC256(Plaintext, Encrypted, Key, Key);

  //Decrypt encrypted.
  SecureByteArray Decrypted;
  AES::DecryptCBC256(Encrypted, Decrypted, Key, Key);

  //Import reference encryption.
  Array<byte> Reference(LoremIpsumEncryptedLength);
  for(count i = 0; i < Reference.n(); i++)
    Reference[i] = LoremIpsumEncrypted[i];

  EXPECT_EQ(true, Encrypted == Reference);
  EXPECT_EQ(true, Decrypted == Plaintext);
}

////////////////////////////////////////////////////////////////////////////////

void TEST_PrimUnitTests_Base64Encode();
void TEST_PrimUnitTests_Base64Encode()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "Base64Encode";
  for(count j = 1; j < 100; j++)
  {
    uint32 j_32 = uint32(j);
    Random r(j_32);
    Array<byte> a, b;
    String s;
    for(count i = 0; i < 100; i++)
    {
      a.Add() = byte(r.NextInteger());
      Encoding::Base64::Encode(a, s);
      Encoding::Base64::Decode(s, b);
      ASSERT_EQ(a, b);
    }
  }
}

void TEST_PrimUnitTests_Base64Decode();
void TEST_PrimUnitTests_Base64Decode()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "Base64Decode";
  byte Data[4] = {0, 0, 0, 0};
  Array<byte> a;
  String b;
  for(count h = 0; h < 8; h++)
  {
    //c << " ... " << (number)h / (255.) * 100.0 << "%";
    Data[0] = byte(h);
    for(count i = 0; i < 256; i++)
    {
      Data[1] = byte(i);
      for(count j = 65; j <= 65; j++) //Fixed, unfix for more thorough test
      {
        Data[2] = byte(j);
        for(count k = 0; k < 256; k++)
        {
          Data[3] = byte(k);
          for(count l = 0; l <= 4; l++)
          {
            Encoding::Base64::Decode(&Data[0], l, a);
            Encoding::Base64::Encode(a, b);
            bool IsIdentical = true;
            if(l != b.n())
              IsIdentical = false;
            else
            {
              for(count x = 0; x < l; x++)
              {
                if(Data[x] != byte(b[x]))
                {
                  IsIdentical = false;
                  break;
                }
              }
            }
            ASSERT_EQ(false, !IsIdentical && a.n());
            if(!IsIdentical && a.n())
              C::Error() >> "Error: " << i << " " << j << " " << k;
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void TEST_PrimUnitTests_EndianConversion();
void TEST_PrimUnitTests_EndianConversion()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "EndianConversion";

  {
    int16 x = 123 + (45 << 8);
    int16 y = (123 << 8) + 45;
    Endian::ReverseOrder(x);
    EXPECT_EQ(x, y);
  }

  {
    byte d[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for(count i = 0; i < 16; i++)
    {
      Memory::MemSet(d, 0, 16);
      for(count j = 0; j < i; j++)
        d[j] = byte(j);
      Endian::ReverseByteOrder(d, i);
      for(count j = 0; j < i; j++)
        EXPECT_EQ(count(d[j]), i - 1 - j);
      Endian::ReverseByteOrder(d, i);
      for(count j = 0; j < i; j++)
        EXPECT_EQ(count(d[j]), j);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void TEST_PrimUnitTests_FFTStressTest();
void TEST_PrimUnitTests_FFTStressTest()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "FFTStressTest";
  for(count FFTSize = 8; FFTSize <= 65536; FFTSize *= 2)
  {
    Array<Complex<float64> > Data, Original;
    Array<float64> CosineTable;

    //Generate white noise and cosine table.
    GenerateWhiteNoise(Data, FFTSize);
    GenerateCosineLookup<float80>(CosineTable, FFTSize);
    Original = Data;

    //Calculating forwards and backwards transforms and renormalize
    FFT<Forwards>(Data, CosineTable);
    FFT<Backwards>(Data, CosineTable);
    Normalize(Data);

    //Calculate the bits precision
    number BitsPrecision = -CalculateRMSError(Data, Original);
    EXPECT_NEAR(50.0, BitsPrecision, 5.0);
  }
}

////////////////////////////////////////////////////////////////////////////////

void TEST_PrimUnitTests_JSONValid();
void TEST_PrimUnitTests_JSONValid()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "JSONValid";
  List<String> Valid;

  //Valid

  Valid.Add() = "[]";
  Valid.Add() = " []";
  Valid.Add() = "[] ";
  Valid.Add() = "[ ]";
  Valid.Add() = " [ ]";
  Valid.Add() = "[ ] ";
  Valid.Add() = " [ ] ";
  Valid.Add() = " [false, null, true, [], [null] ] ";
  Valid.Add() = "[[[[[[[[[[]]]]]]]]]]";
  Valid.Add() = "[[[[], true, []], true, [[], true, []]], true, [[[], true, "
    "[]], true, [[], true, []]]]";
  Valid.Add() = "[-1.234e+3]";
  Valid.Add() = "[-1.234E+3]";
  Valid.Add() = "[-1.234e3]";
  Valid.Add() = "[-1.234E3]";
  Valid.Add() = "[-1.234e-3]";
  Valid.Add() = "[-1.234E-3]";
  Valid.Add() = "[1.234e+3]";
  Valid.Add() = "[1.234E+3]";
  Valid.Add() = "[1.234e3]";
  Valid.Add() = "[1.234E3]";
  Valid.Add() = "[1.234e-3]";
  Valid.Add() = "[1.234E-3]";
  Valid.Add() = "[-1234e+3]";
  Valid.Add() = "[-1234E+3]";
  Valid.Add() = "[-1234e3]";
  Valid.Add() = "[-1234E3]";
  Valid.Add() = "[-1234e-3]";
  Valid.Add() = "[-1234E-3]";
  Valid.Add() = "[1234e+3]";
  Valid.Add() = "[1234E+3]";
  Valid.Add() = "[1234e3]";
  Valid.Add() = "[1234E3]";
  Valid.Add() = "[1234e-3]";
  Valid.Add() = "[1234E-3]";
  Valid.Add() = "[1234]";
  Valid.Add() = "[1]";
  Valid.Add() = "[\"Hello\", \"O\\nk\"]";
  Valid.Add() = "[\"Test\\ud834\\udd1e\"]"; //G-clef: \ud834\udd1e
  Valid.Add() = "{}";
  Valid.Add() = "{ }";
  Valid.Add() = "{\"hi\":false }";
  Valid.Add() = "{\"hi\":false, \"there\":true }";
  Valid.Add() = "{\"hi\":\"text\" }";
  Valid.Add() = "{\"hi\":1234}";
  Valid.Add() = "{\"hi\":1234.56e-10}";
  Valid.Add() = "{\"hi\":null}";
  Valid.Add() = "{\"hi\":true}";
  Valid.Add() = "{\"hi\":[]}";
  Valid.Add() = "{\"hi\":[10, 20]}";
  Valid.Add() = "{\"hi\":{}}";
  Valid.Add() = "{\"hi\":{\"there\":[]}}";
  Valid.Add() = "{\"hi\":[1]}";

  for(count i = 0; i < Valid.n(); i++)
  {
    Value V;
    String s = Valid[i];
    EXPECT_EQ(true, JSON::Import(s, V));

    String FirstExport = V.ExportJSON();
    //JSON::Export(V, FirstExport, true);
    JSON::Import(FirstExport, V);

    //Test round-trip internal copy.
    Value B = V;
    V.Clear();
    V = B;

    String SecondExport;
    JSON::Export(V, SecondExport, true);
    EXPECT_EQ(FirstExport, SecondExport);
  }
}

void TEST_PrimUnitTests_JSONInvalid();
void TEST_PrimUnitTests_JSONInvalid()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "JSONInvalid";
  List<String> Invalid;

  //Invalid
  Invalid.Add() = "";
  Invalid.Add() = " ";
  Invalid.Add() = "[,]";
  Invalid.Add() = "[.]";
  Invalid.Add() = "[.1234]";
  Invalid.Add() = "[-.1234]";
  Invalid.Add() = "[-.]";
  Invalid.Add() = "[0.]";
  Invalid.Add() = "[0.e]";
  Invalid.Add() = "[001.234e-31]";
  Invalid.Add() = "[-0.]";
  Invalid.Add() = "[-0.e]";
  Invalid.Add() = "[-001.234e-31]";
  Invalid.Add() = "[+100]";
  Invalid.Add() = " [";
  Invalid.Add() = "a";
  Invalid.Add() = "{\"a\":1, \"a\":2}";
  Invalid.Add() = " [false, null, true, [[], [null] ] ";
  Invalid.Add() = "[\"Test\\ud834\"]";
  Invalid.Add() = "[\"Test\\udd1e\"]";
  Invalid.Add() = "[\"Test\\ud834\\n\"]";
  Invalid.Add() = "[\"Test\\ud83\"]";
  Invalid.Add() = "[\"Test\\ud";
  Invalid.Add() = "[\"Test\\ud834\\u1234\"]";

  for(count i = 0; i < Invalid.n(); i++)
  {
    Value V;
    String s = Invalid[i];
    EXPECT_EQ(false, JSON::Import(s, V));
  }
}

////////////////////////////////////////////////////////////////////////////////

void TEST_PrimUnitTests_MD5Calculate();
void TEST_PrimUnitTests_MD5Calculate()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "MD5Calculate";
  EXPECT_EQ(
    String("9e107d9d372bb6826bd81d3542a419d6"),
    MD5::Hex("The quick brown fox jumps over the lazy dog"));

  EXPECT_EQ(
    String("d41d8cd98f00b204e9800998ecf8427e"),
    MD5::Hex(""));

  EXPECT_EQ(
    String("0cc175b9c0f1b6a831c399e269772661"),
    MD5::Hex("a"));

  EXPECT_EQ(
    String("900150983cd24fb0d6963f7d28e17f72"),
    MD5::Hex("abc"));

  EXPECT_EQ(
    String("f96b697d7cb7938d525a2f31aaf161d0"),
    MD5::Hex("message digest"));

  EXPECT_EQ(
    String("c3fcd3d76192e4007dfb496cca67e13b"),
    MD5::Hex("abcdefghijklmnopqrstuvwxyz"));

  EXPECT_EQ(
    String("d174ab98d277d9f5a5611c2c9f419d9f"),
    MD5::Hex("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));

  EXPECT_EQ(
    String("57edf4a22be3c955ac49da2e2107b67a"),
    MD5::Hex("123456789012345678901234567890123456789012345"
      "67890123456789012345678901234567890"));
}

////////////////////////////////////////////////////////////////////////////////

void TEST_PrimUnitTests_MIDI();
void TEST_PrimUnitTests_MIDI()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "MIDI";
  MIDI::File f;

  f.SetDivisionsPerQuarterNote(120);

  MIDI::Track& Violin = f.Tracks.Add();
  MIDI::Track& Piano1 = f.Tracks.Add();
  MIDI::Track& Piano2 = f.Tracks.Add();

  f.CopyrightNotice("(c) Copyright Holder");
  f.TimeSignature(0, 3, 4);
  f.InstrumentName("Violin", 1);
  f.InstrumentName("Piano", 2);

  Violin.TrackName("Violin");
  Violin.ProgramChange(0, 40, 1);
  Violin.KeySignature(0, 1, true);

  Piano1.TrackName("Piano");
  Piano1.ProgramChange(0, 0, 2);
  Piano1.KeySignature(0, 1, true);

  Piano2.KeySignature(0, 1, true);

  for(count n = 24, t = 0, v = 80; n <= 100; n++, t++)
  {
    Violin.Note(Ratio(t, 8), Ratio(1, 8), n + 7, v, 1);
    Piano1.Note(Ratio(t, 8), Ratio(1, 8), n + 0, v, 2);
    Piano2.Note(Ratio(t, 8), Ratio(1, 8), n - 7, v, 2);
  }

  Array<byte> b;
  f.Write(b);
  MIDI::File f2;
  Array<byte> b2;
  f2.Read(b);
  f2.Write(b2);

  EXPECT_EQ(true, (String(f) == String(f2)));
  EXPECT_EQ(true, (b == b2));
}

////////////////////////////////////////////////////////////////////////////////

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#endif

class NothingFoo
{
  public:
  int x;
  NothingFoo() : x(0) {}
  NothingFoo(int y) : x(y) {}
  bool operator == (const NothingFoo& Other) const {return x == Other.x;}
  bool operator != (const NothingFoo& Other) const {return x != Other.x;}
};

typedef void (*NothingFunction)(void);

template <class T>
void NothingCompare()
{
  T x = Nothing<T>();
  Nothing<T> y;
  T z = T(123);
  EXPECT_EQ(true, y == x);
  EXPECT_EQ(true, z != x);
  EXPECT_EQ(true, x == y);
  EXPECT_EQ(true, x != z);
  EXPECT_EQ(true, Nothing<T>() == Nothing<T>());
  EXPECT_EQ(true, !(Nothing<T>() != Nothing<T>()));
}

template <class T>
void NothingCompareToDefaultObject()
{
  T x = Nothing<T>();
  Nothing<T> y;
  T z = T();
  EXPECT_EQ(true, y == x);
  EXPECT_EQ(true, z != x);
  EXPECT_EQ(true, x == y);
  EXPECT_EQ(true, x != z);
  EXPECT_EQ(true, Nothing<T>() == Nothing<T>());
  EXPECT_EQ(true, !(Nothing<T>() != Nothing<T>()));
}

void TEST_PrimUnitTests_NothingComparison();
void TEST_PrimUnitTests_NothingComparison()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "NothingComparison";
  NothingCompare<bool>();
  NothingCompare<uint8>();
  NothingCompare<int8>();
  NothingCompare<uint16>();
  NothingCompare<int16>();
  NothingCompare<uint32>();
  NothingCompare<int32>();
  NothingCompare<uint64>();
  NothingCompare<int64>();
  NothingCompare<float32>();
  NothingCompare<float64>();
  NothingCompare<float80>();
  NothingCompare<String>();
  NothingCompareToDefaultObject<UUIDv4>();
  NothingCompare<NothingFoo>();
  NothingCompare<int*>();
  NothingCompare<NothingFoo*>();
  NothingCompare<NothingFunction>();
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

////////////////////////////////////////////////////////////////////////////////

void TEST_PrimUnitTests_ListQuicksort();
void TEST_PrimUnitTests_ListQuicksort()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "ListQuicksort";
  Random r(123);
  for(count n = 0; n < 10; n++)
  {
    for(count j = 0; j < 10; j++)
    {
      Sortable::List<integer> L;
      for(count i = 0; i < n; i++)
        L.Add() = r.NextInteger();
      L.Quicksort();
      bool Sorted = L.IsSorted();
      EXPECT_EQ(true, Sorted);
    }
  }
}

void TEST_PrimUnitTests_ListBubblesort();
void TEST_PrimUnitTests_ListBubblesort()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "ListBubblesort";
  Random r(123);
  for(count n = 0; n < 10; n++)
  {
    for(count j = 0; j < 10; j++)
    {
      Sortable::List<integer> L;
      for(count i = 0; i < n; i++)
        L.Add() = r.NextInteger();
      L.BubbleSort();
      bool Sorted = L.IsSorted();
      EXPECT_EQ(true, Sorted);
    }
  }
}

void TEST_PrimUnitTests_ArrayQuicksort();
void TEST_PrimUnitTests_ArrayQuicksort()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "ArrayQuicksort";
  Random r(123);
  for(count n = 0; n < 10; n++)
  {
    for(count j = 0; j < 10; j++)
    {
      Sortable::Array<integer> L;
      for(count i = 0; i < n; i++)
        L.Add() = r.NextInteger();
      L.Quicksort();
      bool Sorted = L.IsSorted();
      EXPECT_EQ(true, Sorted);
    }
  }
}

class SwappableInteger
{
  integer Value;

  public:

  SwappableInteger& operator = (integer Other)
  {
    Value = Other;
    return *this;
  }

  bool operator < (const SwappableInteger& Other) const
    {return Value < Other.Value;}
  bool operator > (const SwappableInteger& Other) const
    {return Value > Other.Value;}
  bool operator == (const SwappableInteger& Other) const
    {return Value == Other.Value;}

  void Swap(SwappableInteger& Other)
  {
    PRIM_NAMESPACE::Swap(*this, Other);
  }
};

void TEST_PrimUnitTests_SwappableArrayQuicksort();
void TEST_PrimUnitTests_SwappableArrayQuicksort()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "SwappableArrayQuicksort";
  Random r(123);
  for(count n = 0; n < 100; n++)
  {
    for(count j = 0; j < 100; j++)
    {
      Sortable::SwappableArray<SwappableInteger> L;
      for(count i = 0; i < n; i++)
        L.Add() = r.NextInteger();
      L.Quicksort();
      bool Sorted = L.IsSorted();
      EXPECT_EQ(true, Sorted);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void TEST_PrimUnitTests_TreeSmokeTest();
void TEST_PrimUnitTests_TreeSmokeTest()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "TreeSmokeTest";
  Tree<int> T;

  /*Use the const Tree<int> whenever possible. Note that the lazy Set and
  operator [], will create the key if it does not exist already, which can cause
  unintended behavior from a testing standpoint.*/
  const Tree<int>& ConstT = T;

  EXPECT_EQ(ConstT[5], Nothing<int>());
  EXPECT_EQ(ConstT.Contains(5), false);
  EXPECT_EQ(ConstT.First(), Nothing<int>());
  EXPECT_EQ(ConstT.Last(), Nothing<int>());
  EXPECT_EQ(ConstT.n(), 0);

  T[5] = 10;
  EXPECT_EQ(ConstT[5], 10);
  EXPECT_EQ(ConstT[10], Nothing<int>());
  EXPECT_EQ(ConstT.Contains(5), true);
  EXPECT_EQ(ConstT.Contains(10), false);
  EXPECT_EQ(ConstT.First(), 5);
  EXPECT_EQ(ConstT.Last(), 5);
  EXPECT_EQ(ConstT.n(), 1);

  T[10] = 20;
  EXPECT_EQ(ConstT[5], 10);
  EXPECT_EQ(ConstT[7], Nothing<int>());
  EXPECT_EQ(ConstT[10], 20);
  EXPECT_EQ(ConstT.Contains(5), true);
  EXPECT_EQ(ConstT.Contains(7), false);
  EXPECT_EQ(ConstT.Contains(10), true);
  EXPECT_EQ(ConstT.First(), 5);
  EXPECT_EQ(ConstT.Last(), 10);
  EXPECT_EQ(ConstT.n(), 2);

  T[7]; //Lazy set will initialize key.
  EXPECT_EQ(ConstT[5], 10);
  EXPECT_EQ(ConstT[7], Nothing<int>());
  EXPECT_EQ(ConstT[10], 20);
  EXPECT_EQ(ConstT.Contains(5), true);
  EXPECT_EQ(ConstT.Contains(7), true);
  EXPECT_EQ(ConstT.Contains(10), true);
  EXPECT_EQ(ConstT.First(), 5);
  EXPECT_EQ(ConstT.Last(), 10);
  EXPECT_EQ(ConstT.n(), 3);

  EXPECT_EQ(T.Prune(), 1);
  EXPECT_EQ(ConstT[5], 10);
  EXPECT_EQ(ConstT[7], Nothing<int>());
  EXPECT_EQ(ConstT[10], 20);
  EXPECT_EQ(ConstT.Contains(5), true);
  EXPECT_EQ(ConstT.Contains(7), false);
  EXPECT_EQ(ConstT.Contains(10), true);
  EXPECT_EQ(ConstT.First(), 5);
  EXPECT_EQ(ConstT.Last(), 10);
  EXPECT_EQ(ConstT.n(), 2);
}

void TEST_PrimUnitTests_TreeLargeInsertion();
void TEST_PrimUnitTests_TreeLargeInsertion()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "TreeLargeInsertion";

  Tree<int> T;
  const Tree<int>& ConstT = T;
  const int N = 10000;
  const int KeyMin = -1000000000;
  const int KeyMax =  1000000000;

  {
    Random r(123);
    for(int i = 0; i < N; i++)
      T[r.Between(KeyMin, KeyMax)] = i;
  }

  {
    Tree<int>::Iterator It;
    int Previous_k = KeyMin - 1;
    for(It.Begin(ConstT); It.Iterating(); It.Next())
    {
      int k = It.Key();
      EXPECT_LT(Previous_k, k);
      EXPECT_EQ(It.Value() >= 0 && It.Value() < N, true);
      Previous_k = k;
    }
  }

  EXPECT_EQ(T.Prune(), 0);
}

void TEST_PrimUnitTests_TreeLargeRemoval();
void TEST_PrimUnitTests_TreeLargeRemoval()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "TreeLargeRemoval";

  Tree<int> T;
  const int N = 10000;
  const int KeyMin = -1000000000;
  const int KeyMax =  1000000000;

  {
    Random r(123);
    for(int i = 0; i < N; i++)
      T[r.Between(KeyMin, KeyMax)] = i;
  }

  EXPECT_GT(T.n(), 0);

  {
    Random r(123);
    for(int i = 0; i < N; i++)
      T.Remove(r.Between(KeyMin, KeyMax));
  }

  EXPECT_EQ(T.n(), 0);
}

void TEST_PrimUnitTests_TreeLargePruning();
void TEST_PrimUnitTests_TreeLargePruning()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "TreeLargePruning";

  Tree<int> T;
  const Tree<int>& ConstT = T;
  const int N = 10000;

  {
    for(int i = 0; i < N; i++)
    {
      if(i % 2)
        T[i] = i;
      else
        T[i];
    }
  }

  {
    for(int i = 0; i < N; i++)
    {
      if(i % 2)
        EXPECT_EQ(ConstT[i], i);
      else
        EXPECT_EQ(ConstT[i], Nothing<int>());
    }
  }

  EXPECT_EQ(T.Prune(), N / 2);
  EXPECT_EQ(T.n(), N / 2);
}

void TEST_PrimUnitTests_TreeTrimming();
void TEST_PrimUnitTests_TreeTrimming()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "TreeTrimming";

  Tree<int> T;
  const Tree<int>& ConstT = T;
  const int N = 1000;

  {
    for(int i = 0; i < N; i++)
      T[i] = i;
  }

  {
    int x = 0;
    int y = N - 1;
    int Previous_n = int(T.n());
    while(T.n())
    {
      T.RemoveFirst();
      T.RemoveLast();
      EXPECT_EQ(Previous_n - 2, T.n());
      x++;
      y--;
      Previous_n = int(T.n());
      for(int i = x; i <= y; i++)
        EXPECT_EQ(ConstT[i], i);
    }
  }
}

void TEST_PrimUnitTests_TreeIterating();
void TEST_PrimUnitTests_TreeIterating()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "TreeIterating";

  Tree<int> T;
  const Tree<int>& ConstT = T;
  const int N = 10000;

  {
    for(int i = 0; i < N; i++)
      T[i] = i;
  }

  {
    Tree<int>::Iterator It;
    int Expected_k = 0;
    for(It.Begin(ConstT); It.Iterating(); It.Next())
    {
      int k = It.Key();
      EXPECT_EQ(Expected_k, k);
      EXPECT_EQ(ConstT[k], It.Value());
      EXPECT_EQ(ConstT[k], k);
      Expected_k++;
    }
  }
}

void TEST_PrimUnitTests_TreeLargeDeepCopy();
void TEST_PrimUnitTests_TreeLargeDeepCopy()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "TreeLargeDeepCopy";

  Tree<int> T;
  const int N = 10000;
  const int KeyMin = -1000000000;
  const int KeyMax =  1000000000;

  {
    Random r(123);
    for(int i = 0; i < N; i++)
      T[r.Between(KeyMin, KeyMax)] = i;
  }
  {
    Tree<int> T2;
    T2 = T;
    EXPECT_EQ(T.n(), T2.n());
    EXPECT_EQ(true, T == T2);

    {
      Random r(123);
      for(int i = 0; i < N; i++)
        T2.Remove(r.Between(KeyMin, KeyMax));
    }

    EXPECT_EQ(T2.n(), 0);
  }
  {
    Tree<int> T3(T);
    EXPECT_EQ(T.n(), T3.n());
    EXPECT_EQ(false, T != T3);

    {
      Random r(123);
      for(int i = 0; i < N; i++)
        T3.Remove(r.Between(KeyMin, KeyMax));
    }

    EXPECT_EQ(T3.n(), 0);
  }
}

////////////////////////////////////////////////////////////////////////////////
static unsigned char UTF16_TestBE[] = {
  0xd8, 0x41, 0xdf, 0x0e, 0x00, 0x20, 0xd8, 0x41, 0xdf, 0x31, 0x00, 0x20,
  0xd8, 0x41, 0xdf, 0x79, 0x00, 0x20, 0xd8, 0x43, 0xdc, 0x53, 0x00, 0x20,
  0xd8, 0x43, 0xdc, 0x78, 0x00, 0x20, 0xd8, 0x43, 0xdc, 0x96, 0x00, 0x20,
  0xd8, 0x43, 0xdc, 0xcf, 0x00, 0x20, 0xd8, 0x43, 0xdc, 0xd5, 0x00, 0x20,
  0xd8, 0x43, 0xdd, 0x15, 0x00, 0x20, 0xd8, 0x43, 0xdd, 0x7c, 0x00, 0x20,
  0xd8, 0x43, 0xdd, 0x7f, 0x00, 0x20, 0xd8, 0x43, 0xde, 0x0e, 0x00, 0x20,
  0xd8, 0x43, 0xde, 0x0f, 0x00, 0x20, 0xd8, 0x43, 0xde, 0x77, 0x00, 0x20,
  0xd8, 0x43, 0xde, 0x9d, 0x00, 0x20, 0xd8, 0x43, 0xde, 0xa2, 0x00, 0x20,
  0xd8, 0x43, 0xde, 0xd7, 0x00, 0x20, 0xd8, 0x43, 0xde, 0xf9, 0x00, 0x20,
  0xd8, 0x43, 0xde, 0xfa, 0x00, 0x20, 0xd8, 0x43, 0xdf, 0x2d, 0x00, 0x20,
  0xd8, 0x43, 0xdf, 0x2e, 0x00, 0x20, 0xd8, 0x43, 0xdf, 0x4c, 0x00, 0x20,
  0xd8, 0x43, 0xdf, 0xb4, 0x00, 0x20, 0xd8, 0x43, 0xdf, 0xbc, 0x00, 0x20,
  0xd8, 0x43, 0xdf, 0xea, 0x00, 0x20, 0xd8, 0x44, 0xdc, 0x5c, 0x00, 0x20,
  0xd8, 0x44, 0xdc, 0x6f, 0x00, 0x20, 0xd8, 0x44, 0xdc, 0x75, 0x00, 0x20,
  0xd8, 0x44, 0xdc, 0x76, 0x00, 0x20, 0xd8, 0x44, 0xdc, 0x7b, 0x00, 0x20,
  0xd8, 0x44, 0xdc, 0xc1, 0x00, 0x20, 0xd8, 0x44, 0xdc, 0xc9, 0x00, 0x20,
  0xd8, 0x44, 0xdd, 0xd9, 0x00, 0x20, 0xd8, 0x48, 0xdc, 0xc7, 0x00, 0x20,
  0xd8, 0x49, 0xdf, 0xb5, 0x00, 0x20, 0xd8, 0x4a, 0xde, 0xd5, 0x00, 0x20,
  0xd8, 0x4a, 0xdf, 0x43, 0x00, 0x20, 0xd8, 0x4a, 0xdf, 0xca, 0x00, 0x20,
  0xd8, 0x4b, 0xdc, 0x51, 0x00, 0x20, 0xd8, 0x4b, 0xdc, 0x55, 0x00, 0x20,
  0xd8, 0x4b, 0xdc, 0xc2, 0x00, 0x20, 0xd8, 0x4b, 0xdd, 0x08, 0x00, 0x20,
  0xd8, 0x4b, 0xdd, 0x4c, 0x00, 0x20, 0xd8, 0x4b, 0xdd, 0x67, 0x00, 0x20,
  0xd8, 0x4b, 0xde, 0xb3, 0x00, 0x20, 0xd8, 0x4f, 0xdc, 0xb7, 0x00, 0x20,
  0xd8, 0x51, 0xdc, 0xd3, 0x00, 0x20, 0xd8, 0x53, 0xdd, 0xb8, 0x00, 0x20,
  0xd8, 0x53, 0xdd, 0xea, 0x00, 0x20, 0xd8, 0x54, 0xdd, 0x2b, 0x00, 0x20,
  0xd8, 0x58, 0xde, 0x58, 0x00, 0x20, 0xd8, 0x59, 0xdf, 0xcc, 0x00, 0x20,
  0xd8, 0x5a, 0xdd, 0xf2, 0x00, 0x20, 0xd8, 0x5a, 0xdd, 0xfa, 0x00, 0x20,
  0xd8, 0x5e, 0xde, 0x3e, 0x00, 0x20, 0xd8, 0x60, 0xdd, 0x5d, 0x00, 0x20,
  0xd8, 0x60, 0xde, 0x07, 0x00, 0x20, 0xd8, 0x60, 0xde, 0xe2, 0x00, 0x20,
  0xd8, 0x63, 0xdc, 0xca, 0x00, 0x20, 0xd8, 0x63, 0xdc, 0xcd, 0x00, 0x20,
  0xd8, 0x63, 0xdc, 0xd2, 0x00, 0x20, 0xd8, 0x67, 0xdd, 0x98
};
static unsigned int UTF16_TestBE_len = 370;

static unsigned char UTF16_TestBEWithBOM[] = {
  0xfe, 0xff, 0xd8, 0x41, 0xdf, 0x0e, 0x00, 0x20, 0xd8, 0x41, 0xdf, 0x31,
  0x00, 0x20, 0xd8, 0x41, 0xdf, 0x79, 0x00, 0x20, 0xd8, 0x43, 0xdc, 0x53,
  0x00, 0x20, 0xd8, 0x43, 0xdc, 0x78, 0x00, 0x20, 0xd8, 0x43, 0xdc, 0x96,
  0x00, 0x20, 0xd8, 0x43, 0xdc, 0xcf, 0x00, 0x20, 0xd8, 0x43, 0xdc, 0xd5,
  0x00, 0x20, 0xd8, 0x43, 0xdd, 0x15, 0x00, 0x20, 0xd8, 0x43, 0xdd, 0x7c,
  0x00, 0x20, 0xd8, 0x43, 0xdd, 0x7f, 0x00, 0x20, 0xd8, 0x43, 0xde, 0x0e,
  0x00, 0x20, 0xd8, 0x43, 0xde, 0x0f, 0x00, 0x20, 0xd8, 0x43, 0xde, 0x77,
  0x00, 0x20, 0xd8, 0x43, 0xde, 0x9d, 0x00, 0x20, 0xd8, 0x43, 0xde, 0xa2,
  0x00, 0x20, 0xd8, 0x43, 0xde, 0xd7, 0x00, 0x20, 0xd8, 0x43, 0xde, 0xf9,
  0x00, 0x20, 0xd8, 0x43, 0xde, 0xfa, 0x00, 0x20, 0xd8, 0x43, 0xdf, 0x2d,
  0x00, 0x20, 0xd8, 0x43, 0xdf, 0x2e, 0x00, 0x20, 0xd8, 0x43, 0xdf, 0x4c,
  0x00, 0x20, 0xd8, 0x43, 0xdf, 0xb4, 0x00, 0x20, 0xd8, 0x43, 0xdf, 0xbc,
  0x00, 0x20, 0xd8, 0x43, 0xdf, 0xea, 0x00, 0x20, 0xd8, 0x44, 0xdc, 0x5c,
  0x00, 0x20, 0xd8, 0x44, 0xdc, 0x6f, 0x00, 0x20, 0xd8, 0x44, 0xdc, 0x75,
  0x00, 0x20, 0xd8, 0x44, 0xdc, 0x76, 0x00, 0x20, 0xd8, 0x44, 0xdc, 0x7b,
  0x00, 0x20, 0xd8, 0x44, 0xdc, 0xc1, 0x00, 0x20, 0xd8, 0x44, 0xdc, 0xc9,
  0x00, 0x20, 0xd8, 0x44, 0xdd, 0xd9, 0x00, 0x20, 0xd8, 0x48, 0xdc, 0xc7,
  0x00, 0x20, 0xd8, 0x49, 0xdf, 0xb5, 0x00, 0x20, 0xd8, 0x4a, 0xde, 0xd5,
  0x00, 0x20, 0xd8, 0x4a, 0xdf, 0x43, 0x00, 0x20, 0xd8, 0x4a, 0xdf, 0xca,
  0x00, 0x20, 0xd8, 0x4b, 0xdc, 0x51, 0x00, 0x20, 0xd8, 0x4b, 0xdc, 0x55,
  0x00, 0x20, 0xd8, 0x4b, 0xdc, 0xc2, 0x00, 0x20, 0xd8, 0x4b, 0xdd, 0x08,
  0x00, 0x20, 0xd8, 0x4b, 0xdd, 0x4c, 0x00, 0x20, 0xd8, 0x4b, 0xdd, 0x67,
  0x00, 0x20, 0xd8, 0x4b, 0xde, 0xb3, 0x00, 0x20, 0xd8, 0x4f, 0xdc, 0xb7,
  0x00, 0x20, 0xd8, 0x51, 0xdc, 0xd3, 0x00, 0x20, 0xd8, 0x53, 0xdd, 0xb8,
  0x00, 0x20, 0xd8, 0x53, 0xdd, 0xea, 0x00, 0x20, 0xd8, 0x54, 0xdd, 0x2b,
  0x00, 0x20, 0xd8, 0x58, 0xde, 0x58, 0x00, 0x20, 0xd8, 0x59, 0xdf, 0xcc,
  0x00, 0x20, 0xd8, 0x5a, 0xdd, 0xf2, 0x00, 0x20, 0xd8, 0x5a, 0xdd, 0xfa,
  0x00, 0x20, 0xd8, 0x5e, 0xde, 0x3e, 0x00, 0x20, 0xd8, 0x60, 0xdd, 0x5d,
  0x00, 0x20, 0xd8, 0x60, 0xde, 0x07, 0x00, 0x20, 0xd8, 0x60, 0xde, 0xe2,
  0x00, 0x20, 0xd8, 0x63, 0xdc, 0xca, 0x00, 0x20, 0xd8, 0x63, 0xdc, 0xcd,
  0x00, 0x20, 0xd8, 0x63, 0xdc, 0xd2, 0x00, 0x20, 0xd8, 0x67, 0xdd, 0x98
};
static unsigned int UTF16_TestBEWithBOM_len = 372;

static unsigned char UTF16_TestLE[] = {
  0x41, 0xd8, 0x0e, 0xdf, 0x20, 0x00, 0x41, 0xd8, 0x31, 0xdf, 0x20, 0x00,
  0x41, 0xd8, 0x79, 0xdf, 0x20, 0x00, 0x43, 0xd8, 0x53, 0xdc, 0x20, 0x00,
  0x43, 0xd8, 0x78, 0xdc, 0x20, 0x00, 0x43, 0xd8, 0x96, 0xdc, 0x20, 0x00,
  0x43, 0xd8, 0xcf, 0xdc, 0x20, 0x00, 0x43, 0xd8, 0xd5, 0xdc, 0x20, 0x00,
  0x43, 0xd8, 0x15, 0xdd, 0x20, 0x00, 0x43, 0xd8, 0x7c, 0xdd, 0x20, 0x00,
  0x43, 0xd8, 0x7f, 0xdd, 0x20, 0x00, 0x43, 0xd8, 0x0e, 0xde, 0x20, 0x00,
  0x43, 0xd8, 0x0f, 0xde, 0x20, 0x00, 0x43, 0xd8, 0x77, 0xde, 0x20, 0x00,
  0x43, 0xd8, 0x9d, 0xde, 0x20, 0x00, 0x43, 0xd8, 0xa2, 0xde, 0x20, 0x00,
  0x43, 0xd8, 0xd7, 0xde, 0x20, 0x00, 0x43, 0xd8, 0xf9, 0xde, 0x20, 0x00,
  0x43, 0xd8, 0xfa, 0xde, 0x20, 0x00, 0x43, 0xd8, 0x2d, 0xdf, 0x20, 0x00,
  0x43, 0xd8, 0x2e, 0xdf, 0x20, 0x00, 0x43, 0xd8, 0x4c, 0xdf, 0x20, 0x00,
  0x43, 0xd8, 0xb4, 0xdf, 0x20, 0x00, 0x43, 0xd8, 0xbc, 0xdf, 0x20, 0x00,
  0x43, 0xd8, 0xea, 0xdf, 0x20, 0x00, 0x44, 0xd8, 0x5c, 0xdc, 0x20, 0x00,
  0x44, 0xd8, 0x6f, 0xdc, 0x20, 0x00, 0x44, 0xd8, 0x75, 0xdc, 0x20, 0x00,
  0x44, 0xd8, 0x76, 0xdc, 0x20, 0x00, 0x44, 0xd8, 0x7b, 0xdc, 0x20, 0x00,
  0x44, 0xd8, 0xc1, 0xdc, 0x20, 0x00, 0x44, 0xd8, 0xc9, 0xdc, 0x20, 0x00,
  0x44, 0xd8, 0xd9, 0xdd, 0x20, 0x00, 0x48, 0xd8, 0xc7, 0xdc, 0x20, 0x00,
  0x49, 0xd8, 0xb5, 0xdf, 0x20, 0x00, 0x4a, 0xd8, 0xd5, 0xde, 0x20, 0x00,
  0x4a, 0xd8, 0x43, 0xdf, 0x20, 0x00, 0x4a, 0xd8, 0xca, 0xdf, 0x20, 0x00,
  0x4b, 0xd8, 0x51, 0xdc, 0x20, 0x00, 0x4b, 0xd8, 0x55, 0xdc, 0x20, 0x00,
  0x4b, 0xd8, 0xc2, 0xdc, 0x20, 0x00, 0x4b, 0xd8, 0x08, 0xdd, 0x20, 0x00,
  0x4b, 0xd8, 0x4c, 0xdd, 0x20, 0x00, 0x4b, 0xd8, 0x67, 0xdd, 0x20, 0x00,
  0x4b, 0xd8, 0xb3, 0xde, 0x20, 0x00, 0x4f, 0xd8, 0xb7, 0xdc, 0x20, 0x00,
  0x51, 0xd8, 0xd3, 0xdc, 0x20, 0x00, 0x53, 0xd8, 0xb8, 0xdd, 0x20, 0x00,
  0x53, 0xd8, 0xea, 0xdd, 0x20, 0x00, 0x54, 0xd8, 0x2b, 0xdd, 0x20, 0x00,
  0x58, 0xd8, 0x58, 0xde, 0x20, 0x00, 0x59, 0xd8, 0xcc, 0xdf, 0x20, 0x00,
  0x5a, 0xd8, 0xf2, 0xdd, 0x20, 0x00, 0x5a, 0xd8, 0xfa, 0xdd, 0x20, 0x00,
  0x5e, 0xd8, 0x3e, 0xde, 0x20, 0x00, 0x60, 0xd8, 0x5d, 0xdd, 0x20, 0x00,
  0x60, 0xd8, 0x07, 0xde, 0x20, 0x00, 0x60, 0xd8, 0xe2, 0xde, 0x20, 0x00,
  0x63, 0xd8, 0xca, 0xdc, 0x20, 0x00, 0x63, 0xd8, 0xcd, 0xdc, 0x20, 0x00,
  0x63, 0xd8, 0xd2, 0xdc, 0x20, 0x00, 0x67, 0xd8, 0x98, 0xdd
};
static unsigned int UTF16_TestLE_len = 370;

static unsigned char UTF16_TestLEWithBOM[] = {
  0xff, 0xfe, 0x41, 0xd8, 0x0e, 0xdf, 0x20, 0x00, 0x41, 0xd8, 0x31, 0xdf,
  0x20, 0x00, 0x41, 0xd8, 0x79, 0xdf, 0x20, 0x00, 0x43, 0xd8, 0x53, 0xdc,
  0x20, 0x00, 0x43, 0xd8, 0x78, 0xdc, 0x20, 0x00, 0x43, 0xd8, 0x96, 0xdc,
  0x20, 0x00, 0x43, 0xd8, 0xcf, 0xdc, 0x20, 0x00, 0x43, 0xd8, 0xd5, 0xdc,
  0x20, 0x00, 0x43, 0xd8, 0x15, 0xdd, 0x20, 0x00, 0x43, 0xd8, 0x7c, 0xdd,
  0x20, 0x00, 0x43, 0xd8, 0x7f, 0xdd, 0x20, 0x00, 0x43, 0xd8, 0x0e, 0xde,
  0x20, 0x00, 0x43, 0xd8, 0x0f, 0xde, 0x20, 0x00, 0x43, 0xd8, 0x77, 0xde,
  0x20, 0x00, 0x43, 0xd8, 0x9d, 0xde, 0x20, 0x00, 0x43, 0xd8, 0xa2, 0xde,
  0x20, 0x00, 0x43, 0xd8, 0xd7, 0xde, 0x20, 0x00, 0x43, 0xd8, 0xf9, 0xde,
  0x20, 0x00, 0x43, 0xd8, 0xfa, 0xde, 0x20, 0x00, 0x43, 0xd8, 0x2d, 0xdf,
  0x20, 0x00, 0x43, 0xd8, 0x2e, 0xdf, 0x20, 0x00, 0x43, 0xd8, 0x4c, 0xdf,
  0x20, 0x00, 0x43, 0xd8, 0xb4, 0xdf, 0x20, 0x00, 0x43, 0xd8, 0xbc, 0xdf,
  0x20, 0x00, 0x43, 0xd8, 0xea, 0xdf, 0x20, 0x00, 0x44, 0xd8, 0x5c, 0xdc,
  0x20, 0x00, 0x44, 0xd8, 0x6f, 0xdc, 0x20, 0x00, 0x44, 0xd8, 0x75, 0xdc,
  0x20, 0x00, 0x44, 0xd8, 0x76, 0xdc, 0x20, 0x00, 0x44, 0xd8, 0x7b, 0xdc,
  0x20, 0x00, 0x44, 0xd8, 0xc1, 0xdc, 0x20, 0x00, 0x44, 0xd8, 0xc9, 0xdc,
  0x20, 0x00, 0x44, 0xd8, 0xd9, 0xdd, 0x20, 0x00, 0x48, 0xd8, 0xc7, 0xdc,
  0x20, 0x00, 0x49, 0xd8, 0xb5, 0xdf, 0x20, 0x00, 0x4a, 0xd8, 0xd5, 0xde,
  0x20, 0x00, 0x4a, 0xd8, 0x43, 0xdf, 0x20, 0x00, 0x4a, 0xd8, 0xca, 0xdf,
  0x20, 0x00, 0x4b, 0xd8, 0x51, 0xdc, 0x20, 0x00, 0x4b, 0xd8, 0x55, 0xdc,
  0x20, 0x00, 0x4b, 0xd8, 0xc2, 0xdc, 0x20, 0x00, 0x4b, 0xd8, 0x08, 0xdd,
  0x20, 0x00, 0x4b, 0xd8, 0x4c, 0xdd, 0x20, 0x00, 0x4b, 0xd8, 0x67, 0xdd,
  0x20, 0x00, 0x4b, 0xd8, 0xb3, 0xde, 0x20, 0x00, 0x4f, 0xd8, 0xb7, 0xdc,
  0x20, 0x00, 0x51, 0xd8, 0xd3, 0xdc, 0x20, 0x00, 0x53, 0xd8, 0xb8, 0xdd,
  0x20, 0x00, 0x53, 0xd8, 0xea, 0xdd, 0x20, 0x00, 0x54, 0xd8, 0x2b, 0xdd,
  0x20, 0x00, 0x58, 0xd8, 0x58, 0xde, 0x20, 0x00, 0x59, 0xd8, 0xcc, 0xdf,
  0x20, 0x00, 0x5a, 0xd8, 0xf2, 0xdd, 0x20, 0x00, 0x5a, 0xd8, 0xfa, 0xdd,
  0x20, 0x00, 0x5e, 0xd8, 0x3e, 0xde, 0x20, 0x00, 0x60, 0xd8, 0x5d, 0xdd,
  0x20, 0x00, 0x60, 0xd8, 0x07, 0xde, 0x20, 0x00, 0x60, 0xd8, 0xe2, 0xde,
  0x20, 0x00, 0x63, 0xd8, 0xca, 0xdc, 0x20, 0x00, 0x63, 0xd8, 0xcd, 0xdc,
  0x20, 0x00, 0x63, 0xd8, 0xd2, 0xdc, 0x20, 0x00, 0x67, 0xd8, 0x98, 0xdd
};
static unsigned int UTF16_TestLEWithBOM_len = 372;

static unsigned char UTF16_TestUTF8[] = {
  0xf0, 0xa0, 0x9c, 0x8e, 0x20, 0xf0, 0xa0, 0x9c, 0xb1, 0x20, 0xf0, 0xa0,
  0x9d, 0xb9, 0x20, 0xf0, 0xa0, 0xb1, 0x93, 0x20, 0xf0, 0xa0, 0xb1, 0xb8,
  0x20, 0xf0, 0xa0, 0xb2, 0x96, 0x20, 0xf0, 0xa0, 0xb3, 0x8f, 0x20, 0xf0,
  0xa0, 0xb3, 0x95, 0x20, 0xf0, 0xa0, 0xb4, 0x95, 0x20, 0xf0, 0xa0, 0xb5,
  0xbc, 0x20, 0xf0, 0xa0, 0xb5, 0xbf, 0x20, 0xf0, 0xa0, 0xb8, 0x8e, 0x20,
  0xf0, 0xa0, 0xb8, 0x8f, 0x20, 0xf0, 0xa0, 0xb9, 0xb7, 0x20, 0xf0, 0xa0,
  0xba, 0x9d, 0x20, 0xf0, 0xa0, 0xba, 0xa2, 0x20, 0xf0, 0xa0, 0xbb, 0x97,
  0x20, 0xf0, 0xa0, 0xbb, 0xb9, 0x20, 0xf0, 0xa0, 0xbb, 0xba, 0x20, 0xf0,
  0xa0, 0xbc, 0xad, 0x20, 0xf0, 0xa0, 0xbc, 0xae, 0x20, 0xf0, 0xa0, 0xbd,
  0x8c, 0x20, 0xf0, 0xa0, 0xbe, 0xb4, 0x20, 0xf0, 0xa0, 0xbe, 0xbc, 0x20,
  0xf0, 0xa0, 0xbf, 0xaa, 0x20, 0xf0, 0xa1, 0x81, 0x9c, 0x20, 0xf0, 0xa1,
  0x81, 0xaf, 0x20, 0xf0, 0xa1, 0x81, 0xb5, 0x20, 0xf0, 0xa1, 0x81, 0xb6,
  0x20, 0xf0, 0xa1, 0x81, 0xbb, 0x20, 0xf0, 0xa1, 0x83, 0x81, 0x20, 0xf0,
  0xa1, 0x83, 0x89, 0x20, 0xf0, 0xa1, 0x87, 0x99, 0x20, 0xf0, 0xa2, 0x83,
  0x87, 0x20, 0xf0, 0xa2, 0x9e, 0xb5, 0x20, 0xf0, 0xa2, 0xab, 0x95, 0x20,
  0xf0, 0xa2, 0xad, 0x83, 0x20, 0xf0, 0xa2, 0xaf, 0x8a, 0x20, 0xf0, 0xa2,
  0xb1, 0x91, 0x20, 0xf0, 0xa2, 0xb1, 0x95, 0x20, 0xf0, 0xa2, 0xb3, 0x82,
  0x20, 0xf0, 0xa2, 0xb4, 0x88, 0x20, 0xf0, 0xa2, 0xb5, 0x8c, 0x20, 0xf0,
  0xa2, 0xb5, 0xa7, 0x20, 0xf0, 0xa2, 0xba, 0xb3, 0x20, 0xf0, 0xa3, 0xb2,
  0xb7, 0x20, 0xf0, 0xa4, 0x93, 0x93, 0x20, 0xf0, 0xa4, 0xb6, 0xb8, 0x20,
  0xf0, 0xa4, 0xb7, 0xaa, 0x20, 0xf0, 0xa5, 0x84, 0xab, 0x20, 0xf0, 0xa6,
  0x89, 0x98, 0x20, 0xf0, 0xa6, 0x9f, 0x8c, 0x20, 0xf0, 0xa6, 0xa7, 0xb2,
  0x20, 0xf0, 0xa6, 0xa7, 0xba, 0x20, 0xf0, 0xa7, 0xa8, 0xbe, 0x20, 0xf0,
  0xa8, 0x85, 0x9d, 0x20, 0xf0, 0xa8, 0x88, 0x87, 0x20, 0xf0, 0xa8, 0x8b,
  0xa2, 0x20, 0xf0, 0xa8, 0xb3, 0x8a, 0x20, 0xf0, 0xa8, 0xb3, 0x8d, 0x20,
  0xf0, 0xa8, 0xb3, 0x92, 0x20, 0xf0, 0xa9, 0xb6, 0x98
};
static unsigned int UTF16_TestUTF8_len = 309;

void TEST_PrimUnitTests_UTF16Decode();
void TEST_PrimUnitTests_UTF16Decode()
{
  using namespace prim;
  Array<String> TestFiles;
  TestFiles.Add() = String(UTF16_TestUTF8, count(UTF16_TestUTF8_len));
  TestFiles.Add() = String(UTF16_TestLE, count(UTF16_TestLE_len));
  TestFiles.Add() = String(UTF16_TestLEWithBOM, count(UTF16_TestLEWithBOM_len));
  TestFiles.Add() = String(UTF16_TestBE, count(UTF16_TestBE_len));
  TestFiles.Add() = String(UTF16_TestBEWithBOM, count(UTF16_TestBEWithBOM_len));

  Array<String> Outs;
  for(count i = 0; i < TestFiles.n(); i++)
  {
    String Filename = TestFiles[i];
    Outs.Add() = String::UTF16::Decode(File::Read(Filename));
    String Convert = File::Read(Filename);
    Convert.ConvertToUTF8();
    EXPECT_EQ(Outs.z(), Convert);
  }
  for(count i = 0; i < Outs.n(); i++)
    EXPECT_EQ(Outs[i], Outs.a());
}

////////////////////////////////////////////////////////////////////////////////

void TEST_PrimUnitTests_UUIDv4NoDuplicates();
void TEST_PrimUnitTests_UUIDv4NoDuplicates()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "UUIDv4NoDuplicates";

  bool Duplicates = false;
  Array<UUIDv4> x(100);
  for(count i = 0; i < x.n(); i++)
    for(count j = 0; j < x.n(); j++)
      if(i != j && x[i] == x[j])
        C::Out() >> "Duplicate found: " << i << " and " << j, Duplicates = true;
  EXPECT_EQ(Duplicates, false);
}

////////////////////////////////////////////////////////////////////////////////

void TEST_PrimUnitTests_ValueNilTest();
void TEST_PrimUnitTests_ValueNilTest()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "ValueNilTest";
  Value v;
  const Value& v_const = v;

  //Type-knowledge
  EXPECT_EQ(true,  v.IsNil());
  EXPECT_EQ(false, v.IsArray());
  EXPECT_EQ(false, v.IsBoolean());
  EXPECT_EQ(false, v.IsInteger());
  EXPECT_EQ(false, v.IsNumber());
  EXPECT_EQ(false, v.IsObject());
  EXPECT_EQ(false, v.IsRatio());
  EXPECT_EQ(false, v.IsBox());
  EXPECT_EQ(false, v.IsString());
  EXPECT_EQ(false, v.IsTree());
  EXPECT_EQ(false, v.IsVector());

  //Indexing
  EXPECT_EQ(0,    v.n());
  EXPECT_EQ(true, v_const[0].IsNil());
  EXPECT_EQ(true, v_const[""].IsNil());

  //Comparison
  EXPECT_EQ(true,  v == Value());
  EXPECT_EQ(false, v == Value(false));
  EXPECT_EQ(false, v == Value(0));
  EXPECT_EQ(false, v == Value("abc"));
  EXPECT_EQ(false, v == Value(Vector(0, 0)));
  EXPECT_EQ(false, v == Value(Ratio(0, 1)));

  EXPECT_EQ(true,  v != Value(true));

  EXPECT_EQ(false, v < Value());
  EXPECT_EQ(true,  v < Value(false));
  EXPECT_EQ(true,  v < Value(0));
  EXPECT_EQ(true,  v < Value("abc"));
  EXPECT_EQ(true,  v < Value(Vector(0, 0)));
  EXPECT_EQ(true,  v < Value(Ratio(0)));

  EXPECT_EQ(false, v > Value());

  EXPECT_EQ(true,  v <= Value());
  EXPECT_EQ(true,  v >= Value());

  //Arithmetic
  EXPECT_EQ(true, Value() == Value(+v));
  EXPECT_EQ(true, Value() == Value(-v));

  //Export
  EXPECT_EQ(String("[null]"), v.ExportJSON(false));

  //Type coercion
  EXPECT_EQ(0, v.AsArray().n());
  EXPECT_EQ(false, v.AsBoolean());
  EXPECT_EQ(Nothing<integer>(), v.AsInteger());
  EXPECT_EQ(Nothing<number>(), v.AsNumber());
  EXPECT_EQ(Ratio(), v.AsRatio());
  EXPECT_EQ(Box(), v.AsBox());
  EXPECT_EQ(String("Nil"), v.AsString());
  EXPECT_EQ(0, v.AsTree().n());
  EXPECT_EQ(Vector::Empty(), v.AsVector());

  //Comparison with coerced types
  EXPECT_EQ(true, false == bool(v));
  EXPECT_EQ(true, Nothing<integer>() == integer(v));
  EXPECT_EQ(true, Nothing<number>() == number(v));
  EXPECT_EQ(true, Ratio() == v.AsRatio());
  EXPECT_EQ(true, Box() == v);
  EXPECT_EQ(true, String("Nil") == v.AsString());
  EXPECT_EQ(true, Vector::Empty() == v);
  EXPECT_EQ(true, reinterpret_cast<const void*>(0) ==
    reinterpret_cast<const void*>(v.ConstObject().Raw()));
  EXPECT_EQ(true, reinterpret_cast<const void*>(0) ==
    reinterpret_cast<const void*>(v.Object().Raw()));

  //Implicit coercion to nil
  EXPECT_EQ(true, v == Value(0.0 / 0.0));
  EXPECT_EQ(true, v == Value(Nothing<integer>()));
  EXPECT_EQ(true, v == Value(Nothing<number>()));
  EXPECT_EQ(true, v == Value(Ratio()));
  EXPECT_EQ(true, v == Value(Vector::Empty()));
  EXPECT_EQ(true, v == Value(Box()));
  EXPECT_EQ(true, v == Value(Pointer<Value::Base>()));
}

////////////////////////////////////////////////////////////////////////////////

void TEST_PrimUnitTests_XMLParse();
void TEST_PrimUnitTests_XMLParse()
{
  C::Out() >> "Testing: " "PrimUnitTests" " - " "XMLParse";

  String s;
  s >> "<test foo=\"2\">";
  s >> "  <subtest bar='2'>";
  s >> "    some text";
  s >> "  </subtest>";
  s >> "</test>";

  for(count i = 0; i < 10; i++)
  {
    XML::Document d;
    XML::Parser::Error e = d.ParseDocument(s);
    bool WasError = e;
    EXPECT_EQ(false, WasError);
    s.Clear();
    d.WriteToString(s);
    bool IsNotEmpty = bool(s);
    EXPECT_EQ(true, IsNotEmpty);
  }
}

////////////////////////////////////////////////////////////////////////////////

void RunAllTests();
void RunAllTests()
{
  TEST_PrimUnitTests_AESRoundtrips();
  TEST_PrimUnitTests_AESReference();
  TEST_PrimUnitTests_Base64Decode();
  TEST_PrimUnitTests_Base64Encode();
  TEST_PrimUnitTests_EndianConversion();
  TEST_PrimUnitTests_FFTStressTest();
  TEST_PrimUnitTests_JSONValid();
  TEST_PrimUnitTests_JSONInvalid();
  TEST_PrimUnitTests_MD5Calculate();
  TEST_PrimUnitTests_MIDI();
  TEST_PrimUnitTests_NothingComparison();
  TEST_PrimUnitTests_ListQuicksort();
  TEST_PrimUnitTests_ListBubblesort();
  TEST_PrimUnitTests_ArrayQuicksort();
  TEST_PrimUnitTests_SwappableArrayQuicksort();
  TEST_PrimUnitTests_TreeSmokeTest();
  TEST_PrimUnitTests_TreeLargeInsertion();
  TEST_PrimUnitTests_TreeLargeRemoval();
  TEST_PrimUnitTests_TreeLargePruning();
  TEST_PrimUnitTests_TreeTrimming();
  TEST_PrimUnitTests_TreeIterating();
  TEST_PrimUnitTests_TreeLargeDeepCopy();
  TEST_PrimUnitTests_UTF16Decode();
  TEST_PrimUnitTests_UUIDv4NoDuplicates();
  TEST_PrimUnitTests_ValueNilTest();
  TEST_PrimUnitTests_XMLParse();
}

int main()
{
  AutoRelease<Console> AutoReleasePool;
  RunAllTests();
  C::Out()++;
  if(ChecksFailed)
  {
    C::Out() >> "Failed " << ChecksFailed << " of " << ChecksRun << " tests";
    return 1;
  }
  else
  {
    C::Out() >> "All tests passed.";
  }
  return 0;
}
