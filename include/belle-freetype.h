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

#ifndef BELLE_MODULES_FREETYPE_H
#define BELLE_MODULES_FREETYPE_H

/*
FreeType2 requires a little bit of configuration so it is optional.

To use:
1) Add include path to freetype2 (i.e. -I/usr/local/include/freetype2)
2) Link to freetype (i.e. -lfreetype)
3) Define BELLE_WITH_FREETYPE

For example:
-I/usr/local/include/freetype2 -lfreetype -DBELLE_WITH_FREETYPE
*/

#ifdef BELLE_COMPILE_INLINE
#ifdef BELLE_WITH_FREETYPE

//Disable diagnostics related to Clang warnings in the FreeType header.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-long-long"
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/ftadvanc.h>

//Re-enable diagnostics that were hidden above.
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace BELLE_NAMESPACE
{
  struct FreeType
  {
    struct CallbackData
    {
      number NormalizingScalar;
      Pointer<Glyph> g;
    };

    ///Internal callback move-to used by OpenFromFontData and FreeType
    static int FreeTypeCallbackMoveTo(const FT_Vector* To, void* Data)
    {
      CallbackData* d = reinterpret_cast<CallbackData*>(Data);

      /*If starting a new subpath and there are existing instructions,
      explicitly create a close path instruction to close the previous path.
      FreeType does not issue close-path commands since they are implied by the
      filled nature of font glyph paths.*/
      if(d->g->n())
        d->g->Add(Instruction());

      d->g->Add(Instruction(Vector(number(To->x), number(To->y)) *
        d->NormalizingScalar, true));
      return 0;
    }

    ///Internal callback line-to used by OpenFromFontData and FreeType
    static int FreeTypeCallbackLineTo(const FT_Vector* To, void* Data)
    {
      CallbackData* d = reinterpret_cast<CallbackData*>(Data);
      d->g->Add(Instruction(Vector(number(To->x), number(To->y)) *
        d->NormalizingScalar));
      return 0;
    }

    ///Internal callback conic-to used by OpenFromFontData and FreeType
    static int FreeTypeCallbackConicTo(const FT_Vector* Control,
      const FT_Vector* To, void* Data)
    {
      CallbackData* d = reinterpret_cast<CallbackData*>(Data);

      //Create the conic control point vectors.
      Vector StartPoint = d->g->End(),
        ControlPoint(number(Control->x), number(Control->y)),
        EndPoint(number(To->x), number(To->y));

      //Scale the control and end points. The start point is already scaled.
      ControlPoint *= d->NormalizingScalar;
      EndPoint *= d->NormalizingScalar;

      //Upconvert the conic to a cubic curve.
      Bezier BezierCurve;
      BezierCurve.SetControlPoints(StartPoint, ControlPoint, EndPoint);
      Vector Control1, Control2;
      BezierCurve.GetControlPoints(StartPoint, Control1, Control2, EndPoint);

      //Add the cubic.
      d->g->Add(Instruction(BezierCurve));
      return 0;
    }

    ///Internal callback cubic-to used by OpenFromFontData and FreeType
    static int FreeTypeCallbackCubicTo(const FT_Vector* Control1,
      const FT_Vector* Control2, const FT_Vector* To, void* Data)
    {
      CallbackData* d = reinterpret_cast<CallbackData*>(Data);
      d->g->Add(Instruction(
        Vector(number(Control1->x), number(Control1->y)) * d->NormalizingScalar,
        Vector(number(Control2->x), number(Control2->y)) * d->NormalizingScalar,
        Vector(number(To->x), number(To->y)) * d->NormalizingScalar));
      return 0;
    }
  };

  String Typeface::ImportFromFontData(
    const byte* ByteArray, count LengthInBytes)
  {
    //Initialize the typeface.
    Clear();

    //Initialize the FreeType library.
    FT_Library Library;
    int ErrorCode = 0;
    if( (ErrorCode = FT_Init_FreeType(&Library)) )
      return "Could not initialize font service.";

    //Open the first font face out of the collection.
    FT_Face Face;
    if(LengthInBytes == -1)
      ErrorCode = FT_New_Face(Library,
        reinterpret_cast<const ascii*>(ByteArray), 0, &Face);
    else
      ErrorCode = FT_New_Memory_Face(Library,
        &ByteArray[0], LengthInBytes, 0, &Face);

    if(ErrorCode == FT_Err_Unknown_File_Format)
      return "The font format is unknown.";
    else if(ErrorCode)
      return "The font file is invalid.";

    //Select a symbol character map if it exists.
    count TotalCMaps = Face->num_charmaps;
    for(count i = 0; i < TotalCMaps; i++)
    {
      FT_Encoding enc = Face->charmaps[i]->encoding;
      if(enc == FT_ENCODING_MS_SYMBOL)
        FT_Select_Charmap(Face, enc);
    }

    //Initialize the callback methods which construct the outlines.
    FT_Outline_Funcs OutlineCallbacks;
    OutlineCallbacks.shift = 0;
    OutlineCallbacks.delta = 0;
    OutlineCallbacks.move_to =
      FT_Outline_MoveToFunc(FreeType::FreeTypeCallbackMoveTo);
    OutlineCallbacks.line_to =
      FT_Outline_LineToFunc(FreeType::FreeTypeCallbackLineTo);
    OutlineCallbacks.conic_to =
      FT_Outline_ConicToFunc(FreeType::FreeTypeCallbackConicTo);
    OutlineCallbacks.cubic_to =
      FT_Outline_CubicToFunc(FreeType::FreeTypeCallbackCubicTo);

    //Determine the normalization scalar.
    number NormalizingScalar = 1.0f;
    number UnitsPerEM = number(Face->units_per_EM);
    if(Limits<number>::IsNotZero(UnitsPerEM))
      NormalizingScalar /= UnitsPerEM;

    //Retrieve and normalize the height, ascender, and descender.
    TypographicHeight = number(Face->height) * NormalizingScalar;
    TypographicAscender = number(Face->ascender) * NormalizingScalar;
    TypographicDescender = number(Face->descender) * NormalizingScalar;

    //Load in each character found in the character map.
    FT_ULong CharacterCode = 0;
    FT_UInt GlyphIndex = 0;
    CharacterCode = FT_Get_First_Char(Face, &GlyphIndex);
    count NumberNotLoaded = 0;
    count NumberNotInOutlineFormat = 0;
    count NumberGetGlyphFails = 0;
    count NumberDecomposeFails = 0;
    count TotalTried = 0;

    do
    {
      TotalTried++;

      /*Attempt to load the glyph given by the glyph index. Note: from
      FT_LOAD_NO_SCALE, FT_LOAD_NO_BITMAP is implied.*/
      if((ErrorCode = FT_Load_Glyph(Face, GlyphIndex,
        FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_TRANSFORM)))
      {
        NumberNotLoaded++;
        continue;
      }

      //Make sure that the glyph is in an outline format.
      if(Face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
      {
        NumberNotInOutlineFormat++;
        continue;
      }

      //Attempt to grab the glyph so that its outline can be processed.
      FT_Glyph FTGlyph;
      if((ErrorCode = FT_Get_Glyph(Face->glyph, &FTGlyph)))
      {
        NumberGetGlyphFails++;
        continue;
      }

      //Cast the glyph to an outline glyph to access the outline.
      FT_Outline *Outline =
        &reinterpret_cast<FT_OutlineGlyph>(FTGlyph)->outline;

      //New Glyph to store the converted result.
      Pointer<Glyph> ConvertedGlyph = Add(unicode(CharacterCode));

      //Remember the glyph index.
      ConvertedGlyph->OriginalDeviceIndex = count(GlyphIndex);

      //Set the advance width.
      FT_Fixed Advance = 0;
      FT_Get_Advance(Face, GlyphIndex,
        FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_TRANSFORM, &Advance);
      ConvertedGlyph->AdvanceWidth = number(Advance * NormalizingScalar);

      //Initialize the callback custom data.
      FreeType::CallbackData d;
      d.NormalizingScalar = NormalizingScalar;
      d.g = ConvertedGlyph;

      //Walk through the outline and convert it to the native Glyph format.
      if((ErrorCode = FT_Outline_Decompose(
        Outline, &OutlineCallbacks, reinterpret_cast<void*>(&d))))
          NumberDecomposeFails++;

      /*If there are existing instructions, explicitly create a close path
      instruction to close the previous path. FreeType does not issue close-
      path commands since they are implied by the filled nature of font glyph
      paths.*/
      if(d.g->n())
        d.g->Add(Instruction());

      //Free the memory associated with the glyph.
      FT_Done_Glyph(FTGlyph);

      //Get the next character code and exit if there are no more.
      CharacterCode = FT_Get_Next_Char(Face, CharacterCode, &GlyphIndex);
    } while(GlyphIndex);

    //Calculate the bounds of the font.
    Bounds(true, false);

    /*Determine all of the kernings. Currently this is a brute-force check in
    which all pairwise possibilities are examined. FreeType does not appear to
    have any direct way to determine which glyphs are kerned against which.*/
    count KerningsFound = 0;

    Array<Pointer<Glyph> > GlyphPointers = Glyphs();
    for(count i = 0; i < GlyphPointers.n(); i++)
    {
      Pointer<Glyph> Left = GlyphPointers[i];
      for(count j = 0; j < GlyphPointers.n(); j++)
      {
        Pointer<Glyph> Right = GlyphPointers[j];

        //Retrieve the kerning from the font.
        FT_Vector KerningVector;
        FT_Get_Kerning(Face, FT_UInt(Left->OriginalDeviceIndex),
          FT_UInt(Right->OriginalDeviceIndex), FT_KERNING_UNSCALED,
          &KerningVector);

        //If a non-zero kerning is found then add it to the kerning array.
        if(KerningVector.x)
        {
          KerningsFound++;
          Glyph::Kerning Kern;
          Kern.FollowingCharacter = Right->Character;
          Kern.HorizontalAdjustment = number(KerningVector.x) *
            NormalizingScalar;
          Left->Kern.Add(Kern);
        }
      }
    }

    //Return detailed information on what failed.
    if(NumberNotLoaded > 0 || NumberNotInOutlineFormat > 0 ||
      NumberGetGlyphFails > 0 || NumberDecomposeFails > 0)
    {
      String e;
      e << "Out of " << TotalTried << " glyphs: ";
      e << NumberNotLoaded << " could not be loaded, ";
      e << NumberNotInOutlineFormat << " were not in outline format, ";
      e << NumberGetGlyphFails <<
        " could not be converted to outline glyphs, and ";
      e << NumberDecomposeFails << " failed to generate path segments.";
      return e;
    }

    //Return empty string for success.
    return "";
  }
}
#else
namespace BELLE_NAMESPACE
{
  String Typeface::ImportFromFontData(
    const byte* ByteArray, count LengthInBytes)
  {
    (void)ByteArray;
    (void)LengthInBytes;
    return "Could not load font data. FreeType extension not compiled.";
  }
}
#endif
#endif
#endif
