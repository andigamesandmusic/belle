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

#define PRIM_WITH_AES
#define PRIM_COMPILE_INLINE
#include "prim.h"

using namespace prim;

static void RoundtripTests()
{
  C::Out() >> "Performing roundtrip tests:";
  Random R(12345);
  bool Failed = false;
  for(count i = 0; i <= 10000; i++)
  {
    if(i % 1000 == 0)
      C::Out() >> "Testing random array, key, and IV of length " << i << "...";
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
  if(Failed)
    C::Error() >> "Roundtrip test failed.";
  else
    C::Out() >> "Roundtrip test succeeded.";
}

int main()
{
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

  C::Out() >> "Original:" >> Plaintext;
  C::Out()++;
  C::Out() >> "Encrypted:" >> Encrypted;
  C::Out()++;
  C::Out() >> "Reference:" >> Reference;
  C::Out()++;
  C::Out() >> "Decrypted:" >> Decrypted;
  C::Out()++;
  C::Out() >> "Encrypted " <<
    ((Encrypted == Reference) ? "==" : "!=") << " Reference";
  C::Out() >> "Decrypted " <<
    ((Decrypted == Plaintext) ? "==" : "!=") << " Plaintext";
  C::Out()++;
  C::Out() >> "Padded length:     " << Encrypted.n();
  C::Out() >> "Calculated length: " <<
    AES::CBC256DecryptedLength(Encrypted, Key, Key);
  C::Out() >> "Actual length:     " << Plaintext.n();
  C::Out()++;
  RoundtripTests();
  return AutoRelease<Console>();
}
