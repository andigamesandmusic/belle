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

#ifndef BELLE_ENGRAVER_TRANSITIVE_H
#define BELLE_ENGRAVER_TRANSITIVE_H

namespace BELLE_NAMESPACE
{
  ///Solves a transitive closure
  class TransitiveClosure
  {
    public: //enumerations

    enum Equality
    {
      Undefined,
      LessThan,
      GreaterThan,
      Conflicted
    };

    public: //methods

    TransitiveClosure(count Size_)
    {
      Clear(Size_);
    }

    void Clear(count Size_)
    {
      Size = Size_;
      Matrix.n(Size * Size);
      Matrix.Zero();
      Conflict = false;
    }

    count n() const {return Size;}

    bool IsConflicted() const {return Conflict;}

    Equality Get(count i, count j) const
    {
      if(!Normal(i, j))
      {
        Swap(i, j);
        return Flip(ij(i, j));
      }
      return ij(i, j);
    }

    bool Set(count i, count j, Equality Value)
    {
      if(i < 0 || j < 0 || i >= Size || j >= Size)
      {
        Conflict = true;
        return false;
      }

      if(ij(i, j) == Flip(Value))
      {
        Conflict = true;
        ij(i, j) = Conflicted;
        Swap(i, j);
        ij(i, j) = Conflicted;
        return false;
      }

      ij(i, j) = Value;
      Swap(i, j);
      ij(i, j) = Flip(Value);
      return true;
    }

    void Close(void)
    {
      while(Iterate()) {}
    }

    operator String(void) const
    {
      String s;
      s >> "   ";
      for(count i = 0; i < Size; i++)
      {
        s << i << " ";
        if(i < 10) s << " ";
      }

      s >> "  /";
      for(count i = 0; i < Size; i++)
        s << "---";

      for(count i = 0; i < Size; i++)
      {
        s >> i;
        if(i < 10)
          s << " ";
        s << "|";
        for(count j = 0; j < Size; j++)
        {
          count v = Get(i, j);
          ascii c = ' ';
          if(v == LessThan) c = '<';
          else if(v == GreaterThan) c = '>';
          else if(v == Conflicted) c = '!';
          else if(i == j) c = '=';
          s << c << "  ";
        }
      }
      return s;
    }

    private: //static methods

    static Equality Flip(Equality x)
    {
      if(x == LessThan)
        x = GreaterThan;
      else if(x == GreaterThan)
        x = LessThan;
      return x;
    }

    static bool Normal(count i, count j)
    {
      return i >= j;
    }

    private: //members

    Array<Equality> Matrix;

    count Size;

    bool Conflict; PRIM_PAD(bool)

    private: //methods

    Equality& ij(count i, count j)
    {
      return Matrix[i * Size + j];
    }

    const Equality& ij(count i, count j) const
    {
      return Matrix[i * Size + j];
    }

    bool Follow(count m, count n)
    {
      Equality v = Get(m, n);
      Equality vinv = (v == LessThan ? GreaterThan : LessThan);

      //Row-normal
      for(count j = 0; j < m; j++)
      {
        if(j == n) continue;
        if(Get(m, j) == vinv && Get(n, j) != vinv)
          return Set(n, j, vinv);
      }

      //Column-reflection
      for(count j = 0; j < n; j++)
      {
        if(Get(n, j) == v && Get(m, j) != v)
          return Set(m, j, v);
      }

      //Column-normal
      for(count i = n + 1; i < Size; i++)
      {
        if(i == m) continue;
        if(Get(i, n) == vinv && Get(i, m) != vinv)
          return Set(i, m, vinv);
      }

      //Row-reflection
      for(count i = m + 1; i < Size; i++)
      {
        if(Get(i, m) == v && Get(i, n) != v)
          return Set(i, n, v);
      }

      return false;
    }

    bool Iterate(void)
    {
      for(count i = 1; i < Size; i++)
        for(count j = 0; j < i; j++)
          if(ij(i, j) == LessThan || ij(i, j) == GreaterThan)
            if(Follow(i, j))
              return true;
      return false;
    }

    public: //documentation

    static void Explain()
    {
      C::Out() >> "See TransitiveMapping::Explain()...";
    }
  };

  ///Call TransitiveMapping::Explain() for more information.
  class TransitiveMapping
  {
    public: //methods

    TransitiveMapping(count Size) : Map(Size)
    {
      Ordering.n(Size);
      for(count i = 0; i < Ordering.n(); i++)
        Ordering[i] = i;
    }

    bool IsConflicted() const
    {
      return Map.IsConflicted();
    }

    TransitiveClosure::Equality Get(count i, count j) const
    {
      return Map.Get(i, j);
    }

    bool Set(count i, count j, TransitiveClosure::Equality Value)
    {
      return Map.Set(i, j, Value);
    }

    void Solve()
    {
      //Create the transitive closure.
      Map.Close();

      //Perform a bubble-sort against the inequality matrix.
      count n0 = Ordering.n();
      count n_1 = n0 - 1;
      for(count i = 0; i < n_1; i++)
        for(count j = i + 1; j < n0; j++)
          if(Map.Get(Ordering[i], Ordering[j]) ==
            TransitiveClosure::GreaterThan)
              Swap(Ordering[i], Ordering[j]);
    }

    count n() const
    {
      return Ordering.n();
    }

    count Mapping(count i) const
    {
      return Ordering[i];
    }

    operator String () const
    {
      String s;
      s >> Map;
      s >> "Mapping   : ";
      for(count i = 0; i < Ordering.n(); i++)
        s << Ordering[i] << " ";
      s >> "Conflicted: " << (IsConflicted() ? "Yes" : "No");
      return s;
    }

    private: //members

    TransitiveClosure Map;

    Array<count> Ordering;

    public: //documentation

    static void Explain()
    {
      String s;
      s >> "A TransitiveMapping is used to solve the staff ordering problem.";
      s >> "Here is a mapping such that IDs '2' > '3', '0' > '2', '1' > '2'";
      s >> "and '0' > '1'.";
      TransitiveMapping tm(4);
      tm.Set(2, 3, TransitiveClosure::GreaterThan);
      tm.Set(0, 2, TransitiveClosure::GreaterThan);
      tm.Set(1, 2, TransitiveClosure::GreaterThan);
      tm.Set(0, 1, TransitiveClosure::GreaterThan);
      s >> tm;
      s++;

      s >> "Calling Solve() will compute the transitive closure and will fix";
      s >> "the mapping, resulting in '3' < '2' < '1' < '0'.";
      tm.Solve();
      s >> tm;
      s++;

      s >> "If conflicting inequalities are set, then the transitive closure";
      s >> "will be in a conflicted state. For example: '2' > '0', '0' > '1',";
      s >> "and '1' > '0':";
      TransitiveMapping Conflicted(3);
      Conflicted.Set(2, 0, TransitiveClosure::GreaterThan);
      Conflicted.Set(0, 1, TransitiveClosure::GreaterThan);
      Conflicted.Set(1, 0, TransitiveClosure::GreaterThan);
      tm.Solve();
      s >> Conflicted;
      s++;
      C::Out() >> s;
    }
  };
}
//Add the Nothing<T> definition for TransitiveClosure::Equality.
namespace prim
{
  template <> inline Nothing<belle::TransitiveClosure::Equality>::
    operator belle::TransitiveClosure::Equality() const
    {return belle::TransitiveClosure::Undefined;}
}
#endif
