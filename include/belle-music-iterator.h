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

#ifndef BELLE_ENGRAVER_MUSICITERATOR_H
#define BELLE_ENGRAVER_MUSICITERATOR_H

namespace BELLE_NAMESPACE
{
  class MusicIterator
  {
    Music::ConstNode CurrentPart;

    Pointer<const class Geometry> CurrentGeometry;
    count CurrentPartIndex;
    count CurrentInstantIndex;

    Music::ConstNode CurrentIsland;

    Array<Music::ConstNode> CurrentTokens;
    count CurrentToken;

    Array<Music::ConstNode> CurrentNotes;
    count CurrentNote;

    void GeometryBasedNextIsland()
    {
      if(CurrentGeometry)
      {
        CurrentIsland = Music::ConstNode();
        while(CurrentGeometry and not CurrentIsland)
        {
          if(++CurrentInstantIndex >= CurrentGeometry->GetNumberOfInstants())
            CurrentInstantIndex = 0, CurrentPartIndex++;
          if(CurrentPartIndex >= CurrentGeometry->GetNumberOfParts())
            Clear(), CurrentGeometry = Music::ConstNode();
          else
            CurrentIsland = CurrentGeometry->LookupIsland(
              CurrentPartIndex, CurrentInstantIndex);
        }
      }
    }

    void GraphBasedNextIsland()
    {
      if(CurrentPart)
      {
        if(CurrentIsland)
        {
          CurrentIsland = CurrentIsland->Next(MusicLabel(mica::Partwise));
          if(not CurrentIsland)
            CurrentPart = CurrentPart->Next(MusicLabel(mica::Instantwise));
        }
        CurrentIsland = CurrentIsland ? CurrentIsland : CurrentPart;
      }
    }

    public:

    void Clear()
    {
      CurrentPart = CurrentIsland = Music::ConstNode();
      CurrentGeometry = Pointer<const class Geometry>();
      CurrentPartIndex = CurrentInstantIndex = CurrentToken = CurrentNote = -1;
      CurrentTokens.Clear();
      CurrentNotes.Clear();
    }

    void Start(Pointer<const Music> M)
    {
      Clear();
      if(M)
        CurrentPart = M->Root();
    }

    void Start(Pointer<const class Geometry> G)
    {
      Clear();
      if(G)
        CurrentGeometry = G, CurrentPartIndex = 0;
    }

    Music::ConstNode Island() const
    {
      return IsIsland(CurrentIsland) ? CurrentIsland : Music::ConstNode();
    }

    count Part() const
    {
      Value v;
      if(Island())
        v = Island()->Label.GetState("PartID");
      return v.IsInteger() ? v.AsCount() : count(-1);
    }

    count Instant() const
    {
      Value v;
      if(Island())
        v = Island()->Label.GetState("InstantID");
      return v.IsInteger() ? v.AsCount() : count(-1);
    }

    Music::ConstNode Token() const
    {
      Music::ConstNode x =
        CurrentToken >= 0 ? CurrentTokens[CurrentToken] : Music::ConstNode();
      return IsToken(x) ? x : Music::ConstNode();
    }

    Music::ConstNode Barline() const
    {
      Music::ConstNode x = Token();
      return IsBarline(x) ? x : Music::ConstNode();
    }

    Music::ConstNode Clef() const
    {
      Music::ConstNode x = Token();
      return IsClef(x) ? x : Music::ConstNode();
    }

    Music::ConstNode KeySignature() const
    {
      Music::ConstNode x = Token();
      return IsKeySignature(x) ? x : Music::ConstNode();
    }

    Music::ConstNode TimeSignature() const
    {
      Music::ConstNode x = Token();
      return (IsToken(x) and x->Get(mica::Kind) == mica::TimeSignature) ?
        x : Music::ConstNode();
    }

    Music::ConstNode Chord() const
    {
      Music::ConstNode x = Token();
      return IsChord(x) ? x : Music::ConstNode();
    }

    Music::ConstNode Note() const
    {
      Music::ConstNode x =
        CurrentNote >= 0 ? CurrentNotes[CurrentNote] : Music::ConstNode();
      return IsNote(x) ? x : Music::ConstNode();
    }

    Music::ConstNode NextIsland()
    {
      if(CurrentGeometry)
        GeometryBasedNextIsland();
      else if(CurrentPart)
        GraphBasedNextIsland();
      CurrentTokens.Clear();
      CurrentNotes.Clear();
      CurrentToken = -1;
      CurrentNote = -1;
      return Island();
    }

    Music::ConstNode NextToken()
    {
      if(not Island())
        NextIsland();
      Music::ConstNode t;
      while(Island() and not t)
      {
        if(not ++CurrentToken)
          CurrentTokens = TokensOfIsland(Island());
        if(CurrentToken >= CurrentTokens.n())
          NextIsland();
        else
          t = CurrentTokens[CurrentToken];
      }
      CurrentNotes.Clear();
      CurrentNote = -1;
      return Token();
    }

    Music::ConstNode NextBarline()
    {
      while(NextToken() and not Barline()) {}
      return Barline();
    }

    Music::ConstNode NextClef()
    {
      while(NextToken() and not Clef()) {}
      return Clef();
    }

    Music::ConstNode NextKeySignature()
    {
      while(NextToken() and not KeySignature()) {}
      return KeySignature();
    }

    Music::ConstNode NextTimeSignature()
    {
      while(NextToken() and not TimeSignature()) {}
      return TimeSignature();
    }

    Music::ConstNode NextChord()
    {
      while(NextToken() and not Chord()) {}
      return Chord();
    }

    Music::ConstNode NextNote()
    {
      if(not Chord())
        NextChord();
      Music::ConstNode n;
      while(Chord() and not n)
      {
        if(not ++CurrentNote)
          CurrentNotes = NotesOfChord(Chord());
        if(CurrentNote >= CurrentNotes.n())
          NextChord();
        else
          n = CurrentNotes[CurrentNote];
      }
      return Note();
    }

    class StableHash
    {
      static Value Normalize(Value v)
      {
        Value Normalized = v;
        Normalized.Prune();
        return Normalized;
      }

      public:

      static String Prehash(Pointer<const Music> M)
      {
        Value Result;
        Result.NewTree();
        if(not M) return Result;
        Pointer<Geometry> G;
        G.New()->Parse(*M);

        MusicIterator Iterator;

        //Iterate through each island in geometry and gather their label.
        {
          Iterator.Start(G.Const());
          Result["Islands"].NewArray();
          while(Iterator.NextIsland())
          {
            if(Result["Islands"][Iterator.Part()].IsNil())
              Result["Islands"][Iterator.Part()].NewTree();
            Result["Islands"][Iterator.Part()][Iterator.Instant()] =
              Normalize(Iterator.Island()->Label.Properties());
          }
        }

        //Iterate through each barline in geometry and gather their labels.
        {
          Iterator.Start(G.Const());
          Result["Barlines"].NewArray();
          while(Iterator.NextBarline())
          {
            Result["Barlines"][Iterator.Part()].NewTree();
            Result["Barlines"][Iterator.Part()][Iterator.Instant()] =
              Normalize(Iterator.Barline()->Label.Properties());
          }
        }

        //Iterate through each clef in geometry and gather their labels.
        {
          Iterator.Start(G.Const());
          Result["Clefs"].NewArray();
          while(Iterator.NextClef())
          {
            Result["Clefs"][Iterator.Part()].NewTree();
            Result["Clefs"][Iterator.Part()][Iterator.Instant()] =
              Normalize(Iterator.Clef()->Label.Properties());
          }
        }

        //Iterate through key signatures in geometry and gather their labels.
        {
          Iterator.Start(G.Const());
          Result["KeySignatures"].NewArray();
          while(Iterator.NextKeySignature())
          {
            Result["KeySignatures"][Iterator.Part()].NewTree();
            Result["KeySignatures"][Iterator.Part()][Iterator.Instant()] =
              Normalize(Iterator.KeySignature()->Label.Properties());
          }
        }

        //Iterate through time signatures in geometry and gather their labels.
        {
          Iterator.Start(G.Const());
          Result["TimeSignatures"].NewArray();
          while(Iterator.NextTimeSignature())
          {
            Result["TimeSignatures"][Iterator.Part()].NewTree();
            Result["TimeSignatures"][Iterator.Part()][Iterator.Instant()] =
              Normalize(Iterator.TimeSignature()->Label.Properties());
          }
        }

        /*Since chord order within island is ill-defined, define global chord
        order based on sorting the string value of their label data.*/
        {
          Sortable::Array<String> ChordData;
          Iterator.Start(G.Const());
          while(Iterator.NextChord())
          {
            Value v = Normalize(Iterator.Chord()->Label.Properties());
            v["_Part"] = Iterator.Part();
            v["_Instant"] = Iterator.Instant();
            v["_EdgeCount"] = Iterator.Chord()->Children(MusicLabel()).n();
            ChordData.Add() = JSON::Export(v);
          }
          ChordData.Sort();
          Result["Chords"].NewArray();
          for(count i = 0; i < ChordData.n(); i++)
            Result["Chords"][i] = JSON::Import(ChordData[i]);
        }

        /*Since note order within chord is ill-defined, define global note order
        based on sorting the string value of their label data.*/
        {
          Sortable::Array<String> NoteData;
          Iterator.Start(G.Const());
          while(Iterator.NextNote())
          {
            Value v = Normalize(Iterator.Note()->Label.Properties());
            v["_Part"] = Iterator.Part();
            v["_Instant"] = Iterator.Instant();
            v["_EdgeCount"] = Iterator.Note()->Children(MusicLabel()).n();
            NoteData.Add() = JSON::Export(v);
          }
          NoteData.Sort();
          Result["Notes"].NewArray();
          for(count i = 0; i < NoteData.n(); i++)
            Result["Notes"][i] = JSON::Import(NoteData[i]);
        }
        return JSON::Export(Result);
      }

      static String Hash(Pointer<const Music> M)
      {
        return MD5::Hex(Prehash(M));
      }
    };
  };
}
#endif
