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

#ifndef BELLE_PAINTERS_SVG_H
#define BELLE_PAINTERS_SVG_H

namespace BELLE_NAMESPACE
{
  class SVG : public Painter
  {
    public:

    struct Properties : public Painter::Properties
    {
      ///Represents an array of output SVG files.
      List<String> Output;

      /**Stem of filename(s) to write out to. If there is more than one page,
      then the number of the page will be appended and .svg. If there is just
      one page, then only .svg will be appended.*/
      String FilenameStem;

      public:

      ///Virtual destructor
      virtual ~Properties();

      friend class SVG;
    };

    private:

    ///Contains SVG-specific pointers.
    Properties* SVGProperties;

    ///Saves the portfolio pointer during painting so draw calls can access it.
    Portfolio* CachedPortfolio;

    ///String containing the current SVG page.
    String CurrentSVGPage;

    ///Dimensions of the current SVG page.
    Inches CurrentSize;

    ///Initializes the SVG page with header information.
    void InitializeSVGPage(Inches Size)
    {
      //Write the header information
      String SVG;
      SVG >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
      SVG >> "<!--Created with Belle "
        "(belle::painters::SVG)-->";
      SVG++;
      SVG >> "<svg ";
      SVG >> "  xmlns=\"http://www.w3.org/2000/svg\"";
      SVG >> "  version=\"1.1\"";
      SVG >> "  width=\"" << Size.x << "in\"";
      SVG >> "  height=\"" << Size.y << "in\"";
      SVG >> "  viewBox=\"0 0 " << Size.x << " " << Size.y << "\"";
      SVG >> ">";
      SVG++;
      SVG >> "<!--Path data for each glyph-->";
      CurrentSVGPage = SVG;
      CurrentSize = Size;
    }

    ///Finalizes the SVG page by closing the svg tag.
    void FinalizeSVGPage()
    {
      CurrentSVGPage >> "</svg>";
    }

    public:

    ///Constructor initializes the JUCE renderer.
    SVG() : SVGProperties(0), CachedPortfolio(0) {}

    ///Virtual destructor
    virtual ~SVG();

    ///Calls the paint event of the current canvas being painted.
    void Paint(Portfolio* PortfolioToPaint,
      Painter::Properties* PortfolioProperties)
    {
      //Cache the portfolio.
      CachedPortfolio = PortfolioToPaint;

      //Get a pointer to the properties.
      SVGProperties = PortfolioProperties->Interface<Properties>();

      //Clear the output.
      SVGProperties->Output.RemoveAll();

      //Go through each canvas and paint it to an SVG string.
      for(count i = 0; i < CachedPortfolio->Canvases.n(); i++)
      {
        //Set the current page number.
        SetPageNumber(i);

        //Initializes the SVG header.
        InitializeSVGPage(CachedPortfolio->Canvases[i]->Dimensions);

        //Paint the current canvas.
        CachedPortfolio->Canvases[i]->Paint(*this, *CachedPortfolio);

        //Finalizes the SVG header.
        FinalizeSVGPage();

        //Add the SVG page to the output.
        SVGProperties->Output.Add() = CurrentSVGPage;

        //Reset the page number to indicate painting is finished.
        ResetPageNumber();
      }

      //Write out to file if a filename stem was provided.
      for(count i = 0; i < SVGProperties->Output.n(); i++)
      {
        String Filename = SVGProperties->FilenameStem;
        if(!Filename)
          break;

        if(SVGProperties->Output.n() > 1)
          Filename << (i + 1);
        Filename << ".svg";
        File::Write(Filename, SVGProperties->Output[i]);
      }

      //Set the properties pointer back to null to be safe.
      SVGProperties = 0;

      //Set the cached portfolio pointer back to null.
      CachedPortfolio = 0;
    }

    ///Draws a path to the current SVG string.
    void Draw(const Path& p, const Affine& a)
    {
      Transform(a);
      Affine B = Affine::Scale(Vector(1.f, -1.f)) *
        Affine::Translate(Vector(0, -CurrentSize.y));
      Affine A = B * CurrentSpace();

      String SVG;
      SVG >> "<path";
      SVG << " d=\"";
      for(count j = 0; j < p.n(); j++)
      {
        const Instruction& i = p[j];
        Vector c1 = i.Control1(), c2 = i.Control2(), e = i.End();
        c1 = A << c1;
        c2 = A << c2;
        e = A << e;

        if(i.IsMove())
          SVG << " M " << e.x << " " << e.y;
        else if(i.IsLine())
          SVG << " L " << e.x << " " << e.y;
        else if(i.IsCubic())
          SVG << " C " << c1.x << " " << c1.y << " " <<
            c2.x << " " << c2.y << " " << e.x << " " << e.y;
        else
          SVG << " Z";
      }
      SVG << "\"";

      if(Limits<number>::IsNotZero(State.FillColor.A))
      {
        SVG << " fill=\"rgb(" <<
          Round(State.FillColor.R * 255.f) << ", " <<
          Round(State.FillColor.G * 255.f) << ", " <<
          Round(State.FillColor.B * 255.f) << ")\"";
      }
      else
        SVG << " fill=\"none\"";

      number ScaledStrokeWidth = State.StrokeWidth *
          Vector(A.a, A.d).Mag() / Sqrt(number(2.f));
      if(ScaledStrokeWidth > 0.f)
      {
        SVG << " style=\"stroke:rgb(" <<
          Round(State.StrokeColor.R * 255.f) << ", " <<
          Round(State.StrokeColor.G * 255.f) << ", " <<
          Round(State.StrokeColor.B * 255.f) << "); "
          "stroke-width: " << ScaledStrokeWidth << "\"";
      }
      else
        SVG << " style=\"stroke:none; stroke-width:0\"";

      SVG << "/>";
      CurrentSVGPage >> SVG;
      Revert();
    }

    ///Drawing image resources is not supported in the SVG implementation.
    void Draw(const Resource& ResourceID, Vector Size)
    {
      (void)ResourceID;
      (void)Size;
    }
  };
}
#endif
