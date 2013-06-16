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

#ifndef BELLE_CORE_COLORS_H
#define BELLE_CORE_COLORS_H

namespace BELLE_NAMESPACE {

  //Forward declarations
  struct Color;

  struct ColorModels
  {
    struct sRGB;
    struct AdobeRGB;
    struct CIEXYZ;
    struct CIExyY;
    
    ///Represents an exact color in the AdobeRGB color profile.
    struct AdobeRGB
    {
      prim::float32 R, G, B;
      
      AdobeRGB(prim::float32 R = 0.5f, prim::float32 G = 0.5f,
        prim::float32 B = 0.5f) : R(R), G(G), B(B) {}
      
      operator ColorModels::CIEXYZ ();
      operator ColorModels::CIExyY ();
      operator ColorModels::sRGB ();
      operator Color ();
    };
  
    ///Represents an exact color in the CIE xyY color space.
    struct CIExyY
    {
      prim::float32 x, y, Y;
      
      CIExyY(prim::float32 x = 0.3f, prim::float32 y = 0.3f,
        prim::float32 Y = 0.3f) : x(x), y(y), Y(Y) {}
  
      operator ColorModels::CIEXYZ ();
      operator ColorModels::sRGB ();
      operator ColorModels::AdobeRGB ();
      operator Color ();
    };
  
    ///Represents an exact color in the CIE XYZ color space.
    struct CIEXYZ
    {
      prim::float32 X, Y, Z;
      
      CIEXYZ(prim::float32 X = 0.3f, prim::float32 Y = 0.3f,
        prim::float32 Z = 0.3f) : X(X), Y(Y), Z(Z) {}
      
      operator ColorModels::CIExyY ();
      operator ColorModels::sRGB ();
      operator ColorModels::AdobeRGB ();
      operator Color ();
    };
    
    ///Represents an exact color in the sRGB profile.
    struct sRGB
    {
      prim::float32 R, G, B;
      
      sRGB(prim::float32 R = 0.5f, prim::float32 G = 0.5f,
        prim::float32 B = 0.5f) : R(R), G(G), B(B) {}
        
      ///Searches the system for the sRGB profile and returns it if possible.
      static prim::String LookForProfile()
      {
        prim::String ICC;
        if(prim::Environment::Apple())
          prim::File::Read("/System/Library/ColorSync/Profiles/"
            "sRGB Profile.icc", ICC);
        else if(prim::Environment::POSIX())
          prim::File::Read("/usr/share/color/icc/sRGB.icc", ICC);
        else if(prim::Environment::Windows())
          prim::File::Read("\\Windows\\System32\\Spool\\Drivers\\Color\\"
            "sRGB Color Space Profile.icc", ICC);
        return ICC;
      }
      
      operator ColorModels::CIEXYZ ();
      operator ColorModels::CIExyY ();
      operator ColorModels::AdobeRGB ();
    };
    
    ///Represents a device-dependent CMYK color.
    struct CMYK
    {
      prim::float32 C, M, Y, K;
      CMYK(prim::float32 C = 0.5f, prim::float32 M = 0.5f,
        prim::float32 Y = 0.5f, prim::float32 K = 0.5f) : C(C), M(M), Y(Y), K(K)
        {}
    };
    
    ///Represents a device-dependent RGB color.
    struct RGB
    {
      prim::float32 R, G, B;
      RGB(prim::float32 R = 0.5f, prim::float32 G = 0.5f,
        prim::float32 B = 0.5f) : R(R), G(G), B(B) {}
    };
  };

  /**An exact color. This class is an attempt to incorporate color management in
  the library. Color management is important for most file-based graphics
  formats. For example, all raster images (PNG, TIFF, etc.) allow color profiles
  to be stored. PDF requires the color profile mode to be specified. Obviously,
  it is possible to ignore the issue altogether by storing device-dependent
  colors, but where possible, a user should make an attempt to use color
  management so that colors are accurately stored and transmitted.

  There are three main groups of colors models: RGB, CMYK, and XYZ. RGB stores
  reds, greens, and blues which is useful for displays. CMYK stores cyan,
  magenta, yellow, and black which is useful for print. XYZ (also xyY, a
  derivative) stores color in terms of chromaticity, XZ or xy, and luminance Y.

  The goal of color managment is consistent color reproduction and perceptual
  color matching. For chromaticity, this is essentially defined by the
  wavelengths of the incoming light. Luminance is considered relative to a white
  point (brightest point). So XYZ is not totally absolute from a perceptual
  standpoint. The reason for this is that it is that for reflective media, such
  as print, the environent lighting affects the brightness and to some degree
  the color temperature of the perceived result. Therefore by making Y relative
  to a white point, it is much easier to compare the colors of media. With
  displays too, the brightness can often be adjusted within a large range, so it
  is up to the user to match white points when comparing colors from different
  outputs.

  In recreating a color, the idea is to use a standardized space from which
  colors are translated into and out of. In this library there are four color
  models that specify device-independent color: sRGB, AdobeRGB, CIEXYZ, CIExyY.
  In most situations sRGB or AdobeRGB are the most appropriate.

  So as an example, if you create an sRGB color (0.1, 0.3, 0.7), then if that
  media is saved with the sRGB profile, then that color will always be the same
  on any calibrated screen which uses color management.

  AdobeRGB is similar to sRGB except it has a larger gamut. A larger gamut means
  deeper colors can be represented. However, it is up to any given display to
  actually acheive them. Moreover, due to the larger gamut, more precision is
  necessary to represent color differences from the sRGB gamut. This is not as
  much of a problem in this library since floating-point color values are used,
  but it may be worth considering whether the destination medium is 8-bits or
  16-bits per channel. A general rule of thumb is if you have 8-bit channels,
  use sRGB, and if you have 16-bit channels use AdobeRGB. sRGB is the de facto
  default as far as displays are concerned.

  The CIEXYZ and CIExyY models are capable of storing any color that is
  perceivable to a human. However, only a small portion of the range is
  attainable on most display and print media. Therefore, these color models are
  mostly of theoretical interest. Remember that the choice of color model has
  more to do with matching the range of the output device. Colors are precise no
  matter which model is chosen so long as they are within gamut.

  There are also two device-dependent (or profile-dependent) color models: RGB
  and CMYK. These are essentially raw device-values which will be translated (in
  the absence of a profile) into whatever the hardware is capable of acheiving.
  For example, an RGB of (1.0, 0.0, 0.0) on a display will mean to use the
  brightest red available. However, it is not particularly clear what color in
  the device-independent spaces it will be. Device dependent values are useful
  for utilizing the maximum range of a given device, but no precise color is
  then guaranteed.

  On the other hand, you may have a specific profile in mind for a certain
  hardware device, and you would like to access the full range of the profile.
  In this case, to, you would use either RGB or CMYK and then in the settings of
  the output file tell it that the RGB or CMYK values you are specifying ought
  to be interpretted as from a particular profile.

  Note that since RGB is device-dependent and CMYK is device-dependent there is
  no mapping between the two. (Conceptually, R=M+Y, G=C+Y, B=M+C, but in reality
  the gamut difference between the two is so large that any formula such as this
  will be completely inaccurate.) However, you could map two device-independent
  profiles to one another, i.e. sRGB to Specific Printer (TM) CMYK profile.

  Note that when a device-dependent color encounters a device-independent
  profile, the result is the values being mapped directly onto the profile, as
  though the values were originally meant for that color space. For example, an
  image with device-dependent RGB input to sRGB will look different from device-
  dependent RGB to AdobeRGB. However, if the sRGB image is displayed on an
  AdobeRGB monitor, there will be no difference because color management will
  automatically translate to the correct colors.

  All of the device-independent color model classes (AdobeRGB, sRGB, CIEXYZ,
  CIExyY) are interchangeable. They will convert to and from each other. The
  Color class is simply a wrapper for sRGB (since that is the closest to most
  displays), though it can also represent colors outside of its gamut (which
  will simply be out of the normal bounds of 0 to 1).*/
  struct Color : public ColorModels::sRGB
  {
    ///Alpha value for transparency when this information is used.
    prim::float32 A;
    
    ///Default constructor creates the color black.
    Color() : ColorModels::sRGB(0.f, 0.f, 0.f), A(1.f) {}
    
    ///Constructor to supply sRGB normalized values.
    Color(prim::float32 sRGBRed, prim::float32 sRGBGreen,
      prim::float32 sRGBBlue) : ColorModels::sRGB(sRGBRed, sRGBGreen, sRGBBlue), A(1.f) {}
      
    ///Constructor to supply sRGB normalized values with alpha.
    Color(prim::float32 sRGBRed, prim::float32 sRGBGreen,
      prim::float32 sRGBBlue, prim::float32 Alpha) : ColorModels::sRGB(sRGBRed, sRGBGreen,
      sRGBBlue), A(Alpha) {}
      
    ///Constructor to supply sRGB normalized values.
    Color(prim::float64 sRGBRed, prim::float64 sRGBGreen,
      prim::float64 sRGBBlue) : ColorModels::sRGB((prim::float32)sRGBRed,
      (prim::float32)sRGBGreen, (prim::float32)sRGBBlue), A(1.f) {}
      
    ///Constructor to supply sRGB normalized values with alpha.
    Color(prim::float64 sRGBRed, prim::float64 sRGBGreen,
      prim::float64 sRGBBlue, prim::float64 Alpha) :
      ColorModels::sRGB((prim::float32)sRGBRed, (prim::float32)sRGBGreen,
      (prim::float32)sRGBBlue), A((prim::float32)Alpha) {}
    
    ///Constructor to supply sRGB byte values.
    Color(prim::uint8 sRGBRed, prim::uint8 sRGBGreen, prim::uint8 sRGBBlue) :
      ColorModels::sRGB((prim::float32)sRGBRed / 255.f, (prim::float32)sRGBGreen / 255.f,
      (prim::float32)sRGBBlue / 255.f), A(1.f) {}
      
    ///Constructor to supply sRGB byte values.
    Color(prim::int16 sRGBRed, prim::int16 sRGBGreen, prim::int16 sRGBBlue) :
      ColorModels::sRGB((prim::float32)sRGBRed / 255.f,
      (prim::float32)sRGBGreen / 255.f, (prim::float32)sRGBBlue / 255.f), A(1.f)
      {}
    
    ///Constructor to supply sRGB byte values.
    Color(prim::int32 sRGBRed, prim::int32 sRGBGreen, prim::int32 sRGBBlue) :
      ColorModels::sRGB((prim::float32)sRGBRed / 255.f, (prim::float32)sRGBGreen / 255.f,
      (prim::float32)sRGBBlue / 255.f), A(1.f) {}
      
    ///Constructor to supply sRGB byte values.
    Color(prim::int64 sRGBRed, prim::int64 sRGBGreen, prim::int64 sRGBBlue) :
      ColorModels::sRGB((prim::float32)sRGBRed / 255.f, (prim::float32)sRGBGreen / 255.f,
      (prim::float32)sRGBBlue / 255.f), A(1.f) {}
      
    /**Constructor to supply sRGB 2-byte values. Note this will only be called
    if uint16 values are explicitly given.*/
    Color(prim::uint16 sRGBRed, prim::uint16 sRGBGreen, prim::uint16 sRGBBlue) :
      ColorModels::sRGB((prim::float32)sRGBRed / 65535.f,
      (prim::float32)sRGBGreen / 65535.f, (prim::float32)sRGBBlue / 65535.f),
      A(1.f) {}

    ///Constructor from sRGB color
    Color(const sRGB& sRGBColor) : ColorModels::sRGB(sRGBColor), A(1.f) {}
    
    /*Interprets color as a raw device-dependent RGB color. Note that by doing
    this the only way to recover the exact colors is to apply the sRGB profile
    to the output.*/
    operator ColorModels::RGB ();
    
    ///Operator equals
    bool operator == (const Color& Other)
    {
      return R == Other.R && G == Other.G && B == Other.B && A == Other.A;
    }

    ///Operator not-equals
    bool operator != (const Color& Other)
    {
      return !(*this == Other);
    }
  };
  
  /**A listing of SVG and web colors from. The source of the data is from:
  http://www.w3.org/TR/SVG/types.html#ColorKeywords . Note that the names have
  been left lowercase so as not to break convention (think of them as namespace
  case). These are exact colors as they have been specified according to the
  sRGB color profile.*/
  struct Colors
  {
    static const Color Empty;
    static const Color aliceblue;
    static const Color antiquewhite;
    static const Color aqua;
    static const Color aquamarine;
    static const Color azure;
    static const Color beige;
    static const Color bisque;
    static const Color black;
    static const Color blanchedalmond;
    static const Color blue;
    static const Color blueviolet;
    static const Color brown;
    static const Color burlywood;
    static const Color cadetblue;
    static const Color chartreuse;
    static const Color chocolate;
    static const Color coral;
    static const Color cornflowerblue;
    static const Color cornsilk;
    static const Color crimson;
    static const Color cyan;
    static const Color darkblue;
    static const Color darkcyan;
    static const Color darkgoldenrod;
    static const Color darkgray;
    static const Color darkgreen;
    static const Color darkgrey;
    static const Color darkkhaki;
    static const Color darkmagenta;
    static const Color darkolivegreen;
    static const Color darkorange;
    static const Color darkorchid;
    static const Color darkred;
    static const Color darksalmon;
    static const Color darkseagreen;
    static const Color darkslateblue;
    static const Color darkslategray;
    static const Color darkslategrey;
    static const Color darkturquoise;
    static const Color darkviolet;
    static const Color deeppink;
    static const Color deepskyblue;
    static const Color dimgray;
    static const Color dimgrey;
    static const Color dodgerblue;
    static const Color firebrick;
    static const Color floralwhite;
    static const Color forestgreen;
    static const Color fuchsia;
    static const Color gainsboro;
    static const Color ghostwhite;
    static const Color gold;
    static const Color goldenrod;
    static const Color gray;
    static const Color grey;
    static const Color green;
    static const Color greenyellow;
    static const Color honeydew;
    static const Color hotpink;
    static const Color indianred;
    static const Color indigo;
    static const Color ivory;
    static const Color khaki;
    static const Color lavender;
    static const Color lavenderblush;
    static const Color lawngreen;
    static const Color lemonchiffon;
    static const Color lightblue;
    static const Color lightcoral;
    static const Color lightcyan;
    static const Color lightgoldenrodyellow;
    static const Color lightgray;
    static const Color lightgreen;
    static const Color lightgrey;
    static const Color lightpink;
    static const Color lightsalmon;
    static const Color lightseagreen;
    static const Color lightskyblue;
    static const Color lightslategray;
    static const Color lightslategrey;
    static const Color lightsteelblue;
    static const Color lightyellow;
    static const Color lime;
    static const Color limegreen;
    static const Color linen;
    static const Color magenta;
    static const Color maroon;
    static const Color mediumaquamarine;
    static const Color mediumblue;
    static const Color mediumorchid;
    static const Color mediumpurple;
    static const Color mediumseagreen;
    static const Color mediumslateblue;
    static const Color mediumspringgreen;
    static const Color mediumturquoise;
    static const Color mediumvioletred;
    static const Color midnightblue;
    static const Color mintcream;
    static const Color mistyrose;
    static const Color moccasin;
    static const Color navajowhite;
    static const Color navy;
    static const Color oldlace;
    static const Color olive;
    static const Color olivedrab;
    static const Color orange;
    static const Color orangered;
    static const Color orchid;
    static const Color palegoldenrod;
    static const Color palegreen;
    static const Color paleturquoise;
    static const Color palevioletred;
    static const Color papayawhip;
    static const Color peachpuff;
    static const Color peru;
    static const Color pink;
    static const Color plum;
    static const Color powderblue;
    static const Color purple;
    static const Color red;
    static const Color rosybrown;
    static const Color royalblue;
    static const Color saddlebrown;
    static const Color salmon;
    static const Color sandybrown;
    static const Color seagreen;
    static const Color seashell;
    static const Color sienna;
    static const Color silver;
    static const Color skyblue;
    static const Color slateblue;
    static const Color slategray;
    static const Color slategrey;
    static const Color snow;
    static const Color springgreen;
    static const Color steelblue;
    static const Color tan;
    static const Color teal;
    static const Color thistle;
    static const Color tomato;
    static const Color turquoise;
    static const Color violet;
    static const Color wheat;
    static const Color white;
    static const Color whitesmoke;
    static const Color yellow;
    static const Color yellowgreen;
  };
  
#ifdef BELLE_COMPILE_INLINE
  Color::operator ColorModels::RGB ()
  {
    //sRGB --> raw RGB//
    return ColorModels::RGB(R, G, B);
  }

  ColorModels::AdobeRGB::operator ColorModels::CIEXYZ ()
  {
    //AdobeRGB --> CIEXYZ//
    return ColorModels::CIEXYZ(
      0.57667f * R + 0.18556f * G + 0.18823f * B,
      0.29734f * R + 0.62736f * G + 0.07529f * B,
      0.02703f * R + 0.07069f * G + 0.99134f * B);
  };

  ColorModels::AdobeRGB::operator ColorModels::sRGB ()
  {
    //AdobeRGB --> CIEXYZ --> sRGB//
    return (ColorModels::sRGB)(ColorModels::CIEXYZ)(*this);
  }

  ColorModels::AdobeRGB::operator ColorModels::CIExyY ()
  {
    //AdobeRGB --> CIEXYZ --> CIExyY//
    return (ColorModels::CIExyY)(ColorModels::CIEXYZ)(*this);
  }
  
  ColorModels::AdobeRGB::operator Color ()
  {
    //AdobeRGB --> sRGB --> Color (sRGB)//
    return Color((ColorModels::sRGB)(*this));
  }

  ColorModels::sRGB::operator ColorModels::CIEXYZ ()
  {
    //sRGB --> CIEXYZ//
    
    //From: http://en.wikipedia.org/wiki/SRGB_color_space (reverse transform)
    return ColorModels::CIEXYZ(
      0.4124f * R + 0.3576f * G + 0.1805f * B,
      0.2126f * R + 0.7152f * G + 0.0722f * B,
      0.0193f * R + 0.1192f * G + 0.9505f * B);
  }

  ColorModels::sRGB::operator ColorModels::CIExyY ()
  {
    //sRGB --> CIEXYZ --> CIExyY//
    return (ColorModels::CIExyY)(ColorModels::CIEXYZ)(*this);
  }

  ColorModels::sRGB::operator ColorModels::AdobeRGB ()
  {
    //sRGB --> CIEXYZ --> AdobeRGB//
    return (AdobeRGB)(CIEXYZ)(*this);
  }

  ColorModels::CIEXYZ::operator ColorModels::CIExyY ()
  {
    //CIEXYZ --> CIExyY//
    
    //From: http://en.wikipedia.org/wiki/CIE_xyY (CIE xyY color space)
    return ColorModels::CIExyY(X / (X + Y + Z + 0.000001f), Y / (X + Y + Z + 0.000001f), Y);
  }

  ColorModels::CIEXYZ::operator ColorModels::sRGB ()
  {
    //CIEXYZ --> sRGB//
    
    //From: http://en.wikipedia.org/wiki/SRGB_color_space (forward transform)
    return ColorModels::sRGB(
      3.2406f * X - 1.5372f * Y - 0.4986f * Z,
     -0.9689f * X + 1.8758f * Y + 0.0415f * Z,
      0.0557f * X - 0.2040f * Y + 1.0570f * Z);
  }

  ColorModels::CIEXYZ::operator ColorModels::AdobeRGB ()
  {
    //CIEXYZ --> AdobeRGB//
    
    //From: http://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf
    return ColorModels::AdobeRGB(
      2.04159f * X - 0.56501f * Y - 0.34473f * Z,
     -0.96924f * X + 1.87597f * Y + 0.04156f * Z,
      0.01344f * X - 0.11836f * Y + 1.01517f * Z);
  }
  
  ColorModels::CIEXYZ::operator Color ()
  {
    //CIEXYZ --> sRGB --> Color (sRGB)//
    return Color((ColorModels::sRGB)(*this));
  }

  ColorModels::CIExyY::operator ColorModels::CIEXYZ ()
  {
    //CIExyY --> CIEXYZ//
    
    //From: http://en.wikipedia.org/wiki/CIE_xyY (CIE xyY color space)
    return ColorModels::CIEXYZ(Y * x / (y + 0.000001f), Y, Y * (1 - x - y) /
      (y + 0.000001f));
  }

  ColorModels::CIExyY::operator ColorModels::sRGB ()
  {
    //CIExyY --> CIEXYZ --> sRGB//
    return (ColorModels::sRGB)(ColorModels::CIEXYZ)(*this);
  }

  ColorModels::CIExyY::operator ColorModels::AdobeRGB ()
  {
    //CIExyY --> CIEXYZ --> AdobeRGB//
    return (ColorModels::AdobeRGB)(ColorModels::CIEXYZ)(*this);
  }
  
  ColorModels::CIExyY::operator Color ()
  {
    //CIExyY --> sRGB --> Color (sRGB)//
    return Color((ColorModels::sRGB)(*this));
  }
  
  const Color Colors::Empty(0.f, 0.f, 0.f, 0.f);
  const Color Colors::aliceblue(240, 248, 255);
  const Color Colors::antiquewhite(250, 235, 215);
  const Color Colors::aqua(0, 255, 255);
  const Color Colors::aquamarine(127, 255, 212);
  const Color Colors::azure(240, 255, 255);
  const Color Colors::beige(245, 245, 220);
  const Color Colors::bisque(255, 228, 196);
  const Color Colors::black(0, 0, 0);
  const Color Colors::blanchedalmond(255, 235, 205);
  const Color Colors::blue(0, 0, 255);
  const Color Colors::blueviolet(138, 43, 226);
  const Color Colors::brown(165, 42, 42);
  const Color Colors::burlywood(222, 184, 135);
  const Color Colors::cadetblue(95, 158, 160);
  const Color Colors::chartreuse(127, 255, 0);
  const Color Colors::chocolate(210, 105, 30);
  const Color Colors::coral(255, 127, 80);
  const Color Colors::cornflowerblue(100, 149, 237);
  const Color Colors::cornsilk(255, 248, 220);
  const Color Colors::crimson(220, 20, 60);
  const Color Colors::cyan(0, 255, 255);
  const Color Colors::darkblue(0, 0, 139);
  const Color Colors::darkcyan(0, 139, 139);
  const Color Colors::darkgoldenrod(184, 134, 11);
  const Color Colors::darkgray(169, 169, 169);
  const Color Colors::darkgreen(0, 100, 0);
  const Color Colors::darkgrey(169, 169, 169);
  const Color Colors::darkkhaki(189, 183, 107);
  const Color Colors::darkmagenta(139, 0, 139);
  const Color Colors::darkolivegreen(85, 107, 47);
  const Color Colors::darkorange(255, 140, 0);
  const Color Colors::darkorchid(153, 50, 204);
  const Color Colors::darkred(139, 0, 0);
  const Color Colors::darksalmon(233, 150, 122);
  const Color Colors::darkseagreen(143, 188, 143);
  const Color Colors::darkslateblue(72, 61, 139);
  const Color Colors::darkslategray(47, 79, 79);
  const Color Colors::darkslategrey(47, 79, 79);
  const Color Colors::darkturquoise(0, 206, 209);
  const Color Colors::darkviolet(148, 0, 211);
  const Color Colors::deeppink(255, 20, 147);
  const Color Colors::deepskyblue(0, 191, 255);
  const Color Colors::dimgray(105, 105, 105);
  const Color Colors::dimgrey(105, 105, 105);
  const Color Colors::dodgerblue(30, 144, 255);
  const Color Colors::firebrick(178, 34, 34);
  const Color Colors::floralwhite(255, 250, 240);
  const Color Colors::forestgreen(34, 139, 34);
  const Color Colors::fuchsia(255, 0, 255);
  const Color Colors::gainsboro(220, 220, 220);
  const Color Colors::ghostwhite(248, 248, 255);
  const Color Colors::gold(255, 215, 0);
  const Color Colors::goldenrod(218, 165, 32);
  const Color Colors::gray(128, 128, 128);
  const Color Colors::grey(128, 128, 128);
  const Color Colors::green(0, 128, 0);
  const Color Colors::greenyellow(173, 255, 47);
  const Color Colors::honeydew(240, 255, 240);
  const Color Colors::hotpink(255, 105, 180);
  const Color Colors::indianred(205, 92, 92);
  const Color Colors::indigo(75, 0, 130);
  const Color Colors::ivory(255, 255, 240);
  const Color Colors::khaki(240, 230, 140);
  const Color Colors::lavender(230, 230, 250);
  const Color Colors::lavenderblush(255, 240, 245);
  const Color Colors::lawngreen(124, 252, 0);
  const Color Colors::lemonchiffon(255, 250, 205);
  const Color Colors::lightblue(173, 216, 230);
  const Color Colors::lightcoral(240, 128, 128);
  const Color Colors::lightcyan(224, 255, 255);
  const Color Colors::lightgoldenrodyellow(250, 250, 210);
  const Color Colors::lightgray(211, 211, 211);
  const Color Colors::lightgreen(144, 238, 144);
  const Color Colors::lightgrey(211, 211, 211);
  const Color Colors::lightpink(255, 182, 193);
  const Color Colors::lightsalmon(255, 160, 122);
  const Color Colors::lightseagreen(32, 178, 170);
  const Color Colors::lightskyblue(135, 206, 250);
  const Color Colors::lightslategray(119, 136, 153);
  const Color Colors::lightslategrey(119, 136, 153);
  const Color Colors::lightsteelblue(176, 196, 222);
  const Color Colors::lightyellow(255, 255, 224);
  const Color Colors::lime(0, 255, 0);
  const Color Colors::limegreen(50, 205, 50);
  const Color Colors::linen(250, 240, 230);
  const Color Colors::magenta(255, 0, 255);
  const Color Colors::maroon(128, 0, 0);
  const Color Colors::mediumaquamarine(102, 205, 170);
  const Color Colors::mediumblue(0, 0, 205);
  const Color Colors::mediumorchid(186, 85, 211);
  const Color Colors::mediumpurple(147, 112, 219);
  const Color Colors::mediumseagreen(60, 179, 113);
  const Color Colors::mediumslateblue(123, 104, 238);
  const Color Colors::mediumspringgreen(0, 250, 154);
  const Color Colors::mediumturquoise(72, 209, 204);
  const Color Colors::mediumvioletred(199, 21, 133);
  const Color Colors::midnightblue(25, 25, 112);
  const Color Colors::mintcream(245, 255, 250);
  const Color Colors::mistyrose(255, 228, 225);
  const Color Colors::moccasin(255, 228, 181);
  const Color Colors::navajowhite(255, 222, 173);
  const Color Colors::navy(0, 0, 128);
  const Color Colors::oldlace(253, 245, 230);
  const Color Colors::olive(128, 128, 0);
  const Color Colors::olivedrab(107, 142, 35);
  const Color Colors::orange(255, 165, 0);
  const Color Colors::orangered(255, 69, 0);
  const Color Colors::orchid(218, 112, 214);
  const Color Colors::palegoldenrod(238, 232, 170);
  const Color Colors::palegreen(152, 251, 152);
  const Color Colors::paleturquoise(175, 238, 238);
  const Color Colors::palevioletred(219, 112, 147);
  const Color Colors::papayawhip(255, 239, 213);
  const Color Colors::peachpuff(255, 218, 185);
  const Color Colors::peru(205, 133, 63);
  const Color Colors::pink(255, 192, 203);
  const Color Colors::plum(221, 160, 221);
  const Color Colors::powderblue(176, 224, 230);
  const Color Colors::purple(128, 0, 128);
  const Color Colors::red(255, 0, 0);
  const Color Colors::rosybrown(188, 143, 143);
  const Color Colors::royalblue(65, 105, 225);
  const Color Colors::saddlebrown(139, 69, 19);
  const Color Colors::salmon(250, 128, 114);
  const Color Colors::sandybrown(244, 164, 96);
  const Color Colors::seagreen(46, 139, 87);
  const Color Colors::seashell(255, 245, 238);
  const Color Colors::sienna(160, 82, 45);
  const Color Colors::silver(192, 192, 192);
  const Color Colors::skyblue(135, 206, 235);
  const Color Colors::slateblue(106, 90, 205);
  const Color Colors::slategray(112, 128, 144);
  const Color Colors::slategrey(112, 128, 144);
  const Color Colors::snow(255, 250, 250);
  const Color Colors::springgreen(0, 255, 127);
  const Color Colors::steelblue(70, 130, 180);
  const Color Colors::tan(210, 180, 140);
  const Color Colors::teal(0, 128, 128);
  const Color Colors::thistle(216, 191, 216);
  const Color Colors::tomato(255, 99, 71);
  const Color Colors::turquoise(64, 224, 208);
  const Color Colors::violet(238, 130, 238);
  const Color Colors::wheat(245, 222, 179);
  const Color Colors::white(255, 255, 255);
  const Color Colors::whitesmoke(245, 245, 245);
  const Color Colors::yellow(255, 255, 0);
  const Color Colors::yellowgreen(154, 205, 50);
#endif
}
#endif
