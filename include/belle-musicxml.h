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

void          MusicXMLApplyArticulation(Music::Node Chord,
                const String& Articulation);
void          MusicXMLApplyLocalDirections(Music& G, Music::Node Chord,
                const Value& Direction);
void          MusicXMLBeaming(Pointer<Music> M);
void          MusicXMLConnectStavesAtLeft(Music& G);
Music::Node   MusicXMLConstructChord(Music& G, const Value& Chord,
                Value& PartState, count CurrentElementIndex);
Music::Node   MusicXMLConstructClef(Music& G, mica::Concept Clef);
void          MusicXMLConstructIslandsForStream(Music& G,
                Array<List<Value> >& Stream);
Music::Node   MusicXMLConstructKeySignature(Music& G,
                mica::Concept KeySignature, mica::Concept Mode);
void          MusicXMLConstructMeasureStream(Music& G, Value& PartState,
                Array<List<Value> >& Stream);
Music::Node   MusicXMLConstructStreamGeometry(Music& G,
                Array<List<Value> >& Stream, bool AddInitialBarline);
Music::Node   MusicXMLConstructTimeSignature(Music& G, Value t);
void          MusicXMLCreateSpans(Music& G, Value& PartState);
Ratio         MusicXMLDurationFromDivisions(Value Duration,
                const Value& PartState);
Ratio         MusicXMLDurationFromText(String Text);
List<String>  MusicXMLGetAttributeExcludes();
count         MusicXMLGetInstrumentalPartCount(const Value& M);
Array<String> MusicXMLGetInstrumentalPartIDs(const Value& M);
Array<String> MusicXMLGetInstrumentalPartNames(const Value& M);
count         MusicXMLGetMaximumNumberOfStaves(const Value& Measure);
const Value&  MusicXMLGetMeasure(const Value& M, count Part,
                count Measure);
count         MusicXMLGetMeasureCount(const Value& M);
count         MusicXMLGetMeasureCount(const Value& M, const String& ID);
const Value&  MusicXMLGetPart(const Value& M, const String& ID);
List<String>  MusicXMLGetTagExcludes();
void          MusicXMLInsertIntoStream(List<Value>& Stream, count Priority,
                Ratio Offset, Music::Node Island, Music::Node Chord);
void          MusicXMLLinkVoices(Music& G);
String        MusicXMLNewVoiceID();
Value         MusicXMLParseAttributes(const Value& Attributes,
                Value& PartState, bool IsFirstAttribute);
Value         MusicXMLParseBackup(const Value& Element, Value& PartState);
Value         MusicXMLParseBarline(const Value& Element);
mica::Concept MusicXMLParseClef(const Value& Attribute);
String        MusicXMLParseComplexTimeSignature(const Value& Attribute);
Value         MusicXMLParseDirection(const Value& Element);
Value         MusicXMLParseForward(const Value& Element, Value& PartState);
mica::Concept MusicXMLParseKeySignature(const Value& Attribute);
Music::Node   MusicXMLParseMeasure(Music& G, const Value& Measure,
                Value& PartState, bool AddInitialBarline, count MeasureIndex);
mica::Concept MusicXMLParseMode(const Value& Attribute);
Value         MusicXMLParseNote(const Value& Element, Value& PartState);
Value         MusicXMLParseNoteContext(const Value& Element);
Value         MusicXMLParseTimeSignature(const Value& Attribute);
void          MusicXMLPrintPartState(const Value& PartState);
bool          MusicXMLParseScore(Pointer<Music> M, const String& MusicXMLData,
                const Array<byte>& MusicXMLValidationZip);
void          MusicXMLPrintStream(const Array<List<Value> >& Stream);
Value         MusicXMLParseMeasureElements(const Value& Measure,
                Value& PartState);
void          MusicXMLRemoveMetadata(Music& G);
void          MusicXMLStitchMeasuresHorizontally(Music& G,
                Music::Node LeftMeasureRoot, Music::Node RightMeasureRoot);
void          MusicXMLStitchPartsVertically(Music& G,
                Music::Node TopPartRoot, Music::Node BottomPartRoot);
Value         MusicXMLValidate(String MusicXMLData,
                const Array<byte>& MusicXMLValidationZip,
                String AbsolutePathToUnzipTo);

#ifdef BELLE_IMPLEMENTATION
void MusicXMLApplyArticulation(Music::Node Chord, const String& Articulation)
{
  mica::Concept ArticulationType = mica::StaccatoWedge;
  if(Articulation == "staccato")
    ArticulationType = mica::Staccato;
  else if(Articulation == "tenuto")
    ArticulationType = mica::Tenuto;
  else if(Articulation == "accent")
    ArticulationType = mica::StandardAccent;
  else if(Articulation == "staccatissimo")
    ArticulationType = mica::StaccatoWedge;
  else if(Articulation == "strong-accent")
    ArticulationType = mica::StrongAccent;
  Chord->Set(ArticulationType) = ArticulationType;
}

void MusicXMLApplyLocalDirections(Music& G, Music::Node Chord,
  const Value& Direction)
{
  if(Direction["type"].AsString() == "expression")
    G.AddExpressionTo(Chord, Direction["text"],
      mica::Concept(Direction["placement"]));
  else if(Direction["type"].AsString() == "dynamic")
    G.AddExpressionTo(Chord, Direction["value"], mica::Below);
}

void MusicXMLBeaming(Pointer<Music> M)
{
  if(not M) return;
  Pointer<Geometry> G = System::MutableGeometry(M);
  G->Parse(*M);
  Array<Music::ConstNode> IslandBeginnings = G->GetPartBeginnings();
  List<List<List<Music::Node> > > SystemRanges;

  for(count i = 0; i < IslandBeginnings.n(); i++)
    SystemRanges.Push(SystemTimeSignatureRanges(M,
      IslandBeginnings[i]));

  bool NoTimeSignatures = false;
  for(count  i = 0; i < SystemRanges.n() and not NoTimeSignatures; i++)
    if(SystemRanges[i].n() == 0)
      NoTimeSignatures = true;

  if(not NoTimeSignatures)
  {
    ClearAllBeams(M, -1, -1);
    for(count i = 0; i < SystemRanges.n(); i++)
      MultivoiceBeamChange(M, SystemRanges[i]);
  }
}

void MusicXMLCreateSpans(Music& G, Value& PartState)
{
  Value& VoiceSpans = PartState["spans"];
  Value VoiceSpanTypes = VoiceSpans.Keys();
  for(count i = 0; i < VoiceSpanTypes.n(); i++)
  {
    String SpanType = VoiceSpanTypes[i].AsString();
    Value Voices = VoiceSpans[SpanType].Keys();
    for(count j = 0; j < Voices.n(); j++)
    {
      Value& SpanInfo = VoiceSpans[SpanType][Voices[j]];
      Value SpanNumbers = SpanInfo.Keys();
      for(count k = 0; k < SpanNumbers.n(); k++)
      {
        Value& SpanElements  = SpanInfo[SpanNumbers[k]];
        for(count m = 0; m < SpanElements.n(); m++)
        {
          Music::Node LeftChord  = SpanElements[m]["start"]["chord"].Object();
          Music::Node LeftNote   = SpanElements[m]["start"]["note"].Object();
          Music::Node RightChord = SpanElements[m]["stop"]["chord"].Object();
          Music::Node RightNote  = SpanElements[m]["stop"]["note"].Object();
          mica::Concept Placement = SpanElements[m]["start"]["placement"];
          if(SpanType == "tied" and LeftNote and RightNote)
          {
            if(Music::Edge e = G.Connect(LeftNote, RightNote))
              e->Set(mica::Type) = mica::Tie;
          }
          else if(SpanType == "slur" and LeftChord and RightChord)
          {
            if(Music::Edge e = G.Connect(LeftChord, RightChord))
              e->Set(mica::Type) = mica::Slur,
              e->Set(mica::Placement) = Placement;
          }
          else if(SpanType == "beam" and LeftChord and RightChord)
          {
            if(Music::Edge e = G.Connect(LeftChord, RightChord))
              e->Set(mica::Type) = mica::Beam;
          }
        }
      }
    }
  }
}

Array<String> MusicXMLGetInstrumentalPartNames(const Value& M)
{
  Array<String> Names;
  for(count i = 0; M.Contains(i); i++) if(M[i].Tag() == "part-list")
    for(count j = 0; M[i].Contains(j); j++) if(M[i][j].Tag() == "score-part")
      for(count k = 0; M[i][j].Contains(k); k++)
        if(M[i][j][k].Tag() == "part-name")
          Names.Add() = M[i][j][k].Val().AsString();
  return Names;
}

Array<String> MusicXMLGetInstrumentalPartIDs(const Value& M)
{
  Array<String> IDs;
  for(count i = 0; M.Contains(i); i++) if(M[i].Tag() == "part-list")
    for(count j = 0; M[i].Contains(j); j++) if(M[i][j].Tag() == "score-part")
      IDs.Add() = M[i][j]["id"].AsString();
  return IDs;
}

count MusicXMLGetInstrumentalPartCount(const Value& M)
{
  return MusicXMLGetInstrumentalPartNames(M).n();
}

const Value& MusicXMLGetPart(const Value& M, const String& ID)
{
  count NumberOfParts = 0;
  count FirstPartIndex = -1;
  for(count i = 0; M.Contains(i); i++)
  {
    if(M[i].Tag() == "part")
    {
      NumberOfParts++;
      if(FirstPartIndex < 0) FirstPartIndex = i;
      if(M[i]["id"] == Value(ID))
        return M[i];
    }
  }
  if(NumberOfParts == 1)
    return M[FirstPartIndex];
  return Value::Empty();
}

count MusicXMLGetMeasureCount(const Value& M, const String& ID)
{
  const Value& Part = MusicXMLGetPart(M, ID);
  count i = 0;
  for(; Part.Contains(i) and i < Part.n(); i++) {}
  return i;
}

count MusicXMLGetMeasureCount(const Value& M)
{
  Array<String> Parts = MusicXMLGetInstrumentalPartIDs(M);
  count MeasureCount = 0, CurrentMeasureCount;
  for(count i = 0; i < Parts.n(); i++)
    CurrentMeasureCount = MusicXMLGetMeasureCount(M, Parts[i]),
    MeasureCount = MeasureCount ? Min(CurrentMeasureCount, MeasureCount) :
      CurrentMeasureCount;
  return MeasureCount;
}

const Value& MusicXMLGetMeasure(const Value& M, count Part,
  count Measure)
{
  Array<String> Parts = MusicXMLGetInstrumentalPartIDs(M);
  return MusicXMLGetPart(M, Parts[Part])[Measure];
}

count MusicXMLGetMaximumNumberOfStaves(const Value& Measure)
{
  count Staves = 1;
  for(count i = 0; Measure.Contains(i); i++)
    for(count j = 0; Measure[i].Tag() == "attributes" and
      Measure[i].Contains(j); j++)
        if(Measure[i][j].Tag() == "staves")
          Staves = Max(Staves, Measure[i][j].Val().AsCount());
  return Staves;
}

mica::Concept MusicXMLParseClef(const Value& Attribute)
{
  String Sign;
  count StaffLine = 0;
  count Octaves = 0;

  for(count i = 0; Attribute.Contains(i); i++)
  {
    if(Attribute[i].Tag() == "sign")
      Sign = Attribute[i].Val().AsString();
    else if(Attribute[i].Tag() == "line")
      StaffLine = Attribute[i].Val().AsCount();
    else if(Attribute[i].Tag() == "clef-octave-change")
      Octaves = Attribute[i].Val().AsCount();
  }
  mica::Concept Result;
  (void)Octaves;
  if(Sign == "G" and StaffLine == 2)
    Result = mica::TrebleClef;
  else if(Sign == "F" and StaffLine == 4)
    Result = mica::BassClef;
  else if(Sign == "C" and StaffLine == 3)
    Result = mica::AltoClef;
  else if(Sign == "C" and StaffLine == 4)
    Result = mica::TenorClef;
  else
    Result = mica::TrebleClef;
  return Result;
}

String MusicXMLParseComplexTimeSignature(const Value& Attribute)
{
  String x;
  count GroupNumber = 0;
  bool FoundNonNumbers = false;
  for(count i = 0; Attribute.Contains(i); i++)
  {
    if(Attribute[i].Tag() == "beats")
    {
      if(GroupNumber++ > 0) x << ";+;";
      String y = Attribute[i].Val().AsString();
      y.Trim();
      if(y != String(count(y.ToNumber()))) FoundNonNumbers = true;
      x << y;
    }
    else if(Attribute[i].Tag() == "beat-type")
    {
      String y = Attribute[i].Val().AsString();
      y.Trim();
      if(y != String(count(y.ToNumber()))) FoundNonNumbers = true;
      x << "|" << y;
    }
  }
  if(GroupNumber == 1 and not FoundNonNumbers) x = "";
  return x;
}

Value MusicXMLParseTimeSignature(const Value& Attribute)
{
  Ratio Numerator, Denominator;
  bool SenzaMisura = false;
  for(count i = 0; Attribute.Contains(i); i++)
    if(Attribute[i].Tag() == "beats")
      Numerator = Ratio(Attribute[i].Val().AsCount());
    else if(Attribute[i].Tag() == "beat-type")
      Denominator = Ratio(1, Attribute[i].Val().AsCount());
    else if(Attribute[i].Tag() == "senza-misura")
      SenzaMisura = true;
  String Formula = MusicXMLParseComplexTimeSignature(Attribute);

  Value Result;
  String Symbol = Attribute["symbol"].AsString();
  if(SenzaMisura)
    Result.Add() = mica::OpenTimeSignature;
  else if(Formula)
  {
    Result.Add() = mica::AdditiveTimeSignature;
    Result.Add() = Formula;
  }
  else if(Numerator.IsDeterminate() or Denominator.IsDeterminate())
  {
    Result.Add() = Symbol == "single-number" ?
      mica::SingleNumberTimeSignature : mica::RegularTimeSignature;
    Result.Add() = mica::Concept(Ratio(4));
    Result.Add() = mica::Concept(Ratio(1, 4));
    if(Numerator.IsDeterminate()) Result[1] = mica::Concept(Numerator);
    if(Denominator.IsDeterminate()) Result[2] = mica::Concept(Denominator);
    if(Numerator == Ratio(4) and Denominator == Ratio(1, 4) and
      Symbol == "common")
        Result.NewArray(), Result.Add() = mica::CommonTime;
    if(Numerator == Ratio(2) and Denominator == Ratio(1, 2) and
      (Symbol == "common" or Symbol == "cut"))
        Result.NewArray(), Result.Add() = mica::CutTime;
  }
  else if(Symbol == "common")
    Result.Add() = mica::CommonTime;
  else if(Symbol == "cut")
    Result.Add() = mica::CutTime;
  else if(Symbol == "note" or
    Symbol == "dotted-note")
      Result.Add() = mica::Undefined;
  else
    Result.Add() = mica::Undefined;
  return Result;
}

mica::Concept MusicXMLParseKeySignature(const Value& Attribute)
{
  mica::Concept SharpsOrFlats;
  for(count i = 0; Attribute.Contains(i); i++)
    if(Attribute[i].Tag() == "fifths")
      SharpsOrFlats = mica::item(mica::KeySignatures, mica::NoAccidentals,
        Attribute[i].Val().AsCount());
  return SharpsOrFlats;
}

mica::Concept MusicXMLParseMode(const Value& Attribute)
{
  mica::Concept Mode;
  for(count i = 0; Attribute.Contains(i); i++)
    if(Attribute[i].Tag() == "mode")
      Mode = mica::named(String("en:") +
        Attribute[i].Val().AsString().ToTitle());
  return Mode;
}

Value MusicXMLParseAttributes(
  const Value& Attributes, Value& PartState, bool IsFirstAttribute)
{
  count Staves = PartState["staves"].AsCount();
  Value CKTMatrix;
  for(count i = 0; Attributes.Contains(i); i++)
  {
    const Value& Attribute = Attributes[i];
    if(Attribute.Tag() == "divisions")
      PartState["divisions"] = Attribute.Val();
    else if(Attribute.Tag() == "clef")
    {
      count StaffIndex = 0;
      if(Attribute["number"].IsInteger())
        StaffIndex = Min(Max(Attribute["number"].AsCount() - 1,
          count(0)), Staves);
      CKTMatrix[0][StaffIndex] = MusicXMLParseClef(Attribute);
    }
    else if(Attribute.Tag() == "key")
    {
      count StaffIndex = -1;
      if(Attribute["number"].IsInteger())
        StaffIndex = Min(Max(Attribute["number"].AsCount() - 1,
          count(0)), Staves);
      Value Key;
      Key.Add() = MusicXMLParseKeySignature(Attribute);
      Key.Add() = MusicXMLParseMode(Attribute);
      for(count j = 0; j < Staves; j++)
        if(StaffIndex < 0 or StaffIndex == j)
          CKTMatrix[1][j] = Key;
    }
    else if(Attribute.Tag() == "time")
    {
      count StaffIndex = -1;
      if(Attribute["number"].IsInteger())
        StaffIndex = Min(Max(Attribute["number"].AsCount() - 1,
          count(0)), Staves);
      Value TimeSignature = MusicXMLParseTimeSignature(Attribute);
      for(count j = 0; j < Staves; j++)
        if(StaffIndex < 0 or StaffIndex == j)
          CKTMatrix[2][j] = TimeSignature;
    }
    else if(Attribute.Tag() == "measure-style")
    {
      for(count j = 0; Attribute.Contains(j); j++)
      {
        const Value& SubAttribute = Attribute[j];
        if(SubAttribute.Tag() == "multiple-rest")
        {
          count MultipleRest = SubAttribute.Val().AsCount();
          if(MultipleRest > 0)
          {
            PartState["multiple-rest-count"] = MultipleRest;
            PartState["multiple-rest-remaining"] = MultipleRest;
          }
        }
      }
    }
  }
  if(IsFirstAttribute)
    for(count i = 0; i < Staves; i++)
      if(CKTMatrix[0][i].IsNil())
        CKTMatrix[0][i] = mica::TrebleClef;
  return CKTMatrix;
}

Ratio MusicXMLDurationFromDivisions(Value Duration, const Value& PartState)
{
  return Ratio(Duration.AsCount(), (PartState["divisions"].IsInteger() ?
    PartState["divisions"].AsCount() * 1 : count(1)) * 4);
}

Ratio MusicXMLDurationFromText(String Text)
{
  Text = Text.ToLower();
  Text.Trim();
  return  Text.StartsWith("10") ? Ratio(1, 1024) :
          Text.StartsWith("51") ? Ratio(1,  512) :
          Text.StartsWith("25") ? Ratio(1,  256) :
          Text.StartsWith("12") ? Ratio(1,  128) :
          Text.StartsWith("64") ? Ratio(1,   64) :
          Text.StartsWith("32") ? Ratio(1,   32) :
          Text.StartsWith("16") ? Ratio(1,   16) :
          Text.StartsWith("ei") ? Ratio(1,    8) :
          Text.StartsWith("qu") ? Ratio(1,    4) :
          Text.StartsWith("ha") ? Ratio(1,    2) :
          Text.StartsWith("wh") ? Ratio(1,    1) :
          Text.StartsWith("br") ? Ratio(2,    1) :
          Text.StartsWith("lo") ? Ratio(4,    1) :
          Text.StartsWith("ma") ? Ratio(8,    1) : Ratio();
}

String MusicXMLNewVoiceID()
{
  return UUIDv4().AsHash();
}

Value MusicXMLParseBackup(const Value& Element, Value& PartState)
{
  Value v;
  v["type"] = "backup";
  v["duration"] = MusicXMLDurationFromDivisions(Element[0].Val(), PartState);
  PartState["current-voice"] = MusicXMLNewVoiceID();
  return v;
}

Value MusicXMLParseBarline(const Value& Element)
{
  Value v;
  v["type"] = "barline";

  {
    String Location = Element["location"].AsString();
    v["location"] = Location == "left"   ? "measure-beginning" :
                    Location == "middle" ? "follows-offset" : "measure-ending";
  }

  {
    mica::Concept BarlineStyle = mica::StandardBarline;
    for(count i = 0; Element.Contains(i); i++)
    {
      String t = Element[i].Tag();
      String s = Element[i].Val().AsString();
      if(t == "bar-style" and BarlineStyle == mica::StandardBarline)
        BarlineStyle =
          s == "dashed"      ? mica::DashedBarline :
          s == "dotted"      ? mica::DottedBarline :
          s == "heavy"       ? mica::ThickBarline :
          s == "heavy-heavy" ? mica::ThickDoubleBarline :
          s == "heavy-light" ? mica::ThickThinBarline :
          s == "light-heavy" ? mica::FinalBarline :
          s == "light-light" ? mica::ThinDoubleBarline :
          s == "none"        ? mica::Undefined :
          s == "regular"     ? mica::StandardBarline :
          s == "short"       ? mica::ShortBarline :
          s == "tick"        ? mica::TickBarline : mica::StandardBarline;
      else if(t == "repeat")
        BarlineStyle = Element[i]["direction"].AsString() == "forward" ?
          mica::BeginRepeatBarline : mica::EndRepeatBarline;
    }
    v["style"] = BarlineStyle;
  }
  return v;
}

Value MusicXMLParseDirection(const Value& Element)
{
  Value v;
  for(count i = 0; Element.Contains(i); i++)
  {
    String t = Element[i].Tag();
    if(t == "voice")
      v["voice"] = MD5::Hex(Element[i].Val());
    else if(t == "staff")
      v["staff"] = Element[i].Val().AsCount();
    else if(t == "direction-type")
    {
      for(count j = 0; Element[i].Contains(j); j++)
      {
        String dt = Element[i][j].Tag();
        if(dt == "words")
        {
          Value w;
          w["type"] = "expression";
          w["style"] = Element[i][j]["font-style"];
          w["placement"] = Element[i][j]["default-y"].AsNumber() > 0.f ?
            mica::Above : mica::Below;
          w["text"] = Element[i][j].Val();
          v["directions"].Add() = w;
        }
        else if(dt == "dynamics")
        {
          String DynamicMark;
          for(count k = 0; Element[i][j].Contains(k); k++)
            DynamicMark << Element[i][j][k].Tag();
          Value w;
          w["type"] = "dynamic";
          w["value"] = DynamicMark;
          v["directions"].Add() = w;
        }
      }
    }
  }
  return v;
}

Value MusicXMLParseForward(const Value& Element, Value& PartState)
{
  Value v;
  v["type"] = "forward";
  v["duration"] = Ratio(0);
  for(count i = 0; Element.Contains(i); i++)
    if(Element[i].Tag() == "duration")
      v["duration"] = MusicXMLDurationFromDivisions(
        Element[i].Val(), PartState);
    else if(Element[i].Tag() == "voice")
      v["voice"] = MD5::Hex(Element[i].Val());
    else if(Element[i].Tag() == "staff")
      v["staff"] = Element[i].Val().AsCount();
  if(v.Contains(Value("voice")))
    PartState["current-voice"] = v["voice"];
  if(v.Contains(Value("staff")))
    PartState["current-staff"] = v["staff"];
  return v;
}

Value MusicXMLParseNoteContext(const Value& Element)
{
  Value v;
  for(count i = 0; Element.Contains(i); i++)
  {
    String t = Element[i].Tag();
    if(t == "chord" or t == "cue" or t == "grace" or t == "time-modification")
      v[t] = true;
  }
  return v;
}

Music::Node MusicXMLConstructChord(Music& G, const Value& Chord,
  Value& PartState, count CurrentElementIndex)
{
  count CurrentMeasureIndex = PartState["current-measure"].AsCount();
  Music::Node c = G.CreateToken(mica::Chord);
  c->Label.Set(mica::NoteValue) = Chord["notated-type"].AsRatio();
  c->Label.Set("MusicXMLVoice") = Chord["voice"].AsString();
  Value& VoiceSpans = PartState["spans"];
  Value VoiceSpanTypes = VoiceSpans.Keys();
  for(count i = 0; i < Chord["articulations"].n(); i++)
  {
    String a = Chord["articulations"][i].AsString();
    MusicXMLApplyArticulation(c, a);
  }
  {
    Value Original = Chord["directions"];
    for(count i = 0; i < Original.n(); i++)
      if(not Original[i].Contains("voice") or
        Original[i]["voice"] == Chord["voice"])
        for(count j = 0; j < Original[i]["directions"].n(); j++)
          MusicXMLApplyLocalDirections(G, c, Original[i]["directions"][j]);
  }
  for(count i = 0; i < Chord["pitches"].n(); i++)
  {
    Music::Node n = G.CreateAndAddNote(c, mica::Concept(Chord["pitches"][i]));
    if(not Chord["accidentals"][i].IsNil())
      n->Set(mica::Accidental) = mica::Concept(Chord["accidentals"][i]);
    for(count j = 0; j < VoiceSpanTypes.n(); j++)
    {
      Value& SpanType = VoiceSpans[VoiceSpanTypes[j]][Chord["voice"]];
      Value Numbers = SpanType.Keys();
      for(count m = 0; m < Numbers.n(); m++)
      {
        Value& SpanInfo = SpanType[Numbers[m]];
        for(count k = 0; k < SpanInfo.n(); k++)
          if(SpanInfo[k]["start"]["element"].AsCount() == CurrentElementIndex
            and SpanInfo[k]["start"]["pitch"].AsCount() == i and
            SpanInfo[k]["start"]["measure"].AsCount() == CurrentMeasureIndex)
              SpanInfo[k]["start"]["chord"] = c,
              SpanInfo[k]["start"]["note"] = n;
        for(count k = 0; k < SpanInfo.n(); k++)
          if(SpanInfo[k]["stop"]["element"].AsCount() == CurrentElementIndex
            and SpanInfo[k]["stop"]["pitch"].AsCount() == i and
            SpanInfo[k]["stop"]["measure"].AsCount() == CurrentMeasureIndex)
              SpanInfo[k]["stop"]["chord"] = c,
              SpanInfo[k]["stop"]["note"] = n;
      }
    }
  }
  return c;
}

Music::Node MusicXMLConstructClef(Music& G, mica::Concept Clef)
{
  return mica::undefined(Clef) ? Music::Node() : G.CreateAndAddClef(Clef);
}

Music::Node MusicXMLConstructKeySignature(Music& G,
  mica::Concept KeySignature, mica::Concept Mode)
{
  return mica::undefined(KeySignature) ? Music::Node() :
    G.CreateAndAddKeySignature(KeySignature, Mode);
}

Music::Node MusicXMLConstructTimeSignature(Music& G, Value t)
{
  mica::Concept k = mica::Concept(t[0]);
  if(not t.n() or mica::undefined(k))
    return Music::Node();
  else if(k == mica::RegularTimeSignature or
    k == mica::SingleNumberTimeSignature)
      return G.CreateAndAddTimeSignature(t[0], t[1].AsCount(), t[2].AsRatio());
  else if(k == mica::AdditiveTimeSignature)
    return G.CreateAndAddTimeSignatureFormula(t[1].AsString());
  return G.CreateAndAddTimeSignature(t[0]);
}

void MusicXMLInsertIntoStream(List<Value>& Stream,
  count Priority, Ratio Offset, Music::Node Island, Music::Node Chord)
{
  count i;
  for(i = 0; i < Stream.n(); i++)
  {
    if(Offset < Stream[i]["offset"].AsRatio() or
      (Offset == Stream[i]["offset"].AsRatio() and
      Priority < Stream[i]["priority"].AsCount()))
        break;
  }
  Value v;
  v["priority"] = Priority;
  v["offset"] = Offset;
  v["island"] = Island;
  v["chord"] = Chord;
  Stream.InsertBefore(v, i);
}

void MusicXMLConstructIslandsForStream(Music& G,
  Array<List<Value> >& Stream)
{
  Ratio CurrentOffset;
  for(count i = 0; i < Stream.n(); i++)
  {
    Music::Node Island;
    for(count j = 0; j < Stream[i].n(); j++)
    {
      if(Stream[i][j]["island"].IsNil())
      {
        if(not Island or Stream[i][j]["offset"].AsRatio() != CurrentOffset)
        {
          Island = G.CreateIsland();
          CurrentOffset = Stream[i][j]["offset"].AsRatio();
        }
        G.AddTokenToIsland(Island, Stream[i][j]["chord"].Object());
        Stream[i][j]["island"] = Island;
      }
    }
  }
}

Value MusicXMLParseNote(const Value& Element, Value& PartState)
{
  static const bool WarnOnCrossStaffBeaming = true;
  Value Exception;

  Value Context = MusicXMLParseNoteContext(Element);
  if(Context.Contains(String("time-modification")))
  {
    Exception["musicxml-parser-exception"] =
      "Score contains a time-modification element (tuplet) that is not "
      "currently supported by the parser.";
    return Exception;
  }

  bool IsChordNote = true;
  if(not Context.Const()["chord"])
  {
    IsChordNote = false;
    PartState["elements"].Add() = Context;
    PartState["elements"].z()["staff"] = 1;
    PartState["elements"].z()["type"] = "chord";
  }
  Value& NoteInfo = PartState["elements"].z();

  if(not PartState["active-directions"].IsNil())
  {
    NoteInfo["directions"] = PartState["active-directions"];
    PartState["active-directions"] = Value();
  }

  count DotCount = 0;
  bool AlreadySawBeam = false;
  for(count i = 0; Element.Contains(i); i++)
  {
    String t = Element[i].Tag();
    if(t == "pitch")
    {
      mica::Concept Letter, Accidental = mica::Natural, Octave;
      for(count j = 0; Element[i].Contains(j); j++)
      {
        String tt = Element[i][j].Tag();
        Value vv = Element[i][j].Val();
        if(tt == "step")
          Letter = mica::named(String("en:") + vv.AsString());
        else if(tt == "alter")
        {
          count Quartertone = count(vv.AsNumber() * 2.f);
          Accidental =
            Quartertone == -6 ? mica::TripleFlat    :
            Quartertone == -4 ? mica::DoubleFlat    :
            Quartertone == -3 ? mica::FlatAndAHalf  :
            Quartertone == -2 ? mica::Flat          :
            Quartertone == -1 ? mica::HalfFlat      :
            Quartertone ==  0 ? mica::Natural       :
            Quartertone ==  1 ? mica::HalfSharp     :
            Quartertone ==  2 ? mica::Sharp         :
            Quartertone ==  3 ? mica::SharpAndAHalf :
            Quartertone ==  4 ? mica::DoubleSharp   :
            Quartertone ==  6 ? mica::TripleSharp   : mica::Undefined;
        }
        else if(tt == "octave")
          Octave = mica::Concept(Ratio(vv.AsCount()));
      }
      mica::Concept Pitch = mica::map(Letter, Accidental, Octave);
      if(Pitch == mica::Undefined)
        Pitch = mica::map(Letter, Octave);
      else
        Accidental = mica::Undefined;
      NoteInfo["pitches"].Add() =  Pitch;
      NoteInfo["accidentals"].Add() = Accidental;
    }
    else if(t == "duration")
    {
      Ratio x = MusicXMLDurationFromDivisions(Element[i].Val(), PartState);

      if(NoteInfo["duration"].IsRatio())
        NoteInfo["duration"] = Max(NoteInfo["duration"].AsRatio(), x);
      else
        NoteInfo["duration"] = x;
    }
    else if(t == "notations")
    {
      if(NoteInfo["voice"].IsNil())
        NoteInfo["voice"] = PartState["current-voice"];
      Value SpanInfo;
      SpanInfo["element"] = (PartState["elements"].n() - 1);
      SpanInfo["measure"] = PartState["current-measure"];
      SpanInfo["pitch"] = (NoteInfo["pitches"].n() - 1);
      for(count j = 0; Element[i].Contains(j); j++)
      {
        String tt = Element[i][j].Tag();
        Value  vv = Element[i][j].Val();
        String ty = Element[i][j]["type"].AsString();
        String nu = Element[i][j]["number"].AsString();
        String pl = Element[i][j]["placement"].AsString();
        Value& VoiceSpans = PartState["spans"][tt][NoteInfo["voice"]];
        SpanInfo["placement"] =
          pl == "above" ? mica::Above :
          pl == "below" ? mica::Below : mica::Undefined;
        if(tt == "tied" or tt == "slur")
        {
          if(ty == "start")
            VoiceSpans[nu].Add()["start"] = SpanInfo;
          else
            VoiceSpans[nu].z()["stop"] = SpanInfo;
        }
        else if(tt == "articulations")
          for(count k = 0; Element[i][j].Contains(k); k++)
            NoteInfo["articulations"].Add() = Element[i][j][k].Tag();
      }
    }
    else if(t == "voice")
      NoteInfo["voice"] = MD5::Hex(Element[i].Val().AsString());
    else if(t == "staff")
      NoteInfo["staff"] = Element[i].Val().AsCount();
    else if(t == "rest")
      NoteInfo["pitches"].NewArray(), NoteInfo["accidentals"].NewArray();
    else if(t == "type")
      NoteInfo["notated-type"] = MusicXMLDurationFromText(
        Element[i].Val().AsString());
    else if(t == "dot")
      DotCount++;
    else if(t == "beam" and not IsChordNote and not AlreadySawBeam)
    {
      String Number = Element[i]["number"].IsNil() ? String("1") :
                      Element[i]["number"].AsString();
      Number = "1";
      if(Number == "1")
      {
        AlreadySawBeam = true;
        String BeamContext = Element[i].Val().AsString();
        if(NoteInfo["voice"].IsNil())
          NoteInfo["voice"] = PartState["current-voice"];
        Value SpanInfo;
        SpanInfo["element"] = (PartState["elements"].n() - 1);
        SpanInfo["measure"] = PartState["current-measure"];
        SpanInfo["pitch"] = (NoteInfo["pitches"].n() - 1);
        Value& VoiceSpans = PartState["spans"]["beam"][NoteInfo["voice"]];
        if(BeamContext.Contains("begin"))
        {
          VoiceSpans[Number].Add()["start"] = SpanInfo;
          PartState["must-close-beam"] = true;
        }
        else if(BeamContext.Contains("continue"))
        {
          VoiceSpans[Number].z()["stop"] = SpanInfo;
          VoiceSpans[Number].Add()["start"] = SpanInfo;
          PartState["must-close-beam"] = true;
        }
        else
        {
          VoiceSpans[Number].z()["stop"] = SpanInfo;
          PartState["must-close-beam"] = false;
        }
      }
    }
  }

  if(not AlreadySawBeam and not IsChordNote and
    PartState["must-close-beam"].AsBoolean())
  {
    String Number = "1";
    if(NoteInfo["voice"].IsNil())
      NoteInfo["voice"] = PartState["current-voice"];
    Value SpanInfo;
    SpanInfo["element"] = (PartState["elements"].n() - 1);
    SpanInfo["measure"] = PartState["current-measure"];
    SpanInfo["pitch"] = (NoteInfo["pitches"].n() - 1);
    Value& VoiceSpans = PartState["spans"]["beam"][NoteInfo["voice"]];
    VoiceSpans[Number].z()["stop"] = SpanInfo;
    PartState["must-close-beam"] = false;
  }

  NoteInfo["notated-dots"] = DotCount;
  NoteInfo["notated-type"] =
    NoteInfo["notated-type"].AsRatio() * DotScale(DotCount);
  if(NoteInfo["notated-type"].IsNil())
    NoteInfo["notated-type"] = NoteInfo["duration"];
  if(NoteInfo["duration"].IsNil())
    NoteInfo["duration"] = 0;

  if(NoteInfo["voice"].IsNil())
    NoteInfo["voice"] = PartState["current-voice"];
  PartState["current-voice"] = NoteInfo["voice"];

  if(NoteInfo["staff"].IsNil())
  {
    if(PartState["current-staff-for-voice"][NoteInfo["voice"]].IsNil())
      NoteInfo["staff"] = PartState["current-staff"];
    else
      NoteInfo["staff"] =
        PartState["current-staff-for-voice"][NoteInfo["voice"]];
  }
  else
    PartState["current-staff"] = NoteInfo["staff"];

  if(WarnOnCrossStaffBeaming)
  {
    Value v = NoteInfo["voice"];
    Value s = NoteInfo["staff"];
    if(not PartState["current-staff-for-voice"][v].IsNil())
    {
      if(PartState["current-staff-for-voice"][v] != s)
      {
        Exception["musicxml-parser-exception"] = String(
          "MusicXML parser detected cross-staff beaming. This feature is "
          "currently not implemented.");
      }
    }
  }
  PartState["current-staff-for-voice"][NoteInfo["voice"]] = NoteInfo["staff"];
  return Exception;
}

void MusicXMLPrintPartState(const Value& PartState)
{
  C::Out() >> JSON::Export(PartState);
}

void MusicXMLPrintStream(const Array<List<Value> >& Stream)
{
  for(count i = 0; i < Stream.n(); i++)
  {
    C::Out() >> "++++++";
    for(count j = 0; j < Stream[i].n(); j++)
      C::Out() >> "------" >> JSON::Export(Stream[i][j]);
  }
}

void MusicXMLConstructMeasureStream(Music& G, Value& PartState,
  Array<List<Value> >& Stream)
{
  count Staves = PartState["staves"].AsCount();
  Stream.Clear();
  Stream.n(Staves);
  count Priority = 0;
  Ratio Offset = 0;
  for(count i = 0; i < PartState["elements"].n(); i++)
  {
    const Value& e = PartState["elements"][i];
    if(e["type"].AsString() == "attribute")
    {
      for(count t = 0; t < 3; t++)
      {
        Music::Node Previous, Current;
        for(count j = 0; j < Staves; j++)
        {
          Previous = Current;
          if(t == 0)
            Current = MusicXMLConstructClef(G, e["attributes"][t][j]);
          else if(t == 1)
            Current = MusicXMLConstructKeySignature(
              G, e["attributes"][t][j][0], e["attributes"][t][j][1]);
          else if(t == 2)
            Current = MusicXMLConstructTimeSignature(G, e["attributes"][t][j]);
          if(Current)
          {
            MusicXMLInsertIntoStream(Stream[j], Priority++, Offset, Current,
              Music::Node());
            if(Previous)
              G.Connect(Previous, Current)->Set(mica::Type) = mica::Instantwise;
          }
        }
      }
    }
    else if(e["type"].AsString() == "chord")
    {
      Music::Node c = MusicXMLConstructChord(G, e, PartState, i);
      Offset = e["offset"].AsRatio();
      count Staff = e["staff"].AsCount() - 1;
      MusicXMLInsertIntoStream(Stream[Staff], Priority++, Offset,
        Music::Node(), c);
    }
    else if(e["type"].AsString() == "barline" and
      mica::Concept(e["style"]) != mica::Undefined)
    {
      Offset = e["offset"].AsRatio();

      Music::Node Previous, Current;
      for(count j = 0; j < Staves; j++)
      {
        Previous = Current;

        Current = G.CreateAndAddBarline(e["style"]);
        Current->Set("MusicXMLBarlineStitch") = "true";
        if(j < Staves - 1)
          Current->Set("StaffConnects") = "true";
        Current->Set("StaffLines") = "5";
        Current->Set("StaffScale") = "1.0";
        Current->Set("StaffOffset") = String(float(j) * -12.f);

        MusicXMLInsertIntoStream(Stream[j], Priority++, Offset, Current,
          Music::Node());
        if(Previous)
          G.Connect(Previous, Current)->Set(mica::Type) = mica::Instantwise;
      }
    }
  }
}

Value MusicXMLParseMeasureElements(const Value& Measure, Value& PartState)
{
  PartState["current-staff"] = 1;
  PartState["current-voice"] = MusicXMLNewVoiceID();
  PartState["elements"].NewArray();
  if(PartState["staves"].IsNil())
    PartState["staves"] = 1;
  PartState["staves"] = Max(PartState["staves"].AsCount(),
    MusicXMLGetMaximumNumberOfStaves(Measure));
  PartState["skipped-measure"] = false;

  Ratio Offset = 0;
  bool IsFirstAttribute = true;
  bool IsFirstMeasure = not PartState["current-measure"].AsCount();

  if(PartState.Contains("multiple-rest-count"))
  {
    count Remaining = PartState["multiple-rest-remaining"].AsCount() - 1;
    if(Remaining > 0)
    {
      PartState["multiple-rest-remaining"] = Remaining,
      PartState["skipped-measure"] = true;
    }
    else
    {
      PartState["multiple-rest-count"] = Value();
      PartState["multiple-rest-remaining"] = Value();
      PartState.Prune();
    }
  }

  if(not PartState["skipped-measure"].AsBoolean())
  {
    if(IsFirstMeasure)
    {
      bool DoesNotHaveInitialAttributes = true;
      for(count i = 0; Measure.Contains(i); i++)
        if(Measure[i].Tag() == "attributes")
          DoesNotHaveInitialAttributes = false;
      if(DoesNotHaveInitialAttributes)
      {
        PartState["elements"].Add()["type"] = "attribute";
        PartState["elements"].z()["attributes"] =
          MusicXMLParseAttributes(Value(), PartState, true);
        PartState["elements"].z()["offset"] = Offset;
      }
    }

    for(count i = 0; Measure.Contains(i); i++)
    {
      const Value& Element = Measure[i];
      if(PartState.Contains("multiple-rest-count"))
      {
        //i.e. Do not process remaining tags if a multiple rest.
      }
      else if(Element.Tag() == "attributes")
      {
        PartState["elements"].Add()["type"] = "attribute";
        PartState["elements"].z()["attributes"] =
          MusicXMLParseAttributes(Element, PartState,
            IsFirstMeasure and IsFirstAttribute);
        PartState["elements"].z()["offset"] = Offset;
        IsFirstAttribute = false;
      }
      else if(Element.Tag() == "note")
      {
        Value Context = MusicXMLParseNoteContext(Element);
        if(not Context.Contains("grace"))
        {
          Ratio OriginalDuration = 0;
          count OriginalElementCount = PartState["elements"].n();
          if(PartState["elements"].z().Const()["duration"].IsRatio())
            OriginalDuration =
              PartState["elements"].z().Const()["duration"].AsRatio();

          Value Exception = MusicXMLParseNote(Element, PartState);
          if(not Exception.IsNil())
            return Exception;

          if(PartState["elements"].n() == OriginalElementCount)
            Offset -= OriginalDuration;

          PartState["elements"].z()["offset"] = Offset;
          Offset += PartState["elements"].z()["duration"].AsRatio();
        }
      }
      else if(Element.Tag() == "forward")
      {
        PartState["must-close-beam"] = false;
        PartState["elements"].Add() = MusicXMLParseForward(Element, PartState);
        Offset += PartState["elements"].z()["duration"].AsRatio();
        Offset = Max(Offset, Ratio(0));
      }
      else if(Element.Tag() == "backup")
      {
        PartState["must-close-beam"] = false;
        PartState["elements"].Add() = MusicXMLParseBackup(Element, PartState);
        Offset -= PartState["elements"].z()["duration"].AsRatio();
        Offset = Max(Offset, Ratio(0));
      }
      else if(Element.Tag() == "barline")
      {
        PartState["must-close-beam"] = false;
        PartState["elements"].Add() = MusicXMLParseBarline(Element);
        PartState["elements"].z()["offset"] = Offset;
      }
      else if(Element.Tag() == "direction")
      {
        Value Result = MusicXMLParseDirection(Element);
        if(not Result.IsNil())
          PartState["active-directions"].Add() = Result;
      }
    }

    if(PartState["elements"].n() and
      PartState["elements"].z()["type"].AsString() != "barline")
    {
      Value v;
      v["type"] = "barline";
      v["location"] = "measure-ending";
      v["style"] = mica::StandardBarline;
      PartState["elements"].Add() = v;
    }
  }
  return Value();
}

Music::Node MusicXMLConstructStreamGeometry(Music& G,
  Array<List<Value> >& Stream, bool AddInitialBarline)
{
  Music::Node PreviousInitialChord;
  for(count i = 0; i < Stream.n(); i++)
  {
    bool EncounteredInitialChord = false;

    if(AddInitialBarline)
    {
      Music::Node LeftBarline = G.CreateAndAddBarline();
      LeftBarline->Set("MusicXMLBarlineStitch") = "true";
      LeftBarline->Set("StaffConnects") = "true";
      LeftBarline->Set("StaffLines") = "5";
      LeftBarline->Set("StaffScale") = "1.0";
      LeftBarline->Set("StaffOffset") = String(float(i) * -12.f);
      Value v;
      v["island"] = LeftBarline;
      Stream[i].Prepend(v);
      if(i) G.Connect(Stream[i - 1].a()["island"].Object(),
        Stream[i].a()["island"].Object())->Set(mica::Type) = mica::Instantwise;
      if(Stream.n() > 1 and i == Stream.n() - 1)
      {
        Music::Node First = Stream.a().a()["island"].Object();
        Music::Node Last  = Stream.z().a()["island"].Object();
        G.Connect(First, Last)->Set(mica::StaffBracket) = mica::Brace;
      }
    }

    for(count j = 1; j < Stream[i].n(); j++)
    {
      Music::Node Left = Stream[i][j - 1]["island"].Object();
      Music::Node Right = Stream[i][j]["island"].Object();
      if(Left and Right and Left != Right)
        G.Connect(Left, Right)->Set(mica::Type) = mica::Partwise;
      if(Left and ChordsOfIsland(Left).n() > 0 and not EncounteredInitialChord)
      {
        EncounteredInitialChord = true;
        Left->Set("MusicXMLInitialChordStitch") = "true";
        if(PreviousInitialChord)
          G.Connect(PreviousInitialChord, Left)->Set(mica::Type) =
            mica::Instantwise;
        PreviousInitialChord = Left;
      }
    }
  }
  return (Stream.n() and Stream.a().n()) ?
    Music::Node(Stream.a().a()["island"].Object()) : Music::Node();
}

void MusicXMLStitchMeasuresHorizontally(Music& G,
  Music::Node LeftMeasureRoot, Music::Node RightMeasureRoot)
{
  Music::Node LeftMeasureEnd = LeftMeasureRoot;
  while(LeftMeasureEnd->Next(MusicLabel(mica::Partwise)))
    LeftMeasureEnd = LeftMeasureEnd->Next(MusicLabel(mica::Partwise));

  Array<Music::Node> LeftMeasureJoin =
    LeftMeasureEnd->Series(MusicLabel(mica::Instantwise));
  Array<Music::Node> RightMeasureJoin =
    RightMeasureRoot->Series(MusicLabel(mica::Instantwise));

  for(count i = 0; i < Min(LeftMeasureJoin.n(), RightMeasureJoin.n()); i++)
    G.Connect(LeftMeasureJoin[i], RightMeasureJoin[i])->Set(mica::Type) =
      mica::Partwise;
}

void MusicXMLConnectStavesAtLeft(Music& G)
{
  Music::Node x = G.Root();
  count i = 0;
  while(x)
  {
    x->Set("StaffConnects") = "true";
    x->Set("StaffOffset") = String(number(i) * -12.f);
    x = x->Next(MusicLabel(mica::Instantwise));
    i++;
  }
}

void MusicXMLStitchPartsVertically(Music& G,
  Music::Node TopPartRoot, Music::Node BottomPartRoot)
{
  Music::Node TopPartBottom = TopPartRoot;
  while(TopPartBottom->Next(MusicLabel(mica::Instantwise)))
    TopPartBottom = TopPartBottom->Next(MusicLabel(mica::Instantwise));

  Array<Music::Node> TopPartJoin =
    TopPartBottom->Series(MusicLabel(mica::Partwise));
  Array<Music::Node> BottomPartJoin =
    BottomPartRoot->Series(MusicLabel(mica::Partwise));

  for(count i = 0, j = 0; i < TopPartJoin.n(); i++)
  {
    if(TopPartJoin[i]->Get("MusicXMLBarlineStitch") == "true")
    {
      bool BottomStitch = false; j--;
      while(not BottomStitch and ++j < BottomPartJoin.n())
        BottomStitch = BottomPartJoin[j]->Get(
          "MusicXMLBarlineStitch") == "true";
      if(j >= BottomPartJoin.n()) break;
      if(not TopPartJoin[i]->Next(MusicLabel(mica::Instantwise)))
        G.Connect(TopPartJoin[i], BottomPartJoin[j])->Set(mica::Type) =
          mica::Instantwise;
      for(count k = 1; k <= 4; k++) //0-4:(Bar)-Clef-Key-Time-Chord
      {
        count ik = i + k, jk = j + k;
        if(ik < TopPartJoin.n() and jk < BottomPartJoin.n())
        {
          if(TopPartJoin[ik]->Get(mica::Type) ==
            BottomPartJoin[jk]->Get(mica::Type))
              if(not TopPartJoin[ik]->Next(MusicLabel(mica::Instantwise)))
                G.Connect(TopPartJoin[ik], BottomPartJoin[jk])->Set(
                  mica::Type) = mica::Instantwise;
          if(TopPartJoin[ik]->Get("MusicXMLInitialChordStitch") == "true" or
            BottomPartJoin[jk]->Get("MusicXMLInitialChordStitch") == "true")
              k = 4;
        }
      }
      j++;
    }
  }
}

Music::Node MusicXMLParseMeasure(Music& G, const Value& Measure,
  Value& PartState, bool AddInitialBarline, count MeasureIndex)
{
  Array<List<Value> > Stream;
  PartState["current-measure"] = MeasureIndex;
  Value Exception = MusicXMLParseMeasureElements(Measure, PartState);
  if(not Exception.IsNil())
  {
    PartState["exception"] = Exception;
    return Music::Node();
  }
  MusicXMLConstructMeasureStream(G, PartState, Stream);
  MusicXMLConstructIslandsForStream(G, Stream);
  return MusicXMLConstructStreamGeometry(G, Stream, AddInitialBarline);
}

List<String> MusicXMLGetAttributeExcludes()
{
  return String("bezier-x;bezier-y;width").Tokenize(";");
}

List<String> MusicXMLGetTagExcludes()
{
  return String(
    "credit;defaults;identification;lyric;midi-instrument;print;"
    "score-instrument;work;footnote;level;instruments;part-symbol"
    "staff-details;transpose;directive").Tokenize(";");
}

void MusicXMLRemoveMetadata(Music& G)
{
  Sortable::Array<Music::Node> Nodes = G.Nodes();
  for(count i = 0; i < Nodes.n(); i++)
  {
    if(Nodes[i]->Get("MusicXMLVoice"))
      Nodes[i]->Set("MusicXMLVoice") = "";
    if(Nodes[i]->Get("MusicXMLBarlineStitch"))
      Nodes[i]->Set("MusicXMLBarlineStitch") = "";
    if(Nodes[i]->Get("MusicXMLInitialChordStitch"))
      Nodes[i]->Set("MusicXMLInitialChordStitch") = "";
  }
}

void MusicXMLLinkVoices(Music& G)
{
  Sortable::Array<Music::Node> Nodes = G.Nodes();
  for(count i = 0; i < Nodes.n(); i++) if(IsChord(Nodes[i]))
  {
    Music::Node Island = G.Promote(IslandOfToken(Nodes[i]));
    while(Island and (Island = Island->Next(MusicLabel(mica::Partwise))))
    {
      Array<Music::Node> Tokens = Island->Children(MusicLabel(mica::Token));
      for(count j = 0; j < Tokens.n(); j++)
        if(Nodes[i]->Get("MusicXMLVoice") == Tokens[j]->Get("MusicXMLVoice"))
        {
          if(not Nodes[i]->Next(MusicLabel(mica::Voice)))
            G.Connect(Nodes[i], Tokens[j])->Set(mica::Type) = mica::Voice,
              j = Tokens.n(), Island = Music::Node();
        }
        else if(Tokens[j]->Get(mica::Kind) == mica::Barline)
          Island = Music::Node();
    }
  }
}

bool MusicXMLParseScore(Pointer<Music> M, const String& MusicXMLData,
  const Array<byte>& MusicXMLValidationZip)
{
  if(not M) return false;
  M->Clear();
  Music& G = *M;

  Value XMLIsValid = MusicXMLValidate(MusicXMLData, MusicXMLValidationZip,
    "/tmp");
  if(MusicXMLValidationZip.n() and not XMLIsValid)
    return false;

  Value MusicXMLAsJSON;
  MusicXMLAsJSON.FromXML(MusicXMLData, MusicXMLGetTagExcludes(),
    MusicXMLGetAttributeExcludes());

  Matrix<Music::Node> PartMeasureMatrix(
    MusicXMLGetInstrumentalPartCount(MusicXMLAsJSON),
    MusicXMLGetMeasureCount(MusicXMLAsJSON));
  if(not PartMeasureMatrix.m() or not PartMeasureMatrix.n())
    return false;

  for(count i = 0; i < PartMeasureMatrix.m(); i++)
  {
    Value PartState;
    for(count j = 0; j < PartMeasureMatrix.n(); j++)
    {
      Value Measure = MusicXMLGetMeasure(MusicXMLAsJSON, i, j);
      count MaxStaves = MusicXMLGetMaximumNumberOfStaves(Measure);
      PartMeasureMatrix(i, j) =
        MusicXMLParseMeasure(G, Measure, PartState,
          not j and (PartMeasureMatrix.m() > 1 or MaxStaves > 1), j);
      if(PartState.Contains(Value("exception")))
      {
        C::Error() >> "Aborting MusicXML parse with exception:";
        C::Error() >> JSON::Export(PartState["exception"]);
        M->Clear();
        return false;
      }
      if(not PartState["skipped-measure"].AsBoolean())
      {
        if(PartState.Contains("previous-measure"))
        {
          MusicXMLStitchMeasuresHorizontally(G,
            PartMeasureMatrix(i, PartState["previous-measure"].AsCount()),
            PartMeasureMatrix(i, j));
        }
        PartState["previous-measure"] = j;
        if(PartState.Contains("multiple-rest-count"))
        {
          Array<Music::Node> LastInstant =
            PartMeasureMatrix(i, j)->Series(
            MusicLabel(mica::Partwise), false).z()->Series(
            MusicLabel(mica::Instantwise), false);
          for(Counter k; k.z(LastInstant); k++)
          {
            Music::Node Right = LastInstant[k];
            Music::Node Left = Right->Previous(MusicLabel(mica::Partwise));
            if(Left and Right)
            {
              Music::Edge e = M->Connect(Left, Right);
              e->Set(mica::Type) = mica::MeasureRest;
              e->Set(mica::Value) = mica::Concept(
                PartState["multiple-rest-count"].AsRatio());
            }
          }
        }
      }
    }
    if(i)
      MusicXMLStitchPartsVertically(G, PartMeasureMatrix(i - 1, 0),
        PartMeasureMatrix(i, 0));
    MusicXMLLinkVoices(G);
    MusicXMLCreateSpans(G, PartState);
  }
  G.Root(PartMeasureMatrix(0, 0));
  MusicXMLConnectStavesAtLeft(G);
  MusicXMLRemoveMetadata(G);
  return true;
}

Value MusicXMLValidate(String MusicXMLData,
  const Array<byte>& MusicXMLValidationZip,
  String AbsolutePathToUnzipTo)
{
  if(not MusicXMLValidationZip.n())
    return Value();
  String PartwiseDTD    = AbsolutePathToUnzipTo + "/musicxml30/partwise.dtd";
  String ZipFile        = AbsolutePathToUnzipTo + "/musicxml30.zip";
  String MusicXMLFolder = AbsolutePathToUnzipTo + "/musicxml30";
  String TestXML        = AbsolutePathToUnzipTo + "/musicxml30/test.xml";
  String Catalog        = AbsolutePathToUnzipTo + "/musicxml30/catalog.xml";

  if(not File::Length(PartwiseDTD))
  {
    File::Write(ZipFile, MusicXMLValidationZip);

    String In, Out, Err;
    Shell::PipeInOut(Shell::GetProcessOnPath("unzip"), In, Out, Err,
      "-o", ZipFile, "-d", MusicXMLFolder);
    if(Err)
    {
      C::Red();
      C::Error() >> Err;
      C::Reset();
      return Value();
    }
    Shell::PipeInOut(Shell::GetProcessOnPath("rm"), In, Out, Err, ZipFile);
    if(Err)
    {
      C::Red();
      C::Error() >> Err;
      C::Reset();
      return Value();
    }
  }

  File::Write(TestXML, MusicXMLData);

  String c;
  c << "<catalog xmlns=\"urn:oasis:names:tc:entity:xmlns:xml:catalog\">";
  c << "<rewriteSystem systemIdStartString=\"http://www.musicxml.org/dtds\"";
  c << " rewritePrefix=\"file://" << MusicXMLFolder << "\"/>";
  c << "</catalog>";
  File::Write(Catalog, c);

  Value Result;
  {
    String In, Out, Err;
    count ReturnCode =
      Shell::PipeInOut(Shell::GetProcessOnPath("bash"), In, Out, Err,
      "-c", String("XML_CATALOG_FILES=") + Catalog +
      " xmllint --valid --nonet " + TestXML);
    if(ReturnCode >= 1 and ReturnCode <= 9)
    {
      C::Red();
      C::Out() >> Err;
      C::Reset();
      Result = false;
      return Value(false);
    }
    else if(ReturnCode == 0)
    {
      Result = true;
    }
    else
    {
      C::Red();
      C::Out() >> "Error: could not find xmllint.";
      C::Reset();
      C::Out() >> "Try: brew install libxml2";
    }
  }
  return Result;
}
#endif
