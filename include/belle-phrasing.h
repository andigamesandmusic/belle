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

#ifndef BELLE_ENGRAVER_PHRASING_H
#define BELLE_ENGRAVER_PHRASING_H

namespace BELLE_NAMESPACE
{
  ///Tie and slur engraving
  class Phrasing
  {
    public:

    static mica::Concept PitchOfNote(Music::ConstNode Note)
    {
      return Note ? Note->Get(mica::Value) : mica::Undefined;
    }

    static mica::Concept DirectionOfNotes(Music::ConstNode Note1,
      Music::ConstNode Note2)
    {
      return mica::Intervals::direction(PitchOfNote(Note1),
        PitchOfNote(Note2));
    }

    class TieInfo
    {
      public:

      enum Style {Regular, PartialIncoming, PartialOutgoing};

      Music::ConstNode StartNote;
      Music::ConstNode EndNote;
      Style TieStyle; PRIM_PAD(Style)
      mica::Concept ForcedTieDirection;

      TieInfo() : TieStyle(Regular) {}

      TieInfo(Music::ConstNode StartNote_, Music::ConstNode EndNote_,
        Style TieStyle_, mica::Concept ForcedTieDirection_) :
        StartNote(StartNote_), EndNote(EndNote_), TieStyle(TieStyle_),
        ForcedTieDirection(ForcedTieDirection_) {}

      bool operator < (const TieInfo& Other) const
      {
        return DirectionOfNotes(StartNote, Other.StartNote) == mica::Below;
      }

      bool operator > (const TieInfo& Other) const
      {
        return DirectionOfNotes(StartNote, Other.StartNote) == mica::Above;
      }
    };

    static void EngraveTie(const TieInfo& Tie, count TieIndex, count Ties)
    {
      Music::ConstNode TieStartIsland = IslandOfNote(Tie.StartNote);
      Music::ConstNode TieEndIsland = IslandOfNote(Tie.EndNote);

      Music::ConstNode TieStartToken = ChordOfNote(Tie.StartNote);
      Music::ConstNode TieEndToken = ChordOfNote(Tie.EndNote);

      Pointer<Stamp> StampStart = StampForIsland(TieStartIsland);

      bool StartStemUp = ChordStemIsUpInPartState(TieStartToken);
      bool EndStemUp = TieEndToken ? ChordStemIsUpInPartState(TieEndToken) :
        StartStemUp;

      number StartPosition = SpacePositionOfNote(Tie.StartNote);
      number EndPosition = SpacePositionOfNote(Tie.EndNote);

      bool TieGoesUp;
      {
        TieGoesUp = TieIndex < (Ties / 2);
        if((Ties % 2) == 1 and TieIndex == (Ties - 1) / 2)
          TieGoesUp = not StartStemUp;
        if(Ties == 1 and StartStemUp != EndStemUp)
          TieGoesUp = StartPosition >= 0.f;
        if(Tie.ForcedTieDirection == mica::Up)
          TieGoesUp = true;
        if(Tie.ForcedTieDirection == mica::Down)
          TieGoesUp = false;
      }

      Vector StartPoint, EndPoint;
      {
        if(Tie.TieStyle == TieInfo::Regular)
        {
          StartPoint.x  = StampBoundsForIsland(TieStartIsland).Right();
          StartPoint.y  = StartPosition;
          EndPoint.x    = StampBoundsForIsland(TieEndIsland).Left() +
            TypesetDistanceToIsland(TieStartIsland, TieEndIsland);
          EndPoint.y    = EndPosition;
          TieStartIsland->SetState("IslandState", "TieDirections")[
            Tie.StartNote] = TieGoesUp ? mica::Up : mica::Down;
        }
        else if(Tie.TieStyle == TieInfo::PartialIncoming)
        {
          const number PartialTieIncomingWidth = 2.5f;
          EndPoint.x   = StampBoundsForIsland(TieStartIsland).Left();
          EndPoint.y   = StartPosition;
          StartPoint.x = EndPoint.x - PartialTieIncomingWidth;
          StartPoint.y = StartPosition;
        }
        else if(Tie.TieStyle == TieInfo::PartialOutgoing)
        {
          Music::ConstNode LastBarline;
          {
            Music::ConstNode CurrentIsland = TieStartIsland;
            while(NextIslandByPart(CurrentIsland))
            {
              CurrentIsland = NextIslandByPart(CurrentIsland);
              if(IslandHasBarline(CurrentIsland))
                LastBarline = CurrentIsland;
            }
          }

          number PartialTieOutgoingWidth = 2.5f;
          if(LastBarline)
            PartialTieOutgoingWidth = Max(PartialTieOutgoingWidth,
              TypesetDistanceToIsland(TieStartIsland, LastBarline) - 0.2f);
          StartPoint.x = StampBoundsForIsland(TieStartIsland).Right();
          StartPoint.y = StartPosition;
          EndPoint.x   = PartialTieOutgoingWidth;
          EndPoint.y   = StartPosition;
        }

        Vector LeftDelta(0.1f, TieGoesUp ? 0.4f : -0.4f);
        Vector RightDelta(-0.1f, TieGoesUp ? 0.4f : -0.4f);
        StartPoint   += LeftDelta;
        EndPoint     += RightDelta;
      }

      if(StampStart)
      {
        {
          Pointer<Path> Slur;
          number Arch = (TieGoesUp ? 0.15f : -0.15f);
          Shapes::Music::AddSlur(*Slur.New(), StartPoint, EndPoint, 1.f, Arch,
            0.5f, 0.3f, 0.1f);
          StampStart->Add()->p = Slur;
          StampStart->z()->Spans = true;
          StampStart->z()->Context = OutgoingTieOfNote(Tie.StartNote);
        }
        {
          Value Grouping;
          Grouping.Add() = Tie.StartNote;
          Grouping.Add() = Tie.EndNote;
        }
      }
    }

    static void EngraveTies(Pointer<const Music> M)
    {
      MusicIterator Iterator;
      Iterator.Start(M);
      while(Iterator.NextChord())
      {
        Sortable::Array<TieInfo> Ties;
        Array<Music::ConstNode> Notes = NotesOfChord(Iterator.Chord());
        for(count i = 0; i < Notes.n(); i++)
        {
          Music::ConstNode CurrentNote = Notes[i];
          if(NoteHasOutgoingTie(CurrentNote))
          {
            Music::ConstNode TieEnd = CurrentNote->Next(MusicLabel(mica::Tie));
            Music::ConstNode TieEndIsland = IslandOfNote(TieEnd);
            if(TieEnd and TieEndIsland)
              Ties.Add() = TieInfo(CurrentNote, TieEnd, TieInfo::Regular,
                mica::Undefined);
            else
              C::Error() >> "Error: Tie end without parent token or island";
          }
          else if(not NoteHasIncomingTie(CurrentNote))
          {
            Music::ConstNode x;
            if(NoteHasPartialIncomingTie(CurrentNote))
              Ties.Add() = TieInfo(CurrentNote, x, TieInfo::PartialIncoming,
                CurrentNote->Get(mica::PartialTieIncoming));
            if(NoteHasPartialOutgoingTie(CurrentNote))
              Ties.Add() = TieInfo(CurrentNote, x, TieInfo::PartialOutgoing,
                CurrentNote->Get(mica::PartialTieOutgoing));
          }
        }
        Ties.Sort();
        for(count i = 0; i < Ties.n(); i++)
          EngraveTie(Ties[i], i, Ties.n());
      }
    }

    static void EngraveSlur(
      Music::ConstNode TieStartIsland, Music::ConstNode TieStartChord,
      Music::ConstNode TieEndIsland, Music::ConstNode TieEndChord,
      Music::ConstEdge SlurEdge)
    {
      bool StemUp = mica::Concept(TieStartIsland->Label.GetState(
        "PartState", "Chord")[TieStartChord]["StemDirection"]) == mica::Up;
      bool EndStemUp = mica::Concept(TieEndIsland->Label.GetState(
        "PartState", "Chord") [TieEndChord]["StemDirection"]) == mica::Up;

      count StartLow = TieStartIsland->Label.GetState("PartState",
        "Chord") [TieStartChord]["StaffPosition"]["Lowest"].AsCount();
      count StartHigh = TieStartIsland->Label.GetState("PartState",
        "Chord")[TieStartChord]["StaffPosition"]["Highest"].AsCount();
      count EndLow = TieEndIsland->Label.GetState("PartState",
        "Chord")[TieEndChord]["StaffPosition"]["Lowest"].AsCount();
      count EndHigh = TieEndIsland->Label.GetState("PartState",
        "Chord")[TieEndChord]["StaffPosition"]["Highest"].AsCount();
      if(StemUp) StartHigh += 6; else StartLow -= 6;
      if(EndStemUp) EndHigh += 6; else EndLow -= 6;

      bool PlaceBelow = not SlurEdge                  ? StemUp :
        SlurEdge->Get(mica::Placement) == mica::Above ? false  :
        SlurEdge->Get(mica::Placement) == mica::Below ? true   : StemUp;

      count StartStaffPosition = (PlaceBelow ? StartLow : StartHigh);
      count EndStaffPosition = (PlaceBelow ? EndLow : EndHigh);
      number StartPosition = number(StartStaffPosition) / 2.f;
      number EndPosition = number(EndStaffPosition) / 2.f;

      number StemDirection = PlaceBelow ? -1.f : 1.f;
      Vector LeftDelta(0.2f, 0.9f * StemDirection);
      Vector RightDelta(-0.2f, 0.9f * StemDirection);

      LeftDelta.x -= 0.8f;
      RightDelta.x += 0.6f;
      
      const number DistanceToGroup = 0.9f;
      if(PlaceBelow)
        StartPosition -= DistanceToGroup, EndPosition -= DistanceToGroup;
      else
        StartPosition += DistanceToGroup, EndPosition += DistanceToGroup;

      Vector StartPoint, EndPoint;
      StartPoint.y = StartPosition;
      EndPoint.y = EndPosition;
      StartPoint.x = 0.f;
      EndPoint.x = TieEndIsland->Label.GetState(
        "IslandState", "TypesetX").AsNumber() -
        TieStartIsland->Label.GetState(
        "IslandState", "TypesetX").AsNumber();
      StartPoint += LeftDelta;
      EndPoint += RightDelta;
      Path Slur;
      number Arch = (PlaceBelow ? -0.15f : 0.15f) * 1.5f;
      Shapes::Music::AddSlur(Slur, StartPoint, EndPoint, 1.f, Arch, 0.5f,
        0.3f, 0.1f);

      Pointer<Stamp> StampStart = TieStartIsland->Label.Stamp().Object();

      Pointer<Path> p = new Path;
      p->Append(Slur);
      StampStart->Add()->p = p;
      StampStart->z()->Spans = true;
      StampStart->z()->Context = TieStartChord->Next(
        MusicLabel(mica::Slur), true);
    }

    static void EngraveSlurs(Pointer<const Music> M)
    {
      if(!M) return;
      Music::ConstNode m, n;
      for(m = M->Root(); m; m = m->Next(MusicLabel(mica::Instantwise)))
      {
        for(n = m; n; n = n->Next(MusicLabel(mica::Partwise)))
        {
          Array<Music::ConstNode> Tokens = n->Children(MusicLabel(mica::Token));
          for(count c = 0; c < Tokens.n(); c++)
          {
            Music::ConstNode t = Tokens[c];
            Array<Music::ConstNode> Slurs =
              t->Children(MusicLabel(mica::Slur));
            /*Note: currently unable to get edges of children, so won't be
            able to get the correct placement info in case of multiple slurs
            emanating from the same chord.*/
            for(count i = 0; i < Slurs.n(); i++)
              EngraveSlur(n, t, Slurs[i]->Previous(MusicLabel(mica::Token)),
                Slurs[i], t->Next(MusicLabel(mica::Slur), true));
          }
        }
      }
    }
  };
}
#endif
