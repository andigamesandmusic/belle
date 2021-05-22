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

#define PRIM_WITH_SHELL
#define PRIM_COMPILE_INLINE
#include "prim.h"

using namespace prim;

class CrashyProcess : public Job
{
  public:
  count Run()
  {
    C::Out() >> "Running crashy process";
    Crash();
    JobManager::Sleep(1.f);
    return AutoRelease<Console>();
  }
  ~CrashyProcess();
};
CrashyProcess::~CrashyProcess() {}

class StableProcess : public Job
{
  public:
  count Run()
  {
    C::Out() >> "Running stable process";
    JobManager::Sleep(0.75f);
    Value v;
    v["hello"] = "world";
    SetSharedData(v);
    return AutoRelease<Console>();
  }
  ~StableProcess();
};
StableProcess::~StableProcess() {}

class ErrorProcess : public Job
{
  public:
  count Run()
  {
    C::Out() >> "Running error process";
    JobManager::Sleep(0.25f);
    return AutoRelease<Console>(3);
  }
  ~ErrorProcess();
};
ErrorProcess::~ErrorProcess() {}

class FastProcess : public Job
{
  public:
  count Run()
  {
    C::Out() >> "Running fast process";
    return AutoRelease<Console>();
  }
  ~FastProcess();
};
FastProcess::~FastProcess() {}

class SlowProcess : public Job
{
  public:
  count Run()
  {
    C::Out() >> "Running slow process";
    JobManager::Sleep(5.f);
    return AutoRelease<Console>();
  }
  ~SlowProcess();
};
SlowProcess::~SlowProcess() {}

class HungProcess : public Job
{
  public:
  count Run()
  {
    C::Out() >> "Running hung process";
    String s;
    for(;not s.n();) {}
    return AutoRelease<Console>();
  }
  ~HungProcess();
};
HungProcess::~HungProcess() {}

int main(void)
{
  JobManager J;
  for(count i = 0; i < 5; i++)
  {
    J.Add(new CrashyProcess, 10.f);
    J.Add(new StableProcess, 10.f);
    J.Add(new ErrorProcess, 10.f);
    J.Add(new FastProcess, 10.f);
    J.Add(new SlowProcess, 10.f);
    J.Add(new HungProcess, 10.f);
  }
  J.Process();
  C::Out() >> J.Summarize();
  return AutoRelease<Console>();
}
