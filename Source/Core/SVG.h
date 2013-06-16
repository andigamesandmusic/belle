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

#ifndef BELLE_CORE_SVG_H
#define BELLE_CORE_SVG_H

namespace BELLE_NAMESPACE
{
  struct SVGHelper
  {
    //-------//
    //Helpers//
    //-------//
  
    ///Appends a path with transform and glyph information to an SVG string.
    static void AppendPathToSVG(const Path& p, prim::String& Destination,
      Affine Transform = Affine::Unit(), prim::unicode Unicode = 0,
      prim::number AdvanceWidth = 0.0)
    {
      Destination >> "<path";
      
      if(Unicode)
        Destination << " belle:unicode=\"" <<
          (prim::integer)Unicode << "\"";
      
      if(AdvanceWidth)
        Destination << " belle:advance-width=\"" <<
          AdvanceWidth << "\"";
      
      if(Transform != Affine::Unit())
        Destination << " transform=\"matrix(" << Transform.a << " " <<
          Transform.b << " " << Transform.c << " " << Transform.d << " " <<
          Transform.e << " " << Transform.f << ")\"";
      
      Destination << " d=\"";
      for(prim::count j = 0; j < p.n(); j++)
      {
        const Instruction& i = p[j];
        prim::planar::Vector c1 = i.Control1(), c2 = i.Control2(), e = i.End();
        
        if(i.IsMove())
          Destination << " M " << e.x << " " << e.y;
        else if(i.IsLine())
          Destination << " L " << e.x << " " << e.y;
        else if(i.IsCubic())
          Destination << " C " << c1.x << " " << c1.y << " " <<
            c2.x << " " << c2.y << " " << e.x << " " << e.y;
        else
          Destination << " Z";
      }
      Destination << "\"/>";
    }
    
    /**Imports and appends all paths from an SVG into an existing list of paths.
    Note, this does not currently read in any transform attributes, so the
    result will only be valid for those paths which have no transform set.*/
    static void Import(prim::List<Path>& Paths, const prim::String& SVGData)
    {
      //Look for path data and then import it.
      prim::count FindIndex = -1;
      while((FindIndex = SVGData.Find(" d=\"", FindIndex + 1)) >= 0)
      {
        FindIndex += 4;
        ImportData(Paths.Add(), &SVGData.Merge()[FindIndex]);
      }
    }
    
    /**Imports data from the 'd' attribute of an SVG path. The pointer to the
    data can either be its own string or a pointer to the a character in the
    original SVG string. In any case, a null or quote will stop the import.*/
    static void ImportData(Path& p, const prim::ascii* SVGData)
    {
      /*The parsing algorithm used in this method goes character by character,
      and builds up state and command arguments. It is entirely incremental, and
      so it is fast since it does not parse numbers as separate tokens.*/
      
      //Iteration of the string is by pointer arithmetic.
      const prim::ascii* Data = SVGData;
      
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
      prim::float64 CurrentNumber = 0.0;
      prim::float64 CurrentSign = 1.0;
      prim::float64 CurrentExponentialSign = 1.0;
      prim::float64 CurrentExponential = 0.0;
      bool ParsingExponential = false;
      bool ParsingDecimal = false;
      prim::float64 DecimalValue = 0.0;
      prim::float64 DecimalSize = 1.0;
      prim::float64 NumberState[7];
      prim::count NumberIndex = 0;
      prim::count TotalNumberIndex = 0;
      prim::ascii CurrentCommand = ' ';
      prim::planar::Vector CurrentPosition;
      
      /*The main parsing here is two-dimensional. It considers the current input
      type against the previous input type. This allows all possibilities to be
      quickly examined, and also permits some recovery in case of error.*/
      for(;;)
      {
        //Get the character.
        prim::ascii d = *Data++;
        
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
            //----------------------Flush Current Number----------------------//
            NumberState[NumberIndex++] = CurrentSign * (CurrentNumber +
              (DecimalValue / DecimalSize)) * prim::Power(10.0,
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
                (prim::float64)(d - '0');
            }
            else if(ParsingDecimal)
            {
              DecimalValue = DecimalValue * 10. + (prim::float64)(d - '0');
              DecimalSize = DecimalSize * 10.;
            }
            else
            {
              CurrentNumber = CurrentNumber * 10. + (prim::float64)(d - '0');
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
              (DecimalValue / DecimalSize)) * prim::Power(10.0,
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
            case NumberPeriod:
            case NumberDigit:
            //----------------------Flush Current Number----------------------//
            NumberState[NumberIndex++] = CurrentSign * (CurrentNumber +
              (DecimalValue / DecimalSize)) * prim::Power(10.0,
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
    static bool ImportDataFlush(Path& p, const prim::float64* NumberState,
      prim::count NumberIndex, prim::count TotalNumberIndex,
      prim::ascii Command, prim::planar::Vector& CurrentPosition)
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
          for(prim::count i = p.n() - 1; i >= 0; i--)
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
            CurrentPosition.x = NumberState[0];
          else if(Command == 'h') //Relative horizontal line-to
            CurrentPosition.x += NumberState[0];
          else if(Command == 'V') //Absolute vertical line-to
            CurrentPosition.y = NumberState[0];
          else if(Command == 'v') //Relative vertical line-to
            CurrentPosition.y += NumberState[0];
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
            CurrentPosition.x = NumberState[0];
            CurrentPosition.y = NumberState[1];
            
            //New subpath if first move pair, afterwards do lines.
            p.Add(Instruction(CurrentPosition, TotalNumberIndex <= 2));
          }
          else if(Command == 'm') //Relative move-to
          {
            CurrentPosition.x += NumberState[0];
            CurrentPosition.y += NumberState[1];
            
            //New subpath if first move pair, afterwards do lines.
            p.Add(Instruction(CurrentPosition, TotalNumberIndex <= 2));
          }
          else if(Command == 'L') //Absolute line-to
          {
            CurrentPosition.x = NumberState[0];
            CurrentPosition.y = NumberState[1];
            p.Add(Instruction(CurrentPosition));
          }
          else if(Command == 'l') //Relative line-to
          {
            CurrentPosition.x += NumberState[0];
            CurrentPosition.y += NumberState[1];
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
        case 'S': //Absolute cubic-reflection
        case 's': //Relative cubic-reflection
        if(NumberIndex == 4)
          return true; //Skip for now.
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
          
          prim::planar::Vector c1 = CurrentPosition, c2 = CurrentPosition,
            e = CurrentPosition;
          
          if(Command == 'C') //Absolute cubic-to
          {
            c1 = prim::planar::Vector(NumberState[0], NumberState[1]);
            c2 = prim::planar::Vector(NumberState[2], NumberState[3]);
            e = prim::planar::Vector(NumberState[4], NumberState[5]);
          }
          else //Relative cubic-to
          {
            c1 += prim::planar::Vector(NumberState[0], NumberState[1]);
            c2 += prim::planar::Vector(NumberState[2], NumberState[3]);
            e += prim::planar::Vector(NumberState[4], NumberState[5]);
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
