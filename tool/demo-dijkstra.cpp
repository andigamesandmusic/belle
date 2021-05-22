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

class CostLabel : public GraphTLabel<String>
{
  public:
  number Cost() {return Get("Cost").ToNumber();}
  bool EdgeEquivalent(const GraphTLabel<String>& L) {(void)L; return true;}
};

typedef GraphT<CostLabel> CostLabeledGraph;
typedef Pointer<GraphT<CostLabel>::Object> CostLabeledEdge;
typedef Pointer<GraphT<CostLabel>::Object> CostLabeledNode;
typedef Pointer<const GraphT<CostLabel>::Object> ConstEdge;
typedef Pointer<const GraphT<CostLabel>::Object> ConstNode;
typedef Array<CostLabeledNode> CostLabeledNodes;
typedef Array<const CostLabeledNode> ConstNodes;

int main()
{
  {
    CostLabeledGraph G;
    CostLabeledNode Start = G.Add(), P = G.Add(), Q = G.Add(), R = G.Add(),
      End = G.Add();
    Start->Set("Name") = "Start";
    P->Set("Name") = "P";
    Q->Set("Name") = "Q";
    R->Set("Name") = "R";
    End->Set("Name") = "End";

    G.Connect(Start, P)->Set("Cost") = 5;
    G.Connect(P, Q)->Set("Cost") = 5;
    G.Connect(Q, R)->Set("Cost") = 5;
    G.Connect(R, End)->Set("Cost") = 5;
    G.Connect(Start, Q)->Set("Cost") = 1;
    G.Connect(Q, End)->Set("Cost") = 1;
    C::Out() >> G;

    List<ConstNode> ShortestPath = G.ShortestPath(Start, End, CostLabel());
    C::Out() >> "Shortest Path:";
    for(count i = 0; i < ShortestPath.n(); i++)
      C::Out() >> ShortestPath[i]->Get("Name");
  }
  {
    count m = 34, n = 34;
    C::Out() >> "Creating Maze...";
    Matrix<CostLabeledNode> M(m, n);
    CostLabeledGraph G;
    Tree<ConstNode, VectorInt> Lookup;
    for(count i = 0; i < m; i++)
      for(count j = 0; j < n; j++)
        M(i, j) = G.Add(), M(i, j)->Set("Name") = String(i) + "," + String(j),
        Lookup[M(i, j)] = VectorInt(integer(i), integer(j));
    for(count i = 0; i < m; i++)
    {
      for(count j = 0; j < n; j++)
      {
        if(i + 1 < m)
          G.Connect(M(i, j), M(i + 1, j))->Set("Cost") = 1.f;
        if(j + 1 < n)
          G.Connect(M(i, j), M(i, j + 1))->Set("Cost") = 1.f;
        if(i + 1 < m and j + 1 < n)
          G.Connect(M(i, j), M(i + 1, j + 1))->Set("Cost") = Sqrt(2.f);
      }
    }
    for(count i = 0; i < m; i++)
    {
      for(count j = 0; j < n; j++)
      {
        number x = i - m / 2;
        number y = j - n / 2;
        if(Distance(number(0), number(0), x, y) < number(Min(m, n)) / 3.f)
          G.Remove(M(i, j)), M(i, j) = CostLabeledNode();
      }
    }
    C::Out() >> "Solving Maze...";
    List<ConstNode> ShortestPath = G.ShortestPath(M(0, 0), M(m - 1, n - 1),
      CostLabel());
    Matrix<ascii> O(m, n);
    for(count i = 0; i < m; i++)
      for(count j = 0; j < n; j++)
        O(i, j) = M(i, j) ? '.' : '*';
    for(count i = 0; i < ShortestPath.n(); i++)
    {
      VectorInt V = Lookup[ShortestPath[i]];
      O(count(V.i()), count(V.j())) = 'X';
    }
    for(count j = 0; j < n; j++)
    {
      C::Out()++;
      for(count i = 0; i < m; i++)
        C::Out() << " " << O(i, j);
    }
  }

  return AutoRelease<Console>();
}
