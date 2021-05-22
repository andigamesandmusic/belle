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

#ifdef PRIM_WITH_JAVASCRIPT_V8

#ifndef PRIM_INCLUDE_MOD_JAVASCRIPT_V8_H
#define PRIM_INCLUDE_MOD_JAVASCRIPT_V8_H

#include <include/v8.h>
#include <include/libplatform/libplatform.h>

// V8 Resources:
// https://developers.google.com/v8/embed
// http://v8.googlecode.com/svn/trunk/samples/shell.cc
// http://thlorenz.github.io/v8-dox/build/v8-3.25.30/html

namespace PRIM_NAMESPACE
{
  class JavaScript
  {
    public:

    ///Callback implementation class for handling JavaScript functions in C++.
    class FunctionHandler
    {
      public:

      ///Implement to return functions names to be registered for callbacks.
      virtual Array<String> Functions()
      {
        return Array<String>();
      }

      ///Implement to handle functions registered with RegisterFunction().
      virtual Value Handle(const String& Function, const Value& Parameters)
      {
        C::Out() >> Function << " called with " << Parameters.n() <<
          " parameters:";
        C::Out() >> JSON::Export(Parameters);
        return Value();
      }

      virtual ~FunctionHandler();
    };

    private:

    FunctionHandler& Handler;
    v8::Isolate* V8Isolate;
    v8::Persistent<v8::Context>* V8Context;
    v8::Persistent<v8::ObjectTemplate>* V8Global;

    public:

    class InstanceManager
    {
      friend class JavaScript;

      Pointer<v8::Platform> V8Platform;
      Tree<v8::Isolate*, JavaScript*> IsolateOwnerTree;

      public:

      InstanceManager()
      {
        v8::V8::InitializeICU();
        V8Platform = v8::platform::CreateDefaultPlatform();
        v8::V8::InitializePlatform(V8Platform.Raw());
        v8::V8::Initialize();
      }

      ~InstanceManager()
      {
        v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
      }
    };

    public:

    ///Creates a JavaScript instance with a given function handler.
    JavaScript(FunctionHandler& Handler_) : Handler(Handler_), V8Isolate(0),
      V8Context(0), V8Global(0)
    {
      InstanceManager& Manager = Singleton<InstanceManager>().Instance();
      V8Isolate = v8::Isolate::New();
      Manager.IsolateOwnerTree[V8Isolate] = this;
      v8::Locker IsolateLocker(V8Isolate);
      v8::Isolate::Scope IsolateScope(V8Isolate);
      v8::HandleScope HandleScope(V8Isolate);

      V8Global = new v8::Persistent<v8::ObjectTemplate>(V8Isolate,
        v8::ObjectTemplate::New(V8Isolate));
      v8::Local<v8::ObjectTemplate> V8LocalGlobal =
        v8::Local<v8::ObjectTemplate>::New(V8Isolate, *V8Global);

      Array<String> FunctionNames = Handler.Functions();
      for(count i = 0; i < FunctionNames.n(); i++)
      {
        v8::Local<v8::String> FunctionNameValueString =
          v8::String::NewFromUtf8(V8Isolate, FunctionNames[i]);
        V8LocalGlobal->Set(
          v8::String::NewFromUtf8(V8Isolate, FunctionNames[i]),
          v8::FunctionTemplate::New(V8Isolate, CallbackEntryPoint,
            FunctionNameValueString));
      }
      V8Context = new v8::Persistent<v8::Context>(V8Isolate,
        v8::Context::New(V8Isolate, NULL, V8LocalGlobal));
    }

    ///Releases the JavaScript instance.
    ~JavaScript()
    {
      InstanceManager& Manager = Singleton<InstanceManager>().Instance();
      Manager.IsolateOwnerTree.Remove(V8Isolate);
      V8Context->Reset();
      V8Global->Reset();
      delete V8Context;
      delete V8Global;
    }

    ///Runs the garbage collector.
    void GarbageCollect()
    {
      //V8 runs the garbage collector during script execution automatically.
    }

    ///Executes the JavaScript code string and returns the result.
    Value Execute(const String& Code)
    {
      String UTF8ResultString;
      {
        v8::Locker IsolateLocker(V8Isolate);
        v8::Isolate::Scope IsolateScope(V8Isolate);
        {
          v8::HandleScope HandleScope(V8Isolate);
          v8::Local<v8::Context> V8LocalContext =
            v8::Local<v8::Context>::New(V8Isolate, *V8Context);
          {
            v8::Context::Scope ContextScope(V8LocalContext);
            v8::Local<v8::String> Source =
              v8::String::NewFromUtf8(V8Isolate, Code.Merge());
            v8::Local<v8::Script> Script = v8::Script::Compile(Source);
            v8::Local<v8::Value> Result = Script->Run();
            v8::String::Utf8Value UTF8Result(Result);
            UTF8ResultString = *UTF8Result;
          }
        }
      }
      Value ResultValue;
      JSON::ImportResult(UTF8ResultString, ResultValue);
      return ResultValue;
    }

    private:

    static void CallbackEntryPoint(
      const v8::FunctionCallbackInfo<v8::Value>& Args)
    {
      v8::Locker IsolateLocker(Args.GetIsolate());
      InstanceManager& Manager = Singleton<InstanceManager>().Instance();
      JavaScript& This = *Manager.IsolateOwnerTree[Args.GetIsolate()];
      v8::HandleScope HandleScope(Args.GetIsolate());
      Value Parameters;
      Parameters.NewArray();
      for(count i = 0; i < Args.Length(); i++)
      {
        v8::Local<v8::Value> Result = Args[i];
        v8::String::Utf8Value UTF8Result(Result);
        JSON::ImportResult(String(*UTF8Result), Parameters.Add());
      }

      String FunctionName;
      {
        v8::Local<v8::Value> Result = v8::Local<v8::Value>::Cast(Args.Data());
        v8::String::Utf8Value UTF8Result(Result);
        FunctionName = String(*UTF8Result);
      }
      Value Result = This.Handler.Handle(FunctionName, Parameters);

      String ResultString;
      JSON::Export(Result, ResultString, false, false);

      Args.GetReturnValue().Set(v8::JSON::Parse(v8::String::NewFromUtf8(
        Args.GetIsolate(), ResultString.Merge())));
    }
  };
#ifdef PRIM_COMPILE_INLINE
  JavaScript::FunctionHandler::~FunctionHandler() {}
#endif
}
#endif
#endif
