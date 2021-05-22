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

#ifndef MICA_EN_CPP_H
#define MICA_EN_CPP_H

#define MICA_NAMESPACE mica

namespace MICA_NAMESPACE
{
/*Determine the simplest 64-bit type. Generally speaking, the long type on LP64
is 64-bit and on most other platform types long long is 64-bit.*/
#if defined(__LP64__) || defined(_LP64)
  
  //Data model: LP64
  
  ///64-bit unsigned integer.
  typedef unsigned long uint64;
  
  ///64-bit signed integer.
  typedef long           int64;
  
#else
  
  //Data models: ILP32, LLP64, and ILP64 (rare)
  
  ///64-bit unsigned integer.
  typedef unsigned long long uint64;

  ///64-bit signed integer.
  typedef long long           int64;
#endif

  /**Stores a UUIDv4 as a POD-compatible type. Note that since this class must
  be initializable via the bracket {} syntax, it can not use a default
  constructor, and is in an indeterminate state upon construction.*/
  struct UUIDv4
  {
    /**High part of the 128-bit identifier. In the case of a number, this stores
    a two's complement signed numerator.*/
    uint64 high;
    
    /**Low part of the 128-bit identifier. In the case of a number, this stores
    a two's complement signed and nonnegative denominator. When the sign bit
    (most significant bit) is on, the 128-bit number is a unique identifier.*/
    uint64 low;

    /**Less-than comparison operator for sorting. Note that in the case of a
    number this less than is not a numeric less than, but a bitwise less than.*/
    bool operator < (const UUIDv4& other) const;

    /**Greater-than comparison operator for sorting. Note that in the case of a
    number this less than is not a numeric less than, but a bitwise less than.*/
    bool operator > (const UUIDv4& other) const;
    
    ///Equal-to comparison operator
    bool operator == (const UUIDv4& other) const;
    
    ///Not equal-to comparison operator
    bool operator != (const UUIDv4& other) const;

#ifdef PRIM_LIBRARY
    ///Returns the identifier as text in the default language.
    operator PRIM_NAMESPACE::String () const;
    
    ///Returns the UUIDv4 as a ratio.
    PRIM_NAMESPACE::Ratio ratio() const;
#else
    ///Returns the identifier as text in the default language.
    operator const char* () const;
#endif
  };

  ///Managed UUIDv4 that constructs to Undefined by default.
  struct Concept : public UUIDv4
  {
    Concept();
    Concept(const UUIDv4& other);
    
    /**Reconstructs a concept from just the high word. This only works for
    identifiers and undefined. It can not reconstruct ratios.*/
    Concept(int64 shortform);
    
    /**Reconstructs a concept from just the high word. This only works for
    identifiers and undefined. It can not reconstruct ratios.*/
    Concept(uint64 shortform);
    
#ifdef PRIM_LIBRARY
    Concept(PRIM_NAMESPACE::Ratio other);
    Concept(const PRIM_NAMESPACE::Value& other);
    Concept(const char* other);
#else
    Concept(const char* other);
#endif
  };
  
  /* Conventions:
    * All functions are one unabbreviated word
    
    * All functions are safe and will accept any input. If the result is
      indeterminate, then Undefined is returned. A slight exception to this
      principle is the length() function, which returns zero if the argument is
      invalid. The only functions that can crash are the name() and named()
      functions if a string is passed in with an illegal memory address.
      
    * Arguments which filter the selection appear first and the argument
      indicating the thing being looked for appears last:
      
      - index(mica::Letters, mica::C, mica::G)
        means: in Letters relative to C, get index of G.
        
      - name("en", mica::Letters)
        means: in English, get the name of Letters
        
      - item(mica::Letters, mica::C, 4)
        means: in Letters relative to C, get the fourth element
  */

  //
  //Functions to get the index of an element in a sequence
  //
  
  /**Returns the index of a concept in a given sequence as a number. If the
  element is not in the sequence then Undefined is returned.*/
  UUIDv4 index(UUIDv4 sequence, UUIDv4 element);
  
  /**Returns the index of an element in a given sequence relative to another
  concept acting as the origin. If the index does not exist then Undefined is
  returned.*/
  UUIDv4 index(UUIDv4 sequence, UUIDv4 origin, UUIDv4 element);
  
  //
  //Functions to get an element from a sequence by index
  //
  
  ///Returns the indexed concept of the given sequence.
  UUIDv4 item(UUIDv4 sequence, int64 index);
  
  /**Returns the indexed concept of the given sequence. In this form a UUIDv4
  with an integer number is used for the index argument.*/
  UUIDv4 item(UUIDv4 sequence, UUIDv4 index);
  
  ///Returns the indexed concept in a sequence relative to an origin.
  UUIDv4 item(UUIDv4 sequence, UUIDv4 origin, int64 index);
  
  /**Returns the indexed concept in a sequence relative to an origin. In this
  form a UUIDv4 with an integer number is used for the index argument.*/
  UUIDv4 item(UUIDv4 sequence, UUIDv4 origin, UUIDv4 index);

  //
  //Functions to get the definitional name of concepts and lookup concepts by
  //  definitional name
  //
  
  /**Returns the language-localized definitional name of the concept. The
  default language of the API is used. To get the name in another language, use
  the name() function with the language filter argument.*/
  const char* name(UUIDv4 concept);
  
  /**Returns the language-localized definitional name of the concept. Note that
  language is a ISO 639-1 code such as "en" (for English) with an optional
  region code such as "en-GB" (English in United Kingdom).*/
  const char* name(const char* language, UUIDv4 concept);
  
  /**Looks up the concept by language-localized definitional name. The lookup is
  case-sensitive. If the input string is null or the definitional name does not
  exist, then the result is Undefined.*/
  UUIDv4 named(const char* name);
  
  ///Returns the fully-localized name string. For example en:A -> A.
  const char* localized(const char* name);
  
  //
  //Functions for determining the sequence properties of a concept
  //
  
  /**Returns the length of the sequence contained by the concept. For concepts
  that do not contain a sequence the return value is zero. If the concept is
  not a valid identifier, this method also returns zero.*/
  int64 length(UUIDv4 concept);
  
  ///Returns whether the concept contains a sequence.
  bool sequence(UUIDv4 concept);
  
  ///Returns whether a concept contains a cyclic sequence.
  bool cyclic(UUIDv4 concept);
  
  //
  //Functions for manipulating rational number concepts
  //
  
  ///If the concept is a number returns its numerator and zero otherwise.
  int64 numerator(UUIDv4 concept);
  
  ///If the concept is a number returns its denominator and zero otherwise.
  int64 denominator(UUIDv4 concept);
  
  /**Returns whether the UUIDv4 is an integer number. This is equivalent to
  testing whether the denominator is one. If the number is an integer, the value
  can be retrieved with the numerator() method. Note that this method assumes
  the rational is stored in reduced form and will fail to report integers with
  denominators other than one (i.e. 2/2).*/
  bool integer(UUIDv4 concept);
  
  ///Returns a UUIDv4 integer given the value specified by the argument.
  UUIDv4 integer(int64 x);
  
  /**Returns whether the UUIDv4 is a number. A number is defined by any signed
  numerator storable in the high portion of the UUIDv4 and a signed but positive
  only number in the denominator. Non-reduced rationals are still considered
  numbers; however, they not valid. The negative range of the denominator with
  the MSB turned on is reserved for identifier UUIDv4s according to RFC 4122.*/
  bool number(UUIDv4 concept);

  //
  //Function for testing for the undefined state
  //
  
  /**Returns whether the UUIDv4 represents the Undefined state. Undefined is the
  result of any operation that did not produce a meaningful value and is
  equivalent to the notion of nil or empty. It consists of an all-zero UUIDv4.*/
  bool undefined(UUIDv4 concept);
  
  /**Returns whether the value is a valid identifier. Undefined is not
  considered an identifier nor are numbers.*/
  bool identifier(UUIDv4 concept);
  
  //
  //Functions for returning mapped values
  //
  
  /**Maps a two-argument key to a value. Returns Undefined if mapping
  does not exist.*/
  UUIDv4 map(UUIDv4 a, UUIDv4 b);

  /**Maps a three-argument key to a value. Returns Undefined if mapping
  does not exist.*/
  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c);

  /**Maps a four-argument key to a value. Returns Undefined if mapping
  does not exist.*/
  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d);
  
  /**Maps a five-argument key to a value. Returns Undefined if mapping
  does not exist.*/
  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d, UUIDv4 e);

  /**Maps a six-argument key to a value. Returns Undefined if mapping
  does not exist.*/
  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d, UUIDv4 e, UUIDv4 f);
  
  /**Maps a seven-argument key to a value. Returns Undefined if mapping
  does not exist.*/
  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d, UUIDv4 e, UUIDv4 f,
    UUIDv4 g);

  /**Maps a eight-argument key to a value. Returns Undefined if mapping
  does not exist.*/
  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d, UUIDv4 e, UUIDv4 f,
    UUIDv4 g, UUIDv4 h);

  /**Maps a nine-argument key to a value. Returns Undefined if mapping
  does not exist.*/
  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d, UUIDv4 e, UUIDv4 f,
    UUIDv4 g, UUIDv4 h, UUIDv4 i);

  /**Maps a ten-argument key to a value. Returns Undefined if mapping
  does not exist.*/
  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d, UUIDv4 e, UUIDv4 f,
    UUIDv4 g, UUIDv4 h, UUIDv4 i, UUIDv4 j);

@@@@@1@@@@@

  ///Class containing interval calculation methods.
  class Intervals
  {
    public:
    
    ///Sorts two pitches in ascending order.
    static void ascending(UUIDv4& x, UUIDv4& y)
    {
      UUIDv4 d = direction(x, y);
      if(undefined(d)) x = y = Undefined;
      else if(d == Below) d = x, x = y, y = d;
    }
    
    /**Returns the direction the pitches are ordered in as Below/Unison/Above.
    If the result can not be determined, Undefined is returned.*/
    static UUIDv4 direction(UUIDv4 x, UUIDv4 y)
    {
      UUIDv4 a = index(Pitches, x, y);
      if(undefined(a)) return Undefined;
      else if(numerator(a) == 0) return Unison;
      else return (numerator(a) < 0 ? Below : Above);
    }

    /**Calculates the number of octaves beyond the pitches' simple interval. The
    algorithm takes the difference of the octave number and then subtracts one
    if the pitches cross an octave number boundary when folded into the same
    octave. Note that since an octave is a simple interval, this function
    returns zero for pitches at a distance of an octave (and distance returns
    Octave).*/
    static UUIDv4 octaves(UUIDv4 x, UUIDv4 y)
    {
      ascending(x, y);
      if(undefined(x) || undefined(y)) return Undefined;
      if(map(DiatonicPitch, x) == map(DiatonicPitch, y))
        return integer(0);
      return integer(numerator(map(y, Octave)) - numerator(
        map(x, Octave)) - (numerator(index(C, map(x, Letter),
        map(y, Letter))) <= 0 ? 1 : 0));
    }
    
    /**Gets the simple intervallic distance. A simple interval is at most one
    octave in distance.*/
    static UUIDv4 distance(UUIDv4 x, UUIDv4 y)
    {
      ascending(x, y);
      UUIDv4 xl = map(x, Letter), yl = map(y, Letter);
      if(xl == yl && map(x, Octave) != map(y, Octave))
        return Octave; //Special case: tell octaves apart from unisons
      return item(Distances, index(xl, yl));
    }
    
    ///Returns the quality of the interval between the two pitches.
    static UUIDv4 quality(UUIDv4 x, UUIDv4 y)
    {
      ascending(x, y);
      UUIDv4 d = index(map(x, Letter), map(y, Letter));
      return item(
        item(DistanceQualities, d), //quality sequence
        item(map(map(x, Letter), QualitiesByLetter), d), //origin
        index(Accidentals, map(Accidental, x), //accidental delta
          map(Accidental, y)));
    }
    
    ///Gets the simple interval between two pitches.
    static UUIDv4 interval(UUIDv4 x, UUIDv4 y)
    {
      return map(distance(x, y), quality(x, y));
    }
    
    ///Transposes a pitch in a direction by an interval and optionally octaves.
    static UUIDv4 transpose(UUIDv4 pitch, UUIDv4 interval,
      UUIDv4 direction, UUIDv4 octaves)
    {
      //Make sure direction is either above or below.
      if(direction != Above && direction != Below)
        return Undefined;
      
      //Get the components...
      UUIDv4 l = map(pitch, Letter), a = map(pitch, Accidental),
        o = map(pitch, Octave), d = map(interval, Distance),
        q = map(interval, Quality);
      UUIDv4 l_result = Undefined, a_result = Undefined,
        o_result = Undefined;
      
      //Get the direction sign.
      int64 s = (direction == Above ? 1 : -1);

      //Get the letter...
      UUIDv4 di = index(Distances, d);
      if(!integer(di)) return Undefined;
      int64 n_di = numerator(di);
      l_result = item(l, s * n_di);

      //Get the accidental...
      UUIDv4 a_delta = index(item(DistanceQualities, di),
        item(map((s == 1 ? l : l_result), QualitiesByLetter), n_di), q);
      if(undefined(a_delta)) return Undefined;
      a_result = item(Accidentals, a, s * numerator(a_delta));
      
      //Determine if the note crossed an octave boundary.
      int64 old_l_index =
        (numerator(index(mica::Letters, mica::C, l)) + 7) % 7;
      int64 new_l_index =
        (numerator(index(mica::Letters, mica::C, l_result)) + 7) % 7;
      bool crossed_octave =
        s == 1 ? new_l_index < old_l_index : old_l_index < new_l_index;
        
      //Get the octave...
      if(!integer(o) || !integer(octaves) || numerator(octaves) < 0)
        return Undefined;
      o_result = integer(numerator(o) + s *
        (numerator(octaves) + (d == Octave || crossed_octave)));
        
      return map(l_result, a_result, o_result);
    }
    
    ///Transposes a pitch in a direction by a simple interval and some octaves.
    static UUIDv4 transpose(UUIDv4 pitch, UUIDv4 interval,
      UUIDv4 direction, int64 octaves = 0)
    {
      UUIDv4 octaves_as_id = integer(octaves);
      return transpose(pitch, interval, direction, octaves_as_id);
    }
  };
}

#ifdef MICA_COMPILE_INLINE

//-------//
//Helpers//
//-------//

namespace mica_internal
{
  using namespace MICA_NAMESPACE;
  
  /*Based on the strcmp algorithm and rewritten here to avoid including the
  C standard library.*/
  int lexicographic_compare(const char* s1, const char* s2);
  int lexicographic_compare(const char* s1, const char* s2)
  {
    unsigned char c1, c2;
    do
    {
      c1 = static_cast<unsigned char>(*s1++);
      c2 = static_cast<unsigned char>(*s2++);
      if(!c1)
        return -int(c2);
    } while(c1 == c2);
    return int(c1) - int(c2);
  }
  
  //Generic algorithm for binary search requiring only < and > operators on T.
  template <class T> int64 binary_search(const T* source,
    int64 items, const T& needle);
  template <class T> int64 binary_search(const T* source,
    int64 items, const T& needle)
  {
    //Precondition: array is sorted before doing binary search.
    int64 low = 0;
    int64 high = items - 1;
    int64 mid;
    while(low <= high)
    {
      mid = (high - low) / 2 + low; //Variant of (low + high) / 2
      const T& mid_val = source[mid];
      
      if(mid_val < needle)
        low = mid + 1;
      else if(mid_val > needle)
        high = mid - 1;
      else
        return mid; //Key found
    }
    return -1; //Key not found
  }
  
  /*Standard MICA hashing algorithm. Takes in two 64-bit integers and produces
  a single 64-bit integer out. This is used for simplifying map lookup.*/
  uint64 hash(uint64 a, uint64 b);
  uint64 hash(uint64 a, uint64 b)
  {
    //Use multiply with carry random number generation with inputs as seed.
    const uint64 low16 = uint64(0xffff);
    const uint64 low32 = uint64(0xffffffff);
    uint64 z_h, w_h, z_l, w_l;
    z_h = 36969 * ((a >> 32) & low16) + (a >> 48);
    w_h = 18000 * ((b >> 32) & low16) + (b >> 48);
    z_l = 36969 * (a & low16) + ((a & low32) >> 16);
    w_l = 18000 * (b & low16) + ((b & low32) >> 16);
    return (((z_l << 16) + w_l) & low32) + (((z_h << 16) + w_h) << 32);
  }
  
  //Hashing for UUIDv4s
  UUIDv4 hash(UUIDv4 a, UUIDv4 b);
  UUIDv4 hash(UUIDv4 a, UUIDv4 b)
  {
    UUIDv4 c = {hash(a.high, b.high), hash(a.low, b.low)};
    return c;
  }
  
  //Undefined return values
  static UUIDv4 undefined_value = {0, 0};
  static const char* empty_string = "";
  static const char* integer_string = "(integer)";
  static const char* ratio_string = "(ratio)";
  static const char* undefined_string = "(undefined)";
  
  //------------------------//
  //Binary Search Structures//
  //------------------------//
  
  //Binary Search Structure: Key -> Concept
  struct Key_Concept
  {
    //Sorted input
    const char* key;
    
    //Output
    UUIDv4 concept;
    
    bool operator < (const Key_Concept& other) const
    {
      return lexicographic_compare(key, other.key) < 0;
    }
    
    bool operator > (const Key_Concept& other) const
    {
      return lexicographic_compare(key, other.key) > 0;
    }
  };
  
  //Binary Search Structure: Concept -> Length
  struct Concept_Length
  {
    //Sorted input
    UUIDv4 concept;
    
    /*Output. Note that:
      length < 0  -> cyclic sequence
      length > 0  -> path sequence (does not cycle)
      length = 0  -> not a sequence
      |length|    -> sequence length
    */
    int64 length;
  
    bool operator < (const Concept_Length& other) const
    {
      return concept < other.concept;
    }
    
    bool operator > (const Concept_Length& other) const
    {
      return concept > other.concept;
    }
  };
  
  //Binary Search Structure: {Sequence, Concept} -> Index
  struct SequenceConcept_Index
  {
    //Sorted inputs (sorted by sequence first, concept second)
    UUIDv4 sequence;
    UUIDv4 concept;
  
    //Output
    int64 index;
    
    bool operator < (const SequenceConcept_Index& other) const
    {
      return sequence < other.sequence ||
        (sequence == other.sequence && concept < other.concept);
    }
    
    bool operator > (const SequenceConcept_Index& other) const
    {
      return sequence > other.sequence ||
        (sequence == other.sequence && concept > other.concept);
    }
  };
  
  //Binary Search Structure: {Sequence, Index} -> Concept
  struct SequenceIndex_Concept
  {
    //Sorted inputs (sorted by sequence first, index second)
    UUIDv4 sequence;
    int64 index;
    
    //Output
    UUIDv4 concept;
    
    bool operator < (const SequenceIndex_Concept& other) const
    {
      return sequence < other.sequence ||
        (sequence == other.sequence && index < other.index);
    }
    
    bool operator > (const SequenceIndex_Concept& other) const
    {
      return sequence > other.sequence ||
        (sequence == other.sequence && index > other.index);
    }
  };
  
  //Binary Search Structure: {Concept, Language} -> Key
  struct ConceptLanguage_Key
  {
    //Sorted inputs (sorted by concept first, language second)
    UUIDv4 concept;
    const char* language;
    
    //Output
    const char* key;
    
    bool operator < (const ConceptLanguage_Key& other) const
    {
      return concept < other.concept ||
        (concept == other.concept &&
        lexicographic_compare(language, other.language) < 0);
    }
    
    bool operator > (const ConceptLanguage_Key& other) const
    {
      return concept > other.concept ||
        (concept == other.concept &&
        lexicographic_compare(language, other.language) > 0);
    }
  };
  
  //Binary Search Structure: Map -> Concept
  struct Map_Concept
  {
    //Sorted input
    UUIDv4 map;
    
    //Output
    UUIDv4 concept;
  
    bool operator < (const Map_Concept& other) const
    {
      return map < other.map;
    }
    
    bool operator > (const Map_Concept& other) const
    {
      return map > other.map;
    }
  };
}

/*
usage
usage  Music Information Concept Archive
usage  ---------------------------------
usage
usage  This file is easily grepped.
usage
usage  grep usage mica-data.h
usage
usage  The following categories are available to search:
usage  KC   Key -> Concept
usage  CS   Concept -> "Sequence Length"
usage  SCI  {Sequence, Concept} -> Index
usage  SIC  {Sequence, Index} -> Concept
usage  CLK  {Concept, Language} -> Key
usage  MC   Map Hashed Concepts -> Concept
usage
usage  And can be searched like this:
usage  grep CATEGORY.*SEARCHTERM mica-data.h
usage
usage  For example:
usage  grep KC.*Letters mica-data.h
usage
*/

namespace MICA_NAMESPACE
{
@@@@@2@@@@@
}

@@@@@3@@@@@

//------------------//
//Additional Helpers//
//------------------//

namespace mica_internal
{
  //Gets the raw length field from the Concept_Length dictionary.
  int64 length_raw(UUIDv4 concept);
  int64 length_raw(UUIDv4 concept)
  {
    Concept_Length needle = {concept, 0};
    int64 index;
    return ((index = binary_search(Concept_Length_Lookup, Concept_Length_Size,
      needle)) < 0)  ?  0  : Concept_Length_Lookup[index].length;
  }
  
  //Maps an Undefined terminated set of inputs to an output.
  UUIDv4 map_any(UUIDv4* inputs);
  UUIDv4 map_any(UUIDv4* inputs)
  {
    //If null, return Undefined.
    if(!inputs) return undefined_value;
    
    //Count the number of map inputs.
    int64 n = 0;
    {
      UUIDv4* inputs_ptr = inputs;
      while(*inputs_ptr++ != undefined_value) n++;
    }
    
    //Must have at least two items in order to map.
    if(n < 2) return undefined_value;
    
    //Sort the inputs.
    for(int64 i = 1; i < n; i++)
    {
      for(int64 j = 0; j < i; j++)
      {
        if(inputs[j] > inputs[i])
        {
          UUIDv4 t = inputs[i];
          inputs[i] = inputs[j];
          inputs[j] = t;
        }
      }
    }
    
    //Hash each item together.
    UUIDv4 x = inputs[0];
    for(int64 i = 1; i < n; i++)
      x = hash(x, inputs[i]);

    //Lookup the hash.
    Map_Concept needle = {x, undefined_value};
    int64 index;
    return ((index = binary_search(Map_Concept_Lookup, Map_Concept_Size,
      needle)) < 0)  ?  undefined_value  : Map_Concept_Lookup[index].concept;
  }
}

//---------------//
//API Definitions//
//---------------//

namespace MICA_NAMESPACE
{
  /*Bring in the internals. They will not be visible to the end-user, nor
  pollute the global namespace.*/
  using namespace mica_internal;
  
  //----//
  //UUIDv4//
  //----//
  
  /*In MICA, a short-form comparison is one in which the lower bits of one of
  the comparison terms has been set to zero. This allows a MICA concept to be
  compactly stored as the high 64 bits and promoted to the full 128-bits
  later.*/
  
  bool UUIDv4::operator < (const UUIDv4& other) const
  {
    //Check for short-form comparison: only compare high word.
    if((!low || !other.low) && high && other.high)
      return high < other.high;
      
    //Long-form comparison: compare high and low word.
    return (high < other.high) || (high == other.high && low < other.low);
  }
  
  bool UUIDv4::operator > (const UUIDv4& other) const
  {
    return !(*this == other) && !(*this < other);
  }
  
  bool UUIDv4::operator == (const UUIDv4& other) const
  {
    //Check for short-form comparison: only compare high word.
    if((!low || !other.low) && high && other.high)
      return high == other.high;
      
    //Long-form comparison: compare high and low word.
    return high == other.high && low == other.low;
  }
  
  bool UUIDv4::operator != (const UUIDv4& other) const
  {
    return !(*this == other);
  }

#ifdef PRIM_LIBRARY
  UUIDv4::operator PRIM_NAMESPACE::String () const
  {
    if(denominator(*this) != 0)
    {
      PRIM_NAMESPACE::Ratio r(numerator(*this), denominator(*this));
      return PRIM_NAMESPACE::String(r);
    }
    return PRIM_NAMESPACE::String(name(*this));
  }
  
  PRIM_NAMESPACE::Ratio UUIDv4::ratio () const
  {
    if(denominator(*this) != 0)
    {
      PRIM_NAMESPACE::Ratio r(numerator(*this), denominator(*this));
      return r;
    }
    return PRIM_NAMESPACE::Ratio();
  }
#else
  UUIDv4::operator const char* () const
  {
    return name(*this);
  }
#endif

  //-------//
  //Concept//
  //-------//
  Concept::Concept()
  {
    high = low = 0;
  }
  
  Concept::Concept(const UUIDv4& other)
  {
    high = other.high;
    low = other.low;
  }
  
  UUIDv4 promote(uint64 shortform);
  UUIDv4 promote(uint64 shortform)
  {
    Concept_Length needle = {{uint64(shortform), 0U}, 0};
    int64 result = binary_search(Concept_Length_Lookup, Concept_Length_Size,
      needle);
    UUIDv4 u = {0, 0};
    if(result >= 0)
      u = Concept_Length_Lookup[result].concept;
    return u;
  }
  
  Concept::Concept(int64 shortform)
  {
    UUIDv4 u = promote(uint64(shortform));
    high = u.high;
    low = u.low;
  }
  
  Concept::Concept(uint64 shortform)
  {
    UUIDv4 u = promote(shortform);
    high = u.high;
    low = u.low;
  }
  
#ifdef PRIM_LIBRARY
  Concept::Concept(PRIM_NAMESPACE::Ratio other)
  {
    high = uint64(other.Numerator());
    low = uint64(other.Denominator());
  }
  
  Concept::Concept(const PRIM_NAMESPACE::Value& other)
  {
    if(other.IsInteger())
    {
      high = uint64(other.AsInteger());
      low = 1;
    }
    else if(other.IsString())
    {
      mica::Concept u(other.AsString().Merge());
      high = u.high;
      low = u.low;
    }
    else if(other.IsRatio())
    {
      PRIM_NAMESPACE::Ratio r = other.AsRatio();
      high = uint64(r.Numerator());
      low = uint64(r.Denominator());
    }
    else
      high = low = 0;
  }
  
  Concept::Concept(const char* other)
  {
    PRIM_NAMESPACE::String s = other;
    PRIM_NAMESPACE::Ratio r = s;
    //Automatically accept English form if nothing is specified.
    if(!s.Contains(":"))
      s.Prepend("en:");
    if(r.IsEmpty())
      *this = named(s.Merge());
    else
      *this = Concept(r);
  }
#else
  Concept::Concept(const char* other)
  {
    *this = named(other);
  }  
#endif

    
  //---------//
  //Functions//
  //---------//
  
  UUIDv4 index(UUIDv4 sequence, UUIDv4 element)
  {
    SequenceConcept_Index needle = {sequence, element, 0};
    int64 index;
    if((index = binary_search(SequenceConcept_Index_Lookup,
      SequenceConcept_Index_Size, needle)) < 0)
        return undefined_value;
    UUIDv4 value = {uint64(SequenceConcept_Index_Lookup[index].index), 1};
    return value;
  }
  
  UUIDv4 index(UUIDv4 sequence, UUIDv4 origin, UUIDv4 element)
  {
    UUIDv4 o = index(sequence, origin), e = index(sequence, element);
    if(!integer(o) || !integer(e))
      return undefined_value;
    UUIDv4 r = {uint64(numerator(e) - numerator(o)), 1};
    return r;
  }
  
  UUIDv4 item(UUIDv4 sequence, int64 index)
  {
    //Calculate canonical index for cyclic sequences.
    {
      int64 n = length_raw(sequence);
      if(n < 0)
      {
        n = -n;
        if(index >= 0)
          index = index % n;
        else
          index = (n - ((-index) % n)) % n;
      }
    }
    
    SequenceIndex_Concept needle = {sequence, index, undefined_value};
    return ((index = binary_search(SequenceIndex_Concept_Lookup,
      SequenceIndex_Concept_Size, needle)) < 0)  ?  undefined_value  :
      SequenceIndex_Concept_Lookup[index].concept;
  }
  
  UUIDv4 item(UUIDv4 sequence, UUIDv4 index)
  {
    return integer(index)  ?  item(sequence, numerator(index))  :
      undefined_value;
  }
  
  UUIDv4 item(UUIDv4 sequence, UUIDv4 origin, int64 index_)
  {
    UUIDv4 o = index(sequence, origin);
    return integer(o)  ?  item(sequence, numerator(o) + index_)  :
      undefined_value;
  }
  
  UUIDv4 item(UUIDv4 sequence, UUIDv4 origin, UUIDv4 index)
  {
    return integer(index)  ?  item(sequence, origin, numerator(index))  :
      undefined_value;
  }
  
  int64 length(UUIDv4 concept)
  {
    int64 l = length_raw(concept);
    return l < 0 ? -l : l;
  }
  
  bool sequence(UUIDv4 concept)
  {
    return length_raw(concept) != 0;
  }
  
  bool cyclic(UUIDv4 concept)
  {
    return length_raw(concept) < 0;
  }
  
  /*Removes the localization prefix on a string. For example, en:Letters ->
  Letters. This is done by simply advancing to after the colon.*/
  const char* localized(const char* s)
  {
    const char* t = s;
    while(t)
      if(*t++ == ':')
        return t;
    return s;
  }
  
  const char* name(UUIDv4 concept)
  {
    return name("en", concept);
  }
  
  const char* name(const char* language, UUIDv4 concept)
  {
    if(undefined(concept))
      return undefined_string;
    else if(integer(concept))
      return integer_string;
    else if(number(concept))
      return ratio_string;
    if(!language) return empty_string;
    ConceptLanguage_Key needle = {concept, language, empty_string};
    int64 i;
    return ((i = binary_search(ConceptLanguage_Key_Lookup,
      ConceptLanguage_Key_Size, needle)) < 0)  ?  empty_string  :
      ConceptLanguage_Key_Lookup[i].key;
  }
  
  UUIDv4 named(const char* name)
  {
    if(!name) return undefined_value;
    Key_Concept needle = {name, undefined_value};
    int64 index;
    return ((index = binary_search(Key_Concept_Lookup, Key_Concept_Size,
      needle)) < 0)  ?  undefined_value  :  Key_Concept_Lookup[index].concept;
  }
  
  int64 numerator(UUIDv4 concept)
  {
    return number(concept) ? int64(concept.high) : 0;
  }
  
  int64 denominator(UUIDv4 concept)
  {
    return number(concept) ? int64(concept.low) : 0;
  }
  
  bool integer(UUIDv4 concept)
  {
    return number(concept) && concept.low == 1;
  }
  
  UUIDv4 integer(int64 x)
  {
    UUIDv4 i = {uint64(x), 1};
    return i;
  }
  
  bool number(UUIDv4 concept)
  {
    return int64(concept.low) > 0;
  }
  
  bool undefined(UUIDv4 concept)
  {
    return concept.high == 0 && concept.low == 0;
  }
  
  bool identifier(UUIDv4 concept)
  {
    if(!concept.high && !concept.low)
      return false;
    Concept_Length needle = {concept, 0};
    return binary_search(Concept_Length_Lookup, Concept_Length_Size,
      needle) >= 0;
  }

  UUIDv4 map(UUIDv4 a, UUIDv4 b)
  {
    UUIDv4 inputs[3] = {a, b, undefined_value};
    return map_any(inputs);
  }

  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c)
  {
    UUIDv4 inputs[4] = {a, b, c, undefined_value};
    return map_any(inputs);
  }

  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d)
  {
    UUIDv4 inputs[5] = {a, b, c, d, undefined_value};
    return map_any(inputs);
  }
  
  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d, UUIDv4 e)
  {
    UUIDv4 inputs[6] = {a, b, c, d, e, undefined_value};
    return map_any(inputs);
  }

  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d, UUIDv4 e, UUIDv4 f)
  {
    UUIDv4 inputs[7] = {a, b, c, d, e, f, undefined_value};
    return map_any(inputs);
  }
  
  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d, UUIDv4 e, UUIDv4 f,
    UUIDv4 g)
  {
    UUIDv4 inputs[8] = {a, b, c, d, e, f, g, undefined_value};
    return map_any(inputs);
  }

  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d, UUIDv4 e, UUIDv4 f,
    UUIDv4 g, UUIDv4 h)
  {
    UUIDv4 inputs[9] = {a, b, c, d, e, f, g, h, undefined_value};
    return map_any(inputs);
  }

  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d, UUIDv4 e, UUIDv4 f,
    UUIDv4 g, UUIDv4 h, UUIDv4 i)
  {
    UUIDv4 inputs[10] = {a, b, c, d, e, f, g, h, i, undefined_value};
    return map_any(inputs);
  }

  UUIDv4 map(UUIDv4 a, UUIDv4 b, UUIDv4 c, UUIDv4 d, UUIDv4 e, UUIDv4 f,
    UUIDv4 g, UUIDv4 h, UUIDv4 i, UUIDv4 j)
  {
    UUIDv4 inputs[11] = {a, b, c, d, e, f, g, h, i, j, undefined_value};
    return map_any(inputs);
  }
}
#endif

#endif
