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

///@name Accidental State
///@{

///Increments the measures ago history of the alterations.
void AccidentalStateIncreaseMeasuresAgo(Value& AccidentalStateValue);

///Calculates accidental state for the part.
void AccidentalStateAssumeAndAccumulateAccidentalStateForPart(
  Music::ConstNode TokenNode, Value& LabelStateValue);

///Determines whether the given accidental should be emitted based on state.
bool AccidentalStateShouldAccidentalBeEmitted(const Value& PartStateValue,
  mica::Concept Accidental, mica::Concept DiatonicPitch,
  count StaffPosition);

///@}

#ifdef BELLE_IMPLEMENTATION

void AccidentalStateIncreaseMeasuresAgo(Value& AccidentalStateValue)
{
  Value& AlteredAccidentals = AccidentalStateValue["Altered"];

  //Increment the measures ago state for the alterations.
  Array<Value> AlteredAccidentalsKeys;
  AlteredAccidentals.EnumerateKeys(AlteredAccidentalsKeys);
  for(count i = 0, n = AlteredAccidentalsKeys.n(); i < n; i++)
  {
    Value k = AlteredAccidentalsKeys[i];
    AlteredAccidentals[k]["MeasuresAgo"] =
      integer(AlteredAccidentals[k]["MeasuresAgo"]) + 1;
  }

  //Prune accidental alterations that no longer have an effect on state.
  for(count i = AlteredAccidentals.n() - 1; i >= 0; i--)
  {
    Value k = AlteredAccidentalsKeys[i];
    if(integer(AlteredAccidentals[k]["MeasuresAgo"]) >= 2)
      AlteredAccidentals[k].Clear();
  }
}

void AccidentalStateAssumeAndAccumulateAccidentalStateForPart(
  Music::ConstNode TokenNode, Value& LabelStateValue)
{
  //Retrieve the chord notes.
  Array<Music::ConstNode> Notes =
    TokenNode->Children(MusicLabel(mica::Note));

  /*For each note determine whether an accidental needs to be emitted.
  This is based on the previous altered accidentals and the key signature.*/
  for(count i = 0; i < Notes.n(); i++)
  {
    //Skip rests.
    if(Notes[i]->Get(mica::Rest) == mica::Rest)
      continue;

    //Get the pitch.
    mica::Concept Pitch = Notes[i]->Get(mica::Value);

    /*Get any override accidental if it exists. If an accidental is present
    that is used for the pitch's accidental. If the key is reflexively set
    to the mica::Accidental value, then it indicates that no accidental
    should be shown, even if there is one given by the original pitch.*/
    mica::Concept OverrideAccidental = Notes[i]->Get(mica::Accidental);
    bool OverrideInEffect = OverrideAccidental != mica::Undefined;
    bool OverrideVisible = OverrideAccidental != mica::Accidental;

    //Get the accidental applying the override if necessary.
    mica::Concept Accidental = (OverrideInEffect ?
      mica::UUIDv4(OverrideAccidental) : mica::map(Pitch, mica::Accidental));

    //Get the pitch without the accidental.
    mica::Concept DiatonicPitch = mica::map(Pitch, mica::DiatonicPitch);

    //Get the staff position.
    count StaffPosition = count(mica::numerator(mica::map(DiatonicPitch,
      mica::Concept(LabelStateValue["PartState"]["Clef"]["Active"]))));

    //Get a copy of the accidentals-to-emit state.
    Value AccidentalsToEmit =
      LabelStateValue["PartState"]["Chord"]["AccidentalsToEmit"];

    //Ensure that accidental-emit list is a tree.
    if(AccidentalsToEmit.IsNil())
      AccidentalsToEmit.NewTree();

    if(AccidentalsToEmit[StaffPosition].IsNil())
      AccidentalsToEmit[StaffPosition].NewTree();

    //Determine whether pitch should have its accidental emitted.
    bool Emit = AccidentalStateShouldAccidentalBeEmitted(
      LabelStateValue["PartState"], Accidental, DiatonicPitch, StaffPosition);

    //If the override is in effect, emit if the override is visible.
    if(OverrideInEffect)
      Emit = OverrideVisible;

    if(Emit && AccidentalsToEmit[StaffPosition][Accidental].IsNil())
    {
      //Have the chord engraver emit the accidental.
      AccidentalsToEmit[StaffPosition][Accidental] = true;

      //Get references to the altered accidental data.
      Value& Altered =
        LabelStateValue["PartState"]["Accidentals"]["Altered"];
      Value& AlteredIndex =
        LabelStateValue["PartState"]["Accidentals"]["AlteredIndex"];

      //Set defaults for altered accidentals data.
      if(Altered.IsNil())
        Altered.NewTree();
      if(AlteredIndex.IsNil())
        AlteredIndex = 0;

      //Set the altered accidental.
      Value& AlteredAccidental = Altered[AlteredIndex];
      AlteredAccidental["Accidental"] = Accidental;
      AlteredAccidental["DiatonicPitch"] = DiatonicPitch;
      AlteredAccidental["StaffPosition"] = StaffPosition;
      AlteredAccidental["MeasuresAgo"] = 0;

      //Increase the index.
      AlteredIndex = integer(AlteredIndex) + 1;
    }

    /*If the note was tied, silence the accidental but allow the state to
    persist for ties across measures.*/
    if(Notes[i]->Previous(MusicLabel(mica::Tie)))
      AccidentalsToEmit[StaffPosition][Accidental].Clear();

    //Flush the accidentals-to-emit state.
    LabelStateValue["PartState"]["Chord"]["AccidentalsToEmit"] =
      AccidentalsToEmit;
  }
}

bool AccidentalStateShouldAccidentalBeEmitted(const Value& PartStateValue,
  mica::Concept Accidental, mica::Concept DiatonicPitch,
  count StaffPosition)
{
  /*The rules for determining whether an accidental should be emitted are
  highly complex. The literal rules are that an accidental is implied by a
  key signature or a previously altered note of the same diatonic pitch in
  the same measure. Beyond the literal rules, determining whether the
  accidental should be emitted is equivalent to asking "is the accidental
  visually ambiguous in this context." Since there are many visual
  situations in which accidentals can appear, this problem is fairly
  tricky to solve deterministically.*/
  Value Altered = PartStateValue["Accidentals"]["Altered"];

  // 1) Is most recent occurrence of letter with a different accidental?
  //    If so - emit (return true immediately).
  for(count i = Altered.n() - 1; i >= 0; i--)
  {
    Value a = Altered[i];
    mica::Concept Letter = map(DiatonicPitch, mica::Letter);
    mica::Concept a_Letter = map(
      mica::Concept(a["DiatonicPitch"]), mica::Letter);
    if(a_Letter == Letter &&
      a["StaffPosition"].AsCount() == StaffPosition &&
      mica::Concept(a["Accidental"]) != Accidental)
        return true;
    else if(a_Letter == Letter &&
      mica::Concept(a["Accidental"]) == Accidental)
        break;
  }

  // 2) Is there a diatonic pitch in same measure with an alteration?
  //    If so - do not emit (return false immediately).
  for(count i = Altered.n() - 1; i >= 0; i--)
  {
    Value a = Altered[i];
    if(integer(a["MeasuresAgo"]) > 0)
      break;

    if(mica::Concept(a["Accidental"]) == Accidental &&
      mica::Concept(a["DiatonicPitch"]) == DiatonicPitch &&
      a["StaffPosition"].AsCount() == StaffPosition)
        return false;
  }

  // 3) Fallback to key signature. Does accidental match that of key
  //    signature? If so - do not emit, if not - emit.
  mica::Concept AccidentalFromKeySignature = mica::Natural;
  {
    mica::Concept ActiveKeySignature =
      PartStateValue["KeySignature"]["Active"];
    mica::Concept PitchLetter = mica::map(DiatonicPitch, mica::Letter);
    for(count i = 0, n = count(mica::length(ActiveKeySignature)); i < n; i++)
    {
      mica::Concept KeySignatureNote = mica::item(ActiveKeySignature, i);
      if(PitchLetter == mica::map(KeySignatureNote, mica::Letter))
      {
        AccidentalFromKeySignature =
          mica::map(KeySignatureNote, mica::Accidental);
        break;
      }
    }
  }

  return Accidental != AccidentalFromKeySignature;
}

#endif
