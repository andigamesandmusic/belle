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

#ifndef BELLE_ENGRAVER_LABEL_H
#define BELLE_ENGRAVER_LABEL_H

namespace BELLE_NAMESPACE
{
  //Class to store music concepts and custom strings
  class MusicLabel : public Value::Base
  {
    ///Stores the concepts.
    Tree<mica::Concept> Concepts;

    ///Stores the strings.
    Tree<String> Strings;

    private:

    ///Stores information related to the typesetting object.
    mutable Value StateValue;

    public:

    ///Returns a reference to the internal state.
    Value& SetState() const {return StateValue;}

    ///Returns a 1-key state property.
    Value& SetState(const String& a) const {return SetState()[a];}

    ///Returns a 2-key state property.
    Value& SetState(const String& a, const String& b) const
    {
      return SetState()[a][b];
    }

    ///Returns a 3-key state property.
    Value& SetState(const String& a, const String& b, const String& c) const
    {
      return SetState()[a][b][c];
    }

    ///Returns a 4-key state property.
    Value& SetState(const String& a, const String& b, const String& c,
      const String& d) const
    {
      return SetState()[a][b][c][d];
    }

    ///Returns a reference to the internal state.
    Value GetState() const {return StateValue;}

    ///Returns a 1-key state property.
    Value GetState(const String& a) const
    {
      bool KeyExists = false;
      if(StateValue.Contains(Value(a)))
        KeyExists = true;
      return KeyExists ? StateValue[a] : Value();
    }

    ///Returns a 2-key state property.
    Value GetState(const String& a, const String& b) const
    {
      bool KeyExists = false;
      if(StateValue.Contains(Value(a)) and StateValue[a].Contains(Value(b)))
        KeyExists = true;
      return KeyExists ? StateValue[a][b] : Value();
    }

    ///Returns a 3-key state property.
    Value GetState(const String& a, const String& b, const String& c) const
    {
      bool KeyExists = false;
      if(StateValue.Contains(Value(a)) and StateValue[a].Contains(Value(b)) and
        StateValue[a][b].Contains(Value(c)))
          KeyExists = true;
      return KeyExists ? StateValue[a][b][c] : Value();
    }

    ///Returns a 4-key state property.
    Value GetState(const String& a, const String& b, const String& c,
      const String& d) const
    {
      bool KeyExists = false;
      if(StateValue.Contains(Value(a)) and StateValue[a].Contains(Value(b)) and
        StateValue[a][b].Contains(Value(c)) and
        StateValue[a][b][c].Contains(Value(d)))
          KeyExists = true;
      return KeyExists ? StateValue[a][b][c][d] : Value();
    }

    ///Clears the information in the internal state.
    void ClearState() const
    {
      StateValue.Clear();
    }

    ///Returns a value reference to the stamp on this object.
    Value& Stamp() const
    {
      return SetState("Stamp");
    }

    private:

    ///Converts a string like "Foo Bar" to "FooBar" (no case check though).
    static String ToCamelCase(String s)
    {
      s.Replace(" ", "");
      return s;
    }

    /**Converts a string like "FooBar" to "Foo Bar" (any sequence of lowercase
    followed by uppercase has a space inserted between).*/
    static String ToSpaceSeparated(String s)
    {
      for(count i = 0; i < s.n() - 1; i++)
      {
        ascii a = ascii(s[i]);
        ascii b = ascii(s[i + 1]);
        if((a >= 'a' and a <= 'z') and (b >= 'A' and b <= 'Z'))
          s.Insert(" ", i++ + 1);
      }
      return s;
    }

    public:

    ///Const key-value lookup
    mica::Concept Get(const mica::Concept& Key) const {return Concepts[Key];}

    ///Mutable key-value lookup
    mica::Concept& Set(const mica::Concept& Key) {return Concepts[Key];}

    ///Const key-value lookup
    String Get(const ascii* Key) const {return Strings[Key];}

    ///Mutable key-value lookup
    String& Set(const ascii* Key) {return Strings[Key];}

    ///Attribute set for XML deserialization.
    void Set(const ascii* Key, const ascii* Value)
    {
      String k = Key, v = Value;
      if(k.StartsWith("data-"))
      {
        k.Erase(0, 4);
        Strings[k] = v;
      }
      else
        Concepts[mica::Concept(ToSpaceSeparated(k).Merge())] =
          mica::Concept(Value);
    }

    ///Removes concept key-value by key.
    void Remove(const mica::Concept& Key) {return Concepts.Remove(Key);}

    ///Removes string key-value by key.
    void Remove(const String& Key) {return Strings.Remove(Key);}

    ///Returns keys for string serialization.
    Array<String> AttributeKeysAsStrings() const
    {
      Array<mica::Concept> ConceptKeys;
      Array<String> StringKeys;
      Concepts.Keys(ConceptKeys);
      Strings.Keys(StringKeys);
      Array<String> Keys(Concepts.n() + Strings.n());
      for(count i = 0; i < Keys.n(); i++)
      {
        if(i < Concepts.n())
          Keys[i] = ToCamelCase(ConceptKeys[i]);
        else
        {
          String s = "data-";
          s << StringKeys[i - Concepts.n()];
          Keys[i] = s;
        }
      }
      return Keys;
    }

    ///Returns values for string serialization.
    Array<String> AttributeValuesAsStrings() const
    {
      Array<mica::Concept> ConceptValues;
      Array<String> StringValues;
      Concepts.Values(ConceptValues);
      Strings.Values(StringValues);
      Array<String> Values(Concepts.n() + Strings.n());
      for(count i = 0; i < Values.n(); i++)
      {
        if(i < Concepts.n())
          Values[i] = ConceptValues[i];
        else
          Values[i] = StringValues[i - Concepts.n()];
      }
      return Values;
    }

    ///For equivalence, the label is only checked against the items in filter.
    bool EdgeEquivalent(const MusicLabel& Filter) const
    {
      {
        Tree<mica::Concept>::Iterator C;
        for(C.Begin(Filter.Concepts); C.Iterating(); C.Next())
          if(Concepts.Get(C.Key()) != C.Value())
            return false;
      }
      {
        Tree<String>::Iterator S;
        for(S.Begin(Filter.Strings); S.Iterating(); S.Next())
          if(Strings.Get(S.Key()) != S.Value())
            return false;
      }
      return true;
    }

    ///String conversion
    operator String () const
    {
      String s;
      Array<String> Keys = AttributeKeysAsStrings();
      Array<String> Values = AttributeValuesAsStrings();
      for(count i = 0; i < Keys.n(); i++)
      {
        if(i) s << " ";
        s << Keys[i] << ":" << Values[i];
      }

      Array<Value> StateValueKeys;
      StateValue.EnumerateKeys(StateValueKeys);
      for(count i = 0, n = StateValueKeys.n(); i < n; i++)
      {
        Value k = StateValueKeys[i];
        s << " ";
        if(k.IsInteger() or k.IsNumber() or k.IsString() or k.IsRatio())
          s << k;
        else
          s << "...";
        s << ":";
        const Value& v = StateValue[k];
        if(v.IsInteger() or v.IsNumber() or v.IsString() or v.IsRatio())
          s << v;
        else
          s << "...";
      }
      return s;
    }

    ///Returns the name of this object.
    String Name() const
    {
      return "MusicLabel";
    }

    //-------------------//
    //Equivalence Testing//
    //-------------------//

    ///Checks to see if the music labels are equivalent.
    bool operator == (const MusicLabel& Other) const
    {
      return Concepts == Other.Concepts and Strings == Other.Strings;
    }

    ///Checks to see if the music labels are not equivalent.
    bool operator != (const MusicLabel& Other) const
    {
      return !(*this == Other);
    }

    //------------//
    //Constructors//
    //------------//

    ///Creates an empty label.
    MusicLabel() {}

    ///Creates other label.
    MusicLabel(const MusicLabel& Other) : Value::Base() {*this = Other;}

    ///Assignment operator.
    MusicLabel& operator = (const MusicLabel& Other)
    {
      Concepts = Other.Concepts;
      Strings = Other.Strings;
      StateValue = Other.StateValue;
      return *this;
    }

    ///Creates a label with a given type.
    MusicLabel(mica::Concept LabelType)
    {
      Set(mica::Type) = LabelType;
    }

    ///Creates a label with a given key and value.
    MusicLabel(mica::Concept Key, mica::Concept Value)
    {
      Set(Key) = Value;
    }

    ///Returns the properties of the label as a value.
    Value Properties() const
    {
      Array<mica::Concept> ConceptKeys;
      Array<mica::Concept> ConceptValues;
      Array<String> StringKeys;
      Array<String> StringValues;

      Concepts.Keys(ConceptKeys);
      Strings.Keys(StringKeys);
      Concepts.Values(ConceptValues);
      Strings.Values(StringValues);

      Value Result;
      for(count i = 0; i < ConceptKeys.n(); i++)
        Result[ConceptKeys[i]] = ConceptValues[i];
      for(count i = 0; i < StringKeys.n(); i++)
        Result[StringKeys[i]] = StringValues[i];
      return Result;
    }

    ///Virtual destructor
    virtual ~MusicLabel();
  };
}
#endif
