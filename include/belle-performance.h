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

///@name Performance
///@{

/**Generates a note list from the system. If only certain notes have been
selected, then only those notes will be returned. This method assumes the
system has already been engraved.*/
Value GenerateNoteListFromSystem(Pointer<const Music> MusicSystem,
  number QuartersPerMinute);

///Resets the colors of all the islands back to a specific default color.
void ResetIslandColors(Pointer<const Music> MusicSystem,
  Color DefaultColor);

///Sets the color of a specific island.
void SetColorOfIsland(Pointer<const Music> MusicSystem,
  count Part, count Instant, Color NewColor);

///Sets the color of a specific island.
void SetColorOfStaff(Pointer<const Music> MusicSystem, count Part,
  Color NewColor);
///@}

#ifdef BELLE_IMPLEMENTATION

Value GenerateNoteListFromSystem(Pointer<const Music> MusicSystem,
  number QuartersPerMinute)
{
  Value NoteList;
  NoteList.NewArray();

  if(not MusicSystem or not MusicSystem->Root())
    return NoteList;
  Pointer<Geometry> G = System::Get(MusicSystem->Root().Const())["Geometry"].
    NewObjectIfEmpty<class Geometry>();
  if(not G or not G->GetNumberOfParts() or not G->GetNumberOfInstants())
    return NoteList;
  if(not (QuartersPerMinute >= 10.f and QuartersPerMinute <= 1000.f))
    QuartersPerMinute = 120.f;
  List<Array<Music::ConstNode> > NodeMatrix;
  Matrix<Ratio> RhythmMatrix;
  Rhythm::CreateRhythmOrderedRegion(G, NodeMatrix);
  RhythmMatrix = Rhythm::GetUnpackedRhythmicOnsets(NodeMatrix);
  RhythmMatrix = Rhythm::ForceAlignRhythmMatrix(RhythmMatrix);
  Array<Ratio> MomentDurations =
    Rhythm::GetMomentDurations(RhythmMatrix, NodeMatrix);
  count Moments = RhythmMatrix.n();
  count Parts = RhythmMatrix.m();
  number WholeNotesPerSecond = QuartersPerMinute / 4.f / 60.f;

  Value NoteCountByKey;
  for(count i = 0; i <= 127; i++)
    NoteCountByKey[i] = 0;
  for(count Moment = 0; Moment < Moments; Moment++)
  {
    for(count Part = 0; Part < Parts; Part++)
    {
      Music::ConstNode Island = NodeMatrix[Moment][Part];
      Ratio Onset = RhythmMatrix(Part, Moment);
      if(not Island or Onset.IsEmpty())
        continue;

      Music::ConstNode PreviousIsland =
        Island->Previous(MusicLabel(mica::Partwise));
      bool IsFirstBeat = not IslandHasChords(PreviousIsland);
      Array<Music::ConstNode> Chords = ChordsOfIsland(Island);
      for(count i = 0; i < Chords.n(); i++)
      {
        bool IsBeginningBeamGroup = IsChordBeginningOfBeamGroup(Chords[i]);
        Array<Music::ConstNode> Notes = NotesOfChord(Chords[i]);
        for(count j = 0; j < Notes.n(); j++)
        {
          Music::ConstNode NoteNode = Notes[j];
          Ratio Duration = TiedDuration(NoteNode);
          mica::Concept Pitch = ActualPitchOfNote(NoteNode);
          mica::Concept NoteNumber = mica::map(Pitch, mica::MIDIKeyNumber);
          if(undefined(NoteNumber) or Duration.IsEmpty() or Duration <= 0 or
            Onset.IsEmpty() or Onset < 0)
              continue;

          Value Note;
          count MIDINoteNumber = count(mica::numerator(NoteNumber));
          Note["Key"] = MIDINoteNumber;
          Note["Pitch"] = String(Pitch);
          Note["RhythmicDuration"] = Duration;
          Note["NotatedDuration"] = Duration.To<number>() / WholeNotesPerSecond;
          {
            count NoteCount = NoteCountByKey[MIDINoteNumber].AsCount();
            Note["Channel"] = (NoteCount % 4) + 1;
            NoteCountByKey[MIDINoteNumber] = NoteCount + 1;
          }
          Note["BeamStart"] = IsBeginningBeamGroup;
          Note["Duration"] = Note["NotatedDuration"];
          Note["Velocity"] = 0.5f;
          Note["WholeNotesPerSecond"] = WholeNotesPerSecond;
          Note["IsFirstBeat"] = IsFirstBeat;

          //Assign island stamp information.
          if(Pointer<Stamp> IslandStamp = Island->Label.Stamp().Object())
          {
            Box PaintedBounds = IslandStamp->GetPaintedBounds();
            Value IslandBounds;
            IslandBounds["x"] = PaintedBounds.Left();
            IslandBounds["y"] = PaintedBounds.Bottom();
            IslandBounds["Width"] = PaintedBounds.Width();
            IslandBounds["Height"] = PaintedBounds.Height();
            Note["IslandBounds"] = IslandBounds;
            Box NoteBox;
            for(count k = 0; k < IslandStamp->n(); k++)
              if(IslandStamp->ith(k)->Context == NoteNode)
                NoteBox = IslandStamp->ith(k)->GetPaintedBounds(),
                  k = IslandStamp->n();
            if(not NoteBox.IsEmpty())
            {
              Value NoteBounds;
              NoteBounds["x"] = NoteBox.Left();
              NoteBounds["y"] = NoteBox.Bottom();
              NoteBounds["Width"] = NoteBox.Width();
              NoteBounds["Height"] = NoteBox.Height();
              Note["NoteBounds"] = NoteBounds;
            }
          }

          Note["Start"] = Onset.To<number>() / WholeNotesPerSecond;
          Note["Moment"] = Moment;
          Note["MomentDuration"] =
            MomentDurations[Moment].To<number>() / WholeNotesPerSecond;
          Note["Part"] = PartIDOfIsland(Island);
          Note["Instant"] = InstantIDOfIsland(Island);
          if(Island->Get("OriginalPartID") and Island->Get("OriginalInstantID"))
          {
            Note["OriginalPart"] =
              count(Island->Get("OriginalPartID").ToNumber());
            Note["OriginalInstant"] =
              count(Island->Get("OriginalInstantID").ToNumber());
          }

          NoteList.Add() = Note;
        }
      }
    }
  }

  return NoteList;
}

void ResetIslandColors(Pointer<const Music> MusicSystem,
  Color DefaultColor)
{
  if(MusicSystem and MusicSystem->Root())
  {
    Value NewColor;
    NewColor["R"] = DefaultColor.R;
    NewColor["G"] = DefaultColor.G;
    NewColor["B"] = DefaultColor.B;
    NewColor["A"] = DefaultColor.A;
    MusicSystem->Root()->SetState("System", "DefaultColor") = NewColor;
    Array<Music::ConstNode> Islands = System::GetIslands(MusicSystem);
    for(count i = 0; i < Islands.n(); i++)
      if(Pointer<Stamp> s = StampForIsland(Islands[i]))
        s->SetColorOfGraphics(DefaultColor, true);
  }
}

void SetColorOfIsland(Pointer<const Music> MusicSystem,
  count Part, count Instant, Color NewColor)
{
  if(not MusicSystem or not MusicSystem->Root()) return;
  Pointer<Geometry> G = System::Get(MusicSystem->Root().Const())["Geometry"].
    NewObjectIfEmpty<class Geometry>();
  if(not G or not G->GetNumberOfParts() or not G->GetNumberOfInstants())
    return;
  Music::ConstNode Island = G->LookupIsland(Part, Instant);
  if(Island)
    if(Pointer<Stamp> s = StampForIsland(Island))
      s->SetColorOfGraphics(NewColor, false);
}

void SetColorOfStaff(Pointer<const Music> MusicSystem, count Part,
  Color NewColor)
{
  if(not MusicSystem or not MusicSystem->Root()) return;
  Pointer<Geometry> G = System::Get(MusicSystem->Root().Const())["Geometry"].
    NewObjectIfEmpty<class Geometry>();
  if(not G or not G->GetNumberOfParts() or not G->GetNumberOfInstants())
    return;
  Music::ConstNode Island = G->LookupIsland(Part, 0);
  while(Island)
  {
    if(not IslandHasBarline(Island))
      if(Pointer<Stamp> s = StampForIsland(Island))
        s->SetColorOfGraphics(NewColor, true);
    Island = Island->Next(MusicLabel(mica::Partwise));
  }
}
#endif
