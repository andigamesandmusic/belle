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

/**
@name Autocorrect

Functions for performing score autocorrection.
*/
void AutocorrectAddToReport(Value& Report, String Tag, String Description,
  Value OtherData);
void AutocorrectAddFinalBarline(List<Pointer<Music> >& Systems,
  Value& Report);
void AutocorrectAddMissingBeginBarlines(List<Pointer<Music> >& Systems,
  Value& Report);
void AutocorrectAddMissingBraces(List<Pointer<Music> >& Systems,
  Value& Report);
void AutocorrectAddMissingEndBarlines(List<Pointer<Music> >& Systems,
  Value& Report);
mica::Concept AutocorrectGetMostLikelyWrittenKeySignature(
  List<Pointer<Music> >& Systems);
String AutocorrectGetMostLikelyWrittenTimeSignature(
  List<Pointer<Music> >& Systems);
void AutocorrectRejectInconsistentPartSystems(List<Pointer<Music> >& Systems,
  Value& Report);
void AutocorrectRejectPartlessSystems(List<Pointer<Music> >& Systems,
  Value& Report);
void AutocorrectRemoveAllOfTokenType(List<Pointer<Music> >& Systems,
  mica::Concept TokenType);
void AutocorrectRemoveEmptyIslands(List<Pointer<Music> >& Systems,
  Value& Report);
void AutocorrectRemoveRestNonSequiturs(List<Pointer<Music> >& Systems,
  Value& Report);
void AutocorrectRemoveWholeNoteNonSequiturs(List<Pointer<Music> >& Systems,
  Value& Report);
void AutocorrectRestoreStaffPositions(List<Pointer<Music> >& Systems);
void AutocorrectRewriteHeaders(List<Pointer<Music> >& Systems,
  Value& Report, String TimeSigRaw, mica::Concept KeySig, bool RebeamToTimeSig);
void AutocorrectRewriteHeaders(List<Pointer<Music> >& Systems,
  Value& Report, String TimeSigRaw, mica::Concept KeySig, bool RebeamToTimeSig,
  bool OmitTimeSig);
void AutocorrectSaveStaffPositions(List<Pointer<Music> >& Systems);
void AutocorrectScore(List<Pointer<Music> >& Systems);
void AutocorrectScore(List<Pointer<Music> >& Systems,
  bool SystemAutocorrectionOnly);
bool IsPopularTimeSignature(String t);
void RemoveIslandAndRestitch(Pointer<Music> MusicSystem, Music::Node Island);
Pointer<const Geometry> ReparseGeometry(Pointer<const Music> MusicSystem);
bool SystemsAreBraced(List<Pointer<Music> >& Systems);
count ValidPartsInGeometry(Pointer<const Music> MusicSystem);

#ifdef BELLE_IMPLEMENTATION

void AutocorrectAddToReport(Value& Report, String Tag, String Description,
  Value OtherData)
{
  Value Entry;
  Entry["Tag"] = Tag;
  Entry["Description"] = Description;
  if(not OtherData.IsNil())
    Entry["Data"] = OtherData;
  Report.Add() = Entry;
}

void AutocorrectAddFinalBarline(List<Pointer<Music> >& Systems,
  Value& Report)
{
  for(count i = Systems.n() - 1; i < Systems.n(); i++)
  {
    Pointer<Music> S = Systems[i];
    Pointer<const Geometry> G = ReparseGeometry(S);
    for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
      if(Music::ConstNode Island = G->LookupIsland(Part,
        G->GetNumberOfInstants() - 1))
          if(Music::ConstNode Token = Island->Next(MusicLabel(mica::Token)))
            if(Token->Get(mica::Kind) == mica::Barline)
              if(Token->Get(mica::Value) == mica::ThinDoubleBarline)
                AutocorrectAddToReport(Report, "MissingFinalBarline",
                  "The double barline at end was replaced by a final barline.",
                  Value()),
                S->Promote(Token)->Set(mica::Value) = mica::FinalBarline;
  }
}

void AutocorrectAddMissingBeginBarlines(List<Pointer<Music> >& Systems,
  Value& Report)
{
  for(count i = 0; i < Systems.n(); i++)
  {
    Pointer<Music> S = Systems[i];
    Pointer<const Geometry> G = ReparseGeometry(S);

    Array<Music::ConstNode> FirstIslands;
    for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
    {
      FirstIslands.Add();
      for(count Instant = 0; Instant < G->GetNumberOfInstants(); Instant++)
        if(IsIsland((FirstIslands.z() = G->LookupIsland(Part, Instant))))
          Instant = G->GetNumberOfInstants();
    }

    bool MissingBarline = false;
    {
      for(count j = 0; j < FirstIslands.n() and not MissingBarline; j++)
        if(Music::ConstNode t = FirstIslands[j]->Next(MusicLabel(mica::Token)))
          if(t->Get(mica::Kind) != mica::Barline)
            MissingBarline = true;
    }

    if(MissingBarline)
    {
      Value v;
      v["System"] = i;
      AutocorrectAddToReport(Report, "MissingBeginBarline",
        "A barline was added to the beginning of the system.", v);
      Music::Node Prev;
      for(count j = 0; j < FirstIslands.n(); j++)
      {
        Music::Node Current = S->CreateAndAddBarline();
        if(j)
          Current->Set("StaffConnects") = "true";
        Current->Set("StaffOffset") = String(number(j * -12));
        if(not j)
          S->Root(Current);
        S->Connect(Current, S->Promote(FirstIslands[j]))->Set(mica::Type) =
          mica::Partwise;
        if(Prev)
          S->Connect(Prev, Current)->Set(mica::Type) = mica::Instantwise;
        Prev = Current;
      }
    }
  }
}

void AutocorrectAddMissingBraces(List<Pointer<Music> >& Systems,
  Value& Report)
{
  if(SystemsAreBraced(Systems))
  {
    for(count i = 0; i < Systems.n(); i++)
    {
      Pointer<Music> S = Systems[i];
      Music::Node Begin = S->Root();
      if(S->Root() and not S->Root()->Next(
        MusicLabel(mica::StaffBracket, mica::Brace)))
      {
        Music::Node End = Begin, Current;
        while((Current = End->Next(MusicLabel(mica::Instantwise))))
          End = Current;
        Value v;
        v["System"] = i;
        AutocorrectAddToReport(Report, "MissingBrace",
          "Adding missing brace inferred from other systems", v);
        Music::Edge e = S->Connect(Begin, End);
        e->Set(mica::StaffBracket) = mica::Brace;
      }
    }
  }
}

void AutocorrectAddMissingEndBarlines(List<Pointer<Music> >& Systems,
  Value& Report)
{
  for(count i = 0; i < Systems.n(); i++)
  {
    Pointer<Music> S = Systems[i];
    Pointer<const Geometry> G = ReparseGeometry(S);

    Array<Music::ConstNode> LastIslands;
    for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
    {
      LastIslands.Add();
      count Instant = G->GetNumberOfInstants();
      while(Instant --> 0)
        if(IsIsland((LastIslands.z() = G->LookupIsland(Part, Instant))))
          Instant = 0;
    }

    bool MissingBarline = false;
    {
      for(count j = 0; j < LastIslands.n() and not MissingBarline; j++)
        if(Music::ConstNode t = LastIslands[j]->Next(MusicLabel(mica::Token)))
          if(t->Get(mica::Kind) != mica::Barline)
            MissingBarline = true;
    }

    if(MissingBarline)
    {
      Value v;
      v["System"] = i;
      AutocorrectAddToReport(Report, "MissingEndBarline",
        "A barline was added to the end of the system.", v);
      Music::Node Prev;
      for(count j = 0; j < LastIslands.n(); j++)
      {
        Music::Node Current = S->CreateAndAddBarline();
        S->Connect(S->Promote(LastIslands[j]), Current)->Set(mica::Type) =
          mica::Partwise;
        if(Prev)
          S->Connect(Prev, Current)->Set(mica::Type) = mica::Instantwise;
        Prev = Current;
      }
    }
  }
}

mica::Concept AutocorrectGetMostLikelyWrittenKeySignature(
  List<Pointer<Music> >& Systems)
{
  Histogram Hist;
  for(count i = 0; i < Systems.n(); i++)
  {
    Pointer<Music> S = Systems[i];
    Pointer<const Geometry> G = ReparseGeometry(S);
    for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
    {
      bool FoundKeySignature = false;
      for(count Instant = 0; Instant < G->GetNumberOfInstants(); Instant++)
        if(Music::ConstNode Island = G->LookupIsland(Part, Instant))
          if(Music::ConstNode Token = Island->Next(MusicLabel(mica::Token)))
            if(Token->Get(mica::Kind) == mica::KeySignature)
              Hist.Increment(Value(Token->Get(mica::Value))),
              FoundKeySignature = true;
      if(not FoundKeySignature)
        Hist.Increment(Value(mica::NoAccidentals));
    }
  }
  mica::Concept MaxBin = Hist.MaxBin();
  return MaxBin;
}

String AutocorrectGetMostLikelyWrittenTimeSignature(
  List<Pointer<Music> >& Systems)
{
  Histogram Hist;
  for(count i = 0; i < Systems.n(); i++)
  {
    Pointer<Music> S = Systems[i];
    Pointer<const Geometry> G = ReparseGeometry(S);
    for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
    {
      for(count Instant = 0; Instant < G->GetNumberOfInstants(); Instant++)
        if(Music::ConstNode Island = G->LookupIsland(Part, Instant))
          if(Music::ConstNode Token = Island->Next(MusicLabel(mica::Token)))
            if(Token->Get(mica::Kind) == mica::TimeSignature)
            {
              String t;
              if(Token->Get(mica::Value) == mica::RegularTimeSignature)
                t << Ratio(Token->Get(mica::Beats)) << "/" <<
                  Ratio(Token->Get(mica::NoteValue)).Denominator();
              else if(Token->Get(mica::Value) == mica::CommonTime)
                t << "Common";
              else if(Token->Get(mica::Value) == mica::CutTime)
                t << "Cut";
              Hist.Increment(Value(t));
            }
    }
  }
  Value MaxBin = Hist.MaxBin();
  if(MaxBin.IsNil())
    MaxBin = "";
  return MaxBin;
}

void AutocorrectRejectInconsistentPartSystems(List<Pointer<Music> >& Systems,
  Value& Report)
{
  Value v;
  v[0] = 0;

  Array<count> PartCounts;
  for(count i = 0; i < Systems.n(); i++)
  {
    count b = ValidPartsInGeometry(Systems[i]);
    PartCounts.Add() = b;
    if(v[Value(b)].IsInteger())
      v[Value(b)] = v[Value(b)].AsCount() + 1;
    else
      v[Value(b)] = 1;
  }
  count ExpectedParts = 1;
  count ExpectedPartsVote = 0;
  for(count i = 1; i < v.n(); i++)
    if(v[i].IsInteger() and v[i].AsCount() >= ExpectedPartsVote)
      ExpectedParts = i, ExpectedPartsVote = v[i].AsCount();
  count i = Systems.n();
  while(i --> 0)
    if(PartCounts[i] != ExpectedParts)
    {
      Systems.Remove(i);
      Value Info;
      Info["Actual"] = PartCounts[i];
      Info["Expected"] = ExpectedParts;
      AutocorrectAddToReport(Report, "InconsistentParts",
        "The system's part count was inconsistent with that of the score.",
        Info);
    }
}

void AutocorrectRejectPartlessSystems(List<Pointer<Music> >& Systems,
  Value& Report)
{
  count i = Systems.n();
  while(i --> 0)
    if(not ValidPartsInGeometry(Systems[i]))
    {
      Systems.Remove(i);
      Value v;
      v["System"] = i;
      AutocorrectAddToReport(Report, "InvalidGeometry",
        "The system geometry could not be parsed.", v);
    }
}

void AutocorrectRemoveAllOfTokenType(List<Pointer<Music> >& Systems,
  mica::Concept TokenType)
{
  bool Modified = false;
  for(count i = 0; i < Systems.n(); i++)
  {
    Pointer<Music> S = Systems[i];
    Pointer<const Geometry> G = ReparseGeometry(S);
    for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
      for(count Instant = 0; Instant < G->GetNumberOfInstants(); Instant++)
        if(Music::ConstNode Current = G->LookupIsland(Part, Instant))
          if(Music::ConstNode Token = Current->Next(MusicLabel(mica::Token)))
            if(Token->Get(mica::Kind) == TokenType)
              RemoveIslandAndRestitch(S, S->Promote(Current)),
              Modified = true;
  }

  if(Modified)
  {
    Value v;
    AutocorrectRemoveEmptyIslands(Systems, v);
  }
}

void AutocorrectRemoveEmptyIslands(List<Pointer<Music> >& Systems,
  Value& Report)
{
  for(count i = 0; i < Systems.n(); i++)
  {
    Pointer<const Geometry> G = ReparseGeometry(Systems[i]);
    for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
    {
      for(count Instant = 0; Instant < G->GetNumberOfInstants(); Instant++)
      {
        Music::ConstNode Current = G->LookupIsland(Part, Instant);
        if(IsIsland(Current) and not TokensOfIsland(Current).n())
        {
          Music::Node Island = Systems[i]->Promote(Current);
          Value v;
          v["Part"] = Part;
          v["Instant"] = Instant;
          v["System"] = i;
          AutocorrectAddToReport(Report, "EmptyIsland",
            "Removing empty island.", v);
          RemoveIslandAndRestitch(Systems[i], Island);
        }
      }
    }
  }
}

void AutocorrectRemoveRestNonSequiturs(List<Pointer<Music> >& Systems,
  Value& Report)
{
  bool Modified = false;
  for(count i = 0; i < Systems.n(); i++)
  {
    Pointer<const Geometry> G = ReparseGeometry(Systems[i]);
    for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
    {
      for(count Instant = 0; Instant < G->GetNumberOfInstants(); Instant++)
      {
        Music::ConstNode Current = G->LookupIsland(Part, Instant);
        Array<Music::ConstNode> Chords = ChordsOfIsland(Current);
        for(count c = 0; c < Chords.n(); c++)
        {
          Music::ConstNode Prev = Chords[c]->Previous(MusicLabel(mica::Voice));
          Music::ConstNode Next = Chords[c]->Next(MusicLabel(mica::Voice));
          if(Chords.n() > 1 and not Prev and not Next and IsRest(Chords[c]))
          {
            Value v;
            v["Part"] = Part;
            v["Instant"] = Instant;
            v["System"] = i;
            AutocorrectAddToReport(Report, "RestNonSequitur",
              "Removing rest found in improbable location.", v);
            Systems[i]->Remove(Systems[i]->Promote(Chords[c]));
            Modified = true;
          }
        }
      }
    }
  }

  if(Modified)
  {
    Value v;
    AutocorrectRemoveEmptyIslands(Systems, v);
  }
}

void AutocorrectRemoveWholeNoteNonSequiturs(List<Pointer<Music> >& Systems,
  Value& Report)
{
  bool Modified = false;
  for(count i = 0; i < Systems.n(); i++)
  {
    Pointer<const Geometry> G = ReparseGeometry(Systems[i]);
    for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
    {
      for(count Instant = 0; Instant < G->GetNumberOfInstants(); Instant++)
      {
        Music::ConstNode Current = G->LookupIsland(Part, Instant);
        Array<Music::ConstNode> Chords = ChordsOfIsland(Current);
        for(count c = 0; c < Chords.n(); c++)
        {
          Music::ConstNode Prev = Current->Previous(MusicLabel(mica::Partwise));
          Music::ConstNode Next = Current->Next(MusicLabel(mica::Partwise));
          if(IntrinsicDurationOfChord(Chords[c]) >= Ratio(1))
          {
            if(Instant < 2 or not IsIsland(Prev) or not IsIsland(Next) or
              ChordsOfIsland(Prev).n() or ChordsOfIsland(Next).n() or
              Chords.n() > 1)
            {
              Music::Node n = Systems[i]->Promote(Chords[c]);
              Value v;
              v["Part"] = Part;
              v["Instant"] = Instant;
              v["System"] = i;
              AutocorrectAddToReport(Report, "WholeNoteNonSequitur",
                "Removing whole note chord found in improbable location.", v);
              Systems[i]->Remove(n);
              Modified = true;
            }
          }
        }
      }
    }
  }

  if(Modified)
  {
    Value v;
    AutocorrectRemoveEmptyIslands(Systems, v);
  }
}

void AutocorrectRestoreStaffPositions(List<Pointer<Music> >& Systems)
{
  for(count i = 0; i < Systems.n(); i++)
  {
    Pointer<Music> S = Systems[i];
    Pointer<const Geometry> G = ReparseGeometry(S);
    for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
    {
      mica::Concept Clef;
      for(count Instant = 0; Instant < G->GetNumberOfInstants(); Instant++)
      {
        Array<Music::ConstNode> Tokens =
          TokensOfIsland(G->LookupIsland(Part, Instant));
        for(count j = 0; j < Tokens.n(); j++)
        {
          Music::ConstNode Chord = Tokens[j];
          if(IsChord(Chord))
          {
            Array<Music::ConstNode> Notes = NotesOfChord(Chord);
            for(count k = 0; k < Notes.n(); k++)
            {
              Music::Node MutableNote = S->Promote(Notes[k]);
              mica::Concept Accidental = mica::map(mica::Accidental,
                MutableNote->Get(mica::Value));
              if(MutableNote->Get(mica::StaffPosition) != mica::Undefined)
                MutableNote->Set(mica::Value) = mica::map(Accidental,
                  mica::map(Clef, MutableNote->Get(mica::StaffPosition)));
            }
          }
          else if(Tokens[j]->Get(mica::Kind) == mica::Clef)
            Clef = Tokens[j]->Get(mica::Value);
        }
      }
    }
  }
}

void AutocorrectRewriteHeaders(List<Pointer<Music> >& Systems,
  Value& Report, String TimeSigRaw, mica::Concept KeySig, bool RebeamToTimeSig)
{
  AutocorrectRewriteHeaders(Systems, Report, TimeSigRaw, KeySig,
    RebeamToTimeSig, false);
}

void AutocorrectRewriteHeaders(List<Pointer<Music> >& Systems,
  Value& Report, String TimeSigRaw, mica::Concept KeySig, bool RebeamToTimeSig,
  bool OmitTimeSig)
{
  AutocorrectSaveStaffPositions(Systems);
  (void)Report;
  mica::Concept TimeSigValue = mica::CommonTime;
  count TimeSigBeats = 0;
  mica::Concept TimeSigNoteValue;
  {
    if(not IsPopularTimeSignature(TimeSigRaw))
      TimeSigRaw = "";
    if(TimeSigRaw == "Common")
      TimeSigValue = mica::CommonTime;
    else if(TimeSigRaw == "Cut")
      TimeSigValue = mica::CutTime;
    else if(TimeSigRaw.Contains("/"))
    {
      TimeSigValue = mica::RegularTimeSignature;
      TimeSigBeats = count(Ratio(TimeSigRaw.Tokenize("/").a()).Numerator());
      TimeSigNoteValue = Ratio(1) / Ratio(TimeSigRaw.Tokenize("/").z());
    }
  }

  Value ClefState;
  for(count s = 0; s < Systems.n(); s++)
  {
    Pointer<Music> S = Systems[s];
    Pointer<const Geometry> G = ReparseGeometry(S);
    for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
    {
      count FirstChord = 0;
      for(count Instant = 1; Instant < G->GetNumberOfInstants() and
        not FirstChord; Instant++)
      {
        if(Music::ConstNode Island = G->LookupIsland(Part, Instant))
        {
          if(ChordsOfIsland(Island).n())
            FirstChord = Instant;
          else if(Music::ConstNode Token =
            Island->Next(MusicLabel(mica::Token)))
          {
            if(Token->Get(mica::Kind) == mica::Barline)
              FirstChord = Instant;
          }
        }
      }

      if(not s)
      {
        count Instant = FirstChord;
        while(Instant --> 0)
          if(Music::ConstNode Island = G->LookupIsland(Part, Instant))
            if(Music::ConstNode Token = Island->Next(MusicLabel(mica::Token)))
              if(Token->Get(mica::Kind) == mica::Clef)
                ClefState[Part] = Token->Get(mica::Value), Instant = 0;
      }

      for(count Instant = 1; Instant < FirstChord; Instant++)
        RemoveIslandAndRestitch(S, S->Promote(G->LookupIsland(Part, Instant)));

      Array<Music::Node> AddedIslands;
      mica::Concept Clef = ClefState[Part];
      if(Clef == mica::Undefined)
      {
        if(Part == 1)
          Clef = mica::BassClef;
        else
          Clef = mica::TrebleClef;
      }
      AddedIslands.Add() = S->CreateAndAddClef(Clef);
      if(KeySig != mica::NoAccidentals and KeySig != mica::Undefined)
        AddedIslands.Add() = S->CreateAndAddKeySignature(KeySig);
      if((not s or RebeamToTimeSig) and not OmitTimeSig)
      {
        if(TimeSigValue == mica::RegularTimeSignature)
          AddedIslands.Add() = S->CreateAndAddTimeSignature(TimeSigBeats,
            TimeSigNoteValue);
        else
          AddedIslands.Add() = S->CreateAndAddTimeSignature(TimeSigValue);
      }

      Music::Node Left = S->Promote(G->LookupIsland(Part, 0));
      S->Remove(Left->Next(MusicLabel(mica::Partwise), true));
      for(count i = 0; i < AddedIslands.n(); i++)
      {
        S->Connect(Left, AddedIslands[i])->Set(mica::Type) = mica::Partwise;
        Left = AddedIslands[i];
      }
      S->Connect(Left,
        S->Promote(G->LookupIsland(Part, FirstChord)))->Set(mica::Type) =
        mica::Partwise;

      if(Part)
      {
        Music::Node Upper = S->Promote(G->LookupIsland(Part - 1, 0));
        for(count i = 0; i < AddedIslands.n() and Upper; i++)
        {
          if((Upper = Upper->Next(MusicLabel(mica::Partwise))))
            S->Connect(Upper, AddedIslands[i])->Set(mica::Type) =
              mica::Instantwise;
        }
      }

      {
        for(count i = FirstChord; i < G->GetNumberOfInstants(); i++)
          if(Music::ConstNode Island = G->LookupIsland(Part, i))
            if(Music::ConstNode Token = Island->Next(MusicLabel(mica::Token)))
              if(Token->Get(mica::Type) == mica::Clef)
                ClefState[Part] = Token->Get(mica::Value),
                i = G->GetNumberOfInstants();
      }
    }
    if(RebeamToTimeSig and TimeSigRaw == "6/8")
      MusicXMLBeaming(Systems[s]);
  }
  AutocorrectRestoreStaffPositions(Systems);
}

void AutocorrectSaveStaffPositions(List<Pointer<Music> >& Systems)
{
  for(count i = 0; i < Systems.n(); i++)
  {
    Pointer<Music> S = Systems[i];
    Pointer<const Geometry> G = ReparseGeometry(S);
    for(count Part = 0; Part < G->GetNumberOfParts(); Part++)
    {
      mica::Concept Clef = mica::TrebleClef;
      for(count Instant = 0; Instant < G->GetNumberOfInstants(); Instant++)
      {
        Array<Music::ConstNode> Tokens =
          TokensOfIsland(G->LookupIsland(Part, Instant));
        for(count j = 0; j < Tokens.n(); j++)
        {
          Music::ConstNode Chord = Tokens[j];
          if(IsChord(Chord))
          {
            Array<Music::ConstNode> Notes = NotesOfChord(Chord);
            for(count k = 0; k < Notes.n(); k++)
            {
              Music::Node MutableNote = S->Promote(Notes[k]);
              MutableNote->Set(mica::StaffPosition) =
                mica::map(mica::map(MutableNote->Get(mica::Value),
                mica::DiatonicPitch), Clef);
            }
          }
          else if(Tokens[j]->Get(mica::Kind) == mica::Clef)
            Clef = Tokens[j]->Get(mica::Value);
        }
      }
    }
  }
}

void AutocorrectScore(List<Pointer<Music> >& Systems)
{
  AutocorrectScore(Systems, false);
}

void AutocorrectScore(List<Pointer<Music> >& Systems,
  bool SystemAutocorrectionOnly)
{
  Value Report;
  AutocorrectRejectPartlessSystems(Systems, Report);
  {
    AutocorrectRejectInconsistentPartSystems(Systems, Report);
    if(not Systems.n())
    {
      C::Out() >> "All systems were rejected.";
      return;
    }
  }
  AutocorrectRemoveEmptyIslands(Systems, Report);
  String TimeSig = AutocorrectGetMostLikelyWrittenTimeSignature(Systems);
  mica::Concept KeySig = AutocorrectGetMostLikelyWrittenKeySignature(Systems);
  AutocorrectRemoveAllOfTokenType(Systems, mica::TimeSignature);
  AutocorrectRemoveAllOfTokenType(Systems, mica::KeySignature);
  AutocorrectRemoveRestNonSequiturs(Systems, Report);
  AutocorrectRemoveWholeNoteNonSequiturs(Systems, Report);
  AutocorrectAddMissingBeginBarlines(Systems, Report);
  AutocorrectAddMissingEndBarlines(Systems, Report);
  AutocorrectAddFinalBarline(Systems, Report);
  AutocorrectAddMissingBraces(Systems, Report);
  if(SystemAutocorrectionOnly)
  {
    AutocorrectRewriteHeaders(Systems, Report, TimeSig, KeySig, false,
      TimeSig == "");
  }
  else
  {
    AutocorrectRewriteHeaders(Systems, Report, TimeSig, KeySig, true);
    AutocorrectRewriteHeaders(Systems, Report, TimeSig, KeySig, false);
  }
}

bool IsPopularTimeSignature(String t)
{
  return t == "Cut" or t == "Common" or t == "4/4" or t == "3/4" or t == "2/4"
    or t == "6/8" or t == "9/8";
}

void RemoveIslandAndRestitch(Pointer<Music> MusicSystem, Music::Node Island)
{
  if(MusicSystem and IsIsland(Island))
  {
    Music::Node Prev = Island->Previous(MusicLabel(mica::Partwise));
    Music::Node Next = Island->Next(MusicLabel(mica::Partwise));
    if(MusicSystem->Root() == Island)
      MusicSystem->Root(Next);
    MusicSystem->Remove(Island);
    if(Prev and Next)
      MusicSystem->Connect(Prev, Next)->Set(mica::Type) = mica::Partwise;
  }
}

Pointer<const Geometry> ReparseGeometry(Pointer<const Music> MusicSystem)
{
  Pointer<Geometry> G = System::MutableGeometry(MusicSystem);
  if(G)
    G->Parse(*MusicSystem);
  return G;
}

bool SystemsAreBraced(List<Pointer<Music> >& Systems)
{
  bool BraceFound = false;
  for(count i = 0; i < Systems.n() and not BraceFound; i++)
  {
    Pointer<Music> S = Systems[i];
    if(S->Root() and S->Root()->Next(
        MusicLabel(mica::StaffBracket, mica::Brace)))
      BraceFound = true;
  }
  return BraceFound;
}

count ValidPartsInGeometry(Pointer<const Music> MusicSystem)
{
  count ValidParts = 0;
  Pointer<const Geometry> G = ReparseGeometry(MusicSystem);
  if(G->GetNumberOfParts() and G->GetNumberOfInstants() >= 2)
    ValidParts = G->GetNumberOfParts();
  return ValidParts;
}

#endif
///@}
