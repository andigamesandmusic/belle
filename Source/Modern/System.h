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

#ifndef BELLE_MODERN_SYSTEM_H
#define BELLE_MODERN_SYSTEM_H

#include "Directory.h"
#include "Stamp.h"

namespace BELLE_NAMESPACE { namespace modern
{
  ///Describes a list of stamp instants and their positions.
  struct System
  {
    ///Stamp instant list describing the graphics that make up the instant.
    prim::List<StampInstant> Instants;

    ///List of instant positions.
    prim::List<prim::number> InstantPositions;
    
    ///The current leading edge.
    prim::Array<prim::number> LeadingEdge;
    
    ///Staff heights.
    prim::Array<prim::number> StaffHeights;
    
    ///Staff uses staff lines.
    prim::Array<bool> HasStaffLines;
    
    ///Total height of the box that would bound the system.
    prim::number SystemHeight;
    
    ///Remembers the minimum system width.
    prim::number MinimumSystemWidth;
    
    ///The final bounds of the system.
    prim::planar::Rectangle Bounds;
    
    ///Gets the last known instant position.
    prim::number LastInstantPosition() const
    {
      if(InstantPositions.n())
        return InstantPositions.z();
      else
        return 0.0;
    }
    
    /**Distribute the staves. If the system width is zero, then it is left at
    minimum spacing.*/
    void CalculateSpacing(graph::Music& g, prim::number SpaceBetweenSystems,
      prim::number SystemWidth = 0.0)
    {
      //If there are no instants then return.
      if(!Instants.n())
      {
        prim::c >> "Error: No instants in this system.";
        return;
      }
      
      /*Get the number of parts in this system. Note this does not yet take
      into account unused staves like it should.*/
      prim::count PartCount = Instants.a().n();
      StaffHeights.n(PartCount);
      
      //Equally space the systems.
      for(prim::count i = 0; i < PartCount; i++)
        StaffHeights[i] = ((prim::number)(PartCount - 1 - i)) *
          SpaceBetweenSystems;
      
      //Get bounds of the system (without adjusted height).
      prim::planar::Rectangle Bound;
      for(prim::count i = 0; i < Instants.n(); i++)
        for(prim::count j = 0; j < Instants[i].n(); j++)
          if(prim::Pointer<Stamp> s = Instants[i][j])
            Bound += s->Bounds(Affine::Translate(prim::planar::Vector(
              InstantPositions[i], StaffHeights[j])));
      
      //Check the bounds to make sure they are sensible.
      if(Bound.IsEmpty())
      {
        prim::c >> "Error: System bound is empty.";
        return;
      }
      
      //Save the system width and height.
      MinimumSystemWidth = Bound.Right();
      SystemHeight = Bound.Height();
      
      //Space the elements.
      if(SystemWidth > 0.0)
        EqualSpace(SystemWidth);
      
      /*Adjust the staff heights to bring the bottom-most element flush with
      the x-axis.*/
      for(prim::count i = 0; i < StaffHeights.n(); i++)
        StaffHeights[i] -= Bound.Bottom();
      
      //Position the stamps in the system.
      for(prim::count i = 0; i < Instants.n(); i++)
        for(prim::count j = 0; j < Instants[i].n(); j++)
          if(prim::Pointer<Stamp> s = Instants[i][j])
            s->Context = Affine::Translate(prim::planar::Vector(
              InstantPositions[i], StaffHeights[j]));
      
      //Adjust stamps which are different due to repeating or non-repeating.
      AdjustStamps(g);
      
      //Calculate the bounds of the system.
      prim::planar::Rectangle NewBounds;
      for(prim::count i = 0; i < Instants.n(); i++)
        for(prim::count j = 0; j < Instants[i].n(); j++)
          if(prim::Pointer<Stamp> s = Instants[i][j])
            NewBounds += s->BoundsInContext();
      Bounds = NewBounds;
    }
    
    ///Gets the line-space of a coordinate given in spaces.
    mica::Concept CoordinateToLineSpace(prim::count StaffOnSystem,
      prim::planar::Vector SpacesRelativeToSystem)
    {
      //Get the relative line space position.
      prim::number LSPosition =
        SpacesRelativeToSystem.y - StaffHeights[StaffOnSystem];
      
      /*Note this line assumes integer spacing for lines. In reality, this is
      needs to ask the house style since spaces above the staff can be
      condensed.*/
      prim::integer LSIndex = prim::Round(LSPosition * 2.0);
      
      return mica::integer(LSIndex);
    }
    
    ///Gets the line space of a coordinate in inches.
    mica::Concept CoordinateToLineSpace(prim::count StaffOnSystem,
      Inches RelativeToSystem, const House& h)
    {
      return CoordinateToLineSpace(StaffOnSystem, RelativeToSystem /
        h.SpaceHeight);
    }
    
    ///Gets the line space of a coordinate in inches.
    mica::Concept CoordinateToLineSpace(graph::Music& g, graph::MusicNode nn,
      Inches RelativeToSystem, const House& h)
    {
      if(!nn) return mica::Undefined;
      graph::MusicNode i = g.Previous(g.Previous(nn, graph::MusicLabel::Note()),
        graph::MusicLabel::Token());
      if(!i || !i->Label.Typesetting) return mica::Undefined;
      return CoordinateToLineSpace(i->Label.Typesetting->PartID,
        RelativeToSystem, h);
    }
    
    private:
    
    ///Temporary hacks to adjust differences in repeated/non-repeated stamps.
    void AdjustStamps(graph::Music& g)
    {
      //If there are no instants, then just return.
      if(!Instants.n()) return;
      
      /*Resize non-repeating clefs to be smaller. Note the starting index is
      not technically general. There could be a score with no barline or key
      signature with a clef [0] and then immediate change of clef [1]. A
      better approach would be to determine if the stamp is dependent or
      independent.*/
      for(prim::count i = 2; i < Instants.n(); i++)
      {
        for(prim::count j = 0; j < Instants[i].n(); j++)
        {
          if(prim::Pointer<Stamp> s = Instants[i][j])
          {
            graph::MusicNode ct = g.Next(s->Parent, graph::MusicLabel::Token());
            if(ct)
            {
              //Hack: constant should come from house style.
              for(prim::count k = 0; k < s->Graphics.n(); k++)
                s->Graphics[k]->a *= Affine::Scale(0.8);
            }
          }
        }
      }
      
      //Check to see if the staff will take staff lines.
      HasStaffLines.n(Instants.a().n());
      HasStaffLines.Zero();
      for(prim::count j = 0; j < HasStaffLines.n(); j++)
      {
        for(prim::count i = 0; i < Instants.n(); i++)
        {
          if(prim::Pointer<Stamp> s = Instants[i][j])
          {
            graph::MusicNode ct = g.Next(s->Parent, graph::MusicLabel::Token());
            if(ct)
            {
              HasStaffLines[j] = true;
              break;
            }
          }
        }
      }
    }
    
    ///Temporary hack to space the instants.
    void EqualSpace(prim::number SystemWidth)
    {
      //Make sure there are more than one instants to space.
      if(InstantPositions.n() <= 1) return;
      
      //Calculate extra space per instant.
      prim::number ExtraSpace = SystemWidth - MinimumSystemWidth;
      prim::number ExtraSpacePerInstant = ExtraSpace /
        (prim::number)(InstantPositions.n() - 1);
      
      //Add equal extra space.
      for(prim::count i = 0; i < InstantPositions.n(); i++)
      {
        InstantPositions[i] += ((prim::number)i) * ExtraSpacePerInstant;
      }
    }
    
    public:
    
    ///Paints the system.
    void Paint(Painter& Painter, prim::planar::Vector BottomLeftPosition =
      prim::planar::Vector(), prim::number SpaceHeight = 1.0,
      ClickThroughManager* CTM = 0, bool GuessBrace = true)
    {
      prim::number LineThickness = 0.15;
    
      //If there are no instants, then just return.
      if(!Instants.n()) return;
      
      //Translate and scale the system.
      Painter.Translate(BottomLeftPosition);
      Painter.Scale(SpaceHeight);
      
      /*Quickly calculate the bounds to determine where staff lines should be
      painted. Note the i += Instants.n() - 1 optimization.*/
      prim::planar::Rectangle Bounds;
      prim::count Increment = (Instants.n() > 1 ? Instants.n() - 1 : 1);
      for(prim::count i = 0; i < Instants.n(); i += Increment)
        for(prim::count j = 0; j < Instants[i].n(); j++)
          if(prim::Pointer<Stamp> s = Instants[i][j])
            Bounds += s->BoundsInContext();
      
      //Paint the staff lines.
      {
        Painter.SetFill(Colors::black);
        Path p;
        Shapes::AddLine(p, prim::planar::Vector(LineThickness * 0.5, 0),
          prim::planar::Vector(Bounds.Right() - LineThickness * 0.5, 0),
          LineThickness);
        for(prim::count i = 0; i < Instants.a().n(); i++)
        {
          if(!HasStaffLines[i]) continue;
          for(prim::count j = -2; j <= 2; j++)
            Painter.Draw(p, Affine::Translate(prim::planar::Vector(0,
              StaffHeights[i] + (prim::number)j)));
        }
      }
      
      //Paint the stamps in the system.
      for(prim::count i = 0; i < Instants.n(); i++)
      {
        for(prim::count j = 0; j < Instants[i].n(); j++)
        {
          if(prim::Pointer<Stamp> s = Instants[i][j])
          {
            if(CTM)
            {
              //Apply selection coloring.
              for(prim::count k = 0; k < s->Graphics.n(); k++)
                s->Graphics[k]->ApplySelectionColoring(
                  CTM->GetSelectionIndex());
            }

            //Paint
            s->Paint(&Painter);
             
            if(CTM)
            {
              //Add each constituent stamp graphic to the click through.
              for(prim::count k = 0; k < s->Graphics.n(); k++)
                CTM->AddStampGraphicForConsideration(s->Graphics[k]);
            }
          }
        }
      }
      
      //Paint the brace.
      if(GuessBrace) //Assume piano for the moment.
      {
        Path p;
        Shapes::Music::AddBrace(p,
          prim::planar::Vector(-2.0, (StaffHeights[0] + StaffHeights[1]) / 2.0),
          StaffHeights[0] - StaffHeights[1] + 3.0);
        Painter.SetFill(Colors::black);
        Painter.Draw(p);
      }
      
#if 0
      //Paint the bounding box.
      {
        Path b;
        Shapes::AddRectangleFromLines(b, System::Bounds, 0.1);
        Painter.Draw(b);
      }
#endif
      
      //Undo the transformation.
      Painter.Revert(2);
    }
    
    //Prints the instant positions of the system.
    operator prim::String () const
    {
      prim::String s;
      s << InstantPositions;
      return s;
    }
  };
}}
#endif
