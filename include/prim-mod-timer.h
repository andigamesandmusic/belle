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

#ifndef PRIM_INCLUDE_MOD_TIMER_H
#define PRIM_INCLUDE_MOD_TIMER_H
#ifdef PRIM_WITH_TIMER //Depends on non-portable system headers.

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///High-precision timer capable of clocking to the microsecond.
  class Timer
  {
    ///Indicates whether the timer is currently running.
    bool Running; PRIM_PAD(bool)

#ifdef PRIM_ENVIRONMENT_WINDOWS
    ///Length of a tick on systems that use ticks
    float64 Period;
#endif

    ///Start time point in seconds
    float64 StartTime;

    ///End time point in seconds
    float64 EndTime;

    ///Seconds accumulated if multiple passes used
    float64 AccumulatedTime;

    public:

    ///Initializes the timer.
    Timer();

    ///Begins the timer.
    void Start();

    /**Stops the timer and reports the elapsed time in seconds. Any accumulation
    from pauses is cleared after being added to the elapsed time.*/
    number Stop();

    /**Pauses the timer until the timer is started again. Time is accumulated.
    Currently elapsed time is reported in seconds.*/
    number Pause();

    ///Reports the elapsed time in seconds without stopping the timer.
    number Elapsed();

    ///Returns the elapsed time and restarts the timer.
    number Lap() {number t = Stop(); Start(); return t;}

    ///Reports the elapsed time in seconds without stopping the timer.
    number s() {return Elapsed();}

    ///Reports the elapsed time in milliseconds without stopping the timer.
    number ms() {return Elapsed() * 1000.f;}

    ///Reports the elapsed time in microseconds without stopping the timer.
    number us() {return Elapsed() * 1000000.f;}
  };
}

#ifdef PRIM_COMPILE_INLINE

#ifdef PRIM_ENVIRONMENT_WINDOWS
#include <windows.h>
#else
#include <sys/time.h>
#endif

namespace PRIM_NAMESPACE
{
  Timer::Timer() : Running(false),
#ifdef PRIM_ENVIRONMENT_WINDOWS
  Period(0.0),
#endif
  StartTime(0.0), EndTime(0.0),
    AccumulatedTime(0.0)
  {
#ifdef PRIM_ENVIRONMENT_WINDOWS
    LARGE_INTEGER PerformanceFrequency;
    QueryPerformanceFrequency(&PerformanceFrequency);
    if(PerformanceFrequency.QuadPart)
      Period = 1.0 / float64(PerformanceFrequency.QuadPart);
#endif
  }

  void Timer::Start()
  {
    Running = true;
#ifdef PRIM_ENVIRONMENT_WINDOWS
    LARGE_INTEGER StartTicks;
    QueryPerformanceCounter(&StartTicks);
    StartTime = float64(StartTicks.QuadPart) * Period;
#else
    timeval StartTicks;
    gettimeofday(&StartTicks, 0);
    StartTime = float64(StartTicks.tv_sec) +
      float64(StartTicks.tv_usec) * 0.000001;
#endif
  }

  number Timer::Stop()
  {
    if(not Running)
      return Elapsed();
    Running = false;
#ifdef PRIM_ENVIRONMENT_WINDOWS
    LARGE_INTEGER EndTicks;
    QueryPerformanceCounter(&EndTicks);
    EndTime = float64(EndTicks.QuadPart) * Period;
#else
    timeval EndTicks;
    gettimeofday(&EndTicks, 0);
    EndTime = float64(EndTicks.tv_sec) + float64(EndTicks.tv_usec) * 0.000001;
#endif
    EndTime += AccumulatedTime;
    AccumulatedTime = 0.0;
    return number(EndTime - StartTime);
  }

  number Timer::Pause()
  {
    if(not Running)
      return Elapsed();
    Running = false;
#ifdef PRIM_ENVIRONMENT_WINDOWS
    LARGE_INTEGER EndTicks;
    QueryPerformanceCounter(&EndTicks);
    EndTime = float64(EndTicks.QuadPart) * Period;
#else
    timeval EndTicks;
    gettimeofday(&EndTicks, 0);
    EndTime = float64(EndTicks.tv_sec) + float64(EndTicks.tv_usec) * 0.000001;
#endif
    AccumulatedTime += EndTime - StartTime;
    EndTime = StartTime;
    return Elapsed();
  }

  number Timer::Elapsed()
  {
    if(Running)
    {
#ifdef PRIM_ENVIRONMENT_WINDOWS
      LARGE_INTEGER EndTicks;
      QueryPerformanceCounter(&EndTicks);
      EndTime = float64(EndTicks.QuadPart) * Period;
#else
      timeval EndTicks;
      gettimeofday(&EndTicks, 0);
      EndTime = float64(EndTicks.tv_sec) + float64(EndTicks.tv_usec) * 0.000001;
#endif
    }
    return number(EndTime + AccumulatedTime - StartTime);
  }
}
#endif
#endif
#endif
