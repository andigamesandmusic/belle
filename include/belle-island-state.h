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

#ifndef BELLE_ENGRAVER_ISLANDSTATE_H
#define BELLE_ENGRAVER_ISLANDSTATE_H

namespace BELLE_NAMESPACE
{
  ///Algorithms for typesetting an island.
  class IslandState
  {
    /*Static interface -- instances not allowed*/ IslandState();

    ///Copies an island string attribute to a state value key.
    static void CopyAttributeToValue(Music::ConstNode IslandNode,
      Value& StateValue, const String& AttributeKey,
      String DestinationAttribute, Value DefaultValue)
    {
      if(!DestinationAttribute)
        DestinationAttribute = AttributeKey;

      //Do not copy over existing value in state if it exists.
      if(!StateValue[DestinationAttribute].IsNil())
        return;

      String AttributeValue = IslandNode->Get(AttributeKey);
      if(!AttributeValue)
        StateValue[DestinationAttribute] = DefaultValue;
      else if(AttributeValue == "true")
        StateValue[DestinationAttribute] = true;
      else if(AttributeValue == "false")
        StateValue[DestinationAttribute] = false;
      else if(AttributeValue)
        StateValue[DestinationAttribute] = AttributeValue.ToNumber();
    }

    ///Inspects an array of valid chord tokens.
    static void AssumeChordTokensAndInspect(
      const Array<Music::ConstNode>& Tokens, Value& IslandState)
    {
      for(count i = 0; i < Tokens.n(); i++)
      {
        Music::ConstNode Token = Tokens[i];
        Pointer<const Value::Base> TokenBase = Token;
        IslandState["Chord"][TokenBase]["DiatonicPitch"] =
          Utility::GetPitchExtremes(Token->Children(MusicLabel(mica::Note)));
      }
    }

    ///Inspects the tokens of a valid island node.
    static void AssumeNodeAndInspectTokens(Music::ConstNode IslandNode,
      Value& IslandState)
    {
      //Gather all the tokens in the island.
      Array<Music::ConstNode> Tokens =
        IslandNode->Children(MusicLabel(mica::Token));

      //If there are no tokens in the island, there is no part state.
      if(!Tokens.n()) return;

      //Handle chords.
      if(Tokens.a()->Label.Get(mica::Kind) == mica::Chord)
        AssumeChordTokensAndInspect(Tokens, IslandState);
    }

    ///Accumulates information into the partwise state.
    static void AccumulateStateForIsland(Music::ConstNode IslandNode)
    {
      //Validate parameters.
      if(!IslandNode) return;

      //Create a new island state.
      Value& IslandState = IslandNode->Label.SetState("IslandState");
      Value& IslandStaffState = IslandState["Staff"];

      /*#todo: Ideally, staff attribute defaults should only apply when the
      staff enters. Since part state merges island state, defaults can end up
      overriding state that was intended to be carried along the part.*/

      //Copy staff-related attribute changes to the island state.
      CopyAttributeToValue(IslandNode, IslandStaffState,
        "StaffScale", "Scale", Value(1.f));
      CopyAttributeToValue(IslandNode, IslandStaffState,
        "StaffOffset", "Offset", Value()); //No default -- carry over.
      CopyAttributeToValue(IslandNode, IslandStaffState,
        "StaffLines", "Lines", Value(5.f));
      CopyAttributeToValue(IslandNode, IslandStaffState,
        "StaffConnects", "Connects", Value(true));

      //Inspect tokens and gather data related to the tokens.
      AssumeNodeAndInspectTokens(IslandNode, IslandState);
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
          AccumulateStateForIsland(n);
    }
  };
}
#endif
