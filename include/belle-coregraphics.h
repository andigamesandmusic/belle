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

#ifndef BELLE_PAINTERS_COREGRAPHICS_H
#define BELLE_PAINTERS_COREGRAPHICS_H

#ifdef BELLE_WITH_COREGRAPHICS
#import <CoreGraphics/CoreGraphics.h>

namespace BELLE_NAMESPACE
{
  class CoreGraphics : public Painter
  {
    public:

    class Properties : public Painter::Properties
    {
      CGContextRef RawViewDrawingContext;

      public:

      Properties(CGContextRef displayDrawingContext)
      {
        RawViewDrawingContext = displayDrawingContext;
      }

      CGContextRef GetRawViewDrawingContext() const
      {
        return RawViewDrawingContext;
      }

      ///Virtual destructor
      virtual ~Properties() {}

      friend class CoreGraphics;
    };

    private:

    ///Contains the CoreGraphics drawing context.
    Properties* CGProperties;

    ///Saves the portfolio pointer during painting so draw calls can access it.
    Portfolio* CachedPortfolio;

    public:

    ///Constructor initializes the JUCE renderer.
    CoreGraphics() : CGProperties(0), CachedPortfolio(0) {}

    ///Virtual destructor
    virtual ~CoreGraphics() {}

    ///Calls the paint event of the current canvas being painted.
    void Paint(Portfolio* PortfolioToPaint,
      Painter::Properties* PortfolioProperties)
    {
      //Cache the portfolio.
      CachedPortfolio = PortfolioToPaint;

      //Get a pointer to the properties.
      CGProperties = PortfolioProperties->Interface<Properties>();

      //Bail if no properties or portfolio.
      if(not CGProperties or not CachedPortfolio)
        return;

      //Save the state.
      CGContextSaveGState(CGProperties->GetRawViewDrawingContext());

      //Show only the first canvas
      if(CachedPortfolio->Canvases.n())
      {
        //Set the current page number.
        SetPageNumber(0);

        //Paint the current canvas.
        CachedPortfolio->Canvases.a()->Paint(*this, *CachedPortfolio);

        //Reset the page number to indicate painting is finished.
        ResetPageNumber();
      }

      //Restore the state.
      CGContextRestoreGState(CGProperties->GetRawViewDrawingContext());

      //Set the properties pointer back to null to be safe.
      CGProperties = 0;

      //Set the cached portfolio pointer back to null.
      CachedPortfolio = 0;
    }

    ///Draws a path to the CoreGraphics context.
    void Draw(const Path& p, const Affine& a)
    {
      Transform(a);
      Affine A = CurrentSpace();
      if(A.IsInvertible())
      {
        CGContextRef Context = CGProperties->GetRawViewDrawingContext();

        CGContextBeginPath(Context);
        for(count j = 0; j < p.n(); j++)
        {
          const Instruction& i = p[j];
          Vector c1 = i.Control1(), c2 = i.Control2(), e = i.End();
          c1 = A << c1, c2 = A << c2, e = A << e;

          if(i.IsMove())
            CGContextMoveToPoint(Context, CGFloat(e.x), CGFloat(e.y));
          else if(i.IsLine())
            CGContextAddLineToPoint(Context, CGFloat(e.x), CGFloat(e.y));
          else if(i.IsCubic())
            CGContextAddCurveToPoint(Context, CGFloat(c1.x), CGFloat(c1.y),
            CGFloat(c2.x), CGFloat(c2.y), CGFloat(e.x), CGFloat(e.y));
          else
            CGContextClosePath(Context);
        }

        bool Fill = false;
        if(Limits<number>::IsNotZero(State.FillColor.A))
        {
          Fill = true;
          CGContextSetRGBFillColor(Context, CGFloat(State.FillColor.R),
            CGFloat(State.FillColor.G), CGFloat(State.FillColor.B),
            CGFloat(State.FillColor.A));
        }

        bool Stroke = false;
        number ScaledStrokeWidth = State.StrokeWidth *
            Vector(A.a, A.d).Mag() / Sqrt(number(2.f));
        if(ScaledStrokeWidth > 0.f)
        {
          Stroke = true;
          CGContextSetRGBStrokeColor(Context, CGFloat(State.StrokeColor.R),
            CGFloat(State.StrokeColor.G), CGFloat(State.StrokeColor.B),
            CGFloat(State.StrokeColor.A));
          CGContextSetLineWidth(Context, CGFloat(ScaledStrokeWidth));
        }

        if(Fill and not Stroke)
          CGContextDrawPath(Context, kCGPathFill);
        else if(not Fill and Stroke)
          CGContextDrawPath(Context, kCGPathStroke);
        else if(Fill and Stroke)
          CGContextDrawPath(Context, kCGPathFillStroke);
      }
      else
      {
        static bool HaveWarnedBefore = false;
        if(not HaveWarnedBefore)
        {
          C::Error() >> "Error: can not draw with affine matrix: " << A;
          HaveWarnedBefore = true;
        }
      }
      Revert();
    }

    ///Drawing image resources is not supported.
    void Draw(const Resource& ResourceID, Vector Size)
    {
      (void)ResourceID;
      (void)Size;
    }
  };
}
#endif
#endif
