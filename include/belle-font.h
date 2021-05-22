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

#ifndef BELLE_CORE_FONT_H
#define BELLE_CORE_FONT_H

#include "belle-path.h"
#include "belle-svg.h"

namespace BELLE_NAMESPACE
{
  ///A glyph stores a path as though it were a character from a typeface.
  class Glyph : public Path
  {
    public:

    ///Gets the collision polygon.
    PolygonShape Collision;

    /**Describes a kerning pair. Kerning pairs are special combinations of
    letters which use different spacing than the standard advance width to take
    advantage of their geometry.*/
    class Kerning
    {
      public:

      ///Character code of the following character.
      unicode FollowingCharacter; PRIM_PAD(unicode)

      ///Amount of horizontal space to expand or contract when the glyphs meet.
      number HorizontalAdjustment; PRIM_PAD(number)
    };

    ///Unicode character code assigned to this glyph.
    unicode Character; PRIM_PAD(unicode)

    ///Nominal advance width for the character before kerning is applied.
    number AdvanceWidth; PRIM_PAD(number)

    ///Array of kerning pairs.
    Array<Kerning> Kern;

    ///Glyph index in original font file (useful for ascertaining kerning).
    count OriginalDeviceIndex; PRIM_PAD(count)

    ///Markers for the glyph.
    Value Markers;

    ///Default constructor zeroes fields.
    Glyph() : Character(0), AdvanceWidth(0) {}

    ///Creates a copy of the glyph from another.
    Glyph(const Glyph& Other) : Path(Other)
    {
      Collision = Other.Collision;
      Character = Other.Character;
      AdvanceWidth = Other.AdvanceWidth;
      Kern = Other.Kern;
      OriginalDeviceIndex = Other.OriginalDeviceIndex;
      Markers = Other.Markers;
    }

    ///Deep copies another glyph to this one.
    Glyph& operator = (const Glyph& Other)
    {
      Collision = Other.Collision;
      Character = Other.Character;
      AdvanceWidth = Other.AdvanceWidth;
      Kern = Other.Kern;
      OriginalDeviceIndex = Other.OriginalDeviceIndex;
      Markers = Other.Markers;
      return *this;
    }

    ///Scales the glyph in place.
    void Scale(Vector ScaleFactor)
    {
      Transform(Affine::Scale(ScaleFactor));
      AdvanceWidth *= ScaleFactor.x;
      for(count i = 0; i < Kern.n(); i++)
        Kern[i].HorizontalAdjustment *= ScaleFactor.x;
    }

    ///Virtual destructor
    virtual ~Glyph();

    const PolygonShape& GetCollisionBounds()
    {
      return Collision;
    }
  };

  ///Typeface consisting of glyphs.
  class Typeface
  {
    ///The bounds of the typeface.
    Box GlyphBounds;

    ///Holds a Unicode-sorted glyph list that can be easily searched.
    Tree<unicode, Pointer<Glyph> > GlyphTree;

    public:

    /*Font information is stored in inches which is the equivalent of 72 points
    (because in digital typography 1 inch = 72 points). Therefore to get a
    particular point size, then all that is needed is to scale by the relative
    amount. For example a 12 point font means scaling by 12/72 = 1/6.*/

    ///The typographic height of the typeface in em units.
    number TypographicHeight;

    ///The typographic ascender of the typeface in em units.
    number TypographicAscender;

    ///The typographic descender of the typeface in em units.
    number TypographicDescender;

    ///Adds a glyph to the typeface.
    Pointer<Glyph> Add(unicode Character)
    {
      Pointer<Glyph> g = GlyphTree[Character].New();
      g->Character = Character;
      return g;
    }

    /**Removes a glyph from the typeface. Call Bounds(true) to recalculate the
    typeface bounds.*/
    void Remove(unicode Character)
    {
      GlyphTree.Remove(Character);
    }

    /**Scales the typeface in place. Recalculates bounds.*/
    void Scale(Vector ScaleFactor)
    {
      Array<unicode> Codepoints = GlyphCodepoints();
      for(count i = 0; i < Codepoints.n(); i++)
        GlyphTree[Codepoints[i]]->Scale(ScaleFactor);
      TypographicHeight *= ScaleFactor.y;
      TypographicAscender *= ScaleFactor.y;
      TypographicDescender *= ScaleFactor.y;
      Bounds(true);
    }

    ///Looks up a particular character.
    Pointer<Glyph> LookupGlyph(unicode Character) const
    {
      return GlyphTree[Character];
    }

    ///Returns the number of glyphs in the typeface.
    count n() const
    {
      return GlyphTree.n();
    }

    ///Returns an ordered array of codepoints in the typeface.
    Array<unicode> GlyphCodepoints() const
    {
      Array<unicode> Codepoints;
      GlyphTree.Keys(Codepoints);
      return Codepoints;
    }

    ///Returns a code-point ordered array of pointers to glyphs in the typeface.
    Array<Pointer<Glyph> > Glyphs() const
    {
      Array<Pointer<Glyph> > GlyphPointers;
      GlyphTree.Values(GlyphPointers);
      return GlyphPointers;
    }

    ///Clears the typeface and initializes it.
    void Clear()
    {
      GlyphTree.RemoveAll();
      GlyphBounds = Box();
      TypographicHeight = 0.f;
      TypographicAscender = 0.f;
      TypographicDescender = 0.f;
    }

    ///Default constructor is a typeface with no glyphs.
    Typeface()
    {
      Clear();
    }

    ///Creates a deep copy of the typeface.
    Typeface(const Typeface& Other)
    {
      GlyphBounds = Other.GlyphBounds;
      TypographicHeight = Other.TypographicHeight;
      TypographicAscender = Other.TypographicAscender;
      TypographicDescender = Other.TypographicDescender;
      Array<unicode> Codepoints = Other.GlyphCodepoints();
      Array<Pointer<Glyph> > GlyphPointers = Other.Glyphs();
      for(count i = 0; i < Codepoints.n(); i++)
      {
        Pointer<Glyph> g;
        *g.New() = *GlyphPointers[i];
        GlyphTree[Codepoints[i]] = g;
      }
    }

    ///Destructor clears memory associated with the typeface.
    ~Typeface()
    {
      Clear();
    }

    ///Calculates the bounding box of the whole typeface.
    Box Bounds(bool Recalculate = false,
      bool IgnorePrivateUseCharacters = false)
    {
      if(!GlyphBounds.IsEmpty() || !Recalculate)
        return GlyphBounds;

      Box r;

      Array<Pointer<Glyph> > GlyphPointers = Glyphs();
      for(count i = 0; i < GlyphPointers.n(); i++)
      {
        Pointer<Glyph> g = GlyphPointers[i];
        unicode c = g->Character;

        if(!c || (IgnorePrivateUseCharacters && c >= unicode(0xE000) &&
          c <= unicode(0xF8FF)))
            continue;

        r = r + g->Bounds();
      }
      return GlyphBounds = r;
    }

    number Kerning(unicode Left, unicode Right) const
    {
      Pointer<Glyph> g = LookupGlyph(Left);

      if(!g)
        return 0.0;

      for(count i = 0; i < g->Kern.n(); i++)
        if(g->Kern[i].FollowingCharacter == Right)
          return g->Kern[i].HorizontalAdjustment;

      return 0.0;
    }

    ///Loads a typeface from a JSON string.
    void ImportFromJSON(const String& JSONData)
    {
      Clear();
      Value v;
      JSON::Import(JSONData, v);
      TypographicHeight = v["TypographicHeight"].AsNumber();
      TypographicAscender = v["TypographicAscender"].AsNumber();
      TypographicDescender = v["TypographicDescender"].AsNumber();
      Array<Value> GlyphsKeys;
      v["Glyphs"].EnumerateKeys(GlyphsKeys);
      for(count i = 0, n = GlyphsKeys.n(); i < n; i++)
      {
        Value Unicode = GlyphsKeys[i];
        Pointer<Glyph> g = Add(unicode(Unicode.AsCount()));
        SVGHelper::ImportData(*g, v["Glyphs"][Unicode]["Path"].AsString());
        Path Collision;
        SVGHelper::ImportData(Collision,
          v["Glyphs"][Unicode]["Collision"].AsString());
        if(Collision.Outline().n())
          g->Collision = Collision.Outline().a();

        g->AdvanceWidth = v["Glyphs"][Unicode]["AdvanceWidth"].AsNumber();
        g->Markers = v["Glyphs"][Unicode]["Markers"];
        Value Kerning = v["Glyphs"][Unicode]["Kerning"];
        Array<Value> KerningKeys;
        Kerning.EnumerateKeys(KerningKeys);
        for(count j = 0; j < KerningKeys.n(); j++)
        {
          Glyph::Kerning k;
          k.FollowingCharacter = unicode(KerningKeys[j].AsCount());
          k.HorizontalAdjustment = Kerning[KerningKeys[j]];
          g->Kern.Add(k);
        }
      }
    }

    ///Saves a typeface to a JSON string.
    void ExportToJSON(String& JSONOut) const
    {
      Value v;
      v["TypographicHeight"] = TypographicHeight;
      v["TypographicAscender"] = TypographicAscender;
      v["TypographicDescender"] = TypographicDescender;
      v["Glyphs"].NewTree();
      Array<Pointer<Glyph> > GlyphPointers = Glyphs();
      for(count i = 0; i < GlyphPointers.n(); i++)
      {
        Pointer<Glyph> g = GlyphPointers[i];
        Value GlyphData;

        {
          String PathData;
          SVGHelper::AppendPathData(*g, PathData);
          GlyphData["Path"] = PathData;
        }

        {
          String CollisionData;
          Path p;
          if(g->Collision.n())
            p.Append(g->Collision);
          else
            p.AppendConvex(*g);
          SVGHelper::AppendPathData(p, CollisionData);
          GlyphData["Collision"] = CollisionData;
        }

        GlyphData["AdvanceWidth"] = g->AdvanceWidth;
        GlyphData["Kerning"].NewTree();
        for(count j = 0; j < g->Kern.n(); j++)
        {
          Glyph::Kerning k = g->Kern[j];
          GlyphData["Kerning"][count(k.FollowingCharacter)] =
            k.HorizontalAdjustment;
        }

        GlyphData["Markers"] = g->Markers;
        if(GlyphData["Markers"].IsNil())
          GlyphData["Markers"].NewTree();

        v["Glyphs"][count(g->Character)] = GlyphData;
      }
      JSON::Export(v, JSONOut, true);
    }

    ///Saves the typeface to a string containing SVG path data.
    void ExportToSVGString(String& SVG, count Columns = 8,
      count ThumbnailSize = 100)
    {
      //Calculate the number of rows.
      count Rows = n() / Columns;
      if(n() % Columns)
        Rows++;

      //Get the bounding box of all the glyphs.
      Box BoundingBoxOfAllGlyphs = Bounds(true, false);
      number ThumbnailSizeUnits = Max(
        BoundingBoxOfAllGlyphs.Width(), BoundingBoxOfAllGlyphs.Height());

      //Force a positive size if necessary.
      if(Limits<number>::IsZero(ThumbnailSizeUnits))
        ThumbnailSizeUnits = 1.0;

      //Write the header information
      count DocumentWidth = Columns * ThumbnailSize + 2;
      count DocumentHeight = Rows * ThumbnailSize + 2;
      SVG = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
      SVG >> "<!--Created with Belle "
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
      Array<Pointer<Glyph> > GlyphPointers = Glyphs();
      for(count i = 0; i < GlyphPointers.n(); i++)
      {
        Pointer<Glyph> g = GlyphPointers[i];
        if(!g) continue;

        for(count j = 0; j < g->Kern.n(); j++)
          SVG >> "<belle:kern left=\"" << integer(g->Character)
            << "\" right=\"" << integer(g->Kern[j].FollowingCharacter) <<
            "\" horizontal-adjustment=\"" << g->Kern[j].HorizontalAdjustment <<
            "\"/>";
      }

      SVG++;
      SVG >> "<!--Path data for each glyph-->";

      //Determine how to place the glyphs to make the most of space.
      number PercentageToMoveX = 0;
      number PercentageToMoveY = 0;
      if(BoundingBoxOfAllGlyphs.Width() < BoundingBoxOfAllGlyphs.Height())
        PercentageToMoveX = (1.f - (BoundingBoxOfAllGlyphs.Width() /
          BoundingBoxOfAllGlyphs.Height())) / 2.f;
      else
        PercentageToMoveY = (1.f - (BoundingBoxOfAllGlyphs.Height() /
          BoundingBoxOfAllGlyphs.Width())) / 2.f;

      //Draw thumbnails of each glyph in their own little box.
      for(count i = 0; i < GlyphPointers.n(); i++)
      {
        Pointer<Glyph> g = GlyphPointers[i];
        if(!g) continue;

        //Create a transform to place it in the correct grid box.
        Affine m;
        m *= Affine::Translate(
          Vector(number((i % Columns) * ThumbnailSize),
          number(((i / Columns) + 1) * ThumbnailSize)));
        m *= Affine::Scale(ThumbnailSize / ThumbnailSizeUnits);
        m *= Affine::Scale(Vector(1.0, -1.0));
        m *= Affine::Translate(Vector(
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
      for(count i = 0; i <= Rows; i++)
      {
        number x1 = 0, x2 = number(Columns * ThumbnailSize);
        number y1 = number(i * ThumbnailSize), y2 = y1;
        SVG >> "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 <<
          "\" y2=\"" << y2 << "\" style=\"stroke:rgb(0,0,0);stroke-width:1\"/>";
      }

      for(count i = 0; i <= Columns; i++)
      {
        number x1 = number(i * ThumbnailSize), x2 = x1;
        number y1 = 0, y2 = number(Rows * ThumbnailSize);
        SVG >> "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 <<
          "\" y2=\"" << y2 << "\" style=\"stroke:rgb(0,0,0);stroke-width:1\"/>";
      }

      //Draw the unicode character indexes for each glyph.
      for(count i = 0; i < GlyphPointers.n(); i++)
      {
        Pointer<Glyph> g = GlyphPointers[i];
        integer c = integer(g->Character);
        if(!c) continue;
        number x1 = number((i % Columns) * ThumbnailSize + 2);
        number y1 = number((i / Columns) * ThumbnailSize + 14);
        SVG >> "<text x=\"" << x1 << "\" y=\"" << y1 <<
          "\" fill=\"blue\">" << c << "</text>";
      }

      SVG >> "</svg>";
    }

    ///Opens the typeface from a string containing SVG-saved information.
    void ImportFromSVGString(const String& Input)
    {
      //Clear the typeface.
      Clear();

      //Iterate through all the <path ...> elements in the SVG.
      String::Span PathSpan(-1, -1);
      for(;;)
      {
        //Get the next instance of a <path ...> element.
        String PathBetween;
        PathSpan = Input.FindBetween("<path", ">", PathBetween,
          PathSpan.j() + 1);

        //Check whether path data was found.
        if(PathSpan.j() == -1)
          break;

        //Add a glyph to the typeface.
        Pointer<Glyph> g;

        //Get the unicode value.
        {
          String Value;
          PathBetween.FindBetween("belle:unicode=\"", "\"", Value);
          if(Value)
            g = Add(unicode(Value.ToNumber()));
          if(!g)
            continue;
        }

        //Get the advance width.
        {
          String Value;
          PathBetween.FindBetween(
            "belle:advance-width=\"", "\"", Value);
          if(Value)
            g->AdvanceWidth = Value.ToNumber();
        }

        //Get the path data.
        {
          String Value;
          PathBetween.FindBetween(" d=\"", "\"", Value);
          SVGHelper::ImportData(*g, Value);
        }
      }

      //Calculate the bounds of the font.
      Bounds(true, false);

      //Retrieve kerning information from <belle:kern ...>
      String::Span KernSpan(-1, -1);
      for(;;)
      {
        //Get the next instance of a <belle:kern ...> element.
        String KernBetween;
        count Start = KernSpan.j() + 1;
        KernSpan = Input.FindBetween("<belle:kern", ">",
          KernBetween, Start);

        //If no more kern elements exist then stop looking for them.
        if(KernSpan.j() == -1)
          break;

        unicode Character = 0;
        Glyph::Kerning Kern;
        Kern.FollowingCharacter = 0;

        //Get the left character of the kern pair.
        {
          String Value;
          KernBetween.FindBetween("left=\"", "\"", Value);
          if(Value)
            Character = unicode(Value.ToNumber());
        }

        //Get the right character of the kern pair.
        {
          String Value;
          KernBetween.FindBetween("right=\"", "\"", Value);
          if(Value)
            Kern.FollowingCharacter = unicode(Value.ToNumber());
        }

        //Get the kern adjustment.
        {
          String Value;
          KernBetween.FindBetween("horizontal-adjustment=\"", "\"", Value);
          if(Value)
            Kern.HorizontalAdjustment = Value.ToNumber();
        }

        //If a valid kern was found, then add it.
        if(Character && Kern.FollowingCharacter)
          if(Pointer<Glyph> g = LookupGlyph(Character))
            g->Kern.Add() = Kern;
      }

      //Retrieve font information from <belle:font ...>
      String::Span FontSpan(-1, -1);

      {
        //Get the next instance of a <belle:font ...> element.
        String FontBetween;
        count Start = FontSpan.j() + 1;
        FontSpan = Input.FindBetween("<belle:font", ">",
          FontBetween, Start);

        //If there is no font information then exit.
        if(FontSpan.j() == -1)
          return;

        //Get the typographic height.
        {
          String Value;
          FontBetween.FindBetween("height=\"", "\"", Value);
          if(Value)
            TypographicHeight = Value.ToNumber();
        }

        //Get the typographic ascender.
        {
          String Value;
          FontBetween.FindBetween("ascender=\"", "\"", Value);
          if(Value)
            TypographicAscender = Value.ToNumber();
        }

        //Get the typographic descender.
        {
          String Value;
          FontBetween.FindBetween("descender=\"", "\"", Value);
          if(Value)
            TypographicDescender = Value.ToNumber();
        }
      }
    }

    /**Imports a glyph from a string containing SVG-saved information. If
    successful it returns the codepoint of the added glyph, and zero
    otherwise.*/
    unicode ImportGlyphFromSMuFLSVGString(const String& Input)
    {
      //Codepoint of the glyph that is added.
      unicode CodepointAdded = 0;

      //Add a glyph to the typeface.
      Pointer<Glyph> g;
      PolygonShape Collision;

      //Iterate through all the <path ...> elements in the SVG.
      String::Span PathSpan(-1, -1);

      //Determine the width of the SVG
      Affine AffineTransform;
      {
        String PathBetween;
        PathSpan = Input.FindBetween("width=\"", "\"", PathBetween, 0);
        number ViewSize = PathBetween.ToNumber();

        if(ViewSize < 1.0)
          ViewSize = 1.0;
        else if(ViewSize > 10000.0)
          ViewSize = 10000.0;

        number Scale = 1.f / 100.f;
        AffineTransform =
          Affine::Translate(Vector(-ViewSize, ViewSize) * Scale / 2.f) *
          Affine::Scale(Vector(Scale, -Scale));
      }
      for(;;)
      {
        String PathBetween;
        //Get the next instance of a <g ...> layer element.
        PathSpan = Input.FindBetween("<g", ">", PathBetween,
          PathSpan.j() + 1);

        //If no more layers were found, then break out of the loop.
        if(PathSpan.j() == -1)
          break;

        //Get the layer name.
        String LayerName;
        PathBetween.FindBetween("id=\"", "\"", LayerName);

        //Skip staff lines layer.
        if(LayerName == "Staff Lines")
          continue;

        //Get the next instance of a <path ...> element.
        PathSpan = Input.FindBetween("<path", ">", PathBetween,
          PathSpan.j() + 1);

        //Check whether path data was found.
        if(PathSpan.j() == -1)
          continue;

        //Get the path data.
        if(LayerName == "Collision")
        {
          Path p;
          String Value;
          PathBetween.FindBetween(" d=\"", "\"", Value);
          SVGHelper::ImportData(p, Value);
          Path pTransformed(p, AffineTransform);
          if(pTransformed.Outline().n())
            Collision = pTransformed.Outline().a();
        }
        else if(LayerName.Contains(" (U+"))
        {
          String SMuFLName = LayerName.Substring(0, LayerName.Find(" ") - 1);
          String Codepoint = LayerName;
          Codepoint.Replace(SMuFLName + " (U+", "");
          Codepoint.Replace(")", "");
          Array<byte> Hex = String::Hex(Codepoint);
          CodepointAdded = unicode((Hex[0] << 8) + Hex[1]);
          g = Add(CodepointAdded);
          String Value;
          PathBetween.FindBetween(" d=\"", "\"", Value);

          Path p;
          SVGHelper::ImportData(p, Value);
          g->Append(p, AffineTransform);
        }
      }

      //Update the collision bounds.
      if(g)
      {
        if(Collision.n())
          g->Collision = Collision;
        else
        {
          Path p;
          p.AppendConvex(*g);
          if(p.Outline().n())
            g->Collision = p.Outline().a();
        }
      }

      return CodepointAdded;
    }

    private:

    template <typename T>
    static void AddToArray(Array<byte>& a, const T& Value)
    {
      a.n(a.n() + count(sizeof(T)));
      Memory::MemCopy(reinterpret_cast<void*>(&a.z(count(sizeof(T)) - 1)),
        reinterpret_cast<const void*>(&Value),
        count(sizeof(T)));
      if(!Endian::IsLittleEndian())
        Endian::ReverseByteOrder(&a.z(count(sizeof(T)) - 1), count(sizeof(T)));
    }

    template<typename T>
    static T ReadFromArray(const byte*& b)
    {
      //Alignment-safe copy (replaces T Value = *((const T*)b);)
      T Value;
      Memory::MemCopy(reinterpret_cast<void*>(&Value),
        reinterpret_cast<const void*>(b), count(sizeof(T)));

      Endian::ConvertToLittleEndian(Value);
      b += sizeof(T);
      return Value;
    }

    public:

    /**Exports the typeface or a portion of the typeface to an array. If p and
    q are specified they refer to the inclusive range of glyph indices (not
    character codes) to be exported, where q < 0 means Length - 1.*/
    void ExportToArray(Array<byte>& a, count p = 0,
      count q = -1)
    {
      //Process the range arguments.
      if(q < 0) q = GlyphTree.n() - 1;
      if(p < 0) p = 0;
      if(p > GlyphTree.n()) p = GlyphTree.n();
      if(q - p + 1 < 0) Swap(p, q);
      count NumberToExport = q - p + 1;

      AddToArray(a, int32(49285378));
      AddToArray(a, int32(NumberToExport));
      AddToArray(a, float32(TypographicHeight));
      AddToArray(a, float32(TypographicAscender));
      AddToArray(a, float32(TypographicDescender));
      Array<Pointer<Glyph> > GlyphPointers = Glyphs();
      for(count i = p; i <= q; i++)
      {
        Pointer<Glyph> g = GlyphPointers[i];
        AddToArray(a, int32(g->Character));
        AddToArray(a, float32(g->AdvanceWidth));
        AddToArray(a, int32(g->Kern.n()));
        for(count j = 0; j < g->Kern.n(); j++)
        {
          AddToArray(a, int32(g->Kern[j].FollowingCharacter));
          AddToArray(a, float32(g->Kern[j].HorizontalAdjustment));
        }
        AddToArray(a, int32(g->n()));
        for(count j = 0; j < g->n(); j++)
        {
          const Instruction& k = (*g)[j];
          byte Type = byte(k.Type);
          AddToArray(a, Type);

          if(k.HasEnd())
          {
            AddToArray(a, float32(k.End().x));
            AddToArray(a, float32(k.End().y));
          }

          if(k.HasControls())
          {
            AddToArray(a, float32(k.Control1().x));
            AddToArray(a, float32(k.Control1().y));
            AddToArray(a, float32(k.Control2().x));
            AddToArray(a, float32(k.Control2().y));
          }
        }
      }
    }

    void ImportFromArray(const byte* Data)
    {
      Clear();
      if(!Data)
        return;
      const byte* b = Data;
      count MagicNumber = ReadFromArray<int32>(b);
      if(MagicNumber != 49285378)
        return;
      count GlyphCount = count(ReadFromArray<int32>(b));
      TypographicHeight = ReadFromArray<float32>(b);
      TypographicAscender = ReadFromArray<float32>(b);
      TypographicDescender = ReadFromArray<float32>(b);
      for(count i = 0; i < GlyphCount; i++)
      {
        unicode Character = unicode(ReadFromArray<int32>(b));
        Pointer<Glyph> g = Add(Character);
        g->AdvanceWidth = ReadFromArray<float32>(b);
        g->Kern.n(ReadFromArray<int32>(b));
        for(count j = 0; j < g->Kern.n(); j++)
        {
          g->Kern[j].FollowingCharacter = unicode(ReadFromArray<int32>(b));
          g->Kern[j].HorizontalAdjustment = ReadFromArray<float32>(b);
        }
        count Instructions = count(ReadFromArray<int32>(b));
        for(count j = 0; j < Instructions; j++)
        {
          byte Type = ReadFromArray<byte>(b);
          if(Type == Instruction::ClosePath)
            g->Add(Instruction());
          else
          {
            Vector e;
            e.x = ReadFromArray<float32>(b);
            e.y = ReadFromArray<float32>(b);
            if(Type == Instruction::MoveTo)
              g->Add(Instruction(e, true));
            else if(Type == Instruction::LineTo)
              g->Add(Instruction(e));
            else
            {
              Vector c1, c2;
              c1.x = ReadFromArray<float32>(b);
              c1.y = ReadFromArray<float32>(b);
              c2.x = ReadFromArray<float32>(b);
              c2.y = ReadFromArray<float32>(b);
              g->Add(Instruction(c1, c2, e));
            }
          }
        }
      }
    }

    ///Attempts to load a typeface from a data block using the FreeType library.
    String ImportFromFontData(const byte* ByteArray,
      count LengthInBytes);

    ///Attempts to load a typeface file using the FreeType library.
    String ImportFromFontFile(const ascii* Filename)
    {
      String Result;
      String FileString = Filename;

      if(FileString.EndsWith(".dfont") || FileString.EndsWith(".suit"))
        Result = ImportFromFontData(
          reinterpret_cast<const byte*>(Filename), -1);
      else
      {
        Array<byte> ByteArray;
        File::Read(Filename, ByteArray);
        Result = ImportFromFontData(&ByteArray.a(), ByteArray.n());
      }
      return Result;
    }
  };

  ///Ordered collection of typefaces with a priority-based glyph lookup.
  class Font : public Value::Base
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

      //Notation is an alias for Special1
      Notation = Special1
    };

    private:

    ///Ordered array of styles.
    Array<Style> StyleTable;

    ///Ordered array of typefaces.
    Array<Pointer<Typeface> > TypefaceTable;

    public:

    ///Constructor to initialize empty font
    Font() {}

    ///Copy constructor
    Font(const Font& Other) : Value::Base() {*this = Other;}

    ///Assignment operator
    Font& operator = (const Font& Other)
    {
      StyleTable = Other.StyleTable;
      TypefaceTable = Other.TypefaceTable;
      return *this;
    }

    ///Clears the font.
    void Clear()
    {
      StyleTable.Clear();
      TypefaceTable.Clear();
    }

    /**Returns the underlying style without any font effects. Specifically,
    underline, superscript, subscript, and strikethrough are discarded.*/
    static Style TypefaceStyle(Style s)
    {
      return Style((s & 3) + (s & (256 * 7)));
    }

    ///Finds the closest matching character.
    Pointer<const Glyph> LookupGlyph(unicode c, Style PreferredStyle,
      count& TypefaceChosen) const
    {
      //First lookup by preferred style.
      for(TypefaceChosen = 0; TypefaceChosen < StyleTable.n(); TypefaceChosen++)
        if(TypefaceStyle(PreferredStyle) ==
          TypefaceStyle(StyleTable[TypefaceChosen]))
            if(Pointer<Glyph> g = TypefaceTable[TypefaceChosen]->LookupGlyph(c))
              return g;

      /*If the preferred style is unavailable for that character, ignore style
      and just look for the character.*/
      for(TypefaceChosen = 0; TypefaceChosen < StyleTable.n(); TypefaceChosen++)
        if(Pointer<Glyph> g = TypefaceTable[TypefaceChosen]->LookupGlyph(c))
          return g;

      //No glyph is available.
      TypefaceChosen = -1;
      return 0;
    }

    ///Locates a typeface of a given style and returns the typeface.
    Pointer<const Typeface> GetTypeface(Style StyleToLocate) const
    {
      for(count i = 0; i < StyleTable.n(); i++)
        if(TypefaceStyle(StyleToLocate) == TypefaceStyle(StyleTable[i]))
          return TypefaceTable[i];
      return 0;
    }

    ///Adds an empty typeface to the font.
    Pointer<Typeface> Add(Style StyleDescriptor)
    {
      StyleTable.Add() = StyleDescriptor;
      return TypefaceTable.Add().New();
    }

    ///Adds a typeface to the font.
    void Add(Style StyleDescriptor, Pointer<Typeface> Typeface_)
    {
      StyleTable.Add() = StyleDescriptor;
      TypefaceTable.Add() = Typeface_;
    }

    ///Adds the next highest priority typeface from an SVG string.
    void AddTypefaceFromSVGString(const String& s, Style StyleDescriptor)
    {
      Add(StyleDescriptor)->ImportFromSVGString(s);
    }

    ///Adds the next highest priority typeface from an SVG file.
    void AddTypefaceFromSVG(String Filename, Style StyleDescriptor)
    {
      String s;
      File::Read(Filename, s);
      AddTypefaceFromSVGString(s, StyleDescriptor);
    }

    ///Gets the number of typefaces.
    count n() const
    {
      return TypefaceTable.n();
    }

    ///Gets a particular typeface from the font.
    Pointer<const Typeface> ith(count i) const
    {
      return TypefaceTable[i];
    }

    ///Returns the style of the given typeface index.
    Style StyleOf(count i) const
    {
      return StyleTable[i];
    }

    ///Gets a particular typeface from the font.
    Pointer<const Typeface> operator [] (count i) const
    {
      return ith(i);
    }

    ///Virtual destructor
    virtual ~Font();
  };
}
//Add the Nothing<T> definition for Font::Style.
namespace prim
{
  template <> inline Nothing<belle::Font::Style>::operator
    belle::Font::Style() const {return belle::Font::Regular;}
}
#endif
