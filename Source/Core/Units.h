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

#ifndef BELLE_CORE_UNITS_H
#define BELLE_CORE_UNITS_H

namespace BELLE_NAMESPACE
{
  ///Stores a particular unit.
  typedef prim::count Unit;

  ///Unit and conversion definitions structure
  struct Units
  {
    //---------------//
    //Units of Length//
    //---------------//
    
    ///The SI unit for length.
    static const Unit Meter = 0;
    
    ///One-thousandth of a meter.
    static const Unit Millimeter = 1;
    
    ///One-hundredth of a meter.
    static const Unit Centimeter = 2;
    
    ///Exactly 25.4 millimeters.
    static const Unit Inch = 3;
    
    ///Exactly 1/72 of an inch.
    static const Unit Point = 4;
    
    ///Gets the conversion ratio to the SI unit.
    static prim::number ConversionRatio(Unit OtherUnits)
    {
      switch(OtherUnits)
      {
        case Meter: return 1.0;
        case Millimeter: return 1.0 / 1000.0;
        case Centimeter: return 1.0 / 100.0;
        case Inch: return 25.4 / 1000.0;
        case Point: return 25.4 / 1000.0 / 72.0;
      }
      return 1.0;
    }
  };

  /**A class for storing vectors with compile-time unit information. The
  measurement class is templated to enforce typing differentiation of
  unlike units, and conversions are automatically done when measurements
  of unlike units are assigned or constructed.*/
  template <Unit T>
  struct Measurement : public prim::planar::Vector
  {
    ///Default constructor initializes coordinates with zeroes.
    Measurement() {}

    ///Assigns coordinates in a particular unit of measurement.
    Measurement(prim::number x, prim::number y)
    {
      Measurement::x = x;
      Measurement::y = y;
    }
    
    Measurement(const prim::planar::Vector& v)
    {
      Measurement::x = v.x;
      Measurement::y = v.y;
    }

    ///Creates a polar coordinate in a particular unit of measurement.
    Measurement(prim::number Angle)
    {
      Polar(Angle);
    }

    /**Explictly converts from one measurement to another. Usually this
    is not necessary.*/
    template <Unit F>
    void ConvertFrom(const Measurement<F>& m)
    {
      //Calculate the conversion ratio.
      prim::number Multiplier = Units::ConversionRatio(F) /
        Units::ConversionRatio(T);
      
      //Do the conversion.
      x = m.x * Multiplier;
      y = m.y * Multiplier;
    }

    ///Converts another measurement to the current unit of measurement.
    template <Unit F>
    Measurement(const Measurement<F>& m)
    {
      ConvertFrom(m);
    }
    
    /**Converts to pixels given pixels per inch. Each display has a particular
    ppi (a.k.a. dpi), but unfortunately this information is not available
    through software.*/
    prim::planar::Vector Pixels(prim::number PixelsPerInch)
    {
      return Measurement<Units::Inch>(*this) * PixelsPerInch;
    }

    ///Converts another measurement to the current unit of measurement.
    template <Unit F>
    Measurement<T> operator = (const Measurement<F>& m)
    {
      ConvertFrom(m);
      return *this;
    }
  };

  //Common measurements
  typedef Measurement<Units::Meter> Meters;
  typedef Measurement<Units::Millimeter> Millimeters;
  typedef Measurement<Units::Centimeter> Centimeters;
  typedef Measurement<Units::Inch> Inches;
  typedef Measurement<Units::Point> Points;
  
  ///Enumeration of standard paper sizes.
  struct Paper
  {
    ///Converts a paper size into landscape.
    static Inches Landscape(Inches PaperSize)
    {
      return Inches(prim::Max(PaperSize.x, PaperSize.y),
        prim::Min(PaperSize.x, PaperSize.y));
    }
    
    ///Converts a paper size into portrait.
    static Inches Portrait(Inches PaperSize)
    {
      return Inches(prim::Min(PaperSize.x, PaperSize.y),
        prim::Max(PaperSize.x, PaperSize.y));
    }
    
    //North American paper sizes
    static const Inches Letter;
    static const Inches Legal;
    static const Inches Tabloid;
    
    //ISO A series
    static const Millimeters A0;
    static const Millimeters A1;
    static const Millimeters A2;
    static const Millimeters A3;
    static const Millimeters A4;
    static const Millimeters A5;
    static const Millimeters A6;
    static const Millimeters A7;
    static const Millimeters A8;
    static const Millimeters A9;
    static const Millimeters A10;
    
    //ISO B series
    static const Millimeters B0;
    static const Millimeters B1;
    static const Millimeters B2;
    static const Millimeters B3;
    static const Millimeters B4;
    static const Millimeters B5;
    static const Millimeters B6;
    static const Millimeters B7;
    static const Millimeters B8;
    static const Millimeters B9;
    static const Millimeters B10;
    
    //ISO C series
    static const Millimeters C0;
    static const Millimeters C1;
    static const Millimeters C2;
    static const Millimeters C3;
    static const Millimeters C4;
    static const Millimeters C5;
    static const Millimeters C6;
    static const Millimeters C7;
    static const Millimeters C8;
    static const Millimeters C9;
    static const Millimeters C10;
  };
  
#ifdef BELLE_COMPILE_INLINE
  const Inches Paper::Letter(8.5, 11.0);
  const Inches Paper::Legal(8.5, 14.0);
  const Inches Paper::Tabloid(11.0, 17.0);
  
  const Millimeters Paper::A0(841., 1189.);
  const Millimeters Paper::A1(594., 841.);
  const Millimeters Paper::A2(420., 594.);
  const Millimeters Paper::A3(297., 420.);
  const Millimeters Paper::A4(210., 297.);
  const Millimeters Paper::A5(148., 210.);
  const Millimeters Paper::A6(105., 148.);
  const Millimeters Paper::A7(74., 105.);
  const Millimeters Paper::A8(52., 74.);
  const Millimeters Paper::A9(37., 52.);
  const Millimeters Paper::A10(26., 37.);
  
  const Millimeters Paper::B0(1000., 1414.);
  const Millimeters Paper::B1(707., 1000.);
  const Millimeters Paper::B2(500., 707.);
  const Millimeters Paper::B3(353., 500.);
  const Millimeters Paper::B4(250., 353.);
  const Millimeters Paper::B5(176., 250.);
  const Millimeters Paper::B6(125., 176.);
  const Millimeters Paper::B7(88., 125.);
  const Millimeters Paper::B8(62., 88.);
  const Millimeters Paper::B9(44., 62.);
  const Millimeters Paper::B10(31., 44.);
  
  const Millimeters Paper::C0(917., 1297);
  const Millimeters Paper::C1(648., 917.);
  const Millimeters Paper::C2(458., 648.);
  const Millimeters Paper::C3(324., 458.);
  const Millimeters Paper::C4(229., 324.);
  const Millimeters Paper::C5(162., 229.);
  const Millimeters Paper::C6(114., 162.);
  const Millimeters Paper::C7(81., 114.);
  const Millimeters Paper::C8(57., 81.);
  const Millimeters Paper::C9(40., 57.);
  const Millimeters Paper::C10(28., 40.);
#endif
}
#endif
