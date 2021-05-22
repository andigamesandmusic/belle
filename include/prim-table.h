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

#ifndef PRIM_INCLUDE_TABLE_H
#define PRIM_INCLUDE_TABLE_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  /**Associative array table for independent key and value types. Every table
  has a special value called Nothing, and by definition the table only has keys
  that are not this value. When the value of a key is set to Nothing, then it
  is pruned from the table automatically. The bracket operators return
  reference-counted key-value objects that will cast automatically to the value
  type to allow for semantically nice key-value sets and gets. The table may
  be enumerated, permits self-assigning operations, and allows the value of
  Nothing to be changed on a per table basis.*/
  template <class K, class V = K>
  class Table
  {
    private:

    ///Internal class to store the key-value pair.
    class KeyValueData
    {
      public:

      //Stores the key.
      K Key; PRIM_PAD(K)

      //Stores the value.
      V Value; PRIM_PAD(V)
    };

    public:

    /**Stores a reference-counted handle to the key-value pair. Note that this
    class is not intended to be instantiated directly in caller code. If it is
    used beyond its purpose as a temporary smart reference, the caller may
    inadvertantly interfere with the cached pruning state of the table. The
    effect of this would be enumerations resulting in keys with Nothing values
    that should have been pruned from the table. This circumstance can be
    avoided by simply never instantiating a KeyValue directly and always
    performing value gets and sets at the point of key lookup.*/
    class KeyValue
    {
      protected:

      ///Reference-counted pointer to key-value data.
      Pointer<KeyValueData> PointerToKeyValueData;

      public:

      ///Constructor to create a new reference-counted key-value pair.
      KeyValue() : PointerToKeyValueData(new KeyValueData) {}

      /*The default copy constructor copies the shared pointer so that it
      will have shared access to the key-value data. It needs to be public in
      this case to allow for binding a reference to a temporary.*/
      KeyValue(const KeyValue& Other) :
        PointerToKeyValueData(Other.PointerToKeyValueData) {}

      private:

      ///Mutable accessor to the key data.
      K& Key() {return PointerToKeyValueData->Key;}

      ///The Table class needs mutable access to the Key() method above.
      friend class Table;

      public:

      ///Accessor to the key data as a constant
      const K& Key() const {return PointerToKeyValueData->Key;}

      ///Accessor to the value data
      V& Value() {return PointerToKeyValueData->Value;}

      ///Accessor to the value data as a constant
      const V& Value() const {return PointerToKeyValueData->Value;}

      ///Automatic cast of KeyValue to value data reference
      operator V& () {return Value();}

      ///Automatic cast of KeyValue to constant value data reference
      operator const V& () const {return Value();}

      ///Tests equivalence of the value of this KeyValue to another value.
      bool operator == (const V& Other) const {return Value() == Other;}

      ///Tests non-equivalence of the value of this KeyValue to another value.
      bool operator != (const V& Other) const {return !(*this == Other);}

      ///Assigns another value to this KeyValue.
      KeyValue operator = (const V& NewValue)
      {
        Value() = NewValue;
        return *this;
      }

      /**Assigns the value of another KeyValue to this one. This method grants
      the syntax t["foo"] = t["bar"]. Note that this operation intentionally
      behaves differently from the private copy constructor, which shares
      ownership of the key-value data.*/
      KeyValue operator = (const KeyValue& Other)
      {
        Value() = Other.Value();
        return *this;
      }
    };

    private:

    /* #todo : Determine if SortableKeyValue is necessary now that
    SwappableArray will directly call the user defined Swap method.*/

    /**Separate class to differentiate between KeyValue assignment and swapping.
    This class is necessary since otherwise the sorting algorithm will pick the
    value assign operator = in KeyValue and not actually swap both the key
    and the value.*/
    class SortableKeyValue : public KeyValue
    {
      public:

      ///Creates an empty sortable key value.
      SortableKeyValue() : KeyValue() {}

      /**Copy constructor that copies both key and value from another object.
      It differs from the KeyValue copy constructor in that it does not share
      ownership, rather it copies values of the KeyValueData.*/
      SortableKeyValue(const SortableKeyValue& Other) : KeyValue()
      {
        *this = Other;
      }

      //Performs a fast swap by pointer.
      void Swap(SortableKeyValue& Other)
      {
        KeyValue::PointerToKeyValueData.Swap(Other.PointerToKeyValueData);
      }

      ///Assignment operator to copy both key and value from another object.
      SortableKeyValue& operator = (const SortableKeyValue& Other)
      {
        KeyValue::Value() = Other.Value();
        KeyValue::Key() = Other.Key();
        return *this;
      }

      ///Less-than comparison for sorting by key.
      bool operator < (const SortableKeyValue& Other) const
      {
        return KeyValue::Key() < Other.Key();
      }

      ///Greater-than comparison for sorting by key.
      bool operator > (const SortableKeyValue& Other) const
      {
        return KeyValue::Key() > Other.Key();
      }
    };

    private:

    //--------------//
    //Internal State//
    //--------------//

    ///Stores an array of sortable key-value references.
    mutable Sortable::SwappableArray<SortableKeyValue> Data;

    /**Stores whether the table is currently sorted. The table is allowed to be
    in an unsorted state following a new key operation, and it is sorted prior
    to key-lookup.*/
    mutable bool CurrentlySorted; PRIM_PAD(bool)

    /**Stores whether the table is thought to have no keys with Nothing values.
    Since KeyValueData is stored by reference-counted objects, it is possible
    that a table key-value could change to Nothing by way of a stale
    reference.*/
    mutable bool CurrentlyPruned; PRIM_PAD(bool)

    ///Stores the value to be returned if a key does not exist.
    V NothingValue; PRIM_PAD(V)

    //----------------//
    //Internal Helpers//
    //----------------//

    ///Safely tests whether a value is not equal to nothing.
    bool DiffersFromNothing(const V& Value) const
    {
      return Limits<V>::IsNotEqual(Value, NothingValue);
    }

    ///Sorts the table by key.
    void SortByKey() const
    {
      //Sort the table if necessary.
      if(not CurrentlySorted)
      {
        Data.Sort();
        CurrentlySorted = true;
      }
    }

    ///Prunes all keys with Nothing values from the array and then sorts by key.
    void LazyPruneAndSortByKey() const
    {
      if(not CurrentlyPruned)
      {
        count j = 0;
        for(count i = 0; i < Data.n(); i++)
        {
          if(DiffersFromNothing(Data[i].Value()))
          {
            if(i != j)
              Data[j].Swap(Data[i]);
            j++;
          }
        }
        Data.n(j);
        CurrentlyPruned = true;
        SortByKey();
      }
    }

    ///Forcibly prunes keys with Nothing values from the table.
    void ForcePruneAndSortByKey() const
    {
      CurrentlyPruned = CurrentlySorted = false;
      LazyPruneAndSortByKey();
    }

    ///Looks up the key by index.
    count LookupKeyIndex(const K& Key) const
    {
      //First make sure the table is sorted.
      SortByKey();

      //Look for the key.
      SortableKeyValue Needle;
      Needle.Key() = Key;
      return Data.Search(Needle);
    }

    ///Gets a const reference to the key-value by key.
    const KeyValue Get(const K& Key) const
    {
      count i = LookupKeyIndex(Key);

      //If the key is found return the value, if not create a new value.
      if(i >= 0)
        return Data[i];
      else
      {
        KeyValue NothingReference;
        NothingReference.Key() = Key;
        NothingReference.Value() = NothingValue;
        return NothingReference;
      }
    }

    ///Gets a reference to the key-value by key.
    KeyValue Get(const K& Key)
    {
      count i = LookupKeyIndex(Key);

      /*Returning a mutable key-value permits the caller to change the value to
      Nothing, in which case it would need to be pruned from the table.*/
      CurrentlyPruned = false;

      //If the key is found return the value and if not, create a new value.
      if(i >= 0)
        return Data[i];
      else
      {
        Data.Add();
        Data.z().Key() = Key;
        Data.z().Value() = NothingValue;
        CurrentlySorted = false;
        return Data.z();
      }
    }

    public:

    //----------//
    //Friendship//
    //----------//

    /*Value needs to be able to do special iteration that temporarily disables
    pruning.*/
    friend class Value;

    //------------//
    //Construction//
    //------------//

    ///Creates an empty table.
    Table() : CurrentlySorted(true), CurrentlyPruned(true),
      NothingValue(Nothing<V>()) {}

    ///Creates an empty table and sets a user-defined value for Nothing.
    Table(const V& ValueOfNothing) : CurrentlySorted(true),
      CurrentlyPruned(true), NothingValue(ValueOfNothing) {}

    //------------//
    //Manipulation//
    //------------//

    ///Clears the table.
    void Clear()
    {
      Data.Clear();
      CurrentlyPruned = CurrentlySorted = true;
    }

    //----------------//
    //Key-Value Lookup//
    //----------------//

    ///Gets a mutable reference to the key-value given the key.
    KeyValue operator [] (const K& Key) {return Get(Key);}

    ///Gets a constant reference to the key-value given the key.
    const KeyValue operator [] (const K& Key) const {return Get(Key);}

    ///Returns whether the table contains a value for the given key.
    bool Contains(const K& Key) const
    {
      return DiffersFromNothing(Get(Key));
    }

    //-----------//
    //Enumeration//
    //-----------//

    ///Gets the total number of key-value pairs in the table.
    count n() const
    {
      LazyPruneAndSortByKey();
      return Data.n();
    }

    /**Gets the ith key-value pair in the table sorted by key. Use n() to
    determine the number of key-value pairs in the table.*/
    const KeyValue ith(count i) const
    {
      LazyPruneAndSortByKey();
      if(i < 0 or i >= Data.n())
      {
        KeyValue NothingReference;
        NothingReference.Key() = Nothing<K>();
        NothingReference.Value() = NothingValue;
        return NothingReference;
      }
      return Data[i];
    }

    private:

    ///Gets the total number of key-value pairs in the table without pruning.
    count nRaw() const
    {
      return Data.n();
    }

    ///Same as ith() except there is no pruning or safety checks.
    const KeyValue ithRaw(count i) const
    {
      return Data[i];
    }

    public:

    ///Print the contents of the table to a string.
    operator String () const
    {
      LazyPruneAndSortByKey();
      String s;
      s << "{";
      for(count i = 0; i < Data.n(); i++)
      {
        if(i)
          s << " ";
        s << Data[i].Key() << ":" << Data[i].Value();
      }
      s << "}";
      return s;
    }

    //----------------------//
    //Local Value of Nothing//
    //----------------------//

    ///Returns the current value used to represent Nothing.
    V GetNothing() const {return NothingValue;}

    /**Sets a new value for Nothing. It also prunes all keys whose value was
    either the original or new value for Nothing.*/
    void SetNothing(const V& NewValueOfNothing)
    {
      //Force a prune and sort with the original value for Nothing.
      ForcePruneAndSortByKey();

      //Set the new value for Nothing.
      NothingValue = NewValueOfNothing;

      //Force another prune and sort with the new value of Nothing.
      ForcePruneAndSortByKey();
    }

    //----------------------//
    //Key and Value Equality//
    //----------------------//

    ///Tests whether this table has identical contents to another.
    bool operator == (const Table<K, V>& Other) const
    {
      //If the tables are not the same size then they have different contents.
      if(n() != Other.n())
        return false;

      /*If the tables have the same size and that size is zero then they are
      both identically empty.*/
      if(not n())
        return true;

      //Check each key-value pair for equivalence.
      for(count i = 0; i < n(); i++)
        if(Limits<K>::IsNotEqual(ith(i).Key(), Other.ith(i).Key()) or
          Limits<V>::IsNotEqual(ith(i), Other.ith(i)))
            return false;

      return true;
    }

    ///Tests whether this table has non-identical contents to another.
    bool operator != (const Table<K, V>& Other) const
    {
      return !(*this == Other);
    }
  };
}
#endif
