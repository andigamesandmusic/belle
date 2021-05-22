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

#ifdef PRIM_WITH_JAVASCRIPT_JSC

#ifndef PRIM_INCLUDE_MOD_JAVASCRIPT_JSC_H
#define PRIM_INCLUDE_MOD_JAVASCRIPT_JSC_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///JavaScript instance.
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
        C::Out() >> Function << " called with ";
        C::Out() << Parameters.n() << " parameters:";
        C::Out() >> JSON::Export(Parameters);
        return Value();
      }

      ///Implement to handle exceptions.
      virtual void HandleException(const String& Code,
        const String& Information)
      {
        C::Out()++;
        C::Red();
        C::Error() >> "JavaScript failed to execute:" >> Code;
        C::Error() >> "Exception occured:" >> Information;
        C::Reset();
        C::Out()++;
      }

      virtual ~FunctionHandler();
    };

    private:

    /* Some members listed as void* to avoid including headers. The following
    static method forcibly casts back to original assuming:

    Context                = JSGlobalContextRef
    ContextGroup           = JSContextGroupRef
    FunctionTable Tree key = JSObjectRef
    */
    template <class T> static T P_(void* x)
    {
      return reinterpret_cast<T>(x);
    }

    template <class T> static T PC_(const void* x)
    {
      return reinterpret_cast<T>(x);
    }

    //See "JavaScriptCore Framework Reference" for API details.
    void*               Context;
    const void*         ContextGroup;
    Tree<void*, String> FunctionTable;
    FunctionHandler&    Handler;

    void Cleanup();

    void Initialize();

    bool Initialized();

    String InstanceToUUIDv4String() const;

    void RegisterFunction(String FunctionName);

    void StoreInstanceObject();

    public:

    ///Internal function -- do not use.
    String InternalLookupFunction(void* x);

    public:

    ///Creates a JavaScript instance with a given function handler.
    JavaScript(FunctionHandler& Handler_) :
      Context(0), ContextGroup(0), Handler(Handler_) {Initialize();}

    ///Releases the JavaScript instance.
    ~JavaScript() {Cleanup();}

    ///Do not use copy constructor.
    JavaScript(const JavaScript& Other);

    ///Do not use assignment operator.
    JavaScript& operator = (const JavaScript& Other);

    ///Calls a JavaScript function registered to the handler.
    Value Call(const String& Function, const Value& Parameters)
    {
      return Handler.Handle(Function, Parameters);
    }

    ///Executes the JavaScript code string and returns the result.
    Value Execute(const String& Code);

    ///Runs the garbage collector.
    void GarbageCollect();

    ///Forwards an exception to the exception handler.
    void ForwardException(const String& Code, const String& Information)
    {
      Handler.HandleException(Code, Information);
    }
  };
}
#endif

#ifdef PRIM_COMPILE_INLINE_MM

#ifndef PRIM_ENVIRONMENT_APPLE
#error JavaScriptCore engine requires JavaScriptCore Framework
#endif

#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>
/*The Objective-C headers define bool to bool, which trips the recursive macro
expansion warning.*/
#undef bool

namespace PRIM_NAMESPACE
{
  JavaScript::FunctionHandler::~FunctionHandler() {}

  static String AssumeInitializedGetJSValueAsString(
    JSContextRef ExecutionContext, JSValueRef v)
  {
    String Result;
    if(JSStringRef S = JSValueCreateJSONString(ExecutionContext, v, 0, NULL))
    {
      Array<ascii> c(count(JSStringGetMaximumUTF8CStringSize(S)));
      c.Zero();
      JSStringGetUTF8CString(S, &c.a(), size_t(c.n()));
      Result = &c.a();
      if(not Result.IsUTF8())
        Result = "";
      JSStringRelease(S);
    }
    return Result;
  }

  static JavaScript* InstanceFromUUIDv4String(String x)
  {
    return reinterpret_cast<JavaScript*>(
      reinterpret_cast<void*>(UUIDv4(x).Low()));
  }

  static String InstanceVariableName()
  {
    return "_prim_javascript_instance";
  }

  static Value ExecuteIn(JSContextRef ExecutionContext, const String& Code,
    JavaScript* JavaScriptContext)
  {
    Value ImportedResult;
    if(JSStringRef CodeString = JSStringCreateWithUTF8CString(Code.Merge()))
    {
      JSValueRef ExecutionException;
      JSValueRef Result = JSEvaluateScript(ExecutionContext, CodeString, NULL,
        NULL, 1, &ExecutionException);
      if(Result)
        JSON::ImportResult(
          AssumeInitializedGetJSValueAsString(ExecutionContext, Result),
          ImportedResult);
      else if(JavaScriptContext)
        JavaScriptContext->ForwardException(Code,
          AssumeInitializedGetJSValueAsString(
          ExecutionContext, ExecutionException));
      JSStringRelease(CodeString);
    }
    return ImportedResult;
  }

  static JavaScript* GetInstance(JSContextRef ExecutionContext)
  {
    return InstanceFromUUIDv4String(ExecuteIn(ExecutionContext,
      InstanceVariableName(), 0));
  }

  static JSValueRef CallbackEntryPoint(JSContextRef ExecutionContext,
    JSObjectRef Function, JSObjectRef ThisObject, size_t ArgumentCount,
    const JSValueRef Arguments[], JSValueRef* Exception)
  {
    (void)ThisObject;
    (void)Exception;
    JSValueRef ResultValue = 0;
    if(JavaScript* JavaScriptContext = GetInstance(ExecutionContext))
    {
      Value Parameters;
      Parameters.NewArray();
      String ParameterException;
      for(count i = 0; i < count(ArgumentCount); i++)
      {
        Value Parameter;
        JSON::ImportResult(
          AssumeInitializedGetJSValueAsString(ExecutionContext, Arguments[i]),
          Parameter);
        Parameters.Add() = Parameter;
        if(Parameter.IsNil())
          ParameterException >> "Parameter " << (i + 1) << " of " <<
            count(ArgumentCount) << " could not be parsed";
      }
      if(ParameterException)
        Parameters = ParameterException;

      String HandlerResultJSON;
      @autoreleasepool {
        JSON::Export(JavaScriptContext->Call(
          JavaScriptContext->InternalLookupFunction(Function), Parameters),
          HandlerResultJSON, false, false);
      }

      if(JSStringRef ResultJSONString = JSStringCreateWithUTF8CString(
        HandlerResultJSON))
      {
        ResultValue = JSValueMakeFromJSONString(ExecutionContext,
          ResultJSONString);
        JSStringRelease(ResultJSONString);
      }
    }
    return ResultValue;
  }

  void JavaScript::Cleanup()
  {
    if(Context)
      JSGlobalContextRelease(P_<JSGlobalContextRef>(Context)),
      Context = 0;
    if(ContextGroup)
      JSContextGroupRelease(PC_<JSContextGroupRef>(ContextGroup)),
      ContextGroup = 0;
  }

  void JavaScript::Initialize()
  {
    if((ContextGroup = JSContextGroupCreate()))
      Context = JSGlobalContextCreateInGroup(
        PC_<JSContextGroupRef>(ContextGroup), NULL);
    StoreInstanceObject();

    Array<String> FunctionNames = Handler.Functions();
    for(count i = 0; i < FunctionNames.n(); i++)
      RegisterFunction(FunctionNames[i]);
  }

  bool JavaScript::Initialized() {return ContextGroup and Context;}

  String JavaScript::InstanceToUUIDv4String() const
  {
    PRIM_NAMESPACE::uint64 x = reinterpret_cast<PRIM_NAMESPACE::uint64>(
      reinterpret_cast<const void*>(this));
    return UUIDv4(0, x);
  }

  void JavaScript::RegisterFunction(String FunctionName)
  {
    if(Initialized())
    {
      if(JSStringRef FunctionNameString =
        JSStringCreateWithUTF8CString(FunctionName.Merge()))
      {
        if(JSObjectRef CreatedFunction = JSObjectMakeFunctionWithCallback(
          P_<JSGlobalContextRef>(Context), FunctionNameString,
          &CallbackEntryPoint))
        {
          JSObjectSetProperty(P_<JSGlobalContextRef>(Context),
            JSContextGetGlobalObject(P_<JSGlobalContextRef>(Context)),
            FunctionNameString, CreatedFunction, kJSPropertyAttributeReadOnly,
            NULL);

          FunctionTable[CreatedFunction] = FunctionName;
        }
        JSStringRelease(FunctionNameString);
      }
    }
  }

  void JavaScript::StoreInstanceObject()
  {
    Execute(InstanceVariableName() + "='" + InstanceToUUIDv4String() + "';\n");
  }

  String JavaScript::InternalLookupFunction(void* x)
  {
    return FunctionTable[x];
  }

  Value JavaScript::Execute(const String& Code)
  {
    return Initialized() ?
      ExecuteIn(P_<JSGlobalContextRef>(Context), Code, this) : Value();
  }

  void JavaScript::GarbageCollect()
  {
    if(Initialized()) JSGarbageCollect(P_<JSGlobalContextRef>(Context));
  }
}
#endif
#endif
