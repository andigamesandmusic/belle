/*
  ==============================================================================

  Copyright 2007-2013, 2017 Andi
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
#define PRIM_WITH_TIMER
#include "prim.h"
using namespace prim;
int main()
{
  Random r;
  Timer t;

  for(;;)
  {
    t.Start();
    int64 NumberToCheck = r.Between(int64(19000000000000),
      int64(20000000000000));
    int64 SqrtNumberToCheck = int64(Sqrt(float64(NumberToCheck)));
    bool IsPrime = true;
    for(int64 i = 2; i < SqrtNumberToCheck; i++)
    {
      if(NumberToCheck % i == 0)
        IsPrime = false;
    }
    t.Pause();
    C::Out() >> NumberToCheck;
    if(IsPrime)
    {
      C::Out() << " is prime";
      break;
    }
    else
      C::Out() << " is not prime";
  }
  C::Out() >> "Elapsed time: " << t.Elapsed();
  return AutoRelease<Console>();
}
