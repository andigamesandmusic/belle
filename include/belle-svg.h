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

#ifndef BELLE_CORE_SVG_H
#define BELLE_CORE_SVG_H

#include "belle-abstracts.h"

namespace BELLE_NAMESPACE
{
  class SVGHelper
  {
    public:

    ///Formats a number for appending as an SVG coordinate.
    static String Format(number Number)
    {
      String s;
      s.Append(float64(Number), 3, false);
      s.EraseEnding(".0");
      if(s == "-0")
        s = "0";
      return s;
    }

    //-------//
    //Helpers//
    //-------//

    //Appends the path data to the string.
    static void AppendPathData(const Path& p, String& Destination)
    {
      for(count j = 0; j < p.n(); j++)
      {
        const Instruction& i = p[j];
        Vector c1 = i.Control1(), c2 = i.Control2(), e = i.End();
        String Spacer = " ";
        if(!j)
          Spacer = "";
        if(i.IsMove())
          Destination << Spacer << "M" << Format(e.x) << "," <<
            Format(e.y);
        else if(i.IsLine())
          Destination << Spacer << "L" << Format(e.x) << "," <<
            Format(e.y);
        else if(i.IsCubic())
          Destination << Spacer << "C" << Format(c1.x) << "," <<
            Format(c1.y) << " " << Format(c2.x) << "," <<
            Format(c2.y) << " " << Format(e.x) << "," <<
            Format(e.y);
        else
          Destination << Spacer << "Z";
      }
    }

    ///Appends a path with transform and glyph information to an SVG string.
    static void AppendPathToSVG(const Path& p, String& Destination,
      Affine Transform = Affine::Unit(), unicode Unicode = 0,
      number AdvanceWidth = 0.f)
    {
      Destination >> "<path";

      if(Unicode)
        Destination << " belle:unicode=\"" <<
          integer(Unicode) << "\"";

      if(Limits<number>::IsNotZero(AdvanceWidth))
        Destination << " belle:advance-width=\"" <<
          AdvanceWidth << "\"";

      if(Transform != Affine::Unit())
        Destination << " transform=\"matrix(" << Transform.a << " " <<
          Transform.b << " " << Transform.c << " " << Transform.d << " " <<
          Transform.e << " " << Transform.f << ")\"";

      Destination << " d=\"";
      AppendPathData(p, Destination);
      Destination << "\"/>";
    }

    /**Imports and appends all paths from an SVG into an existing list of paths.
    Note, this does not currently read in any transform attributes, so the
    result will only be valid for those paths which have no transform set.*/
    static void Import(List<Path>& Paths, const String& SVGData)
    {
      List<Brush> BrushStates;
      Import(Paths, BrushStates, SVGData);
    }

    static Box Bounds(const String& SVGData)
    {
      Box SVGBounds;
      List<String> Start; Start.Add() = "viewBox=\"";
      List<String> End; End.Add() = "\"";
      String::Span Location;
      String BetweenText;
      SVGData.FindBetweenAmong(Start, End, Location, BetweenText);
      if(BetweenText)
      {
        BetweenText.Replace(",", " ");
        while(BetweenText.Contains("  "))
          BetweenText.Replace("  ", " ");
        BetweenText.Replace(" ", ",");
        BetweenText.Prepend("[");
        BetweenText.Append("]");
        Value v = JSON::Import(BetweenText);
        Vector xy(v[0].AsNumber(), v[1].AsNumber());
        Vector wh(v[2].AsNumber(), v[3].AsNumber());
        SVGBounds = Box(xy, xy + wh);
      }
      if(not (SVGBounds.Width() > 0.f) or not (SVGBounds.Height() > 0.f))
        SVGBounds = Box(Vector(0.f, 0.f), Vector(1.f, 1.f));
      return SVGBounds;
    }

    /**Imports and appends all paths from an SVG into an existing list of paths.
    Note, this does not currently read in any transform attributes, so the
    result will only be valid for those paths which have no transform set.*/
    static Box Import(List<Path>& Paths, List<Brush>& BrushStates,
      const String& SVGData)
    {
      Box SVGBounds = Bounds(SVGData);
      Paths.RemoveAll();
      BrushStates.RemoveAll();
      count PathStartIndex = -1;
      while((PathStartIndex = SVGData.Find("<path", PathStartIndex + 1)) >= 0)
      {
        count PathEndIndex = SVGData.Find(">", PathStartIndex + 5);
        String PathXML(
          reinterpret_cast<const byte*>(&SVGData.Merge()[PathStartIndex]),
          PathEndIndex - PathStartIndex + 1);
        XML::Document d;
        if(d.ParseDocument(PathXML) or not d.Root)
        {
          C::Error() >> "Error: could not parse path data.";
          Paths.RemoveAll();
          BrushStates.RemoveAll();
          return Box();
        }
        ImportData(Paths.Add(), d.Root->GetAttributeValue("d"));
        String FillColor = d.Root->GetAttributeValue("fill");
        String StrokeColor = d.Root->GetAttributeValue("stroke");
        String StrokeWidth = d.Root->GetAttributeValue("stroke-width");
        Brush b;
        if(FillColor)
          b.FillColor.Import(FillColor);
        else
          b.FillColor = Colors::Black();
        b.StrokeColor.Import(StrokeColor);
        if(Limits<number>::IsZero(b.StrokeColor.A))
          b.StrokeWidth = 0.f;
        else if(!StrokeWidth)
          b.StrokeWidth = 1.f;
        else
          b.StrokeWidth = StrokeWidth.ToNumber();

        BrushStates.Add() = b;
      }
      return SVGBounds;
    }

    /**Imports data from the 'd' attribute of an SVG path. The pointer to the
    data can either be its own string or a pointer to the a character in the
    original SVG string. In any case, a null or quote will stop the import.*/
    static void ImportData(Path& p, const ascii* SVGData)
    {
      /*The parsing algorithm used in this method goes character by character,
      and builds up state and command arguments. It is entirely incremental, and
      so it is fast since it does not parse numbers as separate tokens.*/

      //Iteration of the string is by pointer arithmetic.
      const ascii* Data = SVGData;

      /*Every character that comes in is classified as one of the following
      input types. Then given the current parsing state and the previous input
      type, the character can be appropriately parsed.*/
      enum InputType
      {
        Command,
        NumberDigit,
        NumberSign,
        NumberExponential,
        NumberPeriod,
        WhiteSpace
      };

      //The parsing state.
      InputType CurrentType = WhiteSpace;
      InputType PreviousType = WhiteSpace;
      float64 CurrentNumber = 0.0;
      float64 CurrentSign = 1.0;
      float64 CurrentExponentialSign = 1.0;
      float64 CurrentExponential = 0.0;
      bool ParsingExponential = false;
      bool ParsingDecimal = false;
      float64 DecimalValue = 0.0;
      float64 DecimalSize = 1.0;
      float64 NumberState[7];
      count NumberIndex = 0;
      count TotalNumberIndex = 0;
      ascii CurrentCommand = ' ';
      Vector CurrentPosition;

      /*The main parsing here is two-dimensional. It considers the current input
      type against the previous input type. This allows all possibilities to be
      quickly examined, and also permits some recovery in case of error.*/
      for(;;)
      {
        //Get the character.
        ascii d = *Data++;

        if((d >= 'A' && d <= 'Z' && d != 'E') || //Command
          (d >= 'a' && d <= 'z' && d != 'e'))
        {
          CurrentType = Command;
          switch(PreviousType)
          {
            case Command:
            case NumberDigit:
            case NumberPeriod:
            /*Note that the following code is used three times identically in
            this method. It really ought to be a helper method, but the number
            of arguments that would need to be passed makes it not worth it.*/
            //----------------------Flush Current Number*---------------------//
            NumberState[NumberIndex++] = CurrentSign * (CurrentNumber +
              (DecimalValue / DecimalSize)) * Power(10.0,
              CurrentExponentialSign * CurrentExponential);
            CurrentNumber = 0.0;
            CurrentSign = 1.0;
            CurrentExponentialSign = 1.0;
            CurrentExponential = 0.0;
            ParsingExponential = false;
            ParsingDecimal = false;
            DecimalValue = 0.0;
            DecimalSize = 1.0;
            TotalNumberIndex++;
            ImportDataFlush(p, &NumberState[0], NumberIndex,
              TotalNumberIndex, CurrentCommand, CurrentPosition);
            //--------------------End Flush Current Number--------------------//
            CurrentCommand = d;
            TotalNumberIndex = NumberIndex = 0; //Reset no matter what.
            break;
            /*Note: above was modified to remove NumberIndex = 0 if
            ImportDataFlush returned true since it is already set to zero below
            after the fall-through to the WhiteSpace case statement.*/
            case WhiteSpace:
            CurrentCommand = d;
            TotalNumberIndex = NumberIndex = 0; //Reset no matter what.
            break;
            case NumberSign: break; //Invalid
            case NumberExponential: break; //Invalid
          }
        }
        else if(d >= '0' && d <= '9') //Number digit
        {
          CurrentType = NumberDigit;
          switch(PreviousType)
          {
            case Command:
            case WhiteSpace:
            case NumberExponential:
            case NumberPeriod:
            case NumberDigit:
            case NumberSign:
            //Add to existing number.
            if(ParsingExponential)
            {
              CurrentExponential = CurrentExponential * 10. +
                float64(d - '0');
            }
            else if(ParsingDecimal)
            {
              DecimalValue = DecimalValue * 10. + float64(d - '0');
              DecimalSize = DecimalSize * 10.;
            }
            else
            {
              CurrentNumber = CurrentNumber * 10. + float64(d - '0');
            }
          }
        }
        else if(d == '-' || d == '+') //Number sign
        {
          CurrentType = NumberSign;
          switch(PreviousType)
          {
            case NumberDigit:
            case NumberPeriod:
            //----------------------Flush Current Number----------------------//
            NumberState[NumberIndex++] = CurrentSign * (CurrentNumber +
              (DecimalValue / DecimalSize)) * Power(10.0,
              CurrentExponentialSign * CurrentExponential);
            CurrentNumber = 0.0;
            CurrentExponentialSign = 1.0;
            CurrentExponential = 0.0;
            ParsingExponential = false;
            ParsingDecimal = false;
            DecimalValue = 0.0;
            DecimalSize = 1.0;
            TotalNumberIndex++;
            if(ImportDataFlush(p, &NumberState[0], NumberIndex,
              TotalNumberIndex, CurrentCommand, CurrentPosition))
                NumberIndex = 0;
            //--------------------End Flush Current Number--------------------//
            CurrentSign = (d == '+' ? 1.0 : -1.0); break;
            /*Note: above was modified to remove unnecessary assignment to
            CurrentSign since it is overwritten.*/
            case Command:
            case WhiteSpace:
            CurrentSign = (d == '+' ? 1.0 : -1.0); break;
            case NumberExponential:
            CurrentExponentialSign = (d == '+' ? 1.0 : -1.0); break;
            case NumberSign: break; //Invalid
          }
        }
        else if(d == 'e' || d == 'E') //Exponential sign
        {
          CurrentType = NumberExponential;

          switch(PreviousType)
          {
            case NumberDigit:
            case NumberPeriod:
            CurrentExponential = 0.0;
            CurrentExponentialSign = 1.0; //(default to +)
            ParsingExponential = true;
            break;
            case NumberSign: break; //Invalid
            case NumberExponential: break; //Invalid
            case WhiteSpace: break; //Invalid
            case Command: break; //Invalid
          }
        }
        else if(d == '.') //Number decimal period
        {
          CurrentType = NumberPeriod;

          switch(PreviousType)
          {
            case Command:
            case WhiteSpace:
            case NumberDigit:
            case NumberSign:
            ParsingDecimal = true;
            DecimalValue = 0.0;
            DecimalSize = 1.0;
            break;
            case NumberExponential: break; //Invalid
            case NumberPeriod: break; //Invalid
          }
        }
        else //White space (or any other character in the stream)
        {
          CurrentType = WhiteSpace;
          switch(PreviousType)
          {
            case Command:
            //Flush if previous thing was a close path command.
            if(CurrentCommand != 'Z' && CurrentCommand != 'z')
              break;
            //----------------------Flush Current Number----------------------//
            NumberState[NumberIndex++] = CurrentSign * (CurrentNumber +
              (DecimalValue / DecimalSize)) * Power(10.0,
              CurrentExponentialSign * CurrentExponential);
            CurrentNumber = 0.0;
            CurrentSign = 1.0;
            CurrentExponentialSign = 1.0;
            CurrentExponential = 0.0;
            ParsingExponential = false;
            ParsingDecimal = false;
            DecimalValue = 0.0;
            DecimalSize = 1.0;
            TotalNumberIndex++;
            if(ImportDataFlush(p, &NumberState[0], NumberIndex,
              TotalNumberIndex, CurrentCommand, CurrentPosition))
                NumberIndex = 0;
            //--------------------End Flush Current Number--------------------//
            break;
            case NumberPeriod:
            case NumberDigit:
            //----------------------Flush Current Number----------------------//
            NumberState[NumberIndex++] = CurrentSign * (CurrentNumber +
              (DecimalValue / DecimalSize)) * Power(10.0,
              CurrentExponentialSign * CurrentExponential);
            CurrentNumber = 0.0;
            CurrentSign = 1.0;
            CurrentExponentialSign = 1.0;
            CurrentExponential = 0.0;
            ParsingExponential = false;
            ParsingDecimal = false;
            DecimalValue = 0.0;
            DecimalSize = 1.0;
            TotalNumberIndex++;
            if(ImportDataFlush(p, &NumberState[0], NumberIndex,
              TotalNumberIndex, CurrentCommand, CurrentPosition))
                NumberIndex = 0;
            //--------------------End Flush Current Number--------------------//
            break;
            case WhiteSpace: break; //Do nothing
            case NumberExponential: break; //Invalid
            case NumberSign: break; //Invalid
          }
        }

        //If a null or delimiting quote is reached break now.
        if(d == 0 || d == 34 || d == 39)
          break;

        //Update the previous type.
        PreviousType = CurrentType;
      }
    }

    private:

    ///Helper method for ImportData() to do the actual path creation.
    static bool ImportDataFlush(Path& p, const float64* NumberState,
      count NumberIndex, count TotalNumberIndex,
      ascii Command, Vector& CurrentPosition)
    {
      //Determine if the previous instruction was a close command.
      bool PreviousClosed = p.n() && p.z().IsClosing();

      switch(Command)
      {
        //----------------------//
        //Zero Argument Commands//
        //----------------------//

        case 'Z': //Close subpath
        case 'z': //Close subpath
        if(NumberIndex == 1) //Note: by the time Z/z is flushed the index is 1.
        {
          p.Add(Instruction());

          //Look for the most recent move-to command and update position.
          for(count i = p.n() - 1; i >= 0; i--)
          {
            if(p[i].IsMove())
            {
              CurrentPosition = p[i].End();
              break;
            }
          }
          return true;
        }
        break;

        //---------------------//
        //One Argument Commands//
        //---------------------//

        case 'H': //Absolute horizontal line-to
        case 'h': //Relative horizontal line-to
        case 'V': //Absolute vertical line-to
        case 'v': //Relative vertical line-to
        if(NumberIndex == 1)
        {
          if(PreviousClosed)
            p.Add(Instruction(CurrentPosition, true)); //Make new subpath.

          if(Command == 'H') //Absolute horizontal line-to
            CurrentPosition.x = number(NumberState[0]);
          else if(Command == 'h') //Relative horizontal line-to
            CurrentPosition.x += number(NumberState[0]);
          else if(Command == 'V') //Absolute vertical line-to
            CurrentPosition.y = number(NumberState[0]);
          else if(Command == 'v') //Relative vertical line-to
            CurrentPosition.y += number(NumberState[0]);
          p.Add(Instruction(CurrentPosition));
          return true;
        }
        break;

        //---------------------//
        //Two Argument Commands//
        //---------------------//

        case 'M': //Absolute move-to
        case 'm': //Relative move-to
        case 'L': //Absolute line-to
        case 'l': //Relative line-to
        case 'T': //Absolute quadratic-reflection
        case 't': //Relative quadratic-reflection
        if(NumberIndex == 2)
        {
          if(Command == 'L' || Command == 'l')
            if(PreviousClosed)
              p.Add(Instruction(CurrentPosition, true)); //New subpath

          if(Command == 'M') //Absolute move-to
          {
            CurrentPosition.x = number(NumberState[0]);
            CurrentPosition.y = number(NumberState[1]);

            //New subpath if first move pair, afterwards do lines.
            p.Add(Instruction(CurrentPosition, TotalNumberIndex <= 2));
          }
          else if(Command == 'm') //Relative move-to
          {
            CurrentPosition.x += number(NumberState[0]);
            CurrentPosition.y += number(NumberState[1]);

            //New subpath if first move pair, afterwards do lines.
            p.Add(Instruction(CurrentPosition, TotalNumberIndex <= 2));
          }
          else if(Command == 'L') //Absolute line-to
          {
            CurrentPosition.x = number(NumberState[0]);
            CurrentPosition.y = number(NumberState[1]);
            p.Add(Instruction(CurrentPosition));
          }
          else if(Command == 'l') //Relative line-to
          {
            CurrentPosition.x += number(NumberState[0]);
            CurrentPosition.y += number(NumberState[1]);
            p.Add(Instruction(CurrentPosition));
          }
          return true;
        }
        break;

        //----------------------//
        //Four Argument Commands//
        //----------------------//

        case 'Q': //Absolute quadratic-to
        case 'q': //Relative quadratic-to
        if(NumberIndex == 4)
        {
          if(PreviousClosed)
            p.Add(Instruction(CurrentPosition, true)); //New subpath

          Vector c1 = CurrentPosition, e = CurrentPosition;

          if(Command == 'Q') //Absolute quadratic-to
          {
            c1 = Vector(number(NumberState[0]), number(NumberState[1]));
            e  = Vector(number(NumberState[2]), number(NumberState[3]));
          }
          else //Relative quadratic-to
          {
            c1 += Vector(number(NumberState[0]), number(NumberState[1]));
            e  += Vector(number(NumberState[2]), number(NumberState[3]));
          }
          Bezier b;
          b.SetControlPoints(CurrentPosition, c1, e);
          p.Add(Instruction(b));

          CurrentPosition = e; //Update current position
          return true;
        }
        break;

        case 'S': //Absolute cubic-reflection
        case 's': //Relative cubic-reflection
        if(NumberIndex == 4)
        {
          /*"Draws a cubic Bezier curve from the current point to (x,y). The
          first control point is assumed to be the reflection of the second
          control point on the previous command relative to the current point.
          (If there is no previous command or if the previous command was not a
          C, c, S or s, assume the first control point is coincident with the
          current point.) (x2,y2) is the second control point (i.e., the control
          point at the end of the curve). S (uppercase) indicates that absolute
          coordinates will follow; s (lowercase) indicates that relative
          coordinates will follow."*/

          //Determine reflection point (which is used as control point 1).
          Vector c1 = CurrentPosition;
          if(p.n() && p.z().IsCubic())
            c1 += CurrentPosition - p.z().Control2();

          if(PreviousClosed)
            p.Add(Instruction(CurrentPosition, true)); //New subpath

          Vector c2 = CurrentPosition, e = CurrentPosition;

          if(Command == 'S') //Absolute cubic-to
          {
            c2 = Vector(number(NumberState[0]), number(NumberState[1]));
            e = Vector(number(NumberState[2]), number(NumberState[3]));
          }
          else //Relative cubic-to
          {
            c2 += Vector(number(NumberState[0]), number(NumberState[1]));
            e += Vector(number(NumberState[2]), number(NumberState[3]));
          }
          p.Add(Instruction(c1, c2, e));

          CurrentPosition = e; //Update current position
          return true;
        }
        break;

        //---------------------//
        //Six Argument Commands//
        //---------------------//

        case 'C': //Absolute cubic-to
        case 'c': //Relative cubic-to
        if(NumberIndex == 6)
        {
          if(PreviousClosed)
            p.Add(Instruction(CurrentPosition, true)); //New subpath

          Vector c1 = CurrentPosition, c2 = CurrentPosition,
            e = CurrentPosition;

          if(Command == 'C') //Absolute cubic-to
          {
            c1 = Vector(number(NumberState[0]), number(NumberState[1]));
            c2 = Vector(number(NumberState[2]), number(NumberState[3]));
            e = Vector(number(NumberState[4]), number(NumberState[5]));
          }
          else //Relative cubic-to
          {
            c1 += Vector(number(NumberState[0]), number(NumberState[1]));
            c2 += Vector(number(NumberState[2]), number(NumberState[3]));
            e += Vector(number(NumberState[4]), number(NumberState[5]));
          }
          p.Add(Instruction(c1, c2, e));

          CurrentPosition = e; //Update current position
          return true;
        }
        break;

        //-----------------------//
        //Seven Argument Commands//
        //-----------------------//

        case 'A': //Absolute arc-to
        case 'a': //Relative arc-to
        if(NumberIndex == 7)
          return true; //Skip for now.
        break;
      }

      return false; //Command does not have enough arguments yet.
    }
  };
}
#endif
