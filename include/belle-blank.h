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

#ifndef BELLE_PAINTERS_BLANK_H
#define BELLE_PAINTERS_BLANK_H

namespace BELLE_NAMESPACE
{
  /**Special painter that paints a portfolio to nowhere. It nonetheless calls
  the paint operations of the canvases mimicking the effect of the paint on the
  portfolio, if it is side-effected. This painter does not need a properties
  object and the corresponding call to Create can supply a null properties
  pointer.*/
  class Blank : public Painter
  {
    public:

    Blank() {}

    virtual ~Blank();

    ///Calls the paint event of the current canvas being painted.
    void Paint(Portfolio* PortfolioToPaint,
      Painter::Properties* PortfolioProperties)
    {
      (void)PortfolioProperties;
      //Go through each canvas and paint it.
      for(count i = 0; i < PortfolioToPaint->Canvases.n(); i++)
      {
        //Set the current page number.
        SetPageNumber(i);

        //Paint the current canvas.
        PortfolioToPaint->Canvases[i]->Paint(*this, *PortfolioToPaint);

        //Reset the page number to indicate painting is finished.
        ResetPageNumber();
      }
    }

    ///Draws a path to the current SVG string.
    void Draw(const Path& p, const Affine& a)
    {
      (void)p;
      (void)a;
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
