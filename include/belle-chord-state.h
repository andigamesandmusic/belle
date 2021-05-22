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

///@name Chord State
///@{

///Accumulates information into the partwise state.
void AccumulateChordStateForPart(Music::ConstNode TokenNode,
  Value& LabelStateValue);

///@}

namespace internals
{
  ///Determines part state staff position extremes from the island state.
  void AssumeAndDeduceStaffPositionExtremes(
    Music::ConstNode TokenNode, Value& LabelStateValue);

  ///Calculates the purely staff-position based stem direction.
  void AssumeAndCalculateStaffPositionStemDirection(
    Music::ConstNode TokenNode, Value& LabelStateValue);

  ///Determines if the chord is beamed and updates the beam state.
  void UpdateBeamState(Music::ConstNode TokenNode,
    Value& LabelStateValue);
}

#ifdef BELLE_IMPLEMENTATION

///Accumulates information into the partwise state.
void AccumulateChordStateForPart(Music::ConstNode TokenNode,
  Value& LabelStateValue)
{
  //Validate parameters
  if(not TokenNode) return;

  internals::UpdateBeamState(TokenNode, LabelStateValue);
  internals::AssumeAndDeduceStaffPositionExtremes(TokenNode, LabelStateValue);
  internals::AssumeAndCalculateStaffPositionStemDirection(
    TokenNode, LabelStateValue);
  AccidentalStateAssumeAndAccumulateAccidentalStateForPart(
    TokenNode, LabelStateValue);
}

namespace internals
{
  ///Determines part state staff position extremes from the island state.
  void AssumeAndDeduceStaffPositionExtremes(
    Music::ConstNode TokenNode, Value& LabelStateValue)
  {
    //Get the active clef for this chord.
    mica::Concept ActiveClef = LabelStateValue["PartState"]["Clef"]["Active"];

    //Get the island state chord reference.
    Value& DiatonicPitch =
      LabelStateValue["IslandState"]["Chord"][TokenNode]["DiatonicPitch"];
    Value& StaffPosition =
      LabelStateValue["PartState"]["Chord"][TokenNode]["StaffPosition"];

    //Calculate the highest and lowest staff positions.
    StaffPosition["Highest"] = mica::map(
      mica::Concept(DiatonicPitch["Highest"]), ActiveClef);
    StaffPosition["Lowest"] = mica::map(
      mica::Concept(DiatonicPitch["Lowest"]), ActiveClef);
  }

  ///Calculates the purely staff-position based stem direction.
  void AssumeAndCalculateStaffPositionStemDirection(
    Music::ConstNode TokenNode, Value& LabelStateValue)
  {
    //Find previous direction.
    mica::Concept PreviousDirection;
    mica::Concept PreviousBeamedDirection;
    bool PreviousHasBeam = false;
    integer PreviousAverageNote = 0;
    {
      Music::ConstNode PreviousTokenNode;
      if(IslandOfToken(TokenNode))
        if(Music::ConstNode PreviousIsland =
          IslandOfToken(TokenNode)->Previous(MusicLabel(mica::Partwise)))
            PreviousTokenNode = PreviousIsland->Next(MusicLabel(mica::Token));
      Music::ConstNode PreviousTokenNodeByBeam =
        TokenNode->Previous(MusicLabel(mica::Beam));
      PreviousHasBeam = PreviousTokenNodeByBeam;
      if(PreviousTokenNode)
      {
        PreviousDirection = LabelStateValue["PartState"]["PreviousChord"]
          [PreviousTokenNode]["StemDirectionSingleVoice"];
        PreviousBeamedDirection = LabelStateValue["PartState"]["PreviousChord"]
          [PreviousTokenNode]["StemDirectionSingleVoiceBeamed"];
        if(PreviousHasBeam)
          PreviousDirection = PreviousBeamedDirection;
        integer PreviousHighestNote =
          LabelStateValue["PartState"]["PreviousChord"][PreviousTokenNode]
          ["StaffPosition"]["Highest"];
        integer PreviousLowestNote =
          LabelStateValue["PartState"]["PreviousChord"][PreviousTokenNode]
          ["StaffPosition"]["Lowest"];
        PreviousAverageNote = (PreviousHighestNote + PreviousLowestNote) / 2;
      }
    }

    Value& Chord = LabelStateValue["PartState"]["Chord"][TokenNode];
    Value& StaffPosition = Chord["StaffPosition"];
    bool HasBeam = Chord["StemHasBeam"].AsBoolean();
    integer HighestNote = StaffPosition["Highest"];
    integer LowestNote = StaffPosition["Lowest"];
    integer AverageNote = (HighestNote + LowestNote) / 2;
    integer StaffPositionOfFurthestNote =
      (Abs(HighestNote) >= Abs(LowestNote)) ? HighestNote : LowestNote;
    StaffPosition["StemDirection"] =
      (StaffPositionOfFurthestNote <= 0 ? mica::Up : mica::Down);

    //Apply any overrides and deduce the final stem direction.
    mica::Concept Direction = TokenNode->Get(mica::StemDirection);
    mica::Concept FinalDirection;
    if(HasBeam and PreviousHasBeam and not undefined(PreviousDirection))
      FinalDirection = PreviousDirection; //Beam overrides all
    else if(not undefined(Direction))
      FinalDirection = Direction; //Manual override
    else if(Abs(AverageNote) <= 1 and not undefined(PreviousDirection) and
      Abs(PreviousAverageNote) <= 2)
        FinalDirection = PreviousDirection; //Context override
    else
      FinalDirection = StaffPosition["StemDirection"]; //By staff position
    Chord["StemDirectionSingleVoice"] = FinalDirection;
    if(HasBeam or PreviousHasBeam)
      Chord["StemDirectionSingleVoiceBeamed"] = FinalDirection;
    else if(not undefined(PreviousBeamedDirection))
      Chord["StemDirectionSingleVoiceBeamed"] = PreviousBeamedDirection;
  }

  ///Determines if the chord is beamed and updates the beam state.
  void UpdateBeamState(Music::ConstNode TokenNode,
    Value& LabelStateValue)
  {
    bool HasBeamLinks = TokenNode->Previous(MusicLabel(mica::Beam)) or
      TokenNode->Next(MusicLabel(mica::Beam));
    if(HasBeamLinks)
      LabelStateValue["PartState"]["Chord"][TokenNode]["StemHasBeam"] = true;
  }
}

#endif
