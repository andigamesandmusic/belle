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

///@name Measure rests
///@{

///Engraves all the measure rests.
void MeasureRestEngrave(Music::ConstNode Left, Music::ConstNode Right);
void MeasureRestEngraveAll(Pointer<const Music> M);
///@}

#ifdef BELLE_IMPLEMENTATION
void MeasureRestEngrave(Music::ConstNode Left, Music::ConstNode Right)
{
  const number MeasureRestExtent = 1.f;
  const number MeasureRestMinimumWidth = 2.f;
  const number MeasureRestNumeralHeight = 4.f;
  const number MeasureRestPadding = 2.f;
  const number MeasureRestThickness = 1.f;
  const number MeasureRestTickCap = 0.25f;
  const number MeasureRestTickThickness = 0.2f;

  if(Left and Right)
  {
    if(Music::ConstEdge MeasureRest = Left->Next(MusicLabel(mica::MeasureRest),
      true))
    {
      number LeftOrigin = Left->Label.GetState(
        "IslandState", "TypesetX").AsNumber();
      number RightOrigin = Right->Label.GetState(
        "IslandState", "TypesetX").AsNumber();
      number OriginDistance = RightOrigin - LeftOrigin;
      Pointer<Stamp> LeftStamp = Left->Label.Stamp().Object();
      Pointer<Stamp> RightStamp = Right->Label.Stamp().Object();
      number LeftX = LeftStamp->Bounds().Right() + MeasureRestPadding;
      number RightX = OriginDistance + RightStamp->Bounds().Left() -
        MeasureRestPadding;
      mica::Concept MeasureNumberVal = MeasureRest->Get(mica::Value);
      count MeasureNumber = mica::integer(MeasureNumberVal) ?
        count(mica::numerator(MeasureNumberVal)) : count(0);
      Pointer<const Font> NotationFont = HouseStyle::GetFont(Left);
      if(!NotationFont) return;
      Pointer<const Typeface> NotationTypeface =
        NotationFont->GetTypeface(Font::Notation);
      if(!NotationTypeface) return;
      bool MultirestSpaceAvailable = RightX - LeftX >= MeasureRestMinimumWidth;
      bool ShowNumber = MeasureNumber > 0;
      bool ShowWholeRest = not ShowNumber or not MultirestSpaceAvailable;
      bool ShowMultirest = not ShowWholeRest;

      if(ShowMultirest)
      {
        Pointer<Path> p;
        Shapes::AddBox(*p.New(), Box(
          Vector(LeftX, -Half(MeasureRestThickness)),
          Vector(RightX, Half(MeasureRestThickness))));
        Shapes::AddLine(*p,
          Vector(LeftX, -MeasureRestExtent),
          Vector(LeftX, MeasureRestExtent), MeasureRestTickThickness,
          true, true, true, MeasureRestTickCap);
        Shapes::AddLine(*p,
          Vector(RightX, -MeasureRestExtent),
          Vector(RightX, MeasureRestExtent), MeasureRestTickThickness,
          true, true, true, MeasureRestTickCap);
        LeftStamp->Add()->p = p;
      }

      if(ShowWholeRest)
      {
        Pointer<const Glyph> WholeRest = RestGlyph(NotationFont, Ratio(1));
        LeftStamp->Add()->p = WholeRest;
        Vector Location(Average(LeftX, RightX), 1.f);
        LeftStamp->z()->a = Affine::Translate(Location);
      }

      if(ShowNumber)
      {
        TimeSignatureEngraveRegularNumeral(Left, NotationTypeface,
          MeasureNumber, Value(MeasureRest),
          Vector(Average(LeftX, RightX), MeasureRestNumeralHeight),
          mica::Undefined, Music::ConstNode());
      }
    }
  }
}

void MeasureRestEngraveAll(Pointer<const Music> M)
{
  if(Pointer<const Geometry> G = System::Geometry(M))
    for(count i = 0; i < G->GetNumberOfParts(); i++)
      for(count j = 0; j < G->GetNumberOfInstants(); j++)
        if(Music::ConstNode Left = G->LookupIsland(i, j))
          if(Music::ConstNode Right = Left->Next(MusicLabel(mica::MeasureRest)))
            MeasureRestEngrave(Left, Right);
}
#endif
