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

#ifndef BELLE_ENGRAVER_BARLINE_H
#define BELLE_ENGRAVER_BARLINE_H

namespace BELLE_NAMESPACE
{
  ///Algorithms for typesetting a barline.
  class Barline
  {
    public:

    ///Engrave the different forms of barline.
    static void Engrave(Music::ConstNode IslandNode, Music::ConstNode Token)
    {
      //Get the island stamp.
      Pointer<Stamp> IslandStamp = IslandNode->Label.Stamp().Object();

      //Distance to the next staff in spaces (should perhaps be passed in).
      mica::Concept BarlineType = Token->Get(mica::Value);

      //Get the staff height.
      number StaffHeight = IslandNode->Label.GetState(
        "PartState", "Staff", "Lines").AsNumber() - 1.0f;

      //Determine whether or not this barline connects to the previous staff.
      number AmountToExtend = 0.f;
      if(IslandNode->Label.GetState(
        "InstantState", "BarlineConnectsToPreviousStaff") &&
        IslandNode->Label.GetState("InstantState",
          "InteriorDistanceToPreviousStaff").AsNumber() > 0.f)
      {
        //Determine how far to extend the barline to the previous staff.
        AmountToExtend = Max(IslandNode->Label.GetState("InstantState",
          "InteriorDistanceToPreviousStaff").AsNumber(), number(2.f));
      }

      const number BarlineThickness = HouseStyle::GetValue(IslandNode,
        "BarlineThickness").AsNumber();
      const number ThickBarlineThickness = HouseStyle::GetValue(IslandNode,
        "ThickBarlineThickness").AsNumber();

      if(BarlineType == mica::StandardBarline)
      {
        Pointer<Path> p = new Path;
        IslandStamp->Add()->p = p;
        IslandStamp->z()->Context = Token;
        Shapes::AddLine(*p, Vector(0.f, StaffHeight / 2.f + AmountToExtend),
          Vector(0.f, -StaffHeight / 2.f), BarlineThickness,
          true, false, false);
      }
      else if(BarlineType == mica::DashedBarline)
      {
        Pointer<Path> p = new Path;
        IslandStamp->Add()->p = p;
        IslandStamp->z()->Context = Token;
        Shapes::AddDashedLine(*p,
          Vector(0.f, -StaffHeight / 2.f),
          Vector(0.f, StaffHeight / 2.f + AmountToExtend),
          BarlineThickness,
          0.5f, 0.5f, 0.5f, false, false);
      }
      else if(BarlineType == mica::DottedBarline)
      {
        Pointer<Path> p = new Path;
        IslandStamp->Add()->p = p;
        IslandStamp->z()->Context = Token;
        Shapes::AddDashedLine(*p,
          Vector(0.f, -StaffHeight / 2.f + 0.5f),
          Vector(0.f, StaffHeight / 2.f + AmountToExtend + 0.5f),
          BarlineThickness * 2.f,
          1.f, 0.f, 1.f, false, true);
      }
      else if(BarlineType == mica::ThickBarline)
      {
        Pointer<Path> p = new Path;
        IslandStamp->Add()->p = p;
        IslandStamp->z()->Context = Token;
        Shapes::AddLine(*p, Vector(0.f, StaffHeight / 2.f + AmountToExtend),
          Vector(0.f, -StaffHeight / 2.f), ThickBarlineThickness,
          true, false, false);
      }
      else if(BarlineType == mica::TickBarline)
      {
        Pointer<Path> p = new Path;
        IslandStamp->Add()->p = p;
        IslandStamp->z()->Context = Token;
        Shapes::AddLine(*p, Vector(0.f, StaffHeight * 5.f / 8.f),
          Vector(0.f, StaffHeight * 3.f / 8.f), BarlineThickness,
          true, true, true);
      }
      else if(BarlineType == mica::ShortBarline)
      {
        Pointer<Path> p = new Path;
        IslandStamp->Add()->p = p;
        IslandStamp->z()->Context = Token;
        Shapes::AddLine(*p, Vector(0.f, StaffHeight / 4.f),
          Vector(0.f, -StaffHeight / 4.f), BarlineThickness,
          true, false, false);
      }
      else if(BarlineType == mica::ThinDoubleBarline)
      {
        Pointer<Path> p = new Path;
        IslandStamp->Add()->p = p;
        IslandStamp->z()->Context = Token;
        Shapes::AddLine(*p, Vector(0.3f, StaffHeight / 2.f + AmountToExtend),
          Vector(0.3f, -StaffHeight / 2.f),
          BarlineThickness, true, false, false);
        Shapes::AddLine(*p, Vector(-0.3f, StaffHeight / 2.f + AmountToExtend),
          Vector(-0.3f, -StaffHeight / 2.f),
          BarlineThickness, true, false, false);
      }
      else if(BarlineType == mica::FinalBarline)
      {
        Pointer<Path> p = new Path;
        IslandStamp->Add()->p = p;
        IslandStamp->z()->Context = Token;
        number z = -0.67f - ThickBarlineThickness / 2.f +
          BarlineThickness / 2.f;
        Shapes::AddLine(*p, Vector(0.f + z, StaffHeight / 2.f + AmountToExtend),
          Vector(0.f + z, -StaffHeight / 2.f),
          BarlineThickness, true, false, false);
        Shapes::AddLine(*p, Vector(0.67f + z,
          StaffHeight / 2.f + AmountToExtend),
          Vector(0.67f + z, -StaffHeight / 2.f),
          ThickBarlineThickness, true, false, false);
      }
      else if(BarlineType == mica::BeginRepeatBarline)
      {
        Pointer<Path> p = new Path;
        IslandStamp->Add()->p = p;
        IslandStamp->z()->Context = Token;
        number z = 0.67f + ThickBarlineThickness / 2.f;
        Shapes::AddLine(*p, Vector(0.f + z, StaffHeight / 2.f + AmountToExtend),
          Vector(0.f + z, -StaffHeight / 2.f),
          BarlineThickness, true, false, false);
        Shapes::AddLine(*p, Vector(-0.67f + z,
          StaffHeight / 2.f + AmountToExtend),
          Vector(-0.67f + z, -StaffHeight / 2.f),
          ThickBarlineThickness, true, false, false);
        Shapes::AddCircle(*p, Vector(0.6f + z, 0.5f), 0.52f);
        Shapes::AddCircle(*p, Vector(0.6f + z, -0.5f), 0.52f);
      }
      else if(BarlineType == mica::ThickThinBarline)
      {
        Pointer<Path> p = new Path;
        IslandStamp->Add()->p = p;
        IslandStamp->z()->Context = Token;
        number z = 0.67f + ThickBarlineThickness / 2.f;
        Shapes::AddLine(*p, Vector(0.f + z, StaffHeight / 2.f + AmountToExtend),
          Vector(0.f + z, -StaffHeight / 2.f),
          BarlineThickness, true, false, false);
        Shapes::AddLine(*p, Vector(-0.67f + z,
          StaffHeight / 2.f + AmountToExtend),
          Vector(-0.67f + z, -StaffHeight / 2.f),
          ThickBarlineThickness, true, false, false);
      }
      else if(BarlineType == mica::ThickDoubleBarline)
      {
        Pointer<Path> p = new Path;
        IslandStamp->Add()->p = p;
        IslandStamp->z()->Context = Token;
        number z = 0.97f + ThickBarlineThickness / 2.f;
        Shapes::AddLine(*p, Vector(0.f + z, StaffHeight / 2.f + AmountToExtend),
          Vector(0.f + z, -StaffHeight / 2.f),
          ThickBarlineThickness, true, false, false);
        Shapes::AddLine(*p, Vector(-0.97f + z,
          StaffHeight / 2.f + AmountToExtend),
          Vector(-0.97f + z, -StaffHeight / 2.f),
          ThickBarlineThickness, true, false, false);
      }
      else if(BarlineType == mica::EndRepeatBarline)
      {
        Pointer<Path> p = new Path;
        IslandStamp->Add()->p = p;
        IslandStamp->z()->Context = Token;
        number z = -0.67f - ThickBarlineThickness / 2.f +
          BarlineThickness / 2.f;
        Shapes::AddLine(*p, Vector(0.f + z, StaffHeight / 2.f + AmountToExtend),
          Vector(0.f + z, -StaffHeight / 2.f),
          BarlineThickness, true, false, false);
        Shapes::AddLine(*p, Vector(0.67f + z,
          StaffHeight / 2.f + AmountToExtend),
          Vector(0.67f + z, -StaffHeight / 2.f),
          ThickBarlineThickness, true, false, false);
        Shapes::AddCircle(*p, Vector(-0.6f + z, 0.5f), 0.52f);
        Shapes::AddCircle(*p, Vector(-0.6f + z, -0.5f), 0.52f);
      }
      else //For as of yet unsupported features
      {
        Pointer<Path> p = new Path;
        IslandStamp->Add()->p = p;
        IslandStamp->z()->Context = Token;
        Shapes::AddLine(*p, Vector(0.f, StaffHeight / 2.f + AmountToExtend),
          Vector(0.f, -StaffHeight / 2.f),
          BarlineThickness, true, false, false);

        //Mark in red to show it is being displayed incorrectly.
        IslandStamp->z()->c = Colors::Red();
      }

      //Add artificial bounds for the barline.
      {
        IslandStamp->AddArtificialBounds(Box(
          Vector(IslandStamp->GetAbstractBounds().Left(), -3.f),
          Vector(IslandStamp->GetAbstractBounds().Right(), 3.f)));
      }
    }
  };
}
#endif
