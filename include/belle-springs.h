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

#ifndef BELLE_ENGRAVER_SPRINGS_H
#define BELLE_ENGRAVER_SPRINGS_H

namespace BELLE_NAMESPACE
{
  ///Represents a spring label.
  class SpringLabel
  {
    friend class SpringSystem;

    //---------------//
    //Edge Properties//
    //---------------//
    count  SpringPart;

    number SpringCoefficient;

    number SpringRestLength;

    //---------------//
    //Node Properties//
    //---------------//
    count  NodeID;

    number NodeCalculatedX;

    public:

    ///Sets the part, coefficient, and rest length of the spring.
    void SetSpring(count SpringPart_, number SpringCoefficient_,
      number SpringRestLength_)
    {
      SpringPart = SpringPart_;
      SpringCoefficient = SpringCoefficient_;
      SpringRestLength = SpringRestLength_;
      NodeID = -1;
      NodeCalculatedX = 0.f;
    }

    ///Sets an infinitely te
    void SetRod(count SpringPart_, number SpringRestLength_)
    {
      SpringPart = SpringPart_;
      SpringCoefficient = Limits<number>::Infinity();
      SpringRestLength = SpringRestLength_;
      NodeID = -1;
      NodeCalculatedX = 0.f;
    }

    ///Edge equivalence is part-based for the spring labels.
    bool EdgeEquivalent(const SpringLabel& Filter) const
    {
      return SpringPart == Filter.SpringPart;
    }

    ///Returns the calculated x-value of this node.
    number CalculatedX() const
    {
      return NodeCalculatedX;
    }

    //------------//
    //Constructors//
    //------------//

    ///Creates an empty spring label.
    SpringLabel() : SpringPart(0), SpringCoefficient(0.f),
      SpringRestLength(0.f), NodeID(-1), NodeCalculatedX(0.f) {}
  };

  ///Represents a spring system as a graph of spring edges and nodes.
  class SpringSystem : public GraphT<SpringLabel>
  {
    public:

    typedef Pointer<GraphT<SpringLabel>::Object> Node;
    typedef Pointer<const GraphT<SpringLabel>::Object> ConstNode;
    typedef Pointer<GraphT<SpringLabel>::Object> Edge;
    typedef Pointer<const GraphT<SpringLabel>::Object> ConstEdge;

    /**Gets the horizontal truncation coefficient. This number helps to
    stabilize numerical results from the spring solver.*/
    static number TruncationCoefficient()
    {
      return 1.f / 1024.f;
    }

    ///Gets the minimum spring coefficient.
    static number MinimumCoefficient()
    {
      return 0.000001f;
    }

    ///Gets a rod-like spring coefficient.
    static number RodLikeCoefficient()
    {
      return 1000000.f;
    }

    ///Gets the maximum spring coefficient.
    static number MaximumCoefficient()
    {
      return Limits<number>::Infinity();
    }

    ///Solves the x-positions of the spring system given a total length.
    bool Solve(number TotalLength)
    {
      //Gather all the nodes and edges from the spring system.
      Sortable::Array<SpringSystem::Node> SpringNodes = Nodes();
      Sortable::Array<SpringSystem::ConstEdge> Springs = Edges();

      //Initialize the x-positions of each node and give them ids.
      for(count i = 0; i < SpringNodes.n(); i++)
      {
        SpringNodes[i]->Label.NodeCalculatedX = 0.f;
        SpringNodes[i]->Label.NodeID = i;
      }

      //For exactly two nodes, force the right-most instant to the total length.
      if(SpringNodes.n() == 2)
      {
        if(SpringNodes.a() == Root())
          SpringNodes.z()->Label.NodeCalculatedX = TotalLength;
        else
          SpringNodes.a()->Label.NodeCalculatedX = TotalLength;
      }

      /*If the spring system has two or fewer nodes, no furthing solving is
      required.*/
      if(SpringNodes.n() <= 2 || Springs.n() <= 1)
        return true;

      //Get the first and last nodes, which are anchors and treated special.
      SpringSystem::Node First = Root();
      SpringSystem::Node Last = First;
      if(First)
      {
        SpringSystem::Node Next;
        while((Next = Last->Next(SpringLabel())))
          Last = Next;
      }
      if(!First || !Last)
        return false;

      //Create a coefficient matrix for the number of variables to solve for.
      Matrix<number> M(SpringNodes.n(), SpringNodes.n() + 1);
      M.Zero();

      /*Create the first and last instants as anchors with known positions. This
      helps to generalize the next step, even though it adds two variables (and
      two knowns) to the coefficient matrix.*/
      for(count i = 0; i < SpringNodes.n(); i++)
      {
        if(SpringNodes[i] == First)
        {
          M.ij(i, i) = 1.f;
          M.ij(i, SpringNodes.n()) = 0.f;
        }
        else if(SpringNodes[i] == Last)
        {
          M.ij(i, i) = 1.f;
          M.ij(i, SpringNodes.n()) = TotalLength;
        }
      }

      //Go through each edge and add the forces for the left and right nodes.
      for(count i = 0; i < Springs.n(); i++)
      {
        //Get the left and right nodes.
        SpringSystem::ConstNode Left  = Springs[i]->Tail();
        SpringSystem::ConstNode Right = Springs[i]->Head();

        //Get the spring coefficient and rest length.
        number k = Max(Springs[i]->SpringCoefficient, MinimumCoefficient());
        number RestLength = Springs[i]->SpringRestLength;

        /*The following coefficients are calculated using Hooke's law, F = -kx.
        In this case the x is the delta off of the rest length of the spring,
        or in other words F = -k * (x['right'] - x['left'] - RestLength).
        Therefore the coefficients for the respective variables in the matrix
        are:

        x['left']  :  k
        x['right'] : -k
        coefficient:  k * RestLength

        This is the force acting on the right node. For the force acting on the
        left node, the above are negated since the pull of the spring is in the
        opposite direction.

        Since forces sum, the coefficients for each edge are cumulatively summed
        to an initially zero matrix. The resulting coefficient matrix can then
        be solved using a linear-solve technique such as Gaussian Elimination.*/

        /*Calculate the force due to this spring on the right node. Skip the
        calculation if the right node is the anchor at the end of the system.*/
        if(Right != Last)
        {
          count Row = Right->NodeID;
          M.ij(Row, Left->NodeID)    += k;
          M.ij(Row, Right->NodeID)   -= k;
          M.ij(Row, SpringNodes.n()) -= k * RestLength;
        }

        /*Calculate the force due to this spring on the left node. Skip the
        calculation if the left node is the anchor at the beginning of the
        system.*/
        if(Left != First)
        {
          count Row = Left->NodeID;
          M.ij(Row, Left->NodeID)    -= k;
          M.ij(Row, Right->NodeID)   += k;
          M.ij(Row, SpringNodes.n()) += k * RestLength;
        }
      }

      /*Normalize the rows and treat infinities as extremely large identical
      magnitude numbers.*/
      for(count i = 0; i < M.m(); i++)
      {
        number Highest = 0.f;
        for(count j = 0; j < M.n(); j++)
          Highest = Max(Highest, Abs(M.ij(i, j)));

        if(!Limits<number>::Bounded(Highest))
        {
          for(count j = 0; j < M.n(); j++)
          {
            number x = M.ij(i, j);
            if(x >= MaximumCoefficient())
              x = 1.f;
            else if(x <= -MaximumCoefficient())
              x = -1.f;
            else
              x = 0.f;
            M.ij(i, j) = x;
          }
        }
        else
        {
          for(count j = 0; j < M.n(); j++)
            M.ij(i, j) /= Highest;
        }
      }

      //Solve the matrix.
      Array<number> Solution = M.LinearSolve();
      if(Solution.n() != SpringNodes.n())
        return false;

      //Truncate solution values for output stability.
      for(count i = 0; i < Solution.n(); i++)
      {
        if(!(Solution[i] >= 0.f))
          Solution[i] = 0.f;
        else if(!(Solution[i] <= TotalLength))
          Solution[i] = TotalLength;
        else
          Solution[i] = Truncate(Solution[i], TruncationCoefficient());
      }

      //Update the nodes with the solutions.
      for(count i = 0; i < SpringNodes.n(); i++)
        SpringNodes[i]->Label.NodeCalculatedX = Solution[i];

      return true;
    }

    ///Returns a string representation of the solution.
    Array<Array<number> > Solution() const
    {
      Array<Array<number> > A;
      SpringLabel L;
      bool AddingParts = true;
      while(AddingParts)
      {
        Array<SpringSystem::ConstNode> SpringNodes = Root()->Series(L);
        if(!Root()->Next(L))
          AddingParts = false;
        else
        {
          A.Add();
          for(count i = 0; i < SpringNodes.n(); i++)
            A.z().Add() = SpringNodes[i]->Label.CalculatedX();
        }
        L.SpringPart++;
      }
      return A;
    }

    ///Returns a string representation of the solution.
    String SolutionString() const
    {
      Array<Array<number> > A = Solution();
      String s = "Solution:";
      for(count i = 0; i < A.n(); i++)
      {
        s >> " * [";
        for(count j = 0; j < A[i].n(); j++)
        {
          if(j)
            s << ", ";
          s << A[i][j];
        }
        s << "]";
      }
      return s;
    }

    ///Creates a test spring system with a grid of parallel and series springs.
    void CreateTestGrid(count Columns, count Rows, number SpringConstant = 1.f,
      number SpringRestLength = 1.f)
    {
      Clear();

      Array<SpringSystem::Node> Instants;
      for(count i = 0; i < Columns; i++)
        Instants.Add() = Add();

      for(count i = 0; i < Columns - 1; i++)
        for(count j = 0; j < Rows; j++)
          Connect(Instants[i], Instants[i + 1])->Label.SetSpring(
            j, SpringConstant, SpringRestLength);
    }
  };
}
#endif
