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

#define PRIM_COMPILE_INLINE
#include "prim.h"

using namespace prim;

typedef GraphTLabel<String> Label;
typedef GraphT<Label> LabeledGraph;
typedef Pointer<GraphT<Label>::Object> LabeledEdge;
typedef Pointer<GraphT<Label>::Object> LabeledNode;
typedef Pointer<const GraphT<Label>::Object> ConstEdge;
typedef Pointer<const GraphT<Label>::Object> ConstNode;
typedef Array<LabeledNode> LabeledNodes;
typedef Array<const LabeledNode> ConstNodes;

int main()
{
  C::Out() >> "           Graph g                           Graph h";
  C::Out()++;
  C::Out() >> "            +---+                             +---+";
  C::Out() >> "            | a |                             | f |<----+";
  C::Out() >> "            +---+                             +---+     |";
  C::Out() >> "    +---------+---------+                       +       |";
  C::Out() >> "    |         |         |                       |       |";
  C::Out() >> "    I         I         J                       |       |";
  C::Out() >> "    |         |         |                       +---M---+";
  C::Out() >> "    v         v         v";
  C::Out() >> "  +---+     +---+     +---+";
  C::Out() >> "  | b |     | c |     | d |<----+";
  C::Out() >> "  +---+     +---+     +---+     |";
  C::Out() >> "    +                   +       |";
  C::Out() >> "    |                   |       |";
  C::Out() >> "    K                   |       |";
  C::Out() >> "    |                   +---L---+";
  C::Out() >> "    v";
  C::Out() >> "  +---+";
  C::Out() >> "  | e |";
  C::Out() >> "  +---+";

  //Create the g graph.
  LabeledGraph g;

  LabeledNode a = g.Add(), b = g.Add(), c = g.Add(), d = g.Add(), e = g.Add();

  a->Set("Name") = "a";
  b->Set("Name") = "b";
  c->Set("Name") = "c";
  d->Set("Name") = "d";
  e->Set("Name") = "e";

  LabeledEdge I1 = g.Connect(a, b); I1->Set("Name") = "I";
  LabeledEdge I2 = g.Connect(a, c); I2->Set("Name") = "I";
  LabeledEdge J = g.Connect(a, d); J->Set("Name") = "J";
  LabeledEdge K = g.Connect(b, e); K->Set("Name") = "K";
  LabeledEdge L = g.Connect(d, d); L->Set("Name") = "L";

  //Create the h graph.
  LabeledGraph h;
  LabeledNode f = h.Add();
  f->Set("Name") = "f";
  LabeledEdge M = h.Connect(f, f); M->Set("Name") = "M";

  //Create some filters for traversals.
  Label FilterI, FilterJ, FilterK, FilterL, FilterM;
  FilterI.Set("Name") = "I";
  FilterJ.Set("Name") = "J";
  FilterK.Set("Name") = "K";
  FilterL.Set("Name") = "L";
  FilterM.Set("Name") = "M";

  /*Create const graph references to prevent changing the graph while inspecting
  it.*/
  const LabeledGraph& gc = g;

  //Get next node from a by filter J (will be d).
  C::Out() >> "Next from a by J:   " << gc.Print(gc.Next(a, FilterJ));

  //Get next node from a by filter I (will be Null due to more than one match).
  C::Out() >> "Next from a by I:   " << gc.Print(gc.Next(a, FilterI));

  //Get the children from a by filter I (will be b and c).
  C::Out() >> "Children of a by I: " << gc.Print(gc.Children(a, FilterI));

  //Get next node from a by filter I (will be Null).
  C::Out() >> "Next of b by I:     " << gc.Print(gc.Next(b, FilterI));

  //Get the parent of b by filter I (will be a).
  C::Out() >> "Previous of b by I: " << gc.Print(gc.Previous(b, FilterI));

  //Follow the d node with looping edge L (will be d).
  C::Out() >> "Next of d by L:     " << gc.Print(gc.Next(d, FilterL));

  //Print a node that does not belong to the graph (will be Alien).
  C::Out() >> "Printing f not belonging to graph g: " << gc.Print(f);

  C::Out() >> "Cycles of I: " << gc.Print(gc.Cycle(FilterI));
  C::Out() >> "Cycles of L: " << gc.Print(gc.Cycle(FilterL));
  C::Out() >> "Cycles of M: " << h.Print(h.Cycle(FilterM));

  //Print the graphs out.
  C::Out() >> "Graph g: " << gc;
  C::Out() >> "Graph h: " << h;

  //File::Write("/Users/burnson2/Desktop/foo.tgf", gc.ExportTGF());
  //C::Out() >> gc.ExportDOT();
  //File::Write("/Users/burnson2/Desktop/clust.gv.txt", gc.ExportDOT());
  String x = gc.ExportXML();
  C::Out() >> x;

  g.ImportXML(x);
  C::Out() >> gc;

  C::Out() >> gc.ExportXML();

  return AutoRelease<Console>();
}
