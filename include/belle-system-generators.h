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

#ifndef BELLE_ENGRAVER_SYSTEM_GENERATORS_H
#define BELLE_ENGRAVER_SYSTEM_GENERATORS_H

namespace BELLE_NAMESPACE
{
  ///System-level generators that take in parameters.
  class SystemGenerators
  {
    public:

    template <class T>
    static T Pick(Random& S, Array<T>& A)
    {
      return A[S.Between(count(0), A.n())];
    }

    template <class T>
    static void TruncateArray(Array<T>& A, count NewMax)
    {
      A.n(Max(Min(NewMax + 1, A.n()), count(0)));
    }

    static void NoteheadsSingle(Random& S, Score& ScoreToPopulate,
      count MaxRhythmicLevels, count MaxDots, count MaxDistanceFromCenter)
    {
      Pointer<Music> M;
      ScoreToPopulate.AddSystem(M.New());

      Array<Ratio> D;
      D.Add() = Ratio(1, 1);
      D.Add() = Ratio(1, 2);
      D.Add() = Ratio(1, 4);
      D.Add() = Ratio(1, 8);
      D.Add() = Ratio(1, 16);
      D.Add() = Ratio(1, 32);
      D.Add() = Ratio(1, 64);
      D.Add() = Ratio(1, 128);
      D.Add() = Ratio(1, 256);
      D.Add() = Ratio(1, 512);
      D.Add() = Ratio(1, 1024);
      TruncateArray(D, MaxRhythmicLevels);

      Array<Ratio> Dots;
      Dots.Add() = Ratio(1, 1);
      Dots.Add() = Ratio(3, 2);
      Dots.Add() = Ratio(7, 4);
      Dots.Add() = Ratio(15, 8);
      Dots.Add() = Ratio(31, 16);
      TruncateArray(Dots, MaxDots);

      Array<mica::Concept> P;
      for(count i = -MaxDistanceFromCenter; i <= MaxDistanceFromCenter; i++)
        P.Add() = mica::map(mica::TrebleClef, mica::Concept(Ratio(i)));

      IslandGenerators::Append(M, M->CreateAndAddBarline());
      IslandGenerators::Append(M, M->CreateAndAddClef(mica::TrebleClef));
      for(count i = 0; i < 20; i++)
      {
        Music::Node t = M->CreateChord(Pick(S, D) * Pick(S, Dots));
        Music::Node n = M->CreateAndAddNote(t, Pick(S, P));
        IslandGenerators::Append(M, M->AddChordToNewIsland(t));
      }
      IslandGenerators::Append(M, M->CreateAndAddBarline());
    }

    static void ChordsSpaced(Random& S, Score& ScoreToPopulate,
      count MaxRhythmicLevels, count MaxDots, count MaxDistanceFromCenter,
      count MaxChordHeight, count MaxNotes)
    {
      Pointer<Music> M;
      ScoreToPopulate.AddSystem(M.New());

      Array<Ratio> D;
      D.Add() = Ratio(1, 1);
      D.Add() = Ratio(1, 2);
      D.Add() = Ratio(1, 4);
      D.Add() = Ratio(1, 8);
      D.Add() = Ratio(1, 16);
      D.Add() = Ratio(1, 32);
      D.Add() = Ratio(1, 64);
      D.Add() = Ratio(1, 128);
      D.Add() = Ratio(1, 256);
      D.Add() = Ratio(1, 512);
      D.Add() = Ratio(1, 1024);
      TruncateArray(D, MaxRhythmicLevels);

      Array<Ratio> Dots;
      Dots.Add() = Ratio(1, 1);
      Dots.Add() = Ratio(3, 2);
      Dots.Add() = Ratio(7, 4);
      Dots.Add() = Ratio(15, 8);
      Dots.Add() = Ratio(31, 16);
      TruncateArray(Dots, MaxDots);

      IslandGenerators::Append(M, M->CreateAndAddBarline());
      IslandGenerators::Append(M, M->CreateAndAddClef(mica::TrebleClef));
      for(count i = 0; i < 20; i++)
      {
        Music::Node t = M->CreateChord(Pick(S, D) * Pick(S, Dots));

        Array<mica::Concept> P;
        count Center =
          S.Between(-MaxDistanceFromCenter, MaxDistanceFromCenter + 1);
        for(count j = Center - MaxChordHeight / 2;
          j < Center - MaxChordHeight / 2 + MaxChordHeight; j++)
            P.Add() = mica::map(mica::TrebleClef, mica::Concept(Ratio(j)));

        {
          count NotesToUse = S.Between(count(0), MaxNotes + 1);
          Array<mica::Concept> NotesUsed;
          for(count j = 0; j < NotesToUse; j++)
          {
            mica::Concept NoteToUse = Pick(S, P);
            if(NotesUsed.Contains(NoteToUse))
              continue;
            NotesUsed.Add() = NoteToUse;
            Music::Node n = M->CreateAndAddNote(t, NoteToUse);
          }
        }
        IslandGenerators::Append(M, M->AddChordToNewIsland(t));
      }
      IslandGenerators::Append(M, M->CreateAndAddBarline());
    }

    static bool Generate(Random& State, Score& ScoreToPopulate, Value Generator)
    {
      if(!Validate(Generator))
        return false;

      String GeneratorName = Generator[0].AsString();
      if(GeneratorName == "noteheads-single")
      {
        NoteheadsSingle(State, ScoreToPopulate, Generator[1].AsCount(),
          Generator[2].AsCount(), Generator[3].AsCount());
      }
      else if(GeneratorName == "chords-spaced")
      {
        ChordsSpaced(State, ScoreToPopulate, Generator[1].AsCount(),
          Generator[2].AsCount(), Generator[3].AsCount(),
          Generator[4].AsCount(), Generator[5].AsCount());
      }
      return true;
    }

    static void CreateValidationData(Value& V)
    {
      V.Clear();
      String s;
      {
        s = "noteheads-single";
        V[s][1][0] = "rhythmic-levels";
        V[s][1][1] =
          "Maximum number of rhythmic levels where whole = 0, half = 1, etc.";
        V[s][1][2] = "Integer";
        V[s][1][3] = 0;
        V[s][1][4] = 10;

        V[s][2][0] = "max-dots";
        V[s][2][1] = "Maximum number of duration dots";
        V[s][2][2] = "Integer";
        V[s][2][3] = 0;
        V[s][2][4] = 4;

        V[s][3][0] = "max-staff-position-distance";
        V[s][3][1] = "Maximum staff position distance from staff center";
        V[s][3][2] = "Integer";
        V[s][3][3] = 0;
        V[s][3][4] = 20;
      }
      {
        s = "chords-spaced";
        V[s][1][0] = "rhythmic-levels";
        V[s][1][1] =
          "Maximum number of rhythmic levels where whole = 0, half = 1, etc.";
        V[s][1][2] = "Integer";
        V[s][1][3] = 0;
        V[s][1][4] = 10;

        V[s][2][0] = "max-dots";
        V[s][2][1] = "Maximum number of duration dots";
        V[s][2][2] = "Integer";
        V[s][2][3] = 0;
        V[s][2][4] = 4;

        V[s][3][0] = "max-staff-position-distance";
        V[s][3][1] = "Maximum staff position distance from staff center";
        V[s][3][2] = "Integer";
        V[s][3][3] = 0;
        V[s][3][4] = 20;

        V[s][4][0] = "max-chord-height";
        V[s][4][1] = "Maximum chord height in steps";
        V[s][4][2] = "Integer";
        V[s][4][3] = 1;
        V[s][4][4] = 20;

        V[s][5][0] = "max-notes";
        V[s][5][1] = "Maximum number of notes in chord";
        V[s][5][2] = "Integer";
        V[s][5][3] = 1;
        V[s][5][4] = 10;
      }
    }

    static void ShowUsage(String GeneratorName, const Value& V)
    {
      C::Out() >> "[\"";
      C::Bold(); C::Out() << GeneratorName; C::Reset();
      C::Out() << "\"";
      for(count i = 1; i < V[GeneratorName].n(); i++)
      {
        C::Out() << ", ";
        C::Underline();
        C::Out() << V[GeneratorName][i][0];
        C::Reset();
      }
      C::Out() << "]";
      for(count i = 1; i < V[GeneratorName].n(); i++)
      {
        C::Underline(); C::Out() >> V[GeneratorName][i][0];
        C::Reset();
        C::Out() >> "  Description: " << V[GeneratorName][i][1];
        C::Out() >> "  Type:        " << V[GeneratorName][i][2];
        if(V[GeneratorName][i][2].AsString() != "String")
        {
          C::Out() >> "  Range:       [" << V[GeneratorName][i][3] <<
            ", " << V[GeneratorName][i][4] << "]";
        }
      }
    }

    static void ShowAllGenerators()
    {
      Value V;
      CreateValidationData(V);
      Array<Value> GeneratorNames;
      V.EnumerateKeys(GeneratorNames);
      for(count i = 0; i < GeneratorNames.n(); i++)
      {
        if(i)
          C::Out()++;
        ShowUsage(GeneratorNames[i], V);
      }
    }

    static bool Validate(Value Generator)
    {
      Value V;
      CreateValidationData(V);

      if(!Generator.IsArray())
      {
        C::Red();
        C::Error() >> "Generator ";
        C::Bold();
        C::Error() << Generator;
        C::Reset();
        C::Red();
        C::Error() << " is not an array.";
        C::Reset();
        return false;
      }

      String GeneratorName = Generator[0];

      if(V[GeneratorName].IsNil())
      {
        C::Red();
        C::Error() >> "Unknown generator ";
        C::Bold();
        C::Error() << Generator[0];
        C::Reset();
        return false;
      }

      bool ShowFullUsage = false;
      bool IsSuccess = true;

      if(V[GeneratorName].n() != Generator.n())
      {
        ShowFullUsage = true;
        IsSuccess = false;
        C::Red();
        C::Error() >> "Unexpected number of parameters in generator ";
        C::Reset();
      }
      else
      {
        for(count i = 1; i < Generator.n(); i++)
        {
          if(V[GeneratorName][i][2].AsString() == "Integer")
          {
            if(!Generator[i].IsInteger() ||
              Generator[i].AsInteger() < V[GeneratorName][i][3].AsInteger() ||
              Generator[i].AsInteger() > V[GeneratorName][i][4].AsInteger())
            {
              C::Red();
              C::Error() >> "In generator ";
              C::Bold();
              C::Error() << GeneratorName;
              C::Reset(); C::Red(); C::Error() << ", parameter ";
              C::Bold();
              C::Error() << V[GeneratorName][i][0];
              C::Reset(); C::Red(); C::Error() << " with value ";
              C::Bold();
              C::Error() << Generator[i];
              C::Reset(); C::Red();
              C::Error() << " should be an integer in range [" <<
                V[GeneratorName][i][3] << ", " << V[GeneratorName][i][4] <<
                "].";
              C::Reset();
              IsSuccess = false;
            }
          }
        }
      }

      if(ShowFullUsage)
        ShowUsage(GeneratorName, V);

      return IsSuccess;
    }
  };
}
#endif
