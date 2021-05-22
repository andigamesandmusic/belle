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

package MICA;

import java.util.Arrays;
import java.util.Comparator;
import java.util.UUID;
import MICA.MICAData;
import MICA.MICADefinitions;

public class MICA {
    /* Conventions:
      * All functions are one unabbreviated word.
      
      * All functions are safe and will accept any input. If the result is
        indeterminate, then Undefined is returned. A slight exception to this
        principle is the length() function, which returns zero if the argument
        is invalid.
        
      * Arguments which filter the selection appear first and the argument
        indicating the thing being looked for appears last:
        
        - MICA.index(MICA.Letters, MICA.C, MICA.G)
          means: in Letters relative to C, get index of G.
          
        - MICA.name("en", MICA.Letters)
          means: in English, get the name of Letters
          
        - MICA.item(MICA.Letters, MICA.C, 4)
          means: in Letters relative to C, get the fourth element
    */
    
    //
    //Functions to get the index of an element in a sequence
    //
    
    /**Returns the index of a concept in a given sequence as a number. If the
    element is not in the sequence then Undefined is returned.*/
    public static UUID index(UUID sequence, UUID element) {
        Integer elementResult = MICAData._sequenceConcept__index.get(
            new MICASequenceConcept(coerce(sequence), coerce(element)));
        if(elementResult == null)
            return MICA.Undefined;
        return integer((int)elementResult);
    }
      
    /**Returns the index of an element in a given sequence relative to another
    concept acting as the origin. If the index does not exist then Undefined is
    returned.*/
    public static UUID index(UUID sequence, UUID origin, UUID element) {
        Integer originIndex = MICAData._sequenceConcept__index.get(
                new MICASequenceConcept(coerce(sequence), coerce(origin)));
        Integer elementIndex = MICAData._sequenceConcept__index.get(
                new MICASequenceConcept(coerce(sequence), coerce(element)));
        if(originIndex == null || elementIndex == null)
            return MICA.Undefined;
        return integer((int)elementIndex - (int)originIndex);
    }
    
    //
    //Functions to get an element from a sequence by index
    //
    
    /**Returns the indexed concept of the given sequence.*/
    public static UUID item(UUID sequence, int index) {
        return item(sequence, integer((long)index));
    }
    
    /**Returns the indexed concept of the given sequence. In this form a UUID
    with an integer number is used for the index argument.*/
    public static UUID item(UUID sequence, UUID index) {
        sequence = coerce(sequence);
        if(!integer(index))
            return MICA.Undefined;
        long rawIndex = numerator(index);
        if(rawIndex < 0 || rawIndex > 0x7fffffffL)
            return MICA.Undefined;
        int intIndex = (int)rawIndex;
        
        //Wrap for cyclic sequences.
        {
            int n = length(coerce(sequence));
            if(cyclic(coerce(sequence)))
            {
                if(intIndex >= 0)
                    intIndex = intIndex % n;
                else
                    intIndex = (n - ((-intIndex) % n)) % n;
            }
        }
        
        return coerce(MICAData._sequenceIndex__concept.get(
            new MICASequenceIndex(coerce(sequence), new Integer(intIndex))));
    }
    
    /**Returns the indexed concept in a sequence relative to an origin.*/
    public static UUID item(UUID sequence, UUID origin, int index) {
        return item(sequence, origin, integer((long)index));
    }
    
    /**Returns the indexed concept in a sequence relative to an origin. In this
    form a UUID with an integer number is used for the index argument.*/
    public static UUID item(UUID sequence, UUID origin, UUID index) {
        UUID originIndex = index(coerce(sequence), coerce(origin));
        if(originIndex == null)
            return MICA.Undefined;
        if(!integer(index))
            return MICA.Undefined;
        long rawIndex = numerator(index) + numerator(originIndex);
        if(rawIndex < 0 || rawIndex > 1000000)
            return MICA.Undefined;
        int intIndex = (int)rawIndex;
        
        return item(coerce(sequence), intIndex);
    }
    
    //
    //Functions to get the definitional name of concepts and lookup concepts by
    //  definitional name
    //
    
    /**Returns the language-localized definitional name of the concept. The
    default language of the API is used. To get the name in another language,
    use the name() function with the language filter argument.*/
    public static String name(UUID concept) {
        return name("en", concept);
    }
    
    /**Returns the language-localized definitional name of the concept. Note 
    that language is a ISO 639-1 code such as "en" (for English) with an
    optional region code such as "en-GB" (English in United Kingdom).*/
    public static String name(String language, UUID concept) {
        if(language == null)
            return "";
        if(number(concept)) {
            String x = new String();
            x += numerator(concept);
            if(!integer(concept)) {
                x += "/";
                x += denominator(concept);
            }
            return x;
        }
        return coerce(MICAData._conceptLanguage__key.get(
            new MICAConceptLanguage(coerce(concept), language)));
    }
    
    /**Looks up the concept by language-localized definitional name. The lookup 
    is case-sensitive. If the input string is null or the definitional name
    does not exist, then the result is Undefined.*/
    public static UUID named(String name) {
        if(name == null)
            return MICA.Undefined;
        return coerce(MICAData._key__concept.get(name));
    }
    
    /**Returns the fully-localized name string. For example en:A -> A.*/
    public static String localized(String name) {
        if(name == null)
            return "";
        if(!name.contains(":"))
            return name;
        else
            return name.substring(name.indexOf(":") + 1);
    }
    
    ///Returns the concept definition string for the current language.
    public static String definition(UUID concept) {
        if(!identifier(concept))
            return "";
        String s = MICADefinitions._concept__definitions.get(concept);
        if(s == null)
            return "";
        return s;
    }
    
    //
    //Functions for determining the sequence properties of a concept
    //
    
    /**Returns the length of the sequence contained by the concept. For concepts
    that do not contain a sequence the return value is zero. If the concept is
    not a valid identifier, this method also returns zero.*/
    public static int length(UUID concept) {
        if(!identifier(concept))
            return 0;
        Integer n = MICAData._concept__length.get(concept);
        if(n == null)
            return 0;
        return Math.abs((int)n);
    }
     
    /**Returns whether the concept contains a sequence.*/
    public static boolean sequence(UUID concept) {
        return length(concept) > 0;
    }
    
    /**Returns whether a concept contains a cyclic sequence.*/
    public static boolean cyclic(UUID concept) {
        return MICAData._concept__length.get(concept) < 0;
    }
    
    //
    //Functions for manipulating rational number concepts
    //
    
    /**If the concept is a number returns its numerator and zero otherwise.*/
    public static long numerator(UUID concept) {
        if(!number(concept))
            return 0;
        return concept.getMostSignificantBits();
    }
    
    /**If the concept is a number returns its denominator and zero otherwise.*/
    public static long denominator(UUID concept) {
        if(!number(concept))
            return 0;
        return concept.getLeastSignificantBits();
    }
    
    /**Returns whether the UUID is an integer number. This is equivalent to
    testing whether the denominator is one. If the number is an integer, the
    value can be retrieved with the numerator() method. Note that this method
    assumes the rational is stored in reduced form and will fail to report
    integers with denominators other than one (i.e. 2/2).*/
    public static boolean integer(UUID concept) {
        if(concept == null)
            return false;
        if(concept.getLeastSignificantBits() == 1)
            return true;
        return false;
    }
    
    /**Returns a UUID integer given the value specified by the argument.*/
    public static UUID integer(long x) {
        return new UUID(x, 1);
    }
    
    /**Returns whether the UUID is a number. A number is defined by any signed
    numerator storable in the high portion of the UUID and a signed but positive
    only number in the denominator. Non-reduced rationals are still considered
    numbers; however, they not valid. The negative range of the denominator with
    the MSB turned on is reserved for identifier UUIDs according to RFC 4122.*/
    public static boolean number(UUID concept) {
        if(concept == null)
            return false;
        if(concept.getLeastSignificantBits() > 0)
            return true;
        return false;
    }
    
    //
    //Functions for testing for type
    //
    
    /**Returns whether the value is a valid identifier. Undefined is not
    considered an identifier nor are numbers.*/
    public static boolean identifier(UUID concept) {
        if(undefined(concept))
            return false;
        if(MICAData._concept__length.get(concept) != null)
            return true;
        return false;
    }
    
    /**Returns whether the UUID represents the Undefined state. Undefined is the
    result of any operation that did not produce a meaningful value and is
    equivalent to the notion of nil or empty. It consists of an all-zero UUID.*/
    public static boolean undefined(UUID concept) {
        if(concept == null)
            return true;
        if(concept.equals(MICA.Undefined))
            return true;
        return false;
    }
    
    //
    //Hash function
    //
    
    /**Standard MICA hashing algorithm. Takes in two 64-bit integers and
    produces a single 64-bit integer out. This is used for simplifying map
    lookup.*/
    private static long hash(long a, long b)
    {
        //Use multiply with carry random number generation with inputs as seed.
        long low16 = 0xffffL;
        long low32 = 0xffffffffL;
        long z_h, w_h, z_l, w_l;
        z_h = 36969L * ((a >>> 32L) & low16) + (a >>> 48L);
        w_h = 18000L * ((b >>> 32L) & low16) + (b >>> 48L);
        z_l = 36969L * (a & low16) + ((a & low32) >>> 16L);
        w_l = 18000L * (b & low16) + ((b & low32) >>> 16L);
        return (((z_l << 16L) + w_l) & low32) + (((z_h << 16L) + w_h) << 32L);
    }
    
    /**Standard MICA hashing algorithm for UUIDs.*/
    private static UUID hash(UUID a, UUID b) {
        if(a == null || b == null)
            return MICA.Undefined;
        return new UUID(
                hash(a.getMostSignificantBits(), b.getMostSignificantBits()),
                hash(a.getLeastSignificantBits(), b.getLeastSignificantBits()));
    }
    
    /**Coerces null to Undefined.*/
    private static UUID coerce(UUID a) {
        if(undefined(a))
            a = MICA.Undefined;
        return a;
    }
    
    /**Coerces null to "".*/
    private static String coerce(String a) {
        if(a == null)
            a = "";
        return a;
    }
    
    //
    //Functions for returning mapped values
    //
    
    /**Compares longs according to an unsigned comparison.*/
    private static int unsignedComparison(long x, long y) {
        if(x == y)
            return 0;
        else if((x < y) ^ (x < 0) ^ (y < 0))
            return -1;
        else
            return 1;
    }
    
    /**Custom UUID comparator using canonical unsigned comparison.*/
    private static Comparator<UUID> UUIDUnsignedComparator 
                                          = new Comparator<UUID>() {
        public int compare(UUID a, UUID b) {
            long ah = a.getMostSignificantBits();
            long al = a.getLeastSignificantBits();
            long bh = b.getMostSignificantBits();
            long bl = b.getLeastSignificantBits();
            if(ah == bh && al == bl)
                return 0;
            else if(unsignedComparison(ah, bh) == -1 ||
                (ah == bh && unsignedComparison(al, bl) == -1))
                    return -1;
            else
                return 1;
        }
    };
    
    /**Maps an arbitrary number of concepts.*/
    private static UUID mapAny(UUID[] mapArray) {
        Arrays.sort(mapArray, UUIDUnsignedComparator);
        UUID hashedConcept = mapArray[0];
        for(int i = 1; i < mapArray.length; i++)
            hashedConcept = hash(hashedConcept, mapArray[i]);
        return coerce(MICAData._hashedConcept__concept.get(hashedConcept));
    }
    
    /**Maps a two-argument key to a value. Returns Undefined if mapping
    does not exist.*/
    public static UUID map(UUID a, UUID b) {
        UUID [] mapArray = new UUID[2];
        mapArray[0] = a;
        mapArray[1] = b;
        return mapAny(mapArray);
    }
    
    /**Maps a three-argument key to a value. Returns Undefined if mapping
    does not exist.*/
    public static UUID map(UUID a, UUID b, UUID c) {
        UUID [] mapArray = new UUID[3];
        mapArray[0] = a;
        mapArray[1] = b;
        mapArray[2] = c;
        return mapAny(mapArray);
    }
    
    /**Maps a four-argument key to a value. Returns Undefined if mapping
    does not exist.*/
    public static UUID map(UUID a, UUID b, UUID c, UUID d) {
        UUID [] mapArray = new UUID[4];
        mapArray[0] = a;
        mapArray[1] = b;
        mapArray[2] = c;
        mapArray[3] = d;
        return mapAny(mapArray);
    }
    
    /**Maps a five-argument key to a value. Returns Undefined if mapping
    does not exist.*/
    public static UUID map(UUID a, UUID b, UUID c, UUID d, UUID e) {
        UUID [] mapArray = new UUID[5];
        mapArray[0] = a;
        mapArray[1] = b;
        mapArray[2] = c;
        mapArray[3] = d;
        mapArray[4] = e;
        return mapAny(mapArray);
    }
    
    /**Maps a six-argument key to a value. Returns Undefined if mapping
    does not exist.*/
    public static UUID map(UUID a, UUID b, UUID c, UUID d, UUID e,
            UUID f) {
        UUID [] mapArray = new UUID[6];
        mapArray[0] = a;
        mapArray[1] = b;
        mapArray[2] = c;
        mapArray[3] = d;
        mapArray[4] = e;
        mapArray[5] = f;
        return mapAny(mapArray);
    }
    
    /**Maps a seven-argument key to a value. Returns Undefined if mapping
    does not exist.*/
    public static UUID map(UUID a, UUID b, UUID c, UUID d, UUID e,
            UUID f, UUID g) {
        UUID [] mapArray = new UUID[7];
        mapArray[0] = a;
        mapArray[1] = b;
        mapArray[2] = c;
        mapArray[3] = d;
        mapArray[4] = e;
        mapArray[5] = f;
        mapArray[6] = g;
        return mapAny(mapArray);
    }
    
    /**Maps a eight-argument key to a value. Returns Undefined if mapping
    does not exist.*/
    public static UUID map(UUID a, UUID b, UUID c, UUID d, UUID e,
            UUID f, UUID g, UUID h) {
        UUID [] mapArray = new UUID[8];
        mapArray[0] = a;
        mapArray[1] = b;
        mapArray[2] = c;
        mapArray[3] = d;
        mapArray[4] = e;
        mapArray[5] = f;
        mapArray[6] = g;
        mapArray[7] = h;
        return mapAny(mapArray);
    }
    
    /**Maps a nine-argument key to a value. Returns Undefined if mapping
    does not exist.*/
    public static UUID map(UUID a, UUID b, UUID c, UUID d, UUID e,
            UUID f, UUID g, UUID h, UUID i) {
        UUID [] mapArray = new UUID[9];
        mapArray[0] = a;
        mapArray[1] = b;
        mapArray[2] = c;
        mapArray[3] = d;
        mapArray[4] = e;
        mapArray[5] = f;
        mapArray[6] = g;
        mapArray[7] = h;
        mapArray[8] = i;
        return mapAny(mapArray);
    }
    
    /**Maps a ten-argument key to a value. Returns Undefined if mapping
    does not exist.*/
    public static UUID map(UUID a, UUID b, UUID c, UUID d, UUID e,
            UUID f, UUID g, UUID h, UUID i, UUID j) {
        UUID [] mapArray = new UUID[10];
        mapArray[0] = a;
        mapArray[1] = b;
        mapArray[2] = c;
        mapArray[3] = d;
        mapArray[4] = e;
        mapArray[5] = f;
        mapArray[6] = g;
        mapArray[7] = h;
        mapArray[8] = i;
        mapArray[9] = j;
        return mapAny(mapArray);
    }
    
    //
    //MICA concepts
    //
    
@@@@@1@@@@@
}
