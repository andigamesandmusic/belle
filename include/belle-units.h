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

#ifndef BELLE_CORE_UNITS_H
#define BELLE_CORE_UNITS_H

namespace BELLE_NAMESPACE
{
  ///Stores a particular unit.
  typedef count Unit;

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
    static number ConversionRatio(Unit OtherUnits)
    {
      switch(OtherUnits)
      {
        case Meter: return 1.f;
        case Millimeter: return 1.f / 1000.f;
        case Centimeter: return 1.f / 100.f;
        case Inch: return 25.4f / 1000.f;
        case Point: return 25.4f / 1000.f / 72.f;
      }
      return 1.f;
    }
  };

  class RastralSize
  {
    public:

    ///Returns the space-height in inches of a rastral size (BB483).
    static number Inches(count RastralNumber)
    {
      return Millimeters(RastralNumber) / 25.4f / 4.f;
    }

    ///Returns the space-height in millimeters of a rastral size (BB483).
    static number Millimeters(number RastralNumber)
    {
      /*BB483 gives a table of rastral sizes for integer rastral numbers 0 to 8.
      We can make this a continuous curve by fitting an 8th degree polynomial.
      "fit polynomial {{0,9.2},{1,7.9},{2,7.4},{3,7.0},{4,6.5},{5,6.0},
      {6,5.5},{7,4.8},{8,3.7}}"*/
      number x = Clip(RastralNumber, number(0), number(8));
      number r =
         -number(0.0000124008) * Power(x, number(8)) +
          number(0.000446429)  * Power(x, number(7)) -
          number(0.00635417)   * Power(x, number(6)) +
          number(0.0441667)    * Power(x, number(5)) -
          number(0.142969)     * Power(x, number(4)) +
          number(0.0864583)    * Power(x, number(3)) +
          number(0.649335)     * Power(x, number(2)) -
          number(1.93107)      * x + number(9.2);
      number t = number(1) / number(100);
      return Truncate(r + t / 2.f, t);
    }

    ///Approximates the rastral size from a given number of millimeters.
    static number Rastral(number mm)
    {
      if(mm < Millimeters(8.f))
        return 8.f;
      else if(mm > Millimeters(0.f))
        return 0.f;

      number Best_r = 0.f;
      number Difference = Millimeters(0.f);
      for(count r = 0; r <= 8 * 16; r++)
      {
        number rNumber = number(r) / 16.f;
        number Current = Abs(Millimeters(rNumber) - mm);
        if(Current < Difference)
          Difference = Current, Best_r = rNumber;
      }
      return Best_r;
    }
  };

  /**A class for storing vectors with compile-time unit information. The
  measurement class is templated to enforce typing differentiation of
  unlike units, and conversions are automatically done when measurements
  of unlike units are assigned or constructed.*/
  template <Unit T>
  struct Measurement : public Vector
  {
    ///Default constructor initializes coordinates with zeroes.
    Measurement() {}

    ///Assigns coordinates in a particular unit of measurement.
    Measurement(number x, number y)
    {
      Measurement::x = x;
      Measurement::y = y;
    }

    Measurement(const Vector& v)
    {
      Measurement::x = v.x;
      Measurement::y = v.y;
    }

    ///Creates a polar coordinate in a particular unit of measurement.
    Measurement(number Angle)
    {
      Polar(Angle);
    }

    /**Explictly converts from one measurement to another. Usually this
    is not necessary.*/
    template <Unit F>
    void ConvertFrom(const Measurement<F>& m)
    {
      //Calculate the conversion ratio.
      number Multiplier = Units::ConversionRatio(F) /
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
    Vector Pixels(number PixelsPerInch)
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
      return Inches(Max(PaperSize.x, PaperSize.y),
        Min(PaperSize.x, PaperSize.y));
    }

    ///Converts a paper size into portrait.
    static Inches Portrait(Inches PaperSize)
    {
      return Inches(Min(PaperSize.x, PaperSize.y),
        Max(PaperSize.x, PaperSize.y));
    }

    //North American paper sizes
    static Inches Letter() {return Inches(8.5f, 11.f);}
    static Inches Legal() {return Inches(8.5f, 14.f);}
    static Inches Tabloid() {return Inches(11.f, 17.f);}

    //ISO A series
    static Millimeters A0() {return Millimeters(841.f, 1189.f);}
    static Millimeters A1() {return Millimeters(594.f, 841.f);}
    static Millimeters A2() {return Millimeters(420.f, 594.f);}
    static Millimeters A3() {return Millimeters(297.f, 420.f);}
    static Millimeters A4() {return Millimeters(210.f, 297.f);}
    static Millimeters A5() {return Millimeters(148.f, 210.f);}
    static Millimeters A6() {return Millimeters(105.f, 148.f);}
    static Millimeters A7() {return Millimeters(74.f, 105.f);}
    static Millimeters A8() {return Millimeters(52.f, 74.f);}
    static Millimeters A9() {return Millimeters(37.f, 52.f);}
    static Millimeters A10() {return Millimeters(26.f, 37.f);}

    //ISO B series
    static Millimeters B0() {return Millimeters(1000.f, 1414.f);}
    static Millimeters B1() {return Millimeters(707.f, 1000.f);}
    static Millimeters B2() {return Millimeters(500.f, 707.f);}
    static Millimeters B3() {return Millimeters(353.f, 500.f);}
    static Millimeters B4() {return Millimeters(250.f, 353.f);}
    static Millimeters B5() {return Millimeters(176.f, 250.f);}
    static Millimeters B6() {return Millimeters(125.f, 176.f);}
    static Millimeters B7() {return Millimeters(88.f, 125.f);}
    static Millimeters B8() {return Millimeters(62.f, 88.f);}
    static Millimeters B9() {return Millimeters(44.f, 62.f);}
    static Millimeters B10() {return Millimeters(31.f, 44.f);}

    //ISO C series
    static Millimeters C0() {return Millimeters(917.f, 1297.f);}
    static Millimeters C1() {return Millimeters(648.f, 917.f);}
    static Millimeters C2() {return Millimeters(458.f, 648.f);}
    static Millimeters C3() {return Millimeters(324.f, 458.f);}
    static Millimeters C4() {return Millimeters(229.f, 324.f);}
    static Millimeters C5() {return Millimeters(162.f, 229.f);}
    static Millimeters C6() {return Millimeters(114.f, 162.f);}
    static Millimeters C7() {return Millimeters(81.f, 114.f);}
    static Millimeters C8() {return Millimeters(57.f, 81.f);}
    static Millimeters C9() {return Millimeters(40.f, 57.f);}
    static Millimeters C10() {return Millimeters(28.f, 40.f);}
  };
}
#endif
