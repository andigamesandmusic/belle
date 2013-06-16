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

#ifndef BELLE_PAINTERS_JUCE_H
#define BELLE_PAINTERS_JUCE_H

namespace BELLE_NAMESPACE { namespace painters
{
#ifdef JUCE_VERSION
  /*An painter implementation of Belle, Bonne, Sage using a JUCE component as
  the target device. In very little code, we can neatly map the abstract
  graphics calls to the JUCE graphics engine.*/
  struct JUCE : public Painter
  {
    struct Properties : public Painter::Properties
    {
      juce::Graphics* GraphicsContext;
      juce::Component* ComponentContext;
      prim::count IndexOfCanvas;
      Inches PageDimensions;
      prim::planar::RectangleInt PageVisibility;
      prim::planar::RectangleInt PageArea;

      public:
      
      Properties() : GraphicsContext(0), ComponentContext(0), IndexOfCanvas(-1)
      {
      }

      friend struct JUCE;
    };
    
    class Image : public Image
    {
      friend struct JUCE;
      
      const juce::Image& Handle;
      
      public:
      
      ///Creates an image given a resource ID and a JUCE image reference.
      Image(Resource& ResourceID, const juce::Image& Handle) : 
        Image(ResourceID), Handle(Handle) {}

      ///Virtual destructor
      virtual ~Image() {}
      
      ///Returns the image size.
      prim::planar::VectorInt GetSize() const
      {
        return prim::planar::VectorInt(Handle.getWidth(), Handle.getHeight());
      }
    };
    
  public:

    ///Contains JUCE-specific pointers to the Graphics and Component objects.
    Properties* JUCEProperties;
    
    ///Saves the portfolio pointer during painting so draw calls can access it.
    Portfolio* CachedPortfolio;

    ///Constructor initializes the JUCE renderer.
    JUCE() : JUCEProperties(0), CachedPortfolio(0) {}

    ///Calls the paint event of the current canvas being painted.
    void Paint(Portfolio* PortfolioToPaint,
      Painter::Properties* PortfolioProperties)
    {
      //Cache the portfolio.
      CachedPortfolio = PortfolioToPaint;
      
      //Get a pointer to the properties.
      JUCEProperties = PortfolioProperties->Interface<Properties>();

      //Set the current page number.
      SetPageNumber(JUCEProperties->IndexOfCanvas);
      
      //Paint the current canvas.
      PortfolioToPaint->Canvases[JUCEProperties->IndexOfCanvas]->Paint(*this,
        *PortfolioToPaint);
      
      //Reset the page number to indicate painting is finished.
      ResetPageNumber();

      //Set the properties pointer back to null to be safe.
      JUCEProperties = 0;
      
      //Set the cached portfolio pointer back to null.
      CachedPortfolio = 0;
    }
    
    private:
    
    juce::AffineTransform GetTransform(const Affine& a = Affine::Unit())
    {
      //Determine dimensions of the current canvas and the appropriate scale.
      prim::planar::Vector PageDimensions = JUCEProperties->PageDimensions;
      prim::number ScaleToFitPage =
        (prim::number)JUCEProperties->PageArea.Width() / PageDimensions.x;

      //Calculate the affine transform so that the image is scaled to the page.
      Affine m = Spaces.Forwards();
      m *= a;
      
      //Convert transform to JUCE.
      juce::AffineTransform jat(
        (float)m.a, (float)m.c, (float)m.e,
        (float)m.b, (float)m.d, (float)m.f);
      
      //Transform to bottom-left origin space.
      jat = jat.translated((float)0, (float)-PageDimensions.y);
      jat = jat.scaled((float)ScaleToFitPage, (float)-ScaleToFitPage);
      jat = jat.translated((float)JUCEProperties->PageArea.a.x,
        (float)JUCEProperties->ComponentContext->getHeight() - 
        (float)JUCEProperties->PageArea.b.y);
      
      return jat;
    }
    
    ///Determines whether an object given a rectangle bound needs painting.
    bool IsInsideComponent(prim::planar::Rectangle ObjectBounds,
      const juce::AffineTransform& a)
    {
      //Get the transform of the viewport.
      Affine ViewportTransform(a.mat00, a.mat10, a.mat01, a.mat11, a.mat02,
        a.mat12);
      
      //Get the rectangle of the screen.
      prim::planar::Rectangle Viewport(prim::planar::Vector(),
        prim::planar::Vector(JUCEProperties->ComponentContext->getWidth(),
        JUCEProperties->ComponentContext->getHeight()));
      
      //Transform the object coordinates into the viewport.
      ObjectBounds.a = ViewportTransform << ObjectBounds.a;
      ObjectBounds.b = ViewportTransform << ObjectBounds.b;
      
      //Return whether the object is inside the component.
      return !(ObjectBounds - Viewport).IsEmpty();
    }
    
    ///Determines whether this painter is painting now.
    bool IsInPaintEvent()
    {
      return JUCEProperties && CachedPortfolio;
    }
    
    public:
    
    ///Draws an image.
    void Draw(const Resource& ResourceID, prim::planar::Vector Size)
    {
      //Make sure that this method was called inside a valid paint event.
      if(!IsInPaintEvent()) return;
      
      //Look up the image resource.
      const JUCE::Image* ImageResource =
        CachedPortfolio->FindImage<JUCE::Image>(ResourceID);

      //If a JUCE-context version of the image could not be found, then return.
      if(!ImageResource)
        return;
      
      //Get the dimensions.
      prim::planar::VectorInt Pixels = ImageResource->GetSize();
      
      //Make sure the image has dimension.
      if(!Pixels.x || !Pixels.y)
        return;
      
      //Get the scale of the pixels.
      prim::planar::Vector Scale(Size.x / Pixels.x, -Size.y / Pixels.y);
      
      //Get the current transform.
      Affine a = Affine::Translate(prim::planar::Vector(0, Size.y)) *
        Affine::Scale(Scale);
      juce::AffineTransform ToViewport = GetTransform(a);
      juce::AffineTransform ViewportBounds = GetTransform();
        
      //Optimization: do not draw if image is outside the view.
      if(!IsInsideComponent(prim::planar::Rectangle(
        prim::planar::Vector(), Size), ViewportBounds))
          return;
      
      //Get the JUCE graphics context.
      juce::Graphics* g = JUCEProperties->GraphicsContext;

      //Draw the image.
      g->drawImageTransformed(ImageResource->Handle, ToViewport);
    }
    
    ///Draws a path to the renderer JUCE component.
    void Draw(const Path& p, const Affine& a)
    {
      //Make sure that this method was called inside a valid paint event.
      if(!IsInPaintEvent()) return;

      //Get the current transform.
      juce::AffineTransform ToViewport = GetTransform(a);
        
      //Optimization: do not draw if path is outside the view.
      if(!IsInsideComponent(p.Bounds(), ToViewport))
        return;
      
      //Get the JUCE graphics context.
      juce::Graphics* g = JUCEProperties->GraphicsContext;

      //Fill path if necessary.
      if(State.FillColor.A > 0.f)
      {
        //Set the fill color.
        Color fc = State.FillColor;
        g->setColour(juce::Colour((prim::uint8)(fc.R * 255.f),
          (prim::uint8)(fc.G * 255.f), (prim::uint8)(fc.B * 255.f), fc.A));

        //Fill the path.
        g->fillPath(p, ToViewport);
      }

      //Stroke path if necessary.
      if(State.StrokeColor.A > 0.f)
      {
        //Set the stroke color.
        Color sc = State.StrokeColor;
        g->setColour(juce::Colour((prim::uint8)(sc.R * 255.f),
          (prim::uint8)(sc.G * 255.f), (prim::uint8)(sc.B * 255.f), sc.A));

        //Determine the affine transform scaled stroke width.
        prim::number ScaledStrokeWidth = State.StrokeWidth *
          (prim::number)prim::planar::Vector(ToViewport.mat00,
          ToViewport.mat01).Mag() / prim::Sqrt(2.0);
        
        //Stroke the path.
        g->strokePath(p, juce::PathStrokeType((float)ScaledStrokeWidth), 
          ToViewport);
      }
    }
  };
#endif
}}
#endif
