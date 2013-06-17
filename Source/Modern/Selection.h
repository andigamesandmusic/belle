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

#ifndef BELLE_MODERN_SELECTION_H
#define BELLE_MODERN_SELECTION_H

#include "Stamp.h"

namespace BELLE_NAMESPACE { namespace modern
{
  ///Keeps track of stamp graphic selections.
  class ClickThroughManager
  {
    /**Keeps track of the cycle-through index. Whenever a stamp-graphic is
    clicked, this value increments, and the stamp graphic receives this value.
    When looking for stamp graphics next time, the one with the lowest click
    index is most preferred (because it has not been clicked the longest
    relative to the others).*/
    prim::count CycleThroughIndex;

    ///Array of the stamp graphics painted in the last paint call.
    prim::Array<StampGraphic*> PaintedStampGraphics;
    
    public:
    
    ///Initializes the click-through manager.
    ClickThroughManager() : CycleThroughIndex(0) {}
    
    ///Clears the current selection.
    void ClearSelection()
    {
      CycleThroughIndex++;
    }
    
    ///Returns the current cycle-through selection index.
    prim::count GetSelectionIndex()
    {
      return CycleThroughIndex;
    }
    
    ///Clears the stamp graphics cache.
    void ClearStampGraphicsCache()
    {
      PaintedStampGraphics.Clear();
    }
    
    ///Adds a stamp graphic to the manager.
    void AddStampGraphicForConsideration(StampGraphic* s)
    {
      if(!s) return;
      PaintedStampGraphics.Add() = s;
    }
    
    /**Returns the stamp graphic given a page coordinate in inches. Note that
    this method can easily return different values with the same input even
    called back-to-back due to cycle-through. This method keeps track of the
    click index of each stamp graphic and rotates through the available stamp
    graphics for each call, so if stamp graphics overlap then this call will
    rotate through the available ones. The rotation is accomplished by setting
    stamp graphics to least-preferred following selection, and the selector
    always chooses the most-preferred to return and makes the graphic
    least-preferred for the subsequent call.*/
    StampGraphic* MakeSelection(prim::count PageIndex, Inches PageCoordinate)
    {
      //Look for a selection using the cycle-through algorithm.
      StampGraphic* MostPreferred = 0;
      prim::count LowestClickValue = 0;
      for(prim::count i = 0; i < PaintedStampGraphics.n(); i++)
      {
        //Get the next stamp graphic.
        StampGraphic* Current = PaintedStampGraphics[i];
        
        //Skip stamp graphics that do not contain the coordinate.
        if(PageIndex != Current->PlacementPageIndex ||
          !Current->PlacementOnLastPaint.Contains(PageCoordinate))
            continue;
        
        /*For the first graphic encountered, just use it. Otherwise, compare the
        click value and use the lowest.*/
        if(!MostPreferred || Current->ClickIndex < LowestClickValue)
        {
          LowestClickValue = Current->ClickIndex;
          MostPreferred = Current;
        }
      }
      
      //If nothing was preferred, then there is no selection.
      if(!MostPreferred)
        return 0;
      
      //Get the selected graphic.
      StampGraphic* Selected = MostPreferred;
      
      /*Update the cycle-through so that the latest clicked item has the
      highest, and thus least preferred index.*/
      Selected->ClickIndex = ++CycleThroughIndex;
      
      /*Transfer related selections. For example, clicking on a flag might
      also select the note depending on whether the underlying node is the
      same.*/
      for(prim::count i = 0; i < PaintedStampGraphics.n(); i++)
        PaintedStampGraphics[i]->TransferSelected(Selected);
      
      //Return the selected graphic.
      return Selected;
    }
    
    /**Paints the bounding boxes of the stamp graphics which have been included.
    This is primarily for debug purposes. Note that this method temporarily
    transforms back to page space in order to draw using page coordinates.*/
    void PaintBoundingBoxes(Painter* Painter)
    {
      Affine a = Painter->Space().Forwards();
      Painter->Transform(-a);
      {
        for(prim::count i = 0; i < PaintedStampGraphics.n(); i++)
        {
          StampGraphic* s = PaintedStampGraphics[i];
          Path p;
          Shapes::AddRectangle(p, s->PlacementOnLastPaint);
          if(s->ClickIndex == CycleThroughIndex && CycleThroughIndex)
          {
            Painter->SetStroke(Colors::green, 0.01);
            Painter->Draw(p);
          }
          else
          {
            Painter->SetStroke(Colors::blue, 0.01);
            //Painter->Draw(p);
          }
        }
      }
      Painter->Revert();
    }
    
    ///Shows the currently selected stamp graphic by changing its fill color.
    void ShowSelected(Color SelectedColor,
      Color DefaultColor = Colors::black)
    {
      for(prim::count i = 0; i < PaintedStampGraphics.n(); i++)
      {
        if(StampGraphic* s = PaintedStampGraphics[i])
        {
          if(s->ClickIndex == CycleThroughIndex && CycleThroughIndex)
            s->c = SelectedColor;
          else
            s->c = DefaultColor;
        }
      }
    }
  };
}}
#endif
