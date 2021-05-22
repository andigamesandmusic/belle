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

#if defined(PRIM_WITH_JAVASCRIPT_V8) || defined(PRIM_WITH_JAVASCRIPT_JSC)
#ifndef PRIM_INCLUDE_MOD_JAVASCRIPT_H
#define PRIM_INCLUDE_MOD_JAVASCRIPT_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

/*JavaScript engine wrappers. Needs to be linked with the WebKit framework
on Mac/iOS for JavaScriptCore or V8 if using that library. Example usage:*/

#if 0
#ifndef PRIM_WITH_JAVASCRIPT_V8
#define PRIM_WITH_JAVASCRIPT_JSC
#define PRIM_COMPILE_INLINE_MM
#endif

#define PRIM_WITH_THREAD
#define PRIM_COMPILE_INLINE
#include "../Source/prim.h"

using namespace prim;

class CustomHandler : public JavaScript::FunctionHandler
{
  public:

  Value MeaningOfLife() {return Value(42);}

  Value Sum(const Value& Parameters)
  {
    Value v = Parameters.a();
    for(count i = 1; i < Parameters.n(); i++)
      v = +v + Parameters[i].AsNumber();
    return v;
  }

  Array<String> Functions()
  {
    Array<String> FunctionNames;
    FunctionNames.Add() = "meaning_of_life";
    FunctionNames.Add() = "sum";
    return FunctionNames;
  }

  Value Handle(const String& Function, const Value& Params)
  {
    if(Function == "meaning_of_life")
      return MeaningOfLife();
    else if(Function == "sum")
      return Sum(Params);
    else
      return Value();
  }
  virtual ~CustomHandler();
};

CustomHandler::~CustomHandler() {}

int main()
{
  {
    CustomHandler Handler;
    JavaScriptThread j(Handler);
    C::Out() >> j.ExecuteSynchronously("a = 0");
    for(count i = 0; i < 10; i++)
      j.ExecuteAsynchronously("a = a + 1");
    Thread::Sleep(0.1);
    C::Out() >> j.ExecuteSynchronously("a");
    j.ExecuteAsynchronously("a = meaning_of_life()");
    Thread::Sleep(0.1);
    C::Out() >> j.ExecuteSynchronously("a");
    Thread::Sleep(0.1);
    C::Out() >> j.ExecuteSynchronously("a = sum(1,2,3,4,5,6,7,8,9,10)");
  }
  return AutoRelease<Console>();
}
#endif

//JavaScriptCore - for iOS, Mac OS X
#include "prim-mod-javascript-jsc.h"

//V8 - for Android, Mac OS X
#include "prim-mod-javascript-v8.h"

namespace PRIM_NAMESPACE
{
#ifdef PRIM_WITH_QUEUE
  class JavaScriptThread : public SerialQueue<String, Value>
  {
    JavaScript JavaScriptContext;

    virtual Value Process(const String& Task)
    {
      return JavaScriptContext.Execute(Task);
    }

    public:

    JavaScriptThread(JavaScript::FunctionHandler& Handler) :
      JavaScriptContext(Handler) {Start();}

    virtual ~JavaScriptThread();

    void GarbageCollect()
    {
      JavaScriptContext.GarbageCollect();
    }
  };

#ifdef PRIM_COMPILE_INLINE
  JavaScriptThread::~JavaScriptThread() {Stop(true);}
#endif
#endif
}
#endif
#endif
