/*
  ==============================================================================

  Copyright 2007-2013 William Andrew Burnson. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY WILLIAM ANDREW BURNSON ''AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
  EVENT SHALL WILLIAM ANDREW BURNSON OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  ------------------------------------------------------------------------------

  This file is part of Belle, Bonne, Sage --
    The 'Beautiful, Good, Wise' C++ Vector-Graphics Library for Music Notation 

  ==============================================================================
*/

#ifndef BELLE_GRAPH_BASE_H
#define BELLE_GRAPH_BASE_H

namespace BELLE_NAMESPACE
{
  //Class to store music concepts and custom strings
  class MusicLabel
  {
    ///Stores the concepts.
    prim::Table<mica::Concept> Concepts;
    
    ///Stores the strings.
    prim::Table<prim::String> Strings;
  
    ///Converts a string like "Foo Bar" to "FooBar" (no case check though).
    static prim::String ToCamelCase(prim::String s)
    {
      s.Replace(" ", "");
      return s;
    }
  
    /**Converts a string like "FooBar" to "Foo Bar" (any sequence of lowercase
    followed by uppercase has a space inserted between).*/
    static prim::String ToSpaceSeparated(prim::String s)
    {
      for(prim::count i = 0; i < s.n() - 1; i++)
      {
        prim::ascii a = (prim::ascii)s[i];
        prim::ascii b = (prim::ascii)s[i + 1];
        if((a >= 'a' && a <= 'z') && (b >= 'A' && b <= 'Z'))
          s.Insert(" ", i++ + 1);
      }
      return s;
    }

    public:
  
    ///Const key-value lookup
    const mica::Concept& Get(const mica::Concept& Key) const
    {
      return Concepts[Key];
    }
  
    ///Mutable key-value lookup
    mica::Concept& Set(const mica::Concept& Key) {return Concepts[Key];}

    ///Const key-value lookup
    const prim::String& Get(const prim::ascii* Key) const {return Strings[Key];}
  
    ///Mutable key-value lookup
    prim::String& Set(const prim::ascii* Key) {return Strings[Key];}

    ///Attribute set for XML deserialization.
    void Set(const prim::ascii* Key, const prim::ascii* Value)
    {
      prim::String k = Key, v = Value;
      if(k.StartsWith("data-"))
      {
        k.Erase(0, 4);
        Strings[k] = v;
      }
      else
        Concepts[mica::Concept(ToSpaceSeparated(k).Merge())] =
          mica::Concept(Value);
    }
  
    ///Returns number of attributes for the purpose of XML serialization.
    prim::count Attributes() const
    {
      return Concepts.n() + Strings.n();
    }
  
    ///Returns attribute key for the purpose of XML serialization.
    prim::String AttributeKey(prim::count i) const
    {
      if(i < Concepts.n())
        return ToCamelCase(Concepts.ith(i).Key);
      else
      {
        prim::String s = "data-";
        s << Strings.ith(i - Concepts.n()).Key;
        return s;
      }
    }
  
    ///Returns attribute value for the purpose of XML serialization.
    prim::String AttributeValue(prim::count i) const
    {
      if(i < Concepts.n())
        return Concepts.ith(i).Value;
      else
        return Strings.ith(i - Concepts.n()).Value;
    }

    /**Indicates whether a given label is equivalent for edge traversal. The
    default behavior here is to traverse if the data is exactly the same.*/
    bool EdgeEquivalent(const MusicLabel& EdgeType) const
    {
      return Concepts == EdgeType.Concepts && Strings == EdgeType.Strings;
    }
  
    ///String conversion
    operator prim::String() const
    {
      prim::String s;
      for(prim::count i = 0, n = Attributes(); i < n; i++)
      {
        if(i) s << " ";
        s << AttributeKey(i) << ":" << AttributeValue(i);
      }
      return s;
    }
  };
  
  //Typedefs for music graphs, nodes, and edges.
  typedef prim::Pointer<prim::GraphT<MusicLabel>::Object> MusicEdge;
  typedef prim::Pointer<prim::GraphT<MusicLabel>::Object> MusicNode;
  typedef prim::Pointer<const prim::GraphT<MusicLabel>::Object> ConstMusicEdge;
  typedef prim::Pointer<const prim::GraphT<MusicLabel>::Object> ConstMusicNode;

  class MusicGraph : public prim::GraphT<MusicLabel>
  {
  public:
    ///Converts the graph to a string.
    operator prim::String() const
    {
      return (prim::String)(*(prim::GraphT<MusicLabel>*)this);
    }
  
    ///Creates a new island.
    MusicNode CreateIsland()
    {
      MusicNode i = Add();
      i->Set(mica::Type) = mica::Island;
      return i;
    }
  
    ///Creates a new token.
    MusicNode CreateToken(mica::Concept TokenType)
    {
      MusicNode t = Add();
      t->Set(mica::Type) = mica::Token;
      t->Set(mica::Kind) = TokenType;
      return t;
    }

    ///Adds a token to an island.
    void AddTokenToIsland(MusicNode Island, MusicNode Token)
    {
      Connect(Island, Token)->Set(mica::Type) = mica::Token;
    }

    ///Creates a barline token inside a new island and returns the island.
    MusicNode CreateAndAddBarline(mica::Concept BarlineType = mica::StandardBarline)
    {
      MusicNode t = CreateToken(mica::Barline);
      t->Set(mica::Value) = BarlineType;
      MusicNode i = CreateIsland();
      AddTokenToIsland(i, t);
      return i;
    }
  
    ///Creates a clef token inside a new island and returns the island.
    MusicNode CreateAndAddClef(mica::Concept ClefType = mica::TrebleClef)
    {
      MusicNode t = CreateToken(mica::Clef);
      t->Set(mica::Value) = ClefType;
      MusicNode i = CreateIsland();
      AddTokenToIsland(i, t);
      return i;
    }

    /**Creates a key signature token inside a new island and returns the island.
    Also allows an optional mode to be specified.*/
    MusicNode CreateAndAddKeySignature(mica::Concept KeySignature,
      mica::Concept Mode = mica::Undefined)
    {
      MusicNode t = CreateToken(mica::KeySignature);
      t->Set(mica::Value) = KeySignature;
      t->Set(mica::Mode) = Mode;
      MusicNode i = CreateIsland();
      AddTokenToIsland(i, t);
      return i;
    }

    /**Creates time signature token inside a new island and returns the island.
    This is just a regular time signature with a number of beats such as 4 and a
    rhythm such as "1/4".*/
    MusicNode CreateAndAddTimeSignature(prim::count Beats,
      mica::Concept NoteValue)
    {
      MusicNode t = CreateToken(mica::TimeSignature);
      t->Set(mica::Value) = mica::RegularTimeSignature;
      t->Set(mica::Beats) = mica::Concept(prim::Ratio(Beats));
      t->Set(mica::NoteValue) = NoteValue;
      MusicNode i = CreateIsland();
      AddTokenToIsland(i, t);
      return i;
    }
  
    ///Creates and returns a chord.
    MusicNode CreateChord(mica::Concept NoteValue)
    {
      MusicNode t = CreateToken(mica::Chord);
      t->Set(mica::NoteValue) = NoteValue;
      return t;
    }
  
    ///Adds the chord to a new island and returns that island.
    MusicNode AddChordToNewIsland(MusicNode Chord)
    {
      MusicNode i = CreateIsland();
      AddTokenToIsland(i, Chord);
      return i;
    }
  
    ///Creates note given the pitch, adds it to the chord, and returns note.
    MusicNode CreateAndAddNote(MusicNode ChordToAddTo, mica::Concept Pitch)
    {
      MusicNode n = Add();
      n->Set(mica::Type) = mica::Note;
      n->Set(mica::Value) = Pitch;
      Connect(ChordToAddTo, n)->Set(mica::Type) = mica::Note;
      return n;
    }
  };
}
#endif
