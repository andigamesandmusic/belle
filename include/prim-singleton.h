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

#ifndef PRIM_INCLUDE_SINGLETON_H
#define PRIM_INCLUDE_SINGLETON_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  /**Singleton with lazy-instantiation and manual release. The class handles
  multiple asymmetric calls to Instance() and Release(). The advantage of the
  manual release is that it does not depend on exit-time destructors. See
  AutoRelease for an easier way of releasing singletons.*/
  template<class T>
  class Singleton
  {
    ///Binds the global instance pointer to this method.
    static T* Local(bool Instantiating)
    {
      static T* LazyInstance = 0;
      if(Instantiating and not LazyInstance)
        LazyInstance = new T;
      else if(not Instantiating and LazyInstance)
        delete LazyInstance, LazyInstance = 0;
      return LazyInstance;
    }

    public:

    /**Gets a reference to the instance, creating it if necessary. Note the
    singleton is not initialized until the object constructor is finished, so
    do not call this method or submethod in the constructor of the object.*/
    static T& Instance() {return *Local(true);}

    /**Releases and calls the destructor for the current instance. If Instance()
    has not been called since the last release, then this method does nothing.*/
    static void Release() {Local(false);}
  };

  /**An auto-release singletons destructor. To use, just create one of these
  objects in your main() with the list of singletons as template arguments to be
  released. To avoid a memory leak, there should be no calls to the
  Singleton<T>::Instance() method after this object goes out of scope. An
  alternative syntax you can use is constructor with int return value, which
  will be casted using operator int if used in a return statement.*/
  template<class A, class B = A, class C = A, class D = A, class E = A,
       class F = A, class G = A, class H = A, class I = A, class J = A>
  class AutoRelease
  {
    /**Releases each of the singletons in the template list. The ones that are
    not specified are equivalent to the first specified class. Since Release()
    can be safely called more than once, the multiple releases collapse into
    one.*/
    void Release() const
    {
      Singleton<A>::Release();
      Singleton<B>::Release();
      Singleton<C>::Release();
      Singleton<D>::Release();
      Singleton<E>::Release();
      Singleton<F>::Release();
      Singleton<G>::Release();
      Singleton<H>::Release();
      Singleton<I>::Release();
      Singleton<J>::Release();
    }

    ///Stores the return value for the operator int() method.
    int ReturnValue;

    public:

    ///Constructs an autorelease with a zero (success) return value.
    AutoRelease() : ReturnValue(0) {}

    ///Specifies the return value to be used for the operator int() method.
    AutoRelease(int CustomReturnValue) : ReturnValue(CustomReturnValue) {}

    ///Releases the singletons.
    ~AutoRelease() {Release();}

    ///Releases the singletons and returns value indicated by the constructor.
    operator int () const {Release(); return ReturnValue;}
  };
}
#endif
