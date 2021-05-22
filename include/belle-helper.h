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

#ifndef BELLE_HELPER_H
#define BELLE_HELPER_H

#ifndef BELLE_EXAMPLES_HELPER_NO_RESOURCES
#include "resources.h"
#endif

///Various helper utilities
class Helper
{
  public:

  static void ConvertPDFToPNG(belle::String PDFFile, belle::String ImageFile,
    belle::number PPI, bool Grayscale)
  {
    belle::PDFToPNG(PDFFile, ImageFile, PPI, Grayscale);
  }

  ///Loads the font-notation and Averia typefaces from resources.
  static belle::Font ImportNotationFont()
  {
    belle::Font NotationFont;
#ifndef BELLE_EXAMPLES_HELPER_NO_RESOURCES
    belle::String s;
    belle::C::Out() >> "Loading font-notation.json";
    s = Resources::Load("font-notation.json");
    NotationFont.Add(belle::Font::Notation)->ImportFromJSON(s);
    belle::C::Out() >> "Loading font-text-regular.json";
    s = Resources::Load("font-text-regular.json");
    NotationFont.Add(belle::Font::Regular)->ImportFromJSON(s);
    belle::C::Out() >> "Loading font-text-italic.json";
    s = Resources::Load("font-text-italic.json");
    NotationFont.Add(belle::Font::Italic)->ImportFromJSON(s);
    belle::C::Out() >> "Loading font-text-bold.json";
    s = Resources::Load("font-text-bold.json");
    NotationFont.Add(belle::Font::Bold)->ImportFromJSON(s);
    belle::C::Out() >> "Loading font-text-bold-italic.json";
    s = Resources::Load("font-text-bold-italic.json");
    NotationFont.Add(belle::Font::BoldItalic)->ImportFromJSON(s);
#endif
    return NotationFont;
  }

  static belle::String GetEmbeddedScore(belle::String Filename)
  {
#ifndef BELLE_EXAMPLES_HELPER_NO_RESOURCES
    return ConvertSimplifiedToXML(Resources::Load(Filename));
#else
    (void)Filename;
    return "";
#endif
  }

  static belle::String ConvertSimplifiedToXML(belle::String Input)
  {
    return belle::ShorthandToXML(Input);
  }
};
#endif
