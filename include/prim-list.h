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

#ifndef PRIM_INCLUDE_LIST_H
#define PRIM_INCLUDE_LIST_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  /**Doubly-linked list with an automatic iterator.

  List recognizes and optimizes sequential access by maintaining three points
  of traversal: the beginning, a variable index, and the end. The variable index
  is updated each time an element is requested by index. This way sequential
  access is as efficient as an iterator. Using array-like indexing carries with
  it no algorithmic performance penalty.

  This indexing optimization does not make random access more efficient. If
  random or contiguous access is a requirement, then the Array would be a better
  choice.*/
  template <class T>
  class List
  {
    protected:

    ///Implements a simple double link.
    class DoubleLink
    {
      public:

      ///Object containing data of the list type
      T Data; PRIM_PAD(T)

      ///Pointer to the next link or null if there is none.
      DoubleLink* Next;

      ///Pointer to the previous link or null if there is none.
      DoubleLink* Prev;

      ///Default constructor
      DoubleLink() : Next(0), Prev(0) {}

      ///Copy constructor
      DoubleLink(T& DataToCopy) : Next(0), Prev(0) {Data = DataToCopy;}

      ///Assignment operator
      T operator = (const T& Other)
      {
        Data = Other;
        return Data;
      }

      ///Automatic casting into T
      operator T&()
      {
        return Data;
      }
    };

    ///Pointer to the first link
    DoubleLink* First;

    ///Pointer to the last link
    DoubleLink* Last;

    ///Pointer to the last referenced link
    mutable DoubleLink* LastReferenced;

    ///Index of the last referenced link
    mutable count LastReferencedIndex;

    ///Number of items in the list
    count Items;

    ///Returns a pointer to the link containing the given data.
    DoubleLink* GetLinkFromItem(const T& i)
    {
      /*Using a trick in order to get pointers to the DoubleLink by calculating
      the beginning of the class using a test link for reference to determine
      where the data part is in relation to the rest of the class. By doing this
      no non-portable assumptions are made about what a compiler does with
      member order in physical memory.*/
      const DoubleLink& TestLink = Singleton<DoubleLink>::Instance();
      return reinterpret_cast<DoubleLink*>(
        reinterpret_cast<uintptr>(&i) +
        reinterpret_cast<uintptr>(&TestLink.Data) -
        reinterpret_cast<uintptr>(&TestLink));
    }

    /**Swaps the position of two elements without copying their internal data.
    Only elements from the same list may be swapped.*/
    void SwapElements(const T& A, const T& B)
    {
      //Get pointers to the actual links.
      DoubleLink* PointerA = GetLinkFromItem(A);
      DoubleLink* PointerB = GetLinkFromItem(B);

      //If links are the same, no swap is necessary.
      if(PointerA == PointerB)
        return;

      //Remember the original link pointers.
      DoubleLink* OldAPrev = PointerA->Prev;
      DoubleLink* OldANext = PointerA->Next;
      DoubleLink* OldBPrev = PointerB->Prev;
      DoubleLink* OldBNext = PointerB->Next;

      //Update the traversal optimization.
      if(LastReferenced == PointerA)
        LastReferenced = PointerB;
      else if(LastReferenced == PointerB)
        LastReferenced = PointerA;

      //Perform the swap depending on what the condition is.
      if(PointerA->Next == PointerB)
      {
        //Element A is just to the left of B.
        PointerB->Prev = OldAPrev;
        PointerB->Next = PointerA;
        PointerA->Prev = PointerB;
        PointerA->Next = OldBNext;

        //Fix adjacent links.
        if(OldAPrev)
          OldAPrev->Next = PointerB;
        else
          First = PointerB;

        if(OldBNext)
          OldBNext->Prev = PointerA;
        else
          Last = PointerA;
      }
      else if(PointerB->Next == PointerA)
      {
        //Element B is just to the left of A.
        PointerA->Prev = OldBPrev;
        PointerA->Next = PointerB;
        PointerB->Prev = PointerA;
        PointerB->Next = OldANext;

        //Fix adjacent links.
        if(OldBPrev)
          OldBPrev->Next = PointerA;
        else
          First = PointerA;

        if(OldANext)
          OldANext->Prev = PointerB;
        else
          Last = PointerB;
      }
      else
      {
        //There is a positive number of elements between A and B.
        Memory::Swap(PointerA->Prev, PointerB->Prev);
        Memory::Swap(PointerA->Next, PointerB->Next);

        //Fix adjacent links.
        if(OldAPrev)
          OldAPrev->Next = PointerB;
        else
          First = PointerB;

        if(OldANext)
          OldANext->Prev = PointerB;
        else
          Last = PointerB;

        if(OldBPrev)
          OldBPrev->Next = PointerA;
        else
          First = PointerA;

        if(OldBNext)
          OldBNext->Prev = PointerA;
        else
          Last = PointerA;
      }
    }

    ///Finds the i-th element by traversing the list from the beginning.
    T* TraverseFromBeginning(count i) const
    {
      //Test for out of bounds index.
      if(i < 0 or i >= Items)
      {
        LastReferencedIndex = -1;
        LastReferenced = 0;
        return 0;
      }

      //Traverse the list from the beginning.
      DoubleLink* Current = First;
      for(count j = 0; j < Items; j++)
      {
        if(j == i)
        {
          LastReferencedIndex = i;
          LastReferenced = Current;
          return &Current->Data;
        }
        Current = Current->Next;
      }

      //Could not find the element.
      LastReferencedIndex = -1;
      LastReferenced = 0;
      return 0;
    }

    ///Finds the i-th element by traversing the list from the end.
    T* TraverseFromEnd(count i) const
    {
      //Test for out of bounds index.
      if(i < 0 or i >= Items)
      {
        LastReferencedIndex = -1;
        LastReferenced = 0;
        return 0;
      }

      //Traverse the list from the beginning.
      DoubleLink* Current = Last;
      for(count j = Items - 1; j >= 0; j--)
      {
        if(j == i)
        {
          LastReferencedIndex = i;
          LastReferenced = Current;
          return &Current->Data;
        }
        Current = Current->Prev;
      }

      //Could not find the element.
      LastReferencedIndex = -1;
      LastReferenced = 0;
      return 0;
    }

    ///Finds the i-th element traversing from another element of known position.
    T* TraverseFromElement(count i, DoubleLink* Element,
      count ElementIndex) const
    {
      //Test for out of bounds index.
      if(i < 0 or i >= Items or ElementIndex < 0 or
        ElementIndex >= Items or Element == 0)
      {
        LastReferencedIndex = -1;
        LastReferenced = 0;
        return 0;
      }

      if(i == ElementIndex)
      {
        //No traversal necessary.
        LastReferencedIndex = i;
        LastReferenced = Element;
        return &Element->Data;
      }
      else if(i > ElementIndex)
      {
        for(count j = ElementIndex; j <= i; j++)
        {
          //Traverse forward to the element.
          if(j == i)
          {
            LastReferencedIndex = i;
            LastReferenced = Element;
            return &Element->Data;
          }
          Element = Element->Next;
        }
      }
      else
      {
        for(count j = ElementIndex; j >= i; j--)
        {
          //Traverse backwards to the element.
          if(j == i)
          {
            LastReferencedIndex = i;
            LastReferenced = Element;
            return &Element->Data;
          }
          Element = Element->Prev;
        }
      }

      //Could not find element.
      LastReferencedIndex = -1;
      LastReferenced = 0;
      return 0;
    }

  public:

    ///Constructor initializes empty list.
    List<T>() : First(0), Last(0), LastReferenced(0), LastReferencedIndex(0),
      Items(0) {}

    ///Destructor
    ~List<T>()
    {
      //Traverse the list and delete all of the elements.
      DoubleLink* Current = First;
      for(count i = 0; i < Items; i++)
      {
        DoubleLink* Next = Current->Next;
        delete Current;
        Current = Next;
      }
    }

    ///Returns the number of items in the list.
    inline count n() const {return Items;}

    /**Returns a const item by index. If the next item returned is near to this
    index (i.e. sequential iteration), then the traversal time will be
    efficient.*/
    const T& ith(count i) const
    {
      if(LastReferencedIndex > 0)
      {
        if(i < LastReferencedIndex)
        {
          //Determine whether to traverse from the reference element.
          if(i <= LastReferencedIndex / 2)
            return *TraverseFromBeginning(i);
          else
            return *TraverseFromElement(i, LastReferenced, LastReferencedIndex);
        }
        else
        {
          //Determine whether to traverse from the reference element.
          if(i <= Items + LastReferencedIndex / 2)
            return *TraverseFromElement(i, LastReferenced, LastReferencedIndex);
          else
            return *TraverseFromEnd(i);
        }
      }
      else
      {
        //Determine which end has the quickest traversal.
        if(i <= Items / 2)
          return *TraverseFromBeginning(i);
        else
          return *TraverseFromEnd(i);
      }
    }

    ///Returns an item by index.
    T& ith(count i)
    {
      //Using const-cast in order to avoid duplicating the above code.
      return const_cast<T&>(static_cast<const List<T>*>(this)->ith(i));
    }

    ///Gets a const element reference using the familiar bracket notation.
    inline const T& operator [] (count i) const {return ith(i);}

    ///Gets an element reference using the familiar bracket notation.
    inline T& operator [] (count i) {return ith(i);}

    /**Shorthand for getting the first element. Assumes there is at least one
    element in the list.*/
    inline const T& a() const {return First->Data;}

    /**Shorthand for getting the first element. Assumes there is at least one
    element in the list.*/
    inline T& a() {return First->Data;}

    /**Shorthand for getting an element with respect to the end of the list.
    ItemsFromEnd must be nonnegative. Assumes there is at least one element in
    the list.*/
    inline const T& z(count ItemsFromEnd = 0) const
    {
      return ith(n() - 1 - ItemsFromEnd);
    }

    /**Shorthand for getting an element with respect to the end of the list.
    ItemsFromEnd must be nonnegative. Assumes there is at least one element in
    the list.*/
    inline T& z(count ItemsFromEnd = 0) {return ith(n() - 1 - ItemsFromEnd);}

    ///Swaps the position of two elements without copying their internal data.
    void Swap(count i, count j)
    {
      //Check bounds and make sure the elements are not the same.
      if(i == j or i < 0 or j < 0 or i >= Items or j >= Items)
        return;

      //Swap the items.
      SwapElements(ith(i), ith(j));
    }

    ///Appends an element to the end of the list by copying the argument.
    void Append(const T& NewElement)
    {
      DoubleLink* NewLink = new DoubleLink;
      if(not Items)
        Last = First = NewLink;
      else
      {
        NewLink->Prev = Last;
        Last = Last->Next = NewLink;
      }

      NewLink->Data = NewElement;
      Items++;
    }

    /**Pushes an element to the end of the list as though it were a stack. This
    is equivalent to append.*/
    inline void Push(const T& NewElement)
    {
      Append(NewElement);
    }

    ///Adds an element to the list using its default constructor.
    T& Add()
    {
      DoubleLink* NewLink = new DoubleLink;

      if(not Items)
        Last = First = NewLink;
      else
      {
        NewLink->Prev = Last;
        Last = Last->Next = NewLink;
      }

      Items++;

      return z();
    }

    ///Adds an existing element to the list by assigning it to the new element.
    T& Add(const T& x)
    {
      Add() = x;
      return z();
    }

    ///Prepends an element to the beginning of the list.
    void Prepend(const T& NewElement)
    {
      DoubleLink* NewLink = new DoubleLink;

      if(not Items)
        Last = First = NewLink;
      else
      {
        NewLink->Next = First;
        First = First->Prev = NewLink;
      }

      NewLink->Data = NewElement;

      Items++;

      //The reference link must be updated.
      if(LastReferenced)
        LastReferencedIndex++;
    }

    ///Inserts an element before some other element referenced by index.
    void InsertBefore(const T& NewElement, count ElementAfter)
    {
      //Use normal appends if the index is at one of the ends.
      if(ElementAfter <= 0)
      {
        Prepend(NewElement);
        return;
      }
      if(ElementAfter >= Items)
      {
        Append(NewElement);
        return;
      }

      //Traverse the list to update the reference link.
      ith(ElementAfter);

      //Determine the correct links to squeeze the new element between.
      DoubleLink* LeftLink = LastReferenced->Prev;
      DoubleLink* RightLink = LastReferenced;
      DoubleLink* NewLink = new DoubleLink;

      //Update the link pointers.
      RightLink->Prev = LeftLink->Next = NewLink;
      NewLink->Prev = LeftLink;
      NewLink->Next = RightLink;

      //Copy the data.
      NewLink->Data = NewElement;

      //Increment the size.
      Items++;

      //Update the reference link.
      LastReferencedIndex = ElementAfter;
      LastReferenced = NewLink;
    }

    ///Inserts an element after some other element referenced by index.
    void InsertAfter(const T& NewElement, count ElementBefore)
    {
      //Use normal appends if the index is at one of the ends.
      if(ElementBefore < 0)
      {
        Prepend(NewElement);
        return;
      }
      if(ElementBefore >= Items - 1)
      {
        Append(NewElement);
        return;
      }

      //Traverse the list to update the reference link.
      ith(ElementBefore);

      //Determine the correct links to squeeze the new element between.
      DoubleLink* LeftLink = LastReferenced;
      DoubleLink* RightLink = LastReferenced->Next;
      DoubleLink* NewLink = new DoubleLink;

      //Update the link pointers.
      RightLink->Prev = LeftLink->Next = NewLink;
      NewLink->Prev = LeftLink;
      NewLink->Next = RightLink;

      //Copy the data.
      NewLink->Data = NewElement;

      //Increment the size.
      Items++;

      //The reference link is no good anymore.
      LastReferencedIndex = ElementBefore + 1;
      LastReferenced = NewLink;
    }

    ///Removes an item by its index.
    void Remove(count i)
    {
      //If index is out of bounds, no item is removed.
      if(i < 0 or i >= Items)
        return;

      /*Calling ith() will find the element using the reference link instead of
      having to traverse all the way through.*/
      ith(i);

      DoubleLink* LeftLink = LastReferenced->Prev;
      DoubleLink* RightLink = LastReferenced->Next;

      if(not LeftLink and not RightLink)
      {
        Items = 0;
        First = Last = 0;
      }
      else
      {
        if(LeftLink == 0)
          First = RightLink;
        else
          LeftLink->Next = RightLink;

        if(RightLink == 0)
          Last = LeftLink;
        else
          RightLink->Prev = LeftLink;
        Items--;
      }

      delete LastReferenced;

      if(LeftLink != 0)
      {
        LastReferencedIndex = i - 1;
        LastReferenced = LeftLink;
      }
      else if(RightLink != 0)
      {
        LastReferencedIndex = i;
        LastReferenced = RightLink;
      }
      else
      {
        LastReferencedIndex = -1;
        LastReferenced = 0;
      }
    }

    /**Removes an element from the list and calls delete on it. Only use this
    if the element is a pointer to an object on the heap.*/
    void RemoveAndDelete(count i)
    {
      delete ith(i);
      Remove(i);
    }

    /**Pops the element at the end of the list off as though it were a stack.
    This undoes Push().*/
    T Pop()
    {
      T Copy = Nothing<T>();
      if(n())
        Copy = ith(n() - 1);
      Remove(n() - 1);
      return Copy;
    }

    ///Pops off and deletes the element at the end of the list.
    void PopAndDelete()
    {
      RemoveAndDelete(n() - 1);
    }

    /**Removes each element from the list and calls its destructor. Note if the
    element is a pointer to an object instead call RemoveAndDeleteAll to delete
    the pointers automatically.*/
    void RemoveAll()
    {
      while(Items > 0)
        Remove(0);
    }

    /**Deletes all elements and then removes them from the list. Only call this
    if the element is some kind of pointer to an object on the heap.*/
    void RemoveAndDeleteAll()
    {
      while(Items > 0)
      {
        delete ith(0);
        Remove(0);
      }
    }

    ///Removes all elements greater than or equal to the index given.
    void RemoveFrom(count i)
    {
      while(n() and n() > i)
        Pop();
    }

    ///Removes and deletes elements greater or equal to the index given.
    void RemoveAndDeleteFrom(count i)
    {
      while(n() and n() > i)
        PopAndDelete();
    }

    ///Searches for the index of an item. Returns -1 if not found.
    count Search(const T& ItemToSearchFor) const
    {
      for(count i = 0; i < n(); i++)
        if(ith(i) == ItemToSearchFor)
          return i;
      return -1;
    }

    ///Checks to see whether an item exists in the list.
    bool Contains(const T& ItemToSearchFor) const
    {
      return Search(ItemToSearchFor) != -1;
    }

    ///Copy constructor to create deep copy of another list.
    List<T>(const List<T>& Other) : First(0), Last(0), LastReferenced(0),
      LastReferencedIndex(0), Items(0)
    {
      count ItemCount = Other.Items;
      for(count i = 0; i < ItemCount; i++)
        Add() = Other[i];
    }

    ///Assigns this list a deep copy of another list.
    List<T>& operator = (const List<T>& Other)
    {
      //Exit if the other list is the same as this one.
      if(&Other == this)
        return *this;

      //If anything is in this list, remove it first.
      RemoveAll();

      //Copy each item.
      count ItemCount = Other.Items;
      for(count i = 0; i < ItemCount; i++)
        Add() = Other[i];

      return *this;
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
  };
}
#endif
