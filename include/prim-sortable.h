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

#ifndef PRIM_INCLUDE_SORTABLE_H
#define PRIM_INCLUDE_SORTABLE_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///Container primitives with various sorting algorithms
  class Sortable
  {
    public:

    ///Array with sorting facilities
    template <class T, class GM = meta::CustomGrowthModel>
    class Array : public PRIM_NAMESPACE::Array<T, GM>
    {
      ///Private helper that implements Quicksort recursion.
      static void QuicksortRecursive(T* ArrayData, count Left, count Right)
      {
        count Pivot, LeftIndex = Left, RightIndex = Right;
        if(Right - Left <= 0)
          return;
        Pivot = (Left + Right) / 2;
        while(LeftIndex <= Pivot and RightIndex >= Pivot)
        {
          while(ArrayData[LeftIndex] < ArrayData[Pivot] and LeftIndex <= Pivot)
            LeftIndex = LeftIndex + 1;

          while(ArrayData[RightIndex] > ArrayData[Pivot] and
            RightIndex >= Pivot)
               RightIndex = RightIndex - 1;

          T SwapValue = ArrayData[LeftIndex];
          ArrayData[LeftIndex] = ArrayData[RightIndex];
          ArrayData[RightIndex] = SwapValue;
          LeftIndex++;
          RightIndex--;
          if(LeftIndex - 1 == Pivot)
            Pivot = RightIndex = RightIndex + 1;
          else if(RightIndex + 1 == Pivot)
            Pivot = LeftIndex = LeftIndex - 1;
        }
        QuicksortRecursive(ArrayData, Left, Pivot - 1);
        QuicksortRecursive(ArrayData, Pivot + 1, Right);
      }

      public:

      ///Default constructor
      Array() {}

      ///Constructor to initialize with a certain number of elements.
      Array(count Elements) {PRIM_NAMESPACE::Array<T, GM>::n(Elements);}

      ///Runs the Quicksort routine.
      void Quicksort()
      {
        QuicksortRecursive(&PRIM_NAMESPACE::Array<T, GM>::ith(0), 0,
          PRIM_NAMESPACE::Array<T, GM>::n() - 1);
      }

      ///Determines if sorting is necessary and runs the most appropriate sort.
      void Sort()
      {
        if(Array<T, GM>::IsSorted())
          return;

        Quicksort();
      }

      /**Performs a binary search for an item and returns the index if found.
      Note that this depends on the array being sorted beforehand. Otherwise,
      it will fail unpredictably.*/
      count Search(const T& Key) const
      {
        //Precondition: array is sorted before doing binary search.
        count Low = 0;
        count High = PRIM_NAMESPACE::Array<T, GM>::n() - 1;
        while(Low <= High)
        {
          count Mid = (High - Low) / 2 + Low; //Variant of (Low + High) / 2
          const T& MidVal = PRIM_NAMESPACE::Array<T, GM>::ith(Mid);

          if(MidVal < Key)
            Low = Mid + 1;
          else if(MidVal > Key)
            High = Mid - 1;
          else
            return Mid; //Key found
        }
        return -1; //Key not found
      }

      /**Performs a binary search to check that an item exists. Note that this
      depends on the array being sorted beforehand.*/
      bool Contains(const T& Key) const
      {
        return Search(Key) != -1;
      }
    };

    /**Array with sorting facilities whose elements have void Swap(T& Other).
    This form can be used when the elements support a swap that does something
    other than naive value-based swapping.*/
    template <class T, class GM = meta::CustomGrowthModel>
    class SwappableArray : public PRIM_NAMESPACE::Array<T, GM>
    {
      ///Private helper that implements Quicksort recursion.
      static void QuicksortRecursive(T* ArrayData, count Left, count Right)
      {
        count Pivot, LeftIndex = Left, RightIndex = Right;
        if(Right - Left <= 0)
          return;
        Pivot = (Left + Right) / 2;
        while(LeftIndex <= Pivot and RightIndex >= Pivot)
        {
          while(ArrayData[LeftIndex] < ArrayData[Pivot] and LeftIndex <= Pivot)
            LeftIndex = LeftIndex + 1;

          while(ArrayData[RightIndex] > ArrayData[Pivot] and
            RightIndex >= Pivot)
               RightIndex = RightIndex - 1;

          /*In a SwappableArray, instead call a Swap method, which the class
          of the element type can use to speed things up.*/
          ArrayData[LeftIndex].Swap(ArrayData[RightIndex]);

          LeftIndex++;
          RightIndex--;
          if(LeftIndex - 1 == Pivot)
            Pivot = RightIndex = RightIndex + 1;
          else if(RightIndex + 1 == Pivot)
            Pivot = LeftIndex = LeftIndex - 1;
        }
        QuicksortRecursive(ArrayData, Left, Pivot - 1);
        QuicksortRecursive(ArrayData, Pivot + 1, Right);
      }

      public:

      ///Default constructor
      SwappableArray() {}

      ///Constructor to initialize with a certain number of elements
      SwappableArray(count Elements)
      {
        PRIM_NAMESPACE::Array<T, GM>::n(Elements);
      }

      ///Runs the Quicksort routine.
      void Quicksort()
      {
        QuicksortRecursive(&PRIM_NAMESPACE::Array<T, GM>::ith(0), 0,
          PRIM_NAMESPACE::Array<T, GM>::n() - 1);
      }

      ///Determines if sorting is necessary and runs the most appropriate sort.
      void Sort()
      {
        if(Array<T, GM>::IsSorted())
          return;

        Quicksort();
      }

      /**Performs a binary search for an item and returns the index if found.
      Note that this depends on the array being sorted beforehand. Otherwise,
      it will fail unpredictably.*/
      count Search(const T& Key) const
      {
        //Precondition: array is sorted before doing binary search.
        count Low = 0;
        count High = PRIM_NAMESPACE::Array<T, GM>::n() - 1;
        while(Low <= High)
        {
          count Mid = (High - Low) / 2 + Low; //Variant of (Low + High) / 2
          const T& MidVal = PRIM_NAMESPACE::Array<T, GM>::ith(Mid);

          if(MidVal < Key)
            Low = Mid + 1;
          else if(MidVal > Key)
            High = Mid - 1;
          else
            return Mid; //Key found
        }
        return -1; //Key not found
      }

      /**Performs a binary search to check that an item exists. Note that this
      depends on the array being sorted beforehand.*/
      bool Contains(const T& Key) const
      {
        return Search(Key) != -1;
      }
    };

    ///List with sorting facilities.
    template <class T>
    class List : public PRIM_NAMESPACE::List<T>
    {
      /**Internal data structure for Quicksort recursing. Since the Quicksort
      algorithm is massively recursive it overflows stack space very quickly. To
      cope with this inherent problem a non-recursing algorithm was implemented
      utilizing a heap-based stack. Keep in mind that for lists in the
      hundred-thousands of elements, the algorithm can quickly consume around
      10 or 20 megabytes.*/
      class QuicksortStack
      {
        public:

        typename PRIM_NAMESPACE::List<T>::DoubleLink* Left;
        typename PRIM_NAMESPACE::List<T>::DoubleLink* Right;
        typename PRIM_NAMESPACE::List<T>::DoubleLink* Start;
        typename PRIM_NAMESPACE::List<T>::DoubleLink* Current;
        typename PRIM_NAMESPACE::List<T>::DoubleLink* OldCurrent;
        count Control;
        QuicksortStack* Next;

        ///Pushes a state onto the stack.
        void Push(QuicksortStack*& VarHead,
          typename PRIM_NAMESPACE::List<T>::DoubleLink* VarLeft,
          typename PRIM_NAMESPACE::List<T>::DoubleLink* VarRight,
          typename PRIM_NAMESPACE::List<T>::DoubleLink* VarStart,
          typename PRIM_NAMESPACE::List<T>::DoubleLink* VarCurrent,
          typename PRIM_NAMESPACE::List<T>::DoubleLink* VarOldCurrent,
          count VarControl)
        {
          Next = VarHead;
          VarHead = this;
          Left = VarLeft;
          Right = VarRight;
          Start = VarStart;
          Current = VarCurrent;
          OldCurrent = VarOldCurrent;
          Control = VarControl;
        }

        ///Pops the most recent state off the stack.
        QuicksortStack* Pop(QuicksortStack*& VarHead,
          typename PRIM_NAMESPACE::List<T>::DoubleLink*& VarLeft,
          typename PRIM_NAMESPACE::List<T>::DoubleLink*& VarRight,
          typename PRIM_NAMESPACE::List<T>::DoubleLink*& VarStart,
          typename PRIM_NAMESPACE::List<T>::DoubleLink*& VarCurrent,
          typename PRIM_NAMESPACE::List<T>::DoubleLink*& VarOldCurrent,
          count& VarControl)
        {
          VarLeft = VarHead->Left;
          VarRight = VarHead->Right;
          VarStart = VarHead->Start;
          VarCurrent = VarHead->Current;
          VarOldCurrent = VarHead->OldCurrent;
          VarControl = VarHead->Control;
          QuicksortStack* OldHead = VarHead;
          VarHead = VarHead->Next;
          return OldHead;
        }

        ///Code control types that gate execution to one of three given paths.
        enum ControlType
        {
          FirstBranch,
          SecondBranch,
          ThirdBranch
        };

        ///Zeroing constructor
        QuicksortStack() : Left(0), Right(0), Start(0), Current(0),
          OldCurrent(0), Control(0), Next(0) {}
      };

      public:

      ///Sorts the list using the Quicksort algorithm.
      void Quicksort(bool Ascending = true)
      {
        //Ensure there are at least two elements in the list.
        if(PRIM_NAMESPACE::List<T>::Items < 2)
          return;

        //Stack state variables
        typename PRIM_NAMESPACE::List<T>::DoubleLink* Left =
          PRIM_NAMESPACE::List<T>::First;
        typename PRIM_NAMESPACE::List<T>::DoubleLink* Right =
          PRIM_NAMESPACE::List<T>::Last;
        typename PRIM_NAMESPACE::List<T>::DoubleLink* Start = 0;
        typename PRIM_NAMESPACE::List<T>::DoubleLink* Current = 0;
        typename PRIM_NAMESPACE::List<T>::DoubleLink* OldCurrent = 0;
        count Control = QuicksortStack::FirstBranch;

        //Initialize stack state.
        QuicksortStack* Head = 0;

        //Start the control loop.
        do
        {
          switch(Control)
          {
            case QuicksortStack::FirstBranch:
            //Set the Start and the Current item pointers.
            Start = Left;
            Current = Left; //This is the algorithm's pivot.

            //Loop until we encounter the right pointer.
            while(Current != Right)
            {
              //Move to the next item in the list.
              Current = Current->Next;

              //Swap items if they are out of order.
              if(Ascending and Start->Data < Current->Data)
                PRIM_NAMESPACE::Swap(Current->Data, Start->Data);
              else if(not Ascending and Start->Data > Current->Data)
                PRIM_NAMESPACE::Swap(Current->Data, Start->Data);
            }

            //Swap the first and current items.
            PRIM_NAMESPACE::Swap(Left->Data, Current->Data);

            //Save current item.
            OldCurrent = Current;

            //Check if the left-hand side of the current point needs sorting.
            Current = OldCurrent->Prev;

            if(Current and Left->Prev != Current and Current->Next != Left and
              Left != Current)
            {
              Control = QuicksortStack::SecondBranch;
              //---------------SAVE----------------
              //Save the stack state.
              QuicksortStack* NewStackItem = new QuicksortStack;
              NewStackItem->Push(Head, Left, Right, Start, Current,
                OldCurrent, Control);
              //-----------------------------------
              Right = Current;
              Start = 0;
              Current = 0;
              OldCurrent = 0;
              Control = QuicksortStack::FirstBranch;
              break;
            }
            else
            {
              Control = QuicksortStack::SecondBranch;
              break;
            }

            case QuicksortStack::SecondBranch:
            //Check if the right hand side of the current point needs sorting.
            Current = OldCurrent->Next;
            if(Current and Current->Prev != Right and Right->Next != Current
              and Current != Right)
            {
              Control = QuicksortStack::ThirdBranch;
              //---------------SAVE----------------
              //Save the stack state.
              QuicksortStack* NewStackItem = new QuicksortStack;
              NewStackItem->Push(Head, Left, Right, Start, Current,
                OldCurrent, Control);
              //-----------------------------------
              Left = Current;
              Start = 0;
              Current = 0;
              OldCurrent = 0;
              Control = QuicksortStack::FirstBranch;
              break;
            }
            else
            {
              Control = QuicksortStack::ThirdBranch;
              break;
            }

            case QuicksortStack::ThirdBranch:
            if(Head) //If no head, then the sort is over anyway.
            {
              //--------------REVERT---------------
              //Revert to the previous stack state.
              QuicksortStack* StackToDelete = Head->Pop(Head, Left, Right,
                Start, Current, OldCurrent, Control);
              delete StackToDelete;
              //-----------------------------------
            }
            break;

            default: //Default case will never occur.
              break;
          }
        } while(Head);
      }

      ///Bubble-sorts the list in either ascending or descending order.
      void BubbleSort(bool Ascending = true)
      {
        if(Ascending)
        {
          for(count i = 0; i < PRIM_NAMESPACE::List<T>::Items - 1; i++)
            for(count j = i + 1; j < PRIM_NAMESPACE::List<T>::Items; j++)
              if(PRIM_NAMESPACE::List<T>::ith(i) >
                PRIM_NAMESPACE::List<T>::ith(j))
                  PRIM_NAMESPACE::Swap(PRIM_NAMESPACE::List<T>::ith(i),
                    PRIM_NAMESPACE::List<T>::ith(j));
        }
        else
        {
          for(count i = 0; i < PRIM_NAMESPACE::List<T>::Items - 1; i++)
            for(count j = i + 1; j < PRIM_NAMESPACE::List<T>::Items; j++)
              if(PRIM_NAMESPACE::List<T>::ith(i) <
                PRIM_NAMESPACE::List<T>::ith(j))
                  PRIM_NAMESPACE::Swap(PRIM_NAMESPACE::List<T>::ith(i),
                    PRIM_NAMESPACE::List<T>::ith(j));
        }
      }

      ///Returns whether or not the list is sorted.
      bool IsSorted(bool Ascending = true) const
      {
        if(Ascending)
        {
          for(count i = 0, n = PRIM_NAMESPACE::List<T>::n() - 1; i < n; i++)
            if(PRIM_NAMESPACE::List<T>::ith(i) >
              PRIM_NAMESPACE::List<T>::ith(i + 1))
                return false;
        }
        else
        {
          for(count i = 0, n = PRIM_NAMESPACE::List<T>::n() - 1; i < n; i++)
            if(PRIM_NAMESPACE::List<T>::ith(i) <
              PRIM_NAMESPACE::List<T>::ith(i + 1))
                return false;
        }
        return true;
      }

      /**Sorts using one of two algorithms. For smaller lists, BubbleSort is
      used in conjunction with link-pointer swapping. For bigger lists, the
      Quicksort algorithm is used with data copying.*/
      void Sort(bool Ascending = true)
      {
        if(IsSorted(Ascending))
          return;
        if(PRIM_NAMESPACE::List<T>::Items < 50)
          BubbleSort(Ascending);
        else
          Quicksort(Ascending);
      }
    };
  };
}
#endif
