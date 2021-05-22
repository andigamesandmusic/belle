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

#ifndef BELLE_ENGRAVER_INSTANTSTATE_H
#define BELLE_ENGRAVER_INSTANTSTATE_H

namespace BELLE_NAMESPACE
{
  /**State related to information within a single instant and its islands.
  Instant state for a particular island may accumulate.*/
  class InstantState
  {
    /*Static interface -- instances not allowed*/ InstantState();

    ///Accumulates information into the instant state.
    static void AccumulateStateForInstant(Music::ConstNode IslandNode)
    {
      if(!IslandNode)
        return;

      IslandNode->Label.SetState("InstantState").NewTree();

      if(Music::ConstNode Previous =
        IslandNode->Previous(MusicLabel(mica::Instantwise)))
      {
        Value& PreviousState = Previous->Label.SetState("PartState");
        Value& CurrentState = IslandNode->Label.SetState("PartState");

        IslandNode->Label.SetState(
          "InstantState", "BarlineConnectsToPreviousStaff") =
          PreviousState["Staff"]["Connects"];
        IslandNode->Label.SetState(
          "InstantState", "InteriorDistanceToPreviousStaff") =
          (+PreviousState["Staff"]["Offset"] -
          (+PreviousState["Staff"]["Lines"] - 1.f) / 2.f) -
          (+CurrentState["Staff"]["Offset"] +
          (+CurrentState["Staff"]["Lines"] - 1.f) / 2.f);
      }
    }

    public:

    ///Accumulates partwise state for each island.
    static void Accumulate(Pointer<const Music> M)
    {
      //Validate parameters.
      if(!M) return;

      /*Start at the root and for each island heading instantwise, traverse
      partwise. #limitation : does not take into account non-grid scores.
      Should traverse by geometry.*/
      Music::ConstNode m, n;
      for(m = M->Root(); m; m = m->Next(MusicLabel(mica::Instantwise)))
        for(n = m; n; n = n->Next(MusicLabel(mica::Partwise)))
          AccumulateStateForInstant(n);
    }
  };
}
#endif
