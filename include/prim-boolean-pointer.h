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

#ifndef PRIM_BOOLEAN_POINTER_H
#define PRIM_BOOLEAN_POINTER_H

namespace PRIM_NAMESPACE
{
  /**Special pointer that stores an extra bit of information in the LSB. This
  works since all modern processors have an alignment greater than 1 byte, and
  therefore the LSB for any pointer is always zero. This class is mainly useful
  in scenarios where there is a boolean expression associated with a pointer in
  a container class.*/
  template <class T>
  class BooleanPointer
  {
    ///Stores the boolean-pointer encoding in a pointer-size unsigned integer.
    uintptr EncodedPointer;

    ///Returns the AND mask used for getting just the pointer.
    static uintptr PointerMask()
    {
      //All bits on except LSB.
      return ~BooleanMask();
    }

    ///Returns the AND mask used for getting just the boolean.
    static uintptr BooleanMask()
    {
      //All bits off except LSB.
      return uintptr(1);
    }

    ///Gets the pointer as pointer-size unsigned integer.
    static uintptr RawPointer(T* Object)
    {
      return reinterpret_cast<uintptr>(Object);
    }

    ///Gets the boolean as pointer-size unsigned integer.
    static uintptr RawBoolean(bool Value)
    {
      //In C++, a boolean converts to an integer with 0 as false and 1 as true.
      return uintptr(Value);
    }

    public:

    ///Creates a boolean-pointer of value false and null.
    BooleanPointer() : EncodedPointer(0) {}

    ///Creates a boolean-pointer using an existing pointer and boolean false.
    BooleanPointer(T* Object) {SetPointerAndBoolean(Object, false);}

    ///Creates a boolean-pointer using an existing pointer and boolean value.
    BooleanPointer(T* Object, bool Value) {SetPointerAndBoolean(Object, Value);}

    ///Gets the type-enforced pointer object.
    T* GetPointer() const
    {
      return reinterpret_cast<T*>(EncodedPointer & PointerMask());
    }

    /**Pointer dereference operator. The same rules apply here as for regular
    pointers. Dereferencing a null pointer will cause an exception.*/
    T* operator -> () const
    {
      return GetPointer();
    }

    /**Dereference operator. The same rules apply here as for regular pointers.
    Dereferencing a null pointer will cause an exception.*/
    T& operator * () const
    {
      return *GetPointer();
    }

    ///Returns whether the pointer is not null.
    bool IsNotNull() const
    {
      return GetPointer();
    }

    ///Returns whether the pointer is null.
    bool IsNull() const
    {
      return !GetPointer();
    }

    ///Gets the boolean value.
    bool GetBoolean() const
    {
      return bool(EncodedPointer & BooleanMask());
    }

    ///Gets the boolean value.
    operator bool () const
    {
      return GetBoolean();
    }

    ///Sets the type-enforced pointer object for this boolean-pointer.
    void SetPointer(T* Object)
    {
      EncodedPointer &= BooleanMask();
      EncodedPointer |= RawPointer(Object);
    }

    ///Deletes pointer to any object if it exists and sets pointer to null.
    void Delete()
    {
      delete GetPointer();
      SetPointer(0);
    }

    ///Instantiates a new object, deleting any previous object if it exists.
    void New()
    {
      Delete();
      SetPointer(new T);
    }

    ///Sets the boolean value for this boolean-pointer.
    void SetBoolean(bool Value)
    {
      EncodedPointer &= PointerMask();
      EncodedPointer |= RawBoolean(Value);
    }

    ///Sets both the pointer and boolean value at the same time.
    void SetPointerAndBoolean(T* Object, bool Value)
    {
      EncodedPointer = RawPointer(Object) | RawBoolean(Value);
    }

    ///Swaps two boolean pointers.
    static void Swap(BooleanPointer& x, BooleanPointer& y)
    {
      uintptr t = x.EncodedPointer;
      x.EncodedPointer = y.EncodedPointer;
      y.EncodedPointer = t;
    }

    ///Swaps just the pointers of two boolean pointers.
    static void SwapPointer(BooleanPointer& x, BooleanPointer& y)
    {
      T* t = x.GetPointer();
      x.SetPointer(y.GetPointer());
      y.SetPointer(t);
    }

    ///Swaps just the boolean values of two boolean pointers.
    static void SwapBoolean(BooleanPointer& x, BooleanPointer& y)
    {
      bool t = x.GetBoolean();
      x.SetBoolean(y.GetBoolean());
      y.SetBoolean(t);
    }
  };
}
#endif
