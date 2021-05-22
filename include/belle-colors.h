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
      float32 R, G, B;

      AdobeRGB(float32 R_ = 0.5f, float32 G_ = 0.5f,
        float32 B_ = 0.5f) : R(R_), G(G_), B(B_) {}

      operator ColorModels::CIEXYZ ();
      operator ColorModels::CIExyY ();
      operator ColorModels::sRGB ();
      operator Color ();
    };

    ///Represents an exact color in the CIE xyY color space.
    struct CIExyY
    {
      float32 x, y, Y;

      CIExyY(float32 x_ = 0.3f, float32 y_ = 0.3f,
        float32 Y_ = 0.3f) : x(x_), y(y_), Y(Y_) {}

      operator ColorModels::CIEXYZ ();
      operator ColorModels::sRGB ();
      operator ColorModels::AdobeRGB ();
      operator Color ();
    };

    ///Represents an exact color in the CIE XYZ color space.
    struct CIEXYZ
    {
      float32 X, Y, Z;

      CIEXYZ(float32 X_ = 0.3f, float32 Y_ = 0.3f,
        float32 Z_ = 0.3f) : X(X_), Y(Y_), Z(Z_) {}

      operator ColorModels::CIExyY ();
      operator ColorModels::sRGB ();
      operator ColorModels::AdobeRGB ();
      operator Color ();
    };

    ///Represents an exact color in the sRGB profile.
    struct sRGB
    {
      float32 R, G, B;

      sRGB(float32 R_ = 0.5f, float32 G_ = 0.5f,
        float32 B_ = 0.5f) : R(R_), G(G_), B(B_) {}

      sRGB(const sRGB& Other)
      {
        R = Other.R;
        G = Other.G;
        B = Other.B;
      }

      sRGB& operator = (const sRGB& Other)
      {
        R = Other.R;
        G = Other.G;
        B = Other.B;
        return *this;
      }

      ///Searches the system for the sRGB profile and returns it if possible.
      static String LookForProfile()
      {
        String ICC;
        if(Environment::Apple())
          File::Read("/System/Library/ColorSync/Profiles/"
            "sRGB Profile.icc", ICC);
        else if(Environment::UnixLike())
          File::Read("/usr/share/color/icc/sRGB.icc", ICC);
        else if(Environment::Windows())
          File::Read("\\Windows\\System32\\Spool\\Drivers\\Color\\"
            "sRGB Color Space Profile.icc", ICC);
        return ICC;
      }

      operator ColorModels::CIEXYZ ();
      operator ColorModels::CIExyY ();
      operator ColorModels::AdobeRGB ();
    };

    ///Represents a device-dependent CMYK color.
    struct CMYKColor
    {
      float32 C, M, Y, K;
      CMYKColor(float32 C_ = 0.5f, float32 M_ = 0.5f,
        float32 Y_ = 0.5f, float32 K_ = 0.5f) : C(C_), M(M_), Y(Y_), K(K_)
        {}
    };

    ///Represents a device-dependent RGB color.
    struct RGBColor
    {
      float32 R, G, B;
      RGBColor(float32 R_ = 0.5f, float32 G_ = 0.5f,
        float32 B_ = 0.5f) : R(R_), G(G_), B(B_) {}
    };
  };

  /**An exact color. This class is an attempt to incorporate color management in
  the library. Color management is important for most file-based graphics
  formats. For example, many raster files (PNG, TIFF, etc.) allow color profiles
  to be stored. PDF requires the color profile mode to be specified. Obviously,
  it is possible to ignore the issue altogether by storing device-dependent
  colors, but where possible, a user should make an attempt to use color
  management so that colors are accurately stored and transmitted.

  There are three main groups of colors models: RGB, CMYK, and XYZ. RGB stores
  reds, greens, and blues which is useful for displays. CMYK stores cyan,
  magenta, yellow, and black which is useful for print. XYZ (also xyY, a
  derivative) stores color in terms of chromaticity, XZ or xy, and luminance Y.

  The goal of color management is consistent color reproduction and perceptual
  color matching. For chromaticity, this is essentially defined by the
  wavelengths of the incoming light. Luminance is considered relative to a white
  point (brightest point). So XYZ is not totally absolute from a perceptual
  standpoint. The reason for this is that for reflective media, such as print,
  the environment lighting affects the brightness, and to some degree, the
  color temperature of the perceived result. Therefore by making Y relative
  to a white point, it is much easier to compare the colors of media. With
  displays too, the brightness can often be adjusted within a large range, so it
  is up to the user to match white points when comparing colors from different
  outputs.

  In recreating a color, the idea is to use a standardized space from which
  colors are translated in to and out of. In this library there are four color
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
  16-bits per channel. A general rule of thumb is that if you have 8-bit
  channels, use sRGB, and if you have 16-bit channels use AdobeRGB. sRGB is the
  de facto default as far as displays are concerned.

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
    float32 A;

    ///Default constructor creates the color black.
    Color() : ColorModels::sRGB(0.f, 0.f, 0.f), A(1.f) {}

    ///Constructor to supply sRGB normalized values.
    Color(float32 sRGBRed, float32 sRGBGreen,
      float32 sRGBBlue) : ColorModels::sRGB(sRGBRed, sRGBGreen, sRGBBlue),
      A(1.f) {}

    ///Constructor to supply sRGB normalized values with alpha.
    Color(float32 sRGBRed, float32 sRGBGreen,
      float32 sRGBBlue, float32 Alpha) : ColorModels::sRGB(sRGBRed,
      sRGBGreen, sRGBBlue), A(Alpha) {}

    ///Constructor to supply sRGB normalized values.
    Color(float64 sRGBRed, float64 sRGBGreen,
      float64 sRGBBlue) : ColorModels::sRGB(float32(sRGBRed),
      float32(sRGBGreen), float32(sRGBBlue)), A(1.f) {}

    ///Constructor to supply sRGB normalized values with alpha.
    Color(float64 sRGBRed, float64 sRGBGreen,
      float64 sRGBBlue, float64 Alpha) :
      ColorModels::sRGB(float32(sRGBRed), float32(sRGBGreen),
      float32(sRGBBlue)), A(float32(Alpha)) {}

    ///Constructor to supply sRGB byte values.
    Color(uint8 sRGBRed, uint8 sRGBGreen, uint8 sRGBBlue) :
      ColorModels::sRGB(float32(sRGBRed) / 255.f, float32(sRGBGreen)
      / 255.f, float32(sRGBBlue) / 255.f), A(1.f) {}

    ///Constructor to supply sRGB byte values.
    Color(int16 sRGBRed, int16 sRGBGreen, int16 sRGBBlue) :
      ColorModels::sRGB(float32(sRGBRed) / 255.f,
      float32(sRGBGreen) / 255.f, float32(sRGBBlue) / 255.f), A(1.f)
      {}

    ///Constructor to supply sRGB byte values.
    Color(int32 sRGBRed, int32 sRGBGreen, int32 sRGBBlue) :
      ColorModels::sRGB(float32(sRGBRed) / 255.f, float32(sRGBGreen)
      / 255.f, float32(sRGBBlue) / 255.f), A(1.f) {}

    ///Constructor to supply sRGB byte values.
    Color(int64 sRGBRed, int64 sRGBGreen, int64 sRGBBlue) :
      ColorModels::sRGB(float32(sRGBRed) / 255.f, float32(sRGBGreen)
      / 255.f, float32(sRGBBlue) / 255.f), A(1.f) {}

    /**Constructor to supply sRGB 2-byte values. Note this will only be called
    if uint16 values are explicitly given.*/
    Color(uint16 sRGBRed, uint16 sRGBGreen, uint16 sRGBBlue) :
      ColorModels::sRGB(float32(sRGBRed) / 65535.f,
      float32(sRGBGreen) / 65535.f, float32(sRGBBlue) / 65535.f),
      A(1.f) {}

    ///Constructor from sRGB color
    Color(const sRGB& sRGBColor) : ColorModels::sRGB(sRGBColor), A(1.f) {}

    ///Constructor from CSS hex color. Supports \#rgb and \#rrggbb.
    Color(const String& CSSColor) : ColorModels::sRGB(0.f, 0.f, 0.f), A(1.f)
    {
      Import(CSSColor);
    }

    /*Interprets color as a raw device-dependent RGB color. Note that by doing
    this the only way to recover the exact colors is to apply the sRGB profile
    to the output.*/
    operator ColorModels::RGBColor ();

    ///Operator equals
    bool operator == (const Color& Other)
    {
      return Limits<float32>::IsEqual(R, Other.R) &&
             Limits<float32>::IsEqual(G, Other.G) &&
             Limits<float32>::IsEqual(B, Other.B) &&
             Limits<float32>::IsEqual(A, Other.A);
    }

    ///Operator not-equals
    bool operator != (const Color& Other)
    {
      return !(*this == Other);
    }

    ///Returns a copy of this color with a given alpha.
    Color WithAlpha(float32 A_)
    {
      return Color(R, G, B, A_);
    }

    ///Ensures that the color is within bounds.
    void Validate()
    {
      if(Limits<number>::Unbounded(A)) A = 1.f;
      if(Limits<number>::Unbounded(R)) R = 0.f;
      if(Limits<number>::Unbounded(G)) G = 0.f;
      if(Limits<number>::Unbounded(B)) B = 0.f;
      A = Clip(A, 0.f, 1.f);
      R = Clip(R, 0.f, 1.f);
      G = Clip(G, 0.f, 1.f);
      B = Clip(B, 0.f, 1.f);
    }

    ///Imports a color from a string. Currently supports \#rgb and \#rrggbb.
    void Import(String CSSColor)
    {
      A = R = G = B = 0.f;
      CSSColor.Trim();
      if(CSSColor == "none")
        return;
      else if(CSSColor.StartsWith("rgb("))
      {
        A = 1.f;
        CSSColor.Replace("rgb(", "");
        CSSColor.Replace(")", "");
        CSSColor.Replace(" ", "");
        List<String> Colors = CSSColor.Tokenize(",");
        if(Colors.n() == 3)
        {
          R = float32(Colors[0].ToNumber()) / 255.f;
          G = float32(Colors[1].ToNumber()) / 255.f;
          B = float32(Colors[2].ToNumber()) / 255.f;
        }
      }
      else if(CSSColor.EraseBeginning("#"))
      {
        String c;
        if(CSSColor.n() == 3)
        {
          c << CSSColor[0];
          c << CSSColor[0];
          c << CSSColor[1];
          c << CSSColor[1];
          c << CSSColor[2];
          c << CSSColor[2];
        }
        else if(CSSColor.n() == 6)
          c = CSSColor;
        else
          return; //Could not parse color.

        Array<byte> Digits = String::Hex(c);
        A = 1.f;
        R = float32(Digits[0]) / 255.f;
        G = float32(Digits[1]) / 255.f;
        B = float32(Digits[2]) / 255.f;
      }
    }
  };

  /**A listing of SVG and web colors. Note that the names have been left
  lowercase so as not to break convention. These are exact colors as they have
  been specified according to the sRGB color profile.*/
  class Colors
  {
    public:
    static Color Empty() {return Color(0.f, 0.f, 0.f, 0.f);}
    static Color AliceBlue() {return Color(240, 248, 255);}
    static Color AntiqueWhite() {return Color(250, 235, 215);}
    static Color Aqua() {return Color(0, 255, 255);}
    static Color Aquamarine() {return Color(127, 255, 212);}
    static Color Azure() {return Color(240, 255, 255);}
    static Color Beige() {return Color(245, 245, 220);}
    static Color Bisque() {return Color(255, 228, 196);}
    static Color Black() {return Color(0, 0, 0);}
    static Color BlanchedAlmond() {return Color(255, 235, 205);}
    static Color Blue() {return Color(0, 0, 255);}
    static Color BlueViolet() {return Color(138, 43, 226);}
    static Color Brown() {return Color(165, 42, 42);}
    static Color BurlyWood() {return Color(222, 184, 135);}
    static Color CadetBlue() {return Color(95, 158, 160);}
    static Color Chartreuse() {return Color(127, 255, 0);}
    static Color Chocolate() {return Color(210, 105, 30);}
    static Color Coral() {return Color(255, 127, 80);}
    static Color CornflowerBlue() {return Color(100, 149, 237);}
    static Color Cornsilk() {return Color(255, 248, 220);}
    static Color Crimson() {return Color(220, 20, 60);}
    static Color Cyan() {return Color(0, 255, 255);}
    static Color DarkBlue() {return Color(0, 0, 139);}
    static Color DarkCyan() {return Color(0, 139, 139);}
    static Color DarkGoldenrod() {return Color(184, 134, 11);}
    static Color DarkGray() {return Color(169, 169, 169);}
    static Color DarkGreen() {return Color(0, 100, 0);}
    static Color DarkGrey() {return Color(169, 169, 169);}
    static Color DarkKhaki() {return Color(189, 183, 107);}
    static Color DarkMagenta() {return Color(139, 0, 139);}
    static Color DarkOliveGreen() {return Color(85, 107, 47);}
    static Color DarkOrange() {return Color(255, 140, 0);}
    static Color DarkOrchid() {return Color(153, 50, 204);}
    static Color DarkRed() {return Color(139, 0, 0);}
    static Color DarkSalmon() {return Color(233, 150, 122);}
    static Color DarkSeaGreen() {return Color(143, 188, 143);}
    static Color DarkSlateBlue() {return Color(72, 61, 139);}
    static Color DarkSlateGray() {return Color(47, 79, 79);}
    static Color DarkSlateGrey() {return Color(47, 79, 79);}
    static Color DarkTurquoise() {return Color(0, 206, 209);}
    static Color DarkViolet() {return Color(148, 0, 211);}
    static Color DeepPink() {return Color(255, 20, 147);}
    static Color DeepSkyBlue() {return Color(0, 191, 255);}
    static Color DimGray() {return Color(105, 105, 105);}
    static Color DimGrey() {return Color(105, 105, 105);}
    static Color DodgerBlue() {return Color(30, 144, 255);}
    static Color FireBrick() {return Color(178, 34, 34);}
    static Color FloralWhite() {return Color(255, 250, 240);}
    static Color ForestGreen() {return Color(34, 139, 34);}
    static Color Fuchsia() {return Color(255, 0, 255);}
    static Color Gainsboro() {return Color(220, 220, 220);}
    static Color GhostWhite() {return Color(248, 248, 255);}
    static Color Gold() {return Color(255, 215, 0);}
    static Color Goldenrod() {return Color(218, 165, 32);}
    static Color Gray() {return Color(128, 128, 128);}
    static Color Grey() {return Color(128, 128, 128);}
    static Color Green() {return Color(0, 128, 0);}
    static Color GreenYellow() {return Color(173, 255, 47);}
    static Color Honeydew() {return Color(240, 255, 240);}
    static Color HotPink() {return Color(255, 105, 180);}
    static Color IndianRed() {return Color(205, 92, 92);}
    static Color Indigo() {return Color(75, 0, 130);}
    static Color Ivory() {return Color(255, 255, 240);}
    static Color Khaki() {return Color(240, 230, 140);}
    static Color Lavender() {return Color(230, 230, 250);}
    static Color LavenderBlush() {return Color(255, 240, 245);}
    static Color LawnGreen() {return Color(124, 252, 0);}
    static Color LemonChiffon() {return Color(255, 250, 205);}
    static Color LightBlue() {return Color(173, 216, 230);}
    static Color LightCoral() {return Color(240, 128, 128);}
    static Color LightCyan() {return Color(224, 255, 255);}
    static Color LightGoldenRodYellow() {return Color(250, 250, 210);}
    static Color LightGray() {return Color(211, 211, 211);}
    static Color LightGreen() {return Color(144, 238, 144);}
    static Color LightGrey() {return Color(211, 211, 211);}
    static Color LightPink() {return Color(255, 182, 193);}
    static Color LightSalmon() {return Color(255, 160, 122);}
    static Color LightSeagreen() {return Color(32, 178, 170);}
    static Color LightSkyBlue() {return Color(135, 206, 250);}
    static Color LightSlateGray() {return Color(119, 136, 153);}
    static Color LightSlateGrey() {return Color(119, 136, 153);}
    static Color LightSteelBlue() {return Color(176, 196, 222);}
    static Color LightYellow() {return Color(255, 255, 224);}
    static Color Lime() {return Color(0, 255, 0);}
    static Color Limegreen() {return Color(50, 205, 50);}
    static Color Linen() {return Color(250, 240, 230);}
    static Color Magenta() {return Color(255, 0, 255);}
    static Color Maroon() {return Color(128, 0, 0);}
    static Color MediumAquamarine() {return Color(102, 205, 170);}
    static Color MediumBlue() {return Color(0, 0, 205);}
    static Color MediumOrchid() {return Color(186, 85, 211);}
    static Color MediumPurple() {return Color(147, 112, 219);}
    static Color MediumSeaGreen() {return Color(60, 179, 113);}
    static Color MediumSlateBlue() {return Color(123, 104, 238);}
    static Color MediumSpringGreen() {return Color(0, 250, 154);}
    static Color MediumTurquoise() {return Color(72, 209, 204);}
    static Color MediumVioletRed() {return Color(199, 21, 133);}
    static Color MidnightBlue() {return Color(25, 25, 112);}
    static Color MintCream() {return Color(245, 255, 250);}
    static Color MistyRose() {return Color(255, 228, 225);}
    static Color Moccasin() {return Color(255, 228, 181);}
    static Color NavajoWhite() {return Color(255, 222, 173);}
    static Color Navy() {return Color(0, 0, 128);}
    static Color OldLace() {return Color(253, 245, 230);}
    static Color Olive() {return Color(128, 128, 0);}
    static Color OliveDrab() {return Color(107, 142, 35);}
    static Color Orange() {return Color(255, 165, 0);}
    static Color OrangeRed() {return Color(255, 69, 0);}
    static Color Orchid() {return Color(218, 112, 214);}
    static Color PaleGoldenrod() {return Color(238, 232, 170);}
    static Color PaleGreen() {return Color(152, 251, 152);}
    static Color PaleTurquoise() {return Color(175, 238, 238);}
    static Color PaleVioletRed() {return Color(219, 112, 147);}
    static Color PapayaWhip() {return Color(255, 239, 213);}
    static Color PeachPuff() {return Color(255, 218, 185);}
    static Color Peru() {return Color(205, 133, 63);}
    static Color Pink() {return Color(255, 192, 203);}
    static Color Plum() {return Color(221, 160, 221);}
    static Color PowderBlue() {return Color(176, 224, 230);}
    static Color Purple() {return Color(128, 0, 128);}
    static Color Red() {return Color(255, 0, 0);}
    static Color RosyBrown() {return Color(188, 143, 143);}
    static Color RoyalBlue() {return Color(65, 105, 225);}
    static Color SaddleBrown() {return Color(139, 69, 19);}
    static Color Salmon() {return Color(250, 128, 114);}
    static Color SandyBrown() {return Color(244, 164, 96);}
    static Color SeaGreen() {return Color(46, 139, 87);}
    static Color Seashell() {return Color(255, 245, 238);}
    static Color Sienna() {return Color(160, 82, 45);}
    static Color Silver() {return Color(192, 192, 192);}
    static Color SkyBlue() {return Color(135, 206, 235);}
    static Color SlateBlue() {return Color(106, 90, 205);}
    static Color SlateGray() {return Color(112, 128, 144);}
    static Color SlateGrey() {return Color(112, 128, 144);}
    static Color Snow() {return Color(255, 250, 250);}
    static Color SpringGreen() {return Color(0, 255, 127);}
    static Color SteelBlue() {return Color(70, 130, 180);}
    static Color Tan() {return Color(210, 180, 140);}
    static Color Teal() {return Color(0, 128, 128);}
    static Color Thistle() {return Color(216, 191, 216);}
    static Color Tomato() {return Color(255, 99, 71);}
    static Color Turquoise() {return Color(64, 224, 208);}
    static Color Violet() {return Color(238, 130, 238);}
    static Color Wheat() {return Color(245, 222, 179);}
    static Color White() {return Color(255, 255, 255);}
    static Color WhiteSmoke() {return Color(245, 245, 245);}
    static Color Yellow() {return Color(255, 255, 0);}
    static Color YellowGreen() {return Color(154, 205, 50);}
  };

#ifdef BELLE_COMPILE_INLINE
  Color::operator ColorModels::RGBColor ()
  {
    //sRGB --> raw RGB//
    return ColorModels::RGBColor(R, G, B);
  }

  ColorModels::AdobeRGB::operator ColorModels::CIEXYZ ()
  {
    //AdobeRGB --> CIEXYZ//
    return ColorModels::CIEXYZ(
      0.57667f * R + 0.18556f * G + 0.18823f * B,
      0.29734f * R + 0.62736f * G + 0.07529f * B,
      0.02703f * R + 0.07069f * G + 0.99134f * B);
  }

  ColorModels::AdobeRGB::operator ColorModels::sRGB ()
  {
    //AdobeRGB --> CIEXYZ --> sRGB//
    return ColorModels::sRGB(ColorModels::CIEXYZ(*this));
  }

  ColorModels::AdobeRGB::operator ColorModels::CIExyY ()
  {
    //AdobeRGB --> CIEXYZ --> CIExyY//
    return ColorModels::CIExyY(ColorModels::CIEXYZ(*this));
  }

  ColorModels::AdobeRGB::operator Color ()
  {
    //AdobeRGB --> sRGB --> Color (sRGB)//
    return Color(ColorModels::sRGB(*this));
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
    return ColorModels::CIExyY(ColorModels::CIEXYZ(*this));
  }

  ColorModels::sRGB::operator ColorModels::AdobeRGB ()
  {
    //sRGB --> CIEXYZ --> AdobeRGB//
    return AdobeRGB(CIEXYZ(*this));
  }

  ColorModels::CIEXYZ::operator ColorModels::CIExyY ()
  {
    //CIEXYZ --> CIExyY//

    //From: http://en.wikipedia.org/wiki/CIE_xyY (CIE xyY color space)
    return ColorModels::CIExyY(X / (X + Y + Z + 0.000001f),
      Y / (X + Y + Z + 0.000001f), Y);
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
    return Color(ColorModels::sRGB(*this));
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
    return ColorModels::sRGB(ColorModels::CIEXYZ(*this));
  }

  ColorModels::CIExyY::operator ColorModels::AdobeRGB ()
  {
    //CIExyY --> CIEXYZ --> AdobeRGB//
    return ColorModels::AdobeRGB(ColorModels::CIEXYZ(*this));
  }

  ColorModels::CIExyY::operator Color ()
  {
    //CIExyY --> sRGB --> Color (sRGB)//
    return Color(ColorModels::sRGB(*this));
  }
#endif
}
#endif
