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

#define BELLE_COMPILE_INLINE
#define BELLE_WITH_FREETYPE
#include "belle.h"

//Use some namespaces to help with scoping.
using namespace prim;
using namespace belle;

//Shows how to use this utility.
void DisplayUsage(String Program);
void DisplayUsage(String Program)
{
  C::Out()++;
  C::Out() >> "To convert to SVG:       " << Program <<
    " <font-file> <output>.svg (--smufl)";
  C::Out() >> "To convert to BelleFont: " << Program <<
    " <font-file> <output>.bellefont (--smufl)";
  C::Out() >> "To convert to JSON:      " << Program <<
    " <font-file> <output>.json (--smufl)";
  C::Out()++;
}

//Converts a font file to SVG, bellefont, or JSON font
void ConvertFont(String FontFile, String OutputFile, bool IsSMuFL);
void ConvertFont(String FontFile, String OutputFile, bool IsSMuFL)
{
  //Load the font file into a typeface.
  Typeface f;
  C::Out() >> "Converting '" << FontFile << "'...";
  String Result = f.ImportFromFontFile(FontFile);
  if(IsSMuFL)
  {
    Array<unicode> Codepoints = f.GlyphCodepoints();
    for(count i = 0; i < Codepoints.n(); i++)
    {
      unicode Character = Codepoints[i];
      if(Character < 0xE000U || Character >= 0xF400)
        f.Remove(Character);
      else
      {
        Pointer<Glyph> g = f.LookupGlyph(Character);
        if(g->Bounds().IsEmpty() && !(g->AdvanceWidth > 0.f))
          f.Remove(Character);
      }
    }
    f.Scale(Vector(4.f, 4.f));
  }
  if(Result)
    C::Out() >> Result;

  if(OutputFile.Contains(".svg"))
  {
    //Save the typeface as an SVG font file.
    String SVGString;
    C::Out() >> "Saving to '" << OutputFile << "'...";
    f.ExportToSVGString(SVGString);
    File::Write(OutputFile, SVGString);
  }
  else if(OutputFile.Contains(".bellefont"))
  {
    //Save the typeface as a binary bellefont.
    Array<byte> BelleFontArray;
    C::Out() >> "Saving to '" << OutputFile << "'...";
    f.ExportToArray(BelleFontArray);
    File::Write(OutputFile, BelleFontArray);
  }
  else if(OutputFile.Contains(".json"))
  {
    //Save the typeface as JSON.
    C::Out() >> "Saving to '" << OutputFile << "'...";
    String JSONString;
    f.ExportToJSON(JSONString);
    File::Write(OutputFile, JSONString);
  }
  else
  {
    C::Out() >> "Output extension unknown in " << OutputFile;
    return;
  }

  if(Result)
    C::Out() >> "Conversion may have failed. Check the output.";
  else
    C::Out() >> "Conversion was successful.";
}

void CommandLine(List<String>& Arguments);
void CommandLine(List<String>& Arguments)
{
  if(Arguments.n() == 3)
    ConvertFont(Arguments[1], Arguments[2], false);
  else if(Arguments.n() == 4)
    ConvertFont(Arguments[1], Arguments[2], Arguments[3] == "--smufl");
  else
    DisplayUsage(Arguments[0]);
}

int main(int ArgumentCount, ascii* ArgumentValues[])
{
  List<String> Arguments;
  for(count i = 0; i < count(ArgumentCount); i++)
    Arguments.Add() = ArgumentValues[i];
  CommandLine(Arguments);
  return AutoRelease<Console>();
}
