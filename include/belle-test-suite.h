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

#ifndef BELLE_ENGRAVER_TESTSUITE_H
#define BELLE_ENGRAVER_TESTSUITE_H

namespace BELLE_NAMESPACE
{
  ///Extensive test suite for the engraver.
  class TestSuite
  {
    public:

    static void AppendAll(Score& S)
    {
      AppendTimeSignatureTests(S);
      AppendClefTests(S);
      AppendBarlineTests(S);
      AppendKeySignatureTests(S);
      AppendRestTests(S);
      AppendMeasureRestTests(S);
      AppendStemDirectionTests(S);
      AppendLedgerLineTests(S);
      AppendSmallClusterTests(S);
      AppendBehindBars_087_090_AccidentalTests(S);
      AppendBeamingTests(S);
      AppendSlurTests(S);
      AppendTieTests(S);
      AppendMultivoiceTests(S);
      {
        Pointer<Music> M;
        MV12_FourVoiceTest(M.New());
        S.AddSystem(M);
      }
    }

    static void MV12_FourVoiceTest(Pointer<Music> M)
    {
      Music::Node Clef;

      Array<Music::Node> Islands;
      count Offset = 2;
      for(count i = 0; i < 9 + Offset + 1; i++)
      {
        if(i == 0 or i == (9 + Offset + 1) - 1)
          Islands.Add() = M->CreateAndAddBarline();
        else if(i == 1)
          Islands.Add() = M->CreateAndAddClef(mica::TrebleClef);
        else
          Islands.Add() = M->CreateIsland();
      }

      M->AddExpressionTo(Islands[2], "cresc.", mica::Below);
      M->AddExpressionTo(Islands[3], "sfz", mica::Below);
      M->AddExpressionTo(Islands[4], "ffff", mica::Below);
      M->AddExpressionTo(Islands[5], "dim.", mica::Above);
      M->AddExpressionTo(Islands[6], "p", mica::Below);
      M->AddExpressionTo(Islands[7], "ff", mica::Above);
      M->AddExpressionTo(Islands[8], "pppppp", mica::Below);
      M->AddExpressionTo(Islands[9], "mp", mica::Below);
      M->AddExpressionTo(Islands[10], "fp", mica::Below);
      M->AddOctaveTransposition(Islands[2], Islands[4], "8va", mica::Above);
      M->AddOctaveTransposition(Islands[5], Islands[5], "15mb", mica::Below);
      M->AddOctaveTransposition(Islands[6], Islands[8], "22ma", mica::Above);
      M->AddOctaveTransposition(Islands[9], Islands[10], "8vb", mica::Below);
      M->AddPedalMarking(Islands[3], Islands[5], mica::DamperPedal,
        mica::DamperPedalRelease);
      Array<Music::Node> Voice1, Voice2, Voice3, Voice4;
      for(count i = 0; i < 6; i++)
      {
        Voice1.Add() = M->CreateToken(mica::Chord);
        Voice1.z()->Set(mica::Tenuto) = mica::Tenuto;
        Voice1.z()->Set(mica::StandardAccent) = mica::StandardAccent;
        Voice1.z()->Set(mica::ArticulationPlacement) = mica::Above;
        Voice1.z()->Set(mica::NoteValue) = mica::Concept(Ratio(1, 4));
        if(i == 0)
        {
          M->CreateAndAddNote(Voice1.z(), mica::GSharp6);
          M->CreateAndAddNote(Voice1.z(), mica::AFlat6);
          M->CreateAndAddNote(Voice1.z(), mica::B6);
        }
        else if(i == 1)
        {
          M->CreateAndAddNote(Voice1.z(), mica::BDoubleSharp4);
        }
        else
          M->CreateAndAddNote(Voice1.z(), mica::DDoubleFlat5);
      }
      for(count i = 0; i < 6; i++)
      {
        Voice2.Add() = M->CreateToken(mica::Chord);
        Voice2.z()->Set(mica::NoteValue) = mica::Concept(Ratio(1, 4));
        Voice2.z()->Set(mica::Staccato) = mica::Staccato;
        Voice2.z()->Set(mica::Tenuto) = mica::Tenuto;
        Voice2.z()->Set(mica::ArticulationPlacement) = mica::Below;
        if(i == 0)
        {
          M->CreateAndAddNote(Voice2.z(), mica::CTripleSharp6);
          M->CreateAndAddNote(Voice2.z(), mica::BTripleFlat5);
          M->CreateAndAddNote(Voice2.z(), mica::GSharp5);
          M->CreateAndAddNote(Voice2.z(), mica::FFlat5);
        }
        else
          M->CreateAndAddNote(Voice2.z(), mica::BDoubleSharp4);
      }
      for(count i = 0; i < 4; i++)
      {
        Voice3.Add() = M->CreateToken(mica::Chord);
        Voice3.z()->Set(mica::StrongAccent) = mica::StrongAccent;
        Voice3.z()->Set(mica::Tenuto) = mica::Tenuto;
        Voice3.z()->Set(mica::ArticulationPlacement) = mica::Above;
        //M->CreateAndAddNote(Voice3.z(), mica::GFlat4);
        if(i > 0)
          M->CreateAndAddNote(Voice3.z(), mica::GFlat4);
      }
      Voice3[0]->Set(mica::NoteValue) = mica::Concept(Ratio(1, 8));
      Voice3[1]->Set(mica::NoteValue) = mica::Concept(Ratio(1, 4));
      Voice3[2]->Set(mica::NoteValue) = mica::Concept(Ratio(1, 4));
      Voice3[3]->Set(mica::NoteValue) = mica::Concept(Ratio(1, 8));
      for(count i = 0; i < 2; i++)
      {
        Voice4.Add() = M->CreateToken(mica::Chord);
        Voice4.z()->Set(mica::StandardAccent) = mica::StandardAccent;
        Voice4.z()->Set(mica::StaccatoWedge) = mica::StaccatoWedge;
        Voice4.z()->Set(mica::ArticulationPlacement) = mica::Below;
        M->CreateAndAddNote(Voice4.z(), mica::CDoubleFlat4);
        M->CreateAndAddNote(Voice4.z(), mica::DDoubleSharp4);
        M->CreateAndAddNote(Voice4.z(), mica::ESharp4);
        M->CreateAndAddNote(Voice4.z(), mica::FSharp4);
        M->CreateAndAddNote(Voice4.z(), mica::GFlat4);
      }
      Voice4[0]->Set(mica::NoteValue) = mica::Concept(Ratio(7, 8));
      Voice4[1]->Set(mica::NoteValue) = mica::Concept(Ratio(1, 8));

      for(count i = 0; i < Voice1.n() - 1; i++)
        M->Connect(Voice1[i], Voice1[i + 1])->Set(mica::Type) =
          mica::Voice;
      for(count i = 0; i < Voice2.n() - 1; i++)
        M->Connect(Voice2[i], Voice2[i + 1])->Set(mica::Type) =
          mica::Voice;
      for(count i = 0; i < Voice3.n() - 1; i++)
        M->Connect(Voice3[i], Voice3[i + 1])->Set(mica::Type) =
          mica::Voice;
      for(count i = 0; i < Voice4.n() - 1; i++)
        M->Connect(Voice4[i], Voice4[i + 1])->Set(mica::Type) =
          mica::Voice;

      M->AddTokenToIsland(Islands[0 + Offset], Voice1[0]);
      M->AddTokenToIsland(Islands[0 + Offset], Voice2[0]);
      M->AddTokenToIsland(Islands[1 + Offset], Voice1[1]);
      M->AddTokenToIsland(Islands[1 + Offset], Voice2[1]);
      M->AddTokenToIsland(Islands[1 + Offset], Voice4[0]);
      M->AddTokenToIsland(Islands[2 + Offset], Voice1[2]);
      M->AddTokenToIsland(Islands[2 + Offset], Voice2[2]);
      M->AddTokenToIsland(Islands[2 + Offset], Voice3[0]);
      M->AddTokenToIsland(Islands[3 + Offset], Voice3[1]);
      M->AddTokenToIsland(Islands[4 + Offset], Voice1[3]);
      M->AddTokenToIsland(Islands[4 + Offset], Voice2[3]);
      M->AddTokenToIsland(Islands[5 + Offset], Voice3[2]);
      M->AddTokenToIsland(Islands[6 + Offset], Voice1[4]);
      M->AddTokenToIsland(Islands[6 + Offset], Voice2[4]);
      M->AddTokenToIsland(Islands[7 + Offset], Voice3[3]);
      M->AddTokenToIsland(Islands[7 + Offset], Voice4[1]);
      M->AddTokenToIsland(Islands[8 + Offset], Voice1[5]);
      M->AddTokenToIsland(Islands[8 + Offset], Voice2[5]);

      for(count i = 0; i < Islands.n() - 1; i++)
      {
        M->Connect(Islands[i], Islands[i + 1])->Set(mica::Type) =
          mica::Partwise;
      }
    }

    static void AppendTimeSignatureTests(Score& S)
    {
      Pointer<Music> M;
      IslandGenerators::AppendTimeSignatures(M.New(), 1, 9, Ratio(1, 1));
      S.AddSystem(M); System::SetLabel(M,
        "Time signatures with whole-note denominator and numerators 1 to 9");
      IslandGenerators::AppendTimeSignatures(M.New(), 1, 9, Ratio(1, 2));
      S.AddSystem(M); System::SetLabel(M,
        "Time signatures with half-note denominator and numerators 1 to 9");
      IslandGenerators::AppendTimeSignatures(M.New(), 1, 9, Ratio(1, 4));
      S.AddSystem(M); System::SetLabel(M,
        "Time signatures with quarter-note denominator and numerators 1 to 9");
      IslandGenerators::AppendTimeSignatures(M.New(), 1, 12, Ratio(1, 8));
      S.AddSystem(M); System::SetLabel(M,
        "Time signatures with eighth-note denominator and numerators 1 to 12");
      IslandGenerators::AppendTimeSignatures(M.New(), 1, 15, Ratio(1, 16));
      S.AddSystem(M); System::SetLabel(M,
        "Time signatures with 16th-note denominator and numerators 1 to 15");
      IslandGenerators::AppendTimeSignatures(M.New(), 100, 109,
        Ratio(1, 1024));
      S.AddSystem(M); System::SetLabel(M,
        "Time signatures with 1024th-note denominator and numerators "
        "100 to 109");
      Array<String> Formulae;
      Formulae.Add() = "3+2|5";
      Formulae.Add() = "1+3|5;x;2";
      Formulae.Add() = ";(;3+(2x3)/π|8x8;);+;2⅔;x;Ω";
      IslandGenerators::AppendFormulaTimeSignatures(M.New(), Formulae);
      S.AddSystem(M); System::SetLabel(M,
        "Formulaic time signatures");
    }

    static void AppendClefTests(Score& S)
    {
      Pointer<Music> M;
      IslandGenerators::AppendClefs(M.New());
      S.AddSystem(M); System::SetLabel(M,
        "Common clefs");
    }

    static void AppendBarlineTests(Score& S)
    {
      Pointer<Music> M;
      IslandGenerators::AppendBarlines(M.New());
      S.AddSystem(M); System::SetLabel(M,
        "Common barlines");
    }

    static void AppendKeySignatureTests(Score& S)
    {
      Pointer<Music> M;
      IslandGenerators::AppendKeySignatures(M.New(), mica::TrebleClef, true);
      S.AddSystem(M); System::SetLabel(M,
        "Sharp key signatures up to 7 accidentals on a treble staff");
      IslandGenerators::AppendKeySignatures(M.New(), mica::TrebleClef, false);
      S.AddSystem(M); System::SetLabel(M,
        "Flat key signatures up to 7 accidentals on a treble staff");
      IslandGenerators::AppendKeySignatures(M.New(), mica::BassClef, true);
      S.AddSystem(M); System::SetLabel(M,
        "Sharp key signatures up to 7 accidentals on a bass staff");
      IslandGenerators::AppendKeySignatures(M.New(), mica::BassClef, false);
      S.AddSystem(M); System::SetLabel(M,
        "Flat key signatures up to 7 accidentals on a bass staff");
    }

    static void AppendRestTests(Score& S)
    {
      Pointer<Music> M;
      IslandGenerators::AppendRests(M.New(), Ratio(1, 1));
      S.AddSystem(M); System::SetLabel(M,
        "Rests from double whole to 1024th");
      IslandGenerators::AppendRests(M.New(), Ratio(3, 2));
      S.AddSystem(M); System::SetLabel(M,
        "Rests from double whole to 1024th with single duration dot");
      IslandGenerators::AppendRests(M.New(), Ratio(7, 4));
      S.AddSystem(M); System::SetLabel(M,
        "Rests from double whole to 1024th with double dots");
      IslandGenerators::AppendRests(M.New(), Ratio(15, 8));
      S.AddSystem(M); System::SetLabel(M,
        "Rests from double whole to 1024th with triple dots");
      IslandGenerators::AppendRests(M.New(), Ratio(31, 16));
      S.AddSystem(M); System::SetLabel(M,
        "Rests from double whole to 1024th with quadruple dots");
    }

    static void AppendMeasureRestTests(Score& S)
    {
      Pointer<Music> M;
      IslandGenerators::AppendMeasureRests(M.New(), 0, 5);
      S.AddSystem(M); System::SetLabel(M,
        "Measure rests from 1 to 5");
      IslandGenerators::AppendMeasureRests(M.New(), 5, 10);
      S.AddSystem(M); System::SetLabel(M,
        "Measure rests from 5 to 10");
    }

    static void AppendStemDirectionTests(Score& S)
    {
      Pointer<Music> M;
      List<Ratio> L;
      L.Add() = Ratio(1, 1);
      L.Add() = Ratio(3, 2);
      L.Add() = Ratio(7, 4);
      L.Add() = Ratio(15, 8);
      L.Add() = Ratio(31, 16);
      for(count i = 0; i < L.n(); i++)
      {
        IslandGenerators::AppendSingleNotes(M.New(), L[i],
          mica::TrebleClef, mica::A4);
        S.AddSystem(M); System::SetLabel(M,
          "Stem up durations on a treble staff");
        IslandGenerators::AppendSingleNotes(M.New(), L[i],
          mica::TrebleClef, mica::C5);
        S.AddSystem(M); System::SetLabel(M,
          "Stem down durations on a treble staff");
        IslandGenerators::AppendSingleNotes(M.New(), L[i],
          mica::BassClef, mica::C3);
        S.AddSystem(M); System::SetLabel(M,
          "Stem up durations on a bass staff");
        IslandGenerators::AppendSingleNotes(M.New(), L[i],
          mica::BassClef, mica::E3);
        S.AddSystem(M); System::SetLabel(M,
          "Stem up durations on a bass staff");
      }
    }

    static void AppendLedgerLineTests(Score& S)
    {
      Pointer<Music> M;
      List<mica::Concept> Clefs;
      Clefs.Add() = mica::TrebleClef;
      Clefs.Add() = mica::BassClef;

      for(count c = 0; c < Clefs.n(); c++)
      {
        List<List<mica::Concept> > L;
        for(count i = -18; i <= 18; i++)
          L.Add().Add() = mica::map(mica::Concept(Ratio(i)), Clefs[c]);

        IslandGenerators::AppendChords(M.New(), Clefs[c], Ratio(1, 4),
          L, 0, 19, false);
        S.AddSystem(M); System::SetLabel(M,
          "Ledger lines and stem heights");

        IslandGenerators::AppendChords(M.New(), Clefs[c], Ratio(1, 4),
          L, 19, L.n() - 1, false);
        S.AddSystem(M);
      }
    }

    static void AppendSmallClusterTests(Score& S)
    {
      List<List<mica::Concept> > L = TestSuiteData::SmallClusters();

      List<Ratio> R;
      R.Add() = Ratio(1, 8);
      R.Add() = Ratio(1, 4);
      R.Add() = Ratio(1, 2);
      R.Add() = Ratio(1, 1);

      for(count r = 0; r < R.n(); r++)
      {
        for(count i = 0; i < L.n(); i += 6)
        {
          count j = i + 6;
          if(j >= L.n())
            j = L.n();

          Pointer<Music> M;
          IslandGenerators::AppendChords(M.New(), mica::BassClef, R[r], L, i,
            j - 1, false, true);
          S.AddSystem(M); System::SetLabel(M,
          "Note clusters spanning to fifths with accidentals on each note");
        }
      }
    }

    static void AppendBehindBars_087_090_AccidentalTests(Score& S)
    {
      Pointer<Music> M;
      List<List<mica::Concept> > L =
        TestSuiteData::BehindBars_087_090_AccidentalTests();

      List<Ratio> R;
      R.Add() = Ratio(1, 4);
      R.Add() = Ratio(1, 1);

      for(count r = 0; r < R.n(); r++)
      {
        for(count i = 0; i < L.n(); i += 8)
        {
          count j = i + 8;
          if(j >= L.n())
            j = L.n();
          IslandGenerators::AppendChords(M.New(), mica::TrebleClef, R[r],
            L, i, j - 1, false, true);
          S.AddSystem(M); System::SetLabel(M,
            "Accidental placement examples in Behind Bars pp. 87–90");
        }
      }
    }

    static void AppendBeamingTests(Score& S)
    {
      Pointer<Music> M;
      List<mica::Concept> Clefs;
      Clefs.Add() = mica::TrebleClef;

      for(count c = 0; c < Clefs.n(); c++)
      {
        List<List<List<mica::Concept> > > L;
        List<List<Ratio> > D;
        for(count i = -10; i <= 9; i++)
        {
          L.Add();
          D.Add().Add() = Ratio(1, 8);
          L.z().Add().Add() = mica::map(mica::Concept(Ratio(-1)), Clefs[c]);
          D.z().Add() = Ratio(1, 8);
          L.z().Add().Add() = mica::map(mica::Concept(Ratio(i)), Clefs[c]);
          L.Add();
          D.Add().Add() = Ratio(1, 8);
          L.z().Add().Add() = mica::map(mica::Concept(Ratio(1)), Clefs[c]);
          D.z().Add() = Ratio(1, 16);
          L.z().Add().Add() = mica::map(mica::Concept(Ratio(i + 2)), Clefs[c]);
          L.Add();
          D.Add().Add() = Ratio(1, 16);
          L.z().Add().Add() = mica::map(mica::Concept(Ratio(-1)), Clefs[c]);
          L.z().z().Add() = mica::map(mica::Concept(Ratio(6)), Clefs[c]);
          D.z().Add() = Ratio(1, 8);
          L.z().Add().Add() = mica::map(mica::Concept(Ratio(i)), Clefs[c]);
          L.z().z().Add() = mica::map(mica::Concept(Ratio(i + 7)), Clefs[c]);
          L.Add();
          D.Add().Add() = Ratio(1, 16);
          L.z().Add().Add() = mica::map(mica::Concept(Ratio(1)), Clefs[c]);
          L.z().z().Add() = mica::map(mica::Concept(Ratio(1 + 7)), Clefs[c]);
          D.z().Add() = Ratio(1, 16);
          L.z().Add().Add() = mica::map(mica::Concept(Ratio(i + 2)), Clefs[c]);
          L.z().z().Add() = mica::map(mica::Concept(Ratio(i + 9)), Clefs[c]);
        }

        for(count i = 0; i < L.n(); i += 8)
        {
          IslandGenerators::AppendSpannedChords(M.New(), Clefs[c], D,
            L, mica::Beam, i, Min(i + 7, L.n() - 1));
          S.AddSystem(M); System::SetLabel(M,
            "Beam slants");
        }
      }
    }

    static void AppendSlurTests(Score& S)
    {
      Pointer<Music> M;
      List<mica::Concept> Clefs;
      Clefs.Add() = mica::TrebleClef;

      for(count c = 0; c < Clefs.n(); c++)
      {
        List<List<List<mica::Concept> > > L;
        List<List<Ratio> > D;
        for(count i = -10; i <= 9; i++)
        {
          L.Add();
          D.Add().Add() = Ratio(1, 4);
          L.z().Add().Add() = mica::map(mica::Concept(Ratio(-1)), Clefs[c]);
          D.z().Add() = Ratio(1, 4);
          L.z().Add().Add() = mica::map(mica::Concept(Ratio(i)), Clefs[c]);
          L.Add();
          D.Add().Add() = Ratio(1, 4);
          L.z().Add().Add() = mica::map(mica::Concept(Ratio(1)), Clefs[c]);
          D.z().Add() = Ratio(1, 4);
          L.z().Add().Add() = mica::map(mica::Concept(Ratio(i + 2)), Clefs[c]);
        }

        for(count i = 0; i < L.n(); i += 8)
        {
          IslandGenerators::AppendSpannedChords(M.New(), Clefs[c], D,
            L, mica::Slur, i, Min(i + 7, L.n() - 1));
          S.AddSystem(M); System::SetLabel(M,
            "Two-note slurs");
        }
      }
    }

    static void AppendTieTests(Score& S)
    {
      Pointer<Music> M;
      List<mica::Concept> Notes;
      Notes.Add() = mica::C4;
      Notes.Add() = mica::D4;
      Notes.Add() = mica::E4;
      Notes.Add() = mica::F4;
      Notes.Add() = mica::G4;
      Notes.Add() = mica::A4;
      Notes.Add() = mica::B4;
      Notes.Add() = mica::C5;
      Notes.Add() = mica::D5;
      Notes.Add() = mica::E5;
      Notes.Add() = mica::F5;
      Notes.Add() = mica::G5;
      Notes.Add() = mica::A5;

      IslandGenerators::AppendTiedNotes(M.New(), mica::TrebleClef, Ratio(1, 4),
        Notes);
      S.AddSystem(M); System::SetLabel(M,
        "Tied notes");
    }

    static void AppendMultivoiceTests(Score& S)
    {
      List<List<List<TestSuiteData::ChordData> > > ExampleIslandChord =
        TestSuiteData::BehindBars_047_047_ChordTests();

      for(count i = 0; i < ExampleIslandChord.n(); i++)
      {
        Pointer<Music> M;
        IslandGenerators::AppendMultivoiceChords(M.New(), mica::TrebleClef,
          ExampleIslandChord[i]);
        S.AddSystem(M);
        System::SetLabel(M, "Chord examples in Behind Bars pp. 47–47");
      }
    }
  };
}
#endif
