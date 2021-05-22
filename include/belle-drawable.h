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

#ifndef BELLE_CORE_DRAWABLE_H
#define BELLE_CORE_DRAWABLE_H

#include "belle-abstracts.h"
#include "belle-svg.h"

namespace BELLE_NAMESPACE
{
  /**Class for drawing SVG like data. This is not intended to be an
  implementation of SVG; it simply reads in SVG path data and simple fill and
  stroke attributes.*/
  class Drawable
  {
    List<Path> Paths;
    List<Brush> Brushes;
    Vector Dimensions;

    public:

    Drawable() : Dimensions(1.f, 1.f) {}

    Drawable(const String& SVGData, bool ScaleToUnitSquare = false)
    {
      Import(SVGData, ScaleToUnitSquare);
    }

    Vector GetDimensions() {return Dimensions;}

    void Import(const String& SVGData, bool ScaleToUnitSquare = false)
    {
      Box r = SVGHelper::Import(Paths, Brushes, SVGData);
      if(r.IsEmpty())
        for(count i = 0; i < Paths.n(); i++)
          r += Paths[i].Bounds();
      Dimensions = r.IsEmpty() ? Vector(1.f, 1.f) : r.Size();
      if(ScaleToUnitSquare and not r.IsEmpty())
      {
        number ScaleFactor = 1.f / Max(r.Width(), r.Height());
        Dimensions *= ScaleFactor;
        Affine Translate = Affine::Translate(r.BottomLeft() * -1.f);
        Affine Scale = Affine::Scale(Vector(ScaleFactor, -ScaleFactor));
        Affine TranslateScale = Affine::Translate(Vector(0.f, 1.f)) *
          Scale * Translate;
        for(count i = 0; i < Paths.n(); i++)
        {
          Path p(Paths[i], TranslateScale);
          Paths[i] = p;
          Brushes[i].StrokeWidth *= ScaleFactor;
        }
      }
    }

    void Paint(Painter& Painter) const
    {
      for(count i = 0; i < Paths.n(); i++)
      {
        Painter.SetBrush(Brushes[i]);
        Painter.Draw(Paths[i]);
      }
    }

    void PaintWithBrush(Painter& Painter, const Brush& BrushToUse) const
    {
      Painter.SetBrush(BrushToUse);
      for(count i = 0; i < Paths.n(); i++)
        Painter.Draw(Paths[i]);
    }
  };
}
#endif
