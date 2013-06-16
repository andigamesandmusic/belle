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

#ifndef BELLE_CORE_TEXT_H
#define BELLE_CORE_TEXT_H

#include "Colors.h"
#include "Font.h"

namespace BELLE_NAMESPACE
{
  struct Character
  {
    prim::unicode Code;
    Font::Style Style;
    prim::number PointSize;
    Color FaceColor;
    Glyph* TypesetGlyph;
    prim::planar::Vector TypesetPosition;
    prim::planar::Vector TypesetSize;
  
    Character() : Code(0), Style(Font::Regular), PointSize(12),
      TypesetGlyph(0) {}
    
    bool IsSpace() const
    {
      return Code == 32;
    }
    
    bool IsLineFeed() const
    {
      return Code == 10;
    }
    
    bool IsCarraigeReturn() const
    {
      return Code == 13;
    }
    
    bool IsTab() const
    {
      return Code == 9;
    }
    
    bool IsCJK() const
    {
      if(Code >= 0x2E80 && Code <= 0x9FFF)
        return true;
      return false;
    }
    
    bool BreaksBefore() const
    {
      if(Code == 9)
        return true;
      return false;
    }
    
    bool CanNotBreakBefore() const
    {
      if(Code == 0xFF0C || Code == 0x3002)
        return true;
      return false;
    }
    
    bool BreaksAfter() const
    {
      //        tab                         en-dash           em-dash
      if(Code == 9 || Code == '-' || Code == 0x2013 || Code == 0x2014 ||
      //full-width comma
        Code == 0xFF0C || IsCJK())
          return true;
      return false;
    }
  };
  
  struct Word : public prim::Array<Character>
  {
    ///Offset of word in original string
    prim::count Offset;
    
    bool IsSpace() const
    {
      for(prim::count i = 0; i < n(); i++)
        if(!ith(i).IsSpace())
          return false;
      return true;
    }
    
    bool IsLineBreak() const
    {
      if(n() && ith(0).IsLineFeed())
        return true;
      return false;
    }
    
    bool IsTab() const
    {
      if(n() && ith(0).IsTab())
        return true;
      return false;
    }

    bool IsWhiteSpace() const
    {
      return ((n() == 0) || IsSpace() || IsTab() || IsLineBreak());
    }
    
    /**Computes the bounds of the word. It takes into account changing style
    and size and also computes kerns for the horizontal, and ascender and
    descender values for the height. The line height returned is computed
    separately from the ascender and descender and may differ depending on how
    the font was encoded.*/
    prim::planar::Rectangle Bounds(const Font& FontToUse,
      prim::number& LineHeight)
    {
      //      LineHeight = 0.0;
      prim::number MaxAscent = 0.0;
      prim::number MinDescent = 0.0;
      prim::number TotalWidth = 0.0;
      
      Character Previous;
      for(prim::count i = 0; i < n(); i++)
      {
        Character& Current = ith(i);
        
        //Set the relative position of the letter in the word.
        Current.TypesetPosition = prim::planar::Vector(TotalWidth, 0.0);
        Current.TypesetGlyph = 0;
        
        prim::count TypefaceUsed = 0;
        if(Glyph* g = FontToUse.LookupGlyph(Current.Code, Current.Style,
          TypefaceUsed))
        {
          //Set the glyph used for the character.
          Current.TypesetGlyph = g;
          
          //Get a reference to the typeface.
          const Typeface& t = *FontToUse[TypefaceUsed];
          
          //Get the primary advance width.
          prim::number GlyphWidth = g->AdvanceWidth;
          
          /*Compute the kern adjustment. Do not compute if style or size has
          changed.*/
          if(i >= 1 && Previous.Style == Current.Style &&
            Previous.PointSize == Current.PointSize)
              GlyphWidth += 
                FontToUse[TypefaceUsed]->Kerning(Previous.Code, Current.Code);
          
          //Get the points to inches conversion for this character.
          prim::number Scale = Current.PointSize / 72.0;
          
          //Scale the glyph width by the point size.
          GlyphWidth *= Scale;
          LineHeight = prim::Max(LineHeight, t.TypographicHeight * Scale);
          MaxAscent = prim::Max(MaxAscent, t.TypographicAscender * Scale);
          MinDescent = prim::Min(MinDescent, t.TypographicDescender * Scale);
          
          //Cache the character's computed width and height
          Current.TypesetSize = prim::planar::Vector(GlyphWidth,
            t.TypographicHeight * Scale);
          
          //Sum the glyph width.
          TotalWidth += GlyphWidth;
        }
        Previous = Current;
      }
      
      return prim::planar::Rectangle(prim::planar::Vector(0.0, MinDescent),
        prim::planar::Vector(TotalWidth, MaxAscent));
    }
  };
  
  class Text
  {
    public:
    
    typedef prim::count Justification;

    struct Justifications
    {
      static const Justification Left = 0;
      static const Justification Center = 1;
      static const Justification Right = 2;
      static const Justification Full = 3;
    };
    
    const Font& FontToUse;
    Font::Style FontStyle;
    prim::number PointSize;

    /**The height of the font calcuated from the specified font and
        point style.*/
    prim::number FontHeight;

    /**The total width of the text layout in inches. */
    prim::number LineWidth;

    /**The color of the text in the layout.*/
    Color TextColor;

    /**The text justification style of the layout.*/
    Justification Justify;
    prim::number TabSize;

    /**Total height of the Text layout, caculated after typsetting the
       Text.*/
    prim::number TotalHeight;

    /**Holds all the words in the Text object, indexed by word
       count.*/
    prim::Array<Word> Words;

    /**Holds the line number of each word, indexed by word count.*/
    prim::Array<prim::count> WordLineAssignment;

    /**Holds the number of chars in each word, indexed by word
        count.*/
    prim::Array<prim::number> WordLengths;    

    /**Holds the amount of LineWidth in inches consumed by words on a
        line, indexed by line count. */
    prim::Array<prim::number> LineSpaceConsumed;

    /**Holds the height of each line, indexed by line count.**/
    prim::Array<prim::number> LineHeights;

    /**Holds the word indexes of the first word on each line, indexed
        by line count.*/
    prim::Array<prim::count> LineWordStart;

    /**Holds the word indexes of the last word on each line, indexed
        by line count.*/
    prim::Array<prim::count> LineWordEnd;

    /**True if a line is the last line in paragraph, indexed by line
       count.*/
    prim::Array<bool> LineLastInParagraph;
    
    public:
    
    ///Constructor to set typesetting arguments.
    Text(const Font& FontToUse, Font::Style FontStyle, prim::number PointSize, 
         prim::number LineWidth, 
         Justification Justify = Justifications::Left, 
         Color TextColor = Colors::black,
         prim::number TabSize = 0.5) 
      : FontToUse(FontToUse), 
        FontStyle(FontStyle),
        PointSize(PointSize),
        FontHeight(FontToUse.GetTypeface(FontStyle)->
                   TypographicHeight * (PointSize / 72.0)),
        LineWidth(LineWidth),
        TextColor(TextColor),
        Justify(Justify),
        TabSize(TabSize), 
        TotalHeight(FontHeight) 
    {
    }

    void SetLineWidth(prim::number NewLineWidth)
    {
      LineWidth = NewLineWidth;
    }
    
    /**Clear arrays for new typesetting. Call this to clear all typesetting info
    before retypesetting existing words.*/
    void ClearTypesetting()
    {
      WordLineAssignment.n(0);
      WordLengths.n(0);
      LineSpaceConsumed.n(0);
      LineHeights.n(0);
      LineWordStart.n(0);
      LineWordEnd.n(0);
      LineLastInParagraph.n(0);
      TotalHeight=FontHeight;
    }

    ///Imports an array of characters and determines the word breaks.
    void ImportStringToWords(prim::Array<Character>& In)
    {
      //Clear words array.
      Words.n(0);
      
      //Clear typesetting info.
      ClearTypesetting();

      //If there is no input, then return.
      if(!In.n()) return;
      
      //Initialize word parse state.
      bool AccumulatingSpace = In[0].IsSpace();
      bool MustBreakNext = true;
      
      //Parse the words.
      for(prim::count i = 0; i < In.n(); i++)
      {
        Character& Current = In[i];
        
        //Add style information
        Current.Style = FontStyle;
        Current.PointSize = PointSize;
        Current.FaceColor = TextColor;

        //Handle line breaks separately.
        if(Current.IsCarraigeReturn())
          continue; //Skip Windows-style CR completely.
        else if(Current.IsLineFeed())
        {
          Words.Add().Add() = Current;
          MustBreakNext = true;
          Words.z().Offset = i;
          continue;
        }
        
        if((MustBreakNext || Current.BreaksBefore()) &&
          !Current.CanNotBreakBefore())
        {
          Words.Add();
          Words.z().Offset = i;
        }
        
        if(AccumulatingSpace)
        {
          if(Current.IsSpace())
            MustBreakNext = false;
          else
          {
            MustBreakNext = Current.BreaksAfter();
            AccumulatingSpace = false;
            if(Words.z().n())
            {
              Words.Add();
              Words.z().Offset = i;
            }
          }
        }
        else
        {
          if(Current.IsSpace())
          {
            MustBreakNext = false;
            AccumulatingSpace = true;
            if(Words.z().n())
            {
             Words.Add();
             Words.z().Offset = i;
            }
          }
          else
            MustBreakNext = Current.BreaksAfter();
        }
        Words.z().Add() = Current;
      }
    }
    
    ///Imports a string and determines the word breaks.
    void ImportStringToWords(const prim::String& In)
    {
      const prim::byte*
        Begin = (const prim::byte*)In.Merge(), *End = Begin + In.n();
      
      prim::Array<Character> InCharacters;
      
      while(Begin < End)
        InCharacters.Add().Code = prim::String::Decode(Begin, End);
      
      ImportStringToWords(InCharacters);
    }
    
    void DetermineLineBreaks()
    {
      /*Note that this a maximum length (greedy) word wrap algorithm. Spaces and
      tab characters may appear at the end of the line ad infinitum without
      causing a line break until an actual word appears. The goal of this method
      is to produce a sequence of word length and line assignments, as well as
      line space consumed and heights. These can then be used by the typesetter
      in any justification mode to then place each word. Note that while this
      method is justification-less, it does presume a left-to-right appearance
      of words, and the meaning of tabs in any justification but left is not
      considered.*/
      prim::number SpaceTakenOnCurrentLine = 0.0;
      prim::count CurrentLine = 0;
      LineHeights.Add() = FontHeight; //0.0
      LineSpaceConsumed.Add() = 0.0;
      LineWordStart.Add() = 0;
      LineWordEnd.Add() = -1;
      LineLastInParagraph.Add() = true;
      TotalHeight = 0.0;
      for(prim::count i = 0; i < Words.n(); i++)
      {
        Word& w = Words[i];
        prim::number WordHeight = 0.0;
        prim::planar::Rectangle r = w.Bounds(FontToUse, WordHeight);
        prim::number WordWidth = r.Width();
        
        if(w.IsLineBreak())
        {
          //Hard line break.
          SpaceTakenOnCurrentLine = 0.0;
          WordLengths.Add() = 0.0;
          WordLineAssignment.Add() = ++CurrentLine;
          LineHeights.Add() = FontHeight; //0.0
          LineSpaceConsumed.Add() = 0.0;
          LineWordStart.Add() = i;
          LineWordEnd.Add() = i;
          LineLastInParagraph.Add() = true;
        }
        else if(w.IsSpace() || w.IsTab())
        {
          //Tab or a space sequence was reached.
          if(w.IsTab()) //Adjust tab width to meet next tab break.
            WordWidth = TabSize - prim::Mod(SpaceTakenOnCurrentLine, TabSize);
          
          SpaceTakenOnCurrentLine += WordWidth;
          WordLengths.Add() = WordWidth;
          WordLineAssignment.Add() = CurrentLine;
          //Tabs and spaces do not add to line height or space consumed.
          LineWordEnd.z() = i;
        }
        else if(SpaceTakenOnCurrentLine + WordWidth <= LineWidth)
        {
          //The word fits on the line.
          SpaceTakenOnCurrentLine += WordWidth;
          WordLengths.Add() = WordWidth;
          WordLineAssignment.Add() = CurrentLine;
          LineHeights.z() = prim::Max(LineHeights.z(), WordHeight);
          LineSpaceConsumed.z() = SpaceTakenOnCurrentLine;
          LineWordEnd.z() = i;
        }
        else if(i == 0)
        {
          //The word is the first in the string and does not fit the line.
          SpaceTakenOnCurrentLine = WordWidth;
          WordLengths.Add() = WordWidth;
          WordLineAssignment.Add() = CurrentLine++;
          LineHeights.z() = WordHeight;
          LineSpaceConsumed.z() = SpaceTakenOnCurrentLine;
          LineLastInParagraph.z() = false;
          LineHeights.Add() = FontHeight; // 0.0
          LineSpaceConsumed.Add() = 0.0;
          LineWordStart.Add() = i + 1;
          LineWordEnd.Add() = i + 1;
          LineLastInParagraph.Add() = true;
        }
        else
        {
          //The word must be placed on the next line (even if it does not fit).
          SpaceTakenOnCurrentLine = WordWidth;
          LineLastInParagraph.z() = false;
          WordLengths.Add() = WordWidth;
          WordLineAssignment.Add() = ++CurrentLine;
          LineHeights.Add() = WordHeight;
          LineSpaceConsumed.Add() = SpaceTakenOnCurrentLine;
          LineWordStart.Add() = i;
          LineWordEnd.Add() = i;
          LineLastInParagraph.Add() = true;
        }
      }

      for(prim::count i = LineHeights.n() - 1; i >= 0; i--)
        TotalHeight += LineHeights[i];
    }
    
    prim::planar::Vector Typeset()
    {
      prim::planar::Vector CurrentWordPosition;
      for(prim::count i = LineHeights.n() - 1; i >= 0; i--)
      {
        prim::count Start = LineWordStart[i];
        prim::count End = LineWordEnd[i];
        if(End >= Words.n())
        {
          CurrentWordPosition.y += LineHeights[i];
          continue; //Skip if the line contains nothing.
        }
        
        CurrentWordPosition.x = 0.0;
        prim::number LineSpaceRemaining = LineWidth - LineSpaceConsumed[i];
        prim::number ExtraWordSpace = 0.0;
           
        if(Justify == Justifications::Right)
        {
          CurrentWordPosition.x += LineSpaceRemaining;
        }
        else if(Justify == Justifications::Center)
        {
          CurrentWordPosition.x += LineSpaceRemaining / 2.0;
        }
        else if(Justify == Justifications::Full)
        {
          if(!LineLastInParagraph[i] && End > Start)
            ExtraWordSpace = LineSpaceRemaining / (prim::number)(End - Start);
        }
        
        for(prim::count j = Start; j <= End; j++)
        {
          Word& w = Words[j];
          for(prim::count k = 0; k < w.n(); k++)
            w[k].TypesetPosition += CurrentWordPosition;    
          CurrentWordPosition.x += WordLengths[j] + ExtraWordSpace;
        }
        CurrentWordPosition.y += LineHeights[i];
      }
      return prim::planar::Vector(LineWidth, CurrentWordPosition.y);
    }
    
    /**Gets the words to be typeset. A word is simply any sequence of characters
    whose begins and ends are possible wrap points.*/
    prim::Array<Word>& GetWords()
    {
      return Words;
    }
    
    /**Returns the number of Lines in the typeset. Note that this value will be
    0 if the text has not yet been typeset and at least 1 after typesetting
    (even if there is no text).*/
    prim::count GetNumLines()
    {
      return LineHeights.n();
    }

    /**Returns the total number of characters in the specified line.*/
    prim::count GetNumCharsInLine(prim::count Line)
    {
      prim::count n = 0;
      if(Words.n() == 0)
        return n;
#if 0 //Debugging
      if(Line >= GetNumLines())
      {
        c >> "Warning Line out of bounds: Line=" << Line << " NumLines=" <<
          GetNumLines() << "\n";
        return 0;
      }
#endif
      for(prim::count i = LineWordStart[Line]; i <= LineWordEnd[Line]; i++)
        n += Words[i].n();
      return n;
    }

    ///Returns the index of the closest line bounding point's Y.
    prim::count GetLine(const Inches& Point)
    {
      prim::number LineTop=TotalHeight;
      for(prim::count i = 0; i < GetNumLines(); i++)
      {
        prim::number LineBottom = (LineTop - LineHeights[i]);
        
        //If point y is above top line (line zero) return top line if forced.
        if((i == 0) && (Point.y >= LineTop))
          return 0;
          
        /*Else if point y is below bottom line (GetNumLines()-1) then return
        bottom line if forced.*/
        else if((i == GetNumLines()-1) && (Point.y < LineBottom))
          return GetNumLines()-1;
        
        /*Else if point y between bottom and top return current line. Subtract a
        1/4 lineheight 'descent' to allow the mouse point to be a bit under the
        line.*/
        else if((Point.y < LineTop) && (Point.y >= (LineBottom -
          (LineHeights[i] / 4))))
            return i;
        LineTop = LineBottom;
      }
      return 0;
    }

    /**Returns the closest word and character indexes to the specified point as
    vector x and y values. If the specified point's x value lies after the
    bounds of the last character in a line the vector's x will be set to the
    last word in the line but its y will be the the size of the last word in
    line (this condition marks the EOL position for the cursor code) If rounding
    is 0 then the index returned will contain the point. If Rounding is -1 then
    if the point is actuall closer to the previous index that index will be
    returned. Else if Rounding is 1 then if the point is closer to the next
    character index the that will be returned.*/
    prim::planar::VectorInt GetWordAndChar(Inches& Point,
      prim::count Rounding = 0)
    {
      if(Words.n() == 0)
        return prim::planar::VectorInt(0, 0);
      prim::count l = GetLine(Point);
      //Get indexes to first and last words on line.
      prim::count a = LineWordStart[l];
      prim::count z = LineWordEnd[l];
      
      //Word, character and size variables
      prim::count i = a, j = 0, n = 0;
      bool Done = false;
      while(i <= z && !Done)
      {
        n = GetWords()[i].n();
        j = 0;
        while(j < n && !Done)
        {
          Character& c = Words[i][j];
          prim::number x1 = c.TypesetPosition.x;
          prim::number x2 = x1 + c.TypesetSize.x;
          
          /*If point x is leftward of first character on line force index to
          [a, 0].*/
          if((i == a) && (j == 0) && (Point.x < x1) )
          {
            Done = true;
          }
          //Else if point x is right of last character on line use [z,n]
          else if((i == z) && (j == n - 1) && (Point.x >= x2) )
          {
            j = n;
            Done = true;
          }
          //Else if point x is within the rect of the current char
          else if((Point.x >= x1) && (Point.x < x2))
          {
            /*If rounding down and x is closer to x1 then use previous character
            index.*/
            if((Rounding == -1) && ((Point.x - x1) < (x2 - Point.x)))
            {
              //If at start of word move before last character of previous word.
              if(j == 0)
              {
                if(i > 0)
                {
                  i--;
                  j = GetWords()[i].n() - 1;
                }
              }
              //Else move to previous character in current word.
              else
                j--;
              Done = true;
            }
            /*Else if rounding up and x is closer to x2 then use next
            character's index.*/
            else if((Rounding == 1) && ((x2 - Point.x)) < (Point.x - x1))
            {
              //If j is not last valid character in word then just increment it
              if(j < n - 1)
              {
                j++;
              }
              else //Else j is on last valid character in word.
              {
                //If its the last word in line then move cursor to EOL (j = n)
                if(i == LineWordEnd[WordLineAssignment[i]])
                {
                  j = n;
                }
                else //Else move to first character of next word.
                {
                  i++;
                  j = 0;
                }
              }
              Done = true;
            }
            else
              Done = true;
          }
          else
            j++;
        }

        if(Done)
          break;
        else
          i++;
      }
      return prim::planar::VectorInt(i, j);
    }

    /**Returns the closest word and character indexes for the specified line and
    column positions.*/
    prim::planar::VectorInt GetWordAndChar(prim::count Line, prim::count Column)
    {
      if(Words.n() == 0)
        return prim::planar::VectorInt(0,0);
      if(Line >= GetNumLines())
        return prim::planar::VectorInt(Words.n()-1, Words.z().n());
      prim::count n = 0;
      for(prim::count i = LineWordStart[Line]; i <= LineWordEnd[Line]; i++)
        for(prim::count j = 0; j < Words[i].n(); j++)
          if(n == Column)
            return prim::planar::VectorInt(i,j);
          else
            n++;
      
      //Put at EOL (just after last character of last word in line)
      n = LineWordEnd[Line];
      return prim::planar::VectorInt(n, Words[n].n());
    }

    /**Returns the word and character index given the specified offset in the
    source string. This will attempt to return an end of line position if
    PreserveEOL is true.*/
    prim::planar::VectorInt GetWordAndCharFromIndex(prim::count StringIndex,
      bool PreserveEOL)
    {
      if(Words.n() == 0)
        return prim::planar::VectorInt(0,0);
      prim::count i = 0;
      prim::count j = 0;
      for( ; i < Words.n(); i++)
      {       
        if(Words[i].Offset <= StringIndex && (Words[i].Offset +
          Words[i].n()) > StringIndex)
            break;
        else if (PreserveEOL)
        {
          if(Words[i].Offset <= StringIndex && (Words[i].Offset +
            Words[i].n()) == StringIndex)
              break;     
        }
      }
      if(i < Words.n())
        j = StringIndex - Words[i].Offset;
      else //Move to EOB
      {
        i = Words.n() - 1;
        j = Words.z().n();
      }
      return prim::planar::VectorInt(i, j);
    }

    /**Returns the line and column positions for the specified word and
    character indexes.*/
    prim::planar::VectorInt GetLineAndColumn(prim::count WordIndex,
      prim::count CharIndex)
    {
      if(Words.n() == 0)
        return prim::planar::VectorInt(0, 0);
      
      //Get closest word.
      prim::count w = prim::Min(WordIndex, Words.n() - 1);
      
      //Get line of the word.
      prim::count l = WordLineAssignment[w];
      
      //Get column position in line.
      prim::count c = 0;
      
      //Iterate line counting columns
      for(prim::count i = LineWordStart[l]; i <= LineWordEnd[l]; i++)
        for(prim::count j = 0; j < Words[i].n(); j++)
          if(i == w && j == CharIndex)
            return prim::planar::VectorInt(l,c);
      else
        c++;
      
      //Return EOL (position after last character in last word on line).
      return prim::planar::VectorInt(l, c);
    }

    /**Returns the lower left position (in inches) of the character at the
    specified word and character indexes.*/
    Inches GetTextPosition(int WordIndex, int CharIndex, bool UseLower = true)
    {
      if(Words.n() == 0) //No words
        return Inches(0.0, 0.0);
      Inches Result;
      prim::number CharacterHeight = 0.0;
      if(WordIndex >= Words.n()) //Goto position after last word
      {
        //Debugging
        prim::c >> "WordIndex out of bounds: WordIndex=" << WordIndex <<
          " NumWords=" << Words.n();
        Character& c = Words.z().z();
        CharacterHeight = c.TypesetSize.y;
        Result = Inches(c.TypesetPosition.x + c.TypesetSize.x,
          c.TypesetPosition.y);
      }
      else
      {
        Word& w = GetWords()[WordIndex];
        if(CharIndex == w.n())//Goto position after last character
        {
          Character& c = w.z();
          CharacterHeight = c.TypesetSize.y;
          Result = Inches(c.TypesetPosition.x + c.TypesetSize.x,
                        c.TypesetPosition.y);
        }
        else
        {
          Character& c = w.ith(CharIndex);
          CharacterHeight = c.TypesetSize.y;
          Result = Inches(c.TypesetPosition.x, c.TypesetPosition.y);
        }
      }
      
      //Adding a fudge to avoid a LSB floating-point comparison on the line threshold.
      prim::count LineIndex = GetLine(Inches(Result.x, Result.y + CharacterHeight / 2.0));
      if(UseLower)
        Result.y += LineHeights[LineIndex];
      return Result;
    }

    /**Converts the specified word and character indexes into a linear string
    index.*/
    prim::count GetStringIndex(prim::count WordIndex, prim::count CharIndex)
    {
      if(Words.n() == 0)
        return 0;
      if(WordIndex >= Words.n()) // past last word
        return Words.z().Offset + Words.z().n();
      return Words[WordIndex].Offset + CharIndex;
    }

    /**Returns the character at the specified indexes or null if either index is
    out of bounds.*/
    prim::ascii LookingAt(prim::count WordIndex, prim::count CharIndex)
    {
      if(WordIndex >= Words.n())
        return 0;
      if(CharIndex >= Words[WordIndex].n())
        return 0;
      return Words[WordIndex][CharIndex].Code;
    }
    
#if 0 //Unstable code. Word::Bounds() is not const and typesets.
    ///Gets the bounds of the typeset text.
    prim::planar::Rectangle Bounds()
    {
      //      prim::number Scale = 12.0 / 72.0;
      prim::number Scale = PointSize / 72.0;
      const Typeface& t = *FontToUse.GetTypeface(Font::Regular);
      prim::number Ascender = t.TypographicAscender * Scale;
      prim::number Descender = t.TypographicDescender * Scale;
      prim::planar::Rectangle r(prim::planar::Vector(0.0, Descender),
        prim::planar::Vector(0.0, Ascender));
      prim::number LineHeight = 0;
      for(prim::count i = 0; i < Words.n(); i++)
        r = r + Words[i].Bounds(FontToUse, LineHeight);
      return r;
    }
#endif
  };
}
#endif
