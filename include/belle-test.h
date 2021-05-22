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

#ifndef BELLE_ENGRAVER_TEST_H
#define BELLE_ENGRAVER_TEST_H

namespace BELLE_NAMESPACE
{
  /**Test structure for creating island structures from grids. This is not meant
  to be used for anything other than testing and the interpretation of the grid
  may change over time.*/
  struct IslandGrid : public Music, public Array<Array<Music::Node> >
  {
    void PrintIDs(bool Part)
    {
      typedef Array<Array<Music::Node> > MusicNodeArray;
      for(count i = 0; i < MusicNodeArray::n(); i++)
      {
        C::Out() >> "";
        for(count j = 0; j < MusicNodeArray::ith(i).n(); j++)
        {
          Music::Node CurrentIsland = MusicNodeArray::ith(i)[j];
          if(CurrentIsland)
          {
            String s;
            count n = (Part ?
              CurrentIsland->Label.GetState("PartID").AsCount() :
              CurrentIsland->Label.GetState("InstantID").AsCount());
            s << n;
            if(n < 10 && n >= 0)
              s << " ";
            C::Out() << s << " ";
          }
          else
            C::Out() << "   ";
        }
      }
    }

    void CreateFromGrid(const count* Grid,
      count Rows, count Columns)
    {
      IslandGrid& g = *this;
      Music::Clear();

      //Read in the grid as an grid of islands.
      g.n(Rows);
      Music::Node Top = 0;
      count n = 0;
      for(count i = 0; i < Rows; i++)
      {
        g[i].n(Columns);
        g[i].Zero();
        for(count j = 0; j < Columns; j++)
        {
          bool IslandExists = Grid[i * Columns + j] != 0;
          String s;
          s << n;
          if(n < 10)
            s << " ";
          if(IslandExists)
          {
            Music::Node& NewIsland = g[i][j];
            NewIsland = Music::Add();
            NewIsland->Set("Index") = String(n++);
            NewIsland->Set(mica::Type) = mica::Island;
            if(!Top)
              Top = NewIsland;
          }
        }
      }

      //Connect adjacent islands with links.
      for(count i = 0; i < Rows; i++)
      {
        for(count j = 0; j < Columns; j++)
        {
          Music::Node Current = g[i][j];
          Music::Node NextOver;
          if(j + 1 < Columns)
            NextOver = g[i][j + 1];

          if(!Current) continue;

          if(NextOver && Grid[i * Columns + j] != 3)
            this->Connect(Current, NextOver)->Set(mica::Type) = mica::Partwise;

          for(count k = i + 1; k < Rows; k++)
          {
            Music::Node NextDown = g[k][j];
            if(NextDown && Grid[i * Columns + j] != 2)
            {
              this->Connect(Current, NextDown)->Set(mica::Type) =
                mica::Instantwise;
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
      const count Rows = 10, Columns = 10;
      const count Islands[Rows][Columns] =
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
      CreateFromGrid(Islands[0], Rows, Columns);
    }

    void MakeInstantTest(void)
    {
      //0 nothing, 1 island, 2 island horizontal only, 3 island vertical only
      const count Rows = 10, Columns = 10;
      const count Islands[Rows][Columns] =
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
      CreateFromGrid(Islands[0], Rows, Columns);
    }

    void MakeDiagonalTest(void)
    {
      const count Rows = 10, Columns = 10;
      const count Islands[Rows][Columns] =
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
      CreateFromGrid(Islands[0], Rows, Columns);
    }

    void MakeCrossTest(void)
    {
      const count Rows = 10, Columns = 10;
      const count Islands[Rows][Columns] =
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
      CreateFromGrid(Islands[0], Rows, Columns);
    }

    void MakeSpiralTest(void)
    {
      const count Rows = 10, Columns = 10;
      const count Islands[Rows][Columns] =
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
      CreateFromGrid(Islands[0], Rows, Columns);
    }

    void MakeOssiaAndStaffChangeTest(void)
    {
      const count Rows = 10, Columns = 10;
      const count Islands[Rows][Columns] =
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
      CreateFromGrid(Islands[0], Rows, Columns);
    }
  };
}
#endif
