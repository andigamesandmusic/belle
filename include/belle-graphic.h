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

#ifndef BELLE_ENGRAVER_GRAPHIC_H
#define BELLE_ENGRAVER_GRAPHIC_H

namespace BELLE_NAMESPACE
{
  /**Persistent graphical object containing path, affine transform, and color.
  It can also link back to a node on the graph, which is useful for tracking
  where the graphic object originated from in the music representation.*/
  class Graphic
  {
    public:

    ///@name Properties
    ///@{

    ///Path of the graphic.
    Pointer<const Path> p;

    ///Affine transformation of the graphic in stamp space.
    Affine a;

    ///Fill or stroke color of the graphic.
    mutable Color c;

    ///If non-zero, strokes the path with this width instead of filling it.
    number w;

    ///The graph node contextually related to this graphic.
    Music::ConstNode Context;

    ///Indicates whether this graphic spans into another graphic.
    bool Spans; PRIM_PAD(bool)

    ///@}

    private:

    /*The following are updated when the stamp gets painted. Bounding boxes for
    UI are always generated *after* the paint because that is when the final
    position of the stamp is known.*/

    ///Affine space of the graphic as painted.
    mutable Affine PaintedSpace;

    ///Bounds of the graphic in page space on the most recent paint.
    mutable Box PaintedBounds;

    ///Page index of the graphic on the most recent paint.
    mutable count PaintedPageNumber;

    public:

    ///@}
    ///@name Construction
    ///@{

    ///Constructor to initialize properties.
    Graphic() : c(Colors::Black()), w(0.0),  Spans(false),
      PaintedPageNumber(-1) {}

    ///@}
    ///@name Bounds
    ///@{

    ///Gets the bounds of the stamp graphic.
    Box Bounds(Affine Other = Affine::Unit()) const
    {
      if(!p)
        return Box();
      return p->Bounds(Other * a);
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

    ///Returns whether the engraver marked graphic as spanning.
    bool IsSpan() const {return Spans;}

    ///@}
    ///@name Painting
    ///@{

    ///Paints the stamp graphic.
    void Paint(Painter& Painter) const
    {
      //Set the stroke and fill style.
      if(Limits<number>::IsNotZero(w))
        Painter.SetStroke(c, w);
      else
        Painter.SetFill(c);

      //Paint the stamp graphic.
      Painter.Draw(p, a);

      //Cache the bounds of the stamp graphic on the page.
      PaintedSpace = Painter.CurrentSpace();
      PaintedBounds = Bounds(PaintedSpace);
      PaintedPageNumber = Painter.GetPageNumber();
    }

    ///Returns the affine space in which the graphic is drawn relative to page.
    Affine GetPaintedSpace() const {return PaintedSpace;}

    ///Returns the bounds of the graphic in page space on the most recent paint.
    Box GetPaintedBounds() const {return PaintedBounds;}

    ///Returns the page number of the graphic on the most recent paint.
    count GetPaintedPageNumber() const {return PaintedPageNumber;}

    ///@}

    private:

    ///Helper function to print out more useful rectangle descriptions.
    static String Print(const Box& r)
    {
      String s;
      s.Precision(2);
      s << "[x:" << r.Left() << " to " << r.Right() << " y:"
        << r.Bottom() << " to " << r.Top() << "]";
      return s;
    }

    public:

    ///@name Debugging
    ///@{

    ///Returns a string description of the graphic.
    operator String () const
    {
      String s;
      s >> "Path at " << Print(Bounds());
      if(!GetPaintedBounds().IsEmpty())
        s << " last painted to " << Print(GetPaintedBounds()) << " on page " <<
          GetPaintedPageNumber();
      return s;
    }
    ///@}
  };
}
#endif
