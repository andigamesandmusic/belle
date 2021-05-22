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

#ifndef BELLE_VTABLES_H
#define BELLE_VTABLES_H

/*Defines virtual functions to prevent vtables from being emitted in each
compilation unit.*/
#ifdef BELLE_COMPILE_INLINE
namespace BELLE_NAMESPACE
{
  Blank::~Blank() {}
  Glyph::~Glyph() {}
  Image::~Image() {}
  Painter::~Painter() {CheckStack();}
  Canvas::~Canvas() {}
  Portfolio::~Portfolio() {}
  void Path::VtableEmit() {}
  PDF::JPEGImage::~JPEGImage() {}
  #ifdef JUCE_VERSION
  JUCE::~JUCE() {}
  JUCE::Image::~Image() {}
  JUCE::Properties::~Properties() {}
  #endif
  Painter::Properties::~Properties() {}
  PDF::Properties::~Properties() {}
  PDF::~PDF() {}
  SVG::Properties::~Properties() {}
  SVG::~SVG() {}
  MusicLabel::~MusicLabel() {}
  Geometry::~Geometry() {}
  Font::~Font() {}
  Stamp::~Stamp() {}
  Score::Progress::~Progress() {}
}
#endif

#endif
