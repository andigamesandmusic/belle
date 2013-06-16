/*
  ==============================================================================

  Copyright 2007-2013 William Andrew Burnson. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY WILLIAM ANDREW BURNSON ''AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
  EVENT SHALL WILLIAM ANDREW BURNSON OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  ------------------------------------------------------------------------------

  This file is part of Belle, Bonne, Sage --
    The 'Beautiful, Good, Wise' C++ Vector-Graphics Library for Music Notation 

  ==============================================================================
*/

//------------------------------------------------------------------------------

/*
Tutorial 3: Using MICA, the Music Information and Concept Archive

This tutorial explains how musical information is encoded in Belle using the
MICA library, which is a separate standalone library developed by author with
the specific aim of providing a means to differentiate and relate musical
concepts in a general way.

On Mac/Linux you can build and run from the terminal using:
Scripts/MakeAndRun Tutorial3

For more information related to building, see the README.
*/

//------------------------------------------------------------------------------

/*Include Belle, Bonne, Sage and compile it in this .cpp file. See the previous
tutorials for an explanation.*/
#define BELLE_COMPILE_INLINE
#include "Belle.h"
using namespace prim;

/*MICA lives in the mica namespace. There are thousands of identifiers (the
'concepts') in the namespace so it is very important to not use
'using namespace mica' as this will pollute the global namespace with these
thousands of concepts no doubt causing name conflicts. See below for the 
recommended usage of the mica:: namespace.*/

//------------------------------------------------------------------------------

int main()
{
  //-------------------//
  //Part 1 - Using MICA//
  //-------------------//
  c >> "Using MICA:";
  c >> mica::Intervals::transpose(mica::C4, mica::PerfectOctave,
    mica::Above);
  c >> mica::Intervals::transpose(mica::G4, mica::PerfectOctave,
  mica::Above);
  c >> mica::Intervals::transpose(mica::E4, mica::MajorSeventh,
  mica::Above);
  c >> mica::Intervals::transpose(mica::C4, mica::PerfectOctave,
    mica::Below);
  c >> mica::Intervals::transpose(mica::G4, mica::PerfectOctave,
    mica::Below);
  c >> mica::Intervals::transpose(mica::E4, mica::MajorSeventh,
    mica::Below);
  /*Create a MICA Concept and assign it. Concept is the only end-user type in
  MICA.*/
  mica::Concept x = mica::C; //Assigns x to note letter C.
  
  /*Note that C is the en-cpp (English C++) identifier for the musical concept
  of C. Other natural and programming language combinations may use a different
  identifier. For example, the German identifier might be Do instead of C.*/

  /*Get the definitional name in the default language. This is an overload for
  the const char* operator. You can also call name(x) directly. Note that the
  definitional name is not necessarily the same as the identifier. This name is
  the one that would appear in a dictionary in the given language.*/
  c >> "[C]: " << x;
  c >> "[C] Name: " << name(x); //e.g. mica::name(x)

  /*Note in the previous example that the function actually called was
  mica::name(x). Because of argument-dependent name lookup, in many cases it is
  unnecessary to specify the namespace to the function. This is a C++ language
  feature and due to the way in which MICA was designed.*/
  
  //You can also get the high and low 64-bit halves of the UUID directly.
  c >> "[C]: " << x.high << ":" << x.low;

  /*The mica::Concept class is identical to the mica::UUID class except that
  Concept is automatically initialized to mica::Undefined on construction.
  Therefore user code should only use the mica::Concept subclass.*/
  mica::Concept undefined;
  mica::UUID hmmm;
  c >> "Concept initialized to: " << undefined.high << ":" << undefined.low
   ;
  c >> "UUID initialized to   : " << hmmm.high << ":" << hmmm.low;
  
  /*You can also convert a text string to a Concept. Note that since named()
  takes a const char* string, you will need the mica:: namespace to call the
  function unlike the other mica functions.*/
  //c >> "'f': " << mica::named("f"); //Means the dynamic marking for forte
  c >> "'F': " << mica::named("en:F"); //Means the pitch letter F

  /*Note of caution: in MICA a concept is defined by its underlying UUID, not by
  its definitional name (though in most cases the UUID has been generated from
  an identifier using a hashing algorithm). You should not rely on named()
  except within a single known revision of the library as names can change
  without notice in order to improve the library.*/

  //Maps are used to store relational information between multiple elements.
  c >> "[A B]: " << map(mica::A, mica::B);
  
  //Note maps are unordered so the order of arguments does not matter.
  c >> "[B A]: " << map(mica::B, mica::A);
  
  //If a mapping does not exist, Undefined is returned.
  c >> "[A Undefined]: " << map(mica::A, mica::Undefined);

  /*Some concepts in MICA are special in that they define sequences of other
  concepts. Sequences have a non-zero length and may be cyclic.*/
  c >> "[Letters] Sequence? " <<
    (sequence(mica::Letters) ? "Yes" : "No");
  c >> "[Letters] Cyclic?   " <<
    (cyclic(mica::Letters) ? "Yes" : "No");
  c >> "[Letters] Length:   " << length(mica::Letters);
  
  /*Items in a sequence are indexable through the item() method. The indexing
  is zero-based. The following enumerates the Letters sequence.*/
  c >> "[Letters]:";
  for(int i = 0, n = length(mica::Letters); i < n; i++)
    c >> " * " << item(mica::Letters, i);
  
  /*For many possible reasons, you may want to get the ith item relative to
  another item in the sequence. For example, you may want to start on C in
  Letters instead. Note that the sequence automatically wraps around to A after
  G because this sequence is cyclic.*/
  c >> "[Letters] relative to C:";
  for(int i = 0, n = length(mica::Letters); i < n; i++)
  {
    /*The sequence and origin are given first. The thing you are looking for
    always goes last.*/
    c >> " * " << item(mica::Letters, mica::C, i);
  }
  
  /*Cyclic wrapping occurs modulo the length of the sequence and works with
  negative numbers.*/
  c >> "Letters[-100]: " << item(mica::Letters, -100);
  c >> "Letters[100]: " << item(mica::Letters,  100);

  /*You may also want to lookup the index of an item in a sequence. For safety,
  MICA returns an integer UUID so that you can check for Undefined in case the
  element was not in the sequence or the sequence did not exist.*/
  c >> "Letters[C]: " << index(mica::Letters, mica::C);

  /*Just as item can have an origin specified, you can also specify an origin
  with index. The sequence and origin appear first and the element whose index
  you are looking for comes last.*/
  c >> "Letters[G - C]: " <<
    numerator(index(mica::Letters, mica::C, mica::G));

  /*When you are certain the value is safe to use, you can get the value using
  the numerator() function. If the returned integer UUID was Undefined, then
  numerator() will return zero and thus you would not be able to distinguish
  between an error and an actual index of zero.*/
  c >> "Letters[C]: " << numerator(index(mica::Letters, mica::C));

  //If the concept does not belong to the sequence, then Undefined is returned.
  c >> "Letters[Undefined]: " <<
    index(mica::Letters, mica::Undefined);

  //You can test for Undefined like you would test for any equality.
  mica::Concept r = index(mica::Letters, mica::C);
  if(r == mica::Undefined)
  {
    c >> "Aborting!";
    return -1;
  }
  
  //You can also test for failure using the undefined() function.
  if(mica::undefined(r))
  {
    c >> "Aborting!";
    return -1;
  }
  
  /*An invalid index passed to item() such as one generated from index() will
  simply propogate the Undefined to the outermost call. Therefore, you never
  need to worry about catching an error in the middle of a compound expression.
  Just let the expression carry out and if the return value is Undefined, then
  you know something went wrong.*/
  c >> "Letters[Letters[C]]: " <<
    item(mica::Letters, index(mica::Letters, mica::C));
  c >> "Letters[Letters[Undefined]]: " <<
    item(mica::Letters, index(mica::Letters, mica::Undefined));
  
  //---------------------------//
  //Part 2 - Practical Examples//
  //---------------------------//
  
  //-------//
  //Letters//
  //-------//
  c >> "\nLetters:";
  
  /*Get index of letter relative to another letter. If either of the concepts
  does not exist in the sequence, then Undefined is returned.*/
  c >> "Letters[E - C]: " <<
    numerator(index(mica::Letters, mica::C, mica::E));

  //Get 100th letter above another letter.
  c >> "Letters[C + 100]: " << item(mica::Letters, mica::C, 100);
               
  //-----------//
  //Accidentals//
  //-----------//
  c >> "\nAccidentals:";
  
  //Get index of accidental relative to another accidental.
  c >> "Accidentals[Flat - Natural]: " <<
    numerator(index(mica::Accidentals, mica::Natural, mica::Flat));
  
  //-----//
  //Notes//
  //-----//
  c >> "\nNotes:";
  
  //Get a chromatic note given a letter and accidental.
  c >> "[D Flat]: " << map(mica::D, mica::Flat);
  
  //Get a pitch given a letter, accidental, and octave number.
  c >> "[D Flat 4]: " << map(mica::D, mica::Flat, mica::integer(4));
  
  //Get a diatonic pitch (pitch with no accidental).
  c >> "[D 4]: " << map(mica::D, mica::integer(4));
  
  //Convert pitch to diatonic pitch (remove accidental).
  c >> "[DFlat4 DiatonicPitch]: " <<
    map(mica::DFlat4, mica::DiatonicPitch);
  
  //Get chromatic note of pitch.
  c >> "[DFlat4 ChromaticNote]: " <<
    map(mica::DFlat4, mica::ChromaticNote);
  
  //Get accidental of pitch.
  c >> "[DFlat4 Accidental]: " <<
    map(mica::DFlat4, mica::Accidental);
  
  //Get letter of pitch.
  c >> "[DFlat4 Letter]: " << map(mica::DFlat4, mica::Letter);
  
  //Get octave of pitch.
  c >> "[DFlat4 Octave]: " << numerator(map(mica::DFlat4, mica::Octave));

  //Get MIDI keynumber of pitch.
  c >> "[DFlat4 MIDIKeyNumber]: " <<
    numerator(map(mica::DFlat4, mica::MIDIKeyNumber));

  //Get accidental of chromatic note.
  c >> "[DFlat Accidental]: " << map(mica::DFlat, mica::Accidental);

  //Get letter of chromatic note.
  c >> "[DFlat Letter]: " << map(mica::DFlat, mica::Letter);

  //---------------//
  //Staff Positions//
  //---------------//
  c >> "\nStaff-Positions:";
  
  /*A staff position is an integer expressing the line or space a note is
  situated on relative to the middle line (or space) of a given staff. For
  example, B4 in Treble Clef is staff position 0, the center line.*/

  //Get staff position of diatonic pitch given clef.
  c >> "[D4 TrebleClef]: " << numerator(map(mica::D4, mica::TrebleClef))
   ;
  
  //Get staff position of chromatic pitch given clef.
  c >> "[[DSharp4 DiatonicPitch] BassClef]: " <<
    numerator(map(map(mica::DSharp4, mica::DiatonicPitch), mica::BassClef))
     ;
  
  //Get diatonic pitch from staff position in given clef.
  c >> "[2 TrebleClef]: " << map(mica::integer(2), mica::TrebleClef);
  
  //Get pitch from staff position in given clef with accidental.
  c >> "[[2 TrebleClef] Flat]: " <<
    map(map(mica::integer(2), mica::TrebleClef), mica::Flat);

  //----//
  //Keys//
  //----//
  c >> "\nKeys:";

  //Create key from notename and mode (Major).
  c >> "[DFlat Major]: " << map(mica::DFlat, mica::Major);

  //Create key from notename and mode (Dorian).
  c >> "[DFlat Dorian]: " << map(mica::DFlat, mica::Dorian);

  //Get key signature from key (Major).
  c >> "[DFlatMajor KeySignature]: " <<
    map(mica::DFlatMajor, mica::KeySignature);

  //Get key signature from key (Dorian).
  c >> "[DFlatDorian KeySignature]: " <<
    map(mica::DFlatDorian, mica::KeySignature);

  //Get key from key signature and mode.
  c >> "[ThreeFlats Minor]: " <<
    map(mica::ThreeFlats, mica::Minor);

  //Get mode from key.
  c >> "[DFlatMajor Mode]: " << map(mica::DFlatMajor, mica::Mode);

  //Get mode from key.
  c >> "[DFlatDorian Mode]: " << map(mica::DFlatDorian, mica::Mode);
  
  //Get the staff positions of each sharp in treble clef.
  {
    mica::Concept SharpPositions = map(mica::TrebleClef, mica::Sharp);
    c >> "Inspecting [TrebleClef Sharp]: " << SharpPositions;
    for(int i = 0; i < length(SharpPositions); i++)
      c >> " * " << numerator(item(SharpPositions, i));
  }
  
  //---------//
  //Intervals//
  //---------//
  c >> "\nIntervals:";
  
  //Combine distance with quality.
  c >> "[Fourth Perfect]: " << map(mica::Fourth, mica::Perfect);
    
  //Distance of interval.
  c >> "[PerfectFourth Distance]: " <<
    map(mica::PerfectFourth, mica::Distance);
  
  //Quality of interval.
  c >> "[PerfectFourth Quality]: " <<
    map(mica::PerfectFourth, mica::Quality);
  
  //Transpose note up interval.
  c >> "transpose(D4 MajorThird Above): " <<
    mica::Intervals::transpose(mica::D4, mica::MajorThird, mica::Above);
  
  //Transpose note down interval.
  c >> "transpose(D4 MajorThird Below): " <<
    mica::Intervals::transpose(mica::D4, mica::MajorThird, mica::Below);
  
  //Simple interval between two pitches.
  c >> "interval(D5 B3): " <<
    mica::Intervals::interval(mica::D5, mica::B3);

  //Quality of interval between two pitches.
  c >> "quality(D5 B3): " <<
    mica::Intervals::quality(mica::D5, mica::B3);

  //Distance of interval between two pitches.
  c >> "distance(D5 B3): " <<
    mica::Intervals::distance(mica::D5, mica::B3);

  //Number of extra octaves between two pitches.
  c >> "octaves(D5 B3): " <<
    numerator(mica::Intervals::octaves(mica::D5, mica::B3));

  //Position of second pitch with respect to first pitch.
  c >> "direction(D5 B3): "
    << mica::Intervals::direction(mica::D5, mica::B3);
  
  //Key signature transposition.
  c >> "[TwoSharps MinorSecond Above]: " <<
    map(mica::TwoSharps, mica::MinorSecond, mica::Above);
  
  return c.Finish();
}
