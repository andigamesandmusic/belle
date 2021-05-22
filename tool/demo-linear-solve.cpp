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

#define PRIM_WITH_TIMER
#define PRIM_COMPILE_INLINE
#include "prim.h"
using namespace prim;

template <class T> void LinearSolve()
{
  C::Out() >> "Random Linear Solver";
  C::Out() >> "--------------------";
  C::Out() >> "How many equations to generate: ";
  count S = 3;
  std::cin >> S;
  Matrix<T> M(S, S + 1);
  Random r;
  for(count i = 0; i < M.m(); i++)
    for(count j = 0; j < M.n(); j++)
      M(i, j) = T(r.Between(-1.0, 1.0));

  Timer t;
  t.Start();
  Array<T> Solution = M.LinearSolve();
  t.Stop();
  if(Solution.n() == 0)
  {
    C::Out() >> "The matrix could not be solved. This was unexpected.";
    return;
  }

  C::Out() >> "Matrix: ";
  C::Out() >> M;
  C::Out()++;
  C::Out() >> "Solution: ";
  C::Out() >> Solution;
  C::Out()++;
  C::Out() >> "Maximum error is: ";
  T MaxError = 0.0;
  for(count i = 0; i < M.m(); i++)
  {
    T Sum = -M(i, S);
    for(count j = 0; j < M.n() - 1; j++)
      Sum += M(i, j) * Solution[j];
    if(Sum > MaxError)
      MaxError = Sum;
  }
  C::Out() << MaxError * 1000000000000.0 << " * 1.0e-12";
  C::Out() >> "Time to Solve (us): " << t.Elapsed() * 1000000.0;
}

int main()
{
  LinearSolve<float64>();
  return AutoRelease<Console>();
}
