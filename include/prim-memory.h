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

#ifndef PRIM_INCLUDE_MEMORY_H
#define PRIM_INCLUDE_MEMORY_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

/** \file
Provides a wrapper for memory operations via the Memory class.*/

namespace PRIM_NAMESPACE
{
  /**Wrapper for clearing and copying memory via templated methods.Also
  provides a templated placement new. You should only use the Clear and Copy
  methods on plain-old data objects, that is, objects not containing members
  with their own special constructors or vtables.*/
  class Memory
  {
    public:

    ///Wrapper for memset. Use Clear instead since it is strongly typed.
    static void MemSet(void* Destination, byte ValueToSet, count BytesToSet);

    ///Wrapper for memcpy. Use Copy instead since it is strongly typed.
    static void MemCopy(void* Destination, const void* Source,
      count BytesToCopy);

    ///Clears an object with a particular byte-value.
    template <class T>
    static void Clear(T& Object, byte Value = 0)
    {
      MemSet(reinterpret_cast<void*>(&Object), Value, sizeof(T));
    }

    ///Clears an array of consecutive objects with a particular byte-value.
    template <class T>
    static void Clear(T* Object, count Items, byte Value = 0)
    {
      MemSet(reinterpret_cast<void*>(Object), Value, count(sizeof(T)) * Items);
    }

    ///Copies an object's data to another object.
    template <class T>
    static void Copy(T& Destination, const T& Source)
    {
      MemCopy(reinterpret_cast<void*>(&Destination),
        reinterpret_cast<const void*>(&Source), count(sizeof(T)));
    }

    ///Copies an array of consecutive objects to another array.
    template <class T>
    static void Copy(T* Destination, const T* Source, count Items)
    {
      MemCopy(reinterpret_cast<void*>(Destination),
        reinterpret_cast<const void*>(Source),
        count(sizeof(T)) * Items);
    }

    /**Calls a placement new on an object. A placement new is used to call an
    object's constructor in the case that the memory for the object has already
    been allocated. To use this method you will need to include &lt;new&gt;
    before you include prim.h.*/
    template <class T>
    static T* PlacementNew(T* Object)
    {
      return new (Object) T;
    }

    /**Safely swaps two objects of the same type using an intermediate object.
    This requires a default constructor and a copy constructor for the temporary
    placeholder as well as an assignment operator if the type is a class.*/
    template <class T>
    static void Swap(T& a, T& b)
    {
      T c = a;
      a = b;
      b = c;
    }

    /**Iterates through two arrays of the same type and size safely swapping
    their elements using an intermediate object. Internally calls Swap(a, b).*/
    template <class T>
    static void Swap(T* a, T* b, count Items)
    {
      for(count i = 0; i < Items; i++)
        Swap(a[i], b[i]);
    }

    ///Swaps the memory of a single object. Internally calls Swap(a, b, Items).
    template <class T>
    static void SwapMemory(T& a, T& b)
    {
      SwapMemory(&a, &b, 1);
    }

    /**Swaps the memory of two arrays using an un-typed temporary buffer. By
    doing this you avoid the construction-destruction of the temporary, as well
    as the assignment operator. Obviously, you must be careful to do this only
    when the result is defined. The regions of memory may not overlap.*/
    template <class T>
    static void SwapMemory(T* a, T* b, count Items)
    {
      if(a == b)
        return;

      const count BufferSize = 1024;
      byte Buffer[BufferSize];
      byte* a_byte = reinterpret_cast<byte*>(a);
      byte* b_byte = reinterpret_cast<byte*>(b);
      count BytesToSwap = count(sizeof(T)) * Items;
      while(BytesToSwap > 0)
      {
        count BytesToCopy = BytesToSwap < BufferSize ? BytesToSwap : BufferSize;
        Copy(&Buffer[0], a_byte, BytesToCopy);
        Copy(a_byte, b_byte, BytesToCopy);
        Copy(b_byte, &Buffer[0], BytesToCopy);
        a_byte += BytesToCopy;
        b_byte += BytesToCopy;
        BytesToSwap -= BytesToCopy;
      }
    }
  };

#ifdef PRIM_COMPILE_INLINE
  void Memory::MemSet(void* Destination, uint8 ValueToSet, count BytesToSet)
  {
    if(Destination != 0 and BytesToSet > 0)
      memset(Destination, ValueToSet, size_t(BytesToSet));
  }

  void Memory::MemCopy(void* Destination, const void* Source, count BytesToCopy)
  {
#ifndef __clang_analyzer__
    if(Destination != 0 and Source != 0 and BytesToCopy > 0)
      memcpy(Destination, Source, size_t(BytesToCopy));
#endif
  }
#endif
}
#endif
