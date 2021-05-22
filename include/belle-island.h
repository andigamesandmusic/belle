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

#ifndef BELLE_ENGRAVER_ISLAND_H
#define BELLE_ENGRAVER_ISLAND_H

namespace BELLE_NAMESPACE
{
  ///Algorithms for typesetting an island.
  class Island
  {
    /*Static interface -- instances not allowed*/ Island();

    ///Retrieves the requested color of a token.
    static void SetStampToTokenColor(
      Pointer<Stamp> IslandStamp, Music::ConstNode Token)
    {
      if(!IslandStamp || !Token) return;

      if(String ColorString = Token->Label.Get("Color"))
        IslandStamp->SetColorOfGraphics(Color(ColorString));
    }

    ///Engraves the tokens in an island.
    static void EngraveTokens(Music::ConstNode IslandNode)
    {
      //Find all the tokens in the island.
      Array<Music::ConstNode> Tokens =
        IslandNode->Children(MusicLabel(mica::Token));

      if(not Tokens.n())
        return;

      //Get the island stamp.
      Pointer<Stamp> IslandStamp = IslandNode->Label.Stamp().Object();

      //Get the token and token kind.
      Music::ConstNode Token = Tokens.a();
      mica::Concept Kind = Token->Label.Get(mica::Kind);

      //Engrave the token with the token-specific engraver.
      if(Kind == mica::Barline)
        Barline::Engrave(IslandNode, Token);
      else if(Kind == mica::Clef)
        Clef::Engrave(IslandNode, Token);
      else if(Kind == mica::TimeSignature)
        TimeSignatureEngrave(IslandNode, Token);
      else if(Kind == mica::KeySignature)
        KeySignature::Engrave(IslandNode, Token);
      else if(Kind == mica::Chord)
        EngraveMultivoice(IslandNode, Tokens);

      //Remember the size of just the token content.
      IslandNode->Label.SetState("IslandState", "TokenBounds") =
        IslandStamp->Bounds();

      //Apply token coloring if necessary.
      SetStampToTokenColor(IslandStamp, Token);
    }

    ///Engraves a single island.
    static void EngraveIsland(Music::ConstNode IslandNode)
    {
      //Get the island stamp.
      Pointer<Stamp> IslandStamp = IslandNode->Label.Stamp().Object();

      //Clears any existing typesetting in the stamp.
      IslandStamp->ClearTypesetting();

      //If there are no tokens, then typesetting is finished.
      EngraveTokens(IslandNode);

      //Indicate that typesetting is finished.
      IslandStamp->FinishedTypesetting();
    }

    public:

    ///Typesets only the islands needing to be typeset.
    static void EngraveIslands(Pointer<const Music> M, Pointer<const Value> H)
    {
      //Validate parameters.
      if(!M) return;

      /*Start at the root and for each island heading instantwise, traverse
      partwise. #limitation : does not take into account non-grid scores.
      Should traverse by geometry.*/
      Music::ConstNode m, n;
      for(m = M->Root(); m; m = m->Next(MusicLabel(mica::Instantwise)))
      {
        Value PartState;
        for(n = m; n; n = n->Next(MusicLabel(mica::Partwise)))
        {
          /*Create a new empty stamp. May want to reuse stamps later and only
          retypeset the ones that need it (for an editor situation).*/
          n->Label.Stamp() = new Stamp;
          n->Label.SetState("HouseStyle", "Global") =
            new Value::ConstReference(H);

          //Engrave the island.
          EngraveIsland(n);
        }
      }
    }
  };
}
#endif
