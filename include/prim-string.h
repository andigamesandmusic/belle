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

#ifndef PRIM_INCLUDE_STRING_H
#define PRIM_INCLUDE_STRING_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  namespace meta
  {
    ///Character used for when a bad Unicode character sequence is decoded.
    static const unicode BadCharacter = 0xfffd; //Unicode replacement character
  }

  /**Efficient flat container of linked substrings. The string is equally fast
  at append, prepend, insert, and erase, with nearly constant manipulation
  speed with respect to string length.*/
  class String
  {
    public:

    /**Representation of a UTF-32 string stored in system endianness. Since it
    is just an array of unicode values, it does not have any string operation
    methods. To use those methods, append the String::UTF32 to an empty
    string.*/
    typedef Array<unicode> UTF32;

    private:

    ///Container of linked-substrings.
    mutable Array<byte> Data;

    ///Total length of the combined fragments.
    mutable count InternalLength;

    ///Starting position of the last fragment.
    mutable count LastFragmentIndex;

    ///Precision of number to string conversions.
    mutable count NumberPrecision;

    public:

    ///Standard streams that can be attached to the string.
    enum StreamAttachment
    {
      StandardInput,
      StandardOutput,
      StandardError,
      NotAttached
    };

    private:

    ///Stream to which this string is attached
    StreamAttachment AttachedStream; PRIM_PAD(StreamAttachment)

    //--------//
    //Iterator//
    //--------//

    //Iterator needs to access the Data member.
    friend class Iterator;

    ///Internal iterator used by the string manipulator methods.
    class Iterator
    {
      public:

      ///String character index at the fragment beginning
      count i;

      ///Index of the fragment data
      count f;

      ///Length of the fragment
      count n;

      ///Location of the previous next-marker
      count p;

      ///Delay mechanism so that copied iterators will begin correctly.
      bool DelayIterator; PRIM_PAD(bool)

      ///Resets the iterator to the beginning of the string
      void Reset()
      {
        i = 0;
        f = 1;
        n = 0;
        p = 1;
        DelayIterator = false;
      }

      ///Constructor from integer to initialize from default parameter.
      Iterator()
      {
        Reset();
      }

      ///Copy constructor with delay mechanism turned on
      Iterator(const Iterator& Other) : i(Other.i), f(Other.f), n(Other.n),
        DelayIterator(true) {}

      ///Assignment operator with delay mechanism turned on
      Iterator& operator = (const Iterator& Other)
      {
        i = Other.i;
        f = Other.f;
        n = Other.n;
        DelayIterator = true;
        return *this;
      }

      ///Hops to the next fragment.
      count Next(const String* s)
      {
        //If delaying hold off for one iteration if the fragment is valid.
        if(DelayIterator and f != 1)
        {
          DelayIterator = false;
          return -1;
        }

        //If starting go to the first fragment.
        if(f == 1)
        {
          i = 0;
          f = count(s->ReadMarker(1));
          n = count(s->ReadMarker(f));
          return f;
        }

        //If the end of the string is reached just leave.
        if(n == 0)
          return 0;

        //Increase string index.
        i += n;

        //Determine next fragment length.
        p = f + 4 + n;
        f = count(s->ReadMarker(p));
        if(not f)
          n = 0;
        else
          n = count(s->ReadMarker(f));

        return f;
      }

      ///Checks whether the requested character index is in this fragment.
      inline bool Contains(count j)
      {
        return j >= i and j < i + n;
      }
    };

    ///The default iterator is used for Get() operations.
    mutable Iterator DefaultIterator;

    //-------------------//
    //Character Retrieval//
    //-------------------//

    ///Gets a character at the specified index.
    inline byte Get(count i) const
    {
      //If index is out of bounds return null.
      if(i < 0 or i >= InternalLength)
        return 0;

      //If the iterator is not useful then reset it.
      if(i < DefaultIterator.i)
        DefaultIterator.Reset();

      //Start iterating on the current fragment.
      DefaultIterator.DelayIterator = true;
      while(DefaultIterator.Next(this))
      {
        if(i < DefaultIterator.i + DefaultIterator.n)
          return Data[DefaultIterator.f + 4 + (i - DefaultIterator.i)];
      }
      return 0;
    }

    //--------------//
    //Marker Control//
    //--------------//

    ///Reads a marker at some index.
    inline uint32 ReadMarker(count Index) const
    {
#ifdef PRIM_DEBUG_INTERNAL
      //Sanity check--debug only.
      if(Index < 0 or Index + 4 > Data.n())
      {
        PrintData();
        std::cout << "The requested marker position is bad." << std::endl;
        std::cout.flush();
      }
#endif
      uint32 v = 0; Memory::MemCopy(&v, &Data[Index], sizeof(v)); return v;
    }

    ///Writes a marker value at some index.
    inline void WriteMarker(count Index, uint32 Value)
    {
#ifdef PRIM_DEBUG_INTERNAL
      //Sanity check--debug only.
      if(Index < 0 or Index + 4 > Data.n())
      {
        PrintData();
        std::cout << "The requested marker position is bad." << std::endl;
        std::cout.flush();
      }
#endif
      Memory::MemCopy(&Data[Index], &Value, sizeof(Value));
    }

    ///Writes a fragment assuming space for it has been allocated.
    void WriteFragment(count Index, count Length, const byte* Fragment,
      count Next)
    {
      WriteMarker(Index, uint32(Length));
      WriteMarker(Index + 4 + Length, uint32(Next));
      Memory::Copy(&Data[Index + 4], Fragment, Length);
    }

    //------------//
    //Optimization//
    //------------//

    /**Ensures that the string will never exceed twice its length in its
    representation. For example, this happens when a lot of short strings are
    inserted causing the majority of the data to be link data.*/
    void CollapseBloatedLinks()
    {
      if(Data.n() > InternalLength * 2)
        Merge();
    }

    //-----//
    //Debug//
    //-----//

    ///Binary dump of current string array data. Debug use only.
    void PrintData() const
    {
#ifdef PRIM_DEBUG_INTERNAL
      std::cout << std::endl << "BEGIN (ROWS CONTAIN 16 BYTES)" << std::endl;
      std::cout << "0    1    2    3    4    5    6    7    8"
        "    9    10   11   12   13   14   15" << std::endl;
      std::cout << "=----=----=----=----=----=----=----=----="
        "----=----==---==---==---==---==---==---";

      for(count i = 0; i < Data.n(); i++)
      {
        if(i % (80 / 5) == 0)
          std::cout << std::endl;

        byte c = Data[i];

        if(c >= byte(' ') and c <= byte('~'))
          std::cout << ascii(c);
        else
          std::cout << " ";

        if(c < 10)
          std::cout << "  " << int(c);
        else if(c < 100)
          std::cout << " " << int(c);
        else
          std::cout << "" << int(c);

        std::cout << " ";
      }
      std::cout << std::endl;
      std::cout << "LAST FRAGMENT INDEX: " << LastFragmentIndex << std::endl;
      std::cout << "INTERNAL LENGTH: " << InternalLength << std::endl;
      std::cout << "END" << std::endl;
#endif
    }

    ///Checks the internal consistency of the string. Debug use only.
    bool CheckConsistency()
    {
#ifdef PRIM_DEBUG_INTERNAL
      //Check empty string.
      if(InternalLength == 0)
      {
        if(Data.n() != 0 or LastFragmentIndex != 0)
        {
          std::cout << "Bad empty string" << std::endl; std::cout.flush();
          PrintData();
          return false;
        }
      }

      if(Data.n() == 0)
      {
        if(InternalLength != 0 or LastFragmentIndex != 0)
        {
          std::cout << "Bad empty string" << std::endl; std::cout.flush();
          PrintData();
          return false;
        }
      }

      if(LastFragmentIndex == 0)
      {
        if(InternalLength != 0 or LastFragmentIndex != 0)
        {
          std::cout << "Bad empty string" << std::endl; std::cout.flush();
          PrintData();
          return false;
        }
        else
          return true;
      }

      if(Data[0] != 0)
      {
        std::cout << "Bad null boundary character" << std::endl;
        std::cout.flush();
        PrintData();
        return false;
      }

      Iterator q;
      count ActualLength = 0;
      count LastF = 0;
      while(q.Next(this))
      {
        LastF = q.f;
        ActualLength += q.n;
      }

      if(ActualLength != InternalLength)
      {
        std::cout << "Bad internal length (expected " << ActualLength <<
          ")" << std::endl; std::cout.flush();
        PrintData();
        return false;
      }

      if(LastF != LastFragmentIndex)
      {
        std::cout << "Bad last fragment index (expected " << LastF <<
          ")" << std::endl; std::cout.flush();
        PrintData();
        return false;
      }
#endif
      return true;
    }

    public:

    ///Represents an interval of characters in a string by index.
    typedef Complex<count> Span;

    //----------------//
    //Number Precision//
    //----------------//

    ///Sets the current floating-point conversion precision.
    void Precision(count NewPrecision)
    {
      if(NewPrecision < 1)
        NewPrecision = 1;
      else if(NewPrecision > 18)
        NewPrecision = 18;
      NumberPrecision = NewPrecision;
    }

    ///Gets the current floating-point conversion precision.
    count Precision(void)
    {
      return NumberPrecision;
    }

    //------------------------//
    //Fundamental Manipulators//
    //------------------------//

    ///Appends a byte fragment of a certain length.
    void Append(const byte* Fragment, count Length)
    {
      //If there is no content to add, then just leave.
      if(Length <= 0 or not Fragment)
        return;

      AppendToStream(Fragment, Length);

      if(InternalLength == 0)
      {
        //Create first fragment of initially empty string.
        Data.n(1 + 4 + 4 + Length + 4);
        Data[0] = 0;
        WriteMarker(1, 5);
        WriteFragment(5, Length, Fragment, 0);
        LastFragmentIndex = 5;
      }
      else if(LastFragmentIndex + 4 + count(ReadMarker(LastFragmentIndex)) +
        4 == Data.n())
      {
        //The last fragment is at the end, so it can be extended.
        Data.n(Data.n() + Length);
        count OldFragmentLength = count(ReadMarker(LastFragmentIndex));
        count NewFragmentLength = OldFragmentLength + Length;
        WriteMarker(LastFragmentIndex, uint32(NewFragmentLength));
        Memory::Copy(&Data[LastFragmentIndex + 4 + OldFragmentLength],
          Fragment, Length);
        WriteMarker(LastFragmentIndex + 4 + NewFragmentLength, 0);
      }
      else
      {
        //The last fragment not at the end, so a new fragment must be created.
        count OldStringLength = Data.n();
        Data.n(OldStringLength + 4 + Length + 4);
        WriteMarker(LastFragmentIndex + 4 +
          count(ReadMarker(LastFragmentIndex)), uint32(OldStringLength));
        WriteFragment(OldStringLength, Length, Fragment, 0);
        LastFragmentIndex = OldStringLength;
      }

      //Increase the internal length.
      InternalLength += Length;

      //Reset the default iterator.
      DefaultIterator.Reset();

      //Optimize links.
      CollapseBloatedLinks();
    }

    ///Appends a number with a given precision and format.
    void Append(float64 v, count Precision, bool ScientificNotation);

    ///Prepends a byte string of a certain length.
    void Prepend(const byte* Fragment, count Length)
    {
      //If there is no content to add, then just leave.
      if(Length <= 0 or not Fragment)
        return;

      /*Prepend is equivalent to Append for length zero string, so use the
      simpler Append.*/
      if(InternalLength == 0)
      {
        Append(Fragment, Length);
        return;
      }

      //Add a new fragment containing the prepended string.
      uint32 OriginalFirstFragment = ReadMarker(1);
      count OldStringLength = Data.n();
      Data.n(OldStringLength + Length + 4 + 4);
      WriteFragment(OldStringLength, Length, Fragment,
        count(OriginalFirstFragment));
      WriteMarker(1, uint32(OldStringLength));
      InternalLength += Length;

      //Reset the default iterator.
      DefaultIterator.Reset();

      //Optimize string.
      CollapseBloatedLinks();
    }

    /**Combines all the string fragments to form a single contiguous string. It
    returns a pointer to the byte or ASCII data. The end of the string will
    always contain a null-terminator regardless of the string contents. Note
    that Merge is declared const because it only changes the representation of
    the content of the string, not the content itself.*/
    const ascii* Merge() const
    {
      /*#voodoo This is a little bit spurious, but it prevents having to
      allocate array space on the heap for a null string. Since InternalLength
      is zero, the zero-length ascii* null-terminated string can be set here.*/
      if(InternalLength == 0)
        return reinterpret_cast<const ascii*>(&InternalLength);

      /*If already merged, do not create a new copy. This is determined by
      checking whether the string takes up the smallest possible space.*/
      if(Data.n() == 1 + 4 + 4 + InternalLength + 4)
        return reinterpret_cast<ascii*>(&Data[1 + 4 + 4]);

      //Create an array to the flattened string data.
      Array<byte> FlatData;
      FlatData.n(1 + 4 + 4 + InternalLength + 4);
      FlatData[0] = 0;

      //Alignment-safe copy
      {uint32 v = 1 + 4;
        Memory::MemCopy(&FlatData[1], &v, sizeof(v));}
      {uint32 v = uint32(InternalLength);
        Memory::MemCopy(&FlatData[5], &v, sizeof(v));}
      {uint32 v = 0;
        Memory::MemCopy(&FlatData[1 + 4 + 4 + InternalLength], &v, sizeof(v));}

      //Location of flattened string in data
      byte* NewString = &FlatData[1 + 4 + 4];

      //Iterate through fragments and copy substrings one by one.
      Iterator q;
      while(q.Next(this))
        Memory::Copy(&NewString[q.i], &Data[q.f + 4], q.n);

      //Swap array data so that the old data goes out of scope.
      Data.SwapWith(FlatData);
      LastFragmentIndex = 1 + 4;

      //Reset the default iterator.
      DefaultIterator.Reset();

      //Return pointer to new string.
      return reinterpret_cast<ascii*>(&Data[1 + 4 + 4]);
    }

    ///Inserts a fragment before the character at the index.
    void Insert(const byte* Fragment, count Length, count IndexBefore)
    {
      //If there is no content to add, then just leave.
      if(Length <= 0 or not Fragment)
        return;

      //If index is 0 or below, then this is a prepend.
      if(IndexBefore <= 0)
      {
        Prepend(Fragment, Length);
        return;
      }

      //If index is n or above, then this is an append.
      if(IndexBefore >= InternalLength)
      {
        Append(Fragment, Length);
        return;
      }

      //Add length of incoming fragment to length of total string.
      InternalLength += Length;
      Iterator q;
      while(q.Next(this))
      {
        if(q.i + q.n == IndexBefore)
        {
          //Inserting in between two fragments.
          count OldLength = Data.n();
          Data.n(OldLength + 4 + Length + 4);
          count NextIndex = q.f + 4 + q.n;
          count RightFragment = count(ReadMarker(NextIndex));
          WriteFragment(OldLength, Length, Fragment, RightFragment);
          WriteMarker(NextIndex, uint32(OldLength));
          break;
        }
        else if(q.Contains(IndexBefore))
        {
          count IndexInFragment = IndexBefore - q.i;
          count OldLength = Data.n();

          /*There will always be at least one character on the left, since the
          above case takes care of fragment insert.*/

          //-----//
          //Cases//
          //-----//

          //1) Partition (large enough to hold NEXTLENG and 1 char each side)
          if(q.n - IndexInFragment > 8)
          {
            count SquashStart = q.f + 4 + IndexInFragment;
            uint32 SquashedCharacters[2] = {ReadMarker(SquashStart),
              ReadMarker(SquashStart + 4)};
            Data.n(OldLength + 4 + (4 + 4) + Length + 4);

            //Write fragment A length.
            WriteMarker(q.f, uint32(IndexInFragment));

            //Write fragment A pointer.
            WriteMarker(SquashStart, uint32(OldLength));

            //Write fragment C length (pointer does not change).
            WriteMarker(SquashStart + 4,
              uint32(q.n - IndexInFragment - (4 + 4)));

            //Write fragment B length.
            WriteMarker(OldLength, uint32(Length + (4 + 4)));

            //Copy fragment B data.
            Memory::Copy(&Data[OldLength + 4], Fragment, Length);

            //Copy fragment B first squash part data.
            WriteMarker(OldLength + 4 + Length, SquashedCharacters[0]);

            //Copy fragment B second squash part data.
            WriteMarker(OldLength + 4 + Length + 4, SquashedCharacters[1]);

            //Write fragment B pointer.
            WriteMarker(OldLength + 4 + Length + 4 + 4,
              uint32(SquashStart + 4));

            //Update last fragment index if necessary.
            if(LastFragmentIndex == q.f)
              LastFragmentIndex = SquashStart + 4;
          }

          /*2) Total absorb and collapse. There is no space for pointer
          redirection in the existing fragment.*/
          else if(q.n < 1 + (4 + 4) + 1)
          {
            //Precalculate some constants.
            count LeftSideLength = IndexBefore - q.i;
            count RightSideLength = q.n - LeftSideLength;
            count NewFragmentLength = LeftSideLength + Length + RightSideLength;

            //Resize the data array to make room for the new fragment.
            Data.n(OldLength + 4 + LeftSideLength +
              Length + RightSideLength + 4);

            //Point previous fragment to new fragment.
            WriteMarker(q.p, uint32(OldLength));

            //Length of new conflated fragment.
            WriteMarker(OldLength, uint32(NewFragmentLength));

            //Subfragment 1 (left) copy
            Memory::Copy(&Data[OldLength + 4], &Data[q.f + 4], LeftSideLength);

            //Subfragment 2 (incoming) copy
            Memory::Copy(&Data[OldLength + 4 + LeftSideLength],
              Fragment, Length);

            //Subfragment 3 (right) copy
            Memory::Copy(&Data[OldLength + 4 + LeftSideLength + Length],
              &Data[q.f + 4 + LeftSideLength], RightSideLength);

            //Pointer to next fragment
            WriteMarker(OldLength + 4 + NewFragmentLength,
              ReadMarker(q.f + 4 + q.n));

            //Clear out original fragment.
            Memory::Clear(&Data[q.f], q.n + 8);

            //Update last fragment index.
            if(LastFragmentIndex == q.f)
              LastFragmentIndex = OldLength;

            //Update iterator.
            q.f = OldLength;
            q.n = NewFragmentLength;
          }

          //3) Right-side absorb
          else
          {
            //Precalculate some constants.
            count EightCharAbsorbStart = q.f + 4 + q.n - 8;
            count LeftSideLength = (IndexBefore - q.i) - (q.n - 8);
            count RightSideLength = 8 - LeftSideLength;
            count NewFragmentLength = LeftSideLength + Length + RightSideLength;

            //Resize the data array to make room for the new fragment.
            Data.n(OldLength + 4 + LeftSideLength +
              Length + RightSideLength + 4);

            //Length of new amalgamated fragment.
            WriteMarker(OldLength, uint32(NewFragmentLength));

            //Subfragment 1 (left) copy
            Memory::Copy(&Data[OldLength + 4],
              &Data[EightCharAbsorbStart], LeftSideLength);

            //Subfragment 2 (incoming) copy
            Memory::Copy(&Data[OldLength + 4 + LeftSideLength],
              Fragment, Length);

            //Subfragment 3 (right) copy
            Memory::Copy(&Data[OldLength + 4 + LeftSideLength + Length],
              &Data[EightCharAbsorbStart + LeftSideLength], RightSideLength);

            //Pointer to next fragment
            WriteMarker(OldLength + 4 + NewFragmentLength,
              ReadMarker(q.f + 4 + q.n));

            //Pointer to new fragment from old fragment
            WriteMarker(EightCharAbsorbStart, uint32(OldLength));

            //Update old fragment length.
            WriteMarker(q.f, uint32(EightCharAbsorbStart - (q.f + 4)));

            //Clear out original fragment.
            Memory::Clear(&Data[EightCharAbsorbStart + 4], 8);

            //Update last fragment index.
            if(LastFragmentIndex == q.f)
              LastFragmentIndex = OldLength;

            //Update iterator.
            q.n = count(ReadMarker(q.f));
          }

          //Insert is finished.
          break;
        }
      }

      //Reset the default iterator.
      DefaultIterator.Reset();
    }

    /**Erases any characters that exist from i to j. If j < i, then no
    characters are erased.*/
    void Erase(count i, count j)
    {
      Iterator q;

      //If no characters to erase, then leave.
      if(not InternalLength)
        return;

      //If j < i, then no characters should be erased.
      if(j < i)
        return;

      //If i >= n or j < 0 then no characters need to be erased.
      if(i >= InternalLength or j < 0)
        return;

      //If i is out of bounds, then bring it back in bounds.
      if(i < 0)
        i = 0;

      //If j is out of bounds, then bring it back in bounds.
      if(j >= InternalLength)
        j = InternalLength - 1;

      //Calculate number of characters to erase.
      count EraseLength = j - i + 1;

      //If erasing the whole string, just clear it.
      if(EraseLength == InternalLength)
      {
        Clear();
        return;
      }

      //Subtract length of erased portion.
      InternalLength -= ((j + 1) - i);

      //Tracks the last seen fragment for last fragment index update.
      Iterator LastSeenFragment = q;

      while(q.Next(this))
      {
        //Get the left and right boundaries of the fragment.
        count l = q.i, r = q.i + q.n - 1;

        if(r < i)
        {
          //The iterator has not hit an erasure point yet.
          LastSeenFragment = q;
        }
        else if(l > j)
        {
          /*The iterator is past the erasure point, so the iterator may stop.
          Also, since this means the last fragment was not touched, the last
          fragment index does not change either.*/
          break;
        }
        else if(i <= l and j >= r)
        {
          //The entire fragment may be erased.

          //Calculate the previous pointer position.
          count PrevPointerPosition;
          if(LastSeenFragment.f != 1)
          {
            PrevPointerPosition = LastSeenFragment.f + 4 +
              count(ReadMarker(LastSeenFragment.f));
          }
          else
            PrevPointerPosition = 1;

          //Update previous pointer.
          WriteMarker(PrevPointerPosition, ReadMarker(q.f + 4 + q.n));

          //Do not clear the next pointer since iteration must continue.
          Memory::Clear(&Data[q.f], 4 + q.n);

          //If the last fragment was cleared, update the last fragment index.
          if(q.f == LastFragmentIndex)
            LastFragmentIndex = LastSeenFragment.f;
        }
        else if(i <= l and (j >= l and j < r))
        {
          //Perform a left trim.

          //Make a new length.
          count NewFragmentPosition = q.f + (j + 1 - q.i);
          WriteMarker(NewFragmentPosition, uint32(q.n - (j + 1 - q.i)));

          //Calculate the previous pointer position.
          count PrevPointerPosition;
          if(LastSeenFragment.f != 1)
          {
            PrevPointerPosition = LastSeenFragment.f + 4 +
              count(ReadMarker(LastSeenFragment.f));
          }
          else
            PrevPointerPosition = 1;

          //Update previous pointer.
          WriteMarker(PrevPointerPosition, uint32(NewFragmentPosition));

          //If the last fragment was cleared, update the last fragment index.
          if(q.f == LastFragmentIndex)
            LastFragmentIndex = NewFragmentPosition;

          //By definition the erasing is over if left trim was used.
          break;
        }
        else if(j >= r and (i > l and i <= r))
        {
          //Perform a right trim.

          //Move next pointer down.
          count FragmentALength = i - q.i;
          WriteMarker(q.f + 4 + FragmentALength, ReadMarker(q.f + 4 + q.n));

          /*Update iterator so it does not break when it looks for the pointer,
          and also adjust bounds of erasure so erase does not overshoot.*/
          j -= q.n - FragmentALength;
          q.n = FragmentALength;

          //Adjust length.
          WriteMarker(q.f, uint32(FragmentALength));

          //Remember that this fragment was kept.
          LastSeenFragment = q;
        }
        else
        {
          /*Excise within fragment. There are three cases:
              1) Large excise, in middle of long fragment
              2) Small excise, in middle of long fragment
              3) Small excise near right edge of long fragment*/
          if(EraseLength < (4 + 4))
          {
            //Small excise
            if((i - q.i) + (4 + 4) < q.n)
            {
              //In middle: create new fragment for squashed characters.
              count SquashedLength = (4 + 4) - EraseLength;
              count SquashedStart = q.f + 4 + (j + 1 - q.i);
              count NewFragmentStart = Data.n();
              count FragmentALength = i - q.i;
              count FragmentBStart = q.f + 4 + FragmentALength + 4;
              count FragmentBLength = q.n - FragmentALength - (4 + 4);

              //Create a new fragment for the squashed characters.
              Data.n(NewFragmentStart + 4 + SquashedLength + 4);
              WriteMarker(NewFragmentStart, uint32(SquashedLength));
              Memory::Copy(&Data[NewFragmentStart + 4], &Data[SquashedStart],
                SquashedLength);
              WriteMarker(NewFragmentStart + 4 + SquashedLength,
                uint32(FragmentBStart));

              //Update fragment A length.
              WriteMarker(q.f, uint32(FragmentALength));

              //Point fragment A to the squashed fragment.
              WriteMarker(q.f + 4 + FragmentALength, uint32(NewFragmentStart));

              //Set fragment B length.
              WriteMarker(q.f + 4 + FragmentALength + 4,
                uint32(FragmentBLength));

              //If the last fragment was altered, update last fragment index.
              if(q.f == LastFragmentIndex)
                LastFragmentIndex = FragmentBStart;
            }
            else
            {
              //Near right edge: shift characters down including pointer.
              count ShiftStart = q.f + 4 + (i - q.i);
              count ShiftDistance = EraseLength;
              count ShiftSize = q.n - (j + 1 - q.i) + 4;
              for(count k = ShiftStart; k < ShiftStart + ShiftSize; k++)
                Data[k] = Data[k + ShiftDistance];

              //Update the fragment length.
              WriteMarker(q.f, uint32(q.n - EraseLength));

              /*Since fragment beginnings were not created or altered, there is
              no need to update the last fragment index.*/
            }
          }
          else
          {
            /*Note that it is implied that since this is not a fragment trim
            operation, the maximum value of 'j' is the position of the second-
            to-last character of the fragment. Thus, with an erase length of
            at least 4 + 4, there will be enough room in the erasure space to
            create a new length-pointer marker.*/

            /*Large excise using pointers to skip erased section. Fragment is
            split into two fragments A and B using erased character space for
            the length-pointer markers.*/

            //Adjust fragment A length.
            count FragmentALength = i - q.i;
            WriteMarker(q.f, uint32(FragmentALength));

            //Write fragment A pointer to fragment B.
            count FragmentBStart = q.f + 4 + ((j + 1) - q.i) - 4;
            WriteMarker(q.f + 4 + FragmentALength, uint32(FragmentBStart));

            //Write fragment B length (fragment B pointer to next is the same).
            count FragmentBLength = q.n - (j - q.i) - 1;
            WriteMarker(FragmentBStart, uint32(FragmentBLength));

            //Clear any additional erased space.
            Memory::Clear(&Data[q.f + 4 + FragmentALength + 4],
              EraseLength - (4 + 4));

            //If the last fragment was altered, update the last fragment index.
            if(q.f == LastFragmentIndex)
              LastFragmentIndex = FragmentBStart;
          }

          /*Since the excise occurred within the fragment, the operation is
          complete.*/
          break;
        }
      }

      //Reset the default iterator.
      DefaultIterator.Reset();
    }

    ///Erase a single character at the given index.
    void Erase(count i) {Erase(i, i);}

    ///Erase the last character of the string.
    void EraseLast() {Erase(n() - 1);}

    ///Erase the first character of the string.
    void EraseFirst() {Erase(0);}

    /**Erases the given ending from the string if it exists. Returns whether or
    not the erase took place.*/
    bool EraseEnding(const ascii* s)
    {
      if(not EndsWith(s))
        return false;
      Erase(n() - LengthOf(s), n() - 1);
      return true;
    }

    /**Erases the given beginning from the string if it exists. Returns whether
    or not the erase took place.*/
    bool EraseBeginning(const ascii* s)
    {
      if(not StartsWith(s))
        return false;
      Erase(0, LengthOf(s) - 1);
      return true;
    }

    ///Removes the given quote characters if they exist.
    void Unquote(unicode QuoteCharacter = '"')
    {
      String s(QuoteCharacter);
      if(StartsWith(s) and EndsWith(s))
        EraseFirst(), EraseLast();
    }

    ///Returns this string with the given quote characters removed.
    String Unquoted(unicode QuoteCharacter = '"') const
    {
      String s = *this;
      s.Unquote(QuoteCharacter);
      return s;
    }

    ///Wrapper to insert null-terminated string.
    void Insert(const ascii* s, count IndexBefore)
    {
      Insert(reinterpret_cast<const byte*>(s), LengthOf(s), IndexBefore);
    }

    ///Wrapper to append null-terminated string.
    void Append(const ascii* s)
    {
      Append(reinterpret_cast<const byte*>(s), LengthOf(s));
    }

    ///Wrapper to prepend null-terminated string.
    void Prepend(const ascii* s)
    {
      Prepend(reinterpret_cast<const byte*>(s), LengthOf(s));
    }

    ///Clears the string so that it is zero-length and has no data on the heap.
    inline void Clear()
    {
      //Remove any attached stream.
      AttachedStream = NotAttached;

      //Zero out the members.
      Data.n(LastFragmentIndex = InternalLength = 0);

      //Set precision to default.
      NumberPrecision = 5;

      //Reset the default iterator.
      DefaultIterator.Reset();
    }

    ///Attaches the string to one of the standard streams.
    void Attach(StreamAttachment StreamToAttachTo);

    private:

    ///Appends the fragment to the current attached stream.
    void AppendToStream(const byte* Fragment, count Length);

    public:

    /**Replaces a fragment with another string. The method first erases the
    fragment to be replaced and then inserts the new string.*/
    void Replace(count SourceIndex, count SourceLength, const byte* Destination,
      count DestinationLength)
    {
      Erase(SourceIndex, SourceIndex + SourceLength - 1);
      Insert(Destination, DestinationLength, SourceIndex);
    }

    ///Wrapper to replace fragment with null-terminated string.
    void Replace(count SourceIndex, count SourceLength,
      const ascii* Destination)
    {
      Replace(SourceIndex, SourceLength,
        reinterpret_cast<const byte*>(Destination), LengthOf(Destination));
    }

    //------------------//
    //Character Indexing//
    //------------------//

    ///Gets a character at the specified index.
    inline byte operator [] (count i) const
    {
      return Get(i);
    }

    //----------------//
    //Find and Replace//
    //----------------//

    /**Finds the next occurrence of the source string. Returns -1 if no match
    is found. Use StartIndex to start the find at a different position. Also,
    if StartIndex is less than 0, then no find occurs and -1 is returned.*/
    count Find(const byte* Source, count SourceLength, count StartIndex) const
    {
      if(not Source or SourceLength == 0 or InternalLength == 0 or
        StartIndex < 0)
          return -1;

      count MaxCharacter = InternalLength - SourceLength;

      if(Data.n() == 1 + 4 + 4 + InternalLength + 4)
      {
        //Take advantage of a merged string by randomly accessing.
        const byte* StringStart =
          reinterpret_cast<const byte*>(&(Merge()[StartIndex]));
        for(count i = StartIndex; i <= MaxCharacter; i++, StringStart++)
        {
          bool MatchFound = true;
          for(count j = 0; j < SourceLength; j++)
          {
            if(StringStart[j] != Source[j])
            {
              MatchFound = false;
              break;
            }
          }
          if(MatchFound)
            return i;
        }
      }
      else
      {
        /*If not merged use the slower but more string-manipulation friendly
        find, which uses an iterator.*/
        for(count i = StartIndex; i <= MaxCharacter; i++)
        {
          bool MatchFound = true;
          for(count j = 0; j < SourceLength; j++)
          {
            if(Get(i + j) != Source[j])
            {
              MatchFound = false;
              break;
            }
          }
          if(MatchFound)
            return i;
        }
      }
      return -1;
    }

    ///Finds next occurrence of source string.
    count Find(const ascii* Source, count StartIndex) const
    {
      return Find(reinterpret_cast<const byte*>(Source),
        LengthOf(Source), StartIndex);
    }

    ///Finds first occurrence of source string.
    count Find(const ascii* Source) const
    {
      return Find(Source, 0);
    }

    /**Globally replaces source string with destination string. Returns the
    number of replacements made. Since global replacement can make a lot of
    changes to the string, this can create many internal links. To prevent this
    from slowing down replaces on a long string with many replaces, the string
    must be merged at an interval of replacements that can be adjusted with
    MergeEvery.*/
    count Replace(const ascii* Source, const ascii* Destination,
      count MergeEvery = 30)
    {
      return Replace(reinterpret_cast<const byte*>(Source), LengthOf(Source),
        reinterpret_cast<const byte*>(Destination), LengthOf(Destination),
        MergeEvery);
    }

    /**Global replace by byte-wise block of data. Generally, you should use the
    null-terminated string Replace method unless for the search or replace term
    could contain a null.*/
    count Replace(const byte* Source, count SourceLength,
      const byte* Destination, count DestinationLength, count MergeEvery = 30)
    {
      //Make sure the merge every interval is reasonable.
      MergeEvery = Max(count(1), Min(count(1000), MergeEvery));

      count Replacements = 0;

      //For each occurrence of the source, replace it with the destination.
      count Next = Find(Source, SourceLength, 0);
      while(Next != -1)
      {
        Replace(Next, SourceLength, Destination, DestinationLength);
        Replacements++;

        /*If there are a ton of replacements happening, then merge every now
        and then to prevent iterations from taking up a lot of time.*/
        if(Replacements % MergeEvery == 0)
          Merge();
        Next = Find(Source, SourceLength, Next + DestinationLength);
      }

      //Return the number of replacements made.
      return Replacements;
    }

    ///Returns whether the string contains the source string.
    bool Contains(const ascii* Source) const
    {
      return Find(Source) != -1;
    }

    ///Returns a substring of characters between two indices inclusively.
    String Substring(count i, count j) const
    {
      if(i < 0 or j < i or j >= n())
        return "";

      return String(reinterpret_cast<const byte*>(&(Merge()[i])), j - i + 1);
    }

    ///Returns a substring of characters between two indices inclusively.
    String Substring(String::Span Selection) const
    {
      return Substring(Selection.i(), Selection.j());
    }

    /**Returns the start and end index of the next string containing the tokens.
    If no start is found then <-1, -1> is returned. If a start is found, but no
    end is found then <Start Index, -1> is returned. If no span is found, then
    BetweenText will be empty as well.*/
    String::Span FindBetween(const ascii* Begin, const ascii* End,
      String& BetweenText, count StartIndex = 0) const
    {
      BetweenText = "";
      count StartPlace = Find(Begin, StartIndex);
      if(StartPlace == -1)
        return String::Span(-1, -1);
      count BeginLength = LengthOf(Begin);
      count EndPlace = Find(End, StartPlace + BeginLength);
      if(EndPlace == -1)
        return String::Span(StartPlace, -1);
      BetweenText = Substring(String::Span(StartPlace + BeginLength,
        EndPlace - 1));
      return String::Span(StartPlace, EndPlace + LengthOf(End) - 1);
    }

    /**Finds which pair of tokens is next. Returns the index if one is found,
    and otherwise, or if the lists are not of equal size, returns -1. Call
    FindBetween using the given tokens to then determine the location of the
    token pair.*/
    count FindBetweenAmong(const List<String>& StartTokens,
      const List<String>& EndTokens, String::Span& Location,
      String& BetweenText, count StartIndex = 0) const
    {
      if(StartTokens.n() != EndTokens.n())
        return -1;

      count LeastIndex = -1, IndexOfFound = -1;

      for(count i = 0; i < StartTokens.n(); i++)
      {
        String FindBetweenText;
        String::Span Next = FindBetween(StartTokens[i], EndTokens[i],
          FindBetweenText, StartIndex);
        if(Next.j() == -1)
          continue;
        if(Next.i() < LeastIndex or LeastIndex == -1)
        {
          Location = Next;
          BetweenText = FindBetweenText;
          LeastIndex = Next.i();
          IndexOfFound = i;
        }
      }
      return IndexOfFound;
    }

    ///Converts all line endings (LF, CR, CRLF) to LF.
    void LineEndingsToLF()
    {
      Replace("\r\n", "\r");
      Replace("\r", "\n");
      Merge();
    }

    ///Converts all line endings (LF, CR, CRLF) to CRLF.
    void LineEndingsToCRLF()
    {
      LineEndingsToLF();
      Replace("\n", "\r\n");
      Merge();
    }

    ///Tokenizes the string by a delimiter and returns a list of strings.
    List<String> Tokenize(String Delimiter, bool RemoveEmptyEntries = false)
      const
    {
      //Make a copy of the string.
      String s = *this;

      //Add tokens at beginning and ending for using FindBetween.
      s.Prepend(Delimiter.Merge());
      s.Append(Delimiter.Merge());

      //Parse the paths into a list.
      List<String> Result;
      String::Span Location;
      String NextResult;
      while((Location = s.FindBetween(Delimiter, Delimiter, NextResult,
        Location.j())).j() != -1)
          Result.Add(NextResult);

      //Remove empty entries if requested.
      if(RemoveEmptyEntries)
        for(count i = Result.n() - 1; i >= 0; i--)
          if(not Result[i])
            Result.Remove(i);

      //Return the list of tokenized strings.
      return Result;
    }

    /**Removes all whitespace at the beginning and ending of the string. This is
    just removing spaces, line feeds, carriage returns, and tabs. It does not
    take into account the Unicode definition of whitespace.*/
    void Trim()
    {
      while(StartsWith(" ") or StartsWith("\n") or StartsWith("\r") or
        StartsWith("\t"))
          EraseFirst();
      while(EndsWith(" ") or EndsWith("\n") or EndsWith("\r") or EndsWith("\t"))
        EraseLast();
    }

    //----------------//
    //Operator Appends//
    //----------------//

    //String used for newlines

    ///Unix and Mac newline
    static const ascii* LF;

    ///Microsoft Windows newline
    static const ascii* CRLF;

    ///Global newline operator for ++ operations
    static const ascii* Newline;

    ///Appends a space to the string.
    void operator -- (int Dummy)
    {
      (void)Dummy;
      Append(" ");
    }

    ///Appends a new line to the string.
    void operator ++ (int Dummy)
    {
      (void)Dummy;
      Append(Newline);
    }

    ///Returns whether the string starts with the source.
    bool StartsWith(const ascii* Source) const
    {
      if(not Source) return false;
      return Find(Source) == 0;
    }

    ///Returns whether the string ends with the source.
    bool EndsWith(const ascii* Source) const
    {
      if(not Source) return false;
      count SourceLength = LengthOf(Source);
      return Find(reinterpret_cast<const byte*>(Source), SourceLength,
        InternalLength - SourceLength) != -1;
    }

    ///Returns whether the string matches the other byte for byte.
    bool operator == (const ascii* Other) const
    {
      if(LengthOf(Other) != n())
        return false;
      if(n() == 0)
        return true;

      const ascii* This = Merge();
      for(count i = n() - 1; i >= 0; i--)
        if(This[i] != Other[i])
          return false;
      return true;
    }

    ///Returns the opposite of the equivalence operator test.
    bool operator != (const ascii* Other) const
    {
      return not (*this == Other);
    }

    //----------------//
    //Stream Appending//
    //----------------//

    ///Appends a bool using the familiar standard stream << operator.
    String& operator << (bool v);

    ///Appends an uint8 using the familiar standard stream << operator.
    String& operator << (uint8 v);

    //Skipping int8 since it will be handled as ascii.

    ///Appends an uint16 using the familiar standard stream << operator.
    String& operator << (uint16 v);

    ///Appends an int16 using the familiar standard stream << operator.
    String& operator << (int16 v);

    ///Appends an uint32 using the familiar standard stream << operator.
    String& operator << (uint32 v);

    ///Appends an int32 using the familiar standard stream << operator.
    String& operator << (int32 v);

    ///Appends an uint64 using the familiar standard stream << operator.
    String& operator << (uint64 v);

    ///Appends an int64 using the familiar standard stream << operator.
    String& operator << (int64 v);

    ///Appends a float32 using the familiar standard stream << operator.
    String& operator << (float32 v);

    ///Appends a float64 using the familiar standard stream << operator.
    String& operator << (float64 v);

    ///Appends a float80 using the familiar standard stream << operator.
    String& operator << (float80 v);

    ///Appends a void pointer using the familiar standard stream << operator.
    String& operator << (const void* v);

    ///Appends a smart pointer using the familiar standard stream << operator.
    template <class T>
    String& operator << (const Pointer<T>& p)
    {
      //Show number of references.
      String s;
      s >> reinterpret_cast<const void*>(p.Raw()) << "+" << p.n();

      //Also show number of weak pointers if they exist.
      if(p.n(true) != p.n(false))
        s << "-" << (p.n(true) - p.n(false));
      Append(s.Merge());
      return *this;
    }

    ///Appends a vector to the stream.
    template <class T>
    String& operator << (const Complex<T>& v)
    {
      String s;
      if(v.IsEmpty())
        s >> "(Empty)";
      else
        s >> "(" << v.x << ", " << v.y << ")";
      Append(s.Merge());
      return *this;
    }

    ///Appends an array to the stream.
    template <class T> String& operator << (const Array<T>& a)
    {
      String s;
      s >> "[";
      for(count i = 0; i < a.n(); i++)
      {
        if(i)
          s << ", ";
        s << a[i];
      }
      s << "]";
      Append(s.Merge());
      return *this;
    }

    ///Appends a list to the stream.
    template <class T> String& operator << (const List<T>& a)
    {
      String s;
      s >> "{";
      for(count i = 0; i < a.n(); i++)
      {
        if(i)
          s << ", ";
        s << a[i];
      }
      s << "}";
      Append(s.Merge());
      return *this;
    }

    ///Appends a tree to the stream.
    template <class K, class V> String& operator << (const Tree<K, V>& a)
    {
      String s;
      s >> "{";
      typename Tree<K, V>::Iterator It;
      bool First = true;
      for(It.Begin(a); It.Iterating(); It.Next(), First = false)
      {
        if(not First)
          s << ", ";
        s << It.Key() << ":" << It.Value();
      }
      s << "}";
      Append(s.Merge());
      return *this;
    }

    ///Appends a matrix to the stream.
    template <class T> String& operator << (const Matrix<T>& M)
    {
      //Return the empty matrix if there are no elements.
      if(M.mn() == 0)
      {
        Append("or");
        return *this;
      }

      //Create a matrix of strings.
      Matrix<String> S(M.m(), M.n());
      for(count j = 0; j < M.n(); j++)
      {
        count MaxLength = 0;
        for(count i = 0; i < M.m(); i++)
        {
          S(i, j) << M.ij(i, j);
          MaxLength = Max(MaxLength, S(i, j).c());
        }

        //Normalize the lengths of each column.
        for(count i = 0; i < M.m(); i++)
          for(count k = S(i, j).c(); k < MaxLength; k++)
            S(i, j) << " ";
      }

      //Create the matrix string.
      String Out;
      for(count i = 0; i < M.m(); i++)
      {
        Out++;
        Out << "|";
        for(count j = 0; j < M.n(); j++)
          Out << S(i, j) << (j < M.n() - 1 ? " " : "|");
      }
      Out++;

      Append(Out.Merge());
      return *this;
    }

    ///Appends a letter using the familiar standard stream << operator.
    String& operator << (ascii c)
    {
      Append(reinterpret_cast<byte*>(&c), 1);
      return *this;
    }

    ///Appends a string using the familiar standard stream << operator.
    String& operator << (const ascii* s)
    {
      Append(s);
      return *this;
    }

    ///Appends a string using the familiar standard stream << operator.
    String& operator << (const String& s)
    {
      Append(reinterpret_cast<const byte*>(s.Merge()), s.n());
      return *this;
    }

    ///Appends a newline if string is not empty and then applies << operator.
    template <class T>
    String& operator >> (const T& v)
    {
      if(InternalLength != 0)
        Append(Newline);
      *this << v;
      return *this;
    }

    ///Concatenates this string with another object.
    template <class T>
    String operator + (const T& v) const
    {
      String s = *this;
      s << v;
      return s;
    }

    //------//
    //Length//
    //------//

    ///Returns the number of characters in a null-terminated string.
    static count LengthOf(const ascii* s);

    ///Returns the number of bytes in the string.
    inline count n() const {return InternalLength;}

    //----------------------//
    //Constructor-Destructor//
    //----------------------//

    ///Constructor creates an empty string.
    inline String() {Clear();}

    ///Constructs a string that is attached to one of the standard streams.
    String(StreamAttachment StreamToAttachTo)
    {
      Clear();
      Attach(StreamToAttachTo);
    }

    ///Copy constructor to initialize string with contents of other string.
    String(const ascii* Other) {Clear(); Append(Other);}

    ///Copy constructor to initialize string with contents of other string.
    String(const byte* Other, count Length) {Clear(); Append(Other, Length);}

    ///Appends a bool during construction.
    String(bool v) {Clear(); (*this) << v;}

    ///Appends an uint8 during construction.
    String(uint8 v) {Clear(); (*this) << v;}

    //Skipping int8 since it will be handled as ascii.

    ///Appends a letter during construction.
    String(ascii c) {Clear(); (*this) << c;}

    ///Appends an uint16 during construction.
    String(uint16 v) {Clear(); (*this) << v;}

    ///Appends an int16 during construction.
    String(int16 v) {Clear(); (*this) << v;}

    //Skipping uint32 since it will be handled as unicode.

    ///Constructor to initialize string with a unicode codepoint.
    String(unicode Codepoint) {Clear(); Append(Codepoint);}

    ///Appends an int32 during construction.
    String(int32 v) {Clear(); (*this) << v;}

    ///Appends an uint64 during construction.
    String(uint64 v) {Clear(); (*this) << v;}

    ///Appends an int64 during construction.
    String(int64 v) {Clear(); (*this) << v;}

    ///Appends a float32 during construction.
    String(float32 v) {Clear(); (*this) << v;}

    ///Appends a float64 during construction.
    String(float64 v) {Clear(); (*this) << v;}

    ///Appends a float80 during construction.
    String(float80 v) {Clear(); (*this) << v;}

    ///Appends a void pointer during construction.
    String(void* v) {Clear(); (*this) << v;}

    ///Appends a smart pointer during construction.
    template <class T> String(const Pointer<T>& p) {Clear(); (*this) << p;}

    ///Appends a vector during construction.
    template <class T> String(const Complex<T>& v) {Clear(); (*this) << v;}

    ///Appends an array during construction.
    template <class T> String(const Array<T>& a) {Clear(); (*this) << a;}

    ///Appends a list during construction.
    template <class T> String(const List<T>& a) {Clear(); (*this) << a;}

    ///Appends a tree during construction.
    template <class K, class V> String(const Tree<K, V>& a)
    {
      Clear();
      (*this) << a;
    }

    ///Appends a matrix during construction.
    template <class T> String(const Matrix<T>& M) {Clear(); (*this) << M;}

    //----------//
    //Assignment//
    //----------//

    ///Assigns string to contents of other string.
    String& operator = (const ascii* Other)
    {
      Clear();
      Append(Other);
      return *this;
    }

    ///Assigns string to a bool.
    String& operator = (bool v) {Clear(); return (*this) << v;}

    ///Assigns string to an uint8.
    String& operator = (uint8 v) {Clear(); return (*this) << v;}

    //Skipping int8 since it will be handled as ascii.

    ///Assigns string to a letter.
    String& operator = (ascii c) {Clear(); return (*this) << c;}

    ///Assigns string to an uint16.
    String& operator = (uint16 v) {Clear(); return (*this) << v;}

    ///Assigns string to an int16.
    String& operator = (int16 v) {Clear(); return (*this) << v;}

    //Skipping uint32 since it will be handle as unicode.

    ///Assigns string to a unicode codepoint.
    String& operator = (unicode Codepoint)
    {
      Clear();
      Append(Codepoint);
      return *this;
    }

    ///Assigns string to an int32.
    String& operator = (int32 v) {Clear(); return (*this) << v;}

    ///Assigns string to an uint64.
    String& operator = (uint64 v) {Clear(); return (*this) << v;}

    ///Assigns string to an int64.
    String& operator = (int64 v) {Clear(); return (*this) << v;}

    ///Assigns string to a float32.
    String& operator = (float32 v) {Clear(); return (*this) << v;}

    ///Assigns string to a float64.
    String& operator = (float64 v) {Clear(); return (*this) << v;}

    ///Assigns string to a float80.
    String& operator = (float80 v) {Clear(); return (*this) << v;}

    ///Assigns string to a void pointer.
    String& operator = (void* v) {Clear(); return (*this) << v;}

    ///Assigns string to a smart pointer.
    template <class T> String& operator = (const Pointer<T>& p)
    {
      Clear();
      return (*this) << p;
    }

    ///Assigns string to a vector.
    template <class T> String& operator = (const Complex<T>& v)
    {
      Clear();
      return (*this) << v;
    }

    ///Assigns string to an array.
    template <class T> String& operator = (const Array<T>& a)
    {
      Clear();
      return (*this) << a;
    }

    ///Assigns string to a list.
    template <class T> String& operator = (const List<T>& a)
    {
      Clear();
      return (*this) << a;
    }

    ///Assigns string to a list.
    template <class K, class V> String& operator = (const Tree<K, V>& a)
    {
      Clear();
      return (*this) << a;
    }

    ///Assigns string to a matrix.
    template <class T> String& operator = (const Matrix<T>& M)
    {
      Clear();
      return (*this) << M;
    }

    //---------//
    //Operators//
    //---------//

    ///Automatic conversion to const ascii* where possible.
    inline operator const ascii* () const {return Merge();}

    /**Returns whether or not the string is not empty. The following shows how
    this can be used in an if-statement:
    \code
    *  String s;
    *  ...
    *  if(s)
    *    c >> "s contains something";
    *  else
    *    c >> "s is empty";
    \endcode
    */
    inline operator bool() const {return n() > 0;}

    //---------------------//
    //Conversion to Numbers//
    //---------------------//

    ///Attempts to convert the string to a number.
    number ToNumber() const;

    //--------------//
    //Hex Conversion//
    //--------------//

    private:

    ///Give UUIDv4 access to the hex utilities here.
    friend class UUIDv4;

    ///Lookup table for hexadecimal to digit conversion.
    static const byte HexMap[256];

    ///Converts a byte into two hexadecimal digits.
    static void ToHex(byte Data, ascii& High, ascii& Low)
    {
      ascii DataHigh = ascii(Data >> 4);
      ascii DataLow = ascii(Data % 16);
      High = (DataHigh < 10 ? DataHigh + '0' : (DataHigh - 10) + 'a');
      Low = (DataLow < 10 ? DataLow + '0' : (DataLow - 10) + 'a');
    }

    public:

    ///Converts hex string to byte array.
    static Array<byte> Hex(const String& HexString)
    {
      //Clear the output array.
      Array<byte> ByteArray;

      //Get access to merged string.
      const byte* d = reinterpret_cast<const byte*>(HexString.Merge());

      //Must have even number of hex digits.
      if(HexString.n() % 2 != 0)
        return ByteArray;

      //Set the array size.
      ByteArray.n(HexString.n() / 2);

      //Go through pairs of characters and set the bytes of the array.
      for(count i = 0, j = 0; i < HexString.n(); i += 2, j++)
      {
        byte h = HexMap[count(d[i])], l = HexMap[count(d[i + 1])];

        //If an unexpected character occurs, then abort with a null array.
        if(h >= 16 or l >= 16)
        {
          ByteArray.Clear();
          break;
        }
        ByteArray[j] = byte((h << 4) + l);
      }
      return ByteArray;
    }

    ///Converts byte array to hex string.
    static String Hex(const Array<byte>& ByteArray)
    {
      Array<ascii> HexedArray(ByteArray.n() * 2);

      for(count i = 0; i < ByteArray.n(); i++)
      {
        ascii High = 0, Low = 0;
        ToHex(ByteArray[i], High, Low);
        HexedArray[i * 2] = High;
        HexedArray[i * 2 + 1] = Low;
      }

      return String(reinterpret_cast<byte*>(&HexedArray.a()), HexedArray.n());
    }

    /**Converts a number written in hex to an integer ignoring non-hex letters.
    Returns zero in case of an overflow.*/
    uint64 ToHexNumber() const
    {
      const byte* d = reinterpret_cast<const byte*>(Merge());
      uint64 CurrentValue = 0;
      uint64 PreviousValue = 0;
      for(count i = 0; i < n(); i++)
      {
        uint64 Digit = HexMap[count(d[i])];
        if(Digit < 16)
          CurrentValue = CurrentValue * uint64(16) + Digit;
        if(PreviousValue * 16 < PreviousValue || CurrentValue < PreviousValue)
          return 0;
        PreviousValue = CurrentValue;
      }
      return CurrentValue;
    }

    //---------------//
    //Case Conversion//
    //---------------//

    ///Returns the Latin Basic and Latin-1 uppercase of the string.
    String ToUpper() const
    {
      return ToLatin1Upper();
    }

    ///Returns the Latin Basic and Latin-1 lowercase of the string.
    String ToLower() const
    {
      return ToLatin1Lower();
    }

    ///Returns the Latin Basic and Latin-1 title case of the string.
    String ToTitle() const
    {
      return ToLatin1Title();
    }

    ///Sanitizes to an identifier suitable for common programming languages.
    String ToIdentifier() const
    {
      String In = Merge();
      //Convert special characters to numbers.
      const char* Filter[32] = {"`", "~", "!", "@", "#", "$", "%", "^", "&",
        "*", "(", ")", "-", "=", "+", "{", "[", "}", "]", "|", "\\", ";", ":",
        "\"", "'", "<", ",", ">", ".", "/", "?", " "};
      Array<String> Filters(Filter, 32);
      for(count i = 0; i < Filters.n(); i++)
        In.Replace(Filters[i], "_");

      //Prefix leading numbers and underscores with text.
      const char* Number[11] = {"0", "1", "2", "3", "4", "5", "6", "7", "8",
        "9", "_"};
      Array<String> Numbers(Number, 11);
      for(count i = 0; i < Numbers.n(); i++)
        if(In.StartsWith(Numbers[i]))
          In.Prepend("n");

      return In;
    }

    ///Given an absolute filename returns the path with trailing slash.
    String ToPath() const
    {
      String Filename = Merge();
      for(count i = Filename.n() - 1; i > 0; i--)
      {
        if(Filename[i] == '/' or Filename[i] == '\\')
        {
          Filename.Erase(i + 1, Filename.n() - 1);
          return Filename;
        }
      }
      return "";
    }

    ///Given an absolute filename returns the filename with no path.
    String ToFilename() const
    {
      String Filename = Merge();
      for(count i = Filename.n() - 1; i > 0; i--)
      {
        if(Filename[i] == '/' or Filename[i] == '\\')
        {
          Filename.Erase(0, i);
          return Filename;
        }
      }
      return Filename;
    }

    ///Returns the Latin-1 uppercase character equivalent.
    static unicode ToLatin1Upper(unicode c)
    {
      if(c >= 'a' and c <= 'z')
        c -= ('a' - 'A');
      else if(c >= 0xE0 and c <= 0xFE and c != 0xF7)
        c -= 0x20;
      else if(c == 0xDF)
        c = 0x1E9E;
      else if(c == 0xFF)
        c = 0x0178;
      return c;
    }

    ///Returns the Latin-1 lowercase character equivalent.
    static unicode ToLatin1Lower(unicode c)
    {
      if(c >= 'A' and c <= 'Z')
        c += ('a' - 'A');
      else if(c >= 0xC0 and c <= 0xDE and c != 0xD7)
        c += 0x20;
      else if(c == 0x1E9E)
        c = 0xDF;
      else if(c == 0x0178)
        c = 0xFF;
      return c;
    }

    ///Uppercases any Latin Basic or Latin-1 characters.
    String ToLatin1Upper() const
    {
      const byte* ReadPosition = reinterpret_cast<const byte*>(Merge());
      const byte* EndMarker = &ReadPosition[InternalLength];
      String s;
      while(ReadPosition != EndMarker)
        s.Append(ToLatin1Upper(Decode(ReadPosition, EndMarker)));
      return s;
    }

    ///Lowercases any Latin Basic or Latin-1 characters.
    String ToLatin1Lower() const
    {
      const byte* ReadPosition = reinterpret_cast<const byte*>(Merge());
      const byte* EndMarker = &ReadPosition[InternalLength];
      String s;
      while(ReadPosition != EndMarker)
        s.Append(ToLatin1Lower(Decode(ReadPosition, EndMarker)));
      return s;
    }

    ///Title-cases any Latin Basic or Latin-1 characters.
    String ToLatin1Title() const
    {
      const byte* ReadPosition = reinterpret_cast<const byte*>(Merge());
      const byte* EndMarker = &ReadPosition[InternalLength];
      String s;
      bool InitialCharacter = true;
      while(ReadPosition != EndMarker)
      {
        unicode c = Decode(ReadPosition, EndMarker);
        if(c == 0x20)
          InitialCharacter = true;
        else if(InitialCharacter)
          c = ToLatin1Upper(c), InitialCharacter = false;
        else
          c = ToLatin1Lower(c);
        s.Append(c);
      }
      return s;
    }

    //-----//
    //UTF-8//
    //-----//

    /*Encodes and decodes UTF-8 in a String. Internally, String is a byte array
    with no inherent encoding. String simply provides a means to safely encode
    and decode a string as UTF-8. It will filter out bad character sequences as
    a part of encoding and decoding, but it does nothing to ensure that the
    string data is valid UTF-8 since String does not have an explicit
    encoding.*/

    private:

    ///Lookup table for codepoint bias
    static const unicode CodepointBias[256];

    ///Lookup table for octet classification
    static const count OctetClassification[256];

    /*Given an octet class, the encoded value must be at least this number.
    Otherwise, the value is overlong (has been encoded with higher octet class
    than strictly necessary.*/
    static const count OverlongThresholds[7];

    public:

    /**Decodes the next character of a UTF-8 string according to the ISO
    10646-1:2000 standard for safely decoding an arbitrary byte stream as UTF-8.
    StreamEnd is the location of the first out-of-bounds byte. It is the
    responsibility of the caller to make sure that Stream < StreamEnd. The
    method does handle a sudden stream termination in the middle of a multi-byte
    UTF-8 sequence.*/
    static unicode Decode(const byte*& Stream, const byte* StreamEnd)
    {
      byte Octet1, Octet2, Octet3, Octet4, Octet5, Octet6;
      unicode Value = meta::BadCharacter;

      //Classify by initial octet.
      Octet1 = *Stream++; //Assumes caller knows Stream < StreamEnd.
      count OctetClass = OctetClassification[Octet1];

      if(OctetClass == 1) //ASCII character
        return Octet1; //No need to validate since ASCII is always valid.
      else if(OctetClass == 2 and Stream != StreamEnd) //2-octet sequence
      {
        Octet2 = *Stream;
        if(OctetClassification[Octet2] == 0 and ++Stream)
          Value = CodepointBias[Octet1] + CodepointBias[Octet2];
      }
      else if(OctetClass == 3 and Stream != StreamEnd) //3-octet sequence
      {
        Octet2 = *Stream;
        if(OctetClassification[Octet2] == 0 and ++Stream != StreamEnd)
        {
          Octet3 = *Stream;
          if(OctetClassification[Octet3] == 0 and ++Stream)
          {
            Value = CodepointBias[Octet1] + (CodepointBias[Octet2] << 6) +
              CodepointBias[Octet3];
          }
        }
      }
      else if(OctetClass == 4 and Stream != StreamEnd) //4-octet sequence
      {
        Octet2 = *Stream;
        if(OctetClassification[Octet2] == 0 and ++Stream != StreamEnd)
        {
          Octet3 = *Stream;
          if(OctetClassification[Octet3] == 0 and ++Stream != StreamEnd)
          {
            Octet4 = *Stream;
            if(OctetClassification[Octet4] == 0 and ++Stream)
            {
              Value = CodepointBias[Octet1] + (CodepointBias[Octet2] << 12) +
                (CodepointBias[Octet3] << 6) + CodepointBias[Octet4];
            }
          }
        }
      }
      else if(OctetClass == 5 and Stream != StreamEnd) //5-octet sequence (bad)
      {
        Octet2 = *Stream;
        if(OctetClassification[Octet2] == 0 and ++Stream != StreamEnd)
        {
          Octet3 = *Stream;
          if(OctetClassification[Octet3] == 0 and ++Stream != StreamEnd)
          {
            Octet4 = *Stream;
            if(OctetClassification[Octet4] == 0 and ++Stream != StreamEnd)
            {
              Octet5 = *Stream;
              if(OctetClassification[Octet5] == 0 and ++Stream)
              {
                Value = CodepointBias[Octet1] + (CodepointBias[Octet2] << 18) +
                  (CodepointBias[Octet3] << 12) + (CodepointBias[Octet4] << 6) +
                  CodepointBias[Octet5];
              }
            }
          }
        }
      }
      else if(OctetClass == 6 and Stream != StreamEnd) //6-octet sequence (bad)
      {
        Octet2 = *Stream;
        if(OctetClassification[Octet2] == 0 and ++Stream != StreamEnd)
        {
          Octet3 = *Stream;
          if(OctetClassification[Octet3] == 0 and ++Stream != StreamEnd)
          {
            Octet4 = *Stream;
            if(OctetClassification[Octet4] == 0 and ++Stream != StreamEnd)
            {
              Octet5 = *Stream;
              if(OctetClassification[Octet5] == 0 and ++Stream != StreamEnd)
              {
                Octet6 = *Stream;
                if(OctetClassification[Octet6] == 0 and ++Stream)
                {
                  Value = CodepointBias[Octet1] +
                    (CodepointBias[Octet2] << 24) +
                    (CodepointBias[Octet3] << 18) +
                    (CodepointBias[Octet4] << 12) +
                    (CodepointBias[Octet5] << 6) +
                    CodepointBias[Octet6];
                }
              }
            }
          }
        }
      }

      //Check for overlongs forms.
      if(OctetClass >= 1 and OctetClass <= 6 and
        Value < unicode(OverlongThresholds[OctetClass]))
          Value = meta::BadCharacter;

      //Check if value is above maximum usable Unicode point.
      if(Value > 0x10FFFF)
        Value = meta::BadCharacter;

      /*Strip any UTF-16 surrogates and illegal code positions that are invalid
      in UTF-8.*/
      if(Value == 0xD800 or Value == 0xDB7F or Value == 0xDB80 or
         Value == 0xDBFF or Value == 0xDC00 or Value == 0xDF80 or
         Value == 0xDFFF or Value == 0xFFFE or Value == 0xFFFF)
           Value = meta::BadCharacter;

      return Value;
    }

    ///Reads the string to determine if it is a valid UTF-8 string.
    bool IsUTF8() const
    {
      const byte* a = reinterpret_cast<const byte*>(Merge());
      const byte* z = a + n();
      unicode d;
      do
      {
        d = Decode(a, z);
        if(d == meta::BadCharacter)
          return false;
      } while(d);
      return true;
    }

    /**Reads the string to determine if it is a valid ASCII string. Note that
    non-printable characters aside from tab, line space, carriage return are
    excluded (other control characters will cause the string to not report as
    ASCII).*/
    bool IsASCII() const
    {
      const byte* a = reinterpret_cast<const byte*>(Merge());
      byte d;
      do
      {
        d = *a++;
        if(d != 0 and d != 9 and d != 10 and d != 13 and not
          (d >= 32 and d <= 126))
            return false;
      } while(d);
      return true;
    }

    /**Reads the string to determine if it is a valid Latin-1 string (conforms
    to ISO-8859-1).*/
    bool IsLatin1() const
    {
      if(IsASCII()) return true;
      if(IsUTF8()) return false;

      const byte* a = reinterpret_cast<const byte*>(Merge());
      byte d;
      do
      {
        d = *a++;
        if(d != 0 and d != 9 and d != 10 and d != 13 and not
          (d >= 32 and d <= 126) and not (d >= 160))
            return false;
      } while(d);
      return true;
    }

    ///Returns whether the character is alphanumeric.
    static bool IsAlphanumeric(unicode c)
    {
      return (c >= '0' and c <= '9') or
        (c >= 'A' and c <= 'Z') or
        (c >= 'a' and c <= 'z');
    }

    ///Returns whether the character is alphanumeric.
    static bool IsAlphanumeric(ascii c)
    {
      return IsAlphanumeric(unicode(c));
    }

    ///Returns the canonical lorem ipsum filler text.
    static String LoremIpsum()
    {
      return "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed "
      "do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim "
      "ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut "
      "aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit "
      "in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
      "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui "
      "officia deserunt mollit anim id est laborum.";
    }

    class UTF16
    {
      public:

      enum ByteOrder
      {
        ByteOrderUnspecified = 0x0,
        ByteOrderLE          = 0xfffe,
        ByteOrderBE          = 0xfeff
      };

      ///Detects the presence of a BOM at the beginning of the string.
      static ByteOrder BOM(const String& In)
      {
        return BOM(StringToUTF16Pointer(In));
      }

      /**Uses common characters (space 0x20, line feed 0x0A) to find byte order.
      If there are more 0x2000 and 0x0A00 values than 0x0020 and 0x00A0 values,
      then the byte order is guessed to be little-endian. Likewise, if there
      are more 0x0020 and 0x00A0 values than 0x2000 and 0x0A00 values then the
      byte order is guessed to be big-endian. If neither is found, the byte
      order is unspecified. Note that this method assumes that the input is a
      UTF-16 string.*/
      static ByteOrder GuessByteOrder(const String& In)
      {
        return GuessByteOrder(
          StringToUTF16Pointer(In), ByteOrderSearchLength(In));
      }

      static bool IsUTF16(const String& In)
      {
        return BOM(In) != ByteOrderUnspecified or
          GuessByteOrder(In) != ByteOrderUnspecified;
      }

      static String Decode(const String& In)
      {
        ByteOrder AssumedByteOrder = BOM(In);
        if(AssumedByteOrder == ByteOrderUnspecified)
          AssumedByteOrder = GuessByteOrder(In);
        String Out;
        Decode(StringToUTF16Pointer(In), &StringToUTF16Pointer(In)[In.n() / 2],
          AssumedByteOrder, Out);
        if(not Out)
          Out = In;
        return Out;
      }

      private:

      static count ByteOrderSearchLength(const String& In)
      {
        count SearchLength = 0;
        if(In.n() % 2 == 0)
          SearchLength = Min(In.n() / 2, count(1024));
        return SearchLength;
      }

      static void Decode(const uint16* In, const uint16* InEnd,
        ByteOrder ByteOrderToUse, String& Out)
      {
        Out.Clear();
        if(ByteOrderToUse == ByteOrderUnspecified)
          return;

        uint16 BEMask = ByteOrderToUse == ByteOrderBE ? 0xffff : 0x0000;
        uint16 LEMask = ByteOrderToUse == ByteOrderLE ? 0xffff : 0x0000;

        if(BOM(In) != ByteOrderUnspecified) In++;
        const uint8* Bytes = UTF16ToBytePointer(In);
        const uint8* BytesEnd = UTF16ToBytePointer(InEnd);
        while(Bytes and Bytes < BytesEnd)
        {
          uint16 a1 = *Bytes++;
          uint16 b1 = *Bytes++;
          uint16 w1 = (((a1 << 8) + b1) & BEMask) + (((b1 << 8) + a1) & LEMask);
          unicode u = meta::BadCharacter;
          if(w1 < 0xd800 or w1 > 0xdfff)
            u = unicode(w1);
          else if(Bytes < BytesEnd)
          {
            uint16 a2 = *Bytes++;
            uint16 b2 = *Bytes++;
            uint16 w2 =
              (((a2 << 8) + b2) & BEMask) + (((b2 << 8) + a2) & LEMask);
            if(w2 >= 0xdc00 and w2 <= 0xdfff)
              u = (unicode(w1 & 0x3ff) << 10) + unicode(w2 & 0x3ff) +
                unicode(0x10000);
          }
          Out.Append(u);
        }
      }

      ///Detects the presence of a BOM at the beginning of the string.
      static ByteOrder BOM(const uint16* In)
      {
        ByteOrder DetectedBOM = ByteOrderUnspecified;
        if(In and *In)
        {
          byte a = UTF16ToBytePointer(In)[0], b = UTF16ToBytePointer(In)[1];
          if(a == 0xff and b == 0xfe)
            DetectedBOM = ByteOrderLE;
          else if(a == 0xfe and b == 0xff)
            DetectedBOM = ByteOrderBE;
        }
        return DetectedBOM;
      }

      /**Uses common characters (space 0x20, line feed 0x0A) to find byte order.
      If there are more 0x2000 and 0x0A00 values than 0x0020 and 0x00A0 values,
      then the byte order is guessed to be little-endian. Likewise, if there
      are more 0x0020 and 0x00A0 values than 0x2000 and 0x0A00 values then the
      byte order is guessed to be big-endian. If neither is found, the byte
      order is unspecified. Note that this method assumes that the input is a
      UTF-16 string.*/
      static ByteOrder GuessByteOrder(const uint16* In,
        count MaximumSearchLength)
      {
        count LEPoints = 0, BEPoints = 0;
        count BytesExamined = 0;
        while(In and *In and BytesExamined++ < MaximumSearchLength)
        {
          uint8 a = UTF16ToBytePointer(In)[0];
          uint8 b = UTF16ToBytePointer(In++)[1];
          if(not a and (b == 0x0a or b == 0x20)) BEPoints++;
          if(not b and (a == 0x0a or a == 0x20)) LEPoints++;
        }
        ByteOrder GuessedByteOrder = ByteOrderUnspecified;
        if(LEPoints > BEPoints)
          GuessedByteOrder = ByteOrderLE;
        else if(BEPoints > LEPoints)
          GuessedByteOrder = ByteOrderBE;
        return GuessedByteOrder;
      }

      static const uint16* StringToUTF16Pointer(const String& In)
      {
        return reinterpret_cast<const uint16*>(In.Merge());
      }

      static const byte* UTF16ToBytePointer(const uint16* In)
      {
        return reinterpret_cast<const byte*>(In);
      }
    };

    /**Attempts to convert the string to UTF-8. It first attempts to detect the
    encoding as UTF-8 or Latin-1. If the string is in Latin-1, then it will
    convert the string to UTF-8. If the encoding can not be detected then the
    method will return false and leave the string untouched.*/
    bool ConvertToUTF8()
    {
      bool Success = true;
      if(IsUTF8());
      else if(UTF16::IsUTF16(*this))
        *this = UTF16::Decode(*this);
      else if(IsLatin1())
      {
        String s;
        for(count i = 0; i < n(); i++)
          s.Append(unicode(Get(i)));
        *this = s;
      }
      else
        Success = false;
      return Success;
    }

    /**Removes all non-ASCII characters including rare control characters.
    Returns whether the string was changed.*/
    bool ForceToASCII()
    {
      bool WasChanged = false;
      for(count i = n() - 1; i >= 0; i--)
      {
        byte b = Get(i);
        if(b != 9 and b != 10 and b != 13 and
          not (b >= 32 and b <= 126))
        {
          Erase(i, i);
          WasChanged = true;
        }
      }
      Merge();
      return WasChanged;
    }

    /**Forces to UTF-8 and removes rare control characters. Returns whether the
    string was changed.*/
    bool ForceToUTF8()
    {
      const byte* a = reinterpret_cast<const byte*>(Merge());
      const byte* z = a + n();
      String NewString;
      unicode d;
      do
      {
        if((d = Decode(a, z)))
        {
          if(d != 9 and d != 10 and d != 13 and
            not (d >= 32 and d <= 126) and d < 128)
              d = meta::BadCharacter;
          NewString << d;
        }
      } while(d);
      bool WasChanged = *this != NewString;
      *this = NewString;
      return WasChanged;
    }

    ///Calculates the number of UTF-8 characters in the string.
    count Characters() const
    {
      const byte* Start = reinterpret_cast<const byte*>(Merge());
      const byte* ReadPosition = Start;
      const byte* EndMarker = &Start[n()];
      count CharacterCount = 0;
      while(ReadPosition != EndMarker)
      {
        Decode(ReadPosition, EndMarker);
        CharacterCount++;
      }
      return CharacterCount;
    }

    ///Calculates the number of UTF-8 characters in the string.
    count c() const {return Characters();}

    /**Translates a character index to a byte index in UTF-8. If the character
    is negative or if the character is beyond the null-terminator, then -1 is
    returned. The null-terminator is considered a valid character from the
    perspective of indexing.*/
    count CharacterIndex(count c) const
    {
      if(not c) return 0;
      if(c < 0) return -1;
      const byte* Start = reinterpret_cast<const byte*>(Merge());
      const byte* ReadPosition = Start;
      const byte* EndMarker = &Start[n()];
      count CharacterCount = 0;
      while(ReadPosition != EndMarker)
      {
        Decode(ReadPosition, EndMarker);
        if(++CharacterCount == c)
          return count(ReadPosition - Start);
      }
      return -1;
    }

    ///Shorthand for CharacterIndex().
    count ci(count Character) const {return CharacterIndex(Character);}

    ///Returns the unicode value of the c-th character.
    unicode cth(count c) const
    {
      count i = CharacterIndex(c);
      if(i < 0)
        return 0;
      const byte* Start = reinterpret_cast<const byte*>(Merge());
      const byte* ReadPosition = &Start[i];
      const byte* EndMarker = &Start[n()];
      return Decode(ReadPosition, EndMarker);
    }

    ///Decodes an entire byte array to a String::UTF32.
    static void DecodeStream(const byte* Start, count Length,
      String::UTF32& Output)
    {
      const byte* ReadPosition = Start;
      const byte* EndMarker = &Start[Length];

      Output.n(0);
      while(ReadPosition != EndMarker)
        Output.Add(Decode(ReadPosition, EndMarker));
    }

    ///Decodes the current string to a String::UTF32.
    void DecodeTo(String::UTF32& Output) const
    {
      DecodeStream(reinterpret_cast<const byte*>(Merge()), n(), Output);
    }

    /**Appends a Unicode codepoint to the string as UTF-8. If the character is
    not representable as a valid Unicode codepoint, then it is substituted with
    the bad character symbol defined at meta::BadCharacter.*/
    inline void Append(unicode Codepoint)
    {
      //Create an alias of the incoming variable.
      unicode d = Codepoint;

      //Do not encode an illegal character.
      if(d > 0x10FFFF or d == 0xD800 or d == 0xDB7F or d == 0xDB80 or
          d == 0xDBFF or d == 0xDC00 or d == 0xDF80 or
          d == 0xDFFF or d == 0xFFFE or d == 0xFFFF)
            d = meta::BadCharacter;

      //Encode the codepoint in between 1 and 4 octets.
      byte e[4] = {0, 0, 0, 0};
      if(d < 0x80) //1 octet
      {
        e[0] = byte(d);
        Append(&e[0], 1);
      }
      else if(d < 0x800) //2 octets
      {
        e[0] = byte(((d >> 6) & 0x1F) + 0xC0); //110xxxxx
        e[1] = byte((d & 0x3F) + 0x80); //10xxxxxx
        Append(&e[0], 2);
      }
      else if(d < 0x10000) //3 octets
      {
        e[0] = byte(((d >> 12) & 0x0F) + 0xE0); //1110xxxx
        e[1] = byte(((d >> 6) & 0x3F) + 0x80); //10xxxxxx
        e[2] = byte((d & 0x3F) + 0x80); //10xxxxxx
        Append(&e[0], 3);
      }
      else //4 octets
      {
        e[0] = byte(((d >> 18) & 0x07) + 0xF0); //11110xxx
        e[1] = byte(((d >> 12) & 0x3F) + 0x80); //10xxxxxx
        e[2] = byte(((d >> 6) & 0x3F) + 0x80); //10xxxxxx
        e[3] = byte((d & 0x3F) + 0x80); //10xxxxxx
        Append(&e[0], 4);
      }
    }

    ///Appends an array of codepoints to the string as UTF-8.
    void Append(const String::UTF32& Codepoints)
    {
      for(count i = 0; i < Codepoints.n(); i++)
        Append(Codepoints[i]);
    }

    ///Returns true if the left is (binary-string wise) greater than the right.
    bool operator < (const String& Other) const;

    ///Returns true if the left is (binary-string wise) greater than the right.
    bool operator > (const String& Other) const;

    ///Less than or equal operator.
    bool operator <= (const String& Other) const
    {
      return *this < Other or *this == Other;
    }

    ///Greater than or equal operator.
    bool operator >= (const String& Other) const
    {
      return *this > Other or *this == Other;
    }
  };

#ifdef PRIM_COMPILE_INLINE
  const ascii* String::LF = "\x0A"; //Practically equivalent to '\n'
  const ascii* String::CRLF = "\x0D\x0A"; //Practically equivalent to '\r\n'
#ifdef PRIM_ENVIRONMENT_WINDOWS
  const ascii* String::Newline = "\x0A"; //Windows is mostly happy with LF.
#else
  const ascii* String::Newline = "\x0A";
#endif

  void String::AppendToStream(const byte* Fragment, count Length)
  {
    if(AttachedStream == StandardOutput)
    {
      fwrite(reinterpret_cast<const void*>(Fragment), 1,
        size_t(Length), stdout);
      fflush(stdout);
    }
    else if(AttachedStream == StandardError)
    {
      fwrite(reinterpret_cast<const void*>(Fragment), 1,
        size_t(Length), stderr);
      fflush(stderr);
    }
  }

  void String::Attach(StreamAttachment StreamToAttachTo)
  {
    AttachedStream = StreamToAttachTo;
    if(AttachedStream == StandardInput)
    {
      std::string s;
      std::cin >> s;
      Append(reinterpret_cast<const byte*>(s.c_str()), count(s.size()));
    }
  }

  bool String::operator < (const String& Other) const
  {
    return strcmp(Merge(), Other.Merge()) < 0;
  }

  bool String::operator > (const String& Other) const
  {
    return strcmp(Merge(), Other.Merge()) > 0;
  }

  String& String::operator << (bool v)
  {
    (*this) << (v ? "True" : "False");
    return *this;
  }

  String& String::operator << (uint8 v)
  {
    std::stringstream ss;
    ss << int32(v);
    char BufferData[32]; ss.read(&BufferData[0], 32);
    Append(reinterpret_cast<byte*>(&BufferData[0]), count(ss.gcount()));
    return *this;
  }

  String& String::operator << (uint16 v)
  {
    std::stringstream ss;
    ss << v;
    char BufferData[32]; ss.read(&BufferData[0], 32);
    Append(reinterpret_cast<byte*>(&BufferData[0]), count(ss.gcount()));
    return *this;
  }

  String& String::operator << (int16 v)
  {
    std::stringstream ss;
    ss << v;
    char BufferData[32]; ss.read(&BufferData[0], 32);
    Append(reinterpret_cast<byte*>(&BufferData[0]), count(ss.gcount()));
    return *this;
  }

  String& String::operator << (unicode v)
  {
    Append(v);
    return *this;
  }

  String& String::operator << (int32 v)
  {
    std::stringstream ss;
    ss << v;
    char BufferData[32]; ss.read(&BufferData[0], 32);
    Append(reinterpret_cast<byte*>(&BufferData[0]), count(ss.gcount()));
    return *this;
  }

  String& String::operator << (uint64 v)
  {
    std::stringstream ss;
    ss << v;
    char BufferData[32]; ss.read(&BufferData[0], 32);
    Append(reinterpret_cast<byte*>(&BufferData[0]), count(ss.gcount()));
    return *this;
  }

  String& String::operator << (int64 v)
  {
    std::stringstream ss;
    ss << v;
    char BufferData[32]; ss.read(&BufferData[0], 32);
    Append(reinterpret_cast<byte*>(&BufferData[0]), count(ss.gcount()));
    return *this;
  }

  void String::Append(float64 v, count Precision = 17,
    bool ScientificNotation = true)
  {
    if(Limits<float64>::IsEqual(v, Limits<float64>::Infinity()))
    {
      *this << Constants::Infinity();
      return;
    }
    else if(Limits<float64>::IsEqual(v, Limits<float64>::NegativeInfinity()))
    {
      *this << "-" << Constants::Infinity();
      return;
    }
    else if(Limits<float64>::IsNaN(v))
    {
      *this << Constants::NullSet();
      return;
    }

    if(Precision < 1)
      Precision = 1;
    else if(Precision > 17)
      Precision = 17;
    if(ScientificNotation)
    {
      std::stringstream ss;
      ss.precision(int(Precision));
      ss << v;
      char BufferData[32];
      Memory::MemSet(BufferData, 0, 32);
      ss.read(&BufferData[0], 32);
      Append(reinterpret_cast<byte*>(&BufferData[0]), LengthOf(BufferData));
    }
    else
    {
      v = Chop(v, 1.0e-16);
      if(Abs(v) >= 1.0e+16) v = 1.0e+16 * Sign(v);
      std::stringstream ss;
      ss.precision(int(Precision));
      ss.setf(std::ios::fixed, std::ios::floatfield);
      ss << v;
      char BufferData[32];
      Memory::MemSet(BufferData, 0, 32);
      ss.read(&BufferData[0], 32);

      //Remove trailing zeroes.
      for(count i = LengthOf(BufferData) - 1; i > 1; i--)
      {
        if(BufferData[i] == '0' and BufferData[i - 1] != '.')
          BufferData[i] = 0;
        else
          break;
      }

      Append(reinterpret_cast<byte*>(&BufferData[0]), LengthOf(BufferData));
    }
  }

  String& String::operator << (float64 v)
  {
    Append(v, NumberPrecision, false);
    return *this;
  }

  String& String::operator << (float32 v)
  {
    return (*this) << (float64(v));
  }

  String& String::operator << (float80 v)
  {
    return (*this) << (float64(v));
  }

  String& String::operator << (const void* v)
  {
    std::stringstream ss;
    ss << v;
    char BufferData[32]; ss.read(&BufferData[0], 32);
    Append(reinterpret_cast<byte*>(&BufferData[0]), count(ss.gcount()));
    return *this;
  }

  count String::LengthOf(const ascii* s)
  {
    if(not s) return 0;
    return count(strchr(s, 0) - s);
  }

  number String::ToNumber() const
  {
    return number(atof(Merge()));
  }

  const unicode String::CodepointBias[256] =
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
  22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
  60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,
  79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97,
  98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
  114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 0x0,
  0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
  0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
  0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
  0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x0, 0x40, 0x80, 0xC0,
  0x100, 0x140, 0x180, 0x1C0, 0x200, 0x240, 0x280, 0x2C0, 0x300, 0x340, 0x380,
  0x3C0, 0x400, 0x440, 0x480, 0x4C0, 0x500, 0x540, 0x580, 0x5C0, 0x600, 0x640,
  0x680, 0x6C0, 0x700, 0x740, 0x780, 0x7C0, 0x0, 0x1000, 0x2000, 0x3000, 0x4000,
  0x5000, 0x6000, 0x7000, 0x8000, 0x9000, 0xA000, 0xB000, 0xC000, 0xD000,
  0xE000, 0xF000, 0x0, 0x40000, 0x80000, 0xC0000, 0x100000, 0x140000, 0x180000,
  0x1C0000, 0x0, 0x1000000, 0x2000000, 0x3000000, 0x0, 0x4000000, 0, 0};

  const count String::OctetClassification[256] =
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, -1, -1};

  const count String::OverlongThresholds[7] = {0, 0, 0x80, 0x800, 0x10000,
    0x200000, 0x4000000};

  /*Lookup table to map ASCII byte codes to hexadecimal digits. Anything that is
  a hexadecimal digit is mapped to its corresponding value, and anything else is
  mapped to 16, indicating it is not a valid hexadecimal digit.*/
  const byte String::HexMap[256] = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 16, 16, 16, 16, 16, 16, 10, 11, 12,
  13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 10, 11, 12, 13, 14, 15, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16};
#endif
}
#endif
