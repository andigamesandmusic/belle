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
#define PRIM_WITH_TIMER
#define PRIM_WITH_THREAD
#include "prim.h"

using namespace prim;

static void BusyWait(number SecondsToWait)
{
  Timer t;
  t.Start();
  while(t.Elapsed() < SecondsToWait) {}
}

static void Message(Mutex& m, const String& MessageToPrint)
{
  Lock L(m);
  C::Out() >> MessageToPrint;
}

class Producer : public Thread
{
  Event& e;
  Mutex& m;
  public:

  Producer(Event& e_, Mutex& m_) : e(e_), m(m_) {}

  virtual ~Producer();

  void Run()
  {
    BusyWait(2.f);
    Message(m, "Producer signaling...");
    e.Signal();
    BusyWait(12.f);
    Message(m, "Producer finished.");
  }
};

class Consumer : public Thread
{
  Event& e;
  Mutex& m;
  count Index;

  public:

  Consumer(Event& e_, Mutex& m_, count Index_) : e(e_), m(m_),
    Index(Index_) {}

  virtual ~Consumer();

  void Run()
  {
    BusyWait(number(Index) * 0.1f);
    Message(m, String(Index) + " Consumer waiting...");
    e.Wait();
    BusyWait(number(Index) * 0.1f);
    Message(m, String(Index) + " Consumer consuming...");
    BusyWait(number(Index));
    Message(m, String(Index) + " Consumer finished.");
  }
};

Producer::~Producer() {}
Consumer::~Consumer() {}

int main()
{
  Event e;
  Mutex m;
  const count ConsumerCount = 10;
  Message(m, "Initializing...");
  Array<Pointer<Producer> > Producers;
  Array<Pointer<Consumer> > Consumers;
  Producers.Add() = new Producer(e, m);
  for(count i = 0; i < ConsumerCount; i++)
    Consumers.Add() = new Consumer(e, m, i);

  Message(m, "Waiting one second before starting...");
  BusyWait(1.f);
  Producers.z()->Begin();
  for(count i = 0; i < ConsumerCount; i++)
    Consumers[i]->Begin();
  Producers.z()->WaitToEnd();
  for(count i = 0; i < ConsumerCount; i++)
    Consumers[i]->WaitToEnd();
  Message(m, "Finished.");
  return AutoRelease<Console>();
}
