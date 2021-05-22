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

#ifndef BELLE_ENGRAVER_MUSIC_H
#define BELLE_ENGRAVER_MUSIC_H

namespace BELLE_NAMESPACE
{
  //Helpers imported from API.
  Pointer<GraphT<MusicLabel>::Object> HighestInFloatStack(
    Pointer<GraphT<MusicLabel>::Object> Island, mica::Concept Placement);
  MusicLabel TraverseFloatStack(mica::Concept Placement);

  class Music : public GraphT<MusicLabel>
  {
    public:

    //----------------------//
    //Templated Helper Types//
    //----------------------//

    typedef MusicLabel Label;
    typedef GraphT<MusicLabel> Graph;
    typedef Pointer<GraphT<MusicLabel>::Object> Edge;
    typedef Pointer<GraphT<MusicLabel>::Object> Node;
    typedef Pointer<const GraphT<MusicLabel>::Object> ConstEdge;
    typedef Pointer<const GraphT<MusicLabel>::Object> ConstNode;

    ///Converts the graph to a string.
    operator String() const
    {
      return String(*static_cast<const Graph*>(this));
    }

    ///Returns subgraph with only islands and partwise and instant-wise edges.
    Pointer<Music> GeometrySubgraph() const
    {
      Pointer<Music> m;
      m.New()->ImportXML(ExportXML());
      Sortable::Array<Node> mNodes = m->Nodes();
      for(count i = 0; i < mNodes.n(); i++)
        if(mNodes[i]->Label.Get(mica::Type) != mica::Island)
          m->Remove(mNodes[i]);
      Sortable::Array<ConstEdge> mEdges = m->Edges();
      for(count i = 0; i < mEdges.n(); i++)
        if(mEdges[i]->Label.Get(mica::Type) != mica::Partwise and
          mEdges[i]->Label.Get(mica::Type) != mica::Instantwise)
            m->Remove(m->Promote(mEdges[i]));
      return m;
    }

    //-------------//
    //Node Creation//
    //-------------//

    ///Creates a new island.
    Node CreateIsland()
    {
      Node i = Graph::Add();
      i->Set(mica::Type) = mica::Island;
      return i;
    }

    ///Creates a new token.
    Node CreateToken(mica::Concept TokenType)
    {
      Node t = Graph::Add();
      t->Set(mica::Type) = mica::Token;
      t->Set(mica::Kind) = TokenType;
      return t;
    }

    ///Adds a token to an island.
    void AddTokenToIsland(Node Island, Node Token)
    {
      Graph::Connect(Island, Token)->Set(mica::Type) = mica::Token;
    }

    ///Creates a barline token inside a new island and returns the island.
    Node CreateAndAddBarline(mica::Concept BarlineType =
      mica::StandardBarline)
    {
      Node i = CreateIsland(); //Create island first to set root.
      Node t = CreateToken(mica::Barline);
      t->Set(mica::Value) = BarlineType;
      AddTokenToIsland(i, t);
      return i;
    }

    ///Creates a float node of type expression with expression text.
    Node AddExpressionTo(Node IslandOrChord, String Expression,
      mica::Concept Placement)
    {
      /*
      Floats append in two stacks like this:
           .--> (Float, Above) --> (Float, Above) --> (Float, Above)
          /
      (island)
          \
           `--> (Float, Below) --> (Float, Below) --> (Float, Below)

      So we create a filter by type Float and given placement, and append to
      the respective stack.
      */
      Node LastInSeries = HighestInFloatStack(IslandOrChord, Placement);

      Node e = Graph::Add();
      e->Set(mica::Type) = mica::Float;
      e->Set(mica::Kind) = mica::Expression;
      e->Set(mica::Placement) = Placement;
      Edge FloatEdge = Graph::Connect(LastInSeries, e);
      FloatEdge->Set(mica::Type) = mica::Float;
      FloatEdge->Set(mica::Placement) = Placement;
      e->Set("Text") = Expression;
      if(count(Expression.ToNumber()) > 0)
      {
        e->Set("Size") = String(8);
        e->Set("Style") = "Bold";
      }
      return e;
    }

    /**Adds an octave transposition line between two islands. The signature
    text can be one of the common markings, such as 8, 8va, 8vb, 8ba, 15, 15ma,
    15mb, 22, 22ma, 22mb. Otherwise, the signature will be rendered in an
    italicized font.*/
    Edge AddOctaveTransposition(Node LeftIsland, Node RightIsland,
      String Signature, mica::Concept Placement)
    {
      Edge OctaveTransposition = Graph::Connect(LeftIsland, RightIsland);
      OctaveTransposition->Set(mica::Type) = mica::Span;
      OctaveTransposition->Set(mica::Kind) = mica::OctaveTransposition;
      OctaveTransposition->Set(mica::Placement) = Placement;
      OctaveTransposition->Set("Text") = Signature;
      return OctaveTransposition;
    }

    void AddPedalMarking(Node LeftIsland, Node RightIsland,
      mica::Concept Begin, mica::Concept End)
    {
      Node LeftStack = HighestInFloatStack(LeftIsland, mica::Below);
      Node RightStack = HighestInFloatStack(RightIsland, mica::Below);

      Node LeftFloat = Graph::Add();
      LeftFloat->Set(mica::Type) = mica::Float;
      LeftFloat->Set(mica::Kind) = mica::PedalMarking;
      LeftFloat->Set(mica::Placement) = mica::Below;
      LeftFloat->Set(mica::PedalMarking) = Begin;
      Edge LeftFloatEdge = Graph::Connect(LeftStack, LeftFloat);
      LeftFloatEdge->Set(mica::Type) = mica::Float;
      LeftFloatEdge->Set(mica::Placement) = mica::Below;

      Node RightFloat = Graph::Add();
      RightFloat->Set(mica::Type) = mica::Float;
      RightFloat->Set(mica::Kind) = mica::PedalMarking;
      RightFloat->Set(mica::Placement) = mica::Below;
      RightFloat->Set(mica::PedalMarking) = End;
      Edge RightFloatEdge = Graph::Connect(RightStack, RightFloat);
      RightFloatEdge->Set(mica::Type) = mica::Float;
      RightFloatEdge->Set(mica::Placement) = mica::Below;

      Edge Span = Graph::Connect(LeftFloat, RightFloat);
      Span->Set(mica::Type) = mica::Span;
      Span->Set(mica::Kind) = mica::PedalMarking;
    }

    ///Creates a clef token inside a new island and returns the island.
    Node CreateAndAddClef(mica::Concept ClefType = mica::TrebleClef)
    {
      Node i = CreateIsland(); //Create island first to set root.
      Node t = CreateToken(mica::Clef);
      t->Set(mica::Value) = ClefType;
      AddTokenToIsland(i, t);
      return i;
    }

    /**Creates a key signature token inside a new island and returns the island.
    Also allows an optional mode to be specified.*/
    Node CreateAndAddKeySignature(mica::Concept KeySignature,
      mica::Concept Mode = mica::Undefined)
    {
      Node i = CreateIsland(); //Create island first to set root.
      Node t = CreateToken(mica::KeySignature);
      t->Set(mica::Value) = KeySignature;
      t->Set(mica::Mode) = Mode;
      AddTokenToIsland(i, t);
      return i;
    }

    /**Creates time signature token inside a new island and returns the island.
    This is just a regular time signature with a number of beats such as 4 and a
    rhythm such as "1/4".*/
    Node CreateAndAddTimeSignature(mica::Concept Kind,
      count Beats, mica::Concept NoteValue)
    {
      Node i = CreateIsland(); //Create island first to set root correctly.
      Node t = CreateToken(mica::TimeSignature);
      t->Set(mica::Value) = Kind;
      t->Set(mica::Beats) = mica::Concept(Ratio(Beats));
      t->Set(mica::NoteValue) = NoteValue;
      AddTokenToIsland(i, t);
      return i;
    }

    /**Creates time signature token inside a new island and returns the island.
    This is just a regular time signature with a number of beats such as 4 and a
    rhythm such as "1/4".*/
    Node CreateAndAddTimeSignature(count Beats, mica::Concept NoteValue)
    {
      return CreateAndAddTimeSignature(mica::RegularTimeSignature, Beats,
        NoteValue);
    }

    /**Creates time signature token inside a new island and returns the island.
    This is just a regular time signature with a number of beats such as 4 and a
    rhythm such as "1/4".*/
    Node CreateAndAddTimeSignature(mica::Concept SpecialType)
    {
      Node i = CreateIsland();
      Node t = CreateToken(mica::TimeSignature);
      t->Set(mica::Value) = SpecialType;
      AddTokenToIsland(i, t);
      return i;
    }

    /**Creates time signature token inside a new island and returns the island.
    This is just a regular time signature with a number of beats such as 4 and a
    rhythm such as "1/4".*/
    Node CreateAndAddTimeSignatureFormula(String Formula)
    {
      Node i = CreateIsland();
      Node t = CreateToken(mica::TimeSignature);
      t->Set(mica::Value) = mica::AdditiveTimeSignature;
      t->Set("Formula") = Formula;
      AddTokenToIsland(i, t);
      return i;
    }

    ///Creates and returns a chord.
    Node CreateChord(mica::Concept NoteValue)
    {
      Node t = CreateToken(mica::Chord);
      t->Set(mica::NoteValue) = NoteValue;
      return t;
    }

    ///Adds the chord to a new island and returns that island.
    Node AddChordToNewIsland(Node Chord)
    {
      Node i = CreateIsland();
      AddTokenToIsland(i, Chord);
      return i;
    }

    ///Creates note given the pitch, adds it to the chord, and returns note.
    Node CreateAndAddNote(Node ChordToAddTo, mica::Concept Pitch,
      bool ForceAccidental = false)
    {
      Node n = Graph::Add();
      n->Set(mica::Type) = mica::Note;
      n->Set(mica::Value) = Pitch;
      if(Pitch == mica::Rest)
      {
        n->Set(mica::Value) = mica::Undefined;
        n->Set(mica::Rest) = mica::Rest;
        n->Set(mica::StaffPosition) = mica::Concept(Ratio(0, 1));
      }
      if(ForceAccidental)
        n->Set(mica::Accidental) = mica::map(Pitch, mica::Accidental);
      Graph::Connect(ChordToAddTo, n)->Set(mica::Type) = mica::Note;
      return n;
    }

    //-----------------//
    //Node Manipulation//
    //-----------------//

    ///Returns whether the node is an island.
    static bool IsIsland(ConstNode n)
    {
      return n->Get(mica::Type) == mica::Island;
    }

    /**Moves a node to the top-most part in the instant. If a null pointer is
    sent or if the given node is not an island, but some other type of node,
    then the node is changed to null. If the node returned is not null, then it
    returns true, and otherwise false.*/
    static bool RaiseToTopPart(Node& n)
    {
      //Make sure the node passed in is an island.
      if(!IsIsland(n))
      {
        n = Node();
        return false;
      }

      //Go to the top part of the instant.
      Node m;
      while((m = n->Previous(MusicLabel(mica::Instantwise))))
        n = m;

      return true;
    }

    /**Moves a node to the top-most part in the instant. If a null pointer is
    sent or if the given node is not an island, but some other type of node,
    then the node is changed to null. If the node returned is not null, then it
    returns true, and otherwise false.*/
    static bool RaiseToTopPart(ConstNode& n)
    {
      //Make sure the node passed in is an island.
      if(!IsIsland(n))
      {
        n = ConstNode();
        return false;
      }

      //Go to the top part of the instant.
      ConstNode m;
      while((m = n->Previous(MusicLabel(mica::Instantwise))))
        n = m;

      return true;
    }
  };
}
#endif
