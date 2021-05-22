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

#ifndef BELLE_ENGRAVER_KEYSIGNATURE_H
#define BELLE_ENGRAVER_KEYSIGNATURE_H

namespace BELLE_NAMESPACE
{
  ///Algorithms for typesetting a key signature.
  class KeySignature
  {
    public:

    /*Static interface -- instances not allowed*/ KeySignature();

    ///Engrave the different forms of key signature.
    static void Engrave(Music::ConstNode IslandNode, Music::ConstNode Token)
    {
      //Get the island stamp.
      Pointer<Stamp> IslandStamp = IslandNode->Label.Stamp().Object();
      Pointer<Value::ConstReference> H =
        IslandNode->Label.GetState("HouseStyle", "Global").Object();
      Pointer<const Font> NotationFont = H->Get()["NotationFont"].ConstObject();
      if(!NotationFont) return;
      Pointer<const Typeface> NotationTypeface =
        NotationFont->GetTypeface(Font::Notation);
      if(!NotationTypeface) return;

      mica::Concept ActiveClef =
        IslandNode->Label.GetState("PartState", "Clef", "Active");
      mica::Concept k = Token->Get(mica::Value);
      count n = Utility::GetNumberOfAccidentals(k);
      mica::Concept AccidentalType = mica::map(k, mica::Accidental);
      mica::Concept OverrideType = Token->Get(mica::Accidental);
      if(not mica::undefined(OverrideType))
        AccidentalType = OverrideType;
      Pointer<const Glyph> GlyphSymbol =
        SMuFLGlyphFromTypeface(NotationTypeface, AccidentalType);
      for(count i = 0; i < n; i++)
      {
        Vector Offset(number(i) * 1.f,
          number(Utility::GetAccidentalPosition(k, ActiveClef, i)) / 2.f);
        IslandStamp->Add()->p = GlyphSymbol;
        IslandStamp->z()->a = Affine::Translate(Offset);
        IslandStamp->z()->Context = Token;
      }
      Box r = IslandStamp->Bounds();
      r.Dilate(0.1f);
      IslandStamp->AddArtificialBounds(r);
    }
  };
}
#endif
