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

#ifndef BELLE_GRAPH_MUSIC_H
#define BELLE_GRAPH_MUSIC_H

#include "Label.h"

namespace BELLE_NAMESPACE { namespace graph
{
  class Music : public prim::GraphT<MusicLabel>
  {
  public:
    ///Converts the graph to a string.
    operator prim::String() const
    {
      return (prim::String)(*(prim::GraphT<MusicLabel>*)this);
    }
    
    //-------------//
    //Node Creation//
    //-------------//
  
    ///Creates a new island.
    MusicNode CreateIsland()
    {
      MusicNode i = Add();
      i->Set(mica::Type) = mica::Island;
      //i->Label.Typesetting = new Typesetting;
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
    MusicNode CreateAndAddBarline(mica::Concept BarlineType =
        mica::StandardBarline)
    {
      MusicNode i = CreateIsland();
      MusicNode t = CreateToken(mica::Barline);
      t->Set(mica::Value) = BarlineType;
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
    
    //-----------------//
    //Node Manipulation//
    //-----------------//

    ///Returns whether the node is an island.
    static bool IsIsland(MusicNode n)
    {
      return n->Get(mica::Type) == mica::Island;
    }
    
    /**Moves a node to the top-most part in the instant. If a null pointer is
    sent or if the given node is not an island, but some other type of node,
    then the node is changed to null. If the node returned is not null, then it
    returns true, and otherwise false.*/
    bool RaiseToTopPart(MusicNode& n)
    {
      //Make sure the node passed in is an island.
      if(!IsIsland(n))
      {
        n = 0;
        return false;
      }
  
      //Go to the top part of the instant.
      MusicNode m;      
      while((m = Previous(n, MusicLabel::Instantwise())))
        n = m;
      
      return true;
    }
  };
}}
#endif
