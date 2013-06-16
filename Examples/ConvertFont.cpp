/*
  ==============================================================================

  Copyright 2007-2013 William Andrew Burnson. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY WILLIAM ANDREW BURNSON ''AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
  EVENT SHALL WILLIAM ANDREW BURNSON OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  ------------------------------------------------------------------------------

  This file is part of Belle, Bonne, Sage --
    The 'Beautiful, Good, Wise' C++ Vector-Graphics Library for Music Notation 

  ==============================================================================
*/

#define BELLE_COMPILE_INLINE
#include "Belle.h"

//Use some namespaces to help with scoping.
using namespace prim;
using namespace belle;
using namespace prim::planar;

//Shows how to use this utility.
void DisplayUsage()
{
  c++;
  c >> "To convert to SVG:       ConvertFont [font file] [output].svg";
  c >> "To convert to BelleFont: ConvertFont [font file] [output].bellefont";
  c++;
}

//Converts a font file to either the SVG or bellefont format.
void ConvertFont(String FontFile, String OutputFile)
{
  //Load the font file into a typeface.
  Typeface f;
  c >> "Converting '" << FontFile << "'...";
  String Result = f.ImportFromFontFile(FontFile);
  if(Result)
    c >> Result;
  
  if(OutputFile.Contains(".svg"))
  {
    //Save the typeface as an SVG font file.
    String SVGString;
    c >> "Saving to '" << OutputFile << "'...";
    f.ExportToSVGString(SVGString);
    File::Write(OutputFile, SVGString);
  }
  else if(OutputFile.Contains(".bellefont"))
  {
    //Save the typeface as a binary bellefont.
    Array<byte> BelleFontArray;
    c >> "Saving to '" << OutputFile << "'...";
    f.ExportToArray(BelleFontArray);
    File::Write(OutputFile, BelleFontArray);
  }
  else
  {
    c >> "Output extension unknown in " << OutputFile;
    DisplayUsage();
    return;
  }
  
  if(Result)
    c >> "Conversion may have failed. Check the output.";
  else
    c >> "Conversion was successful.";
}

void CommandLine(List<String>& Arguments)
{
  if(Arguments.n() == 3)
    ConvertFont(Arguments[1], Arguments[2]);
  else
    DisplayUsage();
}

//Stub main that parses incoming arguments into a list.
int main(int ArgumentCount, ascii* ArgumentValues[])
{
  List<String> Arguments;
  for(count i = 0; i < (count)ArgumentCount; i++)
    Arguments.Add() = ArgumentValues[i];

  //Run the program.
  CommandLine(Arguments);

  //Add a blank line to the end of the program and wait if on Windows.
  c++;
#ifdef PRIM_ENVIRONMENT_WINDOWS
  c.Wait("Press any key and hit return to close...");
#endif
  return 0;
}
