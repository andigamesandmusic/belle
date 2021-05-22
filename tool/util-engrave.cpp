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

#define BELLE_COMPILE_INLINE
#define PRIM_WITH_DIRECTORY
#define PRIM_WITH_TIMER
#include "belle.h"

//Use the example helper
#include "belle-helper.h"

//Import the belle namespace for convenience
using namespace belle;

class CustomScore : public Score
{
  public:

  bool IsFilterTestSuite; PRIM_PAD(bool)
  bool IsTestSuite; PRIM_PAD(bool)
  bool HidePageNumbers; PRIM_PAD(bool)

  number PrePaintTime;

  CustomScore() : IsFilterTestSuite(false), IsTestSuite(false),
    HidePageNumbers(false), PrePaintTime(0.f)
  {
  }

  virtual Page* NewPage();

  virtual ~CustomScore();
};

class CustomPage : public Page
{
  public:

  void DrawTitle(Painter& Painter, Portfolio& Portfolio)
  {
    CustomScore& ScoreRef = dynamic_cast<CustomScore&>(Portfolio);
    ScopedAffine a(Painter, Affine::Translate(Vector(0.f, -1.f)));
    ScopedAffine b(Painter, Affine::Translate(Vector(1.8f, 8.3f)));
    Font f = ScoreRef.GetFont();
    Painter.Draw("Belle Music Engraver", f, 36.f,
      Font::Bold, Text::Justifications::Left, 0.f);
    String Name = "Comprehensive Test Suite";
    Painter.Draw(Name, f, 24.f, Font::Regular,
      Text::Justifications::Left, 0.f, Colors::Black(),
      Affine::Translate(Vector(0.f, -0.5f)));
    {
      String Timestamp = Time();
      Timestamp.Replace(" (Daylight Savings)", "");
      String GeneratedOn = "Generated ";
      GeneratedOn << Timestamp;
      GeneratedOn << " in " << Truncate(ScoreRef.PrePaintTime, number(0.01f)) <<
        " seconds";
      Painter.Draw(GeneratedOn, f, 10.f, Font::Regular,
        Text::Justifications::Right, 6.7f, Colors::Black(),
        Affine::Translate(Vector(-1.f, -1.f)));
    }
  }

  void DrawLines(Painter& Painter)
  {
    ScopedAffine a(Painter, Affine::Translate(Vector(0.f, -1.f)));
    Path p;
    Shapes::AddLine(p, Vector(1.5f, 7.25f), Vector(1.5f, 9.f), 0.01f);
    Shapes::AddLine(p, Vector(1.5f, 7.5f), Vector(7.5f, 7.5f), 0.01f);
    Painter.Draw(p);
  }

  void DrawTitlePage(Painter& Painter, Portfolio& Portfolio)
  {
    DrawLines(Painter);
    DrawTitle(Painter, Portfolio);
  }

  void DrawPageNumber(Painter& Painter, Portfolio& Portfolio)
  {
    if(Portfolio.Canvases.n() < 2)
      return;
    ScopedAffine a(Painter, Affine::Translate(Vector(0.f, 0.5f)));
    Font f = dynamic_cast<Score&>(Portfolio).GetFont();
    Painter.Draw(String(Painter.GetPageNumber() + 1), f, 12.f,
      Font::Italic, Text::Justifications::Center,
      Inches(Portfolio.Canvases[Painter.GetPageNumber()]->Dimensions).x);
  }

  virtual void Paint(Painter& Painter, Portfolio& Portfolio)
  {
    bool IsTestSuite = dynamic_cast<CustomScore&>(Portfolio).IsTestSuite;
    if(Painter.GetPageNumber() > 0 or not IsTestSuite)
    {
      Page::Paint(Painter, Portfolio);
      if(not dynamic_cast<CustomScore&>(Portfolio).HidePageNumbers)
        DrawPageNumber(Painter, Portfolio);
    }
    else
      DrawTitlePage(Painter, Portfolio);
  }

  virtual ~CustomPage();
};

CustomPage::~CustomPage() {}
CustomScore::~CustomScore() {}
Page* CustomScore::NewPage() {return new CustomPage;}

//Pre-engrave debug hook

static void PreEngrave(Pointer<const Music> M, bool HideLabels)
{
  if(HideLabels)
    System::SetLabel(M, "");
}

//Post-engrave debug hook
static void PostEngrave(Pointer<const Music> M)
{
  (void)M;
}

int main(int ArgumentCount, const char** Arguments)
{
  //Release the console singleton automatically.
  AutoRelease<Console> ReleasePool;

  //Start timer for total time.
  Timer TotalTime, PrePaintTime;
  TotalTime.Start();
  PrePaintTime.Start();

  //Read the parameters.
  Array<String> Parameters;
  for(count i = 1; i < ArgumentCount; i++)
    Parameters.Add() = Arguments[i];

  //Show usage options.
  if(Parameters.n() == 0)
  {
    C::Out() >> "Usage: Engrave [score-files] [options]";
    C::Out()++;
    C::Out() >> "Pre-Engrave Options:";
    C::Out() >> " --autocorrect Invokes score autocorrection";
    C::Out() >> " --export    Export XML scores systems";
    C::Out() >> " --filter   'filter1;filter2;...'";
    C::Out() >> " --generate '[[\"gen1\",args,...],[\"gen2\",args,...],...]'";
    C::Out() >> "             Invoke specific generators by name and arguments";
    C::Out() >> " --incipitpdf Create PDF incipit of each system";
    C::Out() >> " --incipitsvg Create SVG incipit of each system";
    C::Out() >> " --list      Lists usage for each generator";
    C::Out() >> " --publish   Publish to score JSON";
    C::Out() >> " --repeat [integer, default = 1, max = 999]";
    C::Out() >> "             Repeat each generator a certain number of times";
    C::Out() >> " --seed [integer]";
    C::Out() >> "             Set the seed used for the generators";
    C::Out() >> " --test      Append test suite systems";
    C::Out() >> " --titles    Force display of system label titles";
    C::Out() >> " --wrap      Wrap long systems";
    C::Out()++;
    C::Out() >> "Engrave Options:";
    C::Out() >> " --bottommargin   [number] Inches for bottom margin";
    C::Out() >> " --firstmargin [number] Inches for extra top margin on page 1";
    C::Out() >> " --firstbottommargin [number] Inches for extra bottom margin";
    C::Out() >> " --forcebreaks    info|[array] Force wrap breaks";
    C::Out() >> " --hidepagenumbers         Hides page numbers";
    C::Out() >> " --longsystems             Retypeset long systems to length";
    C::Out() >> " --maxsystemdistancerelative [1.5] Max system distance scalar";
    C::Out() >> " --minimumwidth   [1.0]    Retypeset system to minimum width";
    C::Out() >> "                  [number] Scale minimum width";
    C::Out() >> " --pagewidth     [number]  Width of page in inches";
    C::Out() >> " --pageheight     [number] Height of page in inches";
    C::Out() >> " --rastral        [0...8]  Rastral size of staff";
    C::Out() >> " --systemwidth    [number] Width of system in inches";
    C::Out() >> " --staffdistance  [number] Staff distance in staff spaces";
    C::Out() >> " --staffspaceheight  [number] Height of staff space in inches";
    C::Out() >> " --systemdistance [number] System distance in staff spaces";
    C::Out() >> " --systemleft     [number] Left of system in inches";
    C::Out() >> " --firstsystemleft [number] Left of first system in inches";
    C::Out() >> " --topmargin [number] Inches for top margin";
    C::Out()++;
    C::Out() >> "Post-Engrave Options:";
    C::Out() >> " --contexts  Shows contexts for data marker labels";
    C::Out() >> " --midi      Generates MIDI file of score";
    C::Out() >> " --open      Opens score in PDF viewer after creation";
    C::Out() >> " --spaceheight [size in pixels, default = 6]";
    C::Out() >> "             Determines the PPI from the space height";
    C::Out() >> " --rasterize Rasterizes the PDF (loads and views faster)";
    C::Out()++;
  }
  
  //Score for publishing
  Value PublishedScore;

  //Create a score.
  CustomScore MyScore;

  //Adjust rastral size.
  if(count ParameterIndex = Parameters.Search("--rastral") + 1)
  {
    count RastralSize = count(Parameters[ParameterIndex].ToNumber());
    number RastralSizeInches = RastralSize::Inches(RastralSize);
    C::Out() >> "Setting rastral size " << RastralSize << " (" <<
      RastralSizeInches << ")";
    MyScore.SetSpaceHeight(RastralSizeInches);
  }

  //Adjust space height size.
  if(count ParameterIndex = Parameters.Search("--staffspaceheight") + 1)
  {
    number StaffSpaceHeight = Parameters[ParameterIndex].ToNumber();
    MyScore.SetSpaceHeight(StaffSpaceHeight);
  }

  number PageWidth = 8.5f;
  if(count ParameterIndex = Parameters.Search("--pagewidth") + 1)
  {
    PageWidth = Clip(Parameters[ParameterIndex].ToNumber(),
      number(2.f), number(1000.f));
  }

  number SystemLeft = 0.625f / 2.f;
  if(count ParameterIndex = Parameters.Search("--systemleft") + 1)
  {
    SystemLeft = Clip(Parameters[ParameterIndex].ToNumber(),
      number(0.f), number(1000.f));
  }

  number FirstSystemLeft = SystemLeft + 0.4f;
  if(count ParameterIndex = Parameters.Search("--firstsystemleft") + 1)
  {
    FirstSystemLeft = Clip(Parameters[ParameterIndex].ToNumber(),
      number(0.f), number(1000.f));
  }

  if(count ParameterIndex = Parameters.Search("--systemwidth") + 1)
  {
    number Width = Clip(Parameters[ParameterIndex].ToNumber(),
      number(2.f), number(1000.f));
    MyScore.SetSystemWidth(Width);
    MyScore.SetSystemLeft(FirstSystemLeft, SystemLeft);
  }

  //Adjust page height.
  number PageHeight = 11.f;
  if(count ParameterIndex = Parameters.Search("--pageheight") + 1)
  {
    PageHeight = Clip(Parameters[ParameterIndex].ToNumber(),
      number(2.f), number(1000.f));
  }

  //Adjust staff to staff.
  number StaffToStaffDistance = 12.f;
  if(count ParameterIndex = Parameters.Search("--staffdistance") + 1)
  {
    StaffToStaffDistance = Clip(Parameters[ParameterIndex].ToNumber(),
      number(4.f), number(100.f));
  }

  //Adjust system to system.
  number SystemToSystemDistance = 25.f;
  if(count ParameterIndex = Parameters.Search("--systemdistance") + 1)
  {
    SystemToSystemDistance = Clip(Parameters[ParameterIndex].ToNumber(),
      number(4.f), number(100.f));
  }

  Font NotationFont = Helper::ImportNotationFont();

  Timer TimeToLoadInput;
  String OutFileStem = "output";
  {
    TimeToLoadInput.Start();

    //Show usage for the generators.
    if(Parameters.Contains("--list"))
    {
      SystemGenerators::ShowAllGenerators();
      return 0;
    }

    //Add systems for the test suite.
    if(Parameters.Contains("--test"))
    {
      if(Parameters.Contains("--test"))
        TestSuite::AppendAll(MyScore);

      //Add the test suite.
      Array<String> Files;
      Directory::Files("./resources", Files, Directory::JustFiles, false);
      if(!Files.n())
        Directory::Files("../resources", Files, Directory::JustFiles, false);
      if(!Files.n())
        Directory::Files("../../resources", Files, Directory::JustFiles, false);
#ifdef BELLE_DEBUG_PROTOTYPE_TEST_SUITE
      Files.Clear();
#endif

      Array<String> FilesToAdd;
      for(count i = 0; i < Files.n(); i++)
      {
        if(Parameters.Contains("--test") && Files[i].Contains("test-suite-"))
          FilesToAdd.Add() = Files[i];
      }
      for(count i = 0; i < Parameters.n(); i++)
        FilesToAdd.Add() = Parameters[i];

      Parameters = FilesToAdd;
    }

    /*(If space-height parameter is not found, then the parameter index will
    be zero and bypass the if-body since search returns -1 for not found.)*/
    if(count GenerateParameterIndex = Parameters.Search("--generate") + 1)
    {
      uint32 Seed = 123;
      if(count SeedParameterIndex = Parameters.Search("--seed") + 1)
        Seed = uint32(Parameters[SeedParameterIndex].ToNumber());
      C::Out() >> "Generating with seed " << integer(Seed);
      Random R(Seed);
      Value Generators = JSON::Import(Parameters[GenerateParameterIndex]);
      if(!Generators.IsArray())
      {
        C::Red();
        C::Error() >> "The JSON syntax of the generators list is incorrect.";
        C::Reset();
        return 1;
      }

      count Repeat = 1;
      if(count RepeatParameterIndex = Parameters.Search("--repeat") + 1)
      {
        count x = count(Parameters[RepeatParameterIndex].ToNumber());
        if(x > 1 && x < 1000)
          Repeat = x;
      }

      C::Out() >> "Invoking each generator " << Repeat <<
        (Repeat == 1 ? " time" : " times");

      for(count i = 0; i < Generators.n(); i++)
      {
        Value Generator = Generators[i];
        C::Out() >> " * " << Generator;
        for(count r = 0; r < Repeat; r++)
        {
          if(r % 20 == 0)
            C::Out() >> "   ";
          C::Out() << "=";
          if(!SystemGenerators::Generate(R, MyScore, Generator))
            return 1;
        }
      }
    }

    //Load the systems into the score.
    {
      //Treat each parameter as a score filename until an option is encountered.
      String FirstFilename;
      for(count i = 0; i < Parameters.n(); i++)
      {
        //Stop treating input parameters as score files at the first option.
        if(Parameters[i].StartsWith("--"))
          break;

        //Remember the name of the first score file to use as the file stem.
        if(!FirstFilename)
          FirstFilename = Parameters[i];

        //Read in the file.
        C::Out() >> "Reading file " << Parameters[i] << "...";
        String InputData;
        File::Read(Parameters[i], InputData);

        if(Parameters.Contains("--publish") and i == 0)
        {
          String JSONMetadataFile = Parameters[i];
          JSONMetadataFile.Replace(".xml", ".json");
          if(JSONMetadataFile.Contains("Level"))
          {
            JSONMetadataFile.Erase(JSONMetadataFile.Find("Level"),
              JSONMetadataFile.n() - 1);
            JSONMetadataFile << ".json";
          }
          String JSONMetadata = File::Read(JSONMetadataFile);
          Value JSONValue;
          if(JSONMetadata.Contains("\"title\""))
            JSONValue = JSON::Import(JSONMetadata);
          else
          {
            C::Error() >> "Could not read corresponding metadata at:";
            C::Error() >> JSONMetadataFile;
            C::Error() >> "Aborting.";
            return 1;
          }
          PublishedScore["metadata"] = JSONValue;
        }

        if(Parameters.Contains("--incipitsvg") or
          Parameters.Contains("--incipitpdf"))
        {
          bool UseSVG = Parameters.Contains("--incipitsvg");
          String SVGFilename = Parameters[i] + (UseSVG ? ".svg" : ".pdf");
          Pointer<Music> MusicalIncipit;
          MusicalIncipit.New()->ImportXML(ConvertToXML(InputData));
          if(String SVGOutput = RenderIncipitAsSVG(
            MusicalIncipit, NotationFont, 6.f, 0.1f,
            0.065f, not UseSVG))
          {
            File::Write(SVGFilename, SVGOutput);
            C::Green(); C::Out() >> "Wrote:         ";
            C::Blue(); C::Out() << SVGFilename; C::Reset();
          }
        }

        //Import the score file and add it to the score.
        Pointer<Music> M;
        Array<byte> MusicXMLValidationZip;
        //Resources::Load("MusicXMLValidation.zip", MusicXMLValidationZip);
        M.New()->ImportXML(ConvertToXML(InputData, MusicXMLValidationZip));
        UnlinkUnnecessaryInstantwiseEdges(*M);

        System::SpaceStaves(M, StaffToStaffDistance);
        if(M->Nodes().n())
          MyScore.AddSystem(M);

        {
          String Label = Parameters[i].ToFilename();
          System::SetLabel(M, Label);
        }
      }

      //If there was a score file then create an output file stem from its name.
      if(FirstFilename && !Parameters.Contains("--test"))
      {
        OutFileStem = FirstFilename;
        OutFileStem.Replace(".txt", "");
        OutFileStem.Replace(".xml", "");
      }
      if(Parameters.Contains("--publish"))
        OutFileStem << "Published";
    }

    TimeToLoadInput.Stop();
  }

  if(not MyScore.n())
  {
    C::Error() >> "There are no systems to engrave.";
    return 1;
  }

  Timer TimeToInitialize;
  {
    TimeToInitialize.Start();
    MyScore.InitializeFont(NotationFont);
    TimeToInitialize.Stop();
  }

  if(Parameters.Contains("--autocorrect"))
  {
    Timer TimeToAutocorrect;
    TimeToAutocorrect.Start();
    MyScore.Autocorrect();
    C::Green();
    C::Error() >> "Autocorrected in: " << TimeToAutocorrect.Stop();
    C::Error()++;
    C::Reset();
  }

  if(Parameters.Contains("--wrap"))
  {
    number RelaxFactor = 1.5f;
    if(count MinimumWidthIndex = Parameters.Search("--minimumwidth") + 1)
      RelaxFactor = Clip(Parameters[MinimumWidthIndex].ToNumber(),
        number(0.5f), number(4.f));
    String ForceBreaks;
    if(count ForceBreaksIndex = Parameters.Search("--forcebreaks") + 1)
      ForceBreaks = Parameters[ForceBreaksIndex];

    MyScore.Wrap(MyScore.GetSystemWidth(), RelaxFactor, ForceBreaks);
  }

  Timer TimeToEngrave;
  {
    TimeToEngrave.Start();

    for(count i = 0; i < MyScore.n(); i++)
    {
      if(count FilterParameterIndex = Parameters.Search("--filter") + 1)
      {
        List<String> Filters = Parameters[FilterParameterIndex].Tokenize("+");
        List<String> FilterArguments;
        Pointer<Music> FilteredSystem = MyScore.ith(i);
        for(count j = 0; j < Filters.n(); j++)
        {
          FilterArguments = Filters[j].Tokenize(",");
          String CommandName = FilterArguments.a();

          C::Out() >> "Processing filter " << FilterArguments.a();

          if(CommandName == "pitch-collapse")
          {
            bool Staff2 = false;
            if(FilterArguments.n() == 2)
            {
              if (String(FilterArguments[1]) == "1") Staff2 = false;
              else if (String(FilterArguments[1]) == "2") Staff2 = true;
              else Staff2 = false;

              PitchCollapse(*FilteredSystem, Staff2, Music::Node(),
                Music::Node());
            }

            else
              PitchCollapse(*FilteredSystem, Staff2, Music::Node(),
                Music::Node());
          }
          else if(CommandName == "dot")
          {
            if(FilterArguments.n() == 5)
            {
              Ratio Initial = Ratio(String(FilterArguments[1]));
              Ratio Base = Ratio(String(FilterArguments[2]));

              bool Dot;
              if (String(FilterArguments[3]) == "dot1") Dot = true;
              else if (String(FilterArguments[3]) == "dot2") Dot = false;
              else Dot = true;

              bool Staff2;
              if (String(FilterArguments[4]) == "1") Staff2 = false;
              else if (String(FilterArguments[4]) == "2") Staff2 = true;
              else Staff2 = false;

              DottedRhythmicManipulation(*FilteredSystem, Initial,
                Base, Dot, Staff2, Music::Node(), Music::Node());
            }

            else
              DottedRhythmicManipulation(*FilteredSystem, Ratio(1, 4),
                Ratio(1, 64),true, false, Music::Node(), Music::Node());
          }
          else if(CommandName == "interval-transpose")
          {
            if(FilterArguments.n() == 5)
            {
              count StaffNumber = count(String(Arguments[1]).ToNumber());

              mica::Concept IntervalTranspose =
                mica::named(String(FilterArguments[2]));
              mica::Concept Direction =
                mica::named(String(FilterArguments[3]));
              mica::Concept Octaves =
                mica::Concept(Ratio(String(FilterArguments[4])));

              IntervalTransposition(FilteredSystem, StaffNumber,
                IntervalTranspose, Direction, Octaves);
            }

            else
              IntervalTransposition(FilteredSystem, 1, mica::PerfectFifth,
                mica::Above, mica::Concept(Ratio(0)));
          }
          else if(CommandName == "undot")
          {
            if(FilterArguments.n() == 4)
            {
              Ratio Initial = Ratio(String(FilterArguments[1]));
              Ratio Base = Ratio(String(FilterArguments[2]));

              bool Staff2;
              if (String(Arguments[3]) == "1") Staff2 = false;
              else if (String(Arguments[3]) == "2") Staff2 = true;
              else Staff2 = false;

              UndotRhythmicManipulation(*FilteredSystem, Initial, Base, Staff2,
                Music::Node(), Music::Node());
            }

            else
              UndotRhythmicManipulation(*FilteredSystem, Ratio(1, 4),
                Ratio(1, 64), false, Music::Node(), Music::Node());
          }
          else if(CommandName == "harmonize")
          {
            List<List<mica::Concept> > HarmonizationList;
            List<Ratio> NoteValueList;
            if(FilterArguments.n() > 2)
            {
              Ratio NoteValue = Ratio(String(FilterArguments[1]));
              NoteValueList.Push(NoteValue);
              count StaffNumber = 1;

              for(count k = 2; k < FilterArguments.n(); k++)
              {
                if(String(FilterArguments[k]) == "Ch:")
                {
                  HarmonizationList.Push(List<mica::Concept>());
                  continue;
                }
                HarmonizationList.z().Push(mica::named(
                  String(FilterArguments[k])));
              }
              Harmonize(FilteredSystem, HarmonizationList, NoteValueList,
                StaffNumber);
            }

            else
            {
              HarmonizationList.Push(List<mica::Concept>());
              NoteValueList.Push(Ratio(3, 8));
              HarmonizationList.z().Push(mica::E4);
              HarmonizationList.z().Push(mica::C4);
              Harmonize(FilteredSystem, HarmonizationList, NoteValueList, 1);
            }
          }
          else if(CommandName == "rebeam")
          {
            MusicXMLBeaming(FilteredSystem);
          }
        }
      }
      if(Parameters.Contains("--export"))
      {
        String f = "exported-";
        if(i < 10 and MyScore.n() >= 10) f << "0";
        if(i < 100 and MyScore.n() >= 100) f << "0";
        if(i < 1000 and MyScore.n() >= 1000) f << "0";
        f << i << ".xml";
        File::Write(f, MyScore.ith(i)->ExportXML());
      }
      if(Parameters.Contains("--publish"))
      {
        PublishedScore["systems"].Add() = MyScore.ith(i)->ExportXML();
        if(i == 0)
        {
          Pointer<Music> MusicalIncipit;
          MusicalIncipit.New()->ImportXML(
            PublishedScore["systems"][0].AsString());
          String SVGOutput = RenderIncipitAsSVG(
            MusicalIncipit, NotationFont, 6.f, 0.1f,
            0.065f, false);
          PublishedScore["incipit"] = SVGOutput;
          PublishedScore["notelist"].NewArray();
        }
      }
      bool HideLabels = not (Parameters.Contains("--test"));
      if(Parameters.Contains("--titles"))
        HideLabels = false;
      PreEngrave(MyScore.ith(i), HideLabels);
    }

    number MinimumWidth = 0.f;
    if(not Parameters.Contains("--wrap"))
    {
      if(count MinimumWidthParameterIndex =
        Parameters.Search("--minimumwidth") + 1)
          MinimumWidth = Parameters[MinimumWidthParameterIndex].ToNumber();
    }

    //Engrave the score.
    MyScore.Engrave(true, 0, Parameters.Contains("--longsystems"),
      MinimumWidth);

    for(count i = 0; i < MyScore.n(); i++)
      PostEngrave(MyScore.ith(i));

    TimeToEngrave.Stop();
  }

  //Get page margins
  number TopMargin = 1.f, BottomMargin = 1.f,
    FirstMargin = 0.f, FirstBottomMargin = 0.f;
  if(count m = Parameters.Search("--topmargin") + 1)
    TopMargin = Parameters[m].ToNumber();
  if(count m = Parameters.Search("--bottommargin") + 1)
    BottomMargin = Parameters[m].ToNumber();
  if(count m = Parameters.Search("--firstmargin") + 1)
    FirstMargin = Parameters[m].ToNumber();
  if(count m = Parameters.Search("--firstbottommargin") + 1)
    FirstBottomMargin = Parameters[m].ToNumber();

  //Layout the pages of the score.
  number MaxSystemDistanceRelative = 1.5f;
  if(count MaxSystemDistanceRelativeIndex =
    Parameters.Search("--maxsystemdistancerelative") + 1)
      MaxSystemDistanceRelative =
        Parameters[MaxSystemDistanceRelativeIndex].ToNumber();
  Inches PageSize(PageWidth, PageHeight);
  MyScore.Layout(PageSize, TopMargin, BottomMargin, StaffToStaffDistance,
    SystemToSystemDistance, SystemToSystemDistance * MaxSystemDistanceRelative,
    FirstMargin, FirstBottomMargin);

  MyScore.IsTestSuite = MyScore.IsFilterTestSuite = false;
#ifndef BELLE_DEBUG_PROTOTYPE_TEST_SUITE
  if(Parameters.Contains("--test"))
  {
    MyScore.IsTestSuite = true;
    MyScore.Canvases.InsertBefore(MyScore.NewPage(), 0);
  }
#endif

  if(Parameters.Contains("--hidepagenumbers"))
    MyScore.HidePageNumbers = true;

  //Set the PDF-specific properties.
  String PDFFile = OutFileStem + ".pdf";
  PDF::Properties PDFSpecificProperties(PDFFile);

  //Write the score to PDF.
  Timer TimeToPaint;
  {
    TimeToPaint.Start();

    C::Out() >> "Painting score...";
    MyScore.PrePaintTime = PrePaintTime.Stop();
    MyScore.Create<PDF>(PDFSpecificProperties);

    TimeToPaint.Stop();
  }

  //Indicate that the PDF has been written out to file.
  C::Green(); C::Out() >> "Wrote:         ";
  C::Blue(); C::Out() << PDFSpecificProperties.Filename; C::Reset();

  //Write out a MIDI file of the score if requested.
  if(Parameters.Contains("--midi"))
  {
    C::Out() >> "Creating MIDI file... (deprecated, skipping)";
#if 0
    String MIDIFile = OutFileStem + ".mid";
    File::Write(MIDIFile, MyScore.CreateMIDI());
    C::Green(); C::Out() >> "Wrote:         ";
    C::Blue(); C::Out() << MIDIFile; C::Reset();
#endif
  }

  /*Convert each page of the PDF to a PNG if requested. This option helps reduce
  the file size of marker-annotated files, since the vector graphics information
  for the text art can consume a lot of space (upwards of 100MB in some cases).
  By rasterizing the file, its size becomes independent of the number of text
  items at the expense of fixing its resolution.*/
  if(Parameters.Contains("--rasterize"))
  {
    String AllPNGs;

    //Rasterize each PDF page to a PNG at 300ppi.
    for(count i = 0; i < MyScore.Canvases.n(); i++)
    {
      C::Out() >> "Rasterizing page " << (i + 1) << " of " <<
        MyScore.Canvases.n() << "...";
      String In = PDFFile;
      In << "[" << i << "]";
      String Out = OutFileStem;
      Out << "-" << i << ".png";
      Helper::ConvertPDFToPNG(In, Out, 300.f, false);
      AllPNGs << Out << " ";
    }

    /*Recombine all of the PNGs back into the PDF thus creating a rasterized
    (no vector graphics) version of the PDF.*/
    {
      C::Out() >> "Recombining rasterized images into PDF...";
      String ConvertArgs = "convert ";
      ConvertArgs << AllPNGs << "-density 300 " << PDFFile << "; rm " <<
       AllPNGs;
      C::Magenta(); C::Out() >> ConvertArgs; C::Reset();
      String Input, Output, Error;
      Shell::PipeInOut(Shell::GetProcessOnPath("bash"), Input, Output,
        Error, "-c", ConvertArgs);
      if(Error)
      {
        C::Red();
        C::Out() >> Error;
        C::Reset();
      }
    }
  }

  if(Parameters.Contains("--publish"))
  {
    String PublishedJSONFilename = OutFileStem + ".json";
    C::Out() >> "Publishing to " << PublishedJSONFilename;
    File::Write(PublishedJSONFilename, JSON::Export(PublishedScore));
  }

  //Open the score if requested.
  if(Parameters.Contains("--open"))
  {
    String Input, Output, Error;
    Shell::PipeInOut(Shell::GetProcessOnPath("open"), Input, Output, Error,
      PDFFile);
  }

  //Report time for various parts of the typesetting.
  {
    C::Out() >> "Time to initialize:                    " <<
      (TimeToInitialize.Elapsed() * 1000.f) << " ms";
    C::Out() >> "Average time to load input per system: " <<
      (TimeToLoadInput.Elapsed() / number(MyScore.n()) * 1000.f) << " ms";
    C::Out() >> "Average time to engrave per system:    " <<
      (TimeToEngrave.Elapsed() / number(MyScore.n()) * 1000.f)  << " ms";
    C::Out() >> "Average time to paint per system:      " <<
      (TimeToPaint.Elapsed() / number(MyScore.n()) * 1000.f)  << " ms";
  }

  return 0;
}
