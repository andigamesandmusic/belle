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

#ifndef BELLE_ENGRAVER_HOUSESTYLE_H
#define BELLE_ENGRAVER_HOUSESTYLE_H

namespace BELLE_NAMESPACE
{
  /**House style for engraving. All fixed numbers used in the engraving process
  are stored here. The class can also be inherited to provide control the
  various magic numbers.*/
  class HouseStyle
  {
    ///Sets the house style defaults.
    static void SetDefaults(Value& HouseStyleKey)
    {
      //Height of a single space in inches
      HouseStyleKey["SpaceHeight"] = 0.055;

      //Distance between staves
      HouseStyleKey["StaffDistance"] = 12.0;

      //Default stem height when stems are not explicitly adjusted
      HouseStyleKey["DefaultStemHeight"] = 3.5;

      //Maximum number of dots to consider typesetting on a note
      HouseStyleKey["MaxDotsToConsider"] = 4.0;

      //Staff line thickness as fraction of space height
      HouseStyleKey["StaffLineThickness"] = 0.10;

      //Thickness of barlines
      HouseStyleKey["BarlineThickness"] = 0.18;

      //Thickness of thick barlines
      HouseStyleKey["ThickBarlineThickness"] = 0.56;

      //Black notehead width as a proportion of width to height
      HouseStyleKey["BlackNoteheadWidth"] = 1.33;

      //Black notehead angle
      HouseStyleKey["BlackNoteheadAngle"] = 23.0 * Deg<number>();

      //Black notehead size
      HouseStyleKey["BlackNoteheadSize"] = 1.05f;

      //White notehead width as a proportion of width to height
      HouseStyleKey["WhiteNoteheadWidth"] = 1.33;

      //White notehead angle
      HouseStyleKey["WhiteNoteheadAngle"] = 23.0 * Deg<number>();

      //White notehead size
      HouseStyleKey["WhiteNoteheadSize"] = 1.05f;

      //Notehead precise width when taking into account angle
      HouseStyleKey["BlackNoteheadWidthPrecise"] = Ellipse::VerticalTangent(
        +HouseStyleKey["BlackNoteheadWidth"] *
        +HouseStyleKey["BlackNoteheadSize"], HouseStyleKey["BlackNoteheadSize"],
        HouseStyleKey["BlackNoteheadAngle"]).x;

      //Notehead precise width when taking into account angle
      HouseStyleKey["WhiteNoteheadWidthPrecise"] = Ellipse::VerticalTangent(
        +HouseStyleKey["WhiteNoteheadWidth"] *
        +HouseStyleKey["WhiteNoteheadSize"], HouseStyleKey["WhiteNoteheadSize"],
        HouseStyleKey["WhiteNoteheadAngle"]).x;

      //Notehead precise width when taking into account angle
      HouseStyleKey["WholeNoteWidth"] = 1.7;

      //Note stem proportion
      HouseStyleKey["StemWidth"] = 0.14;

      //Note stem cap height
      HouseStyleKey["StemCapHeight"] = 0.8;

      //Note stem height in line spaces.
      HouseStyleKey["StemHeight"] = 6.8f;

      //Extra hanging width of ledger line end closest to stem
      HouseStyleKey["LedgerLineExtraInner"] = 0.25;

      //Extra hanging width of ledger line end furthest from stem
      HouseStyleKey["LedgerLineExtraOuter"] = 0.20;

      //Proportional amount to scrunch spacing on ledger lined notes
      HouseStyleKey["LedgerLineScrunch"] = 0.96;

      //Amount of optional gap between ledger lined notes and staff notes
      HouseStyleKey["LedgerLineGap"] = 0.1;

      //Relative thickness of ledger line to regular staff line.
      HouseStyleKey["LedgerLineRelativeThickness"] = 1.5f;

      //Relative thickness of ledger line to regular staff line.
      HouseStyleKey["LedgerLineExtension"] = 0.25f;

      //Amount of extra spacing in between accidentals
      HouseStyleKey["AccidentalExtraSpacing"] = 0.1;

      //Size of rhythmic dots
      HouseStyleKey["RhythmicDotSize"] = 0.5;

      //Distance from end of notehead to rhythmic dot
      HouseStyleKey["RhythmicDotNoteheadDistance"] = 0.6;

      //Spacing between rhythmic dots
      HouseStyleKey["RhythmicDotSpacing"] = 0.7;

      //Proportional size of a non initial clef
      HouseStyleKey["NonInitialClefSize"] = 0.8;

      //Staff-space distance between adjacent flags
      HouseStyleKey["FlagSeparation"] = 0.8;

      //Thickness of a beam
      HouseStyleKey["BeamThickness"] = 0.5;

      //Offsets to barline for staff brackets.
      HouseStyleKey["StaffBracketDistance"]["Brace"][0] = -1.7;
      HouseStyleKey["StaffBracketDistance"]["Brace"][1] = -2.8;
      HouseStyleKey["StaffBracketDistance"]["Brace"][2] = -3.3;
      HouseStyleKey["StaffBracketDistance"]["SquareBracket"] = -0.8;
      HouseStyleKey["StaffBracketDistance"]["ThinSquareBracket"][0] = -0.8;
      HouseStyleKey["StaffBracketDistance"]["ThinSquareBracket"][1] = -1.6;

      HouseStyleKey["SquareBracketThickness"] = 0.5;
      HouseStyleKey["SquareBracketExtent"] = 0.5;
      HouseStyleKey["SquareBracketCurlSquash"] = 0.8;

      {
        Value& MinimumDistances = HouseStyleKey["MinimumDistances"];
        MinimumDistances[mica::Barline][mica::Barline] = 10.f;
        MinimumDistances[mica::Barline][mica::Clef] = 0.f;
        MinimumDistances[mica::Barline][mica::Chord] = 1.f;
        MinimumDistances[mica::Barline][mica::KeySignature] = 0.5f;
        MinimumDistances[mica::Barline][mica::TimeSignature] = 0.5f;
        MinimumDistances[mica::Clef][mica::Barline] = 0.5f;
        MinimumDistances[mica::Clef][mica::Chord] = 1.f;
        MinimumDistances[mica::Clef][mica::KeySignature] = 0.8f;
        MinimumDistances[mica::Clef][mica::TimeSignature] = 0.5f;
        MinimumDistances[mica::Chord][mica::Barline] = 1.f;
        MinimumDistances[mica::Chord][mica::Chord] = 0.1f;
        MinimumDistances[mica::Chord][mica::Clef] = 0.f;
        MinimumDistances[mica::Chord][mica::KeySignature] = 1.f;
        MinimumDistances[mica::Chord][mica::TimeSignature] = 1.f;
        MinimumDistances[mica::KeySignature][mica::Barline] = 0.5f;
        MinimumDistances[mica::KeySignature][mica::Clef] = 0.f;
        MinimumDistances[mica::KeySignature][mica::Chord] = 1.f;
        MinimumDistances[mica::KeySignature][mica::TimeSignature] = 1.f;
        MinimumDistances[mica::TimeSignature][mica::Barline] = 1.f;
        MinimumDistances[mica::TimeSignature][mica::Clef] = 0.f;
        MinimumDistances[mica::TimeSignature][mica::Chord] = 1.f;
        MinimumDistances[mica::TimeSignature][mica::KeySignature] = 1.f;
        MinimumDistances["FrontMatterToChord"] = 2.f;
        MinimumDistances["TiedChord"] = 1.5f;
      }
    }

    ///Sets the default house style.
    static void Initialize(Value& HouseStyleKey, const Font& FontToUse,
      const String& Settings = "")
    {
      //Clear the house style.
      HouseStyleKey.Clear();

      //Set the house style defaults.
      SetDefaults(HouseStyleKey);

      //Override with any incoming settings.
      if(Settings)
      {
        Value v = JSON::Import(Settings);
        Array<Value> Keys;
        v.EnumerateKeys(Keys);
        for(count i = 0, n = Keys.n(); i < n; i++)
          HouseStyleKey[Keys[i]] = v[Keys[i]];
      }

      //Copy the incoming font.
      HouseStyleKey["NotationFont"] = new Font(FontToUse);

      //Create the cache from the house style and font.
      Cache::Initialize(HouseStyleKey["Cache"], HouseStyleKey,
        HouseStyleKey["NotationFont"].ConstObject());
    }

    public:

    ///Creates a new reference-counted house style.
    static Pointer<Value> Create(const Font& FontToUse,
      const String& Settings = "")
    {
      Pointer<Value> v = new Value;
      Initialize(*v, FontToUse, Settings);
      return v;
    }

    /**Returns value of a house style key. The local style is applied on top of
    the global style.*/
    static Value GetValue(Music::ConstNode Island, const String& Key)
    {
      if(!Island)
        return Value();

      if(Island->Label.SetState()["HouseStyle"]["Local"].Contains(Key))
        return Island->Label.SetState()["HouseStyle"]["Local"][Key];

      Pointer<Value::ConstReference> vr =
        Island->Label.SetState()["HouseStyle"]["Global"].Object();
      if(!vr)
        return Value();
      return vr->Get()[Key];
    }

    /**Returns the value of a house style multi-key. The local style is applied
    on top of the global style.*/
    static Value GetValue(Music::ConstNode Island, const String& Key1,
      const Value& Key2)
    {
      return GetValue(Island, Key1)[Key2];
    }

    /**Returns the value of a house style multi-key. The local style is applied
    on top of the global style.*/
    static Value GetValue(Music::ConstNode Island, const String& Key1,
      const Value& Key2, const Value& Key3)
    {
      return GetValue(Island, Key1)[Key2][Key3];
    }

    ///Returns the global house style on an island.
    static Pointer<const Value::ConstReference> GetGlobalHouseStyle(
      Music::ConstNode Island)
    {
      if(!Island)
        return Pointer<const Value::ConstReference>();
      return Island->GetState("HouseStyle", "Global").ConstObject();
    }

    ///Returns the local house style on an island.
    static Pointer<const Value::ConstReference> GetLocalHouseStyle(
      Music::ConstNode Island)
    {
      if(!Island)
        return Pointer<const Value::ConstReference>();
      return Island->GetState("HouseStyle", "Local").ConstObject();
    }

    ///Returns the notation font specified on the island.
    static Pointer<const Font> GetFont(Music::ConstNode Island)
    {
      if(!Island)
        return Pointer<const Font>();
      return HouseStyle::GetValue(Island, "NotationFont").ConstObject();
    }

    ///Returns the notation typeface specified on the island.
    static Pointer<const Typeface> GetTypeface(Music::ConstNode Island)
    {
      Pointer<const Font> FontToUse = GetFont(Island);
      if(!FontToUse)
        return Pointer<const Typeface>();
      return FontToUse->GetTypeface(Font::Notation);
    }

    ///Returns the notation cache.
    static Value GetCache(Music::ConstNode Island)
    {
      return GetGlobalHouseStyle(Island)->Get()["Cache"];
    }

    ///Returns a cached glyph.
    static Pointer<const Path> GetCached(Music::ConstNode Island,
      const String& Key)
    {
      return GetGlobalHouseStyle(Island)->Get()["Cache"][Key].ConstObject();
    }
  };
}
#endif
