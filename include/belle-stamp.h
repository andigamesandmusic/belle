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

#ifndef BELLE_ENGRAVER_STAMP_H
#define BELLE_ENGRAVER_STAMP_H

namespace BELLE_NAMESPACE
{
  ///Persistent collection of graphical objects related to a music graph island.
  class Stamp : public Value::Base
  {
    ///Collection of individual graphical objects.
    Array<Pointer<Graphic> > Graphics;

    ///Stores artificial bounds.
    Array<Box> ArtificialBounds;

    public:

    ///@}
    ///@name Properties
    ///@{

    ///The transformation (in system space) to be applied to the stamp.
    Affine a;

    ///Indicates the parent on which this stamp was placed.
    Music::ConstNode Context;

    ///@}
    ///@name Typesetting
    ///@{

    ///Checks whether the stamp needs to be typeset.
    bool NeedsTypesetting() const {return !Typeset;}

    /**Clears the typesetting. This will clear everything but the parent node
    and accumulated statefulness.*/
    void ClearTypesetting()
    {
      Typeset = false;
      Graphics.Clear();
      ClearLayout();
    }

    ///Clears the graphics for retypesetting.
    void ClearGraphicsForRetypesetting()
    {
      Graphics.Clear();
    }

    ///Indicates that typesetting on this stamp has finished.
    void FinishedTypesetting() {Typeset = true;}

    /**Clears the layout of the stamp. The layout is the position of the stamp
    on the system.*/
    void ClearLayout()
    {
      a = Affine::Unit();
    }

    ///@}

    private:

    ///Indicates whether the stamp needs to be retypeset before displaying it.
    bool Typeset; PRIM_PAD(bool)

    static void PaintVerticalBorders(Painter& Painter, List<Vector> L)
    {
      for(count i = 0; i < L.n() - 1; i++)
      {
        Path p1, p2;
        Shapes::AddLine(p1, L[i], Vector(L[i].x, L[i + 1].y), 0.05f);
        Painter.SetFill(Colors::Red());
        Painter.Draw(p1);

        if(i == L.n() - 2)
          break;
        Shapes::AddLine(p2, Vector(L[i].x, L[i + 1].y), L[i + 1], 0.05f);
        Painter.SetFill(Colors::Orange());
        Painter.Draw(p2);
      }
    }

    static void PaintHorizontalBorders(Painter& Painter, List<Vector> L)
    {
      for(count i = 0; i < L.n() - 1; i++)
      {
        Path p1, p2;
        Shapes::AddLine(p1, L[i], Vector(L[i + 1].x, L[i].y), 0.05f);
        Painter.SetFill(Colors::Red());
        Painter.Draw(p1);

        if(i == L.n() - 2)
          break;
        Shapes::AddLine(p2, Vector(L[i + 1].x, L[i].y), L[i + 1], 0.05f);
        Painter.SetFill(Colors::Orange());
        Painter.Draw(p2);
      }
    }

    public:

    ///@name Painting
    ///@{

    ///Paints the stamp.
    void Paint(Painter& Painter) const
    {
      //Display a warning if the path has not been typeset.
      if(NeedsTypesetting())
        C::Error() >> "Warning: path needs typesetting: " << String(*this);

      //Transform to the context of the stamp.
      Painter.Transform(a);

      //Paint each object in the stamp.
      for(count i = 0; i < Graphics.n(); i++)
        Graphics[i]->Paint(Painter);

      //Undo the transformation.
      Painter.Revert();

      //Paint the vertical borders
#ifdef BELLE_DEBUG_SHOW_VERTICAL_ISLAND_BORDERS
      {
        Array<Box> A = GetGraphicBounds(a);
        PaintVerticalBorders(Painter,
          Box::SegmentedHull(A, Box::LeftSide));
        PaintVerticalBorders(Painter,
          Box::SegmentedHull(A, Box::RightSide));
      }
#endif

      //Cache the bounds of the stamp graphic on the page.
      PaintedBounds = Box();
      for(count i = 0; i < Graphics.n(); i++)
        if(not Graphics[i]->Spans)
          PaintedBounds = PaintedBounds + Graphics[i]->GetPaintedBounds();
      PaintedPageNumber = Painter.GetPageNumber();
#ifdef BELLE_DEBUG_SHOW_STAMP_BOUNDS
      {
        Path p;
        Painter.SetStroke(Colors::Red(), 0.03f);
        Shapes::AddBox(p, Bounds());
        Painter.Draw(p);
      }
#endif
    }

    ///Returns whether the stamp is empty.
    bool IsEmpty()
    {
      return not Graphics.n();
    }

    ///Returns the bounds of the graphic in page space on the most recent paint.
    Box GetPaintedBounds() const {return PaintedBounds;}

    ///Returns the page number of the graphic on the most recent paint.
    count GetPaintedPageNumber() const {return PaintedPageNumber;}

    ///Returns the rectangle bounds of each graphic.
    Array<Box> GetGraphicBounds(Affine Other = Affine::Unit()) const
    {
      Array<Box> A;
      for(count i = 0; i < Graphics.n(); i++)
        A.Add() = Graphics[i]->Bounds(Other);
      for(count i = 0; i < ArtificialBounds.n(); i++)
        A.Add() = Other << ArtificialBounds[i];
      return A;
    }

    ///Returns the rectangle painted bounds of each graphic.
    Array<Box> GetGraphicPaintedBounds() const
    {
      Array<Box> A;
      for(count i = 0; i < Graphics.n(); i++)
        A.Add() = Graphics[i]->GetPaintedBounds();
      return A;
    }

    /**Adds the graphics of another stamp into this one. The other stamp graphic
    affine transforms are transformed by the other stamp transform as they are
    inserted into the current stamp.*/
    void AccumulateGraphics(const Stamp& Other)
    {
      for(count i = 0; i < Other.Graphics.n(); i++)
      {
        Graphics.Add() = Other.Graphics[i],
        Graphics.z()->a = Other.a * Graphics.z()->a;
        if(Other.Context and not Other.Graphics[i]->Context)
          Graphics.z()->Context = Other.Context;
      }
    }

    ///@}

    private:

    /*The following are updated when the stamp gets painted. Bounding boxes for
    UI are always generated *after* the paint because that is when the final
    position of the stamp is known.*/

    ///Bounds of the graphic in page space on the most recent paint.
    mutable Box PaintedBounds;

    ///Page index of the graphic on the most recent paint.
    mutable count PaintedPageNumber;

    public:

    ///@name Construction
    ///@{

    ///Default constructor
    Stamp() : Typeset(false), PaintedPageNumber(-1) {}

    ///Virtual destructor
    virtual ~Stamp();

    ///@}
    ///@name Manipulation
    ///@{

    ///Adds a graphic to the stamp and returns a reference to it.
    Pointer<Graphic> Add()
    {
      Graphics.Add() = new Graphic;
      return Graphics.z();
    }

    ///Gets a reference to the last stamp graphic.
    Pointer<Graphic> z()
    {
      return Graphics.z();
    }

    ///Returns the number of stamp graphics.
    count n() const
    {
      return Graphics.n();
    }

    ///Returns the ith graphic in the stamp.
    Pointer<const Graphic> ith(count i) const
    {
      return Graphics[i];
    }

    ///Gets a const reference to the last stamp graphic.
    Pointer<const Graphic> z() const
    {
      return Graphics.z();
    }

    /**Sets the color of any graphics that have node back-references. Also
    returns the color of unreferenced graphics to the given color for
    unreferenced graphics.*/
    void SetColorOfReferencedGraphics(belle::Color NewColor,
      belle::Color ColorOfUnreferenced = Colors::Black())
    {
      for(count i = 0; i < Graphics.n(); i++)
      {
        if(Graphics[i]->Context)
          Graphics[i]->c = NewColor;
        else
          Graphics[i]->c = ColorOfUnreferenced;
      }
    }

    ///Sets the color of all the graphics in the stamp.
    void SetColorOfGraphics(belle::Color NewColor, bool IncludingSpans = false)
    {
      for(count i = 0; i < Graphics.n(); i++)
        if(IncludingSpans or not Graphics[i]->Spans)
          Graphics[i]->c = NewColor;
    }

    ///@}
    ///@name Bounds
    ///@{

    ///Gets the artificial bounds of the stamp in abstract space.
    Box GetArtificialBounds(Affine Other = Affine::Unit()) const
    {
      Box r;
      for(count i = 0; i < ArtificialBounds.n(); i++)
        r += (Other << ArtificialBounds[i]);
      return r;
    }

    ///Gets the bounds of all the graphics in this stamp in abstract space.
    Box GetAbstractBounds(Affine Other = Affine::Unit()) const
    {
      Box r = GetArtificialBounds(Other);
      for(count i = 0; i < Graphics.n(); i++)
        r += Graphics[i]->Bounds(Other);
      return r;
    }

    ///Gets the bounds of all the graphics in this stamp in abstract space.
    Box GetAbstractBoundsWithoutSpans(Affine Other = Affine::Unit()) const
    {
      Box r = GetArtificialBounds(Other);
      for(count i = 0; i < Graphics.n(); i++)
        if(not Graphics[i]->IsSpan())
          r += Graphics[i]->Bounds(Other);
      return r;
    }

    ///Gets the bounds of the stamp given its current affine transform.
    Box Bounds() const
    {
      return GetAbstractBounds(a);
    }

    ///@}
    ///@name Island
    ///@{

    ///Returns the stamp on a given island.
    static Pointer<Stamp> FromIsland(Music::ConstNode Island)
    {
      if(!Island)
        return Pointer<Stamp>();
      return Island->Stamp().Object();
    }

    ///@}
    ///@name Artificial Bounds
    ///@{

    ///Adds artificial bounds to the stamp.
    void AddArtificialBounds(Box AdditionalBounds)
    {
      ArtificialBounds.Add() = AdditionalBounds;
    }

    ///@}
    ///@name Debugging
    ///@{

    ///Returns a string description of the stamp.
    operator String () const
    {
      if(!Graphics.n())
        return "Empty stamp";

      String s;
      for(count i = 0; i < Graphics.n(); i++)
        s >> " * " << *Graphics[i];
      return s;
    }

    ///Returns the name of this object.
    String Name() const
    {
      return "Stamp";
    }

    ///@}
  };
}

#endif
