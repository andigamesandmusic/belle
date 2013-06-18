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

#ifndef BELLE_MODERN_STAMP_H
#define BELLE_MODERN_STAMP_H

namespace BELLE_NAMESPACE { namespace modern
{
  ///Single graphical object consisting of a path and an affine transform.
  struct StampGraphic
  {
    ///Path of the graphic.
    Path p;
    
    ///Fill color of the stamp.
    Color c;
    
    ///Path to cached graphic.
    const Path* p2;
    
    ///Affine transformation of the graphic.
    Affine a;
    
    ///Text of the graphic (provisional object, needs rework).
    Text* t;
    
    ///Strokes the path with the width instead of filling it.
    prim::number StrokeWidth;
    
    ///Graph node related to the graphic.
    graph::MusicNode n;
    
    ///Placement of the stamp graphic on the last paint.
    prim::planar::Rectangle PlacementOnLastPaint;
    
    ///Index of page.
    prim::count PlacementPageIndex;
    
    /**Click index of the stamp graphic. This keeps track of cycle-through.
    Essentially when an object is clicked, its click index becomes the next
    cycle-through value and thus becomes least-preferred for the next click.
    This value is maintained by the ClickThroughManager.*/
    prim::count ClickIndex;
    
    /**Transfers the selection of another stamp graphic if node is the same.
    This is used to transfer a selection of the part to the whole. For example,
    clicking on a flag would select the rest of the note if the underlying node
    was the same.*/
    void TransferSelected(StampGraphic* SelectedStamp)
    {
      if(SelectedStamp && SelectedStamp->n && SelectedStamp->n == n)
        ClickIndex = SelectedStamp->ClickIndex;
    }
    
    ///Gets the bounds of the stamp graphic.
    prim::planar::Rectangle Bounds(Affine Other = Affine::Unit()) const
    {
      return p2 ? p2->Bounds(Other * a) : p.Bounds(Other * a);
    }
    
    ///Returns whether the stamp graphic does not contain visible graphics.
    bool IsEmpty() const
    {
      return Bounds().IsEmpty();
    }
    
    ///Returns whether the stamp graphic contains visible graphics.
    bool IsNotEmpty() const
    {
      return !IsEmpty();
    }
    
    ///Applies a selection coloring if the selection matches the click index.
    void ApplySelectionColoring(prim::count SelectionIndex,
      Color SelectionColor =
        Colors::mediumseagreen)
    {
      if(SelectionIndex == ClickIndex && SelectionIndex)
        c = SelectionColor;
      else
        ApplyNodeColoring();
    }

    ///Applies the coloring from the node.
    void ApplyNodeColoring()
    {
      if(!n) return;
      c = Colors::black; //n->IntendedColor;
    }
    
    ///Paints the stamp graphic.
    void Paint(Painter* Painter)
    {
      //Set the stroke and fill style.
      if(StrokeWidth)
        Painter->SetStroke(c, StrokeWidth);
      else
        Painter->SetFill(c);

      //Paint the stamp graphic.
      if(p2)
        Painter->Draw(*p2, a);
      else if(t)
      {
        Painter->Transform(Affine::Scale(20.0));
        Painter->Draw(*t, a * Affine::Scale(20.0));
      }
      else
        Painter->Draw(p, a);
      
      //Cache the bounds of the stamp graphic on the page.
      PlacementOnLastPaint = Bounds(Painter->CurrentSpace());
      PlacementPageIndex = Painter->GetPageNumber();
    }
    
    ///Constructor to zero the text pointer.
    StampGraphic() : c(Colors::black), p2(0), t(0),
     StrokeWidth(0.0), n(0), PlacementPageIndex(-1), ClickIndex(0) {}
    
    ///Destructor to delete the text pointer.
    ~StampGraphic() {delete t;}
  };
  
  ///Graphical object with multiple items positioned relative to each other.
  struct Stamp : public graph::TypesettingInfo
  {
    ///Array of individual graphical objects with their own affine transforms.
    prim::Array<StampGraphic*> Graphics;
    
    ///The transformation to be applied to the stamp to position and scale it.
    Affine Context;
    
    ///Indicates whether the stamp needs to be retypeset before displaying it.
    bool NeedsTypesetting;
    
    ///Indicates the parent on which this stamp was placed.
    graph::MusicNode Parent;
    
    ///Copy constructor to deep copy the stamp.
    Stamp(const Stamp& Other) : graph::TypesettingInfo(Other)
    {
      //Make deep-copies of the stamp graphic.
      for(prim::count i = 0; i < Other.Graphics.n(); i++)
        Add() = *Other.Graphics[i];
      
      //Copy the other attributes.
      Context = Other.Context;
      NeedsTypesetting = Other.NeedsTypesetting;
      Parent = Other.Parent;
    }
    
    ///Adds a stamp graphic.
    StampGraphic& Add()
    {
      return *(Graphics.Add() = new StampGraphic);
    }

    ///Gets the last stamp graphic.
    StampGraphic& z()
    {
      return *Graphics.z();
    }
    
    ///Gets the last stamp graphic.
    const StampGraphic& z() const
    {
      return *Graphics.z();
    }
    
    ///Resets the stamp to have no graphical objects.
    void Clear(graph::MusicNode WithParent = 0)
    {
      NeedsTypesetting = true;
      Graphics.ClearAndDeleteAll();
      Context = Affine::Unit();
      Parent = WithParent;
    }
    
    ///Paints the stamp.
    void Paint(Painter* Painter)
    {
      //Transform to the context of the stamp.
      Painter->Transform(Context);
      
      //Paint each object in the stamp.
      for(prim::count i = 0; i < Graphics.n(); i++)
        Graphics[i]->Paint(Painter);
      
      //Reset the color and undo the transformation.
      Painter->Revert();
      
#if 0 //Draw stamp bounding box.
      {
        Path p;
        Shapes::AddRectangleFromLines(p, Bounds(Context), 0.1);
        Painter->Draw(p);
      }
#endif
    }
  
    ///Gets the bounds of the stamp.
    prim::planar::Rectangle Bounds(Affine a = Affine::Unit()) const
    {
      prim::planar::Rectangle r;
      for(prim::count i = 0; i < Graphics.n(); i++)
        r = r + Graphics[i]->Bounds(a);
      return r;
    }
    
    ///Gets the bounds of the stamp in the current context.
    prim::planar::Rectangle BoundsInContext() const
    {
      return Bounds(Context);
    }
    
    ///Constructor creates a blank stamp.
    Stamp(graph::MusicNode Parent) {Clear(Parent);}
  };
  
  ///Information relating to the stamps in a single instant.
  struct StampInstant : public prim::Array<prim::Pointer<Stamp> >
  {
    ///Store the instant properties.
    graph::Instant::Properties Properties;
    
    ///Copies the stamp references from an instant in a graph.
    void CopyFromInstant(graph::Music& g,
      graph::MusicNode IslandInInstant,
      prim::count GeometryPartCount)
    {
      //Clear this object.
      Properties.Clear();
      Clear();

      //Get the top of the instant.
      graph::MusicNode Isle = IslandInInstant;
      if(!g.RaiseToTopPart(Isle)) return;
      
      //Get the properties.
      graph::Instant::GetProperties(g, Properties, Isle); 
      
      //Size the array to the total number of possible stamps (the part count).
      n(GeometryPartCount);
      Zero();
      
      //Travel through all the islands in this instant.
      while(Isle)
      {
        //Copy the stamp pointer to the array.
        if(prim::Pointer<Stamp> s = Isle->Label.Typesetting)
          ith(s->PartID) = s;
        
        //Go to the next island.
        Isle = g.Next(Isle, graph::MusicLabel::Instantwise());
      }
    }
    
    ///Default constructor.
    StampInstant() {}
    
    ///Constructor to copy an instant.
    StampInstant(graph::Music& g, graph::MusicNode IslandInInstant,
      prim::count GeometryPartCount)
    {
      CopyFromInstant(g, IslandInInstant, GeometryPartCount);
    }
    
    ///Deep copies the stamp instant from another.
    void DeepCopyFrom(const StampInstant& Other)
    {
      n(Other.n());
      for(prim::count i = 0; i < n(); i++)
        if(Other[i])
          ith(i) = new Stamp(*Other[i]);
    }
  };
  
  ///Stores the repeated instants.
  struct RepeatedInstants : public prim::List<StampInstant>
  {
    /**Considers an instant for repeating. This should be called once per
    instant.*/
    void Consider(graph::Music& g, const StampInstant& Other)
    {
      //If a repeating instant is encountered then add it to the list.
      if(Other.Properties.IsRepeatingInstant())
      {
        //Copy the stamp instant to a new instant entry in the list.
        Add() = Other;
        
        /*The properties need to be cleared so that this stamp instant is not
        reconsidered on each system break.*/
        z().Properties.Clear();
        return;
      }
      
      /*Go through each stamp in the instant and see if it can replace one
      already in the repeating instant list. This is done by checking to see if
      the type matches.*/
      for(prim::count j = 0; j < Other.n(); j++)
      {
        for(prim::count i = 0; i < n(); i++)
        {
          //If there is no stamp then skip it.
          if(!ith(i)[j] || !Other[j])
            continue;
          
          //All stamps should have been initialized with a parent.
          if(!ith(i)[j]->Parent || !Other[j]->Parent)
          {
            prim::c >> "Error: Stamp with no parent: " << i << ", " << j << ": "
              << ith(i)[j]->Parent << ", " << Other[j]->Parent;
            continue;
          }
          
          //Get the child tokens of the islands.
          graph::MusicNode t1, t2;
          t1 = g.Next(ith(i)[j]->Parent, graph::MusicLabel::Token());
          t2 = g.Next(Other[j]->Parent, graph::MusicLabel::Token());
          
          if(!t1 || !t2)
            continue;
            
          //Copy the stamp reference if it is of the same type.
          if(t1->Get(mica::Type) == t2->Get(mica::Type))
          {
            ith(i)[j] = Other[j];
            break; //Do not copy the same stamp more than once.
          }
        }
      }
    }
  };
}}
#endif
