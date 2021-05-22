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
#include "prim.h"
using namespace prim;

void CommandLine(List<String>& Arguments);
bool ModuleIncluded(bool UsingModules, List<String>& ModulesList, String Name);
String GetPath(String x);

//To help with identifying token span types (same order as above).
struct Preprocessor
{
  List<String> a, b;
  Preprocessor()
  {
    a.Add() = "/*"; b.Add() = "*/";
    a.Add() = "//"; b.Add() = String::LF;
    a.Add() = "\""; b.Add() = "\"";
    a.Add() = "\'"; b.Add() = "\'";
    a.Add() = "#include \""; b.Add() = "\"";
  }

  enum Definitions
  {
    BlockComment,
    LineComment,
    LiteralString,
    Character,
    IncludeHeader
  };
};

bool ModuleIncluded(bool UsingModules, List<String>& ModulesList, String Name)
{
  if(!UsingModules)
    return true;
  Name.EraseEnding(".h");
  Name.EraseBeginning("prim_");
  return ModulesList.Contains(Name);
}

String GetPath(String x)
{
  for(count i = x.n() - 1; i >= 0; i--)
  {
    if(x[i] == '/')
    {
      x.Erase(i + 1, x.n());
      return x;
    }
  }
  return "";
}

void CommandLine(List<String>& Arguments)
{
  Preprocessor pp;

  if(Arguments.n() < 3)
  {
    C::Out() >> "Usage: condense in.h out.h [--nocomments] [--modules] ...";
    C::Out()++;
    C::Out() >> "--nocomment : Deletes all comments (to save space)";
    C::Out() >> "--modules   : Individually specify each module to include.";
    C::Out() >> "  The modules you can include are listed in prim.h.";
    C::Out() >> "  You can specify them by filename or simple name (with or";
    C::Out() >> "  without .h). Note that some headers such as Encoding.h have";
    C::Out() >> "  submodules that are included within a class. You need to";
    C::Out() >> "  specify the submodules too.";
    C::Out()++;
    return;
  }

  if(Arguments[1] == Arguments[2])
  {
    C::Out() >> "Aborting: input file is the same as the output file.";
    C::Out()++;
    return;
  }

  String Input = Arguments[1];
  String Output = Arguments[2];
  bool NoComments = Arguments.Contains("--nocomments");
  bool Modules = Arguments.Contains("--modules");
  List<String> ModuleList;
  if(Modules)
  {
    for(count i = 3; i < Arguments.n(); i++)
    {
      if(!Arguments[i].StartsWith("--"))
      {
        String s = Arguments[i];
        s.EraseEnding(".h");
        ModuleList.Add() = s;
      }
    }
  }

  C::Out() >> "Condensing " << Arguments[1] <<
    (NoComments ? " without comments" : " retaining comments");
  if(Modules)
  {
    C::Out() >> "Selected modules: ";
    for(count i = 0; i < ModuleList.n(); i++)
      C::Out() >> " * " << ModuleList[i];
  }

  String In, Out;
  File::Read(Input, In);
  Out = In;

  //Tab and endline preprocessing.
  Out.Replace(String::CRLF, String::LF);
  Out.Replace("\t", "  ");

  //Special macros.
  Out.Replace("SPECIAL_GENERATED_DATE", String(Time()));

  //Preprocess the file incrementally.
  String IncludedFiles;
  count StartIndex = 0;
  while(StartIndex < Out.n())
  {
    String Between;
    String::Span Place;
    count Result = Out.FindBetweenAmong(pp.a, pp.b, Place,
      Between, StartIndex);
    if(Result == -1)
      break;

    switch(Result)
    {
      case Preprocessor::BlockComment:
        if((NoComments && Place.i() != 0) ||
          (Out.Substring(Place.i(), Place.j()).Contains("opyright") &&
          Place.i() != 0))
        {
          Out.Erase(Place.i(), Place.j());
          StartIndex = Place.i();
        }
        else
          StartIndex = Place.j() + 1;
        break;
      case Preprocessor::LineComment:
        if(NoComments)
        {
          Out.Erase(Place.i(), Place.j() - 1);
          StartIndex = Place.i();
        }
        else
          StartIndex = Place.j() + 1;
        break;
      case Preprocessor::IncludeHeader:
      {
        Out.Erase(Place.i(), Place.j());
        StartIndex = Place.i();
        String JustFilename = Between;
        JustFilename.Erase(0, Between.Find("/"));
        JustFilename.Erase(0, Between.Find("\\"));

        if(!ModuleIncluded(Modules, ModuleList, JustFilename))
        {
          String NotIncluded = "//Omitted: ";
          NotIncluded << JustFilename;
          Out.Insert(NotIncluded, Place.i());
        }
        else if(!IncludedFiles.Contains(JustFilename)) //Only include once.
        {
          //Keep track of included files.
          IncludedFiles >> JustFilename;
          String IncludeFile;
          String RelativePath;
          RelativePath << GetPath(Input);
          RelativePath << Between;
          if(!File::Read(RelativePath, IncludeFile))
            C::Out() >> "Warning: could not read " << RelativePath <<
              ".";

          //Remove escapes for easier parsing.
          IncludeFile.Replace("\\\\", "$CONDENSE_SPECIAL_ESCAPED_BACKSLASH");
          IncludeFile.Replace("\\\"", "$CONDENSE_SPECIAL_ESCAPED_DOUBLE_QUOTE");
          IncludeFile.Replace("\\'", "$CONDENSE_SPECIAL_ESCAPED_SINGLE_QUOTE");

          Out.Insert(IncludeFile, Place.i());
          {
            String m = JustFilename;
            m.EraseEnding(".h");
            m.EraseBeginning("prim_");
            m.Prepend("\n  * ");
            m << "SPECIAL_GENERATED_MODULES";
            Out.Replace("SPECIAL_GENERATED_MODULES", m);
          }
        }
        break;
      }
      default:
        StartIndex = Place.j() + 1;
    };
  }

  Out.Replace("SPECIAL_GENERATED_MODULES", "");

  //Add back escapes.
  Out.Replace("$CONDENSE_SPECIAL_ESCAPED_BACKSLASH", "\\\\");
  Out.Replace("$CONDENSE_SPECIAL_ESCAPED_DOUBLE_QUOTE", "\\\"");
  Out.Replace("$CONDENSE_SPECIAL_ESCAPED_SINGLE_QUOTE", "\\'");

  //Condense multiple lines and format beginning and ending.
  while(Out.Replace(" \n", "\n")) {}
  while(Out.Replace("\n\n\n", "\n\n")) {}
  if(Out.StartsWith(String::LF))
    Out.Erase(0, 0);
  if(!Out.EndsWith(String::LF))
    Out.Append(String::LF);

  //Write the file.
  File::Write(Output, Out);

  //Indicate included files.
  C::Out() >> /*"Also included files: " >> IncludedFiles >>*/
    "Wrote " << Output;
  C::Out()++;
}

int main(int ArgumentCount, ascii* ArgumentValues[])
{
  List<String> Arguments;
  for(count i = 0; i < count(ArgumentCount); i++)
  {
    String Arg(ArgumentValues[i]);
    Arguments.Append(Arg);
  }

  CommandLine(Arguments);
  return AutoRelease<Console>();
}
