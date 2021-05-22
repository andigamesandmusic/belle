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

#ifndef BELLE_ENGRAVER_CLEF_H
#define BELLE_ENGRAVER_CLEF_H

namespace BELLE_NAMESPACE
{
  ///Algorithms for typesetting a clef.
  class Clef
  {
    public:

    ///Engrave the different forms of clefs.
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

      //Get the clef size. For non-initial clefs the clef appears smaller.
      number Size = 1.0;
      integer ClefInstance =
        IslandNode->Label.GetState("PartState", "Clef", "Instance").AsInteger();
      if(ClefInstance > 1)
        Size = 0.2f / 0.3f;
      number LeftPadding = IslandNode->Previous(MusicLabel(mica::Partwise)) ?
        0.f : 1.f;

      mica::Concept ClefType = Token->Label.Get(mica::Value);

      if(ClefType == mica::TrebleClef)
      {
        Vector Offset(LeftPadding, -1.f);
        Pointer<const Glyph> GlyphSymbol =
          SMuFLGlyphFromTypeface(NotationTypeface, mica::GClef);
        IslandStamp->Add()->p = GlyphSymbol;
        IslandStamp->z()->a = Affine::Translate(Offset) * Affine::Scale(Size);
        IslandStamp->z()->Context = Token;
      }
      else if(ClefType == mica::AltoClef or ClefType == mica::TenorClef)
      {
        Vector Offset(LeftPadding, 0.f);
        if(ClefType == mica::TenorClef)
          Offset = Vector(LeftPadding, 1.f);
        Pointer<const Glyph> GlyphSymbol =
          SMuFLGlyphFromTypeface(NotationTypeface, mica::CClef);
        IslandStamp->Add()->p = GlyphSymbol;
        IslandStamp->z()->a = Affine::Translate(Offset) * Affine::Scale(Size);
        IslandStamp->z()->Context = Token;
      }
      else if(ClefType == mica::BassClef)
      {
        Vector Offset(LeftPadding, 1.f);
        Pointer<const Glyph> GlyphSymbol =
          SMuFLGlyphFromTypeface(NotationTypeface, mica::FClef);
        IslandStamp->Add()->p = GlyphSymbol;
        IslandStamp->z()->a = Affine::Translate(Offset) * Affine::Scale(Size);
        IslandStamp->z()->Context = Token;
      }

      //Add artificial bounds for the clef.
      {
        number RightExtent = IslandStamp->GetAbstractBounds().Right();
        IslandStamp->AddArtificialBounds(Box(
          Vector(RightExtent, -3.f), Vector(RightExtent, 3.f)));

        number LeftExtent = IslandStamp->GetAbstractBounds().Left();
        LeftExtent -= 0.8f;
        IslandStamp->AddArtificialBounds(Box(
          Vector(LeftExtent, -2.f), Vector(LeftExtent, 2.f)));
      }
    }
  };
}
#endif
