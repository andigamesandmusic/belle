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

#ifndef BELLE_MODERN_SPRING_H
#define BELLE_MODERN_SPRING_H

namespace BELLE_NAMESPACE { namespace modern
{
  /**The spring node represents an island and the spring to the right of it. The
  node is considered active only if k is a non-zero value. The is*/
  struct SpringNode
  {
    ///The island left extent.
    prim::number LeftExtent;
    
    ///The island right extent.
    prim::number RightExtent;
    
    ///Spring force constant.
    prim::number k;
    
    ///Spring equilibrium size.
    prim::number e;
    
    //Calculated...
    prim::number Displacement;
    prim::number Position;
    
    ///Returns whether the node is active (considered as part of the system).
    bool IsActive() const
    {
      return k != 0.0;
    }
    
    ///Activates the spring node.
    void Activate(prim::number k = 1.0)
    {
      if(!IsActive())
        SpringNode::k = k;
    }
    
    ///Deactivates the spring node.
    void Deactivate()
    {
      k = 0.0;
    }
    
    ///Gets the left extent.
    prim::number l()
    {
      return LeftExtent;
    }
    
    ///Gets the right extent.
    prim::number r()
    {
      return RightExtent;
    }
    
    ///Gets the width of the extent.
    prim::number w()
    {
      return RightExtent - LeftExtent;
    }
    
    ///Default constructor creates a blank inactive spring node.
    SpringNode() : LeftExtent(0.0), RightExtent(0.0), k(0.0), e(1.0) {}
    
    ///Converts the node into a string for display purposes.
    operator prim::String () const
    {
      return IsActive() ? "S" : " ";
      prim::String s;
      s << k << ", " << e << ": [" << LeftExtent << ", " << RightExtent << "]";
      return IsActive() ? s : " ";
    }
  };

  struct SeriesSystem : public prim::Array<SpringNode*>
  {
    ///Returns the current width of the system.
    prim::number SystemWidth()
    {
      return z()->Position;
    }
    
    ///Calculates the size of the void.
    prim::number Void()
    {
      prim::number V = SystemWidth();
      for(prim::count i = 0; i < n() - 1; i++)
        V -= ith(i)->r();
      for(prim::count i = 1; i < n(); i++)
        V -= -ith(i)->l();
      return V;
    }
    
    ///Calculates the amount of stretch.
    prim::number Stretch()
    {
      prim::number S = Void();
      for(prim::count i = 0; i < n() - 1; i++)
        S -= ith(i)->e;
      return S;
    }
    
    ///Calculates the amount total equilibrium length.
    prim::number EquilibriumLength()
    {
      return Equilibrium() + Extents();
    }
    
    ///Calculates the total length of the spring equilibrium.
    prim::number Equilibrium()
    {
      prim::number x = 0;
      for(prim::count i = 0; i < n() - 1; i++)
        x += ith(i)->e;
      return x;
    }
    
    ///Calculates the total length of the extents.
    prim::number Extents()
    {
      prim::number x = 0;
      for(prim::count i = 0; i < n() - 1; i++)
        x += ith(i)->r();
      for(prim::count i = 1; i < n(); i++)
        x += -ith(i)->l();
      return x;
    }
    
    ///Returns the number of active springs.
    prim::count Springs()
    {
      return n() - 1;
    }
    
    prim::number k(prim::count i) {return ith(i)->k;}
    
    ///Calculates the displacements due to the spring force.
    void CalculateDisplacements()
    {
      //Pre-calculate some of the knowns.
      prim::count m = Springs();
      prim::number S = Stretch();
      
      //Pre-calculate the denominator (common to all springs).
      prim::number d = 0.0;
      for(prim::count i = 0; i < m; i++)
      {
        prim::number dProduct = 1.0;
        for(prim::count j = 0; j < m; j++)
        {
          if(j == i) continue; //Skip multiplying by k(i).
          dProduct *= k(j);
        }
        d += dProduct;
      }
      
      prim::c >> "Denominator: " << d;
      //Bail out if the denominator is zero or near zero.
      if(prim::Abs(d) < 1.0e-10)
        return;
      
      //Go through each spring and calculate its displacement.
      for(prim::count a = 0; a < m; a++)
      {
        prim::number x = S / d;
        for(prim::count i = 0; i < m; i++)
        {
          if(i == a) continue; //Skip multiplying by k(a)
          x *= k(i);
        }
        
        prim::c >> "Displacement " << a << ": " << x;
        ith(a)->Displacement = x;
      }
      
      //Reposition the islands.
      for(prim::count i = 0; i < n() - 1; i++)
      {
        //Force the first island to have an origin of zero.
        if(!i) ith(i)->Position = 0.0;
        
        prim::number Distance = ith(i)->e + ith(i)->Displacement +
          -ith(i + 1)->l() + ith(i)->r();
        prim::c >> "Distance " << i << ": " << Distance;
        
        ith(i + 1)->Position = ith(i)->Position + Distance;
      }
    }
  };
  
  struct SpringMatrix : public prim::Matrix<SpringNode>
  {
    prim::Array<prim::count> FirstInRow, LastInRow;
    prim::Array<prim::count> ElementsInRow;
    prim::Array<prim::count> ElementsInColumn;

    public:
    
    ///Returns the node count.
    prim::count NodeCount()
    {
      prim::count Total = 0;
      for(prim::count i = 0; i < m(); i++)
        for(prim::count j = 0; j < n(); j++)
          if(ij(i, j).IsActive())
            Total++;
      return Total;
    }
    
    ///Returns true if the matrix can not be simplified any further.
    bool IsSimplified()
    {
      return NodeCount() == 2;
    }
    
    //--------------------------//
    //Cached Information Methods//
    //--------------------------//
    
    ///Examines the matrix to collect information about it.
    void CollectInformation()
    {
      //Determine the first and last nodes for each row.
      FirstInRow.n(m());
      LastInRow.n(m());
      ElementsInRow.n(m());
      ElementsInRow.Zero();
      for(prim::count i = 0; i < m(); i++)
      {
        FirstInRow[i] = LastInRow[i] = -1;
        
        for(prim::count j = 0; j < n(); j++)
        {
          if(ij(i, j).IsActive())
          {
            if(FirstInRow[i] == -1)
              FirstInRow[i] = j;
            LastInRow[i] = j;
            ElementsInRow[i]++;
          }
        }
      }
      
      //Determine the number of elements in each column.
      ElementsInColumn.n(n());
      ElementsInColumn.Zero();
      for(prim::count j = 0; j < n(); j++)
        for(prim::count i = 0; i < m(); i++)
          if(ij(i, j).IsActive())
            ElementsInColumn[j]++;
    }
    
    ///Returns true if the column has one element in it.
    bool IsColumnIndependent(prim::count j)
    {
      return ElementsInColumn[j] == 1;
    }
    
    /**Returns the active springs between two column parallel spring. Note this
    is only valid when there are no intermediate nodes between the columns in
    question.*/
    prim::Array<prim::count> EnumerateSprings(prim::count j1, prim::count j2)
    {
      prim::Array<prim::count> Springs;      
      for(prim::count i = 0; i < m(); i++)
        if(ij(i, j1).IsActive() && ij(i, j2).IsActive())
          Springs.Add() = i;
      return Springs;
    }
    
    /**Returns the active orphans between a two column parallel spring. Note
    this is only valid when there are no intermediate nodes between the columns
    in question.*/
    prim::Array<prim::planar::VectorInt> EnumerateOrphans(prim::count j1,
      prim::count j2)
    {
      prim::Array<prim::planar::VectorInt> Orphans;      
      for(prim::count i = 0; i < m(); i++)
      {
        if(ij(i, j1).IsActive() && !ij(i, j2).IsActive())
        {
          //Make sure j1 is the last active node in the row.
          //if(LastInRow[i] == j1)
            Orphans.Add() = prim::planar::VectorInt(i, j1);
        }
        else if(!ij(i, j1).IsActive() && ij(i, j2).IsActive())
        {
          //Make sure j2 is the first active node in the row.
          //if(FirstInRow[i] == j2)
            Orphans.Add() = prim::planar::VectorInt(i, j2);
        }
      }
      return Orphans;
    }
    
    /**Simulates the effect of the parallel spring reduction to see how many
    nodes would actually be gained or lost.*/
    prim::count NodesToLoseByParallelReduction(
      prim::Array<prim::count>& Springs,
      prim::Array<prim::planar::VectorInt>& Orphans,
      prim::count j1, prim::count j2)
    {
      //If there are no springs, then there is nothing to do.
      if(Springs.n() < 1)
        return 0;
      
      //Determine whether it is a spring or an orphan at the top.
      bool SpringAtTop = !Orphans.n() || Orphans.a().i() > Springs.a();
      
      //Keep track of the total change in nodes.
      prim::count NodesLost = 0;
      
      //If an orphan is at the top, then a node will be added (i.e, -1 is lost).
      if(!SpringAtTop)
        NodesLost--;
      
      //Calculate the change due to reducing springs.
      for(prim::count k = 0; k < Springs.n(); k++)
      {
        //Skip consideration of the top row of the parallel system.
        if(SpringAtTop && !k) continue;
        
        //Retrieve spring row.
        prim::count i = Springs[k];
        
        //If the left spring node is the first in the row, then it disappears.
        if(j1 == FirstInRow[i])
          NodesLost++;

        //If the right spring node is the last in the row, then it disappears.
        if(j2 == LastInRow[i])
          NodesLost++;
      }
      
      //Calculate the change due to disappearing orphans.
      for(prim::count k = 0; k < Orphans.n(); k++)
      {
        //Skip consideration of the top row of the parallel system.
        if(!SpringAtTop && !k) continue;
        
        //Retrieve the orphan coordinate.
        prim::count i = Orphans[k].i();
        prim::count j = Orphans[k].j();
        
        //Orphan can only disappear if it is the only node in the row.
        if(FirstInRow[i] == j && LastInRow[i] == j)
          NodesLost++;
      }
      
      //Return the cumulative number of nodes lost.
      return NodesLost;
    }
    
    ///Finds the next reducible series in the graph.
    prim::Array<prim::planar::VectorInt> FindNextSeries()
    {
      //Array to store a sequence of elements.
      prim::Array<prim::planar::VectorInt> Elements;
      
      //Iterate through each row and look for one containing a series.
      for(prim::count i = 0; i < m(); i++)
      {
        //Check the element count to see if it has at least 3 elements.
        if(ElementsInRow[i] < 3) continue;
        
        //See whether the in-between nodes are in independent columns.
        Elements.Clear();
        Elements.Add() = prim::planar::VectorInt(i, FirstInRow[i]);
        for(prim::count j = FirstInRow[i] + 1; j <= LastInRow[i]; j++)
        {
          //Skip the voids where there are no nodes.
          if(!ij(i, j).IsActive()) continue;
          
          Elements.Add() = prim::planar::VectorInt(i, j);
          
          if(!IsColumnIndependent(j))
          {
            //Iterator hit the end of the series.
            if(Elements.n() >= 3)
              break; //Found a series.
            else
            {
              Elements.a() = Elements.z();
              Elements.n(1); //Start over looking for a series.
            }
          }
        }
        
        //If a series was found then break.
        if(Elements.n() >= 3)
          break;
      }
      
      //If there are not enough elements to form a series clear the array.
      if(Elements.n() < 3)
        Elements.Clear();
      
      //Return the contents of the element array.
      return Elements;
    }

    ///Finds the next reducible parallel system in the graph.
    bool FindNextParallel(prim::Array<prim::count>& Springs,
      prim::Array<prim::planar::VectorInt>& Orphans,
      prim::count& j1, prim::count& j2)
    {
      //Start with small column distances and progress to larger ones.
      for(prim::count d = 1; d < n(); d++)
      {
        for(j1 = 0; j1 < n() - d; j1++)
        {
          //Get the other column index.
          j2 = j1 + d;
         
          //Check in between to make sure the columns are empty.
          bool InBetweenColumnsAreEmpty = true;
          for(prim::count j = j1 + 1; j <= j2 - 1; j++)
          {
            if(ElementsInColumn[j] != 0)
            {
              InBetweenColumnsAreEmpty = false;
              break;
            }
          }
          if(!InBetweenColumnsAreEmpty)
            continue;
          
          //Enumerate the springs and orphans in the parallel system.
          Springs = EnumerateSprings(j1, j2);
          Orphans = EnumerateOrphans(j1, j2);
          
          //See whether the reduction in consideration will help. 
          prim::count NodesLost = NodesToLoseByParallelReduction(
            Springs, Orphans, j1, j2);
          
#if 0
          prim::c >> "Columns: " << j1 << " to " << j2;
          prim::c >> "Springs: " << Springs;
          prim::c >> "Orphans: " << Orphans;
          prim::c >> "Nodes Lost: " << NodesLost;
#endif

          //If nodes are lost (the system reduces) then return success.
          if(NodesLost > 0)
            return true;
        }
      }
      
      //No parallel system could be reduced, so clear the state and return.
      j1 = j2 = 0;
      Springs.Clear();
      Orphans.Clear();
      return false;
    }
  };
  
  struct SpringSolver
  {
    enum SimplificationType
    {
      CouldNotSimplify,
      Series,
      Parallel
    };
    
    static SimplificationType Simplify(const SpringMatrix& S,
      SpringMatrix& Output)
    {
      Output = S;
      Output.CollectInformation();

      if(CollapseNextParallel(Output))
        return Parallel;

      if(CollapseNextSeries(Output))
        return Series;

      return CouldNotSimplify;
    }
    
    static bool CollapseNextSeries(SpringMatrix& Output)
    {
      //Look for a series.
      prim::Array<prim::planar::VectorInt> Series = Output.FindNextSeries();
      if(!Series.n())
        return false;
      
      /*The following code does not yet perform the spring calculations. It only
      simulates the reduction.*/
      
      //Create a series system.
      SeriesSystem s;
      for(prim::count i = 0; i < Series.n(); i++)
        s.Add() = &Output(Series[i].i(), Series[i].j());
      
      prim::c >> "Equilibrium Length: " << s.EquilibriumLength();
      prim::c >> "Equilibrium       : " << s.Equilibrium();
      prim::c >> "Extents           : " << s.Extents();
      
      //Deactivate the in-between springs of the series.
      for(prim::count i = 1; i < Series.n() - 1; i++)
        Output(Series[i].i(), Series[i].j()).Deactivate();
      
      return true;
    }
    
    static bool CollapseNextParallel(SpringMatrix& Output)
    {
      //Declare the state for the parallel system.
      prim::Array<prim::count> Springs;
      prim::Array<prim::planar::VectorInt> Orphans;
      prim::count j1, j2;
      
      //Determine if a reducible parallel system exists.
      if(!Output.FindNextParallel(Springs, Orphans, j1, j2))
        return false;
      
      /*The following code does not yet perform the spring calculations. It only
      simulates the reduction.*/
      
      //Determine whether it is a spring or an orphan at the top.
      bool SpringAtTop = !Orphans.n() || Orphans.a().i() > Springs.a();
      prim::count TopRow = (SpringAtTop ? Springs.a() : Orphans.a().i());
      
      //Activate both nodes at the top.
      Output(TopRow, j1).Activate();
      Output(TopRow, j2).Activate();
      
      //Reduce springs.
      for(prim::count k = 0; k < Springs.n(); k++)
      {
        //Skip consideration of the top row of the parallel system.
        if(SpringAtTop && !k) continue;
        
        //Retrieve spring row.
        prim::count i = Springs[k];
        
        //If the left spring node is the first in the row, then it disappears.
        if(j1 == Output.FirstInRow[i])
          Output(i, j1).Deactivate();
  
        //If the right spring node is the last in the row, then it disappears.
        if(j2 == Output.LastInRow[i])
          Output(i, j2).Deactivate();
      }
      
      //Reduce orphans.
      for(prim::count k = 0; k < Orphans.n(); k++)
      {
        //Skip consideration of the top row of the parallel system.
        if(!SpringAtTop && !k) continue;
        
        //Retrieve the orphan coordinate.
        prim::count i = Orphans[k].i();
        prim::count j = Orphans[k].j();
        
        //Orphan can only disappear if it is the only node in the row.
        if(Output.FirstInRow[i] == j && Output.LastInRow[i] == j)
          Output(i, j).Deactivate();
      }
      
      //A parallel system was reduced.
      return true;
    }
    
    public: //Test functions
    
    static prim::Matrix<prim::count> MakeSeriesTest(void)
    {
      const prim::count Rows = 4, Columns = 4;
      const prim::count Data[Rows][Columns] =
//       {{1, 1, 0, 1},
//        {1, 0, 1, 1}};
///
       {{1, 0, 0, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 0, 0, 1}};
///
      
      return prim::Matrix<prim::count>((prim::count*)Data, Rows, Columns);
    }
    
    static prim::Matrix<prim::count> MakeSeriesParallelTest(void)
    {
      const prim::count Rows = 4, Columns = 3;
      const prim::count Data[Rows][Columns] =
       {{1, 1, 1},
        {1, 1, 1},
        {0, 0, 0},
        {1, 0, 1}};
      
      return prim::Matrix<prim::count>((prim::count*)Data, Rows, Columns);
    }
    
    static void Transfer(const prim::Matrix<prim::count>& M, SpringMatrix& S)
    {
      S.mn(M.m(), M.n());
      for(prim::count i = 0; i < M.m(); i++)
      {
        for(prim::count j = 0; j < M.n(); j++)
        {
          if(M(i, j) == 1)
          {
            S(i, j).Activate();
          }
        }
      }
    }
  };
}}
#endif
