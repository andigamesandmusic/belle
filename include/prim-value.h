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

#ifndef PRIM_INCLUDE_VALUE_H
#define PRIM_INCLUDE_VALUE_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  /**Variant value. The value can be of type nil, bool, integer, number,
  Ratio, Vector, Box, String, Array of type Value, or a key-value tree
  of type Value. Notably, since the tree type stores Value typed key-values,
  this enables the value to store trees of information. The Value can also take
  a Pointer<Value::Base> or Pointer<const Value::Base> as a key or value. This
  allows for storing arbitrary pointers to objects that inherit from the
  polymorphic Value::Base class. However, note that Value does not store the
  original constness of the incoming pointer.*/
  class Value
  {
    public:

    /**Generic object handle. A Value can be assigned to store pointers to
    objects that derive from this class.*/
    class Base
    {
      public:

      ///Empty constructor.
      Base() {}

      ///Empty copy constructor.
      Base(const Base& Other) {*this = Other;}

      ///Empty assignment operator.
      Base& operator = (const Base& Other) {(void)Other; return *this;}

      ///Virtual destructor
      virtual ~Base();

      ///Overloadable string operator
      virtual operator String() const
      {
        String s;
        s << "<Value::Base " << reinterpret_cast<const void*>(this) << ">";
        return s;
      }

      ///Overloadable object name
      virtual String Name() const
      {
        return "Value::Base";
      }
    };

    ///Const reference to another value.
    class ConstReference : public PRIM_NAMESPACE::Value::Base
    {
      ///Stores the const reference.
      Pointer<const Value> ValueReference;

      public:

      ///Creates a const reference to another value.
      ConstReference(Pointer<const Value> v) : ValueReference(v) {}

      ///Returns the const reference.
      operator String() const
      {
        String s;
        s << "<Value::ConstReference " << ValueReference << ">";
        return s;
      }

      ///Returns the type name of the const reference.
      String Name() const
      {
        return "Value::ConstReference";
      }

      ///Gets the const reference to the assigned value.
      const Value& Get() const
      {
        return *ValueReference;
      }

      ///Gets a const reference pointer to the assigned value.
      Pointer<const Value> GetPointer() const
      {
        return ValueReference;
      }

      ///Virtual destructor
      virtual ~ConstReference();
    };

    private:

    ///Hard-coded value types of size 64 bits.
    enum ValueTypes
    {
      ValueTypeNil,       //Stored directly
      ValueTypeBoolean,   // .
      ValueTypeInteger,   // .
      ValueTypeNumber,    // .
      ValueTypeRatio,     //Stored via pointer to object
      ValueTypeVector,    // .
      ValueTypeBox, // .
      ValueTypeString,    // .
      ValueTypeArray,     // .
      ValueTypeTree,      // .
      ValueTypeObject     // .
    };

    public:

    //Alias for the array of value type
    typedef Array<Value> ArrayType;

    //Alias for the key-value tree type
    typedef Tree<Value, Value> TreeType;

    private:

    //Alias for the object type
    typedef Pointer<Base> ObjectType;

    //Alias for the const object type
    typedef Pointer<const Base> ConstObjectType;

    ///Field 1: Stores data or pointer to data.
    union
    {
      bool    DataBooleanValue;
      integer DataIntegerValue;
      number  DataNumberValue;
      void*   DataPointer;
      int64   DataField1; //Used for padding and clearing.
    };

    ///Field 2: Stores the value type according to a value type constant.
    union
    {
      ValueTypes ValueType;
      int64      DataField2; //Used for padding and clearing.
    };

    //----------------------//
    //Internal Data Handling//
    //----------------------//

    ///Clears the fields.
    void InternalClear()
    {
      DataField1 = DataField2 = 0;
    }

    ///Statically casts the data pointer to the requested type.
    template <class T> T* InternalCastTo()
    {
      return reinterpret_cast<T*>(DataPointer);
    }

    ///Statically const casts the data pointer to the requested type.
    template <class T> const T* InternalCastTo() const
    {
      return reinterpret_cast<const T*>(DataPointer);
    }

    ///Deallocates heap memory used for the value.
    void InternalDeallocate()
    {
      switch(ValueType)
      {
      case ValueTypeRatio:
        delete InternalCastTo<Ratio>();
        break;
      case ValueTypeVector:
        delete InternalCastTo<Vector>();
        break;
      case ValueTypeBox:
        delete InternalCastTo<Box>();
        break;
      case ValueTypeString:
        delete InternalCastTo<String>();
        break;
      case ValueTypeArray:
        delete InternalCastTo<ArrayType>();
        break;
      case ValueTypeTree:
        delete InternalCastTo<TreeType>();
        break;
      case ValueTypeObject:
        delete InternalCastTo<ObjectType>();
        break;
      case ValueTypeNil:
      case ValueTypeBoolean:
      case ValueTypeInteger:
      case ValueTypeNumber:
        break;
      }

      //Clear the fields.
      InternalClear();
    }

    ///Allocates heap memory and constructs a value.
    void InternalAllocate()
    {
      switch(ValueType)
      {
      case ValueTypeRatio:
        DataPointer = reinterpret_cast<void*>(new Ratio);
        break;
      case ValueTypeVector:
        DataPointer = reinterpret_cast<void*>(new Vector);
        break;
      case ValueTypeBox:
        DataPointer = reinterpret_cast<void*>(new Box);
        break;
      case ValueTypeString:
        DataPointer = reinterpret_cast<void*>(new String);
        break;
      case ValueTypeArray:
        DataPointer = reinterpret_cast<void*>(new ArrayType);
        break;
      case ValueTypeTree:
        DataPointer = reinterpret_cast<void*>(new TreeType);
        break;
      case ValueTypeObject:
        DataPointer = reinterpret_cast<void*>(new ObjectType);
        break;
      case ValueTypeNil:
      case ValueTypeBoolean:
      case ValueTypeInteger:
      case ValueTypeNumber:
        break;
      }
    }

    ///Coerces empty values from other types to nil.
    void InternalCoerceToNil()
    {
      switch(ValueType)
      {
      case ValueTypeInteger:
        if(AsInteger() == Nothing<integer>())
          Clear();
        break;
      case ValueTypeNumber:
        if(Limits<number>::IsNaN(AsNumber()))
          Clear();
        break;
      case ValueTypeRatio:
        if(AsRatio().IsEmpty())
          Clear();
        break;
      case ValueTypeVector:
        if(AsVector().IsEmpty())
          Clear();
        break;
      case ValueTypeBox:
        if(AsBox().IsEmpty())
          Clear();
        break;
      case ValueTypeObject:
        if(not AssumeAndGet<ObjectType>())
          Clear();
        break;
      case ValueTypeNil:
      case ValueTypeBoolean:
      case ValueTypeString:
      case ValueTypeArray:
      case ValueTypeTree:
        break;
      }
    }

    //-----------------//
    //Value Referencing//
    //-----------------//

    ///Gets a reference to a value of a specific type reallocating if necessary.
    template <class T, ValueTypes ValueTypeT> T& Get()
    {
      if(ValueType != ValueTypeT)
      {
        InternalDeallocate();
        ValueType = ValueTypeT;
        InternalAllocate();
      }
      return *InternalCastTo<T>();
    }

    /**Gets a reference to a value of assumed type. If the value is not of the
    given type, then the behavior is undefined.*/
    template <class T> const T& AssumeAndGet() const
    {
      return *InternalCastTo<T>();
    }

    /**Gets a const reference to value of assumed type boolean. If the value is
    not of the given type, then the behavior is undefined.*/
    const bool& AssumeAndGetBoolean() const {return DataBooleanValue;}

    /**Gets a const reference to value of assumed type integer. If the value is
    not of the given type, then the behavior is undefined.*/
    const integer& AssumeAndGetInteger() const {return DataIntegerValue;}

    /**Gets a const reference to value of assumed type number. If the value is
    not of the given type, then the behavior is undefined.*/
    const number& AssumeAndGetNumber() const {return DataNumberValue;}

    ///Gets a reference to a boolean reallocating if necessary.
    bool& GetBoolean()
    {
      if(ValueType != ValueTypeBoolean)
      {
        InternalDeallocate();
        ValueType = ValueTypeBoolean;
        DataBooleanValue = false;
      }
      return DataBooleanValue;
    }

    ///Gets a reference to an integer reallocating if necessary.
    integer& GetInteger()
    {
      if(ValueType != ValueTypeInteger)
      {
        InternalDeallocate();
        ValueType = ValueTypeInteger;
        DataIntegerValue = 0;
      }
      return DataIntegerValue;
    }

    ///Gets a reference to a number reallocating if necessary.
    number& GetNumber()
    {
      if(ValueType != ValueTypeNumber)
      {
        InternalDeallocate();
        ValueType = ValueTypeNumber;
        DataNumberValue = 0.f;
      }
      return DataNumberValue;
    }

    //-------------//
    //Type Coercion//
    //-------------//

    public:

    ///Returns a const-protected reference to the current value.
    const Value& Const() {const Value& x = *this; return x;}

    ///Returns the global nil value.
    static const Value& Empty()
    {
      return ZeroInitialized<Value>::Object();
    }

    ///Coerces the value to a boolean.
    bool AsBoolean() const
    {
      switch(ValueType)
      {
      case ValueTypeBoolean:
        return DataBooleanValue;
      case ValueTypeInteger:
        return bool(DataIntegerValue);
      case ValueTypeNumber:
        return bool(DataNumberValue);
      case ValueTypeRatio:
        return !AssumeAndGet<Ratio>().IsEmpty();
      case ValueTypeVector:
        return !AssumeAndGet<Vector>().IsEmpty();
      case ValueTypeBox:
        return !AssumeAndGet<Box>().IsEmpty();
      case ValueTypeString:
        return bool(AssumeAndGet<String>());
      case ValueTypeArray:
        return bool(AssumeAndGet<ArrayType>().n());
      case ValueTypeTree:
        return !AssumeAndGet<TreeType>().Empty();
      case ValueTypeObject:
        return bool(AssumeAndGet<ObjectType>());
      case ValueTypeNil:
        break;
      }
      return false;
    }

    ///Coerces the value to an integer.
    integer AsInteger() const
    {
      switch(ValueType)
      {
      case ValueTypeBoolean:
        return DataBooleanValue ? 1 : 0;
      case ValueTypeInteger:
        return DataIntegerValue;
      case ValueTypeNumber:
        return integer(DataNumberValue);
      case ValueTypeRatio:
        return AssumeAndGet<Ratio>().To<integer>();
      case ValueTypeString:
        return integer(AssumeAndGet<String>().ToNumber());
      case ValueTypeArray:
        return integer(AssumeAndGet<ArrayType>().n());
      case ValueTypeTree:
        return integer(AssumeAndGet<TreeType>().n());
      case ValueTypeNil:
      case ValueTypeVector:
      case ValueTypeBox:
      case ValueTypeObject:
        break;
      }
      return Nothing<integer>();
    }

    ///Coerces the value to a count.
    count AsCount() const
    {
      return IsNil() ? Nothing<count>() : count(AsInteger());
    }

    ///Coerces the value to a number.
    number AsNumber() const
    {
      switch(ValueType)
      {
      case ValueTypeBoolean:
        return DataBooleanValue ? 1.f : 0.f;
      case ValueTypeInteger:
        return number(DataIntegerValue);
      case ValueTypeNumber:
        return DataNumberValue;
      case ValueTypeRatio:
        return AssumeAndGet<Ratio>().To<number>();
      case ValueTypeVector:
        return number(AssumeAndGet<Vector>().Mag());
      case ValueTypeString:
        return number(AssumeAndGet<String>().ToNumber());
      case ValueTypeNil:
      case ValueTypeBox:
      case ValueTypeArray:
      case ValueTypeTree:
      case ValueTypeObject:
        break;
      }
      return Nothing<number>();
    }

    ///Coerces the value to a ratio.
    Ratio AsRatio() const
    {
      switch(ValueType)
      {
      case ValueTypeBoolean:
        return DataBooleanValue ? Ratio(1) : Ratio(0);
      case ValueTypeInteger:
        return Ratio(DataIntegerValue);
      case ValueTypeNumber:
        return Ratio(DataNumberValue, 1000, false);
      case ValueTypeRatio:
        return AssumeAndGet<Ratio>();
      case ValueTypeString:
        return Ratio(AssumeAndGet<String>());
      case ValueTypeArray:
        return Ratio(AssumeAndGet<ArrayType>().n());
      case ValueTypeTree:
        return Ratio(AssumeAndGet<TreeType>().n());
      case ValueTypeNil:
      case ValueTypeVector:
      case ValueTypeBox:
      case ValueTypeObject:
        break;
      }
      return Ratio();
    }

    ///Coerces the value to a vector.
    Vector AsVector() const
    {
      switch(ValueType)
      {
      case ValueTypeBoolean:
        return Vector((DataBooleanValue ? 1. : 0.), 0.);
      case ValueTypeInteger:
        return Vector(number(DataIntegerValue), 0.);
      case ValueTypeNumber:
        return Vector(number(DataNumberValue), 0.);
      case ValueTypeRatio:
        return Vector(AssumeAndGet<Ratio>().To<number>(), 0.);
      case ValueTypeVector:
        return AssumeAndGet<Vector>();
      case ValueTypeBox:
        return AssumeAndGet<Box>().Size();
      case ValueTypeNil:
      case ValueTypeString:
      case ValueTypeArray:
      case ValueTypeTree:
      case ValueTypeObject:
        break;
      }
      return Vector::Empty();
    }

    ///Coerces the value to a rectangle.
    Box AsBox() const
    {
      if(ValueType == ValueTypeBox)
        return AssumeAndGet<Box>();
      else if(ValueType == ValueTypeVector)
        return Box(Vector(),
          AssumeAndGet<Vector>());
      return Box();
    }

    ///Coerces the value to a string.
    String AsString() const
    {
      switch(ValueType)
      {
      case ValueTypeNil:
        break;
      case ValueTypeBoolean:
        return String(DataBooleanValue);
      case ValueTypeInteger:
        return String(DataIntegerValue);
      case ValueTypeNumber:
        return String(DataNumberValue);
      case ValueTypeRatio:
        return String(AssumeAndGet<Ratio>());
      case ValueTypeVector:
        return String(AssumeAndGet<Vector>());
      case ValueTypeBox:
        return String(AssumeAndGet<Box>());
      case ValueTypeString:
        return AssumeAndGet<String>();
      case ValueTypeArray:
        return String(AssumeAndGet<ArrayType>());
      case ValueTypeTree:
        return String(AssumeAndGet<TreeType>());
      case ValueTypeObject:
      {
        ObjectType O = AssumeAndGet<ObjectType>();
        if(not O)
          return "<Object 0x0>";
        else
          return String(*O) + " (" + String(O.Raw()) + ")";
      }
      }
      return "Nil";
    }

    ///Coerces the value to an array.
    ArrayType AsArray() const
    {
      if(ValueType == ValueTypeArray)
        return AssumeAndGet<ArrayType>();
      return ArrayType();
    }

    ///Coerces the value to a tree.
    TreeType AsTree() const
    {
      if(ValueType == ValueTypeTree)
        return AssumeAndGet<TreeType>();
      return TreeType();
    }

    public:

    //----------------//
    //Object Lifecycle//
    //----------------//

    ///Creates a nil value.
    Value()
    {
      InternalClear();
    }

    ///Releases the value.
    ~Value()
    {
      InternalDeallocate();
    }

    //------------//
    //Manipulation//
    //------------//

    ///Sets the value to nil.
    void Clear()
    {
      InternalDeallocate();
    }

    //----------------//
    //Type Information//
    //----------------//

    ///Returns whether the value is nil.
    bool IsNil() const {return ValueType == ValueTypeNil;}

    ///Returns whether the value is a boolean.
    bool IsBoolean() const {return ValueType == ValueTypeBoolean;}

    ///Returns whether the value is an integer.
    bool IsInteger() const {return ValueType == ValueTypeInteger;}

    ///Returns whether the value is a number.
    bool IsNumber() const {return ValueType == ValueTypeNumber;}

    ///Returns whether the value is a ratio.
    bool IsRatio() const {return ValueType == ValueTypeRatio;}

    ///Returns whether the value is a vector.
    bool IsVector() const {return ValueType == ValueTypeVector;}

    ///Returns whether the value is a rectangle.
    bool IsBox() const {return ValueType == ValueTypeBox;}

    ///Returns whether the value is a string.
    bool IsString() const {return ValueType == ValueTypeString;}

    ///Returns whether the value is an array.
    bool IsArray() const {return ValueType == ValueTypeArray;}

    ///Returns whether the value is a tree.
    bool IsTree() const {return ValueType == ValueTypeTree;}

    ///Returns whether the value is an object.
    bool IsObject() const {return ValueType == ValueTypeObject;}

    //--------//
    //Indexing//
    //--------//

    ///Returns the number of elements if an array or tree and zero otherwise.
    count n() const
    {
      if(ValueType == ValueTypeArray)
        return AssumeAndGet<ArrayType>().n();
      else if(ValueType == ValueTypeTree)
        return AssumeAndGet<TreeType>().n();
      return 0;
    }

    /**Sets the number of elements in an array. If not already an array, then
    a new array is created with the number elements given.*/
    void n(count Elements)
    {
      if(ValueType != ValueTypeArray)
        Clear();
      Get<ArrayType, ValueTypeArray>().n(Elements);
    }

    ///Enumerates all the keys in the value.
    void EnumerateKeys(Array<Value>& Keys) const
    {
      Keys.Clear();
      if(ValueType == ValueTypeArray)
      {
        Keys.n(n());
        for(count i = 0; i < n(); i++)
          Keys[i] = i;
      }
      else if(ValueType == ValueTypeTree)
        AssumeAndGet<TreeType>().Keys(Keys);
    }

    ///Enumerates all the keys in the value.
    Value Keys() const
    {
      Array<Value> KeyArray;
      EnumerateKeys(KeyArray);
      Value Keys;
      for(count i = 0; i < KeyArray.n(); i++)
        Keys[i] = KeyArray[i];
      return Keys;
    }

    //----------//
    //Comparison//
    //----------//

    ///Indicates whether this value and another can be compared numerically.
    bool NumericComparison(const Value& Other) const
    {
      /*The following assumes that the enumeration order is: integer, number,
      ratio.*/
      return ValueType >= ValueTypeInteger and
             ValueType <= ValueTypeRatio   and
       Other.ValueType >= ValueTypeInteger and
       Other.ValueType <= ValueTypeRatio;
    }

    public:

    /**Returns whether the value is semantically less than another. This is done
    by first comparing via the type-ids of the values. If they are both numeric,
    then a numeric comparison is performed, otherwise the result is the
    comparison of the type-id values. If the types are the same (and not
    numeric), then the following rules apply: nil is equal and not less than
    nil, false is less than true, the generic string comparison operator is used
    for strings, and finally, the compound data types vector, rectangle, and
    tree are compared by converting to string and comparing using the string
    comparator.*/
    bool operator < (const Value& Other) const
    {
      if(ValueType == ValueTypeNil)
        return Other.ValueType != ValueTypeNil;
      else if(NumericComparison(Other))
        return AsNumber() < Other.AsNumber();

      if(ValueType < Other.ValueType)
        return true;
      else if(ValueType > Other.ValueType)
        return false;

      //Now implied: ValueType == Other.ValueType

      if(ValueType == ValueTypeBoolean)
        return AsInteger() < Other.AsInteger();
      else if(ValueType == ValueTypeString)
        return AssumeAndGet<String>() < Other.AssumeAndGet<String>();
      else if(ValueType == ValueTypeObject)
        return AssumeAndGet<ObjectType>() < Other.AssumeAndGet<ObjectType>();

      /*There is no general way to value-compare vectors, rectangles, and
      trees, so convert them to strings and compare that way.*/
      return AsString() < Other.AsString();
    }

    /**Compares whether two values are equal. If both values are numeric, then
    a numeric comparison is performed. Otherwise, if the types are different the
    values are not equal, and if the types are the same, the following rules
    apply: nil equals nil, all other types use their respective equality tests
    for comparison testing.*/
    bool operator == (const Value& Other) const
    {
      //Compare by number if possible.
      if(NumericComparison(Other))
      {
        //Compare exactly if possible. Otherwise compare floating-point.
        if(ValueType != ValueTypeNumber and Other.ValueType != ValueTypeNumber)
          return AsRatio() == Other.AsRatio();
        else
          return Limits<number>::IsEqual(AsNumber(), Other.AsNumber());
      }

      //Non-numeric unlike things can not be equal.
      if(ValueType != Other.ValueType)
        return false;

      //Now implied: ValueType == Other.ValueType
      switch(ValueType)
      {
      case ValueTypeNil:
        return true;
      case ValueTypeBoolean:
        return DataBooleanValue == Other.DataBooleanValue;
      case ValueTypeVector:
        return AssumeAndGet<Vector>() ==
          Other.AssumeAndGet<Vector>();
      case ValueTypeBox:
        return AssumeAndGet<Box>() ==
          Other.AssumeAndGet<Box>();
      case ValueTypeString:
        return AssumeAndGet<String>() == Other.AssumeAndGet<String>();
      case ValueTypeArray:
        return AssumeAndGet<ArrayType>() == Other.AssumeAndGet<ArrayType>();
      case ValueTypeTree:
        return AssumeAndGet<TreeType>() == Other.AssumeAndGet<TreeType>();
      case ValueTypeObject:
        return AssumeAndGet<ObjectType>() == Other.AssumeAndGet<ObjectType>();
      case ValueTypeInteger:
      case ValueTypeNumber:
      case ValueTypeRatio:
        break;
      }
      return false;
    }

    ///Returns whether the value is not equal to another value.
    bool operator != (const Value& Other) const {return !((*this) == Other);}

    ///Returns whether the value is greater than another value.
    bool operator >  (const Value& Other) const {return Other < (*this);}

    ///Returns whether the value is less than or equal to another value.
    bool operator <= (const Value& Other) const {return !(Other < (*this));}

    ///Returns whether the value is greater than or equal to another value.
    bool operator >= (const Value& Other) const {return !((*this) < Other);}

    public:

    //-----------------//
    //Casting Operators//
    //-----------------//

    ///Casts the value to a bool.
    operator bool () const {return AsBoolean();}

    ///Casts the value to an integer.
    operator integer () const {return integer(AsInteger());}

    ///Casts the value to a number.
    operator number () const {return number(AsNumber());}

    ///Casts the value to a ratio.
    operator Ratio () const {return AsRatio();}

    ///Casts the value to a vector.
    operator Vector () const {return AsVector();}

    ///Casts the value to a rectangle.
    operator Box () const {return AsBox();}

    ///Casts the value to a string.
    operator String () const {return AsString();}

    ///Returns a pointer to the const object.
    ConstObjectType Object() const
    {
      if(ValueType == ValueTypeObject)
        return AssumeAndGet<ConstObjectType>();
      return ConstObjectType();
    }

    ///Returns a pointer to the const object.
    ConstObjectType ConstObject() const
    {
      return Object();
    }

    ///Returns a pointer to the object.
    ObjectType Object()
    {
      if(ValueType == ValueTypeObject)
        return AssumeAndGet<ObjectType>();
      return ObjectType();
    }

    /**Unary minus operator returns the negated numeric value. Can be used to
    explicitly request the negated numeric value without using -x.AsNumber().*/
    number operator - () const
    {
      return -AsNumber();
    }

    /**Unary plus operator returns the numeric value. Can be used to explicitly
    request the numeric value without using AsNumber().*/
    number operator + () const
    {
      return AsNumber();
    }

    //-----------------//
    //Copy Constructors//
    //-----------------//

    ///Copy constructor
    Value(const Value& Other)
    {
      InternalClear();
      AssumeDifferentDeepCopy(Other, *this);
    }

    ///Constructs the value using a bool.
    explicit Value(bool x) {InternalClear(); GetBoolean() = x;}

    ///Constructs the value using a uint8.
    explicit Value(uint8 x) {InternalClear(); GetInteger() = integer(x);}

    ///Constructs the value using a int8.
    explicit Value(int8 x) {InternalClear(); GetInteger() = integer(x);}

    ///Constructs the value using a uint16.
    explicit Value(uint16 x) {InternalClear(); GetInteger() = integer(x);}

    ///Constructs the value using a int16.
    explicit Value(int16 x) {InternalClear(); GetInteger() = integer(x);}

    ///Constructs the value using a uint32.
    explicit Value(uint32 x) {InternalClear(); GetInteger() = integer(x);}

    ///Constructs the value using a int32.
    explicit Value(int32 x) {InternalClear(); GetInteger() = integer(x);}

    ///Constructs the value using a uint64.
    explicit Value(uint64 x) {InternalClear(); GetInteger() = integer(x);}

    ///Constructs the value using a int64.
    explicit Value(int64 x)
      {InternalClear(); GetInteger() = integer(x); InternalCoerceToNil();}

    ///Constructs the value using a float32.
    explicit Value(float32 x)
      {InternalClear(); GetNumber() = number(x); InternalCoerceToNil();}

    ///Constructs the value using a float64.
    explicit Value(float64 x)
      {InternalClear(); GetNumber() = number(x); InternalCoerceToNil();}

    ///Constructs the value using a float80.
    explicit Value(float80 x)
      {InternalClear(); GetNumber() = number(x); InternalCoerceToNil();}

    ///Constructs the value using a ratio.
    explicit Value(Ratio x) {InternalClear();
      Get<Ratio, ValueTypeRatio>() = x; InternalCoerceToNil();}

    ///Constructs the value using a string.
    explicit Value(const String& x) {InternalClear();
      Get<String, ValueTypeString>() = x;}

    ///Constructs the value using a constant string.
    explicit Value(const ascii* x) {InternalClear();
      Get<String, ValueTypeString>() = x;}

    ///Constructs the value using a vector.
    explicit Value(Vector x) {InternalClear();
      Get<Vector, ValueTypeVector>() = x; InternalCoerceToNil();}

    ///Constructs the value using a rectangle.
    explicit Value(Box x) {InternalClear();
      Get<Box, ValueTypeBox>() = x; InternalCoerceToNil();}

    ///Constructs the value using an object.
    template <class T>
    explicit Value(Pointer<T> x) {InternalClear();
      Get<ObjectType, ValueTypeObject>() = x; InternalCoerceToNil();}

    ///Constructs the value using an object.
    template <class T>
    explicit Value(Pointer<const T> x) {InternalClear();
      Get<ConstObjectType, ValueTypeObject>() = x; InternalCoerceToNil();}

    private:

    /**Deep copies under the assumption that the copy value reference is not the
    original or contained by original.*/
    static void AssumeDifferentDeepCopy(const Value& Original, Value& Copy)
    {
      switch(Original.ValueType)
      {
      case ValueTypeNil:
        Copy.Clear();
        break;
      case ValueTypeBoolean:
        Copy.GetBoolean() = Original.DataBooleanValue;
        break;
      case ValueTypeInteger:
        Copy.GetInteger() = Original.DataIntegerValue;
        break;
      case ValueTypeNumber:
        Copy.GetNumber() = Original.DataNumberValue;
        break;
      case ValueTypeRatio:
        Copy.Get<Ratio, ValueTypeRatio>() =
          Original.AssumeAndGet<Ratio>();
        break;
      case ValueTypeVector:
        Copy.Get<Vector, ValueTypeVector>() =
          Original.AssumeAndGet<Vector>();
        break;
      case ValueTypeBox:
        Copy.Get<Box, ValueTypeBox>() =
          Original.AssumeAndGet<Box>();
        break;
      case ValueTypeString:
        Copy.Get<String, ValueTypeString>() =
          Original.AssumeAndGet<String>();
        break;
      case ValueTypeArray:
        {
          const ArrayType& OriginalArray = Original.AssumeAndGet<ArrayType>();
          ArrayType& CopyArray = Copy.Get<ArrayType, ValueTypeArray>();
          CopyArray.n(OriginalArray.n());
          for(count i = 0, n = OriginalArray.n(); i < n; i++)
            AssumeDifferentDeepCopy(OriginalArray[i], CopyArray[i]);
        }
        break;
      case ValueTypeTree:
        {
          /*Since key-values are about to be appended into an indefinite space,
          make sure that any existing tree is cleared beforehand so that old
          keys do not remain in the deep copy.*/
          Copy.Clear();
          Copy.Get<TreeType, ValueTypeTree>() =
            Original.AssumeAndGet<TreeType>();
        }
        break;
      case ValueTypeObject:
        Copy.Get<ObjectType, ValueTypeObject>() =
          Original.AssumeAndGet<ObjectType>();
        break;
      }
    }

  public:

    //--------------------//
    //Assignment Operators//
    //--------------------//

    Value& operator = (const Value& Other)
    {
      //First test this == Other to rule out trivial self-assignment.
      if(this == &Other)
      {
        //Skip the copy completely.
      }
      else if(ValueType == ValueTypeArray or ValueType == ValueTypeTree or
        Other.ValueType == ValueTypeArray or Other.ValueType == ValueTypeTree)
      {
        /* #screwcase : a = a, a[0] = a, a[a] = a, and so on...
        Self-assignment and especially partial self-assignment cause many
        headaches. In order to rule out all the types of self-assignment that
        may occur, a temporary deep-copy is made and the final deep-copy
        proceeds from that. Although this doubles the time to make a copy, it is
        guaranteed to be safe.

                                      .-----.
                                     /  ._.  \
                                    |  /   \  |
                                    |  |    | |
                                    |  |    | |
                                   /    \  / /
                                  /      \/ /
                                 /       /`/
                                /       / /\
                                |     .`-'  |
                                |    .'.'   |
                                |    : :    |
                                |  ..: :..  |
                                |.' .' `. '.|
                                \ .'     `. /
                                 \_________/

        Need to do a full deep-copy to temporary if any containers are involved
        to isolate the copy operation.*/
        Value DeepCopyOther;
        AssumeDifferentDeepCopy(Other, DeepCopyOther);
        AssumeDifferentDeepCopy(DeepCopyOther, *this);
      }
      else
      {
        /*Non-containers that have passed the trivial self-assignment test are
        safe to copy under the assumption of this != Other.*/
        AssumeDifferentDeepCopy(Other, *this);
      }

      return *this;
    }

    ///Assigns the value to a bool.
    Value& operator = (bool x)
      {GetBoolean() = x; return *this;}

    ///Assigns the value to a uint8.
    Value& operator = (uint8 x)
      {GetInteger() = integer(x); return *this;}

    ///Assigns the value to a int8.
    Value& operator = (int8 x)
      {GetInteger() = integer(x); return *this;}

    ///Assigns the value to a uint16.
    Value& operator = (uint16 x)
      {GetInteger() = integer(x); return *this;}

    ///Assigns the value to a int16.
    Value& operator = (int16 x)
      {GetInteger() = integer(x); return *this;}

    ///Assigns the value to a uint32.
    Value& operator = (uint32 x)
      {GetInteger() = integer(x); return *this;}

    ///Assigns the value to a int32.
    Value& operator = (int32 x)
      {GetInteger() = integer(x); return *this;}

    ///Assigns the value to a uint64.
    Value& operator = (uint64 x)
      {GetInteger() = integer(x); return *this;}

    ///Assigns the value to a int64.
    Value& operator = (int64 x)
      {GetInteger() = integer(x); InternalCoerceToNil(); return *this;}

    ///Assigns the value to a float32.
    Value& operator = (float32 x)
      {GetNumber() = number(x); InternalCoerceToNil(); return *this;}

    ///Assigns the value to a float64.
    Value& operator = (float64 x)
      {GetNumber() = number(x); InternalCoerceToNil(); return *this;}

    ///Assigns the value to a float80.
    Value& operator = (float80 x)
      {GetNumber() = number(x); InternalCoerceToNil(); return *this;}

    ///Assigns the value to a ratio.
    Value& operator = (Ratio x)
    {
      Get<Ratio, ValueTypeRatio>() = x;
      InternalCoerceToNil();
      return *this;
    }

    ///Assigns the value to a string.
    Value& operator = (const String& x)
      {Get<String, ValueTypeString>() = x; return *this;}

    ///Assigns the value to a constant string.
    Value& operator = (const ascii* x)
      {Get<String, ValueTypeString>() = x; return *this;}

    ///Assigns the value to a vector.
    Value& operator = (Vector x)
    {
      Get<Vector, ValueTypeVector>() = x;
      InternalCoerceToNil();
      return *this;
    }

    ///Assigns the value to a rectangle.
    Value& operator = (Box x)
    {
      Get<Box, ValueTypeBox>() = x;
      InternalCoerceToNil();
      return *this;
    }

    ///Assigns a byte array as a Base64 encoded string.
    Value& operator = (const Array<byte>& x)
    {
      String s;
      Encoding::Base64::Encode(x, s);
      *this = s;
      return *this;
    }

    ///Templated assignment for client-customizable input.
    template <class T> Value& operator = (T x);

    Value& operator = (Nothing<Value> x)
    {
      (void)x;
      Clear();
      return *this;
    }

    ///Assigns the value to an object.
    template <class T>
    Value& operator = (Pointer<T> x)
    {
      Get<ObjectType, ValueTypeObject>() = x;
      InternalCoerceToNil();
      return *this;
    }

    ///Assigns the value to an object.
    template <class T>
    Value& operator = (Pointer<const T> x)
    {
      Get<ConstObjectType, ValueTypeObject>() = x;
      InternalCoerceToNil();
      return *this;
    }

    ///Assigns a new object pointer inherited from Value::Base.
    template <class T>
    Value& operator = (T* x)
    {
      Get<ObjectType, ValueTypeObject>() = x;
      InternalCoerceToNil();
      return *this;
    }

    /**Merges an incoming tree onto this one. Any keys in the incoming tree
    override keys in the current tree. This method has no effect if the
    incoming value is not a tree. The merge is also recursive, so it applies
    merges to any values that are trees.*/
    void Merge(const Value& v, bool CopyNilValues = false)
    {
      if(not v.IsTree())
        return;

      if(not IsTree())
        NewTree();

      const TreeType& Incoming = v.AssumeAndGet<TreeType>();
      TreeType& Original = Get<TreeType, ValueTypeTree>();

      TreeType::Iterator It;
      for(It.Begin(Incoming); It.Iterating(); It.Next())
      {
        if(It.Value().IsTree())
          Original[It.Key()].Merge(It.Value());
        else if(not It.Value().IsNil() or CopyNilValues)
          Original[It.Key()] = It.Value();
      }
    }

    ///Retrieves the byte array of a Base64 encoded string.
    bool RetrieveBase64Encoded(Array<byte>& x) const
    {
      x.Clear();
      if(ValueType == ValueTypeString)
      {
        String s = AssumeAndGet<String>();
        return Encoding::Base64::Decode(s, x);
      }
      return false;
    }

    //----------------------//
    //Container Manipulation//
    //----------------------//

    ///Clears the value and creates an empty tree and returns a reference.
    Value& NewTree()
    {
      Clear();
      Get<TreeType, ValueTypeTree>();
      return *this;
    }

    ///Clears the value and creates an empty array and returns a reference.
    Value& NewArray()
    {
      Clear();
      Get<ArrayType, ValueTypeArray>();
      return *this;
    }

    /**Creates a new tree if the value is empty and returns a reference. This
    method can be used to help guarantee the stability of a Value reference by
    first assigning it to an empty tree that will not get pruned by its
    parent.*/
    Value& NewTreeIfEmpty()
    {
      return IsNil() ? NewTree() : *this;
    }

    /**Creates a new array if the value is empty and returns a reference. This
    method can be used to help guarantee the stability of a Value reference by
    first assigning it to an empty tree that will not get pruned by its
    parent.*/
    Value& NewArrayIfEmpty()
    {
      return IsNil() ? NewArray() : *this;
    }

    /**Adds a value to the end of the array. If the value is not an array, it
    is first cleared and the type is changed to an array.*/
    Value& Add()
    {
      if(ValueType != ValueTypeArray)
        NewArray();
      ArrayType& a = Get<ArrayType, ValueTypeArray>();
      count NextIndexCount = n();
      a.n(NextIndexCount + 1);
      return a[NextIndexCount];
    }

    /**Creates new object of type T (inheriting from Value::Base) returning it.
    Otherwise, it returns an existing object. This method is guaranteed to
    return either the existing object of type T or a new one of type T.*/
    template <class T> Pointer<T> NewObjectIfEmpty()
    {
      if(not Pointer<T>(Object()))
        *this = new T;
      return Pointer<T>(Object());
    }

    ///Clears the value and creates a new instance of the given object.
    template <class T> void New()
    {
      Clear();
      Get<ObjectType, ValueTypeObject>() = new T;
    }

    /**Assumes that the key exists and returns a const reference to the value.
    This method can be considerably faster than operator [] const since it does
    not return a deep-copy value, but carries with it the responsibility of the
    caller to ensure the key exists before looking for the value.*/
    const Value& AssumeKeyAndReturnConstReference(const Value& Key) const
    {
      /* #screwcase : a[a]
      Make a deep-copy in case of a self-assigned key. Usually a key is not a
      container, so making a deep-copy is not expensive.*/
      Value KeyCopy(Key);

      if(ValueType == ValueTypeArray and KeyCopy.ValueType == ValueTypeInteger)
        return AssumeAndGet<ArrayType>()[count(KeyCopy.DataIntegerValue)];
      else
        return AssumeAndGet<TreeType>()[KeyCopy];
    }

    ///Treats value as container and looks up value by key.
    const Value& operator [] (const Value& Key) const
    {
      /*In the const [] method, the underlying data structure can not change, so
      only return a value if the key exists.*/

      /* #screwcase : a[a]
      Make a deep-copy in case of a self-assigned key. Usually a key is not a
      container, so making a deep-copy is not expensive.*/
      Value KeyCopy(Key);

      if(ValueType == ValueTypeArray and KeyCopy.ValueType == ValueTypeInteger)
        return AssumeAndGet<ArrayType>()[count(KeyCopy.DataIntegerValue)];
      else if(ValueType == ValueTypeTree)
        return AssumeAndGet<TreeType>()[KeyCopy];

      //Key does not exist in the current container.
      return Empty();
    }

    /**Treats object as container and look up value by key. If only non-negative
    keys are used then an array will be used, otherwise a tree will be used.
    Note that if the current type is an array and key is not an index then, the
    array will be cleared and a tree will be created with the new key. For an
    array, all intermediate indices are created and assigned to Nil if they did
    not already have a value. Note that this method will automatically change
    the container type if necessary, and this causes data associated with the
    previous type to be lost.*/
    Value& operator [] (const Value& Key)
    {
      /*In the non-const [] method, the underlying data structure may change
      depending on the current type and the type of the key.*/

      /* #screwcase : a[a]
      Make a deep-copy in case of a self-assigned key. Usually a key is not a
      container, so making a deep-copy is usually not expensive.*/
      Value KeyCopy(Key);

      /*If the current type is a tree, then it stays as a tree (which is more
      general than an array).*/
      bool KeyIsIndex = KeyCopy.ValueType == ValueTypeInteger and
        KeyCopy.DataIntegerValue >= 0;

      //Treat as array if not already a tree and key is an index.
      if(ValueType != ValueTypeTree and KeyIsIndex)
      {
        ArrayType& a = Get<ArrayType, ValueTypeArray>();
        if(a.n() <= count(KeyCopy.DataIntegerValue))
          a.n(count(KeyCopy.DataIntegerValue) + 1);
        return a[count(KeyCopy.DataIntegerValue)];
      }
      else //Treat as a tree if the key is generic or already a tree.
        return Get<TreeType, ValueTypeTree>()[KeyCopy];
    }

    /**Returns the first element of a value that is an array. If the value is
    not an array or has no elements then a global nil reference is returned.*/
    const Value& a() const
    {
      return IsArray() and n() ? (*this)[0] : Empty();
    }

    /**Returns a reference to the first element in an array. If the value was
    not previously an array, then it is turned into a new array and a reference
    to the first element is returned.*/
    Value& a()
    {
      return (*this)[0];
    }

    /**Returns the last element of a value that is an array. If the value is
    not an array or has no elements then a global nil reference is returned.*/
    const Value& z() const
    {
      return IsArray() and n() ? (*this)[n() - 1] : Empty();
    }

    /**Returns a reference to the last element in an array. If the value was
    not previously an array, then it is turned into a new array and a reference
    to the first (same as last) element is returned.*/
    Value& z()
    {
      return IsArray() and n() ? (*this)[n() - 1] : a();
    }

    /**Returns whether the value contains the given key. If the value is an
    array, then it contains the key if it is a non-zero integer less than the
    length of the array. If the value is a tree, then it contains the key if the
    tree contains the same key.*/
    bool Contains(Value Key) const
    {
      bool Found = false;
      if(ValueType == ValueTypeArray and Key.ValueType == ValueTypeInteger)
        Found = AsCount() >= 0 and AsCount() < AssumeAndGet<ArrayType>().n();
      else if(ValueType == ValueTypeTree)
        Found = AssumeAndGet<TreeType>().Contains(Key);
      return Found;
    }

    /**Returns whether the value contains the given key. If the value is an
    array, then it contains the key if it is a non-zero integer less than the
    length of the array. If the value is a tree, then it contains the key if the
    tree contains the same key.*/
    bool Contains(count i) const
    {
      return Contains(Value(i));
    }

    /**Returns whether the value contains the given key. If the value is an
    array, then it contains the key if it is a non-zero integer less than the
    length of the array. If the value is a tree, then it contains the key if the
    tree contains the same key.*/
    bool Contains(const String& s) const
    {
      return Contains(Value(s));
    }

    ///Treats object as a key-value tree and looks up the value for the key.
    template <class T> const Value& operator [] (const T& Key) const
    {
      return (*this)[Value(Key)];
    }

    ///Treats object as a key-value tree and looks up the value for the key.
    template <class T> Value& operator [] (const T& Key)
    {
      return (*this)[Value(Key)];
    }

    /**Prunes any nil elements from the value if it is a tree. If it is not a
    tree, then the value is not changed.*/
    void Prune()
    {
      if(ValueType == ValueTypeTree)
        Get<TreeType, ValueTypeTree>().Prune();
    }

    //----//
    //JSON//
    //----//

    private:

    ///Escapes the string for JSON.
    static String EscapeForJSON(String s)
    {
      s.Replace("\\", "\\\\");
      s.Replace("\"", "\\\"");
      s.Replace(reinterpret_cast<const byte*>("\x00"), 1,
        reinterpret_cast<const byte*>("\\u0000"), 6);
      s.Replace("\x01", "\\u0001");
      s.Replace("\x02", "\\u0002");
      s.Replace("\x03", "\\u0003");
      s.Replace("\x04", "\\u0004");
      s.Replace("\x05", "\\u0005");
      s.Replace("\x06", "\\u0006");
      s.Replace("\x07", "\\u0007");
      s.Replace("\x08", "\\u0008");
      s.Replace("\x09", "\\t");
      s.Replace("\x0a", "\\n");
      s.Replace("\x0b", "\\u000b");
      s.Replace("\x0c", "\\u000c");
      s.Replace("\x0d", "\\r");
      s.Replace("\x0e", "\\u000e");
      s.Replace("\x0f", "\\u000f");
      s.Replace("\x10", "\\u0010");
      s.Replace("\x11", "\\u0011");
      s.Replace("\x12", "\\u0012");
      s.Replace("\x13", "\\u0013");
      s.Replace("\x14", "\\u0014");
      s.Replace("\x15", "\\u0015");
      s.Replace("\x16", "\\u0016");
      s.Replace("\x17", "\\u0017");
      s.Replace("\x18", "\\u0018");
      s.Replace("\x19", "\\u0019");
      s.Replace("\x1a", "\\u001a");
      s.Replace("\x1b", "\\u001b");
      s.Replace("\x1c", "\\u001c");
      s.Replace("\x1d", "\\u001d");
      s.Replace("\x1e", "\\u001e");
      s.Replace("\x1f", "\\u001f");
      return s;
    }

    ///Converts any infinities and NaNs to null.
    template <class T> static String SafeNumber(T x)
    {
      String s;
      if(Limits<T>::IsNaN(x) or Limits<T>::IsEqual(x, Limits<T>::Infinity()) or
        Limits<T>::IsEqual(x, Limits<T>::NegativeInfinity()))
          s << "null";
      else
      {
        s.Append(float64(x), 17, true);
        if(not s.Contains("."))
          s << ".0";
      }
      return s;
    }

    private:

    static bool IsInteger(const String& s)
    {
      bool IsStillInteger = true;
      for(count i = 0; i < s.n() and IsStillInteger; i++)
      {
        ascii x = ascii(s[i]);
        bool CharacterOK = false;
        if(x == '+' and not i) CharacterOK = true;
        if(x == '-' and not i) CharacterOK = true;
        if(x >= '0' and x <= '9') CharacterOK = true;
        IsStillInteger = CharacterOK;
      }
      return IsStillInteger;
    }

    void CreateValueFromXML(const meta::tinyxml2::XMLElement* e, Value& x,
      const List<String>& TagsToExclude,
      const List<String>& AttributesToExclude)
    {
      x.Clear();
      if(not e) return;
      x.NewTree();
      x["@name"] = e->Name();
      const meta::tinyxml2::XMLAttribute* a = e->FirstAttribute();
      while(a)
      {
        Value k, v;
        k = String(a->Name());
        {
          int vi; double vd;
          String vs = String(a->Value());
          v = (vs.Contains(".") and a->QueryDoubleValue(&vd) ==
            meta::tinyxml2::XML_NO_ERROR) ? v = Value(vd) :
          (IsInteger(vs) and
            a->QueryIntValue(&vi) == meta::tinyxml2::XML_NO_ERROR) ?
            v = Value(vi) : v = Value(vs);
        }
        if(not AttributesToExclude.Contains(k.AsString()))
          x[k] = v;
        a = a->Next();
      }
      if(e->FirstChild() and e->FirstChild()->ToText())
      {
        Value v;
        {
          int vi; double vd;
          String vs = String(e->GetText());
          v = (vs.Contains(".") and e->QueryDoubleText(&vd) ==
            meta::tinyxml2::XML_NO_ERROR) ? v = Value(vd) :
          (IsInteger(vs) and
            e->QueryIntText(&vi) == meta::tinyxml2::XML_NO_ERROR) ?
            v = Value(vi) : v = Value(vs);
        }
        x["@value"] = v;
      }
      else
      {
        const meta::tinyxml2::XMLElement* c = e->FirstChildElement();
        count i = 0;
        while(c)
        {
          if(not TagsToExclude.Contains(c->Name()))
            CreateValueFromXML(c, x[Value(i++)], TagsToExclude,
              AttributesToExclude);
          c = c->NextSiblingElement();
        }
      }
    }

    public:

    ///Imports an XML string as a JSON-like structure.
    void FromXML(const String& XMLString, const List<String>& TagsToExclude,
      const List<String>& AttributesToExclude)
    {
      meta::tinyxml2::XMLDocument XMLDoc(true,
        meta::tinyxml2::COLLAPSE_WHITESPACE);
      XMLDoc.Parse(XMLString);
      Clear();
      CreateValueFromXML(XMLDoc.RootElement(), *this, TagsToExclude,
        AttributesToExclude);
    }

    ///Imports an XML string as a JSON-like structure.
    void FromXML(const String& XMLString)
    {
      FromXML(XMLString, List<String>(), List<String>());
    }

    ///Returns the tag name of an element that was imported from XML.
    String Tag() const {return (*this)["@name"];}

    ///Returns the value of element that was imported from XML.
    Value Val() const {return (*this)["@value"];}

    ///Gets the value as JSON with indentation level control.
    String AsJSON(count Level, bool WithWhitespace) const
    {
      //Create the leading whitespace based on the level.
      String t;
      String ln = "\n";
      String sep = " ";
      String p = "  ";
      if(not WithWhitespace)
        ln = p = sep = "";
      for(count l = 0; l < Level; l++)
        t << p;

      //Coerce the value to one of the JSON types.
      switch(ValueType)
      {
      case ValueTypeBoolean:
        return (DataBooleanValue ? "true" : "false");
      case ValueTypeInteger:
        return String(DataIntegerValue);
      case ValueTypeNumber:
      {
        return SafeNumber(DataNumberValue);
      }
      case ValueTypeRatio:
        {
          Ratio r = AsRatio();
          return String("\"") + r.Numerator() + "/" + r.Denominator() + "\"";
        }
      case ValueTypeVector:
        {
          Vector v = AsVector();
          String s;
          s << "[\"_JSONVector\"," << sep << SafeNumber(v.x) <<
            "," << sep << SafeNumber(v.y) << "]";
          return s;
        }
      case ValueTypeBox:
        {
          Box r = AsBox();
          String s;
          s << "[\"_JSONRectangle\"," << sep <<
            SafeNumber(r.a.x) << "," << sep <<
            SafeNumber(r.a.y) << "," << sep <<
            SafeNumber(r.b.x) << "," << sep <<
            SafeNumber(r.b.y) << "]";
          return s;
        }
      case ValueTypeString:
        {
          String s = AsString();
          s = EscapeForJSON(s);
          s.Prepend("\"");
          s.Append("\"");
          return s;
        }
      case ValueTypeArray:
        {
          String s;
          s << "[";
          const ArrayType& a = AssumeAndGet<ArrayType>();
          for(count i = 0, n = a.n(); i < n; i++)
          {
            s << ln << t << p <<
              a[i].AsJSON(Level + 1, WithWhitespace);
            if(i != n - 1)
              s << ",";
          }
          s << ln << t << "]";
          return s;
        }
      case ValueTypeTree:
        {
          String s;
          s << "{";
          const TreeType& kv = AssumeAndGet<TreeType>();
          TreeType::Iterator It;
          bool First = true;
          for(It.Begin(kv); It.Iterating(); It.Next(), First = false)
          {
            if(not First)
              s << ",";
            if(It.Key().ValueType == ValueTypeString)
            {
              s << ln << t << p << "" <<
                It.Key().AsJSON(0, WithWhitespace) << ":" << sep;
            }
            else
            {
              s << ln << t << p << "\"" <<
                EscapeForJSON(It.Key().AsJSON(0, WithWhitespace)) <<
                "\":" << sep;
            }
            s << It.Value().AsJSON(Level + 1, WithWhitespace);
          }
          s << ln << t << "}";
          return s;
        }
      case ValueTypeNil:
        break;
      case ValueTypeObject:
        {
          String s;
          const Value::Base* RawPointer = ConstObject().Raw();
          s << "\"_JSONObject<";
          s << (RawPointer ? ConstObject()->Name().Merge() : "null");
          s << ", " << reinterpret_cast<const void*>(RawPointer) << ">\"";
          return s;
        }
      }
      return "null";
    }

    public:

    /**Exports the value as JSON. The Value class is more general than JSON and
    so the following caveats apply: ratios are written as a string containing
    two integers (numerator and denominator) separated by a slash; vectors and
    rectangles are written as an array with an initial type string followed by
    two or four numbers respectively; tree keys are coerced to a string
    representation and for keys that are trees, the corresponding JSON text is
    rendered as a compact escaped JSON string.*/
    String ExportJSON(bool WithWhitespace = true, bool WithRoot = true) const
    {
      String s;

      //Write outside container as an array.
      if(ValueType == ValueTypeArray or ValueType == ValueTypeBox or
        ValueType == ValueTypeVector or ValueType == ValueTypeTree)
          s >> AsJSON(0, WithWhitespace);
      else if(not WithRoot)
        s >> AsJSON(0, WithWhitespace);
      else
      {
        s >> "[";
        if(WithWhitespace) s++;
        s << AsJSON(1, WithWhitespace);
        if(WithWhitespace) s++;
        s << "]";
      }

      return s;
    }
  };

  //------------------------//
  //String Append Extensions//
  //------------------------//

  ///Appends a value to a string.
  String& operator << (String& x, const Value& y);

#ifdef PRIM_COMPILE_INLINE
  //Virtual destructor written out-of-line to satisfy requirement.
  Value::Base::~Base() {}
  Value::ConstReference::~ConstReference() {}

  String& operator << (String& x, const Value& y)
  {
    x.Append(y.AsString().Merge());
    return x;
  }
#endif
}
#endif
