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

#ifndef BELLE_MODERN_PIECE_H
#define BELLE_MODERN_PIECE_H

#include "Directory.h"
#include "Island.h"
#include "Selection.h"
#include "System.h"

namespace BELLE_NAMESPACE { namespace modern
{
  ///Stores a piece of music existing on the same timeline.
  struct Piece : public ClickThroughManager
  {
    ///Stores the music graph.
    graph::Music* Music;
    
    //Objects for typesetting.
    const House* h;
    const Cache* c;
    const Typeface* t;
    const Font* f;
    
    //Calculated...
    graph::Geometry GraphGeometry; 
        
    ///Default constructor.
    Piece() : Music(0), h(0), c(0), t(0), f(0) {}
    
    ///Constructor to initialize typesetting objects.
    Piece(graph::Music* Music, const House& h, const Cache& c,
      const Typeface& t, const Font& f) : Music(Music), h(&h), c(&c), t(&t),
      f(&f) {}
    
    ~Piece() {}
    
    ///Returns whether the class is initialized.
    bool Initialized() {return Music && h && c && t && f;}
    
    ///Initialize the typesetting objects.
    void Initialize(graph::Music* Music, const House& h, const Cache& c,
      const Typeface& t, const Font& f)
    {
      Piece::Music = Music;
      Piece::h = &h;
      Piece::c = &c;
      Piece::t = &t;
      Piece::f = &f;
    }
    
    ///Typesets only the islands needing to be typeset.
    void TypesetRemaining()
    {
      if(!Initialized())
      {
        prim::c >> "Typesetting objects for piece are uninitialized.";
        return;
      }
            
      graph::MusicNode m = Music->Root();
      while(m)
      {
        graph::MusicNode n = m;
        State EngraverState;
        Directory d(EngraverState, *Music, *h, *c, *t, *f);
        IslandEngraver Engraver(d);
        while(n)
        {
          /*Clefs must always retypeset due to a current limitation having to
          do with the small form of the clef.*/
          graph::MusicNode ct = Music->Next(n, graph::MusicLabel::Token());
          if(ct)
            n->Label.Typesetting = new Stamp(n);

          if(prim::Pointer<Stamp> s = n->Label.Typesetting)
          {
            if(s->NeedsTypesetting)
            {
              Engraver.Engrave(*Music, n, *s);
              d.s.AdvanceAccidentalState();
              s->NeedsTypesetting = false;
            }
          }
          else
            prim::c >> "Warning: Stamp not created for MusicNode.";
          
          n = Music->Next(n, graph::MusicLabel::Partwise());
        }
        m = Music->Next(m, graph::MusicLabel::Instantwise());
      }
    }
    
    ///Clears typesetting data for all islands.
    void ClearTypesetting()
    { 
      graph::MusicNode m = Music->Root();
      while(m)
      {
        graph::MusicNode n = m;
        while(n)
        {
          //Create a new stamp if it is not there, or clear an existing one.
          if(!n->Label.Typesetting)
            n->Label.Typesetting = new Stamp(n);
          else if(prim::Pointer<Stamp> s = n->Label.Typesetting)
            s->Clear(n);
          else
            prim::c >> "Error: Non-Stamp class detected while clearing.";
          
          n = Music->Next(n, graph::MusicLabel::Partwise());
        }
        m = Music->Next(m, graph::MusicLabel::Instantwise());
      }
    }
    
    ///Clears typesetting data for all islands.
    void InitializeTypesetting()
    { 
      graph::MusicNode m = Music->Root();
      while(m)
      {
        graph::MusicNode n = m;
        while(n)
        {
          //Create a new stamp if it is not there, or clear an existing one.
          if(!n->Label.Typesetting)
            n->Label.Typesetting = new Stamp(n);
          else if(!prim::Pointer<Stamp>(n->Label.Typesetting))
            prim::c >> "Error: Non-Stamp class detected while clearing.";
          
          n = Music->Next(n, graph::MusicLabel::Partwise());
        }
        m = Music->Next(m, graph::MusicLabel::Instantwise());
      }
    }
    
    ///Typesets the islands.
    void Typeset(bool ClearAll = false)
    {
      //Clear or initialize the typesetting.
      if(ClearAll)
        ClearTypesetting();
      else
        InitializeTypesetting();
      
      //Typeset the remaining stamps.
      TypesetRemaining();
      
      //Parse the music graph geometry.
      GraphGeometry.Parse(*Music);
      
      //Set instant properties.
      graph::Instant::SetDefaultProperties(*Music);
    }

    ///Retypesets all the islands.
    void ClearAndTypeset() {Typeset(true);}
    
    ///Displays the information gathered from the graph parse.
    void DisplayGeometryInfo()
    {
      //Display the extents.
      prim::c >> "" >> "Part Count: " << GraphGeometry.GetNumberOfParts();
      prim::c >> "" >> "Part Extents:";
      for(prim::count i = 0; i < GraphGeometry.GetNumberOfParts(); i++)
        prim::c >> "Part " << i << ": " << GraphGeometry.GetPartRange(i);
      
      prim::c >> "" >> "Instant Count: " << GraphGeometry.GetNumberOfInstants();
      prim::c >> "" >> "Instant Part Membership:";
      for(prim::count i = 0; i < GraphGeometry.GetNumberOfInstants(); i++)
      {
        prim::List<prim::count> l;
        GraphGeometry.GetPartListForInstant(i, l);
        prim::c >> "Instant " << i << ": " << l <<
          (GraphGeometry.IsInstantComplete(i) ? " (complete) " : " ");
      }
    }
    
    ///Displays the instant properties.
    void DisplayInstantProperties()
    {
      graph::MusicNode n = Music->Root();
      prim::count i = 0;
      while(n)
      {
        graph::MusicNode m = Music->Next(n, graph::MusicLabel::Token());
        prim::c >> i << ": ";
        if(m)
          prim::c << *m;
        graph::Instant::PrintProperties(*Music, n);
        prim::c++;
        i++;
        n = Music->Next(n, graph::MusicLabel::Partwise());
      }
    }
    
    //--------------------------------------------------------------------------

    /**Calculates a new left-justified leading edge from an edge and an instant.
    The new leading edge is returned as an array, the new instant origin is
    updated, and*/
    static prim::number CalculateNextLeadingEdge(const StampInstant& Instant,
      prim::Array<prim::number>& LeadingEdge, prim::number& InstantOrigin)
    {      
      ///Array to store new leading edge.
      prim::Array<prim::number> NewLeadingEdge;
      
      //Make sure the leading edge is correctly sized.
      if(LeadingEdge.n() != Instant.n())
      {
        prim::c >> "Error: Leading edge length (" << LeadingEdge.n() << ") "
          << "does not match number of parts (" << Instant.n() << ").";
        return 0.0;
      }
      
      //Calculate the new origin.
      bool SetOrigin = false;
      for(prim::count i = 0; i < LeadingEdge.n(); i++)
      {
        //If the stamp does not exist, then it is not considered.
        if(!Instant[i] || Instant[i]->Bounds().IsEmpty())
          continue;
        
        /*Note that in the future it is possible that collision detection could
        be used here from Source/Optics.h instead of using coarse bounding
        boxes.*/
        prim::number LeastOrigin = LeadingEdge[i] - Instant[i]->Bounds().Left();
        if(!SetOrigin)
        {
          InstantOrigin = LeastOrigin;
          SetOrigin = true;
        }
        else
          InstantOrigin = prim::Max(InstantOrigin, LeastOrigin);
      }
      
      //Calculate the new leading edge.
      NewLeadingEdge = LeadingEdge;
      prim::number FurthestRight = 0.0;
      for(prim::count i = 0; i < LeadingEdge.n(); i++)
      {
        //If the stamp exists on this part then increase the leading edge.
        if(Instant[i] && !Instant[i]->Bounds().IsEmpty())
          NewLeadingEdge[i] = InstantOrigin + Instant[i]->Bounds().Right();
        
        //Keep track of the furthest-right point.
        FurthestRight = prim::Max(FurthestRight, NewLeadingEdge[i]);
      }
      
      //Copy the new leading edge back to the caller array.
      LeadingEdge = NewLeadingEdge;
      
      //Return the furthest right point on the leading edge.
      return FurthestRight;
    }
    
    ///Breaks the music up into systems.
    void CreateSystems(prim::List<System>& Systems,
      prim::number FirstSystemWidth, prim::number RemainingSystemWidth)
    {
      //Initialize a list of systems.
      Systems.RemoveAll();

      //Cache the part and instant count for reference.
      const prim::count PartCount = GraphGeometry.GetNumberOfParts();
      const prim::count InstantCount = GraphGeometry.GetNumberOfInstants();
      
      /*Each system is delineated by a start and end instant, that is to say a
      system contains a continuous range of instants from the total group of
      instants.*/
      prim::count StartInstant = 0, NextStartInstant = 0;

      /*Keep track of repeated instants. Repeated instants are things like 
      clefs, key signatures and barlines.*/
      RepeatedInstants Repeated;
      
      /*While there are instants to still consider create systems and place
      instants on them.*/
      while(NextStartInstant < InstantCount)
      {
        //Consider all the instants in the previous system for repeating.
        if(Systems.n())
        {
          for(prim::count i = 0; i < Systems.z().Instants.n(); i++)
          {
            prim::Debug >> "Considering for repeat: " << i;
            Repeated.Consider(*Music, Systems.z().Instants[i]);
          }
        }
        
        //Start new system and create entries for the leading edge.
        System& Current = Systems.Add();
        Current.LeadingEdge.n(PartCount);
        Current.LeadingEdge.Zero();
        
        /*Deep copy all the repeated elements to the front of the system. The
        stamps need to be deep copied because repeated elements are technically
        different stamps since they may have a different position.*/
        prim::Debug >> "Repeated: " << Repeated.n();
        for(prim::count i = 0; i < Repeated.n(); i++)
        {
          //Deep copy the instant.
          Current.Instants.Add().DeepCopyFrom(Repeated[i]);
          
          //Advance leading edge.
          CalculateNextLeadingEdge(Current.Instants.z(), Current.LeadingEdge,
            Current.InstantPositions.Add(Current.LastInstantPosition()));
        }
        
        /*The furthest wrap point is the right side of the wrap point and is
        exclusive to the current system (the beginning of the next system).*/
        prim::count FurthestWrapPoint = Current.Instants.n();
        
        //Add as many instants to system as will fit.
        for(prim::count i = StartInstant; i < InstantCount; i++)
        {
          //Create the stamp instant from the graph instant.
          Current.Instants.Add() = StampInstant(*Music,
            GraphGeometry.TopMostIslandInInstant(i), PartCount);
          
          //Advance leading edge.
          prim::number FurthestRight =
            CalculateNextLeadingEdge(Current.Instants.z(), Current.LeadingEdge,
            Current.InstantPositions.Add(Current.LastInstantPosition()));
          
          prim::Debug >> i << ": " << FurthestRight;
          
          //Debug print
          for(prim::count j = 0; j < Current.Instants.z().n(); j++)
          {
            if(Current.Instants.z()[j])
            {
              graph::MusicNode p = Current.Instants.z()[j]->Parent;
              if(!p)
              {
                prim::c >> "Warning: Stamp with no parent";
                continue;
              }
              prim::Array<graph::MusicNode> t = Music->Children(p,
                graph::MusicLabel::Token());
              for(prim::count k = 0; k < t.n(); k++)
                prim::Debug >> "  " << j << ": " << *t[k];
            }
          }
          
          /*If the system width has been exceeded, remove anything beyond the
          wrap point and break.*/
          prim::number MaximumSystemWidth =
            (Systems.n() == 1 ? FirstSystemWidth : RemainingSystemWidth);
          if(FurthestRight > MaximumSystemWidth)
          {
            prim::Debug >> "Removing from: " << FurthestWrapPoint;
            Current.Instants.RemoveFrom(FurthestWrapPoint);
            Current.InstantPositions.RemoveFrom(FurthestWrapPoint);
            prim::Debug >> "Instants kept: " << Current.Instants.n();
            
            if(StartInstant == NextStartInstant)
            {
              prim::c >> "Error: Could not break music";
              Systems.RemoveAll();
              return;
            }
            
            StartInstant = NextStartInstant;
            prim::Debug >> "Next start: " << StartInstant;
            break;
          }
          
          /*If this instant is a potential break point, or this is the last
          instant of the last system, then update the wrap point and starting
          instant for the next system (or signal effectively that the main while
          loop should break).*/
          if(Current.Instants.z().Properties.IsOptionalBreak() ||
            i == InstantCount - 1)
          {
            prim::Debug >> "Break point: " << Current.Instants.n();
            prim::Debug >> "  i: " << i;
            FurthestWrapPoint = Current.Instants.n();
            NextStartInstant = i + 1;
          }
        }
      }
    }
    
    void Prepare(prim::List<System>& Systems, prim::number FirstSystemWidth,
      prim::number RemainingSystemWidth)
    {
      //Typeset any graphics that need it.
      Typeset();
      
      //Create the systems.
      prim::number FirstSystemWidthSpaces = FirstSystemWidth / h->SpaceHeight;
      prim::number RemainingSystemWidthSpaces =
        RemainingSystemWidth / h->SpaceHeight;
      prim::number BreathingRoom = 1.5;
      CreateSystems(Systems, FirstSystemWidthSpaces / BreathingRoom,
        RemainingSystemWidthSpaces / BreathingRoom);
      
      //Space the systems.
      for(prim::count i = 0; i < Systems.n(); i++)
      {
        prim::number DesiredSystemWidth;
        if(i == 0 && i != Systems.n() - 1)
          DesiredSystemWidth = FirstSystemWidthSpaces;
        else if(i < Systems.n() - 1)
          DesiredSystemWidth = RemainingSystemWidthSpaces;
        else
          DesiredSystemWidth = 0.0; //Use minimum spacing
        
        Systems[i].CalculateSpacing(*Music,
          h->StaffDistance, DesiredSystemWidth);
        
        if(i == Systems.n() - 1)
          Systems[i].CalculateSpacing(*Music,
            h->StaffDistance, Systems[i].Bounds.Width() * BreathingRoom);
      }
    }
  };
}}
#endif
