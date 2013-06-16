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

#ifndef BELLE_CORE_FONT_H
#define BELLE_CORE_FONT_H

#include "Path.h"
#include "SVG.h"

namespace BELLE_NAMESPACE
{
  ///A glyph stores a path as though it were a character from a typeface.
  struct Glyph : public Path
  {
    /**Describes a kerning pair. Kerning pairs are special combinations of 
    letters which use different spacing than the standard advance width to take 
    advantage of their geometry.*/
    struct Kerning
    {
      ///Character code of the following character.
      prim::unicode FollowingCharacter;

      ///Amount of horizontal space to expand or contract when the glyphs meet.
      prim::number HorizontalAdjustment;
    };

    ///Unicode character code assigned to this glyph.
    prim::unicode Character;
    
    ///Nominal advance width for the character before kerning is applied.
    prim::number AdvanceWidth;
    
    ///Array of kerning pairs.
    prim::Array<Kerning> Kern;
    
    ///Glyph index in original font file (useful for ascertaining kerning).
    prim::count OriginalDeviceIndex;

    ///Default constructor zeroes fields.
    Glyph() : Character(0), AdvanceWidth(0) {}
  };

  ///Typeface consisting of glyphs.
  class Typeface
  {
    ///Used to create a Unicode-sorted glyph list that can be easily searched.
    struct SortableGlyphPointer
    {
      Glyph* g;
      
      ///Default constructor to initialize glyph pointer.
      SortableGlyphPointer() : g(0) {}
      
      ///Constructor to set the glyph pointer.
      SortableGlyphPointer(Glyph* g) : g(g) {}
      
      ///Returns the character code of the glyph.
      prim::unicode c() const
      {
        if(g)
          return g->Character;
        else
          return 0;
      }
      
      ///Sorting < operator.
      bool operator < (const SortableGlyphPointer& Other) const
      {
        return c() < Other.c();
      }

      ///Sorting > operator.
      bool operator > (const SortableGlyphPointer& Other) const
      {
        return c() > Other.c();
      }

      ///Sorting == operator.
      bool operator == (const SortableGlyphPointer& Other) const
      {
        return c() == Other.c();
      }
    };
  
    ///The bounds of the typeface.
    prim::planar::Rectangle GlyphBounds;
    
    /**Holds a Unicode-sorted glyph list that can be easily searched. Since it
    is sorted, the binary-search algorithm can be used to find a particular
    character.*/
    mutable prim::Sortable::Array<SortableGlyphPointer> GlyphTable;

    /**Remembers whether the glyph lookup has been sorted. This allows sorting
    to be lazy, waiting until the first lookup after a modification of the glyph
    table before sorting again.*/
    mutable bool GlyphTableIsSorted;
    
    public:
    
    /*Font information is stored in inches which is the equivalent of 72 points
    (because in digital typography 1 inch = 72 points). Therefore to get a
    particular point size, then all that is needed is to scale by the relative
    amount. For example a 12 point font means scaling by 12/72 = 1/6.*/
    
    ///The typographic height of the typeface in em units.
    prim::number TypographicHeight;
    
    ///The typographic ascender of the typeface in em units.
    prim::number TypographicAscender;
    
    ///The typographic descender of the typeface in em units.
    prim::number TypographicDescender;
    
    ///Adds a glyph to the typeface.
    Glyph& Add()
    {
      GlyphTableIsSorted = false; //Invalidate the lookup table.
      return *(GlyphTable.Add().g = new Glyph);
    }
    
    /**Updates glyph lookup table. This is only necessary if the character code
    of a glyph has been altered after an ith() or LookupGlyph() call.*/
    void UpdateLookup() const
    {
      GlyphTable.Sort();
      GlyphTableIsSorted = true;
    }
    
    ///Looks up a particular character.
    Glyph* LookupGlyph(prim::unicode Character) const
    {
      //Sort the lookup table if necessary.
      if(!GlyphTableIsSorted)
        UpdateLookup();
      
      //Do a binary search for the character.
      prim::count Low = 0;
      prim::count High = GlyphTable.n() - 1;
      prim::unicode Key = Character;
      while(Low <= High)
      {
        prim::count Mid = (High - Low) / 2 + Low; //Variant of (Low + High) / 2
        prim::unicode MidVal = GlyphTable[Mid].c();
        
        if(MidVal < Key)
          Low = Mid + 1;
        else if(MidVal > Key)
          High = Mid - 1;
        else
          return GlyphTable[Mid].g; //Character found
      }
      return 0; //Character not found
    }
    
    ///Returns the number of glyphs in the typeface.
    prim::count n() const
    {
      return GlyphTable.n();
    }
    
    ///Gets the ith glyph sorted by Unicode-character.
    Glyph* ith(prim::count i) const
    {
      //Sort the lookup table if necessary.
      if(!GlyphTableIsSorted)
        UpdateLookup();
      
      if(i < 0 || i >= n())
        return 0;
      return GlyphTable[i].g;
    }
    
    ///Returns the ith glyph sorted by Unicode-character.
    Glyph* operator [] (prim::count i) const
    {
      return ith(i);
    }
    
    ///Clears the typeface and initializes it.
    void Clear()
    {
      for(prim::count i = 0; i < GlyphTable.n(); i++)
        delete GlyphTable[i].g;
      GlyphTable.Clear();
      GlyphBounds = prim::planar::Rectangle();
      TypographicHeight = 0.0;
      TypographicAscender = 0.0;
      TypographicDescender = 0.0;
      GlyphTableIsSorted = false;
    }
    
    ///Default constructor is a typeface with no glyphs.
    Typeface()
    {
      Clear();
    }
    
    ///Do not use copy constructor.
    Typeface(const Typeface& Other);
    
    ///Destructor clears memory associated with the typeface.
    ~Typeface()
    {
      Clear();
    }
    
    ///Calculates the bounding box of the whole typeface.
    prim::planar::Rectangle Bounds(bool Recalculate = false,
      bool IgnorePrivateUseCharacters = false)
    {
      if(!GlyphBounds.IsEmpty() || !Recalculate)
        return GlyphBounds;
      
      prim::planar::Rectangle r;
      for(prim::count i = 0; i < GlyphTable.n(); i++)
      {
        Glyph* g = GlyphTable[i].g;
        prim::unicode c = GlyphTable[i].c();
        
        if(!c || (IgnorePrivateUseCharacters && c >= (prim::unicode)0xE000 &&
          c <= (prim::unicode)0xF8FF))
            continue;
        
        r = r + g->Bounds();
      }
      return GlyphBounds = r;
    }
    
    prim::number Kerning(prim::unicode Left, prim::unicode Right) const
    {
      Glyph* g = LookupGlyph(Left);

      if(!g)
        return 0.0;

      for(prim::count i = 0; i < g->Kern.n(); i++)
        if(g->Kern[i].FollowingCharacter == Right)
          return g->Kern[i].HorizontalAdjustment;

      return 0.0;
    }
    
    ///Saves the typeface to a string containing SVG path data.
    void ExportToSVGString(prim::String& SVG, prim::count Columns = 8,
      prim::count ThumbnailSize = 100)
    {
      //Calculate the number of rows.
      prim::count Rows = n() / Columns;
      if(n() % Columns)
        Rows++;

      //Get the bounding box of all the glyphs.
      prim::planar::Rectangle BoundingBoxOfAllGlyphs = Bounds(true, false);
      prim::number ThumbnailSizeUnits = prim::Max(
        BoundingBoxOfAllGlyphs.Width(), BoundingBoxOfAllGlyphs.Height());
      
      //Force a positive size if necessary.
      if(!ThumbnailSizeUnits)
        ThumbnailSizeUnits = 1.0;

      //Write the header information
      prim::integer DocumentWidth = Columns * ThumbnailSize + 2;
      prim::integer DocumentHeight = Rows * ThumbnailSize + 2; 
      SVG = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
      SVG >> "<!--Created with Belle, Bonne, Sage "
        "(belle::Typeface)-->";
      SVG++;
      SVG >> "<svg ";
      SVG >> "  xmlns=\"http://www.w3.org/2000/svg\"";
      SVG >> "  xmlns:belle=\"https://github.com/burnson/Belle\"";
      SVG >> "  version=\"1.1\"";
      SVG >> "  width=\"" << DocumentWidth << "\"";
      SVG >> "  height=\"" << DocumentHeight << "\">";
      
      SVG++;
      SVG >> "<!--Font Information-->";
      SVG >> "<belle:font height=\"" << TypographicHeight
        << "\" ascender=\"" << TypographicAscender <<
        "\" descender=\"" << TypographicDescender << "\"/>";
      
      SVG++;
      SVG >> "<!--Kerning Information-->";
      for(prim::count i = 0; i < GlyphTable.n(); i++)
      {
        Glyph* g = GlyphTable[i].g;
        if(!g) continue;

        for(prim::count j = 0; j < g->Kern.n(); j++)
          SVG >> "<belle:kern left=\"" << (prim::integer)g->Character
            << "\" right=\"" << (prim::integer)g->Kern[j].FollowingCharacter <<
            "\" horizontal-adjustment=\"" << g->Kern[j].HorizontalAdjustment <<
            "\"/>";
      }

      SVG++;
      SVG >> "<!--Path data for each glyph-->";

      //Determine how to place the glyphs to make the most of space.
      prim::number PercentageToMoveX = 0;
      prim::number PercentageToMoveY = 0;
      if(BoundingBoxOfAllGlyphs.Width() < BoundingBoxOfAllGlyphs.Height())
        PercentageToMoveX = (1.0 - (BoundingBoxOfAllGlyphs.Width() / 
          BoundingBoxOfAllGlyphs.Height())) / 2.0;
      else
        PercentageToMoveY = (1.0 - (BoundingBoxOfAllGlyphs.Height() / 
          BoundingBoxOfAllGlyphs.Width())) / 2.0;

      //Draw thumbnails of each glyph in their own little box.
      for(prim::count i = 0; i < GlyphTable.n(); i++)
      {
        Glyph* g = GlyphTable[i].g;
        if(!g) continue;
        
        //Create a transform to place it in the correct grid box.
        Affine m;
        m *= Affine::Translate(
          prim::planar::Vector((prim::number)((i % Columns) * ThumbnailSize),
          (prim::number)(((i / Columns) + 1) * ThumbnailSize)));
        m *= Affine::Scale(ThumbnailSize / ThumbnailSizeUnits);
        m *= Affine::Scale(prim::planar::Vector(1.0, -1.0));
        m *= Affine::Translate(prim::planar::Vector(
          PercentageToMoveX * BoundingBoxOfAllGlyphs.Height(),
          PercentageToMoveY * BoundingBoxOfAllGlyphs.Width()));
        m *= Affine::Translate(BoundingBoxOfAllGlyphs.BottomLeft() * -1.0);
        
        //Add the path to the SVG string.
        SVGHelper::AppendPathToSVG(*g, SVG, m, g->Character,
          g->AdvanceWidth);
      }

      SVG++;
      SVG >> "<!--Annotations (grid lines, character index, etc.)-->";

      //Draw the grid lines.
      for(prim::count i = 0; i <= Rows; i++)
      {
        prim::number x1 = 0, x2 = (prim::number)(Columns * ThumbnailSize);
        prim::number y1 = (prim::number)(i * ThumbnailSize), y2 = y1;
        SVG >> "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 <<
          "\" y2=\"" << y2 << "\" style=\"stroke:rgb(0,0,0);stroke-width:1\"/>";
      }

      for(prim::count i = 0; i <= Columns; i++)
      {
        prim::number x1 = (prim::number)(i * ThumbnailSize), x2 = x1;
        prim::number y1 = 0, y2 = (prim::number)(Rows * ThumbnailSize);
        SVG >> "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 <<
          "\" y2=\"" << y2 << "\" style=\"stroke:rgb(0,0,0);stroke-width:1\"/>";
      }

      //Draw the unicode character indexes for each glyph.
      for(prim::count i = 0; i < GlyphTable.n(); i++)
      {
        prim::integer c = GlyphTable[i].c();
        if(!c) continue;
        prim::number x1 = (prim::number)((i % Columns) * ThumbnailSize + 2);
        prim::number y1 = (prim::number)((i / Columns) * ThumbnailSize + 14);
        SVG >> "<text x=\"" << x1 << "\" y=\"" << y1 <<
          "\" fill=\"blue\">" << c << "</text>";
      }

      SVG >> "</svg>";
    }
    
    ///Opens the typeface from a string containing SVG-saved information.
    void ImportFromSVGString(const prim::String& Input)
    {
      //Clear the typeface.
      Clear();
      
      //Iterate through all the <path ...> elements in the SVG.
      prim::String::Span PathSpan(-1, -1);
      for(;;)
      {
        //Get the next instance of a <path ...> element.
        prim::String PathBetween;
        PathSpan = Input.FindBetween("<path", ">", PathBetween,
          PathSpan.j() + 1);
        
        //Check whether path data was found.
        if(PathSpan.j() == -1)
          break;
        
        //Add a glyph to the typeface.
        Glyph& g = Add();
        
        //Get the unicode value.
        {
          prim::String Value;
          PathBetween.FindBetween("belle:unicode=\"", "\"", Value);
          if(Value)
            g.Character = (prim::count)Value.ToNumber();
        }
        
        //Get the advance width.
        {
          prim::String Value;
          PathBetween.FindBetween(
            "belle:advance-width=\"", "\"", Value);
          if(Value)
            g.AdvanceWidth = Value.ToNumber();
        }
        
        //Get the path data.
        {
          prim::String Value;
          PathBetween.FindBetween(" d=\"", "\"", Value);
          SVGHelper::ImportData(g, Value);
        }
      }
      
      //Make sure the glyphs are sorted.
      UpdateLookup();
      
      //Calculate the bounds of the font.
      Bounds(true, false);
      
      //Retrieve kerning information from <belle:kern ...>
      prim::String::Span KernSpan(-1, -1);
      for(;;)
      {
        //Get the next instance of a <belle:kern ...> element.
        prim::String KernBetween;
        prim::count Start = KernSpan.j() + 1;
        KernSpan = Input.FindBetween("<belle:kern", ">",
          KernBetween, Start);
        
        //If no more kern elements exist then stop looking for them.
        if(KernSpan.j() == -1)
          break;
          
        prim::unicode Character = 0;
        Glyph::Kerning Kern;
        Kern.FollowingCharacter = 0;
        
        //Get the left character of the kern pair.
        {
          prim::String Value;
          KernBetween.FindBetween("left=\"", "\"", Value);
          if(Value)
            Character = (prim::unicode)Value.ToNumber();
        }
        
        //Get the right character of the kern pair.
        {
          prim::String Value;
          KernBetween.FindBetween("right=\"", "\"", Value);
          if(Value)
            Kern.FollowingCharacter = (prim::unicode)Value.ToNumber();
        }
        
        //Get the kern adjustment.
        {
          prim::String Value;
          KernBetween.FindBetween("horizontal-adjustment=\"", "\"", Value);
          if(Value)
            Kern.HorizontalAdjustment = Value.ToNumber();
        }
        
        //If a valid kern was found, then add it.
        if(Character && Kern.FollowingCharacter)
          if(Glyph* g = LookupGlyph(Character))
            g->Kern.Add() = Kern;
      }
      
      //Retrieve font information from <belle:font ...>
      prim::String::Span FontSpan(-1, -1);
      
      {
        //Get the next instance of a <belle:font ...> element.
        prim::String FontBetween;
        prim::count Start = FontSpan.j() + 1;
        FontSpan = Input.FindBetween("<belle:font", ">",
          FontBetween, Start);
        
        //If there is no font information then exit.
        if(FontSpan.j() == -1)
          return;
        
        //Get the typographic height.
        {
          prim::String Value;
          FontBetween.FindBetween("height=\"", "\"", Value);
          if(Value)
            TypographicHeight = Value.ToNumber();
        }
        
        //Get the typographic ascender.
        {
          prim::String Value;
          FontBetween.FindBetween("ascender=\"", "\"", Value);
          if(Value)
            TypographicAscender = Value.ToNumber();
        }
        
        //Get the typographic descender.
        {
          prim::String Value;
          FontBetween.FindBetween("descender=\"", "\"", Value);
          if(Value)
            TypographicDescender = Value.ToNumber();
        }
      }
    }
    
    private:
    
    template <typename T>
    static void AddToArray(prim::Array<prim::byte>& a, const T& Value)
    {
      a.n(a.n() + sizeof(T));
      T& Destination = *((T*)&a.z(sizeof(T) - 1));
      Destination = Value;
      prim::Endian::ConvertToLittleEndian(Destination);
    }
    
    template<typename T>
    static T ReadFromArray(const prim::byte*& b)
    {
      //Alignment-safe copy (replaces T Value = *((const T*)b);)
      T Value; prim::Memory::Copy(Value, *((const T*)b));
  
      if(!prim::Endian::IsLittleEndian())
        prim::Endian::ReverseOrder(Value);
      b += sizeof(T);
      return Value;
    }
    
    public:
    
    /**Exports the typeface or a portion of the typeface to an array. If p and
    q are specified they refer to the inclusive range of glyph indices (not
    character codes) to be exported, where q < 0 means Length - 1.*/
    void ExportToArray(prim::Array<prim::byte>& a, prim::count p = 0,
      prim::count q = -1)
    {
      //Process the range arguments.
      if(q < 0) q = GlyphTable.n() - 1;
      if(p < 0) p = 0;
      if(p > GlyphTable.n()) p = GlyphTable.n();
      if(q - p + 1 < 0) prim::Swap(p, q);
      prim::count NumberToExport = q - p + 1;
      
      AddToArray(a, (prim::int32)49285378);
      AddToArray(a, (prim::int32)NumberToExport);
      AddToArray(a, (prim::float32)TypographicHeight);
      AddToArray(a, (prim::float32)TypographicAscender);
      AddToArray(a, (prim::float32)TypographicDescender);
      for(prim::count i = p; i <= q; i++)
      {
        Glyph* g = GlyphTable[i].g;
        AddToArray(a, (prim::int32)g->Character);
        AddToArray(a, (prim::float32)g->AdvanceWidth);
        AddToArray(a, (prim::int32)g->Kern.n());
        for(prim::count j = 0; j < g->Kern.n(); j++)
        {
          AddToArray(a, (prim::int32)g->Kern[j].FollowingCharacter);
          AddToArray(a, (prim::float32)g->Kern[j].HorizontalAdjustment);
        }
        AddToArray(a, (prim::int32)g->n());
        for(prim::count j = 0; j < g->n(); j++)
        {
          const Instruction& k = (*g)[j];
          prim::byte Type = k.Type;
          AddToArray(a, Type);
          
          if(k.HasEnd())
          {
            AddToArray(a, (prim::float32)k.End().x);
            AddToArray(a, (prim::float32)k.End().y);
          }
          
          if(k.HasControls())
          {
            AddToArray(a, (prim::float32)k.Control1().x);
            AddToArray(a, (prim::float32)k.Control1().y);
            AddToArray(a, (prim::float32)k.Control2().x);
            AddToArray(a, (prim::float32)k.Control2().y);
          }
        }
      }
    }
    
    void ImportFromArray(const prim::byte* Data)
    {
      Clear();
      if(!Data)
        return;
      const prim::byte* b = Data;
      prim::count MagicNumber = ReadFromArray<prim::int32>(b);
      if(MagicNumber != 49285378)
        return;
      GlyphTable.n(ReadFromArray<prim::int32>(b));
      TypographicHeight = ReadFromArray<prim::float32>(b);
      TypographicAscender = ReadFromArray<prim::float32>(b);
      TypographicDescender = ReadFromArray<prim::float32>(b);
      for(prim::count i = 0; i < GlyphTable.n(); i++)
      {
        Glyph* g = GlyphTable[i].g = new Glyph;
        
        g->Character = ReadFromArray<prim::int32>(b);
        g->AdvanceWidth = ReadFromArray<prim::float32>(b);
        g->Kern.n(ReadFromArray<prim::int32>(b));
        for(prim::count j = 0; j < g->Kern.n(); j++)
        {
          g->Kern[j].FollowingCharacter = ReadFromArray<prim::int32>(b);
          g->Kern[j].HorizontalAdjustment = ReadFromArray<prim::float32>(b);
        }
        prim::count Instructions = ReadFromArray<prim::int32>(b);
        for(prim::count j = 0; j < Instructions; j++)
        {
          prim::byte Type = ReadFromArray<prim::byte>(b);
          if(Type == Instruction::ClosePath)
            g->Add(Instruction());
          else
          {
            prim::planar::Vector e;
            e.x = ReadFromArray<prim::float32>(b);
            e.y = ReadFromArray<prim::float32>(b);
            if(Type == Instruction::MoveTo)
              g->Add(Instruction(e, true));
            else if(Type == Instruction::LineTo)
              g->Add(Instruction(e));
            else
            {
              prim::planar::Vector c1, c2;
              c1.x = ReadFromArray<prim::float32>(b);
              c1.y = ReadFromArray<prim::float32>(b);
              c2.x = ReadFromArray<prim::float32>(b);
              c2.y = ReadFromArray<prim::float32>(b);
              g->Add(Instruction(c1, c2, e));
            }
          }
        }
      }
    }
  
    ///Attempts to load a typeface from a data block using the FreeType library.
    prim::String ImportFromFontData(const prim::byte* ByteArray,
      prim::count LengthInBytes);

    ///Attempts to load a typeface file using the FreeType library.
    prim::String ImportFromFontFile(const prim::ascii* Filename)
    {
      prim::String Result;
      prim::String FileString = Filename;
      
      if(FileString.EndsWith(".dfont") || FileString.EndsWith(".suit"))
          Result = ImportFromFontData((const prim::byte*)Filename, -1);
      else        
      {
        prim::Array<prim::byte> ByteArray;
        prim::File::Read(Filename, ByteArray);
        Result = ImportFromFontData(&ByteArray.a(), ByteArray.n());
      }
      return Result;
    }

#if 0
    ///Calculates the width of a string of text based on a given typeface size.
    prim::number GetTextWidth(const prim::String& Text, 
      prim::number FontSize = 1.0f)
    {
      prim::number Width = 0;
      for(prim::count i = 0; i < Text.n(); i++)
      {
        prim::unicode c = Text[i];
        prim::unicode next_c = 0;
        if(i < Text.n() - 1)
          next_c = Text[i + 1];

        Glyph* g = 0, *next_g = 0;
        if((prim::count)c < GlyphTable.n())
          g = GlyphTable[(prim::count)c];
        if((prim::count)next_c < GlyphTable.n())
          next_g = GlyphTable[(prim::count)next_c];

        if(!g)
          continue;

        Width += g->AdvanceWidth;

        if(!next_g)
          continue;
        
        for(prim::count j = 0; j < g->Kern.n(); j++)
        {
          if(g->Kern[j].FollowingCharacter == next_c)
          {
            Width += g->Kern[j].HorizontalAdjustment;
            break;
          }
        }
      }
      
      return Width * FontSize;
    }
#endif

#if 0    
    ///Generic font file loading method for both native SVG and standard types.
    prim::String Open(const prim::ascii* Filename)
    {
      prim::String FileStr = Filename;
      if(FileStr.EndsWith(".svg"))
        OpenFromSVGFile(Filename);
      else
        return OpenFromFontFile(Filename);
      return "";
    }
#endif
#if 0
    ///Constructor that opens a typeface file by pathname.
    Font(const prim::ascii* Filename) : Height(0), Ascender(0),
      Descender(0), TextMap(0), SymbolMap(0)
    {
      Open(Filename);
    }
#endif
#if 0
    /**Substitutes a string of find text with a string of replacement text if
    the given character exists in the typeface.*/
    void SubstituteCharacters(prim::String& Text, const prim::ascii* FindText, 
      prim::unicode CharacterCodeToSubstitute, 
      prim::String& ReplacementText)
    {
      if((prim::count)CharacterCodeToSubstitute >= GlyphTable.n())
        return;
      if(!GlyphTable[(prim::count)CharacterCodeToSubstitute])
        return;
      Text.Replace(FindText, ReplacementText);
    }
#endif
#if 0
    /**Substitutes a string of find text with a single character if it exists in
    the typeface.*/
    void SubstituteCharacters(prim::String& Text, 
      const prim::ascii* FindText, 
      prim::unicode CharacterCodeToSubstitute)
    {
      prim::String SubstituteText;
      SubstituteText.Append(CharacterCodeToSubstitute);
      SubstituteCharacters(Text, FindText, CharacterCodeToSubstitute,
        SubstituteText);
    }
#endif
#if 0
    /**Converts the combinations ff, fi, fl, ffi, ffl, ft, and st to ligatures.
    Each will only be converted if the ligatures actually exist in the
    typeface.*/
    void MakeLatinLigatures(prim::String& Text)
    {
      SubstituteCharacters(Text, "ff", 0xFB00);
      SubstituteCharacters(Text, "fi", 0xFB01);
      SubstituteCharacters(Text, "fl", 0xFB02);
      SubstituteCharacters(Text, "ffi", 0xFB03);
      SubstituteCharacters(Text, "ffl", 0xFB04);
      prim::String LongSAndT;
      LongSAndT.Append((prim::unicode)0x017F);
      LongSAndT.Append((prim::unicode)0x0074);
      SubstituteCharacters(Text, LongSAndT, 0xFB00);
      //SubstituteCharacters(Text, "st", 0xFB06); //This is uncommon.
    }
#endif
#if 0
    /**Directionalizes quotation marks. Method makes a guess as to where these
    should occur based on context.*/
    void MakeDirectionalQuotationMarks(prim::String& Text)
    {
      prim::String RightQuoteSpace;
      RightQuoteSpace.Append(8217);
      RightQuoteSpace--;
      SubstituteCharacters(Text, "' ", 8217, RightQuoteSpace);
      prim::String CommaRightQuote = ",";
      CommaRightQuote.Append(8217);
      SubstituteCharacters(Text, ",'", 8217, CommaRightQuote);
      prim::String PeriodRightQuote = ".";
      PeriodRightQuote.Append(8217);
      SubstituteCharacters(Text, ".'", 8217, PeriodRightQuote);
      SubstituteCharacters(Text, "'", 8217);

      prim::String DoubleRightQuoteSpace;
      DoubleRightQuoteSpace.Append(8221);
      DoubleRightQuoteSpace--;
      SubstituteCharacters(Text, "\" ", 8221, DoubleRightQuoteSpace);
      prim::String CommaDoubleRightQuote = ",";
      CommaDoubleRightQuote.Append(8221);
      SubstituteCharacters(Text, ",\"", 8221, CommaDoubleRightQuote);
      prim::String PeriodDoubleRightQuote = ".";
      PeriodDoubleRightQuote.Append(8221);
      SubstituteCharacters(Text, ".\"", 8221, PeriodDoubleRightQuote);
      SubstituteCharacters(Text, "\"", 8220);
    }
#endif
#if 0
    ///Turns " - " into an en-dash and "--" into an em-dash.
    void MakeDashes(prim::String& Text)
    {
      SubstituteCharacters(Text, " - ", 8211);
      SubstituteCharacters(Text, " -- ", 8212);
      SubstituteCharacters(Text, "--", 8212);
    }
#endif
  };
  
  ///Ordered collection of typefaces with a priority-based glyph lookup.
  class Font
  {
    public:
    
    ///Typeface and effect possibilities.
    enum Style
    {
      /*Bit-wise style mask. Note that italic and bold are actually typeface
      selectors and are used to determine which typeface to chose from.*/
      Regular = 0,
      Italic = 1,
      Bold = 2,
      BoldItalic = 3,
      Underline = 4,
      Superscript = 8,
      Subscript = 16,
      Strikethrough = 32,
      
      /*The following are used for specialty weights (heavy, light, narrow, etc.
      or any other styles which are not standard.*/
      Special1 = 256 * 1,
      Special2 = 256 * 2,
      Special3 = 256 * 3,
      Special4 = 256 * 4,
      Special5 = 256 * 5,
      Special6 = 256 * 6,
      Special7 = 256 * 7,
    };
    
    private:
    
    ///Ordered array of styles.
    prim::Array<Style> StyleTable;
    
    ///Ordered array of typefaces.
    prim::Array<Typeface*> TypefaceTable;
    
    public:
    
    ///Clears the font.
    void Clear()
    {
      StyleTable.Clear();
      TypefaceTable.ClearAndDeleteAll();
    }
    
    ///Default constructor.
    Font() {Clear();}
    
    ///Do not use copy constructor.
    Font(const Font& Other);
    
    ///Destructor frees all typefaces associated with the font.
    ~Font() {Clear();}
    
    /**Returns the underlying style without any font effects. Specifically,
    underline, superscript, subscript, and strikethrough are discarded.*/
    static Style TypefaceStyle(Style s)
    {
      return (Style)((s & 3) + (s & (256 * 7)));
    }
    
    ///Finds the closest matching character.
    Glyph* LookupGlyph(prim::unicode c, Style PreferredStyle,
      prim::count& TypefaceChosen) const
    {
      //First lookup by preferred style.
      for(TypefaceChosen = 0; TypefaceChosen < StyleTable.n(); TypefaceChosen++)
        if(TypefaceStyle(PreferredStyle) ==
          TypefaceStyle(StyleTable[TypefaceChosen]))
            if(Glyph* g = TypefaceTable[TypefaceChosen]->LookupGlyph(c))
              return g;
      
      /*If the preferred style is unavailable for that character, ignore style
      and just look for the character.*/
      for(TypefaceChosen = 0; TypefaceChosen < StyleTable.n(); TypefaceChosen++)
        if(Glyph* g = TypefaceTable[TypefaceChosen]->LookupGlyph(c))
          return g;
      
      //No glyph is available.
      TypefaceChosen = -1;
      return 0;
    }
    
    ///Locates a typeface of a given style and returns the typeface.
    const Typeface* GetTypeface(Style StyleToLocate) const
    {
      for(prim::count i = 0; i < StyleTable.n(); i++)
        if(TypefaceStyle(StyleToLocate) == TypefaceStyle(StyleTable[i]))
          return TypefaceTable[i];
      return 0;
    }
    
    ///Adds an empty typeface to the font.
    Typeface* Add(Style StyleDescriptor)
    {
      StyleTable.Add() = StyleDescriptor;
      return TypefaceTable.Add() = new Typeface;
    }
    
    ///Adds the next highest priority typeface from an SVG string.
    void AddTypefaceFromSVGString(const prim::String& s, Style StyleDescriptor)
    {
      Add(StyleDescriptor)->ImportFromSVGString(s);
    }
    
    ///Adds the next highest priority typeface from an SVG file.
    void AddTypefaceFromSVG(prim::String Filename, Style StyleDescriptor)
    {
      prim::String s;
      prim::File::Read(Filename, s);
      AddTypefaceFromSVGString(s, StyleDescriptor);
    }
    
    ///Gets the number of typefaces.
    prim::count n() const
    {
      return TypefaceTable.n();
    }
    
    ///Gets a particular typeface from the font.
    const Typeface* ith(prim::count i) const
    {
      return TypefaceTable[i];
    }
    
    ///Gets a particular typeface from the font.
    const Typeface* operator [] (prim::count i) const
    {
      return ith(i);
    }
  };
}
#endif
