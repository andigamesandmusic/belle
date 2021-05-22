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
#define PRIM_WITH_THREAD
#include "prim.h"

using namespace prim;

//Global mutex and data.
extern Mutex* GlobalMutex; Mutex* GlobalMutex = 0;
const count GlobalDataSize = 1000;
extern int GlobalData[GlobalDataSize]; int GlobalData[GlobalDataSize];

class Job : public Thread
{
  count JobIndex, Subtasks;

  public:

  Job(count JobIndex_, count Subtasks_) : JobIndex(JobIndex_),
    Subtasks(Subtasks_) {}

  virtual ~Job();

  static void SlowDown()
  {
    Random r;
    count k = r.Between(0, 100);
    for(count j = 0; j < k; j++)
    {
      //Do something that will not get optimized away by the compiler.
      String s;
    }
  }

  void Run()
  {
    for(int i = 1; i <= Subtasks; i++)
    {
      //Write to a shared resource.
      {
        //Comment out Lock to step GlobalData's toes.
        Lock Lock(*GlobalMutex); // <-- TEST HERE
        for(int j = 0; j < GlobalDataSize; j++)
          GlobalData[j] = int(JobIndex + Subtasks + count(j));

        //Spin some cycles.
        SlowDown();

        for(int j = 0; j < GlobalDataSize; j++)
        {
          if(GlobalData[j] != JobIndex + Subtasks + j)
          {
            C::Out() >> "Someone is stepping on my toes! (Check line 67)";
            break;
          }
        }
      }

      //Test the end-state signal. Uncomment to let job be interrupted.
      //if(IsEnding()) break; // <-- TEST HERE

      //Report work done by thread.
      {
        String s;
        s << "Job " << JobIndex << " (" << i << " of " << Subtasks << ")";

#if 1   // <-- TEST HERE
        //Ensure that a full line is appended.
        C::Out() >> s;
#else
        //Trippy interesecting appends.
        C::Out() >> "Job " << JobIndex <<
          " (" << i << " of " << Subtasks << ")";
#endif
      }
    }
  }
};

Job::~Job() {}

int main()
{
  //Initialize global mutex.
  GlobalMutex = new Mutex;

  //Set job parameters
  const count JobsToRun = 5;
  const count Subtasks = 5;
  Job* Jobs[JobsToRun];

  //Create a bunch of jobs.
  C::Out() >> "Running jobs";
  for(count i = 0; i < JobsToRun; i++)
  {
    Jobs[i] = new Job(i, Subtasks);
    Jobs[i]->Begin();
  }

  //Ask each thread to exit as soon as possible, then clean-up.
  for(count i = 0; i < JobsToRun; i++)
  {
    Jobs[i]->WaitToEnd();
    delete Jobs[i];
  }

  //Delete global mutex.
  delete GlobalMutex;

  return AutoRelease<Console>();
}
