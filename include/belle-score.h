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

#ifndef BELLE_ENGRAVER_SCORE_H
#define BELLE_ENGRAVER_SCORE_H

namespace BELLE_NAMESPACE
{
  //Page of the score
  class Page : public Canvas
  {
    ///Systems to be painted on this page.
    List<Pointer<const Music> > Systems;

    ///Positions of systems on this page.
    List<Vector> Positions;

    public:

    ///Default constructor
    Page() {}

    ///Sets the systems and their positions for this page.
    void SetSystemsAndPositions(List<Pointer<const Music> > Systems_,
      List<Vector> Positions_)
    {
      Systems = Systems_;
      Positions = Positions_;
    }

    ///Gets the font used by the first system.
    Font GetFont()
    {
      if(!Systems.n())
        return Font();

      Pointer<Stamp> IslandStamp = Systems.a()->Root()->Label.Stamp().Object();
      Pointer<Value::ConstReference> H =
        Systems.a()->Root()->Label.GetState("HouseStyle", "Global").Object();
      Pointer<const Font> NotationFont = H->Get()["NotationFont"].ConstObject();

      if(!NotationFont)
        return Font();

      return *NotationFont;
    }

    ///Returns the number of systems.
    count n()
    {
      return Systems.n();
    }

    ///Returns the music graph to the given system.
    Pointer<const Music> ith(count i)
    {
      return (i >= 0 && i < n()) ? Systems[i] : Pointer<const Music>();
    }

    ///Returns a reference to the pointer for the system at the given index.
    Pointer<const Music>& Replace(count i) {return Systems[i];}

    public:

    ///Callback to paint the systems to the page.
    virtual void Paint(Painter& Painter, Portfolio& Portfolio);

    virtual ~Page();
  };

  class Score : public Portfolio
  {
    friend class Page;

    ///The font used for engraving.
    Font NotationFont;

    ///List of music graph pointers, each one representing a single system.
    List<Pointer<Music> > Systems;

    ///Various system width metrics reported by engraver.
    Value SystemWidths;

    ///The left of the first system in inches.
    number FirstSystemLeft;

    ///The left of the systems in inches.
    number SystemLeft;

    ///The width of the systems in inches.
    number SystemWidth;

    ///The space height in inches.
    number SpaceHeight;

    public:

    ///Initializes the score with no systems and default parameters.
    Score()
    {
      Clear();
    }

    ///Returns a new page. This may be overridden.
    virtual Page* NewPage()
    {
      return new Page;
    }

    ///Clears the score of all its systems and settings.
    void Clear()
    {
      NotationFont = Font();
      Systems.RemoveAll();
      FirstSystemLeft = 0.675f;
      SystemLeft = 0.675f;
      SystemWidth = 7.25f;
      SpaceHeight = RastralSize::Inches(6);
    }

    ///Initializes the font and font data.
    void InitializeFont(Font NotationFont_)
    {
      NotationFont = NotationFont_;
    }

    ///Returns the score font.
    Font GetFont() const
    {
      return NotationFont;
    }

    ///Adds a system to the end.
    void AddSystem(Pointer<Music> SystemGraph)
    {
      if(!SystemGraph)
        return;
      Systems.Add(SystemGraph);
    }

    ///Inserts the given system after the index.
    void InsertSystemAfter(Pointer<Music> SystemGraph,
      count ElementBefore)
    {
      if(!SystemGraph)
        return;
      Systems.InsertAfter(SystemGraph, ElementBefore);
    }

    ///Inserts the given system before the index.
    void InsertSystemBefore(Pointer<Music> SystemGraph,
      count ElementAfter)
    {
      if(!SystemGraph)
        return;
      Systems.InsertBefore(SystemGraph, ElementAfter);
    }

    ///Removes the given system by index.
    void RemoveSystem(count Index)
    {
      Systems.Remove(Index);
    }

    ///Runs the auto-correct module on the score.
    void Autocorrect()
    {
      AutocorrectScore(Systems);
    }

    ///Returns the number of systems.
    count n() const
    {
      return Systems.n();
    }

    ///Returns the music graph to the given system.
    Pointer<const Music> ith(count i) const
    {
      return Systems[i];
    }

    ///Returns the music graph to the given system.
    Pointer<Music> ith(count i)
    {
      return Systems[i];
    }

    ///Sets the left of the system in inches.
    void SetSystemLeft(number SystemLeftInInches)
    {
      FirstSystemLeft = SystemLeft = SystemLeftInInches;
    }

    ///Sets the left of the system in inches.
    void SetSystemLeft(number FirstSystemLeftInInches,
      number SystemLeftInInches)
    {
      FirstSystemLeft = FirstSystemLeftInInches;
      SystemLeft = SystemLeftInInches;
    }

    ///Sets the width of the system in inches.
    void SetSystemWidth(number SystemWidthInInches)
    {
      SystemWidth = SystemWidthInInches;
    }

    ///Gets the system width in inches.
    number GetSystemWidth() const
    {
      return SystemWidth;
    }

    ///Sets the height of a standard staff-space in inches.
    void SetSpaceHeight(number SpaceHeightInInches)
    {
      SpaceHeight = SpaceHeightInInches;
    }

    class Progress
    {
      public:
      Progress() {}
      virtual void Update(number Fraction, String Information) = 0;
      virtual ~Progress();
    };

    ///Engraves each of the systems.
    void Engrave(bool ShowProgress = false, Progress* ProgressObject = 0,
      bool JustifyWithAtLeastMinimumWidth = true,
      number MinimumWidthScale = 0.f)
    {
      if(not NotationFont.GetTypeface(Font::Notation) or
        not NotationFont.GetTypeface(Font::Regular) or
        not NotationFont.GetTypeface(Font::Italic))
      {
        C::Error() >> "There are missing typefaces. Make sure InitializeFont()";
        C::Error() >> "has been called with a font containing regular, italic,";
        C::Error() >> "and notation typefaces.";
        return;
      }
      SystemWidths.Clear();
      for(count i = 0; i < Systems.n(); i++)
      {
        if(ProgressObject)
          ProgressObject->Update(number(i) / number(Systems.n()),
            String("Engraving system ") + String(i + 1) + String(" of ") +
            String(Systems.n()));
        System::SetHouseStyle(Systems[i],
          HouseStyle::Create(NotationFont));
        System::SetDimensions(Systems[i], SystemWidth - (i ? number(0) :
          FirstSystemLeft - SystemLeft), SpaceHeight,
          JustifyWithAtLeastMinimumWidth, MinimumWidthScale);
        if(ShowProgress)
          C::Out() >> "Engraving system " << String(i + 1) << ": " <<
            System::GetLabel(Systems[i]);
        SystemWidths.Add() = System::Engrave(Systems[i]);
      }
      if(ProgressObject)
        ProgressObject->Update(1.f, "Preparing display");
    }

    ///Creates pages on the score and lays out the pages accordingly.
    bool Layout(Inches PaperSize = Paper::Letter(),
      number InchesTopMargin = 1.f, number InchesBottomMargin = 1.f,
      number SpacesStaffToStaffDistance = 10.f,
      number SpacesMinimumSystemToSystem = 15.f,
      number SpacesMaximumSystemToSystem = 20.f,
      number InchesExtraFirstPageTopMargin = 0.f,
      number InchesExtraFirstPageBottomMargin = 0.f)
    {
      //Remove existing canvases.
      Canvases.RemoveAll();

      //Create pages with the maximum number of systems give the parameters.
      count StartSystem = 0;
      count PageNumber = 0;
      while(StartSystem < Systems.n())
      {
        PageNumber++;
        List<Pointer<const Music> > PageSpecificSystems;
        List<Vector> SystemPositions;

        //Determine the maximum number of systems that can be placed on page.
        number LargestWidth = SystemWidth;
        {
          List<Pointer<const Music> > SystemsToTry;
          for(count i = StartSystem; i < Systems.n(); i++)
          {
            //Add the next available system.
            SystemsToTry.Add() = Systems[i];
            LargestWidth = Max(LargestWidth,
              +SystemWidths[i]["EngravedSpaceWidth"]);

            number TopMargin = InchesTopMargin;
            number BottomMargin = InchesBottomMargin;
            if(PageNumber == 1)
            {
              TopMargin += InchesExtraFirstPageTopMargin;
              BottomMargin += InchesExtraFirstPageBottomMargin;
            }

            //See if the list of systems can be spaced given the parameters.
            List<number> Positions = System::SpaceSystems(SystemsToTry,
              BottomMargin, PaperSize.y - TopMargin, SpaceHeight,
              SpacesStaffToStaffDistance, SpacesMinimumSystemToSystem,
              SpacesMaximumSystemToSystem);

            //If too many systems were placed on the page, then break.
            if(!Positions.n())
              break;

            //Save the successful spacing for later.
            PageSpecificSystems = SystemsToTry;
            SystemPositions.RemoveAll();
            for(count j = 0; j < Positions.n(); j++)
              SystemPositions.Add() = Vector(StartSystem or j ? SystemLeft :
                FirstSystemLeft, Positions[j]);
          }

          //Skip any problematic systems.
          if(!PageSpecificSystems.n())
          {
            C::Out() >> "Warning: skipping system '" <<
            System::GetLabel(Systems[StartSystem]) << "' (" << StartSystem <<
              ") due to layout constraints";
            StartSystem++;
            continue;
          }
        }

        //Create the page and add the systems and system positions.
        Page* CurrentPage = NewPage();
        Canvases.Add() = CurrentPage;
        Canvases.z()->Dimensions = PaperSize;

        //Adjust paper size for larger systems.
        {
          number Adjustment = Max(LargestWidth - SystemWidth, number(0));
          if(Adjustment > 0.f)
          {
            C::Out() >> "Note: increasing width of page " << Canvases.n() <<
              " by " << Adjustment << " inches to accomodate overlong system.";
            Canvases.z()->Dimensions += Inches(Adjustment, number(0));
          }
        }
        CurrentPage->SetSystemsAndPositions(PageSpecificSystems,
          SystemPositions);

        //Go to the next available system.
        StartSystem += PageSpecificSystems.n();
      }
      if(!Canvases.n())
        Canvases.Add() = NewPage();

      return true;
    }

    ///Returns the list of const systems.
    List<Pointer<const Music> > ConstSystems() const
    {
      List<Pointer<const Music> > s;
      for(count i = 0; i < Systems.n(); i++)
        s.Add() = Systems[i].Const();
      return s;
    }

    ///Returns the list of mutable systems.
    List<Pointer<Music> > MutableSystems()
    {
      return Systems;
    }

    void Wrap(number MaximumWidth, number RelaxFactor = 1.5f,
      String ForceBreaks = "")
    {
      C::Out() >> "Wrapping...";
      const number CostPower = 2.f;
      Engrave(false, 0, true, RelaxFactor);
      List<Pointer<Music> > NewScoreSystems;
      for(Counter s; s.z(Systems); s++)
      {
        Pointer<const Music> System = Systems[s].Const();
        Value PotentialBreaks = WrapPotentialBreaks(System);
        List<VectorInt> BestBreaks = WrapCalculateOptimalBreaks(PotentialBreaks,
          MaximumWidth, MaximumWidth, CostPower);
        if(ForceBreaks == "info")
          C::Out() >> "Default breaks: " << BestBreaks;
        else if(ForceBreaks)
        {
          Value Breaks = JSON::Import(ForceBreaks);
          count Left = 0;
          BestBreaks.RemoveAll();
          for(count i = 0; Breaks.IsArray() and i < Breaks.n(); i++)
          {
            count Right = Breaks[i].AsCount();
            BestBreaks.Add(VectorInt(Left, Right - 1));
            Left = Right;
          }
        }
        List<Pointer<Music> > NewSystems = WrapBreakGraph(
          System, PotentialBreaks, BestBreaks);
        for(Counter i; i.z(NewSystems); i++)
          NewScoreSystems.Add() = NewSystems[i];
      }
      Systems = NewScoreSystems;
      C::Out() >> "Finished wrap";
    }

    virtual ~Score();
  };
#ifdef BELLE_COMPILE_INLINE
  Page::~Page() {}
  Score::~Score() {}

  void Page::Paint(Painter& Painter, Portfolio& Portfolio)
  {
    (void)Portfolio;
    for(count i = 0; i < Systems.n(); i++)
    {
      if(not System::Paint(Systems[i], Painter, Positions[i]))
      {
#if 0
        Score& ScoreObject = dynamic_cast<Score&>(Portfolio);
        Painter.Draw("This system could not be painted.",
          ScoreObject.NotationFont, 12.f, Font::Italic,
          Text::Justifications::Center, Dimensions.x, Colors::Black(),
          Affine::Translate(Vector(0.f, Positions[i].y)));
#endif
      }
    }
  }
#endif
}
#endif
