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

///@name Incipits
///@{
Pointer<Music> AbridgeSystem(Pointer<const Music> M, count MaximumIslands);
Pointer<Music> AbridgeSystem(String ScoreFile, count MaximumIslands);
String AbridgeSystemAsSVG(String ScoreFile, Font NotationFont,
  count MaximumIslands, number InchesWidth,
  number InchesMargin, number SpaceHeight, bool FormatAsPDF);
String AbridgeSystemAsSVG(Pointer<const Music> M, Font NotationFont,
  count MaximumIslands, number InchesWidth,
  number InchesMargin, number SpaceHeight, bool FormatAsPDF);
Pointer<Music> CreateIncipit(String ScoreFile, count MaximumIslands);
Pointer<Music> CreateIncipit(Pointer<const Music> M, count MaximumIslands);
String CreateIncipitAsSVG(String ScoreFile, Font NotationFont,
  count MaximumIslands,
  number InchesWidth, number InchesMargin, number SpaceHeight,
  bool FormatAsPDF);
String CreateIncipitAsSVG(Pointer<const Music> M, Font NotationFont,
  count MaximumIslands,
  number InchesWidth, number InchesMargin, number SpaceHeight,
  bool FormatAsPDF);
String CreateIncipitAsSVG(Pointer<const Music> M, Font NotationFont,
  count MaximumIslands,
  number InchesWidth, number InchesMargin, number SpaceHeight,
  bool FormatAsPDF);
String RenderIncipitAsSVG(Pointer<Music> Incipit, Font NotationFont,
  number InchesWidth, number InchesMargin,
  number SpaceHeight, bool FormatAsPDF);
///@}

#ifdef BELLE_IMPLEMENTATION
Pointer<Music> AbridgeSystem(Pointer<const Music> M, count MaximumIslands)
{
  return AbridgeSystem(M ? M->ExportXML() : String(), MaximumIslands);
}

Pointer<Music> AbridgeSystem(String ScoreFile, count MaximumIslands)
{
  Pointer<Music> M;
  if(ScoreFile)
    M.New()->ImportXML(ConvertToXML(ScoreFile));
  Music::Node BarlineToSnipAt;
  if(M)
  {
    Music::Node x = M->Root();
    count IslandCount = 1;
    MaximumIslands = Max(MaximumIslands, count(3));
    while(x and not BarlineToSnipAt)
      if(IslandCount++ > MaximumIslands and IslandHasBarline(x))
        BarlineToSnipAt = x;
      else
        x = x->Next(Music::Label(mica::Partwise));
  }
  if(BarlineToSnipAt)
  {
    Music::Node x = BarlineToSnipAt;
    while(x)
    {
      Music::Node y = x->Next(Music::Label(mica::Partwise));
      while(y)
      {
        Music::Node z = y->Next(Music::Label(mica::Partwise));
        RemoveIsland(M, y);
        y = z;
      }
      x = x->Next(Music::Label(mica::Instantwise));
    }
  }
  return M;
}

String AbridgeSystemAsSVG(String ScoreFile, Font NotationFont,
  count MaximumIslands, number InchesWidth,
  number InchesMargin, number SpaceHeight, bool FormatAsPDF)
{
  return RenderIncipitAsSVG(AbridgeSystem(ScoreFile, MaximumIslands),
    NotationFont, InchesWidth, InchesMargin, SpaceHeight, FormatAsPDF);
}

String AbridgeSystemAsSVG(Pointer<const Music> M, Font NotationFont,
  count MaximumIslands, number InchesWidth,
  number InchesMargin, number SpaceHeight, bool FormatAsPDF)
{
  return RenderIncipitAsSVG(AbridgeSystem(M, MaximumIslands), NotationFont,
    InchesWidth, InchesMargin, SpaceHeight, FormatAsPDF);
}

Pointer<Music> CreateIncipit(String ScoreFile, count MaximumIslands)
{
  Pointer<Music> M;
  M.New()->ImportXML(ConvertToXML(ScoreFile));
  Pointer<Music> Incipit = CreateIncipit(M.Const(), MaximumIslands);
  return Incipit;
}

Pointer<Music> CreateIncipit(Pointer<const Music> M, count MaximumIslands)
{
  Pointer<Music> Incipit;
  if(not M) return Incipit;
  Incipit.New();
  Music::ConstNode x = M->Root();
  Music::Node RightmostIsland;
  count IslandsCreated = 0;
  while(x and IslandsCreated < MaximumIslands)
  {
    Music::Node IslandToAdd;
    Array<Music::ConstNode> Chords = ChordsOfIsland(x);
    if(IslandHasClef(x))
    {
      IslandToAdd = Incipit->CreateAndAddClef(
        TokensOfIsland(x).a()->Get(mica::Value));
      IslandsCreated++;
    }
    else if(IslandHasKeySignature(x))
    {
      IslandToAdd = Incipit->CreateAndAddKeySignature(
        TokensOfIsland(x).a()->Get(mica::Value),
        TokensOfIsland(x).a()->Get(mica::Mode));
      IslandsCreated++;
    }
    else if(Chords.n())
    {
      IslandToAdd = Incipit->CreateIsland();
      for(count i = 0; i < Chords.n(); i++)
      {
        Music::Node Chord =
          Incipit->CreateChord(Chords[i]->Get(mica::NoteValue));
        Incipit->AddTokenToIsland(IslandToAdd, Chord);
        Array<Music::ConstNode> Notes = NotesOfChord(Chords[i]);
        for(count j = 0; j < Notes.n(); j++)
        {
          Music::Node Note = Incipit->Add();
          Note->Set(mica::Type) = mica::Note;
          Note->Set(mica::Value) = Notes[j]->Get(mica::Value);
          Note->Set(mica::Rest) = Notes[j]->Get(mica::Rest);
          Note->Set(mica::StaffPosition) = Notes[j]->Get(mica::StaffPosition);
          Note->Set(mica::Accidental) = Notes[j]->Get(mica::Accidental);
          Incipit->Connect(Chord, Note)->Set(mica::Type) = mica::Note;
        }
      }
      IslandsCreated++;
    }

    if(IslandToAdd)
    {
      if(RightmostIsland)
        Incipit->Connect(RightmostIsland,
          IslandToAdd)->Set(mica::Type) = mica::Partwise;
      RightmostIsland = IslandToAdd;
    }
    x = x->Next(Music::Label(mica::Partwise));
  }
  if(RightmostIsland)
    Incipit->Connect(RightmostIsland, Incipit->CreateAndAddBarline(
      mica::DottedBarline))->Set(mica::Type) = mica::Partwise;
  return Incipit;
}

String CreateIncipitAsSVG(String ScoreFile, Font NotationFont,
  count MaximumIslands, number InchesWidth,
  number InchesMargin, number SpaceHeight, bool FormatAsPDF)
{
  return RenderIncipitAsSVG(CreateIncipit(ScoreFile, MaximumIslands),
    NotationFont, InchesWidth, InchesMargin, SpaceHeight, FormatAsPDF);
}

String CreateIncipitAsSVG(Pointer<const Music> M, Font NotationFont,
  count MaximumIslands, number InchesWidth,
  number InchesMargin, number SpaceHeight, bool FormatAsPDF)
{
  return RenderIncipitAsSVG(CreateIncipit(M, MaximumIslands), NotationFont,
    InchesWidth, InchesMargin, SpaceHeight, FormatAsPDF);
}

String RenderIncipitAsSVG(Pointer<Music> Incipit, Font NotationFont,
  number InchesWidth, number InchesMargin,
  number SpaceHeight, bool FormatAsPDF)
{
  String Result;
  if(Incipit and Incipit->Root())
  {
    Score IncipitScore;
    IncipitScore.InitializeFont(NotationFont);
    IncipitScore.SetSpaceHeight(SpaceHeight);
    IncipitScore.SetSystemLeft(0.f, 0.f);
    IncipitScore.SetSystemWidth(InchesWidth);
    IncipitScore.AddSystem(Incipit);
    IncipitScore.Engrave(false);
    Box OverallBounds;
    {
      Blank B;
      System::Paint(Incipit, B, Vector());
      {
        Value StaffBounds =
          System::Get(Incipit.Const())["PaintedBounds"]["StaffBounds"];
        for(count i = 0; i < StaffBounds.n(); i++)
          OverallBounds += StaffBounds[i].AsBox();
      }
    }

    Pointer<Page> IncipitPage;
    IncipitScore.Canvases.Add() = IncipitPage.New();
    IncipitPage->Dimensions = OverallBounds.Size() +
      Vector(InchesMargin, InchesMargin) * 2.f;
    List<Pointer<const Music> > Systems;
    List<Vector> Positions;
    Systems.Add() = Incipit;
    Positions.Add() = Vector(InchesMargin - OverallBounds.Left(),
      InchesMargin - OverallBounds.Bottom());
    IncipitPage->SetSystemsAndPositions(Systems, Positions);

    if(not FormatAsPDF)
    {
      SVG::Properties SVGSpecificProperties;
      IncipitScore.Create<SVG>(SVGSpecificProperties);
      if(SVGSpecificProperties.Output.n())
        Result = SVGSpecificProperties.Output.a();
    }
    else
    {
      PDF::Properties PDFSpecificProperties;
      IncipitScore.Create<PDF>(PDFSpecificProperties);
      Result = PDFSpecificProperties.Output;
    }
  }
  return Result;
}
#endif
