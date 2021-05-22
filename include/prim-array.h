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

#ifndef PRIM_INCLUDE_ARRAY_H
#define PRIM_INCLUDE_ARRAY_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

#include <iostream>

namespace PRIM_NAMESPACE {

///Internal classes used by other prim classes
namespace meta
{
  /**Custom growth model. This model 1) does not shrink until array size goes
  back down to zero; 2) allocates exact size for first allocation; 3) if
  previously allocated size fits new size, use existing size; 4) otherwise
  grows by a factor of the new size.*/
  class CustomGrowthModel
  {
    public:

    static count CalculateRealSize(count ActualCurrentSize, count NewSize)
    {
      count ActualNewSize;

      if(NewSize == 0) //Shrink if size becomes zero.
        ActualNewSize = 0;
      else if(ActualCurrentSize == 0) //First time allocation, use exact-size.
        ActualNewSize = NewSize;
      else if(NewSize <= ActualCurrentSize) //If it fits, use current size.
        ActualNewSize = ActualCurrentSize;
      else //Non-initial allocation, so grow by a factor of new size.
        ActualNewSize = NewSize * 3;

      return Max(NewSize, ActualNewSize);
    }
  };

  ///Growth model that rounds apparent size up to nearest power of two.
  class PowerOfTwoGrowthModel
  {
    public:

    /**Returns the lowest power-of-two that can contain the input. The real
    size is always greater than or equal to the apparent size, permitting the
    apparent size to grow for a time before the real size must be increased.*/
    static count CalculateRealSize(count ActualCurrentSize, count NewSize)
    {
      (void)ActualCurrentSize;

      //If 1 or 2, then Real = Apparent.
      if(NewSize <= 2)
        return NewSize;

      /*If 3 or higher, calculate the physical size by finding the least power
      of two greater than or equal to the logical size.*/
      count RealSize = 2;
      do
      {
        RealSize *= 2;
        if(RealSize >= NewSize)
          return RealSize;
      }
      while(RealSize > 0 and uintptr(RealSize) < Limits<uintptr>::HighBit());

      //Overflow returns zero.
      return 0;
    }
  };

  ///Growth model in which the real size is the apparent size.
  class LinearGrowthModel
  {
    public:

    /**Returns the same number of storage elements as actual elements. The real
    size is equal to the apparent size.*/
    static count CalculateRealSize(count ActualCurrentSize, count NewSize)
    {
      (void)ActualCurrentSize;
      //Return 0 if bad argument.
      if(NewSize < 0)
        return 0;

      return NewSize;
    }
  };

  ///Growth model in which apparent size is rounded up to nearest multiple of m.
  template <count m>
  class ConstantGrowthModel
  {
    public:

    /**Returns the least multiple of the argument m containing the actual size.
    The real size is always greater than or equal to the apparent size,
    permitting the apparent size to grow for a time before the real size must be
    increased. This type of growth is good for arrays that have fairly certain
    upper-bounds and sizes that stay mostly constant.*/
    static count CalculateRealSize(count ActualCurrentSize, count NewSize)
    {
      (void)ActualCurrentSize;
      //Return 0 if bad argument.
      if(NewSize < 0)
        return 0;

      if(NewSize % m == 0)
        return NewSize;
      else
        return (NewSize / m + 1) * m;
    }
  };
}}

namespace PRIM_NAMESPACE
{
  /**An array with customizable growth model allocation and dynamic resizing.
  The default growth model is exponential base-two so that memory copies only
  occur when power-of-two lengths are exceeded. This model is a good choice for
  most dynamic array situations since it is reasonable to assume that a given
  array could easily expand to twice its current value. Other models may be more
  appropriate though, for example, when memory conservation is important, or
  when the dynamics of the array growth are well-known ahead of time.*/
  template <typename T, typename GM = meta::CustomGrowthModel>
  class Array
  {
    ///Empty item in case of bad access.
    mutable T Empty; PRIM_PAD(T)

    ///Typed pointer to the beginning of the array data.
    T* Data;

    /**The apparent number of elements in the array. The real number of elements
    allocated can be derived with CalculateRealSize().*/
    count ApparentSize;

    ///Actual number of elements allocated for the array.
    count RealSize;

  public:

    //--------------//
    //Element Access//
    //--------------//

    /**Returns the i-th element by index. The method checks bounds
    before accessing the data and returns an empty value if out-of-bounds.
    The returned element is not const protected.*/
    inline T& ith(count i)
    {
      if(not Data or uint64(i) >= uint64(ApparentSize))
      {
        Empty = Nothing<T>();
        return Empty;
      }
      return Data[i];
    }

    /**Returns the i-th const element by index. The method checks bounds
    before accessing the data and returns an empty value if out-of-bounds.
    The returned element is const protected.*/
    inline const T& ith(count i) const
    {
      if(not Data or uint64(i) >= uint64(ApparentSize))
      {
        Empty = Nothing<T>();
        return Empty;
      }
      return static_cast<const T&>(Data[i]);
    }

    /**Returns the indexed element. Internally inlines ith().*/
    inline T& operator [] (count i)
    {
      return ith(i);
    }

    /**Returns a const indexed element. Internally inlines ith().*/
    inline const T& operator [] (count i) const
    {
      return ith(i);
    }

    /**Returns the first element of the array. Assumes that there is at least
    one element in the array. The returned element is const protected. Note
    lowercase is used because the method is treated like a mathematical
    variable.*/
    inline T& a()
    {
      return ith(0);
    }

    /**Returns the first const element of the array. Assumes that there is at
    least one element in the array. The returned element is const protected.
    Note lowercase is used because the method is treated like a mathematical
    variable.*/
    inline const T& a() const
    {
      return ith(0);
    }

    /**Returns the last element of the array or an element with respect to last.
    ItemsFromEnd must be nonnegative. Assumes that there is at least one element
    in the array. The returned element is const protected. Note lowercase is
    used because the method is treated like a mathematical variable.*/
    inline T& z(count ItemsFromEnd = 0)
    {
      return ith(ApparentSize - 1 - ItemsFromEnd);
    }

    /**Returns the last element of the list or an element with respect to last.
    ItemsFromEnd must be nonnegative. Assumes that there is at least one element
    in the array. The returned element is const protected. Note lowercase is
    used because the method is treated like a mathematical variable.*/
    inline const T& z(count ItemsFromEnd = 0) const
    {
      return ith(ApparentSize - 1 - ItemsFromEnd);
    }

    //------------------//
    //Element Comparison//
    //------------------//

    ///Returns whether all elements are less than a maximum.
    bool operator < (const T& Maximum) const
    {
      bool IsLessThanMaximum = true;
      for(count i = 0; i < n() and IsLessThanMaximum; i++)
        if(not (ith(i) < Maximum))
          IsLessThanMaximum = false;
      return IsLessThanMaximum;
    }

    ///Returns whether all elements are less than or equal to a maximum.
    bool operator <= (const T& Maximum) const
    {
      bool IsLessThanMaximum = true;
      for(count i = 0; i < n() and IsLessThanMaximum; i++)
        if(not (ith(i) <= Maximum))
          IsLessThanMaximum = false;
      return IsLessThanMaximum;
    }

    ///Returns whether all elements are the same as the value.
    bool operator == (const T& Same) const
    {
      bool IsSame = true;
      for(count i = 0; i < n() and IsSame; i++)
        if(not (ith(i) == Same))
          IsSame = false;
      return IsSame;
    }

    ///Returns whether all elements are greater than or equal to a minimum.
    bool operator >= (const T& Minimum) const
    {
      bool IsGreaterThanMinimum = true;
      for(count i = 0; i < n() and IsGreaterThanMinimum; i++)
        if(not (ith(i) >= Minimum))
          IsGreaterThanMinimum = false;
      return IsGreaterThanMinimum;
    }

    ///Returns whether all elements are greater than a minimum.
    bool operator > (const T& Minimum) const
    {
      bool IsGreaterThanMinimum = true;
      for(count i = 0; i < n() and IsGreaterThanMinimum; i++)
        if(not (ith(i) > Minimum))
          IsGreaterThanMinimum = false;
      return IsGreaterThanMinimum;
    }

    //----//
    //Size//
    //----//

    ///Returns the size of the array.
    inline count n() const
    {
      return ApparentSize;
    }

    /**Sets the size of the array. New elements will have constructors called.
    If the array does not fit the current allocation, then the copy constructors
    of all the preexisting elements will also be called. The array returns a
    pointer to the data, which can be used for error-checking. If the array has
    no elements, the pointer will be zero.*/
    const T* n(count NewSize)
    {
#ifndef __clang_analyzer__
      //In case the new requested size is the same, immediately return.
      if(NewSize == ApparentSize)
        return Data;

      //In the case of a no size array, just delete all the data.
      if(NewSize <= 0)
      {
        //Delete each of the elements (this will call destructors).
        delete [] Data;
        Data = 0;
        RealSize = ApparentSize = 0;
        return 0;
      }

      /*Calculate the real size using the growth model passed by template
      argument.*/
      count OldRealSize = RealSize;
      count NewRealSize = GM::CalculateRealSize(OldRealSize, NewSize);

      /*If no extra data needs to be allocated, then just change the size of the
      array.*/
      if(OldRealSize == NewRealSize)
      {
        //Set the new apparent size of the array.
        ApparentSize = NewSize;
        return Data;
      }

      //Allocate a new contiguous block of memory.
      T* NewData = new T[NewRealSize];

      //If memory could not be allocated, return zero.
      if(not NewData)
        return 0;

      //Get the minimum of the new size and the current apparent size.
      count ElementsInCommon = NewSize > ApparentSize ? ApparentSize : NewSize;

      /*Swap memory contents of Data and NewData. The following two lines are a
      bait-and-switch. Data is about to be deleted so we want to take the newly
      constructed empty objects of NewData and swap them for the real content in
      Data. This way, when Data is deleted it is only deleting empty objects and
      any objects which are supposed to be deleted. This absolutely assumes that
      the user has not stored a pointer to the elements in the array (which can
      only be guaranteed for as long as this resize function is not called).*/
      Memory::SwapMemory(Data, NewData, ElementsInCommon);

      //Delete old array.
      delete [] Data;

      //Set the new size of the array.
      ApparentSize = NewSize;
      RealSize = NewRealSize;

      //Return pointer to the new data array.
      return Data = NewData;
#else
      //This line is only seen by the static analyzer.
      return Data;
#endif
    }

    //--------//
    //Clearing//
    //--------//

    /**Clears the array. The destructors are called on each element. The return
    value is always false to facilitate code that must return an error
    condition.*/
    inline bool Clear()
    {
      n(0);
      return false;
    }

    /**Clears the array after deleting the objects pointed to by each element.
    Elements are deleted in reverse order from last to first. This method may
    only be used when the element is a pointer type.*/
    inline void ClearAndDeleteAll()
    {
      for(count i = n() - 1; i >= 0; i--)
        delete Data[i];
      n(0);
    }

    /**Zeroes the memory contents of the array. Only use this method with
    plain-old data. Polymorphic objects with v-tables should not be zeroed.*/
    void Zero()
    {
      Memory::Clear(Data, ApparentSize);
    }

    /**Zeroes the memory contents of the array starting at the given index. Only
    use this method with plain-old-data. Objects with polymorphic v-tables
    should not be zeroed.*/
    void ZeroFrom(count Index)
    {
      if(Index < 0)
        Index = 0;
      else if(Index >= ApparentSize)
        return;
      Memory::Clear(&Data[Index], ApparentSize - Index);
    }

    void RemoveAll()
    {
      Clear();
    }

    void RemoveAndDeleteAll()
    {
      ClearAndDeleteAll();
    }

    //------//
    //Adding//
    //------//

    ///Adds an element to the array and returns a reference to that element.
    T& Add()
    {
      n(ApparentSize + 1);
      return z();
    }

    ///Adds an element to the array using a copy constructor.
    void Add(const T& NewElement)
    {
#ifndef __clang_analyzer__
      n(ApparentSize + 1);
      Data[ApparentSize - 1] = NewElement;
#endif
    }

    ///Alias for Add
    void Push(const T& NewElement)
    {
      Add(NewElement);
    }

    ///Pop preserves API compatibility with other classes
    T Pop()
    {
      T Copy = Nothing<T>();
      if(n())
      {
        Copy = z();
        n(n() - 1);
      }
      return Copy;
    }

    ///Appends the contents of another array to this one.
    void Append(const Array<T>& Other)
    {
      count OriginalSize = n();
      n(OriginalSize + Other.n());
      for(count i = 0, n = Other.n(); i < n; i++)
        ith(OriginalSize + i) = Other[i];
    }

    ///Reverses the elements in the array.
    void Reverse()
    {
      for(count i = n() / 2; i-->0;)
        Swap(ith(i), ith(n() - 1 - i));
    }

    //-------//
    //Copying//
    //-------//

    /**Copys elements one-by-one from other array data. First the array is
    resized to have the same number of elements as the other. Then each element
    from the other is copied by assignment. Note that the resizing step will
    cause constructors to be called, which may be inefficient. If your array
    contains plain-old data then you are better off using CopyMemoryFrom().*/
    void CopyFrom(const T* OtherArray, count NumberOfElementsToTake)
    {
      //If there is nothing to copy, then clear the array.
      if(not OtherArray)
      {
        n(0);
        return;
      }

      //If the source and destination are the same, no copying is necessary.
      if(Data == OtherArray)
        return;

      //Resize the array.
      n(NumberOfElementsToTake);

      //If there is nothing to copy, then return.
      if(not Data)
        return;

      //Copy each element by assignment.
      for(count i = 0; i < ApparentSize; i++)
        Data[i] = OtherArray[i];
    }

    ///Copy from an array of strings.
    void CopyFrom(const ascii** OtherArray, count NumberOfStrings)
    {
      n(NumberOfStrings);
      for(count i = 0; i < NumberOfStrings; i++)
        ith(i) = OtherArray[i];
    }

    /**Copys elements one-by-one from another array. First the array is resized
    to have the same number of elements as the other. Then each element from the
    other is copied by assignment. Note that the resizing step will cause
    constructors to be called, which may be inefficient. If your array contains
    plain-old data then you are better off using CopyMemoryFrom().*/
    void CopyFrom(const Array<T>& Other)
    {
      CopyFrom(static_cast<const T*>(Other.Data), Other.ApparentSize);
    }

    void CopyFrom(const Array<T>& Other, count NumberOfElementsToTake)
    {
      CopyFrom(static_cast<const T*>(Other.Data),
        Min(NumberOfElementsToTake, Other.ApparentSize));
    }

    /**Copies the memory of other array data. This method will only call the
    constructors of the newly created elements. If the source is already an
    Array, then it is preferable to pass in the Array object so that the
    assignment operators will be used. Only use this copy constructor if the
    elements are plain-old data.*/
    void CopyMemoryFrom(const T* OtherArray, count NumberOfElementsToTake)
    {
      //If the source and destination are the same, no copying is necessary.
      if(Data == OtherArray)
        return;

      //Resize the array.
      n(NumberOfElementsToTake);

      //Copy the entire array as plain-old data.
      Memory::Copy(Data, OtherArray, ApparentSize);
    }

    /**Copies the memory of another array. First the array is resized to have
    the same number of elements. Then a memory copy operation is done on the
    array to copy the contents of the other array byte-for-byte into this one.
    Note well, in order for this to be valid, the elements must be plain-old
    data, i.e., they must not contain members with special constructors and must
    not be dynamically polymorphic. Use the CopyFrom() method for complex
    objects so that they will be properly copied using the assignment
    operator.*/
    void CopyMemoryFrom(const Array<T>& Other)
    {
      CopyMemoryFrom(Other.Data, Other.ApparentSize);
    }

    ///Quickly swaps data with another array by exchanging pointers and size.
    void SwapWith(Array<T>& Other)
    {
      Memory::Swap(Data, Other.Data);
      Memory::Swap(ApparentSize, Other.ApparentSize);
      Memory::Swap(RealSize, Other.RealSize);
    }

    //--------//
    //Encoding//
    //--------//

    ///Creates a hex version of this array.
    void EncodeAsASCIIHex(Array<byte>& HexArray) const
    {
      HexArray.Clear();
      HexArray.n(n() * 2 * count(sizeof(T)));
      count HexIndex = 0;
      byte* ByteData = reinterpret_cast<byte*>(Data);
      for(count i = 0; i < n(); i++)
      {
        for(count j = 0; j < count(sizeof(T)); j++)
        {
          byte X = ByteData[i * count(sizeof(T)) + j];
          byte A = (X >> 4) & 15;
          if(A < 10)
            A += 48;
          else
            A += 55;
          byte B = X & 15;
          if(B < 10)
            B += 48;
          else
            B += 55;
          HexArray[HexIndex++] = A;
          HexArray[HexIndex++] = B;
        }
      }
    }

    //---------//
    //Searching//
    //---------//

    ///Returns index of first element matching key or -1 if there is no match.
    count Search(const T& Key) const
    {
      for(count i = 0; i < n(); i++)
        if(ith(i) == Key)
          return i;
      return -1;
    }

    ///Returns whether the array contains the key.
    bool Contains(const T& Key) const
    {
      return Search(Key) != -1;
    }

    ///Returns whether or not the array is sorted.
    bool IsSorted() const
    {
      bool Ordered = true;
      for(count i = 0; i < n() - 1 and Ordered; i++)
        Ordered = not(ith(i) > ith(i + 1));
      return Ordered;
    }

    //----------------------//
    //Constructor-Destructor//
    //----------------------//

    ///Initializes an empty array.
    Array() : Data(0), ApparentSize(0), RealSize(0) {}

    /**Creates an array of a certain number of elements. Note that if the
    element type is a built-in type and not a class, then the elements will be
    in an uninitialized state. The Zero() method can be used to clear the data
    if necessary.*/
    Array(count Elements) : Data(0), ApparentSize(0), RealSize(0) {n(Elements);}

    /**One-by-one element level copy constructor. Internally calls the
    CopyFrom() method. You could also use CopyMemoryFrom() if your element is
    plain-old data.*/
    Array(const Array<T>& Other) : Data(0), ApparentSize(0), RealSize(0)
    {
      CopyFrom(Other);
    }

    ///Copy constructor from a pointer to an array.
    Array(const T* OtherArray, count NumberOfElementsToTake) : Data(0),
      ApparentSize(0), RealSize(0)
    {
      CopyFrom(OtherArray, NumberOfElementsToTake);
    }

    ///Copy constructor from a pointer to an array of strings.
    Array(const ascii** OtherArray, count NumberOfStrings) : Data(0),
      ApparentSize(0), RealSize(0)
    {
      CopyFrom(OtherArray, NumberOfStrings);
    }

    /**Assignment operator to copy array elements one-by-one. Internally calls
    CopyFrom().*/
    Array<T>& operator = (const Array<T>& Other)
    {
      CopyFrom(Other);
      return *this;
    }

    ///Returns whether the contents of this array are identical to another.
    bool operator == (const Array<T>& Other) const
    {
      if(n() != Other.n()) return false;
      for(count i = 0; i < n(); i++)
        if(ith(i) != Other.ith(i))
          return false;
      return true;
    }

    ///Returns whether the contents of this array are not identical to another.
    bool operator != (const Array<T>& Other) const
    {
      return !(*this == Other);
    }

    /**Clears the array, calling each of the elements' destructors. Note that if
    the objects are pointers, make sure to call ClearAndDeleteAll() before the
    array goes out of scope, so that the elements get cleaned up properly.*/
    ~Array() {n(0);}
  };
}
#endif
