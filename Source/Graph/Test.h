
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

#ifndef BELLE_GRAPH_TEST_H
#define BELLE_GRAPH_TEST_H

#include "Music.h"
#include "Typesetting.h"

namespace BELLE_NAMESPACE { namespace graph
{
  /**Test structure for creating island structures from grids. This is not meant
  to be used for anything other than testing and the interpretation of the grid
  may change over time.*/
  struct IslandGrid : public Music,
    public prim::Array<prim::Array<MusicNode> >
  {
    void PrintIDs(bool Part)
    {
      for(prim::count i = 0; i < n(); i++)
      {
        prim::c >> "";
        for(prim::count j = 0; j < ith(i).n(); j++)
        {
          MusicNode CurrentIsland = ith(i)[j];
          if(CurrentIsland)
          {
            prim::String s;
            prim::count n = (Part ? CurrentIsland->Label.Typesetting->PartID :
              CurrentIsland->Label.Typesetting->InstantID);
            s << n;
            if(n < 10 && n >= 0)
              s << " ";
            prim::c << s << " ";
          }
          else
            prim::c << "   ";
        }
      }
    }
  
    void CreateFromGrid(const prim::count* Grid,
      prim::count Rows, prim::count Columns)
    {
      IslandGrid& g = *this;
      Music::Clear();
      
      //Read in the grid as an grid of islands.
      g.n(Rows);
      MusicNode Top = 0;
      prim::count n = 0;
      for(prim::count i = 0; i < Rows; i++)
      {
        g[i].n(Columns);
        g[i].Zero();
        for(prim::count j = 0; j < Columns; j++)
        {
          bool IslandExists = Grid[i * Columns + j] != 0;
          prim::String s;
          s << n;
          if(n < 10)
            s << " ";
          if(IslandExists)
          {
            MusicNode& NewIsland = g[i][j];
            NewIsland = Music::Add();
            NewIsland->Label.Typesetting = new TypesettingInfo;
            NewIsland->Set("Index") = prim::String(n++);
            NewIsland->Set(mica::Type) = mica::Island;
            if(!Top)
              Top = NewIsland;
          }
        }
      }
      
      //Connect adjacent islands with links.
      for(prim::count i = 0; i < Rows; i++)
      {
        for(prim::count j = 0; j < Columns; j++)
        {
          MusicNode Current = g[i][j];
          MusicNode NextOver;
          if(j + 1 < Columns)
            NextOver = g[i][j + 1];
          
          if(!Current) continue;
          
          if(NextOver && Grid[i * Columns + j] != 3)
            Connect(Current, NextOver)->Set(mica::Type) = mica::Partwise;
          
          for(prim::count k = i + 1; k < Rows; k++)
          {
            MusicNode NextDown = g[k][j];
            if(NextDown && Grid[i * Columns + j] != 2)
            {
              Connect(Current, NextDown)->Set(mica::Type) = mica::Instantwise;
              break;
            }
          }
        }
      }
      g.Root(Top);
    }
  
    void MakeNullTest(void)
    {
      //0 nothing, 1 island, 2 island horizontal, 3 island vertical
      const prim::count Rows = 10, Columns = 10;
      const prim::count Data[Rows][Columns] =
       {{0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}};
      CreateFromGrid(Data[0], Rows, Columns);
    }
    
    void MakeInstantTest(void)
    {
      //0 nothing, 1 island, 2 island horizontal only, 3 island vertical only
      const prim::count Rows = 10, Columns = 10;
      const prim::count Data[Rows][Columns] =
       {{0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 1, 2,  2, 1, 1, 0, 0}, 
        {0, 0, 0, 1, 2,  1, 2, 1, 0, 0}, 
        {0, 0, 0, 1, 1,  2, 2, 1, 0, 0}, 
        {0, 0, 0, 1, 2,  2, 2, 1, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}};
      CreateFromGrid(Data[0], Rows, Columns);
    }
    
    void MakeDiagonalTest(void)
    {
      const prim::count Rows = 10, Columns = 10;
      const prim::count Data[Rows][Columns] =
       {{1, 1, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 1, 1, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 1, 1, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 1, 1,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 1,  1, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  1, 1, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 1, 1, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 1, 1, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 1, 1}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 1}};
      CreateFromGrid(Data[0], Rows, Columns);
    }
    
    void MakeCrossTest(void)
    {
      const prim::count Rows = 10, Columns = 10;
      const prim::count Data[Rows][Columns] =
       {{1, 1, 0, 0, 0,  0, 0, 0, 1, 1}, 
        {0, 1, 1, 0, 0,  0, 0, 1, 1, 0}, 
        {0, 0, 1, 1, 0,  0, 1, 1, 0, 0}, 
        {0, 0, 0, 1, 1,  1, 1, 0, 0, 0}, 
        {0, 0, 0, 0, 1,  1, 0, 0, 0, 0}, 
        {0, 0, 0, 1, 1,  1, 1, 0, 0, 0}, 
        {0, 0, 1, 1, 0,  0, 1, 1, 0, 0}, 
        {0, 1, 1, 0, 0,  0, 0, 1, 1, 0}, 
        {1, 1, 0, 0, 0,  0, 0, 0, 1, 1}, 
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 1}};
      CreateFromGrid(Data[0], Rows, Columns);
    }
    
    void MakeSpiralTest(void)
    {
      const prim::count Rows = 10, Columns = 10;
      const prim::count Data[Rows][Columns] =
       {{1, 1, 1, 1, 1,  1, 1, 1, 1, 1}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 1}, 
        {1, 1, 1, 1, 1,  1, 1, 1, 0, 1}, 
        {1, 0, 0, 0, 0,  0, 0, 1, 1, 1}, 
        {1, 0, 1, 1, 1,  1, 0, 1, 0, 1}, 
        {1, 0, 1, 0, 0,  1, 0, 1, 0, 1}, 
        {1, 1, 1, 0, 0,  1, 0, 1, 0, 1}, 
        {1, 0, 1, 1, 1,  1, 1, 1, 0, 1}, 
        {1, 0, 0, 0, 0,  0, 0, 0, 0, 1}, 
        {1, 1, 1, 1, 1,  1, 1, 1, 1, 1}};
      CreateFromGrid(Data[0], Rows, Columns);
    }
    
    void MakeOssiaAndStaffChangeTest(void)
    {
      const prim::count Rows = 10, Columns = 10;
      const prim::count Data[Rows][Columns] =
       {{0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {1, 1, 1, 1, 0,  1, 1, 0, 0, 0}, 
        {0, 0, 1, 1, 1,  1, 1, 1, 1, 1}, 
        {1, 1, 1, 1, 1,  1, 1, 1, 1, 1}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0,  0, 0, 0, 0, 0}};
      CreateFromGrid(Data[0], Rows, Columns);
    }
  };
}}
#endif
