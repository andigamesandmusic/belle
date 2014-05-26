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

#ifndef BELLE_MODERN_CHORD_H
#define BELLE_MODERN_CHORD_H

#include "Beaming.h"

namespace BELLE_NAMESPACE { namespace modern
{
  ///Represents a chord of staff notes with a particular stem direction.
  struct Chord
  {
    ///Keeps state used for determining stem direction.
    struct StateInfo
    {
      ///Possible stem directions
      enum StemDirection
      {
        Down = -1,
        PositionBased = 0,
        Up = 1
      };
      
      //Updates the tessitura of the chord for calculating stem directions.
      void UpdateTessitura(graph::Music& g)
      {
        if(!c) return;
        Chord ThisChord;
        ThisChord.Import(g, c);
        Tessitura = ThisChord.CalculateTessitura();
      }
      
      //Sorting operators
      bool operator < (const StateInfo& Other) const
      {
        return Tessitura < Other.Tessitura;
      }
      
      bool operator > (const StateInfo& Other) const
      {
        return Tessitura > Other.Tessitura;
      }
      
      ///Tessitura of the chord used for making stem decisions
      prim::number Tessitura;

      ///Chord token node of the chord
      graph::MusicNode c;
      
      ///Current direction of the stem
      StemDirection d;
      
      ///Chord token of previous chord by continuity
      graph::MusicNode p;
      
      ///Previous direction of the stem
      StemDirection pd;
      
      ///Set if the voice strand is beginning.
      bool NewVoice;
      
      ///Default constructor
      StateInfo() : c(0), d(PositionBased), p(0), pd(PositionBased),
        NewVoice(false) {}
    };
    
    ///Array of state infos.
    typedef prim::Sortable::Array<StateInfo> State;

    ///Imports the chord data from a graph-based chord token.
    void Import(graph::Music& g, graph::MusicNode ChordToken)
    {
      graph::MusicNode c = ChordToken;
      if(c->Get(mica::Kind) != mica::Chord) return;
      
      prim::Array<graph::MusicNode> a = g.Children(c,
        graph::MusicLabel::Note());
      for(prim::count i = 0; i < a.n(); i++)
      {
        //If rest is detected, then read the rest value and stop importing.
        if(a[i]->Get(mica::Rest) == mica::Rest)
        {
          IsRest = true;
          StaffNotes.RemoveAll(); //In case notes and rests were mixed.
          StaffNotes.Add().LineSpace =
            integer(a[i]->Get(mica::StaffPosition));
          StaffNotes.z().OriginalNode = a[i];
          break;
        }
        StaffNotes.Add().LineSpace = integer(mica::map(mica::TrebleClef,
          a[i]->Get(mica::Value))); //FIX ME: need clef from staff
        StaffNotes.z().Accidental = mica::map(mica::Accidental,
          a[i]->Get(mica::Value));
        StaffNotes.z().OriginalNode = a[i];
      }
      
      Duration = c->Get(mica::NoteValue);
      OriginalNode = c;
    }
    
    ///Engraves the chord onto the stamp.
    void Engrave(Stamp& s, const House& h, const Cache& c, const Typeface& t,
      const Font& f)
    {
      //If the structure is empty, then do not add anything to the stamp.
      if(!StaffNotes.n() && !IsRest) return;
      
      CreateClusters();
      EngraveLedgerLines(s, h, c);
      Path ChordBounds;
      EngraveNoteheads(s, ChordBounds, h, c, f);
      EngraveRest(s, ChordBounds, h, t);
      EngraveDots(s, ChordBounds, h, c);
      
      if(!IsRest)
      {
        EngraveAccidentals(ChordBounds, s, h, c, t);
        EngraveFlags(s, t);
      }
    }
    
    ///Engraves the flags onto the stamp.
    void EngraveFlags(Stamp& s, const Typeface& t)
    {
      prim::count Flags = Utility::CountFlags(Duration);
      prim::planar::Vector f = FlagPosition;
      for(prim::count i = 0; i < Flags; i++)
      {
        //Add the flag to the stamp.
        s.Add().p2 = t.LookupGlyph(87);
        s.z().a = Affine::Translate(f) * Affine::Scale(4.0);
        if(StaffNotes.n() == 1)
          s.z().n = StaffNotes.a().OriginalNode;
        else
          s.z().n = OriginalNode;
        
        //Flip the stem direction for stem down (flag up).
        if(!StemUp)
          s.z().a = s.z().a * Affine::Scale(
            prim::planar::Vector(1.0, -1.0));
        
        /*Flag positions do not take into account the line-space position
        mapping. Instead, they are spaced equally by one space height.*/
        f.y += (StemUp ? -1.0 : 1.0);
      }
    }
    
    ///Engraves a rest.
    void EngraveRest(Stamp& s, Path& Bounds, const House& h, const Typeface& t)
    {
      //If the chord is not a rest, then rests do not pertain.
      if(!IsRest || StaffNotes.n() != 1) return;
      
      //Determine the symbol to use.
      prim::Ratio Base = Utility::GetUndottedValue(Duration);      
      prim::count SymbolID = 0;
      if(Base == prim::Ratio(1, 1)) SymbolID = 79;
      else if(Base == prim::Ratio(1, 2)) SymbolID = 80;
      else if(Base == prim::Ratio(1, 4)) SymbolID = 81;
      else if(Base == prim::Ratio(1, 8)) SymbolID = 82;
      else if(Base == prim::Ratio(1, 16)) SymbolID = 83;
      else if(Base == prim::Ratio(1, 32)) SymbolID = 84;
      else if(Base == prim::Ratio(1, 64)) SymbolID = 85;
      else if(Base == prim::Ratio(1, 128)) SymbolID = 86;
      
      //If the duration is not supported then abort the rest engraving.
      if(!SymbolID)      
        return;
      
      //Create the graphic.
      s.Add().p2 = t.LookupGlyph(SymbolID);
      s.z().a = Affine::Translate(prim::planar::Vector(0.0,
        Utility::GetLineSpacePosition(
        StaffNotes.a().LineSpace, StaffLines, h))) * Affine::Scale(4.0);
      s.z().n = StaffNotes.a().OriginalNode;
      
      //Update the bounding box.
      if(s.z().p2)
        Shapes::AddRectangle(Bounds,
          s.z().p2->Bounds(s.z().a));
    }
    
    ///Engraves the ledger lines onto the stamp.
    void EngraveLedgerLines(Stamp& s, const House& h, const Cache& c)
    {
      //If the chord is a rest, then ledger lines do not pertain.
      if(IsRest) return;
      
      //Get boundaries for painting ledger lines.
      prim::count LowestNote, LowestCluster, HighestNote, HighestCluster;
      GetHighestAndLowestLineSpaces(HighestNote, LowestNote);
      GetHighestAndLowestClusterLineSpaces(HighestCluster, LowestCluster);
      
      //Create an internal Make::Ledger function to prevent code duplication.
      struct Make { static void Ledger(prim::count i, bool IsCluster,
        bool StemUp, prim::count StaffLines, Stamp& s, const House& h,
        const Cache& c)
      {
        if(IsCluster)
          s.Add().p2 = c[StemUp ? Cache::LedgerLineTwoColumnStemUp :
            Cache::LedgerLineTwoColumnStemDown];
        else
          s.Add().p2 = c[StemUp ? Cache::LedgerLineOneColumnStemUp :
            Cache::LedgerLineOneColumnStemDown];
        
        s.z().a = Affine::Translate(
          prim::planar::Vector(0.0, Utility::GetLineSpacePosition(i,
          StaffLines, h)));
      }};
      
      //Create upper ledger lines.
      for(prim::count i = Utility::GetTopLine(StaffLines) + 2;
        i <= HighestNote; i += 2)
          Make::Ledger(i, i < HighestCluster, StemUp, StaffLines, s, h, c);
      
      //Create lower ledger lines.
      for(prim::count i = Utility::GetBottomLine(StaffLines) - 2;
        i >= LowestNote; i -= 2)
          Make::Ledger(i, i > LowestCluster, StemUp, StaffLines, s, h, c);
    }
    
    ///Engraves the noteheads onto the stamp and returns their bounds.
    void EngraveNoteheads(Stamp& s, Path& Bounds, const House& h,
      const Cache& c, const Font& f)
    {
      //If the chord is a rest, then noteheads do not pertain.
      if(IsRest) return;

      //Position the notes of the chord.
      for(prim::count i = 0; i < StaffNotes.n(); i++)
      {
        prim::number VerticalPosition =
          Utility::GetLineSpacePosition(StaffNotes[i].LineSpace, StaffLines, h);
        prim::number VerticalPositionRounded =
          Utility::GetLineSpacePosition(((StaffNotes[i].LineSpace % 2) ?
          StaffNotes[i].LineSpace - 1 : StaffNotes[i].LineSpace - 2),
          StaffLines, h);
        prim::number StaffNoteColumn = (prim::number)GetStaffNoteColumn(i);
        prim::planar::Vector NoteheadPosition(
          StaffNoteColumn * h.NoteheadWidthPrecise, VerticalPosition);
        prim::planar::Vector TextPosition(
          StaffNoteColumn * h.NoteheadWidthPrecise, VerticalPositionRounded);
        
        //Account for the fact that noteheads overlap in stem space.
        if(StaffNoteColumn)
          NoteheadPosition.x -= (prim::number)StaffNoteColumn * h.StemWidth;
        
        prim::planar::Rectangle b;

        if(i || Duration >= prim::Ratio(1, 1))
        {
          s.Add().p2 = c[Utility::GetNotehead(Duration)];
          s.z().a = Affine::Translate(NoteheadPosition);
          b = s.z().p2->Bounds();

          //Force the y coordinates to occupy no more than a space.
          b.Top(0.5);
          b.Bottom(-0.5);
          
          //Translate bounds to the notehead position.
          b.a += NoteheadPosition;
          b.b += NoteheadPosition;
        }
        else
        {
          prim::Ratio Base = Utility::GetUndottedValue(Duration);
          
          prim::count FlagLS = StaffNotes.z().LineSpace;
          FlagLS += (h.StemHeight +
            prim::Min((prim::count)0, Utility::CountFlags(Duration) - 1)) *
            (StemUp ? 1 : -1);
          prim::number FlagY =
            Utility::GetLineSpacePosition(FlagLS, StaffLines, h);

          Shapes::Music::AddQuarterNote(
            s.Add().p, prim::planar::Vector(0, VerticalPosition),
            1.0, true, FlagY - VerticalPosition, &FlagPosition, h.NoteheadAngle,
            h.NoteheadWidth, h.StemWidth, h.StemCapHeight,
            (Base == prim::Ratio(1, 2)));
          
          b = s.z().p.Bounds();
        }
        s.z().n = StaffNotes[i].OriginalNode;
        
        //If the annotation property is set, display the text next to the note.
        if(prim::String t = StaffNotes[i].OriginalNode->Get("Annotation"))
        {
          Painter::Draw(s.Add().p, t, f, 72.0 * 1.2, Font::Regular,
            Text::Justifications::Left, 20.0);
          prim::planar::Vector Adjust(1.0, 0.11);
          if(Utility::CountDots(Duration, h.MaxDotsToConsider) > 0)
            Adjust.x += h.RhythmicDotNoteheadDistance;
          s.z().a = Affine::Translate(TextPosition + Adjust);
          s.z().c = Colors::orangered;
        }
        
        //Add the note bounds.
        Path p;
        Shapes::AddRectangle(p, b);
        Bounds.Append(p);
      }
#if DEVELOPER_COLLISION_BOXES
      s.Add().p = Bounds;
#endif
    }
    
    ///Engraves the dots onto the stamp.
    void EngraveDots(Stamp& s, const Path& Bounds, const House& h,
      const Cache& c)
    {
      //If there are no notes in the chord, then there are no dots.
      if(!StaffNotes.n()) return;
      
      prim::count NumberOfDots =
        Utility::CountDots(Duration, h.MaxDotsToConsider);
      prim::number StartX =
        Bounds.Bounds().Right() + h.RhythmicDotNoteheadDistance;
      prim::Array<prim::number> StartY;
      
      /*Initialize previous dot to anything so long as it is different than the
      first line-space in the staff note list.*/
      prim::count PreviousDot = StaffNotes[0].LineSpace - 1;
      
      //Create a list of dot positions.
      for(prim::count i = 0; i < StaffNotes.n(); i++)
      {
        prim::count LS = Utility::RoundUpToNextSpace(
          StaffNotes[i].LineSpace, StaffLines);
        if(LS == PreviousDot) continue;
        StartY.Add() = Utility::GetLineSpacePosition(LS, StaffLines, h);
        PreviousDot = LS;
      }
      
      for(prim::count i = 0; i < StartY.n(); i++)
      {
        for(prim::count j = 0; j < NumberOfDots; j++)
        {
          s.Add().p2 = c[Cache::RhythmicDot];
          s.z().a = Affine::Translate(prim::planar::Vector(
            StartX + (prim::number)j * h.RhythmicDotSpacing, StartY[i]));
          s.z().n = OriginalNode;
        }
      }
    }
    
    ///Engraves the accidentals onto the stamp.
    void EngraveAccidentals(Path& NoteBounds, Stamp& s, const House& h,
      const Cache& c, const Typeface& t)
    {
      //If the chord is a rest, then accidentals do not pertain.
      if(IsRest) return;

      //Determine the priority in which to place accidentals.
      prim::Array<prim::count> OrderOfAccidentals;
      prim::Array<prim::count> TestOrder;
      prim::Array<prim::planar::Vector> BestPlacement;
      prim::Array<prim::planar::Vector> AccidentalPlacement;
      prim::Array<const Path*> AccidentalPaths;
      
      OrderOfAccidentals.n(StaffNotes.n());
      AccidentalPlacement.n(StaffNotes.n());
      AccidentalPaths.n(StaffNotes.n());
      
      //Lookup the accidentals to use.
      for(prim::count i = 0; i < AccidentalPaths.n(); i++)
      {
        prim::count a = mica::integer(mica::index(mica::Accidentals,
          StaffNotes[i].Accidental));
        prim::count b = mica::integer(mica::index(mica::Accidentals,
          mica::Natural));
        if(a >= 0)
          AccidentalPaths[i] = c[Cache::AccidentalNatural + (a - b)];
        else
          AccidentalPaths[i] = 0;
      }
      
      //Determine an ordering in which to place the accidentals.
      for(prim::count i = 0; i < StaffNotes.n(); i++)
      {
        //Alternate top and bottom going inward.
        if(StemUp)
        {
          if(i % 2 == 0)
            OrderOfAccidentals[i] = StaffNotes.n() - 1 - i / 2;
          else
            OrderOfAccidentals[i] = (i - 1) / 2;
        }
        else
        {
          if(i % 2 == 0)
            OrderOfAccidentals[i] = i / 2;
          else
            OrderOfAccidentals[i] = StaffNotes.n() - 1 - (i - 1) / 2;
        }
      }
      
      prim::number LeastWidth = 100.0;
      for(prim::count Mutation = 0; Mutation < 1; Mutation++)
      {
        TestOrder = OrderOfAccidentals;
        prim::Random r(123);
        for(prim::count i = 0; i < Mutation; i++)
        {
          prim::count j = r.Between((prim::count)0, TestOrder.n());
          prim::count k = r.Between((prim::count)0, TestOrder.n());
          prim::Swap(TestOrder[j], TestOrder[k]);
        }
        
        //Do collision detection to figure out best placement.
        Path CollisionBounds = NoteBounds;
        for(prim::count i = 0; i < TestOrder.n(); i++)
        {
          //Lookup the accidental to use and add it to the stamp.
          prim::count n = TestOrder[i];
          Path CollisionAccidental;
          
          //Skip accidental if it is not visible.
          if(!AccidentalPaths[n]) continue;

          //Set collision detection quality.
          const prim::count CollisionQuality = 2;
          
          if(CollisionQuality == 0) //Rectangle approximation (low)
            Shapes::AddRectangle(CollisionAccidental,
              AccidentalPaths[n]->Bounds());
          else if(CollisionQuality == 1) //Convex hull approximation (medium)
            CollisionAccidental.AppendConvex(*AccidentalPaths[n]);
          else if(CollisionQuality == 2) //Full path approximation (high)
            CollisionAccidental = *AccidentalPaths[n];
          
          //Calculate the vertical position of the accidental.
          prim::number VerticalPosition =
            Utility::GetLineSpacePosition(
            StaffNotes[n].LineSpace, StaffLines, h);
          
          //Do collision detection to place accidentals optimally.        
          prim::number AngleOfApproach = prim::Pi;
          prim::planar::Vector ApproachAnchor(0.0, VerticalPosition);
          prim::number Distance =
            Optics::CalculateClosestNonCollidingDistanceAtAngle(CollisionBounds,
            CollisionAccidental, AngleOfApproach, ApproachAnchor, 0, 1.0);
          //Distance += h.AccidentalExtraSpacing;
    
          //Position the accidental.
          prim::planar::Vector Placement(-Distance, VerticalPosition);
          AccidentalPlacement[n] = Placement;
          
          //Append the accidental to the note bounds.
          CollisionBounds.Append(
            CollisionAccidental.Outline(), Affine::Translate(Placement));
        }
        
        prim::number w = CollisionBounds.Bounds().Width();
        if(w + 0.1 < LeastWidth)
        {
          LeastWidth = w;
          BestPlacement = AccidentalPlacement;
        }
      }
      
      //Add the accidentals to the stamp and place them.
      for(prim::count i = 0; i < AccidentalPlacement.n(); i++)
      {
        s.Add().p2 = AccidentalPaths[i];
        s.z().a = Affine::Translate(BestPlacement[i] +
          prim::planar::Vector(-0.2, 0.0));
        s.z().n = StaffNotes[i].OriginalNode;
      }
    }

    /**Creates clusters from the staff note list. This method will sort the 
    staff note list, remove any duplicates, and create a list of clusters. Each
    cluster contains a staff note list containing a list of sorted staff notes
    (in the direction of the stem) which are adjacent by unison or second.*/
    void CreateClusters()
    {
      //Remove any previously cached clusters.
      Clusters.Clear();
      
      //Do not continue if there are no staff notes.
      if(!StaffNotes.n())
        return;
      
      /*QuickSort with element copying is best since StaffNote is a small plain
      old data class. The notes are sorted in the direction of the stem.*/
      StaffNotes.QuickSort(StemUp);
      
      //Chords do not have duplicate notes [Rule], so they must be removed.
      StaffNotes.RemoveDuplicateNotes();
      
      //The first staff note will form the beginning of the first cluster.
      Clusters.Add().Add() = StaffNotes.a();
      
      for(prim::count i = 1; i < StaffNotes.n(); i++)
      {
        if(StaffNotes[i].IsClusteredWith(StaffNotes[i - 1]))
          Clusters.z().Add() = StaffNotes[i];
        else
          Clusters.Add().Add() = StaffNotes[i];
      }
    }
    
    ///Default constructor places stems up.
    Chord() : StemUp(true), Beam2(0), BeamStemIndex(0), StemLength(0),
      Duration(1, 4), StaffLines(5), IsRest(false) {}
    
    /**Calculates the average value of the bottom and top note's line-spaces.
    Does not consider the intervening notes as usually only the extremes have
    an affect, and not the center of mass.*/
    prim::number CalculateTessitura()
    {
      //No average exists if chord is empty.
      if(!StaffNotes.n())
        return 0;
      
      //Initialize the highest and lowest line-spaces
      prim::count Highest = StaffNotes.a().LineSpace;
      prim::count Lowest = Highest;
      
      //Find the highest and lowest line-spaces.
      for(prim::count i = 1; i < StaffNotes.n(); i++)
      {
        prim::count CurrentLineSpace = StaffNotes[i].LineSpace;
        Highest = prim::Max(Highest, CurrentLineSpace);
        Lowest = prim::Max(Lowest, CurrentLineSpace);
      }
      
      //Return the average line-space value between the extremes.
      return (prim::number)(Highest + Lowest) / 2.0;
    }

    ///Gets the highest line/space used in this chord.
    void GetHighestAndLowestLineSpaces(prim::count& Highest,
      prim::count& Lowest) const
    {
      if(!StaffNotes.n())
      {
        Highest = Lowest = 0;
        return;
      }
      
      Highest = Lowest = StaffNotes[0].LineSpace;
      for(prim::count i = 1; i < StaffNotes.n(); i++)
      {
        prim::count LineSpace = StaffNotes[i].LineSpace;
        Highest = prim::Max(Highest, LineSpace);
        Lowest = prim::Min(Lowest, LineSpace);
      }
    }
    
    ///Gets the highest clustered line/space used.
    void GetHighestAndLowestClusterLineSpaces(prim::count& Highest,
      prim::count& Lowest) const
    {
      Highest = Lowest = 0;
      
      if(!Clusters.n())
        return;

      bool FirstCluster = true;
      for(prim::count i = 0; i < Clusters.n(); i++)
      {
        if(Clusters[i].n() <= 1)
          continue;
        for(prim::count j = 0; j < Clusters[i].n(); j++)
        {
          if(FirstCluster)
          {
            Highest = Lowest = Clusters[i][j].LineSpace;
            FirstCluster = false;
          }
          else
          {
            prim::count LineSpace = Clusters[i][j].LineSpace;
            Highest = prim::Max(Highest, LineSpace);
            Lowest = prim::Min(Lowest, LineSpace);
          }
        }
      }
    }
    
    ///Sets stem direction in opposite direction as the highest or lowest note.
    void DetermineStemDirectionByPosition(bool TieBreakIsStemUp = true)
    {
      if(!StaffNotes.n())
        return;
        
      prim::count Highest = 0, Lowest = 0;
      GetHighestAndLowestLineSpaces(Highest, Lowest);
      
      if(Highest < 0)
        StemUp = true;
      else if(Lowest > 0)
        StemUp = false;
      else if(Highest == prim::Abs((prim::integer)Lowest))
        StemUp = TieBreakIsStemUp;
      else if(Highest > prim::Abs((prim::integer)Lowest))
        StemUp = false;
      else
        StemUp = true;
    }
    
    /**Returns a staff notes column, left -1, center 0, or right 1. This method
    should not be called until CreateClusters() is called.*/
    prim::count GetStaffNoteColumn(prim::count StaffNoteIndex) const
    {
      //Return zero if index is out of bounds.
      if(StaffNoteIndex < 0 || StaffNoteIndex >= StaffNotes.n())
        return 0;

      //Recalculate the note index relative to the cluster it is in.
      for(prim::count i = 0; i < Clusters.n(); i++)
      {
        prim::count ClusterStaffNoteCount = Clusters[i].n();
        if(StaffNoteIndex >= ClusterStaffNoteCount)
          StaffNoteIndex -= ClusterStaffNoteCount;
        else
          break;
      }
      
      //Alternate primary column and secondary column.
      if(prim::Mod(StaffNoteIndex, (prim::count)2) == 0)
        return 0;
      else if(StemUp)
        return 1;
      else
        return -1;
    }
    
    public:
    
    ///The direction of the chord's stem.
    bool StemUp;
    
    ///Pointer to the beam this chord uses. If zero, there is no beam.
    Beam* Beam2;
    
    ///The stem index of the beam, if it exists.
    prim::count BeamStemIndex;
    
    ///The length of the stem beginning on the line-space above-below the join.
    prim::number StemLength;
    
    ///The primary rhythmic type (1/8 = eighth for example)
    prim::Ratio Duration;
    
    ///Number of staff lines in use.
    prim::count StaffLines;
    
    ///Whether the chord is a rest.
    bool IsRest;
    
    ///Position of the flag received from the note-stem construction.
    prim::planar::Vector FlagPosition;
    
    /**Describes vertical position of a note by its line-space and accidental.
    
    Staff notes can also be sorted. The line-space is the primary sorting
    column, and the accidental is the secondary sorting column.*/
    struct StaffNote
    {
      /**The line-space of the staff note. For example, 0 is the middle line (or
      space if the staff has an even number of staff lines.*/
      prim::count LineSpace;
      
      /**The accidental pertaining to the staff note. This property helps
      determine the order of staff notes in the case of imperfect unisons.*/
      mica::Concept Accidental;
      
      ///Pointer back to the original node.
      graph::MusicNode OriginalNode;
      
      ///Default constructor
      StaffNote() : LineSpace(0), Accidental(mica::Undefined) {}
      
      /**Returns whether this staff note is clustered with another. This occurs
      when the note is either at a distance of a unison or a second.*/
      bool IsClusteredWith(const StaffNote& Other)
      {
        return prim::Abs((prim::integer)(LineSpace - Other.LineSpace)) < 2;
      }
      
      //-----------------//
      //Sorting operators//
      //-----------------//
      
      /*Two-column comparison with the line-space being the primary sorting
      column and the accidental being the secondary sorting column.*/
      
      bool operator < (const StaffNote& Other) const
      {
        return LineSpace < Other.LineSpace || (LineSpace == Other.LineSpace &&
          integer(index(mica::Accidentals, Other.Accidental, Accidental)) < 0);
      }
      
      bool operator > (const StaffNote& Other) const
      {
        return LineSpace > Other.LineSpace || (LineSpace == Other.LineSpace &&
          integer(index(mica::Accidentals, Other.Accidental, Accidental)) > 0);
      }
  
      bool operator == (const StaffNote& Other) const
      {
        return LineSpace == Other.LineSpace && Accidental == Other.Accidental;
      }
      
      bool operator <= (const StaffNote& Other) const
      {
        return *this < Other || *this == Other;
      }
      
      bool operator >= (const StaffNote& Other) const
      {
        return *this > Other || *this == Other;
      }
    };
    
    ///List of staff notes
    struct StaffNoteList : public prim::Sortable::List<StaffNote>
    {
      /**Removes duplicate adjacent staff notes. For this to be effective, the
      staff note list should be sorted beforehand.*/
      void RemoveDuplicateNotes()
      {
        for(prim::count i = n() - 1; i >= 1; i--)
          if(ith(i) == ith(i - 1))
            Remove(i);
      }
    };
    
    ///Cached group of clusters (consecutive seconds).
    prim::Array<StaffNoteList> Clusters;
    
    /**List of staff notes. After CreateClusters() is called it will be sorted
    in the direction of the stem and duplicate notes will be removed.*/
    StaffNoteList StaffNotes;
    
    ///Pointer back to the original chord token.
    graph::MusicNode OriginalNode;
  };
}}
#endif
