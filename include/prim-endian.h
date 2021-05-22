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

#ifndef PRIM_INCLUDE_ENDIAN_H
#define PRIM_INCLUDE_ENDIAN_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///Checks endianness to make requested endianness conversions.
  class Endian
  {
    public:

    ///Returns true if the system is little-endian (low-to-high).
    static inline bool IsLittleEndian()
    {
#ifdef PRIM_ENVIRONMENT_LITTLE_ENDIAN
      return true;
#else
      return false;
#endif
    }

    ///Reverses the byte order of an array of bytes.
    static void ReverseByteOrder(void* Data, count BytesToReverse)
    {
      //Access the data by byte.
      byte* ByteData = reinterpret_cast<byte*>(Data);

      /*General reverse order algorithm. In the case of an odd number of bytes,
      the middle byte is not swapped since BytesToReverse / 2 is floored.*/
      for(count i = 0, j = BytesToReverse - 1, n = BytesToReverse / 2;
        i < n; i++, j--)
      {
        //Naive swap the bytes.
        byte t = ByteData[i];
        ByteData[i] = ByteData[j];
        ByteData[j] = t;
      }
    }

    ///Reverses the byte order of any complex data type.
    template <class T>
    static void ReverseOrder(T& Value)
    {
      ReverseByteOrder(&Value, sizeof(T));
    }

    ///Ensures that a value is stored in little-endian.
    template <class T>
    static void ConvertToLittleEndian(T& Value)
    {
      if(not IsLittleEndian())
        ReverseOrder(Value);
    }

    ///Ensures that a value is stored in big-endian.
    template <class T>
    static void ConvertToBigEndian(T& Value)
    {
      if(IsLittleEndian())
        ReverseOrder(Value);
    }

    ///Ensures that an array of items is stored in little-endian.
    template <class T>
    static void ConvertToLittleEndian(T* ItemArray, count ItemCount)
    {
      if(not IsLittleEndian())
        for(count i = 0; i < ItemCount; i++)
          ReverseOrder(ItemArray[i]);
    }

    ///Ensures that an array of items is stored in big-endian.
    template <class T>
    static void ConvertToBigEndian(T* ItemArray, count ItemCount)
    {
      if(IsLittleEndian())
        for(count i = 0; i < ItemCount; i++)
          ReverseOrder(ItemArray[i]);
    }
  };
}
#endif
