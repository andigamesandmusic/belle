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

//------------------------------------------------------------------------------

/*
Tutorial 2: Understanding vector space and affine transformations in Belle

This tutorial explains how affine transformations such as translation, scaling,
and rotation work in Belle and discusses strategies for placing graphical
objects.

On Mac/Linux you can build and run from the terminal using:
Scripts/MakeAndRun Tutorial2

For more information related to building, see the README.
*/

//------------------------------------------------------------------------------

/*Include Belle, Bonne, Sage and compile it in this .cpp file. See the previous
tutorials for an explanation.*/
#define BELLE_COMPILE_INLINE
#include "Belle.h"
using namespace prim;
using namespace prim::planar;
using namespace belle;
using namespace belle::painters;

//------------------------------------------------------------------------------

/*
In Belle, the default vector graphics space for a canvas is inches with the
origin at the bottom-left corner. This means the canvas exists in quadrant one
space (positive x and positive y).

However, there are many reasons to alter this space--to essentially place
objects relative to a sequence of transformations. In Belle this is accomplished
through affine transformations using the Affine object. However, in most
circumstances, you can call one of the pre-written transformation functions on
the Painter:

void Translate(Vector Displacement);
void Scale(number ScalingFactor);
void Scale(Vector ScalingFactor);
void Rotate(number Radians);

Or you can pass an Affine object directly to Transform():
void Transform(const Affine& AffineSpace);

It is important to understand what is being transformed. In Belle, the
transformations are applied to the path space as they are drawn, as opposed to
the canvas space. In other words, the objects drawn to the page are transformed
rather than the page itself, though in principle either approach can be used to
produce identical results (mathematically, one involves a matrix multiplication,
the other a matrix division).

Internally, Belle uses a transformation stack so that you can temporarily
transform to a new space and then go back to the space you were in.

When you are finished with a space, you must call Revert() to pop the space off
of the internal stack. If you do not do this, the result is undetermined and
you will get a warning message on the Console.

Revert() can also be called with a number to pop multiple spaces off the stack.
For example, Revert(3) would undo three prior transformation calls.

Belle also has a units system to convert between physical unit spaces. The
Vector has been subclassed into Inches, Points, Millimeters, Centimeters, etc.
You can use these interchangeably and Belle will do the conversion for you.
For example:
Millimeters x = Inches(1, 1); //x is (25.4, 25.4)

It should be noted that affine space is unitless. No unit conversions are done
in affine space, and the unit is discarded if it is present. For example, the
following two calls do the same thing:
Painter.Translate(Inches(1, 1)); //This doesn't do what you might think.
Painter.Translate(Centimeters(1, 1));

Translate takes a Vector, and that Vector is interpreted according to the
current space since affine space is relative not physical. Inches, Centimeters,
etc. convert to the base class, and so both lines of code above are equivalent
to:
Painter.Translate(Vector(1, 1));

However, this does not mean that units can not be used with affine space. See
the code to page 10 below for an example of using different units.

The Dimensions member of the Canvas is however unit-based because it refers to
a physical size:
//Create letter sized page.
Dimensions = Inches(8.5, 11.0);

//Create A4 sized page (converts to Inches since Dimensions is Inches)
Dimensions = Millimeters(210.0, 297.0);

Note you can alternatively use the Paper class which has the sizes for common
paper types built in.
Dimensions = Paper::Letter;
Dimensions = Paper::A4;
*/

struct Score : public Portfolio
{
  struct Page : public Canvas
  {
    void DrawShape(Painter& Painter, Color ShapeColor = Colors::red,
      Color AxisColor = Colors::black)
    {
      /*To demonstrate affine transformations it helps to show a coordinate
      axis. The Shapes class has a built-in path-maker for an axis with ticks.*/
      Path Axis, Shape;
      Shapes::AddCoordinateAxis(Axis);
      
      /*Show a simple rectangle using filled (not stroked) lines. This allows
      us to use a fill operation instead of a stroke operation and it also
      provides the outline with rounded corners.*/
      Shapes::AddRectangleFromLines(Shape,
        Rectangle(Vector(0, 0), Vector(1, 1)), 0.05);
      
      //Draw the axis using the color for the axis.
      Painter.SetFill(AxisColor);
      Painter.Draw(Axis);
      
      //Draw the shape on top of the axis using the color for the shape.
      Painter.SetFill(ShapeColor);
      Painter.Draw(Shape);
    }
    
    void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      /*For this example, we'll move the origin to the center to make things
      a little easier to see. The most important thing to know is that by
      default Belle uses the bottom-left corner of the canvas as the origin and
      not the top-left corner (with reversed y) as many graphics libraries do. 
      While this is a bit unconventional, it allows for the x and y dimensions
      to be treated the same and makes for clearer code.*/
      Painter.Translate(Dimensions / 2.0);
      
      //Draw a silhouette of the untransformed shape.
      DrawShape(Painter, Colors::gray, Colors::lightgray);
      
      //For each page show a different example of using affine transformations.
      switch(Painter.GetPageNumber())
      {
        case 0: //Page 1
        //Just show the gray coordinate axis...
        break;
        
        case 1: //Page 2
        //Show a translation over 1.3 and up 1.8.
        Painter.Translate(Vector(1.3, 1.8));
        DrawShape(Painter);
        Painter.Revert();
        break;
        
        case 2: //Page 3
        //Show a rotation of 30 degrees.
        Painter.Rotate(30.0 * Deg); /*(Deg is just a unit that converts degrees
        to radians when multiplying and radians to degress when dividing).*/
        DrawShape(Painter);
        Painter.Revert();
        break;

        case 3: //Page 4
        //Show a scaling of 1.5.
        Painter.Scale(1.5);
        DrawShape(Painter);
        Painter.Revert();
        break;
        
        case 4: //Page 5
        /*Scaling and translating is not the same as translating and scaling.
        This is related to the fact that matrix multiplication is not generally
        commutative.*/
        Painter.Translate(Vector(1, 1)); //Translate-scale
        Painter.Scale(2.0);
        DrawShape(Painter, Colors::green);
        Painter.Revert(2); /*(Revert defaults to undoing one transformation, but
          you can specify any number of previous transformations to revert at
          once.)*/
        
        Painter.Scale(2.0); //Scale-translate
        Painter.Translate(Vector(1, 1));
        DrawShape(Painter, Colors::red);
        Painter.Revert(2);
        break;
        
        case 5: //Page 6
        /*For the same underlying reason, rotating and translating is not the
        same as translating and rotating.*/
        Painter.Translate(Vector(1, 1)); //Translate-rotate
        Painter.Rotate(30.0 * Deg);
        DrawShape(Painter, Colors::green);
        Painter.Revert(2);
        
        Painter.Rotate(30.0 * Deg); //Rotate-translate
        Painter.Translate(Vector(1, 1));
        DrawShape(Painter, Colors::red);
        Painter.Revert(2);
        break;
        
        case 6: //Page 7
        //However, scaling and rotation happen to be commutative.
        Painter.Scale(2.0); //Scale-rotate
        Painter.Rotate(30.0 * Deg);
        DrawShape(Painter, Colors::green);
        Painter.Revert(2);
        
        Painter.Rotate(30.0 * Deg); //Rotate-scale
        Painter.Scale(2.0);
        DrawShape(Painter, Colors::green);
        Painter.Revert(2);
        break;
        
        case 7: //Page 8
        /*Occasionally, one may find a need to scale by different amounts in the
        x- and y- dimensions. This is typically done to create a mirror image.*/
        Painter.Scale(Vector(-1.0, 1.0)); //Horizontal mirror
        DrawShape(Painter, Colors::lightgreen);
        Painter.Revert();

        DrawShape(Painter, Colors::green); //Original
        break;
        
        case 8: //Page 9
        {
        /*You can also create an affine transformation using the Affine object,
        and call Transform with the object.
        
        The TranslateScaleRotate method on Affine can be used to position an
        object at a given size and angle. It is equivalent to multiplying by
        a Translate, Scale, and Rotate in that order (though the scale and
        rotation order could be flipped per the result shown on page 7).*/
        Affine a = Affine::TranslateScaleRotate(Vector(1, 1), 2.0, 30.0 * Deg);
        Painter.Transform(a);
        DrawShape(Painter, Colors::green);
        Painter.Revert();
        
        Affine b = (Affine::Translate(Vector(1, 1)) * Affine::Scale(2.0)) *
          Affine::Rotate(30.0 * Deg);
        Painter.Transform(b);
        DrawShape(Painter, Colors::green);
        Painter.Revert();
        
        Painter.Translate(Vector(1, 1));
        Painter.Scale(2.0);
        Painter.Rotate(30.0 * Deg);
        DrawShape(Painter, Colors::green);
        Painter.Revert(3);
        }
        break;
        
        case 9: //Page 10
        /*You can easily change units from the default inches to another unit.
        The following creates a horizontal unit-sized vector in Centimeters and
        converts that to Inches. The x-component is thus the relative scale.*/
        Painter.Scale(Inches(Centimeters(1.0, 0.0)).x);
        DrawShape(Painter);
        Painter.Revert();
        break;
      }
      
      Painter.Revert(); //Revert the page centering transformation.
    }
  };
};

int main()
{
  //Instantiate a score.
  Score MyScore;

  //Add several pages to the score to show different affine transformations.
  for(count i = 0; i < 10; i++)
  {
    MyScore.Canvases.Add() = new Score::Page;
    MyScore.Canvases.z()->Dimensions = Paper::Portrait(Paper::Letter);
  }
  
  /*Write the score out to PDF. See Tutorial 1 for an explanation of how this
  works.*/
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "Tutorial2.pdf";
  MyScore.Create<PDF>(PDFSpecificProperties);
  c >> "Wrote PDF to '" << PDFSpecificProperties.Filename << "'.";

  //Finish the console output.
  c.Finish();
  
  return 0;
}
