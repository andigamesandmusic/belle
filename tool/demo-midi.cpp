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

#define PRIM_COMPILE_INLINE
#define PRIM_WITH_MIDI
#include "prim.h"
using namespace prim;

int main()
{
  //Create a MIDI file.
  MIDI::File f;

  /*Set global information about divisions (default is 840). This only affects
  the accuracy or exactness to which MIDI delta times are quantized, and does
  not affect how notes are created as the Event class stores time in terms of
  fractions of a whole note offset from the beginning of the track.*/
  f.SetDivisionsPerQuarterNote(120);

  /*Optionally: you can use SMPTE mode, which creates divisions in terms of
  physical seconds. The following would divide the second into 25 frames with
  40 subdivisions, or in other words 1000 divisions per second, i.e. millisecond
  accuracy. This mode is not as easy to reimport into notation editors since it
  represents physical time instead of rhythmic time. In this mode, SetTempo
  should not be used since there is no beat, though it may have an effect on the
  player.*/
  //f.SetSMPTEMode(MIDI::SMPTE25, 40);

  //Create some tracks.
  MIDI::Track& Violin = f.Tracks.Add();
  MIDI::Track& Piano1 = f.Tracks.Add();
  MIDI::Track& Piano2 = f.Tracks.Add();

  //Set some global information.
  f.CopyrightNotice("(c) Copyright Holder");
  if(f.HasQuarterNoteDivisions())
    f.SetTempo(0, 240); //from time 0, set tempo = 240 qpm
  f.TimeSignature(0, 3, 4); //from time 0, set time signature = 3/4
  f.InstrumentName("Violin", 1); //set instrument name to Violin on channel 1
  f.InstrumentName("Piano", 2); //set instrument name to Piano on channel 2

  //Set up the violin.
  Violin.TrackName("Violin");
  Violin.ProgramChange(0, 40, 1); //from time 0, set to program 40, on ch. 1
  Violin.KeySignature(0, 1, true); //from time 0, set to 1 sharp, in major

  //Set up the piano parts.
  Piano1.TrackName("Piano");
  Piano1.ProgramChange(0, 0, 2); //from time 0, set to program 40, on ch. 2
  Piano1.KeySignature(0, 1, true); //from time 0, set to 1 sharp, in major

  Piano2.KeySignature(0, 1, true); //from time 0, set to 1 sharp, in major

  //Write notes to the tracks (deliberately testing adding out of range notes).
  for(count n = 24, t = 0, v = 80; n <= 1000; n++, t++)
  {
    Violin.Note(Ratio(t, 8), Ratio(1, 8), n + 7, v, 1);
    Piano1.Note(Ratio(t, 8), Ratio(1, 8), n + 0, v, 2);
    Piano2.Note(Ratio(t, 8), Ratio(1, 8), n - 7, v, 2);
  }

  /*Sanitizes the MIDI stream (optional -- automatically done before writing).
  Doing this now will just show what the stream will look like when written to
  a file when it is printed out on the next line.*/
  f.OrderEventsCanonically();
  C::Out() >> f;

  //Write out the MIDI file to a byte array and then to a file.
  Array<byte> b;
  f.Write(b);
  File::Write("midi-output.mid", b);

  //Read the same file back in to a different MIDI file object.
  MIDI::File f2;
  Array<byte> b2;
  f2.Read("midi-output.mid");

  //Write the read-in MIDI file to another byte array.
  f2.Write(b2);

  //Compare the string output of the MIDI files and their byte representations.
  C::Out()++;
  C::Out() >> "Stream view is identical: " << (String(f) == String(f2));
  C::Out() >> "Bytes are identical:      " << (b == b2);

  return AutoRelease<Console>();
}
