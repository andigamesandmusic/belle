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

#define PRIM_WITH_MODULES
#define PRIM_COMPILE_INLINE
#include "prim.h"
using namespace prim;

void Title(String s);
void MazeSolve(count SquareSize);

int main()
{
  C::Out() >> "     33 reasons why prim is awesome";
  C::Out() >> "     -------------------------------";
  {
    Title("Pretty console printing");
    C::Green();
    C::Out() << "'nough said";
    C::Reset();
  }

  {
    Title("UTF-8 strings");
    String s;
    s << unicode(0x414) << unicode(0x430) << "!";
    C::Out() << s << " is " << s.c() << " characters and " << s.n() << " bytes";
  }

  {
    Title("Easy search-and-replace");
    String s = "Find this";
    C::Out() << s << ", ";
    s.Replace("Find", "Replace");
    C::Out() << s;
  }

  {
    Title("Auto-reducing ratios");
    C::Out() << "1000/25 = " << Ratio(1000, 25) << ", ";
    C::Out() << "3/5 * 8/9 = " << (Ratio(3, 5) * Ratio(8, 9));
  }

  {
    Title("Lists that insert and remove");
    List<count> l;
    for(count i = 0; i < 10; i++)
      l.Add() = i;
    for(count i = l.n() - 1; i >= 0; i -= 3)
      l.Remove(i);
    for(count i = 0; i < 10; i += 2)
      l.InsertBefore(i, 0);
    C::Out() << l;
  }

  {
    Title("Arrays that grow");
    Array<count> a;
    for(count i = 0; i < 5; i++)
    {
      C::Out() << a << ", ";
      a.Add() = i;
    }
    C::Out() << a;
  }

  {
    Title("Sorting");
    Sortable::Array<count> a;
    for(count i = 0; i < 5; i++)
      a.Add() = (((i * 3 + 1) * 11) % 5);
    C::Out() << a << " -> ";
    a.Sort();
    C::Out() << a;
  }

  {
    Title("Key-value trees");
    Tree<String> t;
    t["key"] = "value";
    t["prim"] = "awesome";
    C::Out() << t;
  }

  {
    Title("Shareable smart pointers");
    Pointer<int>::Weak y;
    {
      Pointer<int> x = new int;
      C::Out() << x << ", ";
      y = x;
      C::Out() << y << ", ";
    }
    C::Out() << y;
  }

  {
    Title("High-precision timers");
    Timer t;
    t.Start();
    volatile number v = 1;
    for(number i = 1; i < 1000.f; i += 2.f)
    {
      v /= i;
      v *= (i + 1.f);
    }
    number e = t.Stop() * 1000000.f;
    (void)v;
    C::Out() << "expression calculated in " << e << " microseconds";
  }

  {
    Title("High-quality random number generation");
    Random r;
    C::Out() << r.Between();
  }

  {
    Title("UUIDv4s");
    C::Out() << UUIDv4();
  }

  {
    Title("Python shell");
    C::Out() << "Sieve of Eratosthenes: ";
    String Sieve = Python::Evaluate(
      "n=30;"
      "print sorted(set(range(2,n+1)).difference(set((p*f) "
      "for p in range(2,int(n**0.5)+2) for f in range(2,(n/p)+1))))"
    );
    C::Out() << Sieve;
  }

  {
    Title("Containers that print");
    C::Out() << Ratio(3, 2) << ", ";
    C::Out() << String("Awesome") << ", ";
    C::Out() << Array<int>() << ", ";
    C::Out() << List<int>() << ", ";
    C::Out() << Tree<int>() << ", ";
    C::Out() << Value();
  }

  {
    Title("Complex math");
    Vector a(1.5, 3.75), b(-2.5, 1.375);
    C::Out() << a << " * " << b << " = " << (a * b);
  }

  {
    Title("Polar math");
    Vector a(3, 4);
    C::Out() << "a = " << a << ", ";
    C::Out() << "Mag(a) = " << a.Mag() << ", ";
    C::Out() << "Ang(a) = " << a.Ang() << " rad = ";
    C::Out() << (a.Ang() / Deg<number>()) << unicode(0xb0);
  }

  {
    Title("Shell expressions");
    String DateCommand = Shell::GetProcessOnPath("date");
    String In, Out, Error;
    Shell::PipeInOut(DateCommand, In, Out, Error);
    Out.EraseEnding("\n");
    C::Out() << "`date` -> " << Out;
  }

  {
    Title("Easy tokenizing");
    String s = "a,bunch,of,values";
    C::Out() << s << " -> " << s.Tokenize(",");
  }

  {
    Title("NaN detection");
    number x;
    Memory::Clear(x);
    x /= x;
    C::Out() << x << " == Nothing<number>() is " << (x == Nothing<number>());
  }

  {
    Title("MD5 checksums");
    String s = "hello";
    C::Out() << s << " = " << MD5::Hex(s);
  }

  {
    Title("0th-order modified Bessel function");
    C::Out() << "BesselI0[1.2345] = " << BesselI0(1.2345);
  }

  {
    Title("Base64 encoding");
    String s = "Awesome";
    String e, d;
    Encoding::Base64::Encode(s, e);
    Encoding::Base64::Decode(e, d);
    C::Out() << s << " -> " << e << " -> " << d;
  }

  {
    Title("Threads");
    class Job : public Thread
    {
      void Run()
      {
        C::Out() << "running thread...";
      }
    };
    C::Out() << "starting thread...";
    Job j;
    j.Begin();
    j.WaitToEnd();
    C::Out() << "thread exited";
  }

  {
    Title("Variant-typed values");
    Value a(1);
    C::Out() << a << ", ";
    a = "Awesome";
    C::Out() << a << ", ";
    a[2] = "Awesome";
    C::Out() << a << ", ";
    a["Awesome"] = 3;
    C::Out() << a << ",\n     ";
    a["Awesome"]["Awesome"] = "Awesome";
    C::Out() << a << ", ";
    a["Awesome"][Ratio(7, 3)] = "Super-Awesome";
    C::Out() << a;
  }

  {
    count FFTSize = 4;
    Array<Vector> Data(FFTSize);
    Data[0] = Vector(1.2f, 0);
    Data[1] = Vector(0.3f, 0);
    Data[2] = Vector(-4.5f, 0);
    Data[3] = Vector(6.7f, 0);
    Array<number> CosineTable;
    GenerateCosineLookup<float80>(CosineTable, FFTSize);

    {
      Title(" FFT");
      C::Out() << Data << " ->\n           ";
      FFT<Forwards>(Data, CosineTable);
      C::Out() << Data;
    }
    {
      Title("iFFT");
      C::Out() << Data << " ->\n           ";
      FFT<Backwards>(Data, CosineTable);
      Normalize(Data);
      C::Out() << Data;
    }
  }

  {
    Title("Node- and edge-labeled multidigraphs");
    typedef GraphTLabel<String> Label;
    typedef GraphT<Label> Graph;
    typedef Pointer<GraphT<Label>::Object> Node;
    Graph g;
    Node n1 = g.Add(), n2 = g.Add(), n3 = g.Add();
    n1->Label.Set("Node") = "A";
    n2->Label.Set("Node") = "B";
    n3->Label.Set("Node") = "C";
    g.Connect(n1, n2)->Label.Set("Edge") = "1";
    g.Connect(n2, n3)->Label.Set("Edge") = "2";
    g.Connect(n3, n1)->Label.Set("Edge") = "3";
    C::Out() << g;
  }

  {
    Title("Directory listing");
    Array<String> a;
    Directory::Files("~", a, Directory::FilesAndDirectories);
    C::Out() << "Listing home directory ->";
    C::Out() >> a;
  }

  {
    Value v;
    v["a"] = Ratio(1, 5);
    v["b"] = Box(Vector(3, 4), Vector(5, 6));
    {
      Title("JSON export");
      C::Out() << v << " -> " >> JSON::Export(v);
    }

    {
      Title("JSON import");
      C::Out() << JSON::Import(JSON::Export(v)) << " <- " >> JSON::Export(v);
    }
  }

  {
    Title("Matrices");
    Random r;
    Matrix<number> M(3, 4);
    for(count i = 0; i < 3; i++)
      for(count j = 0; j < 4; j++)
        M(i, j) = r.Between();
    C::Out() >> "Augmented matrix:";
    C::Out() >> M;
    C::Out() >> "Linear solution:";
    C::Out() >> M.LinearSolve();
  }

  {
    Title("Dijkstra's algorithm");
    MazeSolve(34);
  }

  {
    Title("Compile-time environment detection");
    C::Out()++;
    Environment::Print();
  }

  return prim::AutoRelease<prim::Console>();
}

void Title(String s)
{
  static count n = 1;
  C::Out()++;
  if(n < 10)
    C::Out() << " ";
  if(n < 100)
    C::Out() << " ";
  C::Underline();
  C::Out() << n++;
  C::Reset();
  C::Out() << ":";
  C::Bold();
  C::Out() << " " << s << ": ";
  C::Reset();
}

class CostLabel : public GraphTLabel<String>
{
  public:
  number Cost() {return Get("Cost").ToNumber();}
  bool EdgeEquivalent(const GraphTLabel<String>& L) {(void)L; return true;}
};

typedef GraphT<CostLabel> CostLabeledGraph;
typedef Pointer<GraphT<CostLabel>::Object> CostLabeledNode;
typedef Pointer<const GraphT<CostLabel>::Object> ConstNode;

void MazeSolve(count SquareSize)
{
  count m = SquareSize, n = SquareSize;
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
    {
      ascii Character = O(i, j);
      if(Character == '*')
        C::Red();
      else if(Character == 'X')
        C::Green();
      C::Out() << Character << Character;
      C::Reset();
    }
  }
}
