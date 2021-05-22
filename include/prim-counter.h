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

#ifndef PRIM_INCLUDE_COUNTER_H
#define PRIM_INCLUDE_COUNTER_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///Iterator helper for while-loops and for-loops.
  class Counter
  {
    count c;

    public:

    Counter() : c(Limits<count>::Zero()) {}
    template <class Container> Counter(const Container& Object) :
      c(Object.n() - Limits<count>::One()) {}

    operator count() const {return c;}

    count operator ++ () {return ++c;}
    count operator ++ (int Postfix) {(void)Postfix; return c++;}
    count operator -- () {return --c;}
    count operator -- (int Postfix) {(void)Postfix; return c--;}

    count Next() {return ++c;}

    count Previous() {return --c;}

    bool aa() const
    {
      return c >= -Limits<count>::One();
    }

    bool a() const
    {
      return c >= Limits<count>::Zero();
    }

    bool b() const
    {
      return c > Limits<count>::Zero();
    }

    template <class Container> bool y(const Container& Object) const
    {
      return c < Object.n() - Limits<count>::One();
    }

    template <class Container> bool z(const Container& Object) const
    {
      return c < Object.n();
    }

    template <class Container> bool zz(
      const Container& Object) const
    {
      return c <= Object.n();
    }
  };
}
#endif
