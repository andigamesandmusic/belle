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

#ifndef PRIM_INCLUDE_ARGUMENTS_H
#define PRIM_INCLUDE_ARGUMENTS_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///Helpers to process command-line arguments and display usage.
  class Arguments
  {
    public:

    enum AssignableFlag
    {
      Flag,
      Property,
      PropertyList,
      OutputFile,
      InputFile
    };

    enum ConstraintFlag
    {
      Optional,
      Required
    };

    static void AddParameter(Value& v, const String& Name,
      AssignableFlag Assignable, ConstraintFlag Constraint,
      const String& Description, String Group = "")
    {
      if(Assignable == Property)
        v["Parameters"][Group][Name]["Assignable"] = "value";
      else if(Assignable == PropertyList)
        v["Parameters"][Group][Name]["Assignable"] = "value1,value2,...";
      else if(Assignable == OutputFile)
        v["Parameters"][Group][Name]["Assignable"] = "out-file";
      else if(Assignable == InputFile)
        v["Parameters"][Group][Name]["Assignable"] = "in-file";

      v["Parameters"][Group][Name]["Description"] = Description;
      v["Parameters"][Group][Name]["Required"]    = Constraint == Required;
    }

    static Value NewSchema(String ProcessName, String Summary,
      String Description, String TrailingArgsDescription)
    {
      Value v;
      v["Process"] = ProcessName;
      v["Trailing"] = TrailingArgsDescription;
      v["Summary"] = Summary;
      v["Description"] = Description;
      v["Parameters"].NewArray();
      return v;
    }

    static Value Process(Array<String> ArgumentArray, const Value& Schema)
    {
      List<String> ArgumentList;
      for(count i = 0; i < ArgumentArray.n(); i++)
        ArgumentList.Add() = ArgumentArray[i];
      return Process(ArgumentList, Schema);
    }

    static Value Process(int ArgumentCount, const char** ArgumentArray,
      const Value& Schema)
    {
      List<String> ArgumentList;
      for(count i = 0; i < count(ArgumentCount); i++)
        ArgumentList.Add() = ArgumentArray[i];
      return Process(ArgumentList, Schema);
    }

    static Value Process(List<String> ArgumentList, const Value& Schema)
    {
      Value v;
      bool StartedTrailing = false;
      for(count i = 1; i < ArgumentList.n(); i++)
      {
        String Arg = ArgumentList[i];
        Value Param = LookupParameter(GetParameterName(Arg), Schema);
        if(Param.IsNil())
        {
          if(IsParameter(Arg) and GetParameterName(Arg) == "help")
          {
            Usage(Schema);
            return Value();
          }
          else if(IsParameter(Arg))
            return Raise(Schema, Arg + " is not a valid argument.");
          else if(Schema["Trailing"].AsString())
            v["trailing"].Add() = Arg, StartedTrailing = true;
          else
            return Raise(Schema, "Trailing arguments are not allowed.");
        }
        else
        {
          if(StartedTrailing)
            return Raise(Schema,
              "Options may not appear after trailing arguments.");
          if(Param["Assignable"].IsString())
          {
            if(IsAssignment(Arg))
            {
              if(GetParameterValue(Arg))
              {
                if(Param["Assignable"].AsString().StartsWith("value1"))
                  v[GetParameterName(Arg)] = ParsePropertyList(
                    GetParameterValue(ArgumentList[i]));
                else
                  v[GetParameterName(Arg)] = GetParameterValue(Arg);
              }
              else
                Raise(Schema, String("Expected ") +
                  Param["Assignable"].AsString() + String(" for ") +
                  String("--") + GetParameterName(Arg));
            }
            else
            {
              if(++i >= ArgumentList.n() or IsParameter(ArgumentList[i]))
                return Raise(Schema, String("Expected ") +
                  Param["Assignable"].AsString() + String(" for ") +
                  String("--") + GetParameterName(Arg));
              else if(Param["Assignable"].AsString().StartsWith("value1"))
                v[GetParameterName(Arg)] = ParsePropertyList(
                  GetParameterValue(ArgumentList[i]));
              else
                v[GetParameterName(Arg)] = GetParameterValue(ArgumentList[i]);
            }
            if(Param["Assignable"].AsString() == "in-file")
              if(not File::Length(v[GetParameterName(Arg)].AsString()))
                return Raise(Schema, v[GetParameterName(Arg)].AsString() +
                  " does not exist or is empty.");
          }
          else
          {
            if(IsAssignment(Arg))
              return Raise(Schema, String("--") + GetParameterName(Arg) +
                " is not assignable.");
            else
              v[GetParameterName(Arg)] = true;
          }
        }
      }
      if(not HasRequiredParameters(v, Schema))
        return Value();
      return v;
    }

    static void Usage(const Value& Schema,
      count DescriptionStart = 30, count WrapColumn = 80)
    {
      String t = "        ";
      WrapColumn -= t.n();
      {
        String s;
        s << Schema["Process"].AsString().ToUpper() << " USAGE";
        count SpacesToPrepend = WrapColumn / 2 - s.n() / 2;
        while(--SpacesToPrepend)
          s.Prepend(" ");
        C::Error() >> " ";
        C::Bold();
        C::Error() << s;
        C::Reset();
      }
      C::Error()++;
      C::Reset();
      {
        C::Blue();
        C::Bold();
        C::Error() >> "NAME";
        C::Reset();
        C::Green();
        C::Error() >> t << Schema["Process"];
        C::Reset();
        C::Error() << " — " << Schema["Summary"];
      }
      C::Error()++;
      {
        C::Blue();
        C::Bold();
        C::Error() >> "SYNOPSIS";
        C::Reset();
        C::Green();
        C::Error() >> t << Schema["Process"];
        C::Reset();
        Array<Value> Groups, Names;
        Schema["Parameters"].EnumerateKeys(Groups);
        for(count i = 0; i < Groups.n(); i++)
        {
          String Group = Groups[i].AsString();
          Schema["Parameters"][Groups[i]].EnumerateKeys(Names);
          for(count j = 0; j < Names.n(); j++)
          {
            String Name = Names[j].AsString();
            C::Error() << " ";
            bool Required =
              Schema["Parameters"][Group][Name]["Required"].AsBoolean();
            String Assignable =
              Schema["Parameters"][Group][Name]["Assignable"].IsString() ?
              Schema["Parameters"][Group][Name]["Assignable"].AsString() :
              String();
            if(not Required)
            {
              C::Yellow();
              C::Error() << "[";
            }
            else
              C::Red();
            C::Error() << "--" << Name;
            if(Assignable)
              C::Error() << "=" << Assignable;
            if(not Required)
              C::Error() << "]";
            C::Reset();
          }
        }
        if(Schema["Trailing"].AsString())
        {
          C::Magenta();
          C::Error() << " [" << Schema["Trailing"].AsString() << "]";
          C::Reset();
        }
      }
      C::Error()++;
      {
        C::Blue();
        C::Bold();
        C::Error() >> "DESCRIPTION";
        C::Reset();
        List<String> Paragraphs =
          Schema["Description"].AsString().Tokenize("\n");
        for(count i = 0; i < Paragraphs.n(); i++)
        {
          if(i) C::Error()++;
          C::Error() >> t << WrapString(Paragraphs[i], WrapColumn,
            String("\n") + t);
        }
        C::Reset();
      }
      C::Error()++;
      {
        C::Blue();
        C::Bold();
        C::Error() >> "OPTIONS";
        C::Reset();
        Array<Value> Groups, Names;
        Schema["Parameters"].EnumerateKeys(Groups);
        for(count i = 0; i < Groups.n(); i++)
        {
          String Group = Groups[i].AsString();
          Schema["Parameters"][Groups[i]].EnumerateKeys(Names);
          if(Group)
          {
            C::Error() >> t;
            C::Underline();
            C::Error() << Group;
            C::Reset();
          }
          for(count j = 0; j < Names.n(); j++)
          {
            String Name = Names[j].AsString();
            String Assignable =
              Schema["Parameters"][Group][Name]["Assignable"].IsString() ?
              Schema["Parameters"][Group][Name]["Assignable"].AsString() :
              String();
            String Parameter;
            Parameter << t << "--" << Name;
            if(Assignable)
              Parameter << "=" << Assignable;
            count SpacesToAdd = DescriptionStart - Parameter.n();
            while(SpacesToAdd-- > 0)
              Parameter << " ";
            C::Bold();
            C::Error() >> Parameter;
            C::Reset();
            String Wrap = String("\n") + t;
            for(count k = 0; k < DescriptionStart; k++)
              Wrap << " ";
            C::Error() << " " << WrapString(
              Schema["Parameters"][Group][Name]["Description"].AsString(),
              WrapColumn, Wrap);
          }
          C::Error()++;
        }
      }
    }

    private:

    static bool IsAssignment(String x)
    {
      while(x.EraseBeginning("-")){}
      bool Result = x.Contains("=") and not x.StartsWith("=");
      String s = Result ? x.Substring(0, x.Find("=") - 1) : String();
      for(count i = 0; Result and i < s.n(); i++)
        if(not String::IsAlphanumeric(ascii(s[i])))
          Result = false;
      return Result;
    }

    static bool IsHyphen(String x)
    {
      return x.StartsWith("--") or x.StartsWith("-");
    }

    static bool IsParameter(String x)
    {
      return (IsAssignment(x) or IsHyphen(x)) and not IsStandardInHyphen(x);
    }

    static bool IsStandardInHyphen(String x)
    {
      return x == "-";
    }

    static String GetParameterName(String x)
    {
      while(x.EraseBeginning("-")){}
      if(IsAssignment(x))
        x.Erase(x.Find("="), x.n() - 1);
      return x;
    }

    static String GetParameterValue(String x)
    {
      while(x.EraseBeginning("-")){}
      if(IsAssignment(x))
        x.Erase(0, x.Find("="));
      return x;
    }

    static bool HasRequiredParameters(const Value& Arguments,
      const Value& Schema)
    {
      Value Groups = Schema["Parameters"].Keys();
      for(count i = 0; i < Groups.n(); i++)
      {
        Value Parameters = Schema["Parameters"][Groups[i]].Keys();
        for(count j = 0; j < Parameters.n(); j++)
        {
          Value Param = Parameters[j];
          if(Schema["Parameters"][Groups[i]][Param]["Required"].AsBoolean() and
            not Arguments.Contains(Param))
          {
            Raise(Schema, String("Missing required parameter --") + Param);
            return false;
          }
        }
      }
      return true;
    }

    static Value LookupParameter(String Parameter, const Value& Schema)
    {
      Value Keys = Schema["Parameters"].Keys();
      for(count i = 0; i < Keys.n(); i++)
        if(Schema["Parameters"][Keys[i]].Contains(Value(Parameter)))
          return Schema["Parameters"][Keys[i]][Parameter];
      return Value();
    }

    static Value ParsePropertyList(String x)
    {
      List<String> L = x.Tokenize(",");
      Value v; v.NewArray();
      for(count i = 0; i < L.n(); i++)
        L[i].Trim(), v.Add() = L[i];
      return v;
    }

    static Value Raise(const Value& Schema, String Message)
    {
      Usage(Schema);
      C::Reset();
      C::Red();
      C::Error() >> "Error: " << Message;
      C::Reset();
      return Value();
    }

    static String WrapString(String In, count Column, String Wrap)
    {
      for(count i = In.n() - In.n() % Column; i >= Column; i -= Column)
        In.Insert(Wrap, i);
      return In;
    }
  };
}
#endif
