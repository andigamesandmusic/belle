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

#ifndef BELLE_ENGRAVER_EQUIVALENCE_H
#define BELLE_ENGRAVER_EQUIVALENCE_H

namespace BELLE_NAMESPACE
{
  /**Tests various graph objects for equivalence. To be equivalent means to be
  indistinguishable (not that the objects are literally the same reference.)*/
  class Equivalence
  {
    ///Determines whether two notes are equivalent.
    static bool NotesAreEquivalent(Music::ConstNode i, Music::ConstNode j)
    {
      if(!i || !j)
        return false;
      return i->Label.GetState() == j->Label.GetState();
    }

    ///Determines whether two tokens are equivalent.
    static bool TokensAreEquivalent(Music::ConstNode i, Music::ConstNode j)
    {
      //Note this function is very similar to the IslandsAreEquivalent method.
      if(!i || !j)
        return false;

      //Get the notes in each token.
      Array<Music::ConstNode> u = i->Children(Music::Label(mica::Note));
      Array<Music::ConstNode> v = j->Children(Music::Label(mica::Note));

      //Check that the number of notes matches.
      if(u.n() != v.n()) return false;

      //Check that the content of the tokens matches.
      if(i->Label.GetState() != j->Label.GetState()) return false;

      /*The following check determines whether every note in one token has an
      equivalent note in the other token, and vice versa. The reverse case
      needs to be tested since [X X] has all notes in [X Y], but [X Y] does not
      have all its notes in [X X].*/
      for(count a = 0; a < u.n(); a++)
      {
        bool FoundMatchUV = false, FoundMatchVU = false;
        for(count b = 0; b < u.n(); b++)
        {
          if(NotesAreEquivalent(u[a], v[b]))
            FoundMatchUV = true;
          if(NotesAreEquivalent(u[b], v[a]))
            FoundMatchVU = true;
        }
        if(!FoundMatchUV || !FoundMatchVU)
          return false;
      }
      return true;

    }

    ///Determines whether two islands are equivalent.
    static bool IslandsAreEquivalent(Music::ConstNode i, Music::ConstNode j)
    {
      if(!i || !j)
        return false;

      //Get the tokens in each island.
      Array<Music::ConstNode> u = i->Children(Music::Label(mica::Token));
      Array<Music::ConstNode> v = j->Children(Music::Label(mica::Token));

      //Check that the number of tokens matches.
      if(u.n() != v.n()) return false;

      //Check that the content of the islands matches.
      if(i->Label.GetState() != j->Label.GetState()) return false;

      /*The following check determines whether every token in one island has an
      equivalent token in the other island, and vice versa. The reverse case
      needs to be tested since [X X] has all tokens in [X Y], but [X Y] does not
      have all its tokens in [X X].*/
      for(count a = 0; a < u.n(); a++)
      {
        bool FoundMatchUV = false, FoundMatchVU = false;
        for(count b = 0; b < u.n(); b++)
        {
          if(TokensAreEquivalent(u[a], v[b]))
            FoundMatchUV = true;
          if(TokensAreEquivalent(u[b], v[a]))
            FoundMatchVU = true;
        }
        if(!FoundMatchUV || !FoundMatchVU)
          return false;
      }
      return true;
    }

    /**Computes whether the graphs are equal. Note that this operation is only
    intended to solve the special case of solving whether.*/
    static bool GraphsAreEquivalent(const Music& x, const Music& y)
    {
      /*The first check is whether the geometry of the graphs are the same. This
      significantly optimizes the isomorphism problem since the geometry can be
      parsed in polynomial time.*/
      Geometry gx, gy;
      gx.Parse(x);
      gy.Parse(y);
      if(gx != gy)
        return false;

      //The next check is to determine whether the islands are identical.
      for(count i = 0; i < gx.GetNumberOfParts(); i++)
      {
        for(count j = 0; j < gx.GetNumberOfInstants(); j++)
        {
          if(!gx(i, j))
            continue;
          if(!IslandsAreEquivalent(gx(i, j), gy(i, j)))
            return false;
        }
      }

      return true;
    }
  };
}
#endif
