/*Include Belle and compile it in this .cpp file. See the previous
tutorials for an explanation.*/
#define PRIM_WITH_TIMER
#define BELLE_COMPILE_INLINE
#include "belle.h"
#include "resources.h"
#include "belle-helper.h"
using namespace belle;

//An example of deriving a Portfolio and Canvas as Score and Score::Page
struct BlankPageScore : public Portfolio
{
  struct Page : public Canvas
  {
    virtual void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      (void)Painter;
      (void)Portfolio;
    }

    virtual ~Page();
  };
  virtual ~BlankPageScore();
};
BlankPageScore::~BlankPageScore() {}
BlankPageScore::Page::~Page() {}

static int SnippetBlankPage()
{
  //Create a score.
  BlankPageScore MyScore;

  //Add a page to the score.
  MyScore.Canvases.Add() = new BlankPageScore::Page;

  //Set to letter landscape.
  MyScore.Canvases.z()->Dimensions = Paper::Letter();

  //Set the PDF-specific properties.
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "snippet-blank-page.pdf";

  //Write the score to PDF.
  MyScore.Create<PDF>(PDFSpecificProperties);

  //Note success to console window.
  C::Out() >> "Successfully wrote '" << PDFSpecificProperties.Filename << "'.";

  return AutoRelease<Console>();
}
/*An example of deriving a Portfolio and Canvas as ColorScore and
ColorScore::Page.*/
struct ColorScore : public Portfolio
{
  struct Page : public Canvas
  {
    virtual void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      (void)Portfolio;

      //Create a gradient of tiles.
      for(number i = 0.; i < 8.; i += 0.125f)
      {
        for(number j = 0.; j < 8.; j += 0.125f)
        {
          Path p;
          Shapes::AddBox(p, Box(
            Vector(i - 0.01f, j - 0.01f),
            Vector(i + .13f, j + .13f)));
          Color cl(float32(i) / 8.f, float32(j) / 8.f, 0.f);
          Painter.SetFill(cl);
          Painter.Draw(p);
        }
      }
    }
    virtual ~Page();
  };
  virtual ~ColorScore();
};
ColorScore::~ColorScore() {}
ColorScore::Page::~Page() {}

static int SnippetColor()
{
  //Create a score.
  ColorScore MyScore;

  //Add a page to the score.
  MyScore.Canvases.Add() = new ColorScore::Page;

  //Set to letter landscape.
  MyScore.Canvases.z()->Dimensions = Inches(8.f, 8.f);

  //Set the PDF-specific properties.
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "snippet-color.pdf";

  //Write some additional metadata.
  String ExtraMetadata = "This is some metdata embedded in the PDF.";
  PDFSpecificProperties.ExtraData.CopyMemoryFrom(
    reinterpret_cast<const byte*>(ExtraMetadata.Merge()), ExtraMetadata.n());

  //Write the score to PDF.
  MyScore.Create<PDF>(PDFSpecificProperties);

  //Note success to console window.
  C::Out() >> "Successfully wrote '" << PDFSpecificProperties.Filename << "'.";

  //Read in the metadata to confirm that the file was written.
  String MyMetadata;
  PDF::RetrievePDFMetadataAsString(PDFSpecificProperties.Filename, MyMetadata);

  //Send the metadata to the console.
  C::Out() >> MyMetadata;

  return AutoRelease<Console>();
}
/*An example of deriving a Portfolio and Canvas as DrawableScore and
DrawableScore::Page.*/
struct DrawableScore : public Portfolio
{
  struct Page : public Canvas
  {
    virtual void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      (void)Portfolio;
      ScopedAffine a(Painter, Affine::Scale(Dimensions.x));
      Drawable svg(Resources::Load("ghostscript-tiger.svg"), true);
      svg.Paint(Painter);
    }

    virtual ~Page();
  };
  virtual ~DrawableScore();
};
DrawableScore::~DrawableScore() {}
DrawableScore::Page::~Page() {}

static int SnippetDrawable()
{
  //Create a score.
  DrawableScore MyScore;

  //Add a page to the score.
  MyScore.Canvases.Add() = new DrawableScore::Page;

  //Set to letter landscape.
  MyScore.Canvases.z()->Dimensions = Paper::Letter();

  //Set the PDF-specific properties.
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "snippet-drawable.pdf";

  //Write the score to PDF.
  MyScore.Create<PDF>(PDFSpecificProperties);

  //Note success to console window.
  C::Out() >> "Successfully wrote '" << PDFSpecificProperties.Filename << "'.";

  return AutoRelease<Console>();
}
static int SnippetGeometry()
{
  C::Out() >> "SnippetGeometry() no longer supported";
  return 0;
}
static int SnippetGraphSyntax()
{
  C::Out() >> "SnippetGraphSyntax() no longer supported";
  return 0;
}
//An example of deriving a Portfolio and Canvas.
struct ImageScore : public Portfolio
{
  Resource TestResource;

  struct Page : public Canvas
  {
    virtual void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      ImageScore& ImageScore = dynamic_cast<struct ImageScore&>(Portfolio);

      //Tile the test image on the page.
      for(prim::number i = 0; i <= 8; i++)
      {
        for(prim::number j = 0; j < 11; j++)
        {
          //Translate to the square at (i, j).
          Painter.Translate(Vector(i, j));

          //Paint a one-inch block using the resource key.
          Painter.Draw(ImageScore.TestResource, Vector(1.0, 1.0));

          //Revert the translation.
          Painter.Revert();
        }
      }
    }

    virtual ~Page();
  };
  virtual ~ImageScore();
};
ImageScore::~ImageScore() {}
ImageScore::Page::~Page() {}

static int SnippetImage()
{
  //Create a score.
  ImageScore MyScore;

  //Assign a JPEG file to the resource ID.
  Array<byte> Smiley;
  Resources::Load("smiley.jpg", Smiley);
  PDF::JPEGImage Test(MyScore.TestResource, Smiley);

  //Add the JPEG image resource to the score.
  MyScore.AddImageResource(Test);

  //Add a page to the score.
  MyScore.Canvases.Add() = new ImageScore::Page;

  //Set to letter landscape.
  MyScore.Canvases.z()->Dimensions = Paper::Letter();

  //Set the PDF-specific properties.
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "snippet-image.pdf";

  //Write the score to PDF.
  MyScore.Create<PDF>(PDFSpecificProperties);

  //Note success to console window.
  C::Out() >> "Successfully wrote '" << PDFSpecificProperties.Filename << "'.";

  return AutoRelease<Console>();
}
static int SnippetMIDI()
{
  C::Out() >> "SnippetMIDI() no longer supported";
  return 0;
}
//An example of deriving a Portfolio and Canvas as Score and Score::Page
struct SegmentedHullScore : public Portfolio
{
  struct Page : public Canvas
  {
    virtual void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      (void)Painter;
      (void)Portfolio;

      Array<Box> A;
      Random R(123);
      for(count i = 0; i < 200; i++)
      {
        A.Add() = Box(
          Vector(
            Truncate(R.Between(1.f, 7.5f), 0.00390625f),
            Truncate(R.Between(1.f, 10.0f), 0.00390625f)),
          Vector(
            Truncate(R.Between(1.f, 7.5f), 0.00390625f),
            Truncate(R.Between(1.f, 10.0f), 0.00390625f)));
        A.z().Order();
        Path p;
        Shapes::AddBox(p, A.z());
        Painter.SetStroke(Colors::Blue(), 0.005f);
        Painter.Draw(p);
      }
      {
        List<Vector> L = Box::SegmentedHull(A, Box::TopSide);

        for(count i = 0; i < L.n() - 1; i++)
        {
          {
            Path p;
            Shapes::AddLine(p, L[i], Vector(L[i + 1].x, L[i].y), 0.01f);
            Painter.SetFill(Colors::Red());
            Painter.Draw(p);
          }
          {
            Path p;
            Shapes::AddLine(p, Vector(L[i + 1].x, L[i].y), L[i + 1], 0.01f);
            Painter.SetFill(Colors::Green());
            Painter.Draw(p);
          }
        }
      }
      {
        List<Vector> L = Box::SegmentedHull(A, Box::BottomSide);

        for(count i = 0; i < L.n() - 1; i++)
        {
          {
            Path p;
            Shapes::AddLine(p, L[i], Vector(L[i + 1].x, L[i].y), 0.01f);
            Painter.SetFill(Colors::Red());
            Painter.Draw(p);
          }
          {
            Path p;
            Shapes::AddLine(p, Vector(L[i + 1].x, L[i].y), L[i + 1], 0.01f);
            Painter.SetFill(Colors::Green());
            Painter.Draw(p);
          }
        }
      }

      {
        List<Vector> L = Box::SegmentedHull(A, Box::LeftSide);

        for(count i = 0; i < L.n() - 1; i++)
        {
          {
            Path p;
            Shapes::AddLine(p, L[i], Vector(L[i].x, L[i + 1].y), 0.01f);
            Painter.SetFill(Colors::Magenta());
            Painter.Draw(p);
          }
          {
            Path p;
            Shapes::AddLine(p, Vector(L[i].x, L[i + 1].y), L[i + 1], 0.01f);
            Painter.SetFill(Colors::Yellow());
            Painter.Draw(p);
          }
        }
      }
      {
        List<Vector> L = Box::SegmentedHull(A, Box::RightSide);

        for(count i = 0; i < L.n() - 1; i++)
        {
          {
            Path p;
            Shapes::AddLine(p, L[i], Vector(L[i].x, L[i + 1].y), 0.01f);
            Painter.SetFill(Colors::Magenta());
            Painter.Draw(p);
          }
          {
            Path p;
            Shapes::AddLine(p, Vector(L[i].x, L[i + 1].y), L[i + 1], 0.01f);
            Painter.SetFill(Colors::Yellow());
            Painter.Draw(p);
          }
        }
      }
    }

    virtual ~Page();
  };
  virtual ~SegmentedHullScore();
};
SegmentedHullScore::~SegmentedHullScore() {}
SegmentedHullScore::Page::~Page() {}

static int SnippetSegmentedHull()
{
  //Create a score.
  SegmentedHullScore MyScore;

  //Add a page to the score.
  MyScore.Canvases.Add() = new SegmentedHullScore::Page;

  //Set to letter landscape.
  MyScore.Canvases.z()->Dimensions = Paper::Letter();

  //Set the PDF-specific properties.
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "snippet-segmented-hull.pdf";

  //Write the score to PDF.
  MyScore.Create<PDF>(PDFSpecificProperties);

  //Note success to console window.
  C::Out() >> "Successfully wrote '" << PDFSpecificProperties.Filename << "'.";

  return AutoRelease<Console>();
}
//An example of deriving a Portfolio and Canvas.
struct SpiralDemo : public Portfolio
{
  struct Page : public Canvas
  {
    virtual void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      (void)Portfolio;

      number NoteSize = 0.3f;
      number AngleIncrement = 0.8f;
      number CurrentAngle = 0;
      number NoteIncrease = 0.01f;
      number Radius = 1.0f;
      number RadiusIncrease = 0.2f;
      for(count i = 0; i < 20; i++)
      {
        Vector Position;
        Position.Polar(CurrentAngle);
        Position *= Radius;

        Path p;
        if(i % 3 == 0)
          Shapes::Music::AddHalfNote(p, Vector(0,0), NoteSize);
        else if(i % 3 == 2)
          Shapes::Music::AddQuarterNote(p, Vector(0,0), NoteSize);
        else
          Shapes::Music::AddWholeNote(p, Vector(0,0), NoteSize);

        Painter.Translate(Vector(8.5f, 11.0f) / 2.0f);

        Painter.Scale(0.8f);
        Painter.Translate(Position);

        Painter.Rotate(CurrentAngle);
        Painter.SetStroke(Colors::Black(), 0.01f);
        Painter.Draw(p);

        Painter.Revert(4);
        Radius += RadiusIncrease;
        NoteSize += NoteIncrease;
        CurrentAngle += NoteSize + AngleIncrement;
      }
    }

    virtual ~Page();
  };
  virtual ~SpiralDemo();
};
SpiralDemo::~SpiralDemo() {}
SpiralDemo::Page::~Page() {}

static int SnippetSpiral()
{
  //Create a score.
  SpiralDemo MySpiralDemo;

  //Add a page to the score.
  MySpiralDemo.Canvases.Add() = new SpiralDemo::Page;

  //Set to letter landscape.
  MySpiralDemo.Canvases.z()->Dimensions = Paper::Letter();

  //Set the PDF-specific properties.
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "snippet-spiral.pdf";

  //Write the score to PDF.
  MySpiralDemo.Create<PDF>(PDFSpecificProperties);

  //Note success to console window.
  C::Out() >> "Successfully wrote '" << PDFSpecificProperties.Filename << "'.";

  return AutoRelease<Console>();
}
//An example of deriving a Portfolio and Canvas as Score and Score::Page
struct SpringDemoScore : public Portfolio
{
  struct Page : public Canvas
  {
    virtual void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      (void)Portfolio;

      SpringSystem S;

      SpringSystem::Node B  = S.Add();
      SpringSystem::Node N1 = S.Add();
      SpringSystem::Node N2 = S.Add();
      SpringSystem::Node N3 = S.Add();
      SpringSystem::Node N4 = S.Add();
      SpringSystem::Node E  = S.Add();

      S.Connect(B,  N2)->Label.SetSpring(0,  2.f, 1.f);
      S.Connect(N2, N3)->Label.SetSpring(0,  4.f, 1.f);
      S.Connect(N3,  E)->Label.SetSpring(0,  1.f, 1.f);

      //S.Connect(B,  N2)->Label.SetSpring(1,  3.f, 1.f);
      //S.Connect(N2, N4)->Label.SetSpring(1,  2.f, 1.f);
      //S.Connect(N4,  E)->Label.SetRod(1, 1.f);
      S.Connect(B,  N2)->Label.SetRod(1, 1.f);
      S.Connect(N2, N4)->Label.SetSpring(1, SpringSystem::RodLikeCoefficient(),
        1.f);
      S.Connect(N4,  E)->Label.SetSpring(1, 7.f, 1.f);

      S.Connect(B,  N1)->Label.SetSpring(2, 10.f, 1.f);
      S.Connect(N1, N4)->Label.SetSpring(2,  1.f, 1.f);
      S.Connect(N4,  E)->Label.SetSpring(2, 10.f, 1.f);

      S.Solve(9.f);
      Array<Array<number> > s = S.Solution();

      for(count i = 0; i < s.n(); i++)
      {
        for(count j = 0; j < s[i].n(); j++)
        {
          number x = s[i][j] + 1.f;
          number y = 7.5f - i;
          Path p;
          Shapes::AddCircle(p, Vector(x, y), 0.2f);
          Painter.SetFill(Colors::Black());
          Painter.Draw(p);
        }
      }
    }

    virtual ~Page();
  };
  virtual ~SpringDemoScore();
};
SpringDemoScore::~SpringDemoScore() {}
SpringDemoScore::Page::~Page() {}

static int SnippetSpringDemo()
{
  //Create a score.
  SpringDemoScore MyScore;

  //Add a page to the score.
  MyScore.Canvases.Add() = new SpringDemoScore::Page;

  //Set to letter landscape.
  MyScore.Canvases.z()->Dimensions = Paper::Landscape(Paper::Letter());

  //Set the PDF-specific properties.
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "snippet-spring-demo.pdf";

  //Write the score to PDF.
  MyScore.Create<PDF>(PDFSpecificProperties);

  //Note success to console window.
  C::Out() >> "Successfully wrote '" << PDFSpecificProperties.Filename << "'.";

  return AutoRelease<Console>();
}
static int SnippetSprings()
{
  SpringSystem Springs;
  Springs.CreateTestGrid(9, 9);
  Springs.Solve(8.f);
  C::Out() >> Springs.SolutionString();
  return AutoRelease<Console>();
}
//An example of deriving a Portfolio and Canvas.
struct TextScore : public Portfolio
{
  Font MyFont;

  struct Page : public Canvas
  {
    virtual void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      TextScore& TextScore = dynamic_cast<struct TextScore&>(Portfolio);

      //Some normal text first.
      Painter.Translate(Vector(1.0, 10.0));
      Painter.Draw("Hello, world!", TextScore.MyFont);
      Painter.Revert();

      //Tale of Two Cities -- in changing size, style, and color!
      Text t(TextScore.MyFont, Font::Regular, 12.0, 6.0);

      String s = "It was the best of times, it was the worst of times, "
        "it was the age of wisdom, it was the age of foolishness, it was "
        "the epoch of belief, it was the epoch of incredulity, it was the "
        "season of Light, it was the season of Darkness, it was the spring "
        "of hope, it was the winter of despair, we had everything before us, "
        "we had nothing before us, we were all going direct to heaven, we "
        "were all going direct the other way - in short, the period was so "
        "far like the present period, that some of its noisiest authorities "
        "insisted on its being received, for good or for evil, in the "
        "superlative degree of comparison only.";

      t.ImportStringToWords(s);

      count k = 0;
      Random r(123);
      for(count i = 0; i < t.Words.n(); i++)
      {
        Word& w = t.Words[i];
        for(count j = 0; j < w.n(); j++, k++)
        {
          Character& l = w[j];
          l.PointSize = j + 9;
          l.Style = Font::Style((k / 5) % 4);
          l.FaceColor = Color(r.Between(0.0, 0.5), r.Between(0.0, 0.5),
            r.Between(0.0, 0.5));
        }
      }

      t.DetermineLineBreaks();
      t.Typeset();
      Painter.Translate(Vector(1.0, 6.0));
      Painter.Draw(t);
      Painter.Revert();
    }

    virtual ~Page();
  };
  virtual ~TextScore();
};
TextScore::~TextScore() {}
TextScore::Page::~Page() {}

static int SnippetText()
{
  //Create a score.
  TextScore MyTextScore;

  //Load the fonts.
  MyTextScore.MyFont = Helper::ImportNotationFont();

  //Add a page to the score.
  MyTextScore.Canvases.Add() = new TextScore::Page;

  //Set to letter landscape.
  MyTextScore.Canvases.z()->Dimensions = Paper::Letter();

  //Set the PDF-specific properties.
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "snippet-text.pdf";

  //Write the score to PDF.
  MyTextScore.Create<PDF>(PDFSpecificProperties);

  //Note success to console window.
  C::Out() >> "Successfully wrote '" << PDFSpecificProperties.Filename << "'.";

  return AutoRelease<Console>();
}
static int SnippetToJSON()
{
  C::Out() >> "This is a placeholder example. It has no functionality.";
  C::Out()++;
  /*
  <graph>
    <node id="0" root="root" Type="Island">
      <edge to="1" Type="Token"/>
    </node>
  </graph>
  */
  Value n;
  n["0"]["edges"][0]["to"] = "1";
  n["0"]["edges"][0]["type"] = "token";
  n["0"]["root"] = true;
  n["0"]["type"] = "island";

  Value g;
  g["nodes"] = n;
  String s;
  JSON::Export(g, s, false);
  C::Out() >> s;
  JSON::Export(g, s, true);
  C::Out() >> s;

  return prim::AutoRelease<prim::Console>();
}
//------------------------------------------------------------------------------

/*
Tutorial 0: Fundamentals and prim (read through carefully)

This tutorial explains how Belle is included and compiled. It also
shows features of the prim library on which Belle depends. It does not show
any features specific to Belle.

On Mac/Linux you can build and run from the terminal using:
Scripts/MakeAndRun Tutorial0

For more information related to building, see the README.
*/

//------------------------------------------------------------------------------

/*Step 1: Including Belle
There are two main rules to including Belle:

1. Include belle.h in each .cpp that needs it. Here ../Belle... is used
because the file happens to be in the parent directory; however, the relative
path will differ depending on how your project has been set up.

/belle.h includes the whole library which is located in /Source and its
subdirectories.

2. #define BELLE_COMPILE_INLINE in one and only one .cpp file, such as
the main .cpp file or the file that contains int main(), above the
belle.h include.

If you look at the files in /Source you will notice something which may look a
little peculiar at first: there are no .cpp files. This is done primarily for
organizational reasons. By keeping the function definitions (bodies) with the
function declarations, one may quickly inspect a method to see what it does. For
large classes this can make some files unruly, so it is highly recommended that
you use the Doxygen-generated reference material to get an overview of a class.

While most of the code is perfectly happy to keep definition with declaration,
there are times when this is impossible, for example due to mutual dependencies.
Therefore, some code is blocked off as "inline compile":
#ifdef BELLE_COMPILE_INLINE
//code for declarations which are not with definitions
#endif

The _COMPILE_INLINE paradigm is also used in the prim and MICA sublibraries,
however, Belle will automatically trigger the _COMPILE_INLINE switch for these
sublibraries.
*/

//------------------------------------------------------------------------------

/*Step 2: Namespaces
Belle uses the prim library for all of its primitive objects. prim is like a
dialect of C++ built for rapid prototyping, and tries to provide an organized,
high-level interface to do the most common programming tasks.

prim and Belle use namespaces to prevent symbol leakage and name collisions.
In fact, prim is designed in such a way that including prim.h will not bring
any symbols into the global namespace (except for the prim namespace), and so
the global namespace is free of all pollution caused by the C++ standard
libraries. Belle inherits this philosophy.

For small projects like this tutorial, it is perfectly fine to use the
'using namespace ...;' keyword to simply bring the relevant namespaces into the
global namespace. For larger projects, especially those including other
libraries, you will either want to not use 'using namespace', or create a
synthetic namespace that contains everything.

For example:
namespace myproject
{
  using namespace prim;
  using namespace belle;
  using namespace belle::painters;
}

Below is an explanation of what each namespace does:
*/

/*The core prim library. It contains fundamental classes like Array, Console,
Complex, File, Graph, List, Matrix, Memory, Pointer, Random, Ratio(nal), Serial,
and String.*/

//using namespace prim; //included by namespace belle

/*The core belle namespace.
It contains classes relevant to drawing such as Affine, Canvas, Color, Font,
Painter, Path, Portfolio, Shapes, Text. This also imports the prim classes:
Vector(Int), Box(Int), Bezier, Ellipse, Line, PolygonShape.*/

//------------------------------------------------------------------------------

/*Step 3: int main() -- as always your program starts here, but notice that we
do not need the standard library (cout, cin, string, vector, etc.) since most of
the commonly used functionality is already in prim.*/

//This program shows how prim can be used.
static int SnippetTutorial0()
{
  /*The following objects all belong to the prim namespace. The
  using namespace statements above make scoping prim unnecessary.*/

  //----------//
  //Data Types//
  //----------//
  {
    //There are two fundamental abstract types which are used pervasively:
    count a = 1; //Signed integer set to int32/int64 depending on pointer size
    number b = 3.4f; //Floating-point set to float64 by default

    //The count is used anywhere counting is needed, for example in for-loops:
    for(count i = 0; i < 10; i++)
    {
    }

    //The number is used wherever floating-point calculation is done.
    number x = Cos(Pi<number>() / 2.f);

    /*Note that count and number are roughly equivalent to the intention of
    'int' and 'double'. However, 'count' stores more like a 'long' (or
    'long long' in 64-bit Windows).*/

    /*The count is not intended to be used for integer math. Instead use
    integer, which always uses the largest available int type.*/
    integer y = 123456789;
    y *= 987654321;

    //For char and unsigned char use ascii and byte instead:
    ascii d = 'd';
    const ascii* e = "string";
    byte f = 0xFF;

    //For Unicode code-points use unicode:
    unicode g = 0x0416; //Cyrillic 'Zhe'

    //You can always specify word size exactly using the familiar forms:
    uint8 h = 0;
    int8 i = 0;
    uint16 j = 0;
    int16 k = 0;
    uint32 l = 0;
    int32 m = 0;
    uint64 n = 0;
    int64 o = 0;
    float32 p = 0;
    float64 q = 0;
    float80 r = 0;

    /*While not a built-in type, the Ratio and Complex<T> class are also useful
    prim types that are fully overloaded so that they can be used like built-in
    types.*/
    Ratio s(3, 9); //Auto-reduces to 1/3
    s.Numerator(); //Gets numerator
    s.Denominator(); //Gets denominator

    Complex<number> t(1.f, 2.f); //Represents 1 + 2i
    Complex<number> u(-1.f, 2.f);
    t = t + u; //Complex addition
    t = t * u; //Complex multiplication
    t = t / u; //Complex division
    t = t - u; //Complex subtraction
    t.Mag(); //Gets magnitude
    t.Ang(); //Gets angle

    //Hide some warnings about unused parameters (ignore this).
    String hide;
    hide >> (int64(a) + int64(b) + int64(d) + int64(e) + int64(f) +
      int64(g) + int64(h) + int64(i) + int64(j) + int64(k) +
      int64(l) + int64(m) + int64(n) + int64(o) + int64(p) +
      int64(q) + int64(r) + int64(x) + int64(y));
  }

  //------------------//
  //String and Console//
  //------------------//

  /*Note that Console is a subclass of String and so everything here equally
  applies to String. For example, uncommenting the following will simply direct
  the values into a String object (and silence the console output).*/
  //String c;

  /*Print-out with global Console object prim::c. Note << means "append" and >>
  means "append on a new line".*/
  C::Out() >> "Hello " << "World!";

  //Adds a blank line to the output.
  C::Out()++;

  /*Editorial note: it may seem a little screwy using >> and << for append if
  you are used to the standard streams. However, the upshot is you do not need
  to constantly terminate with end lines, and in this developer's mind, breaking
  with convention significantly improves the readability of most string code.*/

  //Consoles and strings are overloaded to take most built-in data types.
  C::Out() >> 5;
  C::Out() >> 1.4;
  C::Out() >> Ratio(4, 8);
  C::Out() >> Vector(3.4f, 4.5f);
  C::Out() >> Array<number>();
  C::Out() >> List<number>();
  C::Out() >> Cos(Pi<number>());
  C::Out() >> Sqrt(9.0);

  /*Note that Vector is just short for Complex<number> (and VectorInt for
  Complex<integer>). Vector is completely unrelated to the C++ standard
  library's notion of vector which is like a prim Array.*/

  //The decimal precision of each String object can be controlled:
  C::Out() >> Pi<number>(); //Default precision (5 decimal digits)
  C::Out().Precision(10);
  C::Out() >> Pi<number>(); //10 decimal digits
  C::Out().Precision(2);
  C::Out() >> Pi<number>(); //2 decimal digits

  //----------------//
  //Lists and Arrays//
  //----------------//

  /*Lists and Arrays are templated types which means they are declared with an
  element type in angle brackets.*/
  Array<number> A;
  A.Add(10.0); //Pass the new value into...
  A.Add() = 20.0; //...or assign to returned reference, whichever you prefer.
  A.Add() = 30.0;
  C::Out() >> A;

  List<number> L;
  L.Add() = 40.0; //Lists always assign the new value to the returned reference.
  L.Add() = 50.0;
  L.Add() = 60.0;
  C::Out() >> L;

  /*Lists and Arrays use n() to get the number of elements and a() and z() to
  get the first and last elements.*/
  C::Out() >> "Array: " << A.n() << " elements: " <<
    A.a() << ", ..., " << A.z();
  C::Out() >> "List : " << L.n() << " elements: " <<
    L.a() << ", ..., " << L.z();

  //Iterating over an array or list typically looks like:
  for(count i = 0; i < A.n(); i++)
    C::Out() >> i << ": " << A[i];

  return AutoRelease<Console>();
}
//------------------------------------------------------------------------------

/*
Tutorial 1: Drawing simple graphics manually in Belle

This tutorial explains the graphics abstraction used by Belle. It
assumes familiarity with prim as seen in Tutorial 0.

On Mac/Linux you can build and run from the terminal using:
Scripts/MakeAndRun Tutorial1

For more information related to building, see the README.
*/

/*In Belle there are three fundamental abstract data types for graphics:
Portfolio, Canvas, and Painter.

The Portfolio contains a list of Canvases and can be thought of as a document
with multiple pages. The user of the library must at least subclass
Canvas, and implement the Paint virtual method. If the user needs the
Portfolio to store any information relevant to the whole document to be accessed
during the Paint, then the Portfolio should also be subclassed.

The Painter is a device-independent vector graphics object and could represent
file or screen-based output.

This example will show how to subclass both Portfolio and Canvas and how to use
the PDF and SVG painters.
*/

//Subclass Tutorial1Score from belle::Portfolio
struct Tutorial1Score : public Portfolio
{
  //An array of rectangles to paint.
  Array<Box> BoxesToPaint;

  /*Subclass Page from belle::Canvas. Note that Page is a class inside
  a class, so it is really a Tutorial1Score::Page; however, it is not necessary
  to do it this way. It just logically groups the Page class with the
  Tutorial1Score to which it pertains.*/
  struct Page : public Canvas
  {
    //This method gets called once per canvas.
    void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      /*Since we need access to the Tutorial1Score (as opposed to base class
      Portfolio) in order to draw the rectangles, we can forward the paint call
      to a custom paint method which uses a Tutorial1Score& instead of a
      Portfolio&.*/
      Paint(Painter, dynamic_cast<Tutorial1Score&>(Portfolio));
    }

    //Custom paint method with score.
    void Paint(Painter& Painter, Tutorial1Score& Tutorial1Score)
    {
      //Print which page is being painted.
      C::Out() >> "Painting page: " << Painter.GetPageNumber();

      //Paint each rectangle in the rectangle array.
      for(count i = 0; i < Tutorial1Score.BoxesToPaint.n(); i++)
      {
        /*Create an empty path. A path is a vector graphics object containing
        a list of core instructions: move-to (start new path), line-to,
        cubic-to (Bezier curve), and close-path. Generally, multiple subpaths
        are interpreted by the rendering targets according to the zero-winding
        rule.*/
        Path p;

        /*Add the rectangle shape to the path. The Shapes class contains several
        primitive building methods.*/
        Shapes::AddBox(p, Tutorial1Score.BoxesToPaint[i]);

        //Alternate green fill with blue stroke.
        if(i % 2 == 0)
          Painter.SetFill(Colors::Green());
        else
          Painter.SetStroke(Colors::Blue(), 0.01f);

        //Draw the path, separating the fills and strokes by page.
        if(i % 2 == Painter.GetPageNumber())
          Painter.Draw(p);
      }
    }

    virtual ~Page();
  };
  virtual ~Tutorial1Score();
};
Tutorial1Score::~Tutorial1Score() {}
Tutorial1Score::Page::~Page() {}

//This program creates a couple of pages with some rectangles using Belle.
static int SnippetTutorial1()
{
  //Step 1: Create a score, add some pages, and give it some information.

  //Instantiate a score.
  Tutorial1Score MyTutorial1Score;

  //Add a portrait page to the score.
  MyTutorial1Score.Canvases.Add() = new Tutorial1Score::Page;
  MyTutorial1Score.Canvases.z()->Dimensions = Paper::Portrait(Paper::Letter());

  //Add a landscape page to the score.
  MyTutorial1Score.Canvases.Add() = new Tutorial1Score::Page;
  MyTutorial1Score.Canvases.z()->Dimensions = Paper::Landscape(Paper::Letter());

  /*Add some rectangles for the score to paint. Note this is just a custom
  member that was created to demonstrate how to pass information to the painter.
  There is nothing intrinsic to the Tutorial1Score about painting rectangles.*/
  const number GeometricConstant = 1.2f;
  for(number i = 0.01f; i < 8.f; i *= GeometricConstant)
    MyTutorial1Score.BoxesToPaint.Add() = Box(Vector(i, i),
      Vector(i, i) * GeometricConstant);

  //Step 2a: Draw the score to PDF.

  /*Set the PDF-specific properties, for example, the output filename. If no
  filename is set, then the contents of the PDF file end up in
  PDF::Properties::Output.*/
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "snippet-tutorial-1.pdf";

  /*Write the score to PDF. Note how in Belle, the Canvas Paint() method is
  never called directly. Instead a portfolio creates a render target which then
  calls back the paint method on each canvas. This is an extension of the
  device-independent graphics paradigm.*/
  MyTutorial1Score.Create<PDF>(PDFSpecificProperties);

  //Print the name of the output file.
  C::Out() >> "Wrote PDF to '" << PDFSpecificProperties.Filename << "'.";

  /*Step 2b: Here is the same thing except using the SVG renderer. Since SVG is
  an image format, the result will be a sequence of files.*/

  /*Set the SVG-specific properties, for example, the output filename prefix. If
  no filename is set, then the contents of the SVG file end up in the
  SVG::Properties::Output array.*/
  SVG::Properties SVGSpecificProperties;
  SVGSpecificProperties.FilenameStem = "snippet-tutorial-1-";

  //Write the score to SVG.
  MyTutorial1Score.Create<SVG>(SVGSpecificProperties);

  //Note the name of the output file to console window.
  C::Out() >> "Wrote SVGs to '" <<
    SVGSpecificProperties.FilenameStem << "*.svg'.";

  return AutoRelease<Console>();
}
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

struct Tutorial2Score : public Portfolio
{
  struct Page : public Canvas
  {
    void DrawShape(Painter& Painter, Color ShapeColor = Colors::Red(),
      Color AxisColor = Colors::Black())
    {
      /*To demonstrate affine transformations it helps to show a coordinate
      axis. The Shapes class has a built-in path-maker for an axis with ticks.*/
      Path Axis, Shape;
      Shapes::AddCoordinateAxis(Axis);

      /*Show a simple rectangle using filled (not stroked) lines. This allows
      us to use a fill operation instead of a stroke operation and it also
      provides the outline with rounded corners.*/
      Shapes::AddBoxFromLines(Shape,
        Box(Vector(0.f, 0.f), Vector(1.f, 1.f)), 0.05f);

      //Draw the axis using the color for the axis.
      Painter.SetFill(AxisColor);
      Painter.Draw(Axis);

      //Draw the shape on top of the axis using the color for the shape.
      Painter.SetFill(ShapeColor);
      Painter.Draw(Shape);
    }

    void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      (void)Portfolio;

      /*For this example, we'll move the origin to the center to make things
      a little easier to see. The most important thing to know is that by
      default Belle uses the bottom-left corner of the canvas as the origin and
      not the top-left corner (with reversed y) as many graphics libraries do.
      While this is a bit unconventional, it allows for the x and y dimensions
      to be treated the same and makes for clearer code.*/
      Painter.Translate(Dimensions / 2.f);

      //Draw a silhouette of the untransformed shape.
      DrawShape(Painter, Colors::Gray(), Colors::LightGray());

      //For each page show a different example of using affine transformations.
      switch(Painter.GetPageNumber())
      {
        case 0: //Page 1
        //Just show the gray coordinate axis...
        break;

        case 1: //Page 2
        //Show a translation over 1.3 and up 1.8.
        Painter.Translate(Vector(1.3f, 1.8f));
        DrawShape(Painter);
        Painter.Revert();
        break;

        case 2: //Page 3
        //Show a rotation of 30 degrees.
        Painter.Rotate(30.f * Deg<number>()); /*(Deg is just a unit that
        converts degrees to radians when multiplying and radians to degress
        when dividing).*/
        DrawShape(Painter);
        Painter.Revert();
        break;

        case 3: //Page 4
        //Show a scaling of 1.5.
        Painter.Scale(1.5f);
        DrawShape(Painter);
        Painter.Revert();
        break;

        case 4: //Page 5
        /*Scaling and translating is not the same as translating and scaling.
        This is related to the fact that matrix multiplication is not generally
        commutative.*/
        Painter.Translate(Vector(1.f, 1.f)); //Translate-scale
        Painter.Scale(2.f);
        DrawShape(Painter, Colors::Green());
        Painter.Revert(2); /*(Revert defaults to undoing one transformation, but
          you can specify any number of previous transformations to revert at
          once.)*/

        Painter.Scale(2.f); //Scale-translate
        Painter.Translate(Vector(1.f, 1.f));
        DrawShape(Painter, Colors::Red());
        Painter.Revert(2);
        break;

        case 5: //Page 6
        /*For the same underlying reason, rotating and translating is not the
        same as translating and rotating.*/
        Painter.Translate(Vector(1.f, 1.f)); //Translate-rotate
        Painter.Rotate(30.f * Deg<number>());
        DrawShape(Painter, Colors::Green());
        Painter.Revert(2);

        Painter.Rotate(30.f * Deg<number>()); //Rotate-translate
        Painter.Translate(Vector(1.f, 1.f));
        DrawShape(Painter, Colors::Red());
        Painter.Revert(2);
        break;

        case 6: //Page 7
        //However, scaling and rotation happen to be commutative.
        Painter.Scale(2.f); //Scale-rotate
        Painter.Rotate(30.f * Deg<number>());
        DrawShape(Painter, Colors::Green());
        Painter.Revert(2);

        Painter.Rotate(30.f * Deg<number>()); //Rotate-scale
        Painter.Scale(2.f);
        DrawShape(Painter, Colors::Green());
        Painter.Revert(2);
        break;

        case 7: //Page 8
        /*Occasionally, one may find a need to scale by different amounts in the
        x- and y- dimensions. This is typically done to create a mirror image.*/
        Painter.Scale(Vector(-1.f, 1.f)); //Horizontal mirror
        DrawShape(Painter, Colors::LightGreen());
        Painter.Revert();

        DrawShape(Painter, Colors::Green()); //Original
        break;

        case 8: //Page 9
        {
        /*You can also create an affine transformation using the Affine object,
        and call Transform with the object.

        The TranslateScaleRotate method on Affine can be used to position an
        object at a given size and angle. It is equivalent to multiplying by
        a Translate, Scale, and Rotate in that order (though the scale and
        rotation order could be flipped per the result shown on page 7).*/
        Affine a = Affine::TranslateScaleRotate(Vector(1.f, 1.f), 2.f, 30.f *
          Deg<number>());
        Painter.Transform(a);
        DrawShape(Painter, Colors::Green());
        Painter.Revert();

        Affine b = (Affine::Translate(Vector(1.f, 1.f)) * Affine::Scale(2.f)) *
          Affine::Rotate(30.f * Deg<number>());
        Painter.Transform(b);
        DrawShape(Painter, Colors::Green());
        Painter.Revert();

        Painter.Translate(Vector(1.f, 1.f));
        Painter.Scale(2.f);
        Painter.Rotate(30.f * Deg<number>());
        DrawShape(Painter, Colors::Green());
        Painter.Revert(3);
        }
        break;

        case 9: //Page 10
        /*You can easily change units from the default inches to another unit.
        The following creates a horizontal unit-sized vector in Centimeters and
        converts that to Inches. The x-component is thus the relative scale.*/
        Painter.Scale(Inches(Centimeters(1.f, 0.f)).x);
        DrawShape(Painter);
        Painter.Revert();
        break;
      }

      Painter.Revert(); //Revert the page centering transformation.
    }

    virtual ~Page();
  };
  virtual ~Tutorial2Score();
};
Tutorial2Score::~Tutorial2Score() {}
Tutorial2Score::Page::~Page() {}

static int SnippetTutorial2()
{
  //Instantiate a score.
  Tutorial2Score MyTutorial2Score;

  //Add several pages to the score to show different affine transformations.
  for(count i = 0; i < 10; i++)
  {
    MyTutorial2Score.Canvases.Add() = new Tutorial2Score::Page;
    MyTutorial2Score.Canvases.z()->Dimensions =
      Paper::Portrait(Paper::Letter());
  }

  /*Write the score out to PDF. See Tutorial 1 for an explanation of how this
  works.*/
  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "snippet-tutorial-2.pdf";
  MyTutorial2Score.Create<PDF>(PDFSpecificProperties);
  C::Out() >> "Wrote PDF to '" << PDFSpecificProperties.Filename << "'.";

  return AutoRelease<Console>();
}
//------------------------------------------------------------------------------

/*
Tutorial 3: Using MICA, the Music Information and Concept Archive

This tutorial explains how musical information is encoded in Belle using the
MICA library, which is a separate standalone library developed by author with
the specific aim of providing a means to differentiate and relate musical
concepts in a general way.

On Mac/Linux you can build and run from the terminal using:
Scripts/MakeAndRun Tutorial3

For more information related to building, see the README.
*/

//------------------------------------------------------------------------------

/*MICA lives in the mica namespace. There are thousands of identifiers (the
'concepts') in the namespace so it is very important to not use
'using namespace mica' as this will pollute the global namespace with these
thousands of concepts no doubt causing name conflicts. See below for the
recommended usage of the mica:: namespace.*/

//------------------------------------------------------------------------------

static int SnippetTutorial3()
{
  //-------------------//
  //Part 1 - Using MICA//
  //-------------------//
  C::Out() >> "Using MICA:";
  C::Out() >> mica::Intervals::transpose(mica::C4, mica::PerfectOctave,
    mica::Above);
  C::Out() >> mica::Intervals::transpose(mica::G4, mica::PerfectOctave,
  mica::Above);
  C::Out() >> mica::Intervals::transpose(mica::E4, mica::MajorSeventh,
  mica::Above);
  C::Out() >> mica::Intervals::transpose(mica::C4, mica::PerfectOctave,
    mica::Below);
  C::Out() >> mica::Intervals::transpose(mica::G4, mica::PerfectOctave,
    mica::Below);
  C::Out() >> mica::Intervals::transpose(mica::E4, mica::MajorSeventh,
    mica::Below);
  /*Create a MICA Concept and assign it. Concept is the only end-user type in
  MICA.*/
  mica::Concept x = mica::C; //Assigns x to note letter C.

  /*Note that C is the en-cpp (English C++) identifier for the musical concept
  of C. Other natural and programming language combinations may use a different
  identifier. For example, the German identifier might be Do instead of C.*/

  /*Get the definitional name in the default language. This is an overload for
  the const char* operator. You can also call name(x) directly. Note that the
  definitional name is not necessarily the same as the identifier. This name is
  the one that would appear in a dictionary in the given language.*/
  C::Out() >> "[C]: " << x;
  C::Out() >> "[C] Name: " << name(x); //e.g. mica::name(x)

  /*Note in the previous example that the function actually called was
  mica::name(x). Because of argument-dependent name lookup, in many cases it is
  unnecessary to specify the namespace to the function. This is a C++ language
  feature and due to the way in which MICA was designed.*/

  //You can also get the high and low 64-bit halves of the UUIDv4 directly.
  C::Out() >> "[C]: " << x.high << ":" << x.low;

  /*The mica::Concept class is identical to the mica::UUIDv4 class except that
  Concept is automatically initialized to mica::Undefined on construction.
  Therefore user code should only use the mica::Concept subclass.*/
  mica::Concept undefined;
  C::Out() >> "Concept initialized to: " <<
    undefined.high << ":" << undefined.low;

  /*
  //Don't do this:
  mica::Concept hmmm;
  C::Out() >> "UUIDv4 initialized to   : " << hmmm.high << ":" << hmmm.low;
  */

  /*You can also convert a text string to a Concept. Note that since named()
  takes a const char* string, you will need the mica:: namespace to call the
  function unlike the other mica functions.*/
  //C::Out() >> "'f': " << mica::named("f"); //Means dynamic marking for forte
  C::Out() >> "'F': " << mica::named("en:F"); //Means the pitch letter F

  /*Note of caution: in MICA a concept is defined by its underlying UUIDv4, not
  by its definitional name (though in most cases the UUIDv4 has been generated
  from an identifier using a hashing algorithm). You should not rely on named()
  except within a single known revision of the library as names can change
  without notice in order to improve the library.*/

  //Maps are used to store relational information between multiple elements.
  C::Out() >> "[A B]: " << map(mica::A, mica::B);

  //Note maps are unordered so the order of arguments does not matter.
  C::Out() >> "[B A]: " << map(mica::B, mica::A);

  //If a mapping does not exist, Undefined is returned.
  C::Out() >> "[A Undefined]: " << map(mica::A, mica::Undefined);

  /*Some concepts in MICA are special in that they define sequences of other
  concepts. Sequences have a non-zero length and may be cyclic.*/
  C::Out() >> "[Letters] Sequence? " <<
    (sequence(mica::Letters) ? "Yes" : "No");
  C::Out() >> "[Letters] Cyclic?   " <<
    (cyclic(mica::Letters) ? "Yes" : "No");
  C::Out() >> "[Letters] Length:   " << length(mica::Letters);

  /*Items in a sequence are indexable through the item() method. The indexing
  is zero-based. The following enumerates the Letters sequence.*/
  C::Out() >> "[Letters]:";
  for(int i = 0, n = int(length(mica::Letters)); i < n; i++)
    C::Out() >> " * " << item(mica::Letters, i);

  /*For many possible reasons, you may want to get the ith item relative to
  another item in the sequence. For example, you may want to start on C in
  Letters instead. Note that the sequence automatically wraps around to A after
  G because this sequence is cyclic.*/
  C::Out() >> "[Letters] relative to C:";
  for(int i = 0, n = int(length(mica::Letters)); i < n; i++)
  {
    /*The sequence and origin are given first. The thing you are looking for
    always goes last.*/
    C::Out() >> " * " << item(mica::Letters, mica::C, i);
  }

  /*Cyclic wrapping occurs modulo the length of the sequence and works with
  negative numbers.*/
  C::Out() >> "Letters[-100]: " << item(mica::Letters, -100);
  C::Out() >> "Letters[100]: " << item(mica::Letters,  100);

  /*You may also want to lookup the index of an item in a sequence. For safety,
  MICA returns an integer UUIDv4 so that you can check for Undefined in case the
  element was not in the sequence or the sequence did not exist.*/
  C::Out() >> "Letters[C]: " << index(mica::Letters, mica::C);

  /*Just as item can have an origin specified, you can also specify an origin
  with index. The sequence and origin appear first and the element whose index
  you are looking for comes last.*/
  C::Out() >> "Letters[G - C]: " <<
    numerator(index(mica::Letters, mica::C, mica::G));

  /*When you are certain the value is safe to use, you can get the value using
  the numerator() function. If the returned integer UUIDv4 was Undefined, then
  numerator() will return zero and thus you would not be able to distinguish
  between an error and an actual index of zero.*/
  C::Out() >> "Letters[C]: " << numerator(index(mica::Letters, mica::C));

  //If the concept does not belong to the sequence, then Undefined is returned.
  C::Out() >> "Letters[Undefined]: " <<
    index(mica::Letters, mica::Undefined);

  //You can test for Undefined like you would test for any equality.
  mica::Concept r = index(mica::Letters, mica::C);
  if(r == mica::Undefined)
  {
    C::Out() >> "Aborting!";
    return -1;
  }

  //You can also test for failure using the undefined() function.
  if(mica::undefined(r))
  {
    C::Out() >> "Aborting!";
    return -1;
  }

  /*An invalid index passed to item() such as one generated from index() will
  simply propogate the Undefined to the outermost call. Therefore, you never
  need to worry about catching an error in the middle of a compound expression.
  Just let the expression carry out and if the return value is Undefined, then
  you know something went wrong.*/
  C::Out() >> "Letters[Letters[C]]: " <<
    item(mica::Letters, index(mica::Letters, mica::C));
  C::Out() >> "Letters[Letters[Undefined]]: " <<
    item(mica::Letters, index(mica::Letters, mica::Undefined));

  //---------------------------//
  //Part 2 - Practical Examples//
  //---------------------------//

  //-------//
  //Letters//
  //-------//
  C::Out() >> "\nLetters:";

  /*Get index of letter relative to another letter. If either of the concepts
  does not exist in the sequence, then Undefined is returned.*/
  C::Out() >> "Letters[E - C]: " <<
    numerator(index(mica::Letters, mica::C, mica::E));

  //Get 100th letter above another letter.
  C::Out() >> "Letters[C + 100]: " << item(mica::Letters, mica::C, 100);

  //-----------//
  //Accidentals//
  //-----------//
  C::Out() >> "\nAccidentals:";

  //Get index of accidental relative to another accidental.
  C::Out() >> "Accidentals[Flat - Natural]: " <<
    numerator(index(mica::Accidentals, mica::Natural, mica::Flat));

  //-----//
  //Notes//
  //-----//
  C::Out() >> "\nNotes:";

  //Get a chromatic note given a letter and accidental.
  C::Out() >> "[D Flat]: " << map(mica::D, mica::Flat);

  //Get a pitch given a letter, accidental, and octave number.
  C::Out() >> "[D Flat 4]: " << map(mica::D, mica::Flat, mica::integer(4));

  //Get a diatonic pitch (pitch with no accidental).
  C::Out() >> "[D 4]: " << map(mica::D, mica::integer(4));

  //Convert pitch to diatonic pitch (remove accidental).
  C::Out() >> "[DFlat4 DiatonicPitch]: " <<
    map(mica::DFlat4, mica::DiatonicPitch);

  //Get chromatic note of pitch.
  C::Out() >> "[DFlat4 ChromaticNote]: " <<
    map(mica::DFlat4, mica::ChromaticNote);

  //Get accidental of pitch.
  C::Out() >> "[DFlat4 Accidental]: " <<
    map(mica::DFlat4, mica::Accidental);

  //Get letter of pitch.
  C::Out() >> "[DFlat4 Letter]: " << map(mica::DFlat4, mica::Letter);

  //Get octave of pitch.
  C::Out() >> "[DFlat4 Octave]: " << numerator(map(mica::DFlat4, mica::Octave));

  //Get MIDI keynumber of pitch.
  C::Out() >> "[DFlat4 MIDIKeyNumber]: " <<
    numerator(map(mica::DFlat4, mica::MIDIKeyNumber));

  //Get accidental of chromatic note.
  C::Out() >> "[DFlat Accidental]: " << map(mica::DFlat, mica::Accidental);

  //Get letter of chromatic note.
  C::Out() >> "[DFlat Letter]: " << map(mica::DFlat, mica::Letter);

  //---------------//
  //Staff Positions//
  //---------------//
  C::Out() >> "\nStaff-Positions:";

  /*A staff position is an integer expressing the line or space a note is
  situated on relative to the middle line (or space) of a given staff. For
  example, B4 in Treble Clef is staff position 0, the center line.*/

  //Get staff position of diatonic pitch given clef.
  C::Out() >> "[D4 TrebleClef]: " << numerator(map(mica::D4, mica::TrebleClef))
   ;

  //Get staff position of chromatic pitch given clef.
  C::Out() >> "[[DSharp4 DiatonicPitch] BassClef]: " <<
    numerator(map(map(mica::DSharp4, mica::DiatonicPitch), mica::BassClef))
     ;

  //Get diatonic pitch from staff position in given clef.
  C::Out() >> "[2 TrebleClef]: " << map(mica::integer(2), mica::TrebleClef);

  //Get pitch from staff position in given clef with accidental.
  C::Out() >> "[[2 TrebleClef] Flat]: " <<
    map(map(mica::integer(2), mica::TrebleClef), mica::Flat);

  //----//
  //Keys//
  //----//
  C::Out() >> "\nKeys:";

  //Create key from notename and mode (Major).
  C::Out() >> "[DFlat Major]: " << map(mica::DFlat, mica::Major);

  //Create key from notename and mode (Dorian).
  C::Out() >> "[DFlat Dorian]: " << map(mica::DFlat, mica::Dorian);

  //Get key signature from key (Major).
  C::Out() >> "[DFlatMajor KeySignature]: " <<
    map(mica::DFlatMajor, mica::KeySignature);

  //Get key signature from key (Dorian).
  C::Out() >> "[DFlatDorian KeySignature]: " <<
    map(mica::DFlatDorian, mica::KeySignature);

  //Get key from key signature and mode.
  C::Out() >> "[ThreeFlats Minor]: " <<
    map(mica::ThreeFlats, mica::Minor);

  //Get mode from key.
  C::Out() >> "[DFlatMajor Mode]: " << map(mica::DFlatMajor, mica::Mode);

  //Get mode from key.
  C::Out() >> "[DFlatDorian Mode]: " << map(mica::DFlatDorian, mica::Mode);

  //Get the staff positions of each sharp in treble clef.
  {
    mica::Concept SharpPositions = map(mica::TrebleClef, mica::Sharp);
    C::Out() >> "Inspecting [TrebleClef Sharp]: " << SharpPositions;
    for(int i = 0; i < length(SharpPositions); i++)
      C::Out() >> " * " << numerator(item(SharpPositions, i));
  }

  //---------//
  //Intervals//
  //---------//
  C::Out() >> "\nIntervals:";

  //Combine distance with quality.
  C::Out() >> "[Fourth Perfect]: " << map(mica::Fourth, mica::Perfect);

  //Distance of interval.
  C::Out() >> "[PerfectFourth Distance]: " <<
    map(mica::PerfectFourth, mica::Distance);

  //Quality of interval.
  C::Out() >> "[PerfectFourth Quality]: " <<
    map(mica::PerfectFourth, mica::Quality);

  //Transpose note up interval.
  C::Out() >> "transpose(D4 MajorThird Above): " <<
    mica::Intervals::transpose(mica::D4, mica::MajorThird, mica::Above);

  //Transpose note down interval.
  C::Out() >> "transpose(D4 MajorThird Below): " <<
    mica::Intervals::transpose(mica::D4, mica::MajorThird, mica::Below);

  //Simple interval between two pitches.
  C::Out() >> "interval(D5 B3): " <<
    mica::Intervals::interval(mica::D5, mica::B3);

  //Quality of interval between two pitches.
  C::Out() >> "quality(D5 B3): " <<
    mica::Intervals::quality(mica::D5, mica::B3);

  //Distance of interval between two pitches.
  C::Out() >> "distance(D5 B3): " <<
    mica::Intervals::distance(mica::D5, mica::B3);

  //Number of extra octaves between two pitches.
  C::Out() >> "octaves(D5 B3): " <<
    numerator(mica::Intervals::octaves(mica::D5, mica::B3));

  //Position of second pitch with respect to first pitch.
  C::Out() >> "direction(D5 B3): "
    << mica::Intervals::direction(mica::D5, mica::B3);

  //Key signature transposition.
  C::Out() >> "[TwoSharps MinorSecond Above]: " <<
    map(mica::TwoSharps, mica::MinorSecond, mica::Above);

  return AutoRelease<Console>();
}

int main(int ArgumentCount, const char** Arguments)
{
  Array<String> Snippets;
  {
    Snippets.Add() = "BlankPage";
    Snippets.Add() = "Color";
    Snippets.Add() = "Drawable";
    Snippets.Add() = "Geometry";
    Snippets.Add() = "GraphSyntax";
    Snippets.Add() = "Image";
    Snippets.Add() = "MIDI";
    Snippets.Add() = "SegmentedHull";
    Snippets.Add() = "Spiral";
    Snippets.Add() = "SpringDemo";
    Snippets.Add() = "Springs";
    Snippets.Add() = "Text";
    Snippets.Add() = "ToJSON";
    Snippets.Add() = "Tutorial0";
    Snippets.Add() = "Tutorial1";
    Snippets.Add() = "Tutorial2";
    Snippets.Add() = "Tutorial3";
  }

  if(ArgumentCount != 2)
  {
    C::Out() >> "Usage: Snippets [name-of-snippet]";
    C::Out() >> "Available Snippets:";
    C::Out() >> " * All (runs every snippet)";
    for(count i = 0; i < Snippets.n(); i++)
      C::Out() >> " * " << Snippets[i];
    return AutoRelease<Console>();
  }

  Array<String> SnippetsToRun;
  {
    String Snippet = String(Arguments[1]).ToLower();
    if(Snippet == "all")
      SnippetsToRun = Snippets;
    else
      SnippetsToRun.Add() = Snippet;
  }

  for(count i = 0; i < SnippetsToRun.n(); i++)
  {
    String Snippet = SnippetsToRun[i].ToLower();
    C::Out() << "";
    C::Bold();
    C::Out() >> Snippet << ":";
    C::Reset();
    if(Snippet == "blankpage") SnippetBlankPage();
    if(Snippet == "color") SnippetColor();
    if(Snippet == "drawable") SnippetDrawable();
    if(Snippet == "geometry") SnippetGeometry();
    if(Snippet == "graphsyntax") SnippetGraphSyntax();
    if(Snippet == "image") SnippetImage();
    if(Snippet == "midi") SnippetMIDI();
    if(Snippet == "segmentedhull") SnippetSegmentedHull();
    if(Snippet == "spiral") SnippetSpiral();
    if(Snippet == "springdemo") SnippetSpringDemo();
    if(Snippet == "springs") SnippetSprings();
    if(Snippet == "text") SnippetText();
    if(Snippet == "tojson") SnippetToJSON();
    if(Snippet == "tutorial0") SnippetTutorial0();
    if(Snippet == "tutorial1") SnippetTutorial1();
    if(Snippet == "tutorial2") SnippetTutorial2();
    if(Snippet == "tutorial3") SnippetTutorial3();
  }
  return AutoRelease<Console>();
}
