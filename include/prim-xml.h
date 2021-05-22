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

#ifndef PRIM_INCLUDE_XML_H
#define PRIM_INCLUDE_XML_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

/*Where possible use the JSON and Value classes instead. Eventually, the XML
class should be revisited, possibly also making use of the Value class.*/

namespace PRIM_NAMESPACE {
///Classes for parsing and binding XML as a DOM tree.
class XML
{
  public:

  ///Base class from which all bound elements derive.
  class Parser
  {
    public:

    ///Internal representation of a segment of text within the XML stream.
    class Token
    {
      public:

      const ascii* Position;
      count ByteLength;
      String Text;

      Token() : Position(0), ByteLength(0) {}

      ///Copy constructor for token.
      Token(const Token& Other) {*this = Other;}

      ///Caches a string copy of the token in member Text.
      void UpdateString()
      {
        Text.Clear();
        if(ByteLength and Position)
          Text.Append(reinterpret_cast<const byte*>(Position), ByteLength);
      }

      Token& operator = (const Token& Other)
      {
        Position = Other.Position;
        ByteLength = Other.ByteLength;
        Text = Other.Text;

        return *this;
      }
    };

    /**Parses the next word. A word is found by first skipping over any
    whitespace (as defined by an array of Unicode codepoints terminated by
    zero) and then parsing a run of characters until either a whitespace
    delimiter is found or a word delimiter is found (also defined by an array of
    Unicode codepoints terminated by zero). The parser automatically increments
    the pointer to the first character after the word (whether it is whitespace
    or not). If the first character in a word is a word delimiter then only
    that character will be returned.*/
    static Token ParseNextWord(const ascii*& UTF8StringReference,
      const ascii* MarkupEnd,
      const unicode* WhiteSpaceDelimiters,
      const unicode* AdditionalWordDelimiters)
    {
      const byte* UTF8String =
        reinterpret_cast<const byte*>(UTF8StringReference);
      //1) Skip through the whitespace.
      Token Word;
      {
        const byte* ptrPreviousCharacter;
        bool isSearchingWhiteSpace = true;
        unicode Character = 0;
        while(isSearchingWhiteSpace)
        {
          //Decode the next UTF8 character.
          ptrPreviousCharacter = UTF8String;
          Character = String::Decode(UTF8String,
            reinterpret_cast<const byte*>(MarkupEnd));

          //At the end of stream there is no word.
          if(not Character)
          {
            Word.UpdateString();
            return Word;
          }

          //Check to see if the character is whitespace.
          bool isWhiteSpace = false;
          const unicode* WhiteSpaceDelimiterCheck = WhiteSpaceDelimiters;
          while(*WhiteSpaceDelimiterCheck)
          {
            if(Character == *WhiteSpaceDelimiterCheck)
            {
              isWhiteSpace = true;
              break;
            }
            WhiteSpaceDelimiterCheck++;
          }

          if(not isWhiteSpace)
          {
            //The beginning of the word has been found.
            isSearchingWhiteSpace = false;

            /*Decrement the UTF8 string pointer so that it points to the
            beginning of the word.*/
            UTF8String = ptrPreviousCharacter;
            Word.Position = reinterpret_cast<const ascii*>(UTF8String);
          }

          //Otherwise keep skipping whitespace characters.
        }
      }

      /*2) Determine the word's length by searching for the next whitespace
      or word delimiter.*/
      {
        const byte* ptrPreviousCharacter;
        bool isSearchingWord = true;
        unicode Character = 0;
        count WordLength = 0;
        while(isSearchingWord)
        {
          //Decode the next UTF8 character.
          ptrPreviousCharacter = UTF8String;
          Character = String::Decode(UTF8String,
            reinterpret_cast<const byte*>(MarkupEnd));

          //At the end of the stream. The word runs to the end of the stream.
          if(not Character)
          {
            UTF8String = ptrPreviousCharacter;
            Word.ByteLength = count(
              reinterpret_cast<const ascii*>(UTF8String) - Word.Position);
            Word.UpdateString();
            return Word;
          }

          //Increment the character count.
          WordLength++;

          //Check to see if the character is whitespace.
          bool isDelimiter = false;
          const unicode* WhiteSpaceDelimiterCheck = WhiteSpaceDelimiters;
          while(*WhiteSpaceDelimiterCheck)
          {
            if(Character == *WhiteSpaceDelimiterCheck)
            {
              isDelimiter = true;
              break;
            }
            WhiteSpaceDelimiterCheck++;
          }

          //Check to see if the character is word delimiter.
          const unicode* AdditionalWordDelimiterCheck =
            AdditionalWordDelimiters;
          while(*AdditionalWordDelimiterCheck and not isDelimiter)
          {
            if(Character == *AdditionalWordDelimiterCheck)
            {
              isDelimiter = true;
              break;
            }
            AdditionalWordDelimiterCheck++;
          }

          if(isDelimiter)
          {
            //The beginning of the word has been found.
            isSearchingWord = false;

            /*Decrement the UTF8 string pointer so that it points to the
            beginning of the word. The exception is if the word is one
            character in which case there is no need for a decrement.*/
            if(WordLength > 1)
              UTF8String = ptrPreviousCharacter;

            Word.ByteLength = count(
              reinterpret_cast<const ascii*>(UTF8String) - Word.Position);
          }

          //Otherwise keep decoding characters from the word.
        }
      }

      Word.UpdateString();
      UTF8StringReference = reinterpret_cast<const ascii*>(UTF8String);
      return Word;
    }

    ///Defines delimiters used for parsing.
    class Delimiters
    {
      public:
      static const unicode None[1];
      static const unicode WhiteSpace[5];
      static const unicode TagEntry[2];
      static const unicode TagExit[2];
      static const unicode TagName[5];
      static const unicode TagAttributeName[4];
      static const unicode TagAttributeValue[4];
      static const unicode TagAttributeValueSingle[3];
      static const unicode TagAttributeValueDouble[3];
    };

    ///Represents a parsing error.
    class Error
    {
      public:

      typedef count Category;

      ///Represents a type of parsing error.
      class Categories
      {
        public:

        static const Category None = 0;
        static const Category EmptyDocument = 1;
        static const Category UnmatchedBracket = 2;
        static const Category UnexpectedTagName = 3;
        static const Category UnexpectedCharacter = 4;
        static const Category UninterpretableElement = 5;
        static const Category UnmatchedTagName = 6;
      };

      ///Type of error.
      Category Type;

      ///Pointer to beginning of the token.
      const ascii* Position;

      ///Pointer to beginning of markup.
      const ascii* Original;

      ///Index of position in original after UpdateIndex() is called.
      count Index;

      ///Default constructor produces object with no error.
      Error() : Type(Categories::None), Position(0), Original(0), Index(0) {}

      ///Constructor taking an error category and position string.
      Error(Category Type_, const ascii* Position_) :
        Type(Type_), Position(Position_), Original(0), Index(0)
      {
      }

      ///Returns a description of the error and its probable location.
      String GetDescription()
      {
        switch(Type)
        {
          case Categories::None:
            return "";
          case Categories::EmptyDocument:
            return "The document appears to empty.";
          case Categories::UnmatchedBracket:
            C::Error() >> "There is an unbalanced bracket.";
            break;
          case Categories::UnexpectedTagName:
            C::Error() >> "There was an unexpected tag name.";
            break;
          case Categories::UnexpectedCharacter:
            C::Error() >> "There was an unexpected character.";
            break;
          case Categories::UninterpretableElement:
            C::Error() >> "There was an uninterpretable element.";
            break;
          case Categories::UnmatchedTagName:
            C::Error() >> "There was an unbalanced tag name.";
            break;
          default:
            break;
        }

        //Find the end of the string.
        const ascii* MarkupEnd = &Original[String::LengthOf(Original) + 1];

        //Find 20 characters before the start of the error.
        const ascii* OriginalCopy = Original;
        const ascii* LineBegin = Original;
        const ascii* HeaderStart = 0;
        count CurrentIndex = 0;
        count CurrentLine = 1;

        while(CurrentIndex < Index)
        {
          //Save this position.
          if(CurrentIndex == Index - 20)
            HeaderStart = OriginalCopy;

          const byte* OriginalCopyCast =
            reinterpret_cast<const byte*>(OriginalCopy);
          unicode Value = String::Decode(OriginalCopyCast,
            reinterpret_cast<const byte*>(MarkupEnd));
          OriginalCopy = reinterpret_cast<const ascii*>(OriginalCopyCast);
          if(Value == unicode(*String::Newline))
          {
            CurrentLine++;
            LineBegin = OriginalCopy;
          }
          CurrentIndex++;
        }

        //Start on the same line as the error.
        count SpacesToUse = 20;
        OriginalCopy = HeaderStart;
        if(LineBegin > OriginalCopy)
        {
          SpacesToUse = 20 - count(LineBegin - OriginalCopy);
          OriginalCopy = LineBegin;
        }

        //Copy 40 characters of the original XML string.
        String ErrorLocator;

        for(count i = 0; i < 40; i++)
        {
          const byte* OriginalCopyCast =
            reinterpret_cast<const byte*>(OriginalCopy);
          unicode Value = String::Decode(OriginalCopyCast,
            reinterpret_cast<const byte*>(MarkupEnd));
          OriginalCopy = reinterpret_cast<const ascii*>(OriginalCopyCast);
          if(not Value)
            break;
          ErrorLocator.Append(Value);
        }

        //Add a pointer to where the error is in the string.
        ErrorLocator.Replace("\n", " ");
        ErrorLocator.Append("\n");
        for(count i = 0; i < SpacesToUse; i++)
          ErrorLocator.Append(" ");
        ErrorLocator.Append("^");
        String LineNumber = "Starting at line ";
        LineNumber << CurrentLine << ":\n";
        ErrorLocator.Prepend(LineNumber);

        return ErrorLocator;
      }

      ///Given the original string and position string finds the position index.
      void UpdateIndex(const ascii* MarkupEnd)
      {
        Index = 0;
        if(Original != 0 and Position != 0)
        {
          const ascii* OriginalCopy = Original;
          while(OriginalCopy < Position)
          {
            const byte* OriginalCopyCast =
              reinterpret_cast<const byte*>(OriginalCopy);
            String::Decode(OriginalCopyCast,
              reinterpret_cast<const byte*>(MarkupEnd));
            OriginalCopy = reinterpret_cast<const ascii*>(OriginalCopyCast);
            Index++;
          }
        }
      }

      ///Casts object into a boolean with true meaning it contains an error.
      operator bool () const
      {
        return Type != Categories::None;
      }
    };
  };

  class Element;
  class Text;
  class Document;

  ///Represents some kind of content within an element.
  class Object
  {
    /*An object can either be an element or a string of text. It is necessary
    to store a list of generic objects so that something like this:
    <foo>hello<foo2></foo2>there</foo> can accurately be represented.*/

    public:

    ///Returns whether the object is an element.
    virtual Element* IsElement() {return 0;}

    ///Returns whether the object is text.
    virtual Text* IsText() {return 0;}

    ///Virtual destructor
    virtual ~Object();
  };

  ///Represents string content in an element.
  class Text : public Object, public String
  {
    public:
    friend class Document;

    ///Constructor creates an empty text object.
    Text() {}

    ///Constructor creates a text object from a string.
    Text(const String& NewString) : String(NewString) {}

    ///Returns the pointer to this text object.
    Text* IsText() {return this;}

    ///Virtual destructor
    virtual ~Text();
  };

  ///Represents an XML tag within an element and possibly between text.
  class Element : public Object
  {
    public:

    ///Give Document access to this class.
    friend class Document;

    ///Stores an XML attribute by its name and value.
    class Attribute
    {
      public:

      ///Name of the attribute
      String Name;

      ///Value of the attribute
      String Value;

      ///Assigns an attribute from another.
      Attribute& operator = (const Attribute& Other)
      {
        Name = Other.Name;
        Value = Other.Value;
        return *this;
      }
    };

    protected:

    ///Stores the name of the element tag.
    String Name;

    ///Stores a list of the element attributes.
    List<Attribute> Attributes;

    ///Stores a list of objects containing text and subelements.
    List<Object*> Objects;

    public:

    ///Returns a pointer to this element object.
    Element* IsElement() {return this;}

    ///Constructor creates an empty element.
    Element() {}

    ///Constructor specifying the tag name of the element.
    Element(const String& Name_)
    {
      Name = Name_;
    }

    ///Returns the tag name of this element.
    const String& GetName() {return Name;}

    ///Sets the tag name of this element.
    void SetName(const String& NewName)
    {
      Name = NewName;
    }

    ///Returns the attribute list for this element.
    const List<Attribute>& GetAttributes() {return Attributes;}

    ///Adds an attribute to the element.
    void AddAttribute(const Attribute& NewAttribute)
    {
      Attributes.Add() = NewAttribute;
    }

    ///Adds an attribute from a string pair.
    void AddAttribute(const String& Name_, const String& Value)
    {
      Attribute a;
      a.Name = Name_;
      a.Value = Value;
      Attributes.Add() = a;
    }

    /**Gets the value of a particular attribute. If the attribute can not be
    located, an empty string is returned.*/
    String GetAttributeValue(const ascii* Attribute)
    {
      for(count i = 0; i < Attributes.n(); i++)
      {
        if(Attributes[i].Name == Attribute)
          return Attributes[i].Value;
      }
      return "";
    }

    /**Removes an attribute by name from the attribute list. Note that it will
    remove all instances of the attribute (however, there should only be
    one).*/
    void RemoveAttribute(const String& AttributeName)
    {
      for(count i = 0; i < Attributes.n(); i++)
      {
        if(Attributes[i].Name == AttributeName)
        {
          Attributes.Remove(i);
          i--;
        }
      }
    }

    ///Returns a list of the objects in this element.
    const List<Object*>& GetObjects() {return Objects;}

    ///Adds a pointer to an object in the object list.
    void AddObject(Object* NewObject, bool AddAtBeginning = false)
    {
      if(AddAtBeginning)
        Objects.Prepend(NewObject);
      else
        Objects.Append(NewObject);
    }

    ///Adds a pointer to an object in the object list.
    void AddElementInOrder(Element* NewElement, const String* Order)
    {
      String& NewElementName = NewElement->Name;
      count NewElementOrder = -1;
      count OrderNumber = 0;

      const String* ptrOrder = Order;

      while(*ptrOrder != "")
      {
        if(*ptrOrder == NewElementName)
          NewElementOrder = OrderNumber;
        ptrOrder++;
        OrderNumber++;
      }

      if(NewElementOrder == -1)
      {
        /*Could not find the element in the ordering scheme, so just add to the
        end and exit.*/
        AddObject(NewElement);
        return;
      }

      count ElementToInsertAfter = -1;
      for(count i = Objects.n() - 1; i >= 0; i--)
      {
        if(Element* e = Objects[i]->IsElement())
        {
          //Get the order index of the element in question.
          count CurrentOrder = -1;
          for(count j = 0; j < OrderNumber; j++)
          {
            if(Order[j] == e->Name)
            {
              CurrentOrder = j;
              break;
            }
          }

          if(CurrentOrder != -1 and CurrentOrder <= NewElementOrder)
          {
            /*Found either the same kind of element or the last element that
            would appear before this kind of element.*/
            ElementToInsertAfter = i;
            break;
          }
        }
      }

      if(ElementToInsertAfter != -1)
      {
        /*Insert the element after either the last element of the same kind or
        the last element whose order comes before an element of this kind.*/
        Objects.InsertAfter(NewElement,ElementToInsertAfter);
      }
      else if(NewElementOrder == 0)
      {
        /*This happens when the new element is ordered first and thus has no
        other element to go after.*/
        AddObject(NewElement, true);
      }
      else
      {
        //Unknown situation. Just add the object to the end in this case.
        AddObject(NewElement, false);
      }
    }

    ///Removes all children text and element objects.
    void RemoveAllObjects()
    {
      Objects.RemoveAndDeleteAll();
    }

    ///Removes all elements by a particular tag name.
    void RemoveElementType(const String& ElementTagName)
    {
      for(count i = 0; i < Objects.n(); i++)
      {
        if(Element* e = Objects[i]->IsElement())
        {
          if(e->Name == ElementTagName)
          {
            Objects.RemoveAndDelete(i);
            i--;
          }
        }
      }
    }

    /**Removes an element by its searching for its tag name and ID attribute.
    If there are more than one by the same ID, then they are all removed.*/
    void RemoveElementByID(const String& ElementTagName, const String& ID)
    {
      for(count i = 0; i < Objects.n(); i++)
      {
        if(Element* e = Objects[i]->IsElement())
        {
          if(e->Name == ElementTagName and e->GetAttributeValue("id") == ID)
          {
            Objects.RemoveAndDelete(i);
            i--;
          }
        }
      }
    }

    //Gets the ith child of a particular tag name.
    Element* GetChildByTagName(const String& Tag, count Index = 0)
    {
      count Count = 0;
      for(count i = 0; i < Objects.n(); i++)
      {
        if(Element* e = Objects[i]->IsElement())
        {
          if(e->GetName() == Tag)
          {
            if(Count == Index)
              return e;
            Count++;
          }
        }
      }
      return 0;
    }

    ///Gets the ith child of a particular type.
    template<class ChildType>
    ChildType* GetChildOfType(count Index = 0)
    {
      count Count = 0;
      for(count i = 0; i < Objects.n(); i++)
      {
        if(ChildType* c = dynamic_cast<ChildType*>(Objects[i]->IsElement()))
        {
          if(Count == Index)
            return c;
          Count++;
        }
      }
      return 0;
    }

    //Gets the ith child of a particular tag name.
    count CountChildrenByTagName(const String& Tag)
    {
      count Count = 0;
      for(count i = 0; i < Objects.n(); i++)
      {
        if(Element* e = Objects[i]->IsElement())
        {
          if(e->GetName() == Tag)
            Count++;
        }
      }
      return Count;
    }

    ///Determines the number of children in an element of a particular type.
    template<class ChildType>
    count CountChildrenOfType()
    {
      count Count = 0;
      for(count i = 0; i < Objects.n(); i++)
      {
        if(dynamic_cast<ChildType*>(Objects[i]->IsElement()))
          Count++;
      }
      return Count;
    }

    ///Virtual destructor.
    virtual ~Element();

  protected:
    /**Overloaded method for creating virtual elements from an arbitrary name.
    /code
    if(TagName == "foo")
      return new foo; //'foo' is a derived Element.
    else if(TagName == "blah")
      return new blah; //'blah' is another derived Element.
    else
      return 0; //Indicates that a generic Element will be created.
    /endcode*/
    virtual Element* CreateChild(const String& TagName)
    {
      TagName.Merge(); //Stop unreferenced formal parameter warning.
      return 0;
    }

    /**Used by virtual elements to cache data in a more efficient manner. This
    is called automatically immediately after parsing. Use this to store cached
    copies of any data so that the structure of the XML document can be used as
    a way to facilitate organization without having to parse strings of text
    every time they are accessed.*/
    virtual bool Interpret()
    {
      bool Success = true;
      for(count i = 0; i < Objects.n(); i++)
      {
        if(Element* e = dynamic_cast<Element*>(Objects[i]))
        {
          if(not e->Interpret())
            Success = false;
        }
      }
      return Success;
    }

    /**Translates cached data in virtual elements back into the XML structure.
    This method is automatically called before XML data is saved to file, for
    example.*/
    virtual void Translate()
    {
      for(count i = 0; i < Objects.n(); i++)
      {
        if(Element* e = dynamic_cast<Element*>(Objects[i]))
          e->Translate();
      }
    }

    private:

    ///Parses a block of markup.
    Parser::Error Parse(const ascii*& Markup, const ascii* MarkupEnd)
    {
      //Local variables to use for parsing.
      Parser::Token t;

      //Remember the original location of the markup start.
      const ascii* m = Markup;

      //Entering tag.
      t = Parser::ParseNextWord(Markup, MarkupEnd,
        Parser::Delimiters::WhiteSpace, Parser::Delimiters::TagEntry);
      if(t.Text != "<")
        return Parser::Error(Parser::Error::Categories::EmptyDocument, m);

      //Parse tag name.
      t = Parser::ParseNextWord(Markup, MarkupEnd,
        Parser::Delimiters::WhiteSpace, Parser::Delimiters::TagName);
      if(t.Text == "" or t.Text.Contains(">") or t.Text.Contains("<"))
        return Parser::Error(Parser::Error::Categories::UnexpectedCharacter, m);
      Name = t.Text;

      //Parse the attributes.
      bool IsParsingAttributes = true;
      while(IsParsingAttributes)
      {
        t = Parser::ParseNextWord(Markup, MarkupEnd,
          Parser::Delimiters::WhiteSpace, Parser::Delimiters::TagAttributeName);

        if(t.Text == "/")
        {
          //Element is in the self-closing notation, i.e. <br/>.

          //Look for the exit character.
          t = Parser::ParseNextWord(Markup, MarkupEnd,
            Parser::Delimiters::WhiteSpace, Parser::Delimiters::TagExit);
          if(t.Text != ">")
            return Parser::Error(Parser::Error::Categories::UnexpectedCharacter,
              m);

          //Element is finished parsing, so return.
          return Parser::Error();
        }
        else if(t.Text == ">")
        {
          //There are no more attributes.
          IsParsingAttributes = false;
        }
        else if(t.Text == "=")
        {
          return Parser::Error(Parser::Error::Categories::UnexpectedCharacter,
            m);
        }
        else
        {
          //Parsed attribute name.
          Attributes.Add();
          Attributes.z().Name = t.Text;

          //Look for the equal sign.
          t = Parser::ParseNextWord(Markup, MarkupEnd,
            Parser::Delimiters::WhiteSpace,
            Parser::Delimiters::TagAttributeName);
          if(t.Text != "=")
            return Parser::Error(Parser::Error::Categories::UnexpectedCharacter,
              m);

          //Look for the beginning quotation mark.
          t = Parser::ParseNextWord(Markup, MarkupEnd,
            Parser::Delimiters::WhiteSpace,
            Parser::Delimiters::TagAttributeValue);
          if(t.Text != "\"" and t.Text != "'")
            return Parser::Error(Parser::Error::Categories::UnexpectedCharacter,
              m);

          //Parse the attribute text.
          t = Parser::ParseNextWord(Markup, MarkupEnd, Parser::Delimiters::None,
            (t.Text == "'" ? Parser::Delimiters::TagAttributeValueSingle :
            Parser::Delimiters::TagAttributeValueDouble));

          if(t.Text != "\"" and t.Text != "'")
          {
            //Attribute value is a non-empty string.
            Attributes.z().Value = t.Text;

            //Look for closing quotation mark.
            t = Parser::ParseNextWord(Markup, MarkupEnd,
              Parser::Delimiters::WhiteSpace,
              Parser::Delimiters::TagAttributeValue);
            if(t.Text != "\"" and t.Text != "'")
              return Parser::Error(
                Parser::Error::Categories::UnexpectedCharacter, m);
          }
        }
      }

      //Parse the text and element objects.
      bool IsParsingObjects = true;
      while(IsParsingObjects)
      {
        //Save the parse pointer in case it is a new tag.
        const ascii* TagBeginning = Markup;

        t = Parser::ParseNextWord(Markup, MarkupEnd, Parser::Delimiters::None,
          Parser::Delimiters::TagEntry);
        if(t.Text == "<") //Making new element or closing this one.
        {
          t = Parser::ParseNextWord(Markup, MarkupEnd,
            Parser::Delimiters::WhiteSpace, Parser::Delimiters::TagName);

          if(t.Text == "!")
          {
            //Inside of a comment. Skip to the '>' character.
            t = Parser::ParseNextWord(Markup, MarkupEnd,
              Parser::Delimiters::None, Parser::Delimiters::TagExit);
            t = Parser::ParseNextWord(Markup, MarkupEnd,
              Parser::Delimiters::None, Parser::Delimiters::TagExit);
            if(t.Text != ">")
              return Parser::Error(
                Parser::Error::Categories::UnexpectedCharacter, m);
          }
          else if(t.Text == "/")
          {
            //Closing this element. Make sure tag matches.
            t = Parser::ParseNextWord(Markup, MarkupEnd,
              Parser::Delimiters::WhiteSpace, Parser::Delimiters::TagName);
            if(t.Text != Name)
              return Parser::Error(Parser::Error::Categories::UnmatchedTagName,
                m);

            //Look for '>'
            t = Parser::ParseNextWord(Markup, MarkupEnd,
              Parser::Delimiters::WhiteSpace, Parser::Delimiters::TagExit);
            if(t.Text != ">")
              return Parser::Error(
                Parser::Error::Categories::UnexpectedCharacter, m);

            IsParsingObjects = false;
          }
          else //New child element
          {
            //Create new child element using virtual overloaded method.
            Element* e = CreateChild(t.Text);
            if(not e)
              e = new Element; //Default to generic element.

            //Add the object (with its children) to the object list.
            Objects.Add() = e;

            //Reverse the markup pointer to the beginning of the tag.
            Markup = TagBeginning;

            //Parse the child element.
            Parser::Error Error = e->Parse(Markup, MarkupEnd);

            /*If there was an error, then the failure cascades and the entire
            parse aborts.*/
            if(Error)
              return Error;

            /*We will let the root element trigger Interpret() after the
            document has been completely parsed. This is done because the
            Interpret() method is itself recursive.*/
          }
        }
        else
        {
          //Make a new text object.
          Text* NewText = new Text;
          *NewText << t.Text;
          Objects.Add() = NewText;
        }
      }
      return Parser::Error();
    }

    public:

    ///Appends this element tag, contents, and closing tag to a string.
    void AppendToString(String& XMLOutput);

    ///Returns all of the element text as a single string.
    String GetAllSubTextAsString()
    {
      String s;
      for(count i = 0; i < Objects.n(); i++)
      {
        Text* t = Objects[i]->IsText();
        if(t)
          s << *static_cast<String*>(t);
      }
      return s;
    }
  };

  /**XML DOM that reads XML from a string. Note that this is a no-frills XML
  parser with no validation and several limitations.

  It currently does not accept CDATA, and comments are not retained. Also,
  comments containing XML markup and comments before the root node will cause
  the parser to fail. Special characters are also not properly substituted
  either.

  Nevertheless, the parser can read most XHTML documents with the above
  limitations in mind. Using a pre-parse filter may also help the parser,
  depending on the application.*/
  class Document
  {
    public:

    ///Stores the root element node.
    Element* Root;

    ///Creates an empty XML document.
    Document() : Root(0) {}

    ///Virtual destructor
    virtual ~Document();

    ///Creates a root element with a given tag name.
    virtual Element* CreateRootElement(String& RootTagName)
    {
      (void)RootTagName;
      return 0;
    }

    protected:

    Parser::Error ParseHeader(const ascii*& Markup, const ascii* MarkupEnd,
      String& RootTagName)
    {
      (void)RootTagName;

      //Local variables to use for parsing.
      Parser::Token t;

      //Remember the original location of the markup start.
      const ascii* m = Markup;

      bool IsParsingHeader = true;
      while(IsParsingHeader)
      {
        //Save the position of the tag beginning for later reference.
        const ascii* TagBeginning = Markup;

        /*At the document level we look for the XML header and the DOCTYPE
        before attempting to parse the root element.*/
        t = Parser::ParseNextWord(Markup, MarkupEnd,
          Parser::Delimiters::WhiteSpace, Parser::Delimiters::TagEntry);

        //Look inside the tag.
        if(t.Text == "<")
        {
          t = Parser::ParseNextWord(Markup, MarkupEnd,
            Parser::Delimiters::WhiteSpace, Parser::Delimiters::TagName);

          if(t.Text == "?")
          {
            /*Header typically looks something like this:
            <?xml version="1.0" encoding="UTF-8"?>. Ignoring for now.*/
            while(t.Text != ">")
            {
              t = Parser::ParseNextWord(Markup, MarkupEnd,
                Parser::Delimiters::WhiteSpace, Parser::Delimiters::TagExit);

              if(t.Text == "")
                return Parser::Error(
                  Parser::Error::Categories::UnmatchedBracket, m);
            }
          }
          else if(t.Text == "!")
          {
            t = Parser::ParseNextWord(Markup, MarkupEnd,
              Parser::Delimiters::WhiteSpace, Parser::Delimiters::TagName);
            if(t.Text == "DOCTYPE")
            {
              //Skip through the DOCTYPE definitions.
              while(t.Text != ">")
              {
                t = Parser::ParseNextWord(Markup, MarkupEnd,
                  Parser::Delimiters::WhiteSpace, Parser::Delimiters::TagExit);

                if(t.Text == "")
                  return Parser::Error(
                    Parser::Error::Categories::UnmatchedBracket, m);
              }
            }
            else
            {
              //Unexpected tag name. Parser failed.
              return Parser::Error(
                Parser::Error::Categories::UnexpectedTagName, m);
            }
          }
          else
          {
            //Now we are inside a tag.
            Markup = TagBeginning;
            IsParsingHeader = false;
          }
        }
        else
        {
          //Presumably an empty document. Parse failed.
          return Parser::Error(Parser::Error::Categories::EmptyDocument, m);
        }
      }

      return Parser::Error();
    }

    /**Method called before writing the root node to the output stream. This
    string should specify the encoding and DOCTYPE if necessary.*/
    virtual void WriteHeader(String& XMLOutput)
    {
      (void)XMLOutput;
    }

    public:

    /**Parses a string as an XML document and returns any error associated with
    the parsing operation. Use the GetDescription() method on the error to
    retrieve the error type.*/
    Parser::Error ParseDocument(const String& MarkupDocument)
    {
      //Get a pointer to the markup string.
      const ascii* Markup = MarkupDocument.Merge();
      const ascii* MarkupEnd = Markup + MarkupDocument.n() + 1;

      String RootTagName;
      Parser::Error Error = ParseHeader(Markup, MarkupEnd, RootTagName);
      if(Error)
      {
        Error.Original = MarkupDocument.Merge();
        Error.UpdateIndex(MarkupEnd);
        return Error;
      }

      //Delete root node if it already exists.
      delete Root;

      //Create the root node.
      Root = CreateRootElement(RootTagName);
      if(not Root)
        Root = new Element;

      //Parse the document tree.
      Error = Root->Parse(Markup, MarkupEnd);
      if(Error)
      {
        Error.Original = MarkupDocument.Merge();
        Error.UpdateIndex(MarkupEnd);
        delete Root;
        Root = 0;
        return Error;
      }

      //Interpret the entire document.
      Root->Interpret();

      return Parser::Error();
    }

    ///Writes the XML out to a string.
    void WriteToString(String& XMLOutput)
    {
      WriteHeader(XMLOutput);
      Root->AppendToString(XMLOutput);
    }

    ///Writes the XML out to a file.
    void WriteToFile(String Filename)
    {
      String XMLOutput;
      WriteToString(XMLOutput);
      File::Write(Filename, XMLOutput);
    }
  };
};}

#ifdef PRIM_COMPILE_INLINE
namespace PRIM_NAMESPACE
{
  //Definitions for delimiters.
  const unicode XML::Parser::Delimiters::None[1] = {0};
  const unicode XML::Parser::Delimiters::WhiteSpace[5] = {32, 9, 10, 13, 0};
  const unicode XML::Parser::Delimiters::TagEntry[2] = {'<', 0};
  const unicode XML::Parser::Delimiters::TagExit[2] = {'>', 0};
  const unicode XML::Parser::Delimiters::TagName[5] =
    {'!', '?', '/', '>', 0};
  const unicode XML::Parser::Delimiters::TagAttributeName[4] =
    {'=','>','/', 0};
  const unicode XML::Parser::Delimiters::TagAttributeValue[4] =
    {'"', '\x27', '>', 0};
  const unicode XML::Parser::Delimiters::TagAttributeValueSingle[3] =
  {'\x27', '>', 0};
  const unicode XML::Parser::Delimiters::TagAttributeValueDouble[3] =
  {'"', '>', 0};

  //Avoid weak-vtable warning by making at least one virtual method out-of-line.
  XML::Object::~Object() {}
  XML::Text::~Text() {}
  XML::Element::~Element() {Objects.RemoveAndDeleteAll();}
  XML::Document::~Document() {delete Root;}

  //This method needs to be outside due to a mutual dependency.
  void XML::Element::AppendToString(String& XMLOutput)
  {
    /*Pretty-printing will enable automatic indentation but always adds
    whitespace. This ought to be moved to where the XML document can control the
    feature.*/
    //const bool PrettyPrint = false;
    static count TabLevel = 0;
    String TabString;
    /*
    if(PrettyPrint)
    {
      for(count i = 0; i < TabLevel; i++)
        TabString << "  ";
      XMLOutput >> TabString;
    }
    */

    //First give the virtual method a chance to update the XML data structure.
    Translate();

    XMLOutput << "<";
    XMLOutput << Name;

    //Write the attributes within the tag.
    for(count i = 0; i < Attributes.n(); i++)
    {
      XMLOutput << " " << Attributes[i].Name;
      XMLOutput << "=\"";
      XMLOutput << Attributes[i].Value;
      XMLOutput << "\"";
    }

    //Write the objects to the element.
    if(Objects.n() > 0)
    {
      XMLOutput << ">";
      bool ContainsTags = false;
      for(count i = 0; i < Objects.n(); i++)
      {
        Object* e = Objects[i];
        Text* t = e->IsText();
        Element* el = e->IsElement();
        if(t)
          XMLOutput << *static_cast<String*>(t);
        else
        {
          ContainsTags = true;
          TabLevel++;
          el->AppendToString(XMLOutput);
          TabLevel--;
        }
      }
      if(ContainsTags/* and PrettyPrint*/)
        XMLOutput >> TabString;
      XMLOutput << "</";
      XMLOutput << Name;
      XMLOutput << ">";
    }
    else
    {
      //Use self-closing syntax: <br/>
      XMLOutput << "/>";
    }
  }
}
#endif
#endif
