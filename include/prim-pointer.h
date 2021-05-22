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

#ifndef PRIM_INCLUDE_POINTER_H
#define PRIM_INCLUDE_POINTER_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  namespace meta
  {
    //Forward declaration
    template <class T> class WeakPointer;

    /**Internal storage class for keeping track of ownership to an object. One
    of these is held per owned object. There is no reason to use this class
    directly.*/
    class PointerOwner
    {
      public:

      ///Pointer to the owned object.
      bool OwnedPointerExists; PRIM_PAD(bool)

      ///Reference count of strong and weak handles to the owner object.
      count OwnerReferenceCount;

      ///Reference count of strong handles to the actual owned object.
      count ReferenceCount;

      ///Constructor to take ownership of a pointer.
      PointerOwner() : OwnedPointerExists(true), OwnerReferenceCount(1),
        ReferenceCount(1) {}
    };
  }

  /**Shared pointer with reference counting for owning and owned object. See
  also the meta::WeakPointer<T> (Pointer<T>::Weak) class for the mutually
  compatible weak pointer form.

  This class replaces traditional pointers T* with Pointer<T> and
  Pointer<T>::Weak. Correct operation is minimally depedent on the user, but
  there are a few rules:

  - A new object must only be assigned once to a Pointer<T>. Pointer<T>'s
  may be freely copied.
  - Raw() should only be used in one of a few circumstances:
    - Crossing a library boundary (third-party library needs the actual pointer)
    - Inspecting the address the pointer points to (for debugging).

  Additionally, the Pointer class is polymorphic and can handle things like:
  \code
  Pointer<Base> B = new Derived;
  Pointer<Derived> D = new Derived;
  B = D;
  \endcode

  It is also much easier to differentiate among derived types. The following:
  \code
  Base* b = ...
  if(Derived1* d1 = dynamic_cast<Derived1*>(b))
    ...
  else if(Derived2* d2 = dynamic_cast<Derived2*>(b))
    ...
  etc.
  \endcode

  Becomes:
  \code
  Pointer<Base> b = ...
  if(Pointer<Derived1> d1 = b)
    ...
  else if(Pointer<Derived2> d2 = b)
    ...
  etc.
  \endcode
  */
  template <class T> class Pointer
  {
    public:

    /**Pointer needs to be friends with pointers of other types in order to
    facilitate base-derived conversions.*/
    template <class A> friend class Pointer;

    ///Makes Pointer<T>::Weak equivalent to WeakPointer<T>.
    typedef meta::WeakPointer<T> Weak;

    ///Default constructor creates a null pointer.
    Pointer() : Reference(0), CachedPointer(0) {}

    ///Owning constructor
    Pointer(T* PointerToOwn) : Reference(0),
      CachedPointer(PointerToOwn)
    {
      //If the incoming pointer is set to null, then leave the pointer as null.
      if(not PointerToOwn)
        return;

      //Create a new owner for the object and initialize its state.
      Reference = new meta::PointerOwner;
      Reference->OwnerReferenceCount = 1;
      Reference->ReferenceCount = 1;
    }

    ///Copy constructor to increment reference counts.
    Pointer(const Pointer<T>& PointerToShare) : Reference(0), CachedPointer(0)
    {
      //Share the incoming pointer.
      Share(PointerToShare, false);
    }

    ///Fast swaps a pointer with another.
    void Swap(Pointer<T>& Other)
    {
      PRIM_NAMESPACE::Swap(Reference, Other.Reference);
      PRIM_NAMESPACE::Swap(CachedPointer, Other.CachedPointer);
    }

    ///Assignment operator to increment reference counts.
    Pointer<T>& operator = (const Pointer<T>& PointerToShare)
    {
      //Prevent self-copy.
      if(Raw() == PointerToShare.Raw())
        return *this;

      //Share the incoming pointer.
      Share(PointerToShare, false);
      return *this;
    }

    ///Assignment operator for recently pointer to new object.
    Pointer<T>& operator = (T* PointerToOwn)
    {
      //Prevent self-copy.
      if(Raw() == PointerToOwn)
        return *this;

      //Clear any preexisting pointer.
      Unshare(false);

      //If the incoming pointer is null, then return.
      if(not PointerToOwn)
        return *this;

      //Create a new owner for the object and set initialize its state.
      Reference = new meta::PointerOwner;
      Reference->OwnerReferenceCount = 1;
      Reference->ReferenceCount = 1;
      CachedPointer = PointerToOwn;
      return *this;
    }

    /**Destructor to decrement reference counts. If the reference count is zero,
    then the object pointed to is deleted automatically.*/
    ~Pointer()
    {
      //Unshare any existing pointer.
      Unshare(false);
    }

    /**Obtains a raw (untracked) pointer to the owned object. Raw pointers are
    like children and should not outlast their parent's scope. They are not
    tracked by the reference counting mechanism. Raw() should only be called
    at a library boundary or in the case of needing to do pointer arithmetic.
    All other uses should simply use the pointer class as though it were a
    regular pointer.*/
    T* Raw() const
    {
      //If an object exists, return a pointer to it, otherwise null.
      return (Reference and Reference->OwnedPointerExists ? CachedPointer : 0);
    }

    ///Creates a new object of the type given by the pointer.
    Pointer<T> New() {*this = new T; return *this;}

    /**Returns a shared pointer of a base or derived type. The new pointer will
    have all the same ownership rights as source. A dynamic cast is used to
    check the safety of a base to derived cast, and so if it fails, the new
    pointer will simply be null.*/
    template <class BaseOrDerived> Pointer<BaseOrDerived> As() const
    {
      /*Cast to base-to-derived and back. This forces the compiler to do at
      least one polymorphic cast and ensures that the class is actually
      polymorphic (compiler will give an error if not).*/
      BaseOrDerived* BaseCached = dynamic_cast<BaseOrDerived*>(CachedPointer);
      T* ReturnTripEnforce = dynamic_cast<T*>(BaseCached);

      //Create base (or derived) pointer.
      Pointer<BaseOrDerived> BasePointer;

      //If not null then initialize the pointer and up the reference counts.
      if(Reference and BaseCached and ReturnTripEnforce)
      {
        //Copy the pointer data over.
        BasePointer.Reference = Reference;
        BasePointer.CachedPointer = BaseCached;

        //Increase the reference counts.
        Reference->OwnerReferenceCount++;
        Reference->ReferenceCount++;
      }
      return BasePointer;
    }

    ///Automatic dynamic-casting to a base or derived pointer type.
    template <class BaseOrDerived> operator Pointer<BaseOrDerived> () const
    {
      return As<BaseOrDerived>();
    }

    /**Returns a const-object version of the pointer. This is the explicit form
    of the operator Pointer<const T>() method, which should perform this
    conversion automatically. Note that constness for pointers comes in two
    dimensions: const pointers and pointers to const objects (that could be
    const as well).*/
    Pointer<const T> Const() const
    {
      //Create a const pointer already in an unshared state.
      Pointer<const T> Consted;

      //Return immediately if null reference or null object.
      if(not Reference or not Reference->OwnedPointerExists) return Consted;

      //Copy the reference and cached pointer.
      Consted.Reference = Reference;
      Consted.CachedPointer = CachedPointer;

      //Increment reference counts.
      Reference->OwnerReferenceCount++;
      Reference->ReferenceCount++;

      //Return the new const pointer.
      return Consted;
    }

    ///Returns a const-object version of the pointer.
    operator Pointer<const T> () const
    {
      return Const();
    }

    /**Returns the number of handles on the owned object. If IncludeWeak is
    false, this method returns the number of strong handles. If true, this
    method returns the total number of handles, both strong and weak.*/
    count n(bool IncludeWeak = false) const
    {
      if(Reference)
      {
        if(IncludeWeak)
          return Reference->OwnerReferenceCount;
        else
          return Reference->ReferenceCount;
      }
      return 0;
    }

    ///Checks whether or not the pointer to the object is non-null.
    operator bool () const
    {
      return Raw() != 0;
    }

    ///Checks whether or not two pointers point to the same object.
    bool operator == (const Pointer<T>& Other) const
    {
      return Raw() == Other.Raw();
    }

    ///Checks whether or not two pointers do not point to the same object.
    bool operator != (const Pointer<T>& Other) const
    {
      return Raw() != Other.Raw();
    }

    ///Checks whether the address of the first is less.
    bool operator < (const Pointer<T>& Other) const
    {
      return Raw() < Other.Raw();
    }

    ///Checks whether the address of the first is less than or equal.
    bool operator <= (const Pointer<T>& Other) const
    {
      return Raw() <= Other.Raw();
    }

    ///Checks whether the address of the first is greater.
    bool operator > (const Pointer<T>& Other) const
    {
      return Raw() > Other.Raw();
    }

    ///Checks whether the address of the first is greater than or equal.
    bool operator >= (const Pointer<T>& Other) const
    {
      return Raw() >= Other.Raw();
    }

    /**Pointer dereference operator. The same rules apply here as for regular
    pointers. Dereferencing a null pointer will cause an exception.*/
    T* operator -> () const
    {
      return Raw();
    }

    /**Dereference operator. The same rules apply here as for regular pointers.
    Dereferencing a null pointer will cause an exception.*/
    T& operator * () const
    {
      return *Raw();
    }

    protected:

    ///Stores a reference to the Pointer::Owner.
    meta::PointerOwner* Reference;

    ///Stores a polymorphically correct pointer.
    T* CachedPointer;

    ///Shares an existing pointer, possibly weakly.
    void Share(const Pointer<T>& PointerToShare, bool Weak)
    {
      //First unshare any existing pointer.
      Unshare(Weak);

      //Get the owner reference, and return if it is null.
      Reference = PointerToShare.Reference;
      CachedPointer = 0;
      if(not Reference)
        return;
      CachedPointer = PointerToShare.CachedPointer;

      //Do not propogate ownership of a null pointer.
      if(not Reference->OwnedPointerExists)
      {
        Reference = 0;
        CachedPointer = 0;
        return;
      }

      //Increment reference counts.
      Reference->OwnerReferenceCount++;
      if(not Weak)
        Reference->ReferenceCount++;
    }

    /**Unshares the stored reference, possibly weakly. If no other pointers are
    holding a reference (the reference count is 0), then the object it points
    to is deleted.*/
    void Unshare(bool Weak)
    {
      /*This method currently triggers new-delete warnings in the Clang static
      analyzer due to limitations and/or bugs. The correctness of this method
      can be more easily verified using leak detection and guard malloc.*/
      //Return if there is no owner reference.
      if(not Reference)
      {
        //Uncache any existing pointer.
        CachedPointer = 0;
        return;
      }

      //Decrement object handle reference count and delete if no longer used.
      if(not Weak)
      {
        Reference->ReferenceCount--;
        if(not Reference->ReferenceCount)
        {
          delete CachedPointer;
          Reference->OwnedPointerExists = false;
        }
      }

      //Uncache any existing pointer.
      CachedPointer = 0;

      //Decrement the owner handle reference count and delete if no longer used.
      Reference->OwnerReferenceCount--;
      if(not Reference->OwnerReferenceCount)
        delete Reference;

      //Clear the owner reference.
      Reference = 0;
    }
  };

  namespace meta
  {
    /**Shares ownership of the meta::PointerOwner but not the actual pointer.
    This is useful for passive cached pointers that should not affect garbage
    collection. It is also useful for determining at a distance whether a
    pointer is still active. With a normal Pointer, the object could never be
    deleted since it would be shared. Use the Pointer<T>::Weak form instead of
    meta::WeakPointer<T>. In a weak pointer, a reference is kept to the owner
    of the object instead of the object itself.*/
    template <class T> class WeakPointer : public Pointer<T>
    {
      public:

      ///Default constructor creates a null pointer.
      WeakPointer() {}

      ///Copy constructor taking in an existing weak pointer.
      WeakPointer(const WeakPointer<T>& PointerToShare) : Pointer<T>()
      {
        //Share the incoming pointer weakly.
        Pointer<T>::Share(PointerToShare, true);
      }

      ///Copy constructor to increment owner but not object reference count.
      WeakPointer(const Pointer<T>& PointerToShare)
      {
        //Share the incoming pointer weakly.
        Pointer<T>::Share(PointerToShare, true);
      }

      ///Assignment operator to increment owner but not object reference count.
      WeakPointer<T>& operator = (const Pointer<T>& PointerToShare)
      {
        //Prevent self-copy.
        if(Pointer<T>::Raw() == PointerToShare.Raw())
          return *this;

        //Share the incoming pointer weakly.
        Pointer<T>::Share(PointerToShare, true);
        return *this;
      }

      /**Assignment operator for assigning null pointers only. Passing a new
      object will result in its deletion. Its reference count becomes zero
      immediately, since no non-weak pointers hold it.*/
      WeakPointer<T>& operator = (T* PointerToOwn)
      {
        //Prevent self-copy.
        if(Pointer<T>::Raw() == PointerToOwn)
          return *this;

        //The pointer will become null no matter what was passed in.
        Pointer<T>::Reference = 0;
        Pointer<T>::CachedPointer = 0;

        /*Any pointer must be deleted immediately to prevent a memory leak.
        There is no good reason for a real pointer to be passed in.*/
        delete PointerToOwn;

        return *this;
      }

      ///Destructor to decrement owner but not object reference count.
      ~WeakPointer()
      {
        //Unshare the existing pointer weakly.
        Pointer<T>::Unshare(true);
      }
    };
  }
}
#endif
