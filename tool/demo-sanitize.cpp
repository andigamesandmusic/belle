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

static count GetLeadingSpace(const String& s)
{
  for(count i = 0; i < s.n(); i++)
    if(s[i] != ' ')
      return i;
  return s.n();
}

static String StringOfSpace(count Spaces)
{
  String s;
  for(count i = 0; i < Spaces; i++)
    s << " ";
  return s;
}

static bool Rewrite(String& s, count MaximumLength, String CopyrightHeader,
  bool IndentBlankLines)
{
  count LinesNotUTF8 = 0;
  count LinesWithTabs = 0;
  count LinesLongerThanMax = 0;
  bool Result = true;

  String LF = s;
  LF.LineEndingsToLF();
  if(s != LF)
  {
    C::Out() >> "Converting all line-endings to LF...";
    s = LF;
  }
  String Trimmed_s = s;
  Trimmed_s.Trim();
  List<String> a = Trimmed_s.Tokenize("\n");
  String t;
  count PreviousWhitespace = 0;
  for(count i = 0; i < a.n(); i++)
  {
    if(a[i].StartsWith("#error STYLE"))
    {
      C::Out() >> "File already contains style problems. Fix these and rerun.";
      return false;
    }

    //Skip multiple blank lines.
    if(i > 0)
    {
      String l1 = a[i - 1];
      String l2 = a[i];
      l1.Trim();
      l2.Trim();
      if(!l1 && !l2)
        continue;
    }

    String l = a[i];
    String lUTF8 = l;
    lUTF8.ForceToUTF8();
    if((l != lUTF8 || l.Contains(String(meta::BadCharacter))) &&
      LinesNotUTF8 < 10)
    {
      t >> "#error STYLE Next line contains malformed UTF-8";
      l = lUTF8;
      LinesNotUTF8++;
    }
    if(l.Contains("\t") && LinesWithTabs < 10)
    {
      t >> "#error STYLE Next line contains tab character";
      LinesWithTabs++;
    }
    if(l.Characters() > MaximumLength && LinesLongerThanMax < 10 &&
      !l.Contains("#include"))
    {
      t >> "#error STYLE Next line longer than " << MaximumLength <<
        " characters";
      LinesLongerThanMax++;
    }

    count LeadingSpace = GetLeadingSpace(l);
    if(LeadingSpace == l.n())
    {
      if(IndentBlankLines)
        l = StringOfSpace(PreviousWhitespace);
      else
        l = "";
    }
    else
    {
      PreviousWhitespace = LeadingSpace;
      while(l.EraseEnding(" ")) {}
    }

    t >> l;
  }
  t++;

  if(CopyrightHeader)
  {
    while(t.StartsWith("//"))
      t.Erase(0, t.Find("\n"));
    while(t.StartsWith("/*"))
      t.Erase(0, t.Find("*/") + 1);
    while(t.StartsWith("\n"))
      t.Erase(0, 0);
    t.Prepend(CopyrightHeader + "\n\n");
  }

  if(s != t)
  {
    s = t;
    C::Out() >> "File altered.";
    if(LinesNotUTF8)
      C::Out() >> "Lines with malformed UTF8:  " << LinesNotUTF8;
    if(LinesWithTabs)
      C::Out() >> "Lines with tabs:            " << LinesWithTabs;
    if(LinesLongerThanMax)
      C::Out() >> "Lines longer than " << MaximumLength << " chars: " <<
        LinesLongerThanMax;
    Result = not LinesNotUTF8 and not LinesWithTabs and not LinesLongerThanMax;
  }
  else
    C::Out() << " no changes";
  return Result;
}

static Value GetCommandLineSchema()
{
  Value Schema = Arguments::NewSchema("sanitize",
    "Canonicalizes whitespace and sanitizes code. For things that\n"
    "need to be manually fixed, an #error is inserted to the line.",
    String("Silent fixes:\n") +
    " * Changes line endings to LF\n"
    " * Removes consecutive blank lines\n"
    " * Adjusts blank line indentation to that of previous line\n"
    "Writes #error for:\n"
    " * Characters not in UTF-8\n"
    " * Tab characters\n"
    " * Lines longer than [maxlen] characters\n",
    "...");
  Arguments::AddParameter(Schema, "indentblanklines", Arguments::Property,
    Arguments::Optional, "Indent blank lines (default = yes)", "Options");
  Arguments::AddParameter(Schema, "maxlen", Arguments::Property,
    Arguments::Optional, "Maximum line length (default = 80)", "Options");
  Arguments::AddParameter(Schema, "copyright", Arguments::Property,
    Arguments::Optional, "Copyright Header", "Options");
  return Schema;
}

int main(int argc, const char** argv)
{
  AutoRelease<Console> ReleasePool;
  bool Result = true;
  if(Value Arguments = Arguments::Process(argc, argv, GetCommandLineSchema()))
  {
    count MaximumLength = 80;
    String CopyrightHeader;
    bool IndentBlankLines = true;
    if(Arguments["maxlen"])
      MaximumLength = Arguments["maxlen"].AsCount();
    if(Arguments["copyright"])
      CopyrightHeader = Arguments["copyright"].AsString();
    if(Arguments["indentblanklines"])
      IndentBlankLines = Arguments["indentblanklines"].AsString() != "no";
    for(count i = 0; i < Arguments["trailing"].n(); i++)
    {
      String Filename = Arguments["trailing"][i].AsString();
      String Input;
      C::Out() >> "Processing " << Filename << "...";
      File::Read(Filename, Input);
      String Output = Input;
      Result = Rewrite(Output, MaximumLength, CopyrightHeader,
        IndentBlankLines) and Result;
      if(Output != Input)
        File::Write(Filename, Output);
    }
  }
  else
    Arguments::Usage(GetCommandLineSchema());

  return Result ? 0 : 1;
}
