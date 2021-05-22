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

#ifndef BELLE_ENGRAVER_VALUE_H
#define BELLE_ENGRAVER_VALUE_H

namespace BELLE_NAMESPACE
{
  class ValueHelper
  {
    public:

    ///Converts a MICA concept to a value.
    static Value AsValue(mica::Concept c)
    {
      Value v;

      if(mica::undefined(c))
        return Value();
      else if(mica::number(c))
      {
        integer n = integer(mica::numerator(c));
        integer d = integer(mica::denominator(c));
        if(d == 1)
          v = n;
        else
          v = Ratio(n, d);
        return v;
      }

  #ifndef BELLE_DEBUG_STORE_MICA_UUIDV4_AS_STRING
      if(sizeof(integer) == 8)
      {
        /*If integer is 64-bits and string-debug mode is not on use the
        high part.*/
        v = integer(c.high);
      }
      else
  #endif
      {
        //Fallback to string if integer not supported or requested.
        v = mica::name(c);
      }

      return v;
    }
  };
}

namespace PRIM_NAMESPACE
{
  //Set assign to MICA for Value.
  template <> inline PRIM_NAMESPACE::Value& PRIM_NAMESPACE::Value::operator = (
    mica::Concept x)
  {
    return *this = BELLE_NAMESPACE::ValueHelper::AsValue(x);
  }

  //Set assign to MICA for Value.
  template <> inline PRIM_NAMESPACE::Value& PRIM_NAMESPACE::Value::operator = (
    mica::UUIDv4 x)
  {
    return *this = BELLE_NAMESPACE::ValueHelper::AsValue(mica::Concept(x));
  }
}
#endif
