/*
  ==============================================================================

  Copyright 2007-2013, 2017 Andi
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

#ifndef BELLE_CORE_ABSTRACTS_H
#define BELLE_CORE_ABSTRACTS_H

#include "belle-colors.h"
#include "belle-brush.h"
#include "belle-font.h"
#include "belle-transform.h"
#include "belle-path.h"
#include "belle-units.h"
#include "belle-text.h"

namespace BELLE_NAMESPACE
{
  //Forward declarations
  class Portfolio;
  class Canvas;

  /**Represents a context-independent resource. This class contains a unique
  identifier which is used to communicate to the painter across device-dependent
  boundaries what object to paint.*/
  class Resource
  {
    ///Stores the unique identifier associated with this resource.
    UUIDv4 ID;

    public:

    ///Constructs the resource with a random ID.
    Resource() {}

    ///Constructs the resource with a preexisting ID.
    Resource(UUIDv4 ExistingID) : ID(ExistingID) {}

    ///Copy constructor
    Resource(const Resource& Other) : ID(Other.ID) {}

    ///Assignment operator
    Resource& operator = (const Resource& Other) {ID = Other.ID; return *this;}

    ///Equals operator
    bool operator == (const Resource& Other) const {return ID == Other.ID;}

    ///Not equals operator
    bool operator != (const Resource& Other) const {return ID != Other.ID;}

    ///Returns the ID that was generated for this Resource.
    UUIDv4 GetID() const {return ID;}
  };

  ///Base class for a raster image, subclassed by each context.
  class Image : public Resource
  {
    public:

    ///Initializes the image with a given resource ID.
    Image(const Resource& ResourceID) : Resource(ResourceID) {}

    ///Virtual destructor.
    virtual ~Image();

    ///Returns the size of the image in pixels.
    virtual VectorInt GetSize() const = 0;
  };

  class Painter
  {
    //------------//
    //Page Numbers//
    //------------//
    protected:

    ///Stores the page number.
    count PageNumber;

    ///Allows a derived painter to set a current page number if relevant.
    void SetPageNumber(count PageNumber_)
    {
      PageNumber = PageNumber_;
    }

    ///Resets the page number.
    void ResetPageNumber()
    {
      PageNumber = -1;
    }

    public:

    /**Returns the current page number being drawn if relevant to the painter.
    Note that the page number is zero-based. A value of -1 indicates that no
    page is currently being painted. For painters which do not have pages (for
    example, image painters) this method will return 0 when the painter is
    painting.*/
    count GetPageNumber() const
    {
      return PageNumber;
    }

    /**Returns whether the painter is currently painting. To get the current
    page being painted, call GetPageNumber().*/
    bool IsPainting() const
    {
      return PageNumber != -1;
    }

    public:

    /**A painter may have properties associated with it. These comprise details
    that might be useful for the Painter device to immediately know, such as the
    name of a file, or a pointer to a native graphics context.*/
    class Properties
    {
      public:

      ///Virtual destructor.
      virtual ~Properties();

      ///Shortcut to return a pointer to the derived class of this object.
      template <typename DerivedProperties>
      DerivedProperties* Interface()
      {
        return dynamic_cast<DerivedProperties*>(this);
      }
    };

    ///Shortcut to return a pointer to the derived class of this object.
    template <typename SpecificPainter> SpecificPainter* Interface()
    {
      return dynamic_cast<SpecificPainter*>(this);
    }

    protected:

    /**Overloaded by a specific painter. When its own initialization and set up
    has finished, it needs to call the Canvas::Paint() of each canvas in the
    portfolio.*/
    virtual void Paint(Portfolio* PortfolioToPaint,
      Properties* PortfolioProperties) = 0;

    //Portfolio needs to call the Paint() method.
    friend class Portfolio;

    protected:

    ///The current brush
    Brush State;

    ///Cached current state
    Affine CurrentState;

    public:

    /**Sets a new brush. This method can be overriden by a specific
    painter to provide additional behavior.*/
    virtual void SetBrush(const Brush& NewState)
    {
      State = NewState;
    }

    ///Sets the current draw mode to fill without stroke.
    void SetFill(Color FillColor)
    {
      Brush s;
      s.FillColor = FillColor;
      SetBrush(s);
    }

    /**Sets the current draw mode to stroke without filling. The stroke width
    should be greater than 0.*/
    void SetStroke(Color StrokeColor, number StrokeWidth)
    {
      Brush s;
      s.StrokeColor = StrokeColor;
      s.StrokeWidth = StrokeWidth;
      SetBrush(s);
    }

    /**Sets the current draw mode to stroke and fill. The stroke width should be
    greater than 0.*/
    void SetFillAndStroke(Color FillColor, Color StrokeColor,
      number StrokeWidth)
    {
      Brush s;
      s.FillColor = FillColor;
      s.StrokeColor = StrokeColor;
      s.StrokeWidth = StrokeWidth;
      SetBrush(s);
    }

    //---------------------//
    //Affine Transformation//
    //---------------------//

    protected:

    AffineStack Spaces;

    void CheckStack()
    {
      if(Spaces.n() != 1)
      {
        C::Error() >>
          "Warning: transformation stack incorrectly collapsed." >>
          "Did you use Revert() for each transformation?";
      }
    }

    public:

    //Default constructor
    Painter() {ResetPageNumber();}

    ///Virtual destructor makes sure transformation stack has been collapsed.
    virtual ~Painter();

    /**Performs an arbitrary affine transformation. It must be undone with the
    Revert() method. The transformation is applied in object space (as opposed
    to camera or page space.) This method can be overriden by the subclass to
    provide specific behavior.*/
    virtual void Transform(const Affine& a)
    {
      Spaces.Push(a);
      CurrentState = Spaces.Forwards();
    }

    /**Reverts any of the above transforms. It is necessary to revert any affine
    transformation when it is no longer necessary. You can revert multiple
    transforms at once by specifying a how many transformations to revert. This
    method can be overriden by the subclass to provide specific behavior.*/
    virtual void Revert(count TransformationsToRevert = 1)
    {
      for(count i = 0; i < TransformationsToRevert; i++)
        Spaces.Pop();
      CurrentState = Spaces.Forwards();
    }

    ///Returns the current affine space.
    Affine CurrentSpace()
    {
      return CurrentState;
    }

    /**Performs a translation affine transformation. It must be undone with the
    Revert() method. The transformation is applied in object space (as opposed
    to camera or page space.)*/
    void Translate(Vector TranslateBy)
    {
      Transform(Affine::Translate(TranslateBy));
    }

    /**Performs a scaling affine transformation. It must be undone with the
    Revert() method. The transformation is applied in object space (as opposed
    to camera or page space.)*/
    void Scale(number ScaleBy)
    {
      Transform(Affine::Scale(ScaleBy));
    }

    /**Performs a vector scaling affine transformation. It must be undone with
    the Revert() method. The transformation is applied in object space (as
    opposed to camera or page space.)*/
    void Scale(Vector ScaleBy)
    {
      Transform(Affine::Scale(ScaleBy));
    }

    /**Performs a rotating affine transformation. It must be undone with the
    Revert() method. The transformation is applied in object space (as opposed
    to camera or page space.)*/
    void Rotate(number RotateBy)
    {
      Transform(Affine::Rotate(RotateBy));
    }

    ///Returns the affine stack of the painter.
    const AffineStack& Space()
    {
      return Spaces;
    }

    //-----//
    //Paths//
    //-----//

    /**Draws the path with an optional transform related to the current space.
    The current brush determines how it will be stroked or filled. This method
    should be overloaded by a specific painter to perform a device specific
    rendering.*/
    virtual void Draw(const Path& p, const Affine& a = Affine::Unit()) = 0;

    /**Draws a path by pointer instead of reference. If the pointer is null,
    then nothing is drawn.*/
    void Draw(Pointer<const Path> p, const Affine& a = Affine::Unit())
    {
      if(!p) return;
      Draw(*p, a);
    }

    //------//
    //Images//
    //------//

    ///Draws an image.
    virtual void Draw(const Resource& ResourceID, Vector Size) = 0;

    //----//
    //Text//
    //----//

    ///Draws text to a path.
    static void Draw(Text& t, Path& p)
    {
      Array<Word>& Words = t.GetWords();
      for(count i = 0; i < Words.n(); i++)
      {
        Word& w = Words[i];
        for(count j = 0; j < w.n(); j++)
        {
          Character& c = w[j];
          if(Pointer<const Glyph> g = c.TypesetGlyph)
          {
            Affine a = Affine::Translate(c.TypesetPosition) *
              Affine::Scale(c.PointSize / 72.f);
            p.Append(*g, a);
          }
        }
      }
    }

    ///Draws text that has been typeset.
    void Draw(Text& t, const Affine& a = Affine::Unit())
    {
      Transform(a);
      Array<Word>& Words = t.GetWords();
      for(count i = 0; i < Words.n(); i++)
      {
        Word& w = Words[i];
        for(count j = 0; j < w.n(); j++)
        {
          Character& c = w[j];
          if(Pointer<const Glyph> g = c.TypesetGlyph)
          {
            Translate(c.TypesetPosition);
            Scale(c.PointSize / 72.f);
            SetFill(c.FaceColor);
            Draw(*g);
            Revert(2);
          }
        }
      }
      Revert();
    }

    ///Typeset and draw text.
    void Draw(String TextToDraw, const Font& FontToUse,
      number PointSize = 12.f,
      Font::Style Style = Font::Regular,
      Text::Justification Justify = Text::Justifications::Left,
      number LineWidth = 0.f,
      Color ColorToFill = Colors::Black(),
      const Affine& a = Affine::Unit())
    {
      //Make a long line if no line width is provided.
      if(Limits<number>::IsZero(LineWidth))
        LineWidth = 10.f;
      Text t(FontToUse, Style, PointSize, LineWidth, Justify, ColorToFill);

      //Import the string.
      t.ImportStringToWords(TextToDraw);

      //Set the styling for each character.
      for(count i = 0; i < t.Words.n(); i++)
      {
        Word& w = t.Words[i];
        for(count j = 0; j < w.n(); j++)
        {
          Character& l = w[j];
          l.PointSize = PointSize;
          l.FaceColor = ColorToFill;
          l.Style = Style;
        }
      }

      //Determine the line breaks.
      t.DetermineLineBreaks();

      //Typeset the text.
      t.Typeset();

      //Draw the text.
      Draw(t, a);
    }

    ///Typeset and draw text to path.
    static void Draw(Path& p, String TextToDraw, const Font& FontToUse,
      number PointSize = 12.f,
      Font::Style Style = Font::Regular,
      Text::Justification Justify = Text::Justifications::Left,
      number LineWidth = 0.f)
    {
      //Make a long line if no line width is provided.
      if(Limits<number>::IsZero(LineWidth))
        LineWidth = 10.f;
      Text t(FontToUse, Style, PointSize, LineWidth, Justify);

      //Import the string.
      t.ImportStringToWords(TextToDraw);

      //Set the styling for each character.
      for(count i = 0; i < t.Words.n(); i++)
      {
        Word& w = t.Words[i];
        for(count j = 0; j < w.n(); j++)
        {
          Character& l = w[j];
          l.PointSize = PointSize;
          l.Style = Style;
        }
      }

      //Determine the line breaks.
      t.DetermineLineBreaks();

      //Typeset the text.
      t.Typeset();

      //Draw the text.
      Draw(t, p);
    }
  };

  /**A collection of canvases for example a score or a book. A portfolio does
  not have any requirement that the canvases be of the same dimension, but the
  canvases are ordered. To use a portfolio, incorporate it as a base class to
  a subclass.*/
  class Portfolio
  {
    public:

    ///A list of canvases which this portfolio comprises.
    List<Pointer<Canvas> > Canvases;

    /**Causes the portfolio to be rendered using the given painter type. If the
    painter has properties, then a pointer to a properties object should be
    passed.*/
    template <typename PainterType>
    void Create(Painter::Properties& Properties)
    {
      Pointer<PainterType> SpecificPainter = new PainterType;
      SpecificPainter->Paint(this, &Properties);
    }

    ///Virtual destructor
    virtual ~Portfolio();

    //------//
    //Images//
    //------//

    protected:

    ///Array of registered images.
    Array<const Image*> Images;

    public:

    ///Adds a context-dependent image to the resource list.
    void AddImageResource(const Image& i)
    {
      Images.Add() = &i;
    }

    ///Clears the image resource list.
    void ClearImageResources()
    {
      Images.Clear();
    }

    ///Finds an image of a context-dependent type from its resoure ID.
    template <class T> const T* FindImage(const Resource& ResourceID)
    {
      //Find the image resource in the context-dependent type.
      for(count i = 0; i < Images.n(); i++)
        if(Images[i]->GetID() == ResourceID.GetID())
          if(const T* x = dynamic_cast<const T*>(Images[i]))
            return x;

      //Image could not be found (context-dependent type does not exist for it).
      return 0;
    }
  };

  class Canvas
  {
    public:

    ///Dimensions of the canvas stored in inches.
    Inches Dimensions;

    ///Sets the canvas size to portrait US Letter.
    Canvas() : Dimensions(Paper::Portrait(Paper::Letter())) {}

    ///Virtual destructor so that memory is freed correctly.
    virtual ~Canvas();

    ///Shortcut to return a pointer to the derived class of this object.
    template <typename DerivedCanvas>
    DerivedCanvas* Interface()
    {
      return dynamic_cast<DerivedCanvas*>(this);
    }

    /**The entry point for painting within a canvas. Subclasses will override
    this to send draw commands to the painter. This method should not be called
    directly, rather it is called by a specific painter when the painter is
    ready to paint.*/
    virtual void Paint(Painter& Painter, Portfolio& Portfolio) = 0;
  };

  ///Handles the Painter.Revert() automatically when object goes out of scope.
  class ScopedAffine
  {
    Painter& p;

    public:
    ScopedAffine(Painter& p_, Affine a_) : p(p_) {p.Transform(a_);}
    ~ScopedAffine() {p.Revert();}
  };
}
#endif
