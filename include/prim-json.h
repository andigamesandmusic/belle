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

#ifndef PRIM_INCLUDE_JSON_H
#define PRIM_INCLUDE_JSON_H

namespace PRIM_NAMESPACE
{
  ///Reads and writes JSON to and from Value objects.
  class JSON
  {
    /*
    This implementation is based on RFC 4627 JavaScript Object Notation (JSON)
    http://www.ietf.org/rfc/rfc4627.txt

    Current limitations:
    * Numbers not strictly checked against standard. Numbers will be imported if
    their value can be deduced.
    * Encodings other than UTF-8 not supported
    */

    /*The following enumeration lists all of the possible categories of
    characters that can appear in JSON in various contexts. Since the
    categories overlap in codepoint space, the context in which they are used
    depends on the state of the parser. This list was created based off the
    language of the RFC.*/
    enum JSONCharacter
    {
      Whitespace,      // 0x09, 0x0a, 0x0d, 0x20 (Tab, LF, CR, Space)

      BeginArray,      // [
      BeginObject,     // {
      EndArray,        // ]
      EndObject,       // }
      NameSeparator,   // :
      ValueSeparator,  // ,

      False1,          // f
      False2,          // a
      False3,          // l
      False4,          // s
      False5,          // e

      Null1,           // n
      Null2,           // u
      Null3,           // l
      Null4,           // l

      True1,           // t
      True2,           // r
      True3,           // u
      True4,           // e

      DecimalPoint,    // .
      Digit1To9,       // 1-9
      Digit,           // 0-9
      ExponentSign,    // e, E
      Minus,           // -
      Plus,            // +
      Zero,            // 0
      Numeric,         // DecimalPoint, Digit, E, Minus, Plus

      QuotationMark,   // "
      Unescaped,       // 0x20, 0x21, 0x23-0x5b, 0x5d-0x10ffff

      Escape,                 // 0x5c (Backslash)
      EscapedQuotationMark,   // "
      EscapedReverseSolidus,  // 0x5c (Backslash)
      EscapedSolidus,         // /
      EscapedBackspace,       // b
      EscapedFormFeed,        // f
      EscapedLineFeed,        // n
      EscapedCarriageReturn,  // r
      EscapedTab,             // t
      EscapedCodepoint,       // u
      EscapedHexDigit         // 0-9, a-f, A-F
    };

    template <JSONCharacter C> static bool Is(ascii a)
    {
      return Is<C>(unicode(a));
    }

    /**This method is templated based on the JSON character type to allow for
    the compiler to statically optimize away the switch.*/
    template <JSONCharacter C> static bool Is(unicode u)
    {
      switch(C)
      {
      case Whitespace:      // 0x09, 0x0a, 0x0d, 0x20 (Tab, LF, CR, Space)
        return u == 0x20 or u == 0x0a or u == 0x0d or u == 0x09;
      case BeginArray:      // [
        return u == '[';
      case BeginObject:     // {
        return u == '{';
      case EndArray:        // ]
        return u == ']';
      case EndObject:       // }
        return u == '}';
      case NameSeparator:   // :
        return u == ':';
      case ValueSeparator:  // ,
        return u == ',';

      case False1:          // f
        return u == 'f';
      case False2:          // a
        return u == 'a';
      case False3:          // l
        return u == 'l';
      case False4:          // s
        return u == 's';
      case False5:          // e
        return u == 'e';

      case Null1:           // n
        return u == 'n';
      case Null2:           // u
        return u == 'u';
      case Null3:           // l
        return u == 'l';
      case Null4:           // l
        return u == 'l';

      case True1:           // t
        return u == 't';
      case True2:           // r
        return u == 'r';
      case True3:           // u
        return u == 'u';
      case True4:           // e
        return u == 'e';

      case DecimalPoint:    // .
        return u == '.';
      case Digit1To9:       // 1-9
        return u >= '1' and u <= '9';
      case Digit:           // 0-9
        return u >= '0' and u <= '9';
      case ExponentSign:    // e, E
        return u == 'e' or u == 'E';
      case Minus:           // -
        return u == '-';
      case Plus:            // +
        return u == '+';
      case Zero:            // 0
        return u == '0';
      case Numeric:         // DecimalPoint, Digit, E, Minus, Plus
        return Is<DecimalPoint>(u) or Is<Digit>(u) or Is<ExponentSign>(u) or
          Is<Minus>(u) or Is<Plus>(u);

      case QuotationMark:   // "
        return u == '"';
      case Unescaped:       // 0x20, 0x21, 0x23-0x5b, 0x5d-0x10ffff
        return u >= 0x20 and u <= 0x10ffff and u != 0x22 and u != 0x5c;

      case Escape:                 // 0x5c (Backslash)
        return u == 0x5c;
      case EscapedQuotationMark:   // "
        return u == '"';
      case EscapedReverseSolidus:  // 0x5c (Backslash)
        return u == 0x5c;
      case EscapedSolidus:         // /
        return u == '/';
      case EscapedBackspace:       // b
        return u == 'b';
      case EscapedFormFeed:        // f
        return u == 'f';
      case EscapedLineFeed:        // n
        return u == 'n';
      case EscapedCarriageReturn:  // r
        return u == 'r';
      case EscapedTab:             // t
        return u == 't';
      case EscapedCodepoint:       // u
        return u == 'u';
      case EscapedHexDigit:        // 0-9, a-f, A-F
        return Unicode::IsHexDigitValue(u);
      }
      return false;
    }

    ///Possible states during parse
    enum JSONParseState
    {
      Beginning,
      WaitingForFirstValue,
      WaitingForName,
      WaitingForValue,
      WaitingForNameSeparator,
      WaitingForArrayEnd,
      WaitingForObjectEnd,
      ValueFalse2,
      ValueFalse3,
      ValueFalse4,
      ValueFalse5,
      ValueNull2,
      ValueNull3,
      ValueNull4,
      ValueTrue2,
      ValueTrue3,
      ValueTrue4,
      Ending,
      Abort
    };

    //Give friendship to the Nothing since JSONParseState is private.
    friend class Nothing<JSON::JSONParseState>;

    ///Stores the entire state of the parser.
    class JSONParseContext
    {
      public:

      const byte* Start;
      const byte* s;
      const byte* p;
      const byte* e;
      unicode d; Pad<unicode> d_padding;
      Value* v;
      JSONParseState State; Pad<JSONParseState> State_padding;
      Value StateValueDummy;
      String ErrorInfo;
      List<Value*> Stack;
      List<Value> StackKeys;
      List<JSONParseState> States;

      JSONParseContext() : s(0), p(0), e(0), d(0), v(0), State(Beginning) {}

      ///Returns error string for the current state and updates the error info.
      String GetErrorString()
      {
        if(State != Abort)
        {
          ErrorInfo = "Value Stack: ";
          ErrorInfo << Stack.n() << ", Parse Stack: " << States.n();
          return "JSON parser encountered unterminated values";
        }
        else if(not ErrorInfo)
          ErrorInfo = "Unexpected character";
        String Snippet;
        Snippet << reinterpret_cast<const ascii*>(s - 1);
        Snippet.Erase(60, Snippet.n() - 1);
        Snippet.Replace("\n", " ");
        String ErrorString;
        ErrorString << "JSON parser aborted at '" << Snippet <<
          "' (character " << integer(s - Start - 1) << ")";
        return ErrorString;
      }
    };

    //Chomps the a JSON string.
    static bool ChompString(JSONParseContext& C)
    {
      bool Escaped = false;
      unicode SurrogateLead = 0;
      String Text;

      //First check for a vanilla string so that it can be copied efficiently.
      bool IsVanillaString = true;
      {
        const byte* s = C.s;
        const byte* e = C.e;
        const byte* LastSeen = s;
        unicode d;
        while(IsVanillaString && s < e && (d = String::Decode(s, e)) != 0)
        {
          if(Is<Escape>(d))
            IsVanillaString = false;
          else if(Is<QuotationMark>(d))
            break;
          else
            LastSeen = s;
        }

        if(IsVanillaString)
          Text.Append(C.s, count(LastSeen - C.s));
      }

      while(C.s < C.e && (C.d = String::Decode(C.s, C.e)) != 0)
      {
        if(not Escaped)
        {
          if(Is<Escape>(C.d))
          {
            C.p = C.s;
            Escaped = true;
          }
          else if(SurrogateLead)
          {
            C.s = C.p;
            C.ErrorInfo = "Lead surrogate followed by unescaped character";
            C.State = Abort;
            break;
          }
          else if(Is<QuotationMark>(C.d))
            break;
          else if(not IsVanillaString)
            Text << C.d;
        }
        else
        {
          Escaped = false;
          if(Is<EscapedCodepoint>(C.d))
          {
            unicode HexDigits[4] = {16, 16, 16, 16};
            for(count i = 0; i < 4; i++)
              if(C.s < C.e)
                HexDigits[i] = Unicode::HexDigitValue(String::Decode(C.s, C.e));

            if(HexDigits[0] < 16 and HexDigits[1] < 16 and
              HexDigits[2] < 16 and HexDigits[3] < 16)
            {
              unicode u = (HexDigits[0] << 12) + (HexDigits[1] << 8) +
                (HexDigits[2] << 4) + HexDigits[3];

              if(Unicode::IsLeadSurrogate(u))
              {
                if(SurrogateLead)
                {
                  C.s = C.p;
                  C.ErrorInfo = "Lead surrogate followed by lead surrogate";
                  C.State = Abort;
                  break;
                }
                else
                  SurrogateLead = u;
              }
              else if(Unicode::IsTrailSurrogate(u))
              {
                if(not SurrogateLead)
                {
                  C.s = C.p;
                  C.ErrorInfo = "Trail surrogate with no lead surrogate";
                  C.State = Abort;
                  break;
                }
                Text << Unicode::FromSurrogatePair(SurrogateLead, u);
                SurrogateLead = 0;
              }
              else if(SurrogateLead)
              {
                C.s = C.p;
                C.ErrorInfo = "Lead surrogate not followed by tail";
                C.State = Abort;
                break;
              }
              else
                Text << u;
            }
            else
            {
              C.s = C.p;
              C.ErrorInfo = "Non-hex digits in escaped Unicode character";
              C.State = Abort;
              break;
            }
          }
          else if(SurrogateLead)
          {
            C.s = C.p;
            C.ErrorInfo = "Lead surrogate not followed by \\uXXXX";
            C.State = Abort;
            break;
          }
          else if(Is<EscapedQuotationMark>(C.d))
            Text << "\x22";
          else if(Is<EscapedReverseSolidus>(C.d))
            Text << "\\";
          else if(Is<EscapedSolidus>(C.d))
            Text << "/";
          else if(Is<EscapedBackspace>(C.d))
            Text << "\b";
          else if(Is<EscapedFormFeed>(C.d))
            Text << "\f";
          else if(Is<EscapedLineFeed>(C.d))
            Text << "\n";
          else if(Is<EscapedCarriageReturn>(C.d))
            Text << "\r";
          else if(Is<EscapedTab>(C.d))
            Text << "\t";
        }
      }
      if(C.State == Abort)
        return false;

      //Coerce to ratio if it is in the canonical form and non-empty.
      Ratio TextAsRatio;
      if(Text.Contains("/"))
      {
        TextAsRatio = Ratio::FromString(Text);
        if(TextAsRatio.ToString() != Text)
          TextAsRatio = Ratio();
      }

      if(C.StackKeys.z().IsNil())
      {
        if(TextAsRatio.IsEmpty())
          C.StackKeys.z() = Text;
        else
          C.StackKeys.z() = TextAsRatio;
      }
      else
      {
        if(not (*C.v)[C.StackKeys.z()].IsNil())
        {
          C.ErrorInfo = "Key redefined: ";
          C.ErrorInfo << C.StackKeys.z();
          C.State = Abort;
          return false;
        }
        if(TextAsRatio.IsEmpty())
          (*C.v)[C.StackKeys.z()] = Text;
        else
          (*C.v)[C.StackKeys.z()] = TextAsRatio;
      }
      C.State = C.States.Pop();
      return true;
    }

    ///Checks whether a number is formatted according to the specification.
    static bool IsNumberCorrectlyFormatted(const ascii* n)
    {
      /* JSON specification for numbers:
      Number        = [ Minus ] Integer [ Fraction ] [ Exponent ]
      DecimalPoint  = .
      Digit1To9     = 1-9
      ExponentSign  = e / E
      Exponent      = ExponentSign [ Minus / Plus ] 1*DIGIT
      Fraction      = DecimalPoint 1*DIGIT
      Integer       = Zero / ( Digit1To9 *DIGIT )
      Minus         = -
      Plus          = +
      Zero          = 0
      */

      //Check for optional minus sign.
      if(Is<Minus>(*n))
        n++;

      //Chomp integer.
      if(Is<Zero>(*n))
        n++;
      else if(Is<Digit1To9>(*n))
      {
        n++;
        while(Is<Digit>(*n))
          n++;
      }
      else
        return false;

      //If no more characters, then it is an integer.
      if(not *n)
        return true;

      //If next character is decimal, chomp the fraction.
      if(Is<DecimalPoint>(*n))
      {
        //Chomp decimal.
        n++;

        //Next character must be a digit.
        if(not Is<Digit>(*n++))
          return false;

        //Chomp remaining digits.
        while(Is<Digit>(*n))
          n++;

        //If no more characters, then it is a simple decimal number.
        if(not *n)
          return true;
      }

      //Next character must be an exponent sign.
      if(not Is<ExponentSign>(*n++))
        return false;

      //Next character must be plus or minus or a digit.
      if(Is<Plus>(*n) or Is<Minus>(*n))
        n++; //Chomp the plus/minus
      else if(not Is<Digit>(*n))
        return false;

      //Next character must be a digit.
      if(not Is<Digit>(*n++))
        return false;

      //Chomp remaining digits.
      while(Is<Digit>(*n))
        n++;

      //Final character must be null-terminator.
      if(*n)
        return false;

      return true;
    }

    ///Chomps a JSON number.
    static bool ChompNumeric(JSONParseContext& C)
    {
      /*Take advantage of the fact that only whitespace, comma, end bracket,
      and end brace can follow a number. None of these characters are permitted
      within a number, so the number can quickly be chomped according to the
      character set for representing numbers.

      To ensure that the number is correctly formatted, the
      IsNumberCorrectlyFormatted() takes the chomped number and parses it
      according to the actual specification.*/

      //Chomp all the numeric characters.
      String n;
      bool ForceFloatingPoint = false;
      n << C.d;
      C.p = C.s;
      while(C.s < C.e && (C.d = String::Decode(C.s, C.e)) != 0)
      {
        if(not Is<Numeric>(C.d))
          break;
        if(Is<DecimalPoint>(C.d))
          ForceFloatingPoint = true; //If expressed with a . force as number.
        n << C.d;
        C.p = C.s; //Backup to avoid chomping beyond the number.
      }

      //Validate the number according to the specification.
      if(not IsNumberCorrectlyFormatted(n.Merge()))
      {
        C.ErrorInfo = "Invalid number: ";
        C.ErrorInfo << n;
        C.State = Abort;
        return false;
      }

      //Store number as integer if it loses no precision.
      number Result = n.ToNumber();
      integer ResultAsInteger = integer(Result);
      if(not (*C.v)[C.StackKeys.z()].IsNil())
      {
        C.ErrorInfo = "Key redefined: ";
        C.ErrorInfo << C.StackKeys.z();
        C.State = Abort;
        return false;
      }
      if(not ForceFloatingPoint and
        Limits<number>::IsEqual(Result, number(ResultAsInteger)))
        (*C.v)[C.StackKeys.z()] = ResultAsInteger; //Number is an integer.
      else
        (*C.v)[C.StackKeys.z()] = Result; //Number is a floating-point.

      //Revert the state.
      C.State = C.States.Pop();

      /*Back up to the previous character so that the outer while loop will
      correctly continue with the first non-numeric character.*/
      C.s = C.p;

      return true;
    }

    ///Chomps a JSON value.
    static bool ChompValue(JSONParseContext& C)
    {
      if(Is<Whitespace>(C.d));
      else if(Is<False1>(C.d))
      {
        if(not (*C.v)[C.StackKeys.z()].IsNil())
        {
          C.ErrorInfo = "Key redefined: ";
          C.ErrorInfo << C.StackKeys.z();
          C.State = Abort;
          return false;
        }
        (*C.v)[C.StackKeys.z()] = false;
        C.State = ValueFalse2;
      }
      else if(Is<Null1>(C.d))
      {
        if(not (*C.v)[C.StackKeys.z()].IsNil())
        {
          C.ErrorInfo = "Key redefined: ";
          C.ErrorInfo << C.StackKeys.z();
          C.State = Abort;
          return false;
        }
        (*C.v)[C.StackKeys.z()].Clear();
        C.State = ValueNull2;
      }
      else if(Is<True1>(C.d))
      {
        if(not (*C.v)[C.StackKeys.z()].IsNil())
        {
          C.ErrorInfo = "Key redefined: ";
          C.ErrorInfo << C.StackKeys.z();
          C.State = Abort;
          return false;
        }
        (*C.v)[C.StackKeys.z()] = true;
        C.State = ValueTrue2;
      }
      else if(Is<BeginArray>(C.d))
      {
        if(not (*C.v)[C.StackKeys.z()].IsNil())
        {
          C.ErrorInfo = "Key redefined: ";
          C.ErrorInfo << C.StackKeys.z();
          C.State = Abort;
          return false;
        }
        (*C.v)[C.StackKeys.z()].NewArray();
        C.Stack.Push(&(*C.v)[C.StackKeys.z()]);
        C.State = WaitingForFirstValue;
        C.StackKeys.Push(Value(0));
        C.States.Push(WaitingForArrayEnd);
      }
      else if(Is<BeginObject>(C.d))
      {
        if(not (*C.v)[C.StackKeys.z()].IsNil())
        {
          C.ErrorInfo = "Key redefined: ";
          C.ErrorInfo << C.StackKeys.z();
          C.State = Abort;
          return false;
        }
        (*C.v)[C.StackKeys.z()].NewTree();
        C.Stack.Push(&(*C.v)[C.StackKeys.z()]);
        C.State = WaitingForName;
        C.StackKeys.Push(Value()); //Nil indicates to create new key.
        C.States.Push(WaitingForObjectEnd);
      }
      else if(Is<QuotationMark>(C.d))
      {
        if(not ChompString(C))
          return false;
      }
      else if(Is<Numeric>(C.d))
      {
        if(not ChompNumeric(C))
          return false;
      }
      else
      {
        C.ErrorInfo = "Unexpected character";
        C.State = Abort;
      }
      return true;
    }

    ///Chomps the beginning of a JSON stream.
    static bool ChompBeginning(JSONParseContext& C)
    {
      if(Is<Whitespace>(C.d));
      else if(Is<BeginArray>(C.d))
      {
        C.Stack.z()->NewArray();
        C.StackKeys.Push(Value(0));
        C.State = WaitingForFirstValue;
        C.States.Push(WaitingForArrayEnd);
      }
      else if(Is<BeginObject>(C.d))
      {
        C.Stack.z()->NewTree();
        C.StackKeys.Push(Value()); //Nil indicates to create new key.
        C.State = WaitingForName;
        C.States.Push(WaitingForObjectEnd);
      }
      else
      {
        C.ErrorInfo = "Unexpected character";
        C.State = Abort;
      }
      return true;
    }

    ///Chomps the name of an name-value pair.
    static bool ChompName(JSONParseContext& C)
    {
      if(Is<QuotationMark>(C.d))
      {
        if(not ChompString(C))
          return false;
        C.States.Push(WaitingForObjectEnd);
        C.State = WaitingForNameSeparator;
      }
      else if(Is<Whitespace>(C.d))
      {
      }
      else if(Is<EndObject>(C.d))
      {
        C.Stack.Pop();
        C.StackKeys.Pop();
        C.States.Pop(); //Pop twice since object ended without any keys.
        C.State = C.States.Pop();
        return true;
      }
      else
      {
        C.ErrorInfo = "Expected name string";
        C.State = Abort;
      }
      return true;
    }

    ///Chomps the name separator in an object name-value pair.
    static bool ChompNameSeparator(JSONParseContext& C)
    {
      if(Is<NameSeparator>(C.d))
        C.State = WaitingForValue;
      else if(not Is<Whitespace>(C.d))
      {
        C.ErrorInfo = "Expected name-value separator";
        C.State = Abort;
      }
      return true;
    }

    ///Chomps the first value in an array.
    static bool ChompFirstValue(JSONParseContext& C)
    {
      if(Is<EndArray>(C.d))
      {
        C.Stack.Pop();
        C.StackKeys.Pop();
        C.States.Pop(); //Pop twice since array ended early.
        C.State = C.States.Pop();
        return true;
      }
      ChompValue(C);
      return true;
    }

    ///Chomps the end of an object.
    static bool ChompObjectEnd(JSONParseContext& C)
    {
      if(Is<Whitespace>(C.d));
      else if(Is<ValueSeparator>(C.d))
      {
        C.StackKeys.z() = Value();
        C.State = WaitingForName;
        C.States.Push(WaitingForObjectEnd);
      }
      else if(Is<EndObject>(C.d))
      {
        C.Stack.Pop();
        C.StackKeys.Pop();
        C.State = C.States.Pop();
      }
      else
      {
        C.ErrorInfo = "Unexpected character";
        C.State = Abort;
      }
      return true;
    }

    ///Chomps the end of an array.
    static bool ChompArrayEnd(JSONParseContext& C)
    {
      if(Is<Whitespace>(C.d));
      else if(Is<ValueSeparator>(C.d))
      {
        C.StackKeys.z() = integer(C.StackKeys.z()) + 1;
        C.State = WaitingForValue;
        C.States.Push(WaitingForArrayEnd);
      }
      else if(Is<EndArray>(C.d))
      {
        C.Stack.Pop();
        C.StackKeys.Pop();
        C.State = C.States.Pop();
      }
      else
      {
        C.ErrorInfo = "Unexpected character";
        C.State = Abort;
      }
      return true;
    }

    public:

    ///Exports a value to JSON data.
    static void Export(const Value& ValueToExport, String& JSONData,
      bool WithWhitespace, bool WithRoot = true)
    {
      JSONData = ValueToExport.ExportJSON(WithWhitespace, WithRoot);
    }

    ///Exports a value to JSON data.
    static String Export(const Value& ValueToExport)
    {
      return ValueToExport.ExportJSON(true);
    }

    ///Exports a value as a JSON result without the required root.
    static String ExportResult(const Value& ValueToExport)
    {
      return ValueToExport.ExportJSON(false, false);
    }

    private:

    ///Coerces any vector and rectangle data to actual vectors and rectangles.
    static void CoerceSpecialTypes(Value& v)
    {
      const Value& v_const = v;
      if(v_const.IsArray())
      {
        if(v_const.n() == 3 and v_const[0].AsString() == "_JSONVector")
          v = Vector(v_const[1].AsNumber(), v_const[2].AsNumber());
        else if(v_const.n() == 5 and v_const[0].AsString() == "_JSONRectangle")
          v = Box(
            Vector(v_const[1].AsNumber(), v_const[2].AsNumber()),
            Vector(v_const[3].AsNumber(), v_const[4].AsNumber()));
        else
        {
          //Recurse through array and coerce each element.
          for(count i = 0, n = v_const.n(); i < n; i++)
            CoerceSpecialTypes(v[i]);
        }
      }
      else if(v_const.IsTree())
      {
        //Enumerate tree and coerce each element.
        Array<Value> Keys;
        v_const.EnumerateKeys(Keys);
        for(count i = 0; i < Keys.n(); i++)
          CoerceSpecialTypes(v[Keys[i]]);
      }
    }

    public:

    /**Imports a value from JSON data. Check for the key _JSONError to see if
    there was an error and _JSONErrorInfo for more information.*/
    static Value Import(const String& JSONData)
    {
      Value v;
      Import(JSONData, v);
      return v;
    }

    /**Imports JSON data to a value. Returns false if there was a parser error.
    Details regarding the error can be found in the keys _JSONError and
    _JSONErrorInfo.*/
    static bool Import(const String& JSONData, Value& ImportedValue)
    {
      //Check for other encodings.
      if(JSONData.n() >= 4)
      {
        String DetectedEncoding;
        if(JSONData[0] == 0 and JSONData[1] == 0 and JSONData[2] == 0)
          DetectedEncoding = "Detected UTF-32BE";
        else if(JSONData[0] == 0 and JSONData[2] == 0)
          DetectedEncoding = "Detected UTF-16BE";
        else if(JSONData[1] ==  0 and JSONData[2] == 0 and JSONData[3] == 0)
          DetectedEncoding = "Detected UTF-32LE";
        else if(JSONData[1] == 0 and JSONData[3] == 0)
          DetectedEncoding = "Detected UTF-16LE";

        if(DetectedEncoding)
        {
          ImportedValue.Clear();
          ImportedValue["_JSONError"] =
            "Encodings other than UTF-8 are unsupported.";
          ImportedValue["_JSONErrorInfo"] = DetectedEncoding;
          return false;
        }
      }

      JSONParseContext C;
      C.p = C.s = C.Start = reinterpret_cast<const byte*>(JSONData.Merge());
      C.e = &C.s[JSONData.n()];
      C.Stack.Add() = &ImportedValue;
      C.Stack.z()->Clear();
      C.States.Push(Ending);
      C.State = Beginning;
      while(C.State != Abort && C.s < C.e &&
        (C.d = String::Decode(C.s, C.e)) != 0)
      {
        C.v = (C.Stack.n() ? C.Stack.z() : &C.StateValueDummy);
        switch(C.State)
        {
        //Chomp the beginning of the stream.
        case Beginning:               ChompBeginning(C);     break;

        //Chomp values for objects and arrays.
        case WaitingForFirstValue:    ChompFirstValue(C);    break;
        case WaitingForValue:         ChompValue(C);         break;
        case WaitingForArrayEnd:      ChompArrayEnd(C);      break;

        //Chomp object name-value pairs.
        case WaitingForName:          ChompName(C);          break;
        case WaitingForNameSeparator: ChompNameSeparator(C); break;
        case WaitingForObjectEnd:     ChompObjectEnd(C);     break;

        //Chomp through the rest of the literal text enforcing each letter.
        case ValueFalse2:C.State=(Is<False2>(C.d) ? ValueFalse3 : Abort); break;
        case ValueFalse3:C.State=(Is<False3>(C.d) ? ValueFalse4 : Abort); break;
        case ValueFalse4:C.State=(Is<False4>(C.d) ? ValueFalse5 : Abort); break;
        case ValueFalse5:C.State=(Is<False5>(C.d) ? C.States.Pop():Abort);break;
        case ValueNull2: C.State=(Is<Null2>(C.d) ? ValueNull3 : Abort);   break;
        case ValueNull3: C.State=(Is<Null3>(C.d) ? ValueNull4 : Abort);   break;
        case ValueNull4: C.State=(Is<Null4>(C.d) ? C.States.Pop():Abort); break;
        case ValueTrue2: C.State=(Is<True2>(C.d) ? ValueTrue3 : Abort);   break;
        case ValueTrue3: C.State=(Is<True3>(C.d) ? ValueTrue4 : Abort);   break;
        case ValueTrue4: C.State=(Is<True4>(C.d) ? C.States.Pop():Abort); break;

        //Chomp through trailing whitespace.
        case Ending: if(not Is<Whitespace>(C.d)) C.State = Abort; break;

        //In case of abort condition, break.
        case Abort: break;
        }
      }

      if(C.State == Abort or C.Stack.n() or C.States.n())
      {
        ImportedValue.Clear();
        ImportedValue["_JSONError"] = C.GetErrorString();
        ImportedValue["_JSONErrorInfo"] = C.ErrorInfo;
        return false;
      }

      //Coerce special tagged arrays to vectors and rectangles.
      CoerceSpecialTypes(ImportedValue);

      return true;
    }

    ///Imports a JSON result even if not wrapped in array or object.
    static void ImportResult(const String& Result, Value& ResultValue)
    {
      String Trimmed = Result;
      Trimmed.Trim();
      if(Trimmed.StartsWith("{") or Trimmed.StartsWith("["))
        JSON::Import(Trimmed, ResultValue);
      else
      {
        Trimmed.Prepend("[");
        Trimmed.Append("]");
        Value ContainedValue;
        JSON::Import(Trimmed, ContainedValue);
        ResultValue = ContainedValue.a();
      }
    }
  };

  //Set the default JSONParseState.
  template <> inline Nothing<PRIM_NAMESPACE::JSON::JSONParseState>::operator
    JSON::JSONParseState() const {return JSON::Abort;}
}
#endif
