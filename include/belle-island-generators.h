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

#ifndef BELLE_ENGRAVER_ISLAND_GENERATORS_H
#define BELLE_ENGRAVER_ISLAND_GENERATORS_H

namespace BELLE_NAMESPACE
{
  ///Helpers for generating music graphs from scratch.
  class IslandGenerators
  {
    public:

    static void Append(Pointer<Music> M, Music::Node Island)
    {
      if(M->Root() == Island)
        return;

      Music::Node RightMost = M->Root();
      for(Music::Node n = M->Root(); n;
        n = n->Next(Music::Label(mica::Partwise)))
      {
        RightMost = n;
      }

      M->Connect(RightMost, Island)->Set(mica::Type) = mica::Partwise;
    }

    static void AppendTimeSignatures(Pointer<Music> M, count First, count Last,
      Ratio NoteValue)
    {
      for(count i = First; i <= Last; i++)
      {
        Append(M, M->CreateAndAddBarline());
        Append(M, M->CreateAndAddTimeSignature(i, NoteValue));
        if(i == 2 && NoteValue == Ratio(1, 2))
          Append(M, M->CreateAndAddTimeSignature(mica::CutTime));
        else if(i == 4 && NoteValue == Ratio(1, 4))
          Append(M, M->CreateAndAddTimeSignature(mica::CommonTime));
      }
      Append(M, M->CreateAndAddBarline());
    }

    static void AppendFormulaTimeSignatures(Pointer<Music> M,
      Array<String> Formulae)
    {
      for(count i = 0; i < Formulae.n(); i++)
        Append(M, M->CreateAndAddBarline()),
        Append(M, M->CreateAndAddTimeSignatureFormula(Formulae[i]));
      Append(M, M->CreateAndAddBarline());
    }

    static void AppendClefs(Pointer<Music> M)
    {
      List<mica::Concept> L;
      L.Add() = mica::TrebleClef;
      L.Add() = mica::BassClef;
      L.Add() = mica::AltoClef;
      L.Add() = mica::TenorClef;

      for(count i = 0; i < L.n(); i++)
      {
        Append(M, M->CreateAndAddBarline());
        Append(M, M->CreateAndAddClef(L[i]));
      }
      Append(M, M->CreateAndAddBarline());
    }

    static void AppendBarlines(Pointer<Music> M)
    {
      List<mica::Concept> L;
      L.Add() = mica::StandardBarline;
      L.Add() = mica::ThinDoubleBarline;
      L.Add() = mica::FinalBarline;
      L.Add() = mica::BeginRepeatBarline;
      L.Add() = mica::EndRepeatBarline;

      for(count i = 0; i < L.n(); i++)
        Append(M, M->CreateAndAddBarline(L[i]));
    }

    static void AppendKeySignatures(Pointer<Music> M, mica::Concept Clef,
      bool Sharps)
    {
      List<mica::Concept> L;

      if(Sharps)
      {
        //L.Add() = mica::NoAccidentals;
        L.Add() = mica::OneSharp;
        L.Add() = mica::TwoSharps;
        L.Add() = mica::ThreeSharps;
        L.Add() = mica::FourSharps;
        L.Add() = mica::FiveSharps;
        L.Add() = mica::SixSharps;
        L.Add() = mica::SevenSharps;
      }
      else
      {
        //L.Add() = mica::NoAccidentals;
        L.Add() = mica::OneFlat;
        L.Add() = mica::TwoFlats;
        L.Add() = mica::ThreeFlats;
        L.Add() = mica::FourFlats;
        L.Add() = mica::FiveFlats;
        L.Add() = mica::SixFlats;
        L.Add() = mica::SevenFlats;
      }

      Append(M, M->CreateAndAddBarline());
      Append(M, M->CreateAndAddClef(Clef));
      for(count i = 0; i < L.n(); i++)
      {
        Append(M, M->CreateAndAddKeySignature(L[i]));
        Append(M, M->CreateAndAddBarline());
      }
    }

    static void AppendRests(Pointer<Music> M, Ratio DotFactor)
    {
      List<Ratio> L;
      L.Add() = Ratio(2, 1);
      L.Add() = Ratio(1, 1);
      L.Add() = Ratio(1, 2);
      L.Add() = Ratio(1, 4);
      L.Add() = Ratio(1, 8);
      L.Add() = Ratio(1, 16);
      L.Add() = Ratio(1, 32);
      L.Add() = Ratio(1, 64);
      L.Add() = Ratio(1, 128);
      L.Add() = Ratio(1, 256);
      L.Add() = Ratio(1, 512);
      L.Add() = Ratio(1, 1024);

      for(count i = 0; i < L.n(); i++)
      {
        Append(M, M->CreateAndAddBarline());
        Music::Node t = M->CreateChord(L[i] * DotFactor);
        Music::Node n = M->CreateAndAddNote(t, mica::Undefined);
        n->Set(mica::Rest) = mica::Rest;
        n->Set(mica::StaffPosition) = mica::Concept(Ratio(0, 1));
        Append(M, M->AddChordToNewIsland(t));
      }
      Append(M, M->CreateAndAddBarline());
    }

    static void AppendMeasureRests(Pointer<Music> M, count First, count Last)
    {
      List<mica::Concept> L;
      for(count i = First; i <= Last; i++)
        L.Add() = Ratio(i);

      Music::Node Left = M->CreateAndAddBarline();
      Append(M, Left);
      for(Counter i; i.z(L); i++)
      {
        Music::Node Right = M->CreateAndAddBarline();
        Append(M, Right);
        Music::Edge e = M->Connect(Left, Right);
        e->Set(mica::Type) = mica::MeasureRest;
        e->Set(mica::Value) = L[i];
        Left = Right;
      }
    }

    static void AppendSingleNotes(Pointer<Music> M, Ratio DotFactor,
      mica::Concept Clef, mica::Concept Pitch)
    {
      List<Ratio> L;
      L.Add() = Ratio(2, 1);
      L.Add() = Ratio(1, 1);
      L.Add() = Ratio(1, 2);
      L.Add() = Ratio(1, 4);
      L.Add() = Ratio(1, 8);
      L.Add() = Ratio(1, 16);
      L.Add() = Ratio(1, 32);
      L.Add() = Ratio(1, 64);
      L.Add() = Ratio(1, 128);
      L.Add() = Ratio(1, 256);
      L.Add() = Ratio(1, 512);
      L.Add() = Ratio(1, 1024);

      Append(M, M->CreateAndAddBarline());
      Append(M, M->CreateAndAddClef(Clef));
      for(count i = 0; i < L.n(); i++)
      {
        Music::Node t = M->CreateChord(L[i] * DotFactor);
        Music::Node n = M->CreateAndAddNote(t, Pitch);
        Append(M, M->AddChordToNewIsland(t));
        Append(M, M->CreateAndAddBarline());
      }
    }

    static void AppendChords(Pointer<Music> M, mica::Concept Clef,
      Ratio Duration, List<List<mica::Concept> > Chords, count Start,
      count End, bool InteriorBarlines = true, bool ShowAllAccidentals = false)
    {
      Append(M, M->CreateAndAddBarline());
      Append(M, M->CreateAndAddClef(Clef));
      if(InteriorBarlines)
        Append(M, M->CreateAndAddBarline());
      for(count i = Start; i <= End; i++)
      {
        Music::Node t = M->CreateChord(Duration);
        for(count j = 0; j < Chords[i].n(); j++)
        {
          if(Chords[i][j] == mica::Accidental)
            continue;
          Music::Node n = M->CreateAndAddNote(t, Chords[i][j],
            ShowAllAccidentals && Chords[i].z() != mica::Accidental);
        }
        Append(M, M->AddChordToNewIsland(t));
        if(InteriorBarlines || i == End ||
          Chords[i + 1].z() == mica::Accidental)
            Append(M, M->CreateAndAddBarline());
      }
    }

    static void AppendMultivoiceChords(Pointer<Music> M, mica::Concept Clef,
      const List<List<TestSuiteData::ChordData> >& Chords)
    {
      Append(M, M->CreateAndAddBarline());
      Append(M, M->CreateAndAddClef(Clef));
      for(count i = 0; i < Chords.n(); i++)
      {
        Music::Node Island = M->CreateIsland();
        for(count j = 0; j < Chords[i].n(); j++)
        {
          Music::Node Token = M->CreateChord(Chords[i][j].Duration);
          if(Chords[i][j].StemUp.IsBoolean())
            Token->Set(mica::StemDirection) = Chords[i][j].StemUp.AsBoolean() ?
              mica::Up : mica::Down;
          for(count k = 0; k < Chords[i][j].Notes.n(); k++)
            Music::Node n = M->CreateAndAddNote(Token, Chords[i][j].Notes[k]);

          M->AddTokenToIsland(Island, Token);
        }
        Append(M, Island);
      }
      Append(M, M->CreateAndAddBarline());
    }

    static void AppendSpannedChords(Pointer<Music> M, mica::Concept Clef,
      List<List<Ratio> > Duration,
      List<List<List<mica::Concept> > > SpannedChords,
      mica::Concept SpanType, count Start, count End,
      bool ShowAllAccidentals = false)
    {
      Append(M, M->CreateAndAddBarline());
      Append(M, M->CreateAndAddClef(Clef));

      for(count i = Start; i <= End; i++)
      {
        Music::Node PreviousT;
        for(count j = 0; j < SpannedChords[i].n(); j++)
        {
          Music::Node t = M->CreateChord(Duration[i][j]);

          for(count k = 0; k < SpannedChords[i][j].n(); k++)
          {
            if(SpannedChords[i][j][k] == mica::Accidental)
              continue;
            Music::Node n = M->CreateAndAddNote(t, SpannedChords[i][j][k],
              ShowAllAccidentals &&
              SpannedChords[i][j].z() != mica::Accidental);
          }
          Append(M, M->AddChordToNewIsland(t));
          {
            if(PreviousT)
              M->Connect(PreviousT, t)->Set(mica::Type) = SpanType;
            PreviousT = t;
          }
        }
        Append(M, M->CreateAndAddBarline());
      }
    }

    static void AppendTiedNotes(Pointer<Music> M, mica::Concept Clef,
      Ratio Duration, List<mica::Concept> Notes)
    {
      Append(M, M->CreateAndAddBarline());
      Append(M, M->CreateAndAddClef(Clef));
      List<Music::Node> Previous;
      for(count i = 1; i <= Notes.n(); i++)
      {
        Music::Node t = M->CreateChord(Duration);

        List<Music::Node> Current;
        for(count j = 0; j < i; j++)
        {
          Current.Add() = M->CreateAndAddNote(t, Notes[j]);
          if(j < Previous.n())
            M->Connect(Previous[j], Current[j])->Set(mica::Type) = mica::Tie;
        }
        Previous = Current;
        Append(M, M->AddChordToNewIsland(t));
      }
      Append(M, M->CreateAndAddBarline());
    }
  };
}
#endif
