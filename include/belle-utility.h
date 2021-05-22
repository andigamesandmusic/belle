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

#ifndef BELLE_ENGRAVER_UTILITY_H
#define BELLE_ENGRAVER_UTILITY_H

namespace BELLE_NAMESPACE
{
  ///Utility methods
  class Utility
  {
    /*Static interface -- instances not allowed*/ Utility();

    public:

    /**Utility function to help visualize graphs. It uses the Graphviz DOT
    utility to convert a DOT representation of the graph into a PNG image.*/
    static void OpenGraphVisualization(const Music& m,
      String TempFile = "/tmp/prim_test.pdf")
    {
#ifdef PRIM_WITH_SHELL
      String In, Out, Error;
      //Pipe a DOT file to dot and get the result back as PDF.
      Shell::PipeInOut(Shell::GetProcessOnPath("dot"),
        m.ExportDOT(), Out, Error, "-Tpdf");

      //Write the PDF to the temp file.
      File::Write(TempFile, Out);

      //Open the PDF using the default application.
      Shell::PipeInOut(Shell::GetProcessOnPath("open"),
        In, Out, Error, TempFile);
#else
      C::Out() >> "PRIM_WITH_SHELL not enabled. Can not open 'dot'.";
#endif
    }

    /**Returns value of the form {Highest:..., Lowest:...} of diatonic pitches.
    Note that any accidental is automatically discarded.*/
    static Value GetPitchExtremes(const Array<Music::ConstNode>& Notes)
    {
      mica::Concept Lowest, Highest;
      const integer Empty = Nothing<integer>();
      integer LowestIndex = Empty, HighestIndex = Empty;

      //Loop through each note and check to see if it is highest and/or lowest.
      for(count i = 0; i < Notes.n(); i++)
      {
        //Get the note pitch and its diatonic pitch.
        mica::Concept Pitch = Notes[i]->Get(mica::Value);
        mica::Concept DiatonicPitch = mica::map(Pitch, mica::DiatonicPitch);

        /*Calculate staff position assuming treble clef. At the level of island
        state, the active clef is not yet known. The purpose here is to
        calculate the highest and lowest notes. #todo : Add DiatonicPitches
        sequence to MICA to calculate this without an arbitrary clef.*/
        mica::Concept StaffPosition = mica::map(DiatonicPitch,
          mica::TrebleClef);

        //Check for non-pitched note like a rest, otherwise get the index.
        if(!mica::integer(StaffPosition))
        {
          HighestIndex = LowestIndex = 0;
          continue;
        }
        integer StaffPositionIndex = integer(mica::numerator(StaffPosition));

        //Check for a new lowest.
        if(LowestIndex == Empty || StaffPositionIndex < LowestIndex)
        {
          LowestIndex = StaffPositionIndex;
          Lowest = DiatonicPitch;
        }

        //Check for a new highest.
        if(HighestIndex == Empty || StaffPositionIndex > HighestIndex)
        {
          HighestIndex = StaffPositionIndex;
          Highest = DiatonicPitch;
        }
      }

      //Construct the value and return it.
      Value PitchExtremes;
      PitchExtremes["Highest"] = Highest;
      PitchExtremes["Lowest"] = Lowest;
      return PitchExtremes;
    }

    static count GetNumberOfAccidentals(mica::Concept KeySignature)
    {
      return Abs(count(mica::numerator(mica::index(mica::KeySignatures,
        mica::NoAccidentals, KeySignature))));
    }

    static count GetAccidentalPosition(mica::Concept KeySignature,
      mica::Concept Clef, count i)
    {
      /*Get the sequence of accidental positions for the combination of clef
      and accidental.*/
      mica::Concept s = mica::map(Clef, mica::map(KeySignature,
        mica::Accidental));
      return count(mica::numerator(mica::item(s, i)));
    }

    static bool IsIslandBarline(Music::ConstNode n)
    {
      if(Music::ConstNode t = n->Next(Music::Label(mica::Token)))
        if(t->Get(mica::Kind) == mica::Barline)
          return true;
      return false;
    }

    static bool IsIslandRhythmic(Music::ConstNode n)
    {
      bool IslandIsRhythmic = false;
      if(n)
      {
        Array<Music::ConstNode> Tokens =
          n->Children(Music::Label(mica::Token));
        if(Tokens.n() and Tokens.a()->Get(mica::Kind) == mica::Chord)
          IslandIsRhythmic = true;
      }
      return IslandIsRhythmic;
    }

    static Array<Music::ConstNode> GetIslandNoteNodes(Music::ConstNode n)
    {
      Array<Music::ConstNode> NoteNodes;
      if(Music::ConstNode c = n->Next(Music::Label(mica::Token)))
        NoteNodes = c->Children(Music::Label(mica::Note));
      return NoteNodes;
    }

    static Array<mica::Concept> GetIslandNotes(Music::ConstNode n)
    {
      Array<Music::ConstNode> NoteNodes = GetIslandNoteNodes(n);
      Array<mica::Concept> Notes;
      for(count i = 0; i < NoteNodes.n(); i++)
        Notes.Add() = NoteNodes[i]->Get(mica::Value);
      return Notes;
    }

    ///Given the initial instant, finds the last instant in the region.
    static count GetLastInstantInRegion(const Geometry& g, count InitialInstant)
    {
      count LastInstant = InitialInstant;
      for(count i = InitialInstant + 1; i < g.GetNumberOfInstants(); i++)
      {
        if(g.IsInstantComplete(i))
          break;
        LastInstant = i;
      }
      return LastInstant;
    }

    ///Determines whether an island has chords with ties heading to the right.
    static bool IslandChordsHaveTies(Music::ConstNode Island)
    {
      Array<Music::ConstNode> Tokens = Island->Children(
        MusicLabel(mica::Token));
      bool ChordsHaveTies = false;
      for(count i = 0; !ChordsHaveTies && i < Tokens.n(); i++)
      {
        Music::ConstNode t = Tokens[i];
        Array<Music::ConstNode> Notes = t->Children(MusicLabel(mica::Note));
        for(count j = 0; !ChordsHaveTies && j < Notes.n(); j++)
          ChordsHaveTies = Notes[j]->Next(MusicLabel(mica::Tie));
      }
      return ChordsHaveTies;
    }
  };
}
#endif
