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
#include "prim.h"
using namespace prim;

struct Base
{
  integer Data;

  Base() : Data(10) {}

  virtual void DoSomething() {Data -= 10;}

  virtual ~Base();
};
Base::~Base() {}

struct Derived : public Base
{
  virtual void DoSomething() {Data += 10;}
  virtual ~Derived();
};
Derived::~Derived() {}

struct UnrelatedVirtual
{
  virtual ~UnrelatedVirtual();
};
UnrelatedVirtual::~UnrelatedVirtual() {}

struct NonVirtualBase
{
  integer Data;
};

struct NonVirtualDerived : public NonVirtualBase
{
  integer OtherData;
};

int main()
{
  //-------//
  //General//
  //-------//
  C::Out() >> "General";
  C::Out() >> "-------";

  //Create a weak pointer.
  C::Out() >> "Creating weak pointer P";
  Pointer<integer>::Weak P;

  //Pointer should read as null.
  C::Out() >> "P: " << P;

  //Note the following is a screw case as weak pointers can not own new objects.
  C::Out() >> "Assigning P to a new integer (do not do this)";
  P = new integer; //Do not do this, will be deleted immediately.

  //Pointer should still read as null.
  C::Out() >> "P: " << P;

  //Create an artificial scope for testing.
  {
    //Create an owned integer and set it to 10.
    C::Out() >> "Creating owning pointer A and assigning integer to 10.";
    Pointer<integer> A(new integer);
    *A = 10;

    //Integer pointer is non-null and object is set to 10.
    C::Out() >> "A: " << A << ": " << *A;

    /*Assign the pointer to a new (different) integer. The old integer is
    deleted automatically.*/
    C::Out() >> "Assigned A to new integer and assigning integer to 20.";
    A = new integer;
    *A = 20;

    //Integer pointer is a different non-null pointer and object is set to 20.
    C::Out() >> "A: " << A << ": " << *A;

    //Set weak pointer to track the active pointer.
    C::Out() >> "Setting P to track A.";
    P = A;

    //Integer pointer is identical to A as printed above.
    C::Out() >> "P: " << P << ": " << *P;
  }
  C::Out() >> "A is going out of scope. Deleting owned integer.";
  /*A goes out of scope. Since P is weak, it does not share ownership of the
  object, and only shares ownership of owner state. Therefore, asking P for its
  pointer simply returns null.*/
  C::Out() >> "P: " << P;

  //Pointers automatically cast to bool for easy checking.
  if(P)
    C::Out() >> "Error: P should be null.";

  //---------//
  //Constness//
  //---------//
  C::Out()++;
  C::Out() >> "Constness";
  C::Out() >> "---------";

  /*Constness comes in two dimensions when working with pointers: constness of
  the pointer and constness of the object pointed to. To read the constness,
  simply read right-to-left:
  int*                        -> Pointer to an int
  int* const                  -> Const pointer to an int
  const int * OR int const *  -> Pointer to an int const/const int
  const int * const           -> Const point to a const int

  When working with the Pointer class these are equivalent to:
  Pointer<int>
  const Pointer<int>
  Pointer<const int>
  const Pointer<const int>

  Most methods on the Pointer class are marked const because they do not affect
  the underlying logical pointer. It should be noted however that a non-const
  Pointer<T> can still side-effect a const Pointer<T>::Weak, for example if the
  reference count hits zero, the weak pointer will be set to null.*/

  Pointer<int> PointerToInt = new int;
  *PointerToInt = 1;

  const Pointer<int> ConstPointerToInt = new int;
  *ConstPointerToInt = 2;

  Pointer<const int> PointerToConstInt = PointerToInt;

  C::Out() >> *PointerToInt;
  C::Out() >> *ConstPointerToInt;

  *PointerToInt = 3;
  C::Out() >> *PointerToConstInt;

  const Pointer<const int> ConstPointerToConstInt = PointerToInt;
  *PointerToInt = 4;
  C::Out() >> *ConstPointerToConstInt;

#if 0 //These will trigger compiler errors due to invalid const conversion.
  //Can not reassign const pointer.
  ConstPointerToConstInt = PointerToInt;

  //Can not mutate const object.
  *PointerToConstInt = 3;

  //Can not remove object constness.
  PointerToInt = PointerToConstInt;

  //Can not remove object constness.
  PointerToInt = ConstPointerToConstInt;
#endif

  //----------------------------//
  //Subclassing and Polymorphism//
  //----------------------------//
  C::Out()++;
  C::Out() >> "Subclassing and Polymorphism";
  C::Out() >> "----------------------------";

  //Example of derived auto-casting to base.
  Pointer<Base> B = new Derived;
  B->DoSomething();
  C::Out() >> "B: " << B << ": " << B->Data;

  //Dynamic casting is automatically done so you can do this:
  if(Pointer<Derived> D = B)
  {
    C::Out() >> "Derived D from B.";
    C::Out() >> "D: " << D << ": " << D->Data;
  }

  //Make a new derived item and assign it to a weak base pointer.
  Pointer<Base>::Weak BW;
  {
    C::Out() >> "Creating derived D";
    Pointer<Derived> D = new Derived;
    D->DoSomething();
    C::Out() >> "D: " << D << ": " << D->Data;

    //First convert to base pointer, then assign to weak form.
    BW = Pointer<Base>(D);

    //The As() method could also be used.
    BW = D.As<Base>();

    //The pointer may be weak, but that doesn't mean it can't do something!
    BW->DoSomething();
    C::Out() >> "BW: " << BW << ": " << BW->Data;
  }
  //Derived D is now out of scope. Base weak pointer can still be inspected.
  C::Out() >> "Derived D going out of scope.";
  C::Out() >> "BW: " << BW;

  /*Trying to convert to an unrelated (but virtual type) will succeed in
  that it will simply set the pointer to null.*/
  Pointer<UnrelatedVirtual> U = B;
  C::Out() >> "U: " << U;

  /*The Pointer class will not allow you to cast between non-virtual types.
  Shared pointer ownership is only technically feasible among virtual types.
  The reason for this is that if you have struct A {}; and
  struct B : public A {}; then a Pointer to B given to a Pointer to an A will
  be unable to correctly delete B (because it is not virtual, i.e. has no
  virtual destructor). You can still create Pointers to non-virtual types,
  just be aware that you will not be able to cast down to the base class except
  through getting the raw pointer via Raw().

  The Pointer class attempts to enforce this at compile-time. See below.*/

  //Fine as long as you only pass it around to pointers of the same type.
  Pointer<NonVirtualDerived> NVD = new NonVirtualDerived;

#if 0 //Blocked out since these will create compile-time or run-time errors.
  /*Compiler error: not virtual, will not do conversion since this would
  fundamentally be a Bad Idea.*/
  NVD = Pointer<NonVirtualBase>();

  /*Compiler error: not virtual, will not do conversion since this would
  fundamentally be a Bad Idea. You might be saying, but I thought a derived
  class could always be converted to a base class! Yes, for unmanaged pointers,
  not so for managed pointers.*/
  Pointer<NonVirtualBase> NVB = NVD;

  /*There are some issues that can not be detected at compile-time, but which
  must be avoided. The pointer here is implicitly cast to the base type on
  assignment and so the Pointer object is not even aware that this came from a
  derived object. When the Pointer frees the base class, Bad Things Will Happen.
  The worst part is that depending on the client code, this usage may produce
  latent (rather than immediate) issues. Never assign ownership of a
  non-virtual derived class pointer to a base class pointer! It's like
  giving a mindless robot the keys to your car, and not opening the garage
  door.*/
  Pointer<NonVirtualBase> DoomsdayDevice = new NonVirtualDerived;

  //At least this usage would give you a compile-time error:
  Pointer<NonVirtualBase> SlapOnWrist =
    Pointer<NonVirtualDerived>(new NonVirtualDerived);
#endif

  /*Note that the above caution only applies to non-virtual types.
  Virtual types will interchange just fine, even on immediate assignment.*/
  Pointer<Base> JustFine = new Derived;

  return AutoRelease<Console>();
}
