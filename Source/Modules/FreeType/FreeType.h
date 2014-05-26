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

#ifndef BELLE_MODULES_FREETYPE_H
#define BELLE_MODULES_FREETYPE_H

#ifdef BELLE_COMPILE_INLINE
#ifdef BELLE_WITH_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H 

#include <ftglyph.h>
#include <ftoutln.h>
#include <ftadvanc.h>

namespace BELLE_NAMESPACE
{
  struct FreeType
  {
    struct CallbackData
    {
      prim::number NormalizingScalar;
      Glyph* g;
    };

    ///Internal callback move-to used by OpenFromFontData and FreeType
    static int FreeTypeCallbackMoveTo(const FT_Vector* To, void* Data)
    {
      using namespace prim;
      using namespace prim::planar;
      CallbackData* d = reinterpret_cast<CallbackData*>(Data);
      d->g->Add(Instruction(Vector((number)To->x, (number)To->y) *
        d->NormalizingScalar, true));
      return 0;
    }

    ///Internal callback line-to used by OpenFromFontData and FreeType
    static int FreeTypeCallbackLineTo(const FT_Vector* To, void* Data)
    {
      using namespace prim;
      using namespace prim::planar;
      CallbackData* d = reinterpret_cast<CallbackData*>(Data);
      d->g->Add(Instruction(Vector((number)To->x, (number)To->y) *
        d->NormalizingScalar));
      return 0;
    }

    ///Internal callback conic-to used by OpenFromFontData and FreeType
    static int FreeTypeCallbackConicTo(const FT_Vector* Control,
      const FT_Vector* To, void* Data)
    {
      using namespace prim;
      using namespace prim::planar;
      CallbackData* d = reinterpret_cast<CallbackData*>(Data);
      
      //Create the conic control point vectors.
      Vector StartPoint = d->g->End(),
        ControlPoint((number)Control->x, (number)Control->y),
        EndPoint((number)To->x, (number)To->y);
      
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
      using namespace prim;
      using namespace prim::planar;
      CallbackData* d = reinterpret_cast<CallbackData*>(Data);
      d->g->Add(Instruction(
        Vector((number)Control1->x, (number)Control1->y) * d->NormalizingScalar,
        Vector((number)Control2->x, (number)Control2->y) * d->NormalizingScalar,
        Vector((number)To->x, (number)To->y) * d->NormalizingScalar));
      return 0;
    }
  };

  prim::String Typeface::ImportFromFontData(
    const prim::byte* ByteArray, prim::count LengthInBytes)
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
      ErrorCode = FT_New_Face(Library, (const prim::ascii*)ByteArray, 0, &Face);
    else
      ErrorCode = FT_New_Memory_Face(Library,
        &ByteArray[0], LengthInBytes, 0, &Face);

    if(ErrorCode == FT_Err_Unknown_File_Format)
      return "The font format is unknown.";
    else if(ErrorCode)
      return "The font file is invalid.";
    
    //Select a symbol character map if it exists.
    prim::count TotalCMaps = Face->num_charmaps;
    for(prim::count i = 0; i < TotalCMaps; i++)
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
      (FT_Outline_MoveToFunc)FreeType::FreeTypeCallbackMoveTo;
    OutlineCallbacks.line_to =
      (FT_Outline_LineToFunc)FreeType::FreeTypeCallbackLineTo;
    OutlineCallbacks.conic_to =
      (FT_Outline_ConicToFunc)FreeType::FreeTypeCallbackConicTo;
    OutlineCallbacks.cubic_to =
      (FT_Outline_CubicToFunc)FreeType::FreeTypeCallbackCubicTo;
    
    //Determine the normalization scalar.
    prim::number NormalizingScalar = 1.0f;
    prim::number UnitsPerEM = (prim::number)Face->units_per_EM;
    if(UnitsPerEM)
      NormalizingScalar /= UnitsPerEM;
      
    //Retrieve and normalize the height, ascender, and descender.
    TypographicHeight = (prim::number)Face->height * NormalizingScalar;
    TypographicAscender = (prim::number)Face->ascender * NormalizingScalar;
    TypographicDescender = (prim::number)Face->descender * NormalizingScalar;

    //Load in each character found in the character map.
    FT_ULong CharacterCode = 0;
    FT_UInt GlyphIndex = 0;
    CharacterCode = FT_Get_First_Char(Face, &GlyphIndex);
    prim::count NumberNotLoaded = 0;
    prim::count NumberNotInOutlineFormat = 0;
    prim::count NumberGetGlyphFails = 0;
    prim::count NumberDecomposeFails = 0;
    prim::count TotalTried = 0;
    
    do
    {
      TotalTried++;
      
      /*Attempt to load the glyph given by the glyph index. Note: from
      FT_LOAD_NO_SCALE, FT_LOAD_NO_BITMAP is implied.*/
      if( (ErrorCode = FT_Load_Glyph(Face, GlyphIndex,
        FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_TRANSFORM)) )
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
      FT_Outline *Outline = &reinterpret_cast<FT_OutlineGlyph>(FTGlyph)->outline;
      
      //New Glyph to store the converted result.
      Glyph& ConvertedGlyph = Add();
      
      //Set the character code.
      ConvertedGlyph.Character = CharacterCode;
      
      //Remember the glyph index.
      ConvertedGlyph.OriginalDeviceIndex = (prim::count)GlyphIndex;
      
      //Set the advance width.
      FT_Fixed Advance = 0;
      FT_Get_Advance(Face, GlyphIndex,
        FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_TRANSFORM, &Advance);
      ConvertedGlyph.AdvanceWidth = (prim::number)Advance * NormalizingScalar;
      
      //Initialize the callback custom data.
      FreeType::CallbackData d;
      d.NormalizingScalar = NormalizingScalar;
      d.g = &ConvertedGlyph;
      
      //Walk through the outline and convert it to the native Glyph format.
      if((ErrorCode = FT_Outline_Decompose(
        Outline, &OutlineCallbacks, (void*)&d)))
          NumberDecomposeFails++;
      
      //Free the memory associated with the glyph.
      FT_Done_Glyph(FTGlyph);
      
      //Get the next character code and exit if there are no more.
    } while((CharacterCode = FT_Get_Next_Char(
        Face, CharacterCode, &GlyphIndex)));
    
    //Update the glyph lookup.
    UpdateLookup();
    
    //Calculate the bounds of the font.
    Bounds(true, false);
        
    /*Determine all of the kernings. Currently this is a brute-force check in
    which all pairwise possibilities are examined. FreeType does not appear to
    have any direct way to determine which glyphs are kerned against which.*/
    prim::count KerningsFound = 0;
    for(prim::count i = 0; i < n(); i++)
    {
      Glyph* Left = ith(i);
      for(prim::count j = 0; j < n(); j++)
      {
        Glyph* Right = ith(j);
        
        //Retrieve the kerning from the font.
        FT_Vector KerningVector;
        FT_Get_Kerning(Face, (FT_UInt)Left->OriginalDeviceIndex,
          (FT_UInt)Right->OriginalDeviceIndex, FT_KERNING_UNSCALED,
          &KerningVector);
          
        //If a non-zero kerning is found then add it to the kerning array.
        if(KerningVector.x)
        {
          KerningsFound++;
          Glyph::Kerning Kern;
          Kern.FollowingCharacter = Right->Character;
          Kern.HorizontalAdjustment = (prim::number)KerningVector.x;
          Left->Kern.Add(Kern);
        }
      }
    }
    
    //Return detailed information on what failed.
    if(NumberNotLoaded > 0 || NumberNotInOutlineFormat > 0 ||
      NumberGetGlyphFails > 0 || NumberDecomposeFails > 0)
    {
      prim::String e;
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
  prim::String Typeface::ImportFromFontData(
    const prim::byte* ByteArray, prim::count LengthInBytes)
  {
    prim::String s(ByteArray, LengthInBytes); //Suppress unused param warning.
    return "Could not load font data. FreeType extension not compiled.";
  }
}
#endif
#endif
#endif
