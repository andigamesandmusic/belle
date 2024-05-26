#define BELLE_COMPILE_INLINE
#include "belle.h"
#include "belle-helper.h"
using namespace belle;

class MeasureScore : public Portfolio
{
  public:

  class Page : public Canvas
  {
    public:
    Array<Box> BoxesToPaint;

    void Paint(Painter& Painter, Portfolio& Portfolio)
    {
      Paint(Painter, dynamic_cast<MeasureScore&>(Portfolio));
    }

    void Paint(Painter& Painter, MeasureScore& MeasureScore)
    {
      (void)MeasureScore;
      for(count i = 0; i < BoxesToPaint.n(); i++)
      {
        number StrokeWidth = 0.01f;
        Box r = BoxesToPaint[i];
        r.Dilate(i ? -StrokeWidth * 2.f : StrokeWidth * 2.f);
        Path p;
        Shapes::AddBox(p, r);
        Painter.SetFill(i % 2 ? Colors::Red() : Colors::Blue());
        if(not i)
          Painter.SetStroke(Colors::Green(), StrokeWidth);
        Painter.Draw(p);
      }
    }

    virtual ~Page();
  };
  virtual ~MeasureScore();
};
MeasureScore::~MeasureScore() {}
MeasureScore::Page::~Page() {}

int main(int ArgumentCount, const char** ArgumentData)
{
  (void)ArgumentCount;
  (void)ArgumentData;
  AutoRelease<Console> ReleasePool;

  List<number> MeasureWidths;
  Random R(123);
  const count Measures = 100;
  for(count i = 0; i < Measures; i++)
  {
    number p = Sqrt(number(i) / number(Measures - 1));
    MeasureWidths.Add() =
      ((R.Between(0.f, 1.f) * 5.f) + 1.f) * (1.f - p)
      + (p * 0.5f);
  }
  C::Out() >> "Measure Widths:" >> MeasureWidths;

  MeasureScore MyMeasureScore;

  for(count p = 0; p < 1; p++)
  {
    const number Height = 0.25f;
    const number MarginSize = 1.f;

    Pointer<MeasureScore::Page> MyPage;
    MyMeasureScore.Canvases.Add() = MyPage.New();
    MyPage->Dimensions = Paper::Portrait(Paper::Letter());
    Box Margin(Vector(), MyPage->Dimensions);
    Margin.Dilate(-MarginSize);
    MyPage->BoxesToPaint.Add() = Margin;

    List<VectorInt> Distributed = WrapDistributeMeasures(
      MeasureWidths, Margin.Width(), Margin.Width(), 2.f);
    for(count i = 0; i < Distributed.n(); i++)
    {
      number Left = 0.f, Right = 0.f;
      for(count j = count(Distributed[i].i());
        j <= count(Distributed[i].j()); j++)
      {
        Right += MeasureWidths[j];
        Box r(Margin.TopLeft() + Vector(Left, -Height * number(i + 1)),
          Margin.TopLeft() + Vector(Right, -Height * number(i)));
        MyPage->BoxesToPaint.Add() = r;
        Left += MeasureWidths[j];
      }
    }
  }

  PDF::Properties PDFSpecificProperties;
  PDFSpecificProperties.Filename = "measure-distribution.pdf";
  MyMeasureScore.Create<PDF>(PDFSpecificProperties);
  C::Out() >> "Wrote PDF to '" << PDFSpecificProperties.Filename << "'.";

  return ReleasePool;
}
