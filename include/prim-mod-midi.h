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

#ifndef PRIM_INCLUDE_MOD_MIDI_H
#define PRIM_INCLUDE_MOD_MIDI_H
#ifdef PRIM_WITH_MIDI //Large class unnecessary for most applications.

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  /*Throughout the documentation where spec quotes appear, they refer to the
  "Standard MIDI-File Format Spec. 1.1" as distributed by the International MIDI
  Association. For deeper reading into the real-time MIDI standard refer to the
  "MIDI 1.0 Detailed Specification".*/

  /**Standard MIDI File reading and writing. It aims to conform to the <a href=
  "https://www.google.com/search?&q=MIDI+1.0+Detailed+Specification+PDF">
  MIDI 1.0 Detailed Specification</a> (regarding MIDI events) and the <a href=
  "https://www.google.com/search?&q=Standard+MIDI+File+Format+Spec+1.1+PDF">
  Standard MIDI-File Format Spec. 1.1</a> (regarding MIDI file structure).*/
  class MIDI
  {
    //All the magic numbers used for MIDI features handled here are below.

    public:

    ///@name SMPTE Time Formats
    ///@{
    static const int8 SMPTE24          = -24; //24 fps
    static const int8 SMPTE25          = -25; //25 fps
    static const int8 SMPTE29DropFrame = -29; //30000 / 1001 ~ 29.97 fps
    static const int8 SMPTE30          = -30; //30 fps
    ///@}

    private:

    ///@name Non-SMPTE Tempo Mode
    ///@{
    static const int8 QuarterNoteMode  =   0; //quarter note divisions in tempo
    ///@}

    ///@name Chunk Types
    ///@{
    static String HeaderChunk() {return "MThd";}
    static String TrackChunk() {return "MTrk";}
    ///@}

    ///@name Tempo
    ///@{
    static const integer MicrosecondsPerMinute    = 60000000;
    ///@}

    ///@name Variable Length Numbers
    ///@{
    static const byte  VariableLengthHighBit      = 0x80;
    static const count VariableLengthMaxBytes     = 4;
    static const count VariableLengthMax          = 0x0fffffff; // = 268,435,455
    static const count VariableLengthBits         = 7;
    static const count VariableLengthBitMask      = 0x7f;
    ///@}

    ///@name Status Bytes
    ///@{
    static const byte NoteOff              = 0x80; //2-byte value
    static const byte NoteOn               = 0x90; //2-byte value
    static const byte PolyphonicAftertouch = 0xa0; //2-byte value
    static const byte ControlChange        = 0xb0; //2-byte value
    static const byte ProgramChange        = 0xc0; //1-byte value
    static const byte ChannelAftertouch    = 0xd0; //1-byte value
    static const byte PitchWheel           = 0xe0; //2-byte value
    static const byte Meta                 = 0xff; //3+ byte value
    ///@}

    ///@name Status Masks
    ///@{
    static const count ChannelMaximum       = 0x10;
    static const byte  ChannelMask          = 0x0f;
    static const byte  StatusMask           = 0xf0;
    static const byte  StatusByteMask       = 0x80;
    ///@}

    //These need to be handled for decoding, but are silently dropped.

    ///@name Decode-Only Statuses
    ///@{
    static const byte SystemExclusiveBegin = 0xf0;
    static const byte SystemExclusiveEnd   = 0xf7;
    static const byte SongPosition         = 0xf2; //2-byte value
    static const byte SongSelect           = 0xf3; //1-byte value
    ///@}

    ///@name Meta Events
    ///@{

    /*Generally, a meta event is of the form: 0xff type length [...]
    Spec: "Meta-event types 0x01 through 0x0f are reserved for various types of
    text events, each of which meets the specification of text:
    ff 0x len text."*/
    static const byte TextEvent            = 0x01;
    static const byte CopyrightNotice      = 0x02;
    static const byte TrackName            = 0x03;
    static const byte InstrumentName       = 0x04;
    static const byte Lyric                = 0x05;
    static const byte Marker               = 0x06;
    static const byte CuePoint             = 0x07;

    /*Spec: "The MIDI channel (0-15) contained in this event may be used to
    associate a MIDI channel with all events which follow."*/
    static const byte ChannelPrefix        = 0x20;

    //Spec: "tt tt tt Set Tempo (in microseconds per MIDI quarter-note)"
    static const byte SetTempo             = 0x51;

    /*Spec: "nn dd cc bb Time Signature. The time signature is expressed as four
    numbers. nn and dd represent the numerator and denominator of the time
    signature as it would be notated. The denominator is a negative power of
    two: 2 represents a quarter-note, 3 represents an eighth-note, etc. The cc
    parameter expresses the number of MIDI clocks in a metronome click. The bb
    parameter expresses the number of notated 32nd-notes in a MIDI quarter-note
    (24 MIDI clocks). This was added because there are already multiple programs
    which allow a user to specify that what MIDI thinks of as a quarter-note (24
    clocks) is to be notated as, or related to in terms of, something else."*/
    static const byte TimeSignature        = 0x58;

    /*Spec: "sf mi where sf = number of flats (negative) or sharps (positive)
    and mi = 0: major key mi = 1: minor key."*/
    static const byte KeySignature         = 0x59;
    ///@}

    ///@name Other Meta Markers
    ///@{
    /*Set the highest text event type byte that will be accepted by this class.
    The spec allows up to 0x0f; however, 0x07 (Cue Point) was the last one to be
    defined.*/
    static const byte TextEventLast        = CuePoint;

    //These need to be handled for decoding, but are silently dropped.

    /*Spec: "This event is not optional. It is included so that an exact ending
    point may be specified for the track, so that an exact length, which is
    necessary for tracks which are looped or concatenated."*/
    static const byte EndOfTrack           = 0x2f;
    ///@}

    /**Reads a variable length number. If the number was malformed, then -1 is
    returned.*/
    static count ReadVariableLength(const Array<byte>& Data, count& Offset)
    {
      //Error check: overly long length, e.g. 80 00 instead of just 00.
      if(Data[Offset] == VariableLengthHighBit) return -1;

      count Length = 0;
      for(count OutOfBounds = Offset + VariableLengthMaxBytes;
        Offset < OutOfBounds;)
      {
        byte DataByte = Data[Offset++];
        count ValueToAdd = count(DataByte);
        if(DataByte >= VariableLengthHighBit)
          ValueToAdd -= count(VariableLengthHighBit);
        Length = Length * count(VariableLengthHighBit) + ValueToAdd;
        if(DataByte < VariableLengthHighBit)
          return Length;
      }

      //Error: Non-terminating variable length.
      return -1;
    }

    /**Writes a variable-length number to the end of the array. If the number
    was invalid, then false is returned.*/
    static bool WriteVariableLength(Array<byte>& Data, count Length)
    {
      /*Spec: "These numbers are represented 7 bits per byte, most significant
      bits first. All bytes except the last have bit 7 set, and the last byte
      has bit 7 clear. If the number is between 0 and 127, it is thus
      represented exactly as one byte.

      0x00000000 00
      0x00000040 40
      0x0000007f 7f
      0x00000080 81 00
      0x00002000 c0 00
      0x00003fff ff 7f
      0x00004000 81 80 00
      0x00100000 c0 80 00
      0x001fffff ff ff 7f
      0x00200000 81 80 80 00
      0x08000000 c0 80 80 00
      0x0fffffff ff ff ff 7f

      The largest number which is allowed is 0x0fffffff so that the variable-
      length representations must fit in 32 bits in a routine to write variable-
      length numbers."*/

      //Error check: make sure the length is representable.
      if(Length < 0 or Length > VariableLengthMax)
        return false;

      count OriginalLength = Data.n();
      for(count i = VariableLengthMaxBytes - 1; i >= 0; i--)
      {
        count OctetShift = VariableLengthBits * i;
        count OctetMask = VariableLengthBitMask << OctetShift;
        byte Value = byte((Length & OctetMask) >> OctetShift);
        if(not i) //Final value (no high bit)
          Data.Add() = Value;
        else if(Value or Data.n() != OriginalLength) //Non-zero, non-final value
          Data.Add() = Value + VariableLengthHighBit;
      }
      return true;
    }

    ///Internal check of the variable length reader and writer.
    static void TestVariableLength()
    {
      for(count i = 0; i <= VariableLengthMax; i++)
      {
        Array<byte> a;
        WriteVariableLength(a, i);
        count Offset = 0;
        if(ReadVariableLength(a, Offset) != i)
        {
          C::Error() >> i << " was incorrectly encoded or decoded";
          return;
        }
      }
    }

    ///Reads a 32-bit integer for a sequence of bytes.
    static uint32 Read32BitInteger(byte Byte1, byte Byte2, byte Byte3,
      byte Byte4)
    {
      //MSB first
      uint32 x = 0;
      x += uint32(Byte1) << 24;
      x += uint32(Byte2) << 16;
      x += uint32(Byte3) << 8;
      x += uint32(Byte4);
      return x;
    }

    ///Reads a 16-bit integer for a sequence of bytes.
    static uint16 Read16BitInteger(byte Byte1, byte Byte2)
    {
      //MSB first
      uint16 x = 0;
      x += uint16(Byte1) << 8;
      x += uint16(Byte2);
      return x;
    }

    //Forward declaration to handle mutual dependency.
    class Chunk;

    public:

    class File;
    class Synthesizer;
    class Track;

    ///Stores a MIDI message event.
    class Event
    {
      friend class MIDI::File;
      friend class MIDI::Chunk;
      friend class MIDI::Synthesizer;
      friend class MIDI::Track;

      /**The exact time since the last tempo mark. When working in
      seconds this represents a fraction of a second (i.e. tempo = 240 QPM).*/
      Ratio Time;

      ///Status byte identifying the main type of event.
      byte Status; PRIM_PAD(byte)

      ///Channel of the event if applicable. Stored 0-15.
      byte Channel; PRIM_PAD(byte)

      /**Tie-breaking order of the event for sorting. Generally, events should
      be sorted by time and status code. This is especially important for making
      sure that note offs appear before note ons. However, there are some types
      of status events that affect following events, possibly of the same time
      and status (for example a MIDI Channel Prefix meta event followed by a
      Text meta event). The tie-break order is used to save the original order
      of the event stream, so that only high-level time and status sorting takes
      place.*/
      count TieBreakOrder;

      //----------//
      //Data bytes//
      //----------//

      //Wrapped into unions to save space.

      union
      {
        byte Note;
        byte Program;
        byte PitchWheelLSB;
        byte Controller;
        byte MetaType;
        int16 DataByte1_padding;
      };

      union
      {
        byte Velocity;
        byte AftertouchPressure;
        byte PitchWheelMSB;
        byte ControllerValue;
        int16 DataByte2_padding;
      };

      //Meta event type data values (for key signature and time signature)
      byte MetaData[4];

      //Tempo value
      Ratio Tempo;

      ///Meta event value
      String Value;

      private:

      ///Returns a number indicating the order of the event.
      static count CanonicalEventOrder(byte Status, byte Channel, byte Data1,
        byte Data2)
      {
        /*Force ordering to:
          - Meta
          - Program Change        + Channel + Data1
          - Note Off              + Channel + Data1 + Data2
          - Note On               + Channel + Data1 + Data2
          - Polyphonic Aftertouch + Channel + Data1 + Data2
          - Control Change        + Channel + Data1 + Data2
          - Channel Aftertouch    + Channel + Data1
          - Pitch Wheel           + Channel + Data1 + Data2
        */
        count Order = 0;

        /*Spec: "[The copyright notice] event should be the first event in the
        track chunk, at time 0."*/
        if(Status == MIDI::Meta and Data1 == MIDI::CopyrightNotice)
          Order = 0;
        else if(Status == MIDI::Meta)
          Order = 1;
        else if(Status >= MIDI::NoteOff and Status <= MIDI::PitchWheel)
        {
          if(Status == MIDI::ProgramChange)
            Order = count(MIDI::NoteOff - 0x10);
          else
            Order = count(Status);
          Order += count(Channel);
          Order = (Order << 16) + (count(Data1) << 8) + count(Data2);
        }

        return Order;
      }

      ///Wrapper to create regular MIDI events.
      static Event MakeEvent(Ratio Time, byte Status, count Channel,
        count Data1, count Data2 = 0)
      {
        Event e;
        if(Time.IsEmpty() or Time < 0 or Channel < 1 or Channel > 0x10 or
          Data1 < 0 or Data1 > 0x7f or Data2 < 0 or Data2 > 0x7f) return e;
        e.Time = Time;
        e.Channel = byte(Channel - 1);
        e.Status = Status;
        e.Note = byte(Data1);
        e.Velocity = byte(Data2);
        return e;
      }

      ///Wrapper to create meta-text events.
      static Event MakeTextEvent(Ratio Time, byte MetaType, String Text)
      {
        Event e;
        if(Time.IsEmpty() or Time < 0) return e;
        e.Time = Time;
        e.Status = MIDI::Meta;
        e.MetaType = MetaType;
        e.Value = Text;
        return e;
      }

      public:

      /**Creates a note-on event. NoteNumber and NoteVelocity must be [0, 127]
      inclusive and Channel must be [1, 16] inclusive. Note that a note-on event
      with velocity at zero is converted automatically into an explicit note-off
      event, per the standard. Time is in exact fractions of a whole note (or a
      second, if in SMPTE mode).*/
      static Event NoteOn(Ratio Time, count NoteNumber, count NoteVelocity,
        count Channel)
      {
        //Note-on with velocity zero is equivalent to note-off.
        if(NoteVelocity == 0)
          return MakeEvent(Time, MIDI::NoteOff, Channel, NoteNumber, 0);
        return MakeEvent(Time, MIDI::NoteOn, Channel, NoteNumber, NoteVelocity);
      }

      /**Creates a note-off event. NoteNumber and NoteVelocity must be [0, 127]
      inclusive and Channel must be [1, 16] inclusive. Time is in exact
      fractions of a whole note (or a second, if in SMPTE mode).*/
      static Event NoteOff(Ratio Time, count NoteNumber, count NoteVelocity,
        count Channel)
      {
        return MakeEvent(Time, MIDI::NoteOff, Channel, NoteNumber,
          NoteVelocity);
      }

      /**Creates a polyphonic aftertouch event. NoteNumber and Pressure must be
      [0, 127] inclusive and Channel must be [1, 16] inclusive. Time is in exact
      fractions of a whole note (or a second, if in SMPTE mode).*/
      static Event PolyphonicAftertouch(Ratio Time, count NoteNumber,
        count Pressure, count Channel)
      {
        return MakeEvent(Time, MIDI::PolyphonicAftertouch, Channel, NoteNumber,
          Pressure);
      }

      /**Creates a control change event. Control and Value must be [0, 127]
      inclusive and Channel must be [1, 16] inclusive. Time is in exact
      fractions of a whole note (or a second, if in SMPTE mode).*/
      static Event ControlChange(Ratio Time, count Control, count Value,
        count Channel)
      {
        return MakeEvent(Time, MIDI::ControlChange, Channel, Control,
          Value);
      }

      /**Creates a program change event. Program must be [0, 127] inclusive and
      Channel must be [1, 16] inclusive. Time is in exact fractions of a whole
      note (or a second, if in SMPTE mode). Program changes are automatically
      sorted by the File writer to occur before note events of the same time.*/
      static Event ProgramChange(Ratio Time, count Program, count Channel)
      {
        return MakeEvent(Time, MIDI::ProgramChange, Channel, Program);
      }

      /**Creates a channel aftertouch event. Program must be [0, 127] inclusive
      and Channel must be [1, 16] inclusive. Time is in exact fractions of a
      whole note (or a second, if in SMPTE mode). Program changes are
      automatically sorted by the File writer to occur before note events of the
      same time.*/
      static Event ChannelAftertouch(Ratio Time, count Pressure, count Channel)
      {
        return MakeEvent(Time, MIDI::ChannelAftertouch, Channel, Pressure);
      }

      /**Creates a pitch wheel event. MSB and LSB must be [0, 127] inclusive and
      Channel must be [1, 16] inclusive. Time is in exact fractions of a whole
      note (or a second, if in SMPTE mode).*/
      static Event PitchWheel(Ratio Time, count MSB, count LSB, count Channel)
      {
        return MakeEvent(Time, MIDI::ControlChange, Channel, MSB, LSB);
      }

      /**Creates a meta channel prefix event. This should be used before the
      InstrumentName() event, for example, to indicate the channel for the
      text.*/
      static Event ChannelPrefix(Ratio Time, count Channel)
      {
        Event e;
        if(Time.IsEmpty() or Time < 0 or Channel < 1 or Channel > 0x10)
          return e;

        e.Time = Time;
        e.Status = MIDI::Meta;
        e.MetaType = MIDI::ChannelPrefix;
        e.Channel = byte(Channel - 1);

        return e;
      }

      ///Creates a generic text event.
      static Event TextEvent(Ratio Time, String Text)
      {
        return MakeTextEvent(Time, MIDI::TextEvent, Text);
      }

      ///Creates a copyright notice at the beginning of the file.
      static Event CopyrightNotice(String Text)
      {
        return MakeTextEvent(0, MIDI::CopyrightNotice, Text);
      }

      /**Creates a track name text event. Add a ChannelPrefix() event before
      this event to assign the track name to the channel correctly.*/
      static Event TrackName(String Text)
      {
        return MakeTextEvent(0, MIDI::TrackName, Text);
      }

      ///Creates an instrument name text event.
      static Event InstrumentName(String Text)
      {
        return MakeTextEvent(0, MIDI::InstrumentName, Text);
      }

      ///Creates a lyric text event.
      static Event Lyric(Ratio Time, String Text)
      {
        return MakeTextEvent(Time, MIDI::Lyric, Text);
      }

      ///Creates a marker text event.
      static Event Marker(Ratio Time, String Text)
      {
        return MakeTextEvent(Time, MIDI::Marker, Text);
      }

      ///Creates a cue point text event.
      static Event CuePoint(Ratio Time, String Text)
      {
        return MakeTextEvent(Time, MIDI::CuePoint, Text);
      }

      /**Creates a set tempo meta event. The tempo is expressed in quarter notes
      per minute. This tempo value must be between [1, 1000] inclusive.*/
      static Event SetTempo(Ratio Time, Ratio QuarterNotesPerMinute)
      {
        Event e;
        if(Time.IsEmpty() or Time < 0 or QuarterNotesPerMinute.IsEmpty() or
          QuarterNotesPerMinute < 1 or QuarterNotesPerMinute > 1000)
            return e;
        e.Time = Time;
        e.Status = MIDI::Meta;
        e.MetaType = MIDI::SetTempo;
        e.Tempo = Ratio(MicrosecondsPerMinute) / QuarterNotesPerMinute;
        return e;
      }

      /**Creates a key signature event. Accidentals is between [-7, 7] where
      negative numbers represent the negated number of flats, 0 represents no
      accidentals, and positive numbers represent the number of sharps.*/
      static Event KeySignature(Ratio Time, count Accidentals, bool Major)
      {
        Event e;
        if(Time.IsEmpty() or Time < 0 or Accidentals < -7 or Accidentals > 7)
          return e;

        e.Time = Time;
        e.Status = MIDI::Meta;
        e.MetaType = MIDI::KeySignature;
        e.MetaData[0] = byte(int8(Accidentals));
        e.MetaData[1] = (Major ? 0 : 1);

        return e;
      }

      /**Creates a time signature event. The numerator must be between [1, 64]
      inclusive and the denominator must be a power of two [1, 64] inclusive.
      The rhythmic value to be used as the metronomic beat must be one of: 1/16,
      1/8, 1/4, 3/8, 1/2, or 1 and defaults to a quarter note (1/4).*/
      static Event TimeSignature(Ratio Time, count Numerator, count Denominator,
        Ratio Beat = Ratio(1, 4))
      {
        Event e;
        if(Time.IsEmpty() or Time < 0 or Numerator < 1 or Numerator > 64 or
          Denominator < 1 or Denominator > 64 or
          Denominator != PowerOfTwoCeiling(Denominator) or
          (Beat != Ratio(1, 16) and Beat != Ratio(1, 8) and Beat != Ratio(1, 4)
          and Beat != Ratio(3, 8) and Beat != Ratio(1, 2) and
          Beat != Ratio(1, 1)))
            return e;
        e.Time = Time;
        e.Status = MIDI::Meta;
        e.MetaType = MIDI::TimeSignature;
        e.MetaData[0] = byte(Numerator);
        if(Denominator == 1)       e.MetaData[1] = 0;
        else if(Denominator == 2)  e.MetaData[1] = 1;
        else if(Denominator == 4)  e.MetaData[1] = 2;
        else if(Denominator == 8)  e.MetaData[1] = 3;
        else if(Denominator == 16) e.MetaData[1] = 4;
        else if(Denominator == 32) e.MetaData[1] = 5;
        else if(Denominator == 64) e.MetaData[1] = 6;
        else e.MetaData[1] = 2;
        e.MetaData[2] = (Beat * 24 * 4).To<byte>();
        e.MetaData[3] = 8;
        return e;
      }

      ///Creates an empty event.
      Event() : Time(0), Status(0), Channel(0), TieBreakOrder(0), Note(0),
        Velocity(0)
      {
        MetaData[0] = MetaData[1] = MetaData[2] = MetaData[3] = 0;
      }

      ///Returns whether this event occurs before another event.
      bool operator < (const Event& Other) const
      {
        count CanonicalStatus1 = CanonicalEventOrder(
          Status, Channel, Note, Velocity);
        count CanonicalStatus2 = CanonicalEventOrder(
          Other.Status, Other.Channel, Other.Note, Other.Velocity);

        if(Time < Other.Time)
          return true;
        else if(Time == Other.Time)
        {
          if(CanonicalStatus1 < CanonicalStatus2)
            return true;
          else if(CanonicalStatus1 == CanonicalStatus2 and
            TieBreakOrder < Other.TieBreakOrder)
            return true;
        }
        return false;
      }

      ///Returns whether this event occurs after another event.
      bool operator > (const Event& Other) const
      {
        count CanonicalStatus1 = CanonicalEventOrder(
          Status, Channel, Note, Velocity);
        count CanonicalStatus2 = CanonicalEventOrder(
          Other.Status, Other.Channel, Other.Note, Other.Velocity);

        if(Time > Other.Time)
          return true;
        else if(Time == Other.Time)
        {
          if(CanonicalStatus1 > CanonicalStatus2)
            return true;
          else if(CanonicalStatus1 == CanonicalStatus2 and
            TieBreakOrder > Other.TieBreakOrder)
            return true;
        }
        return false;
      }

      ///Prints out a string representation of this event.
      operator String() const
      {
        String s;
        s << Time << " ";
        if(Status == MIDI::Meta and MetaType >= MIDI::TextEvent and
          MetaType <= TextEventLast)
        {
          if(MetaType == MIDI::CopyrightNotice) s << "Copyright Notice";
          else if(MetaType == MIDI::TrackName) s << "Track Name";
          else if(MetaType == MIDI::InstrumentName) s << "Instrument Name";
          else if(MetaType == MIDI::Lyric) s << "Lyric";
          else if(MetaType == MIDI::Marker) s << "Marker";
          else if(MetaType == MIDI::CuePoint) s << "Cue Point";
          else s << "Text";
          s << ": " << Value;
        }
        else if(Status == Meta and MetaType == MIDI::SetTempo)
          s << "Set Tempo: " << Tempo << " us/qn";
        else if(Status == Meta and MetaType == MIDI::TimeSignature)
        {
          s << "Time Signature: ";
          s << count(MetaData[0]) << "/";
          s << count(Power(2.0, float64(int8(MetaData[1]))));
          s << " (";
          s << count(MetaData[2]) << " clocks per metronome beat, ";
          s << count(MetaData[3]) << " 32nds per 24 MIDI clocks/quarter note)";
        }
        else if(Status == MIDI::Meta and MetaType == MIDI::KeySignature)
        {
          s << "Key Signature: ";
          count Accidentals = count(int8(MetaData[0]));
          if(Accidentals < 0)
            s << -Accidentals << " flats ";
          else if(Accidentals == 0)
            s << "no accidentals ";
          else
            s << Accidentals << " sharps ";
          s << (MetaData[1] == 0 ? "major" : "minor");
        }
        else if(Status == MIDI::Meta and MetaType == MIDI::ChannelPrefix)
          s << "Channel Prefix: " << (Channel + 1);
        else if(Status == MIDI::NoteOff)
          s << "Note Off: " << Note << " @ " << Velocity << " on ch. " <<
            (Channel + 1);
        else if(Status == MIDI::NoteOn)
          s << "Note On:  " << Note << " @ " << Velocity << " on ch. " <<
            (Channel + 1);
        else if(Status == MIDI::PolyphonicAftertouch)
          s << "Polyphonic Aftertouch: " << Note << " @ " << AftertouchPressure
            << " on ch. " << (Channel + 1);
        else if(Status == MIDI::ControlChange)
          s << "Control Change: " << Controller << " @ " << ControllerValue <<
            " on ch. " << (Channel + 1);
        else if(Status == MIDI::PitchWheel)
          s << "Pitch Wheel: " << PitchWheelMSB << " " << PitchWheelLSB  <<
            " on ch. " << (Channel + 1);
        else if(Status == MIDI::ProgramChange)
          s << "Program Change: " << Program << " on ch. " << (Channel + 1);
        else if(Status == MIDI::ChannelAftertouch)
          s << "Channel Aftertouch: " << AftertouchPressure  << " on ch. " <<
            (Channel + 1);
        if(not Status)
          s << "Empty event";
        return s;
      }
    };

    public:

    ///Represents a MIDI track consisting of a stream of MIDI events.
    class Track
    {
      public:

      ///List of MIDI events for this track.
      Sortable::List<Event> Events;

      ///Returns a string representation of the track.
      operator String() const
      {
        String s;
        for(count i = 0; i < Events.n(); i++)
          s >> Events[i];
        return s;
      }

      /**Creates a note-on note-off pair. NoteNumber and NoteVelocity must be
      [0, 127] inclusive and Channel must be [1, 16] inclusive. Time and
      Duration are in exact fractions of a whole note (or a
      second, if in SMPTE mode).*/
      void Note(Ratio Time, Ratio Duration, count NoteNumber,
        count NoteVelocity, count Channel)
      {
        Events.Add() = Event::NoteOn(Time, NoteNumber, NoteVelocity, Channel);
        Events.Add() = Event::NoteOff(Time + Duration, NoteNumber, 0, Channel);
      }

      /**Creates a polyphonic aftertouch event. NoteNumber and Pressure must be
      [0, 127] inclusive and Channel must be [1, 16] inclusive. Time is in exact
      fractions of a whole note (or a second, if in SMPTE mode).*/
      void PolyphonicAftertouch(Ratio Time, count NoteNumber, count Pressure,
        count Channel)
      {
        Events.Add() = Event::PolyphonicAftertouch(Time, NoteNumber, Pressure,
          Channel);
      }

      /**Creates a control change event. Control and Value must be [0, 127]
      inclusive and Channel must be [1, 16] inclusive. Time is in exact
      fractions of a whole note (or a second, if in SMPTE mode).*/
      void ControlChange(Ratio Time, count Control, count Value, count Channel)
      {
        Events.Add() = Event::ControlChange(Time, Control, Value, Channel);
      }

      /**Creates a program change event. Program must be [0, 127] inclusive and
      Channel must be [1, 16] inclusive. Time is in exact fractions of a whole
      note (or a second, if in SMPTE mode). Program changes are automatically
      sorted by the File writer to occur before note events of the same time.*/
      void ProgramChange(Ratio Time, count Program, count Channel)
      {
        Events.Add() = Event::ProgramChange(Time, Program, Channel);
      }

      /**Creates a channel aftertouch event. Program must be [0, 127] inclusive
      and Channel must be [1, 16] inclusive. Time is in exact fractions of a
      whole note (or a second, if in SMPTE mode). Program changes are
      automatically sorted by the File writer to occur before note events of the
      same time.*/
      void ChannelAftertouch(Ratio Time, count Pressure, count Channel)
      {
        Events.Add() = Event::ChannelAftertouch(Time, Pressure, Channel);
      }

      /**Creates a pitch wheel event. MSB and LSB must be [0, 127] inclusive and
      Channel must be [1, 16] inclusive. Time is in exact fractions of a whole
      note (or a second, if in SMPTE mode).*/
      void PitchWheel(Ratio Time, count MSB, count LSB, count Channel)
      {
        Events.Add() = Event::PitchWheel(Time, MSB, LSB, Channel);
      }

      ///Creates a track name text event.
      void TrackName(String Text)
      {
        Events.Add() = Event::TrackName(Text);
      }

      /**Creates a key signature event. Accidentals is between [-7, 7] where
      negative numbers represent the negated number of flats, 0 represents no
      accidentals, and positive numbers represent the number of sharps.*/
      void KeySignature(Ratio Time, count Accidentals, bool Major)
      {
        Events.Add() = Event::KeySignature(Time, Accidentals, Major);
      }
    };

    private:

    /**Represents an SMF chunk, which is the highest level of organization.
    Spec: "Each chunk has a 4-character type and a 32-bit length... This length
    refers to the number of bytes of data which follow."*/
    class Chunk
    {
      friend class MIDI::File;

      /**Spec: "Each chunk begins with a 4-character ASCII type."*/
      String Type;

      ///Stores the chunk data. The chunk length is implied by the Array length.
      Array<byte> Data;

      ///Clears the type.
      void ClearType() {Type = "    ";}

      public:

      ///Returns a reference to the chunk data.
      const Array<byte>& GetData() const {return Data;}

      ///Creates an empty chunk.
      Chunk() {ClearType();}

      ///Gets the 4-letter type of the chunk.
      String GetType() const
      {
        return Type;
      }

      ///Sets the 4-letter type of the chunk.
      void SetType(String NewType)
      {
        ClearType();
        if(NewType.n() != 4)
          return;
        Type = NewType;
      }

      /**Returns the total size of the chunk including the header.

      Spec: "This length refers to the number of bytes of data which follow: the
      eight bytes of type and length are not included. Therefore, a chunk with a
      length of 6 would actually occupy 14 bytes in the disk file."*/
      count TotalSize() const {return Length() + 8;}

      /**Returns the length of the chunk data.

      Spec: "This length refers to the number of bytes of data which follow: the
      eight bytes of type and length are not included. Therefore, a chunk with a
      length of 6 would actually occupy 14 bytes in the disk file."*/
      count Length() const {return Data.n();}

      ///Reads in the chunk data from a byte array.
      void ImportChunkData(const byte* ByteData, count Length)
      {
        Data.CopyMemoryFrom(ByteData, Length);
      }

      /**Reads the next event in the track. This is main decoder for the MIDI
      event stream.*/
      bool ReadNextEvent(Event& e, count& i, byte& RunningStatus, Ratio& Time,
        bool& CorruptionDetected) const
      {
        //Read the delta time.
        count Delta = ReadVariableLength(Data, i);
        if(Delta < 0)
        {
          CorruptionDetected = true;
          return false;
        }
        Time += Ratio(Delta);
        e.Time = Time;

        /*Spec: "Status bytes of MIDI channel messages may be omitted if the
        preceding event is a MIDI channel message with the same status."*/
        byte Status = RunningStatus;
        if(Data[i] & StatusByteMask)
          RunningStatus = Status = Data[i++];

        byte StatusMasked = Status & StatusMask;
        byte Channel = Status & ChannelMask;

        /*Skip through system exclusive messages. The spec is fairly vague about
        how to rigorously differentiate between the structure of single versus
        multi-pack system exclusive messages. The logic here, for example, does
        not test for the edge case of a multi-packet with an interior delta time
        between 0xf700 and 0xf77f, which would be ambiguous with the 0xf7 end
        message signal. Go figure.*/
        if(Status == SystemExclusiveBegin or Status == SystemExclusiveEnd)
        {
          count Length = ReadVariableLength(Data, i);
          if(Length < 0)
          {
            CorruptionDetected = true;
            return false;
          }
          i += Length;
          if(Data[i] == SystemExclusiveEnd)
            i++;
          return false;
        }
        else if(Status == SongPosition)
        {
          i += 2; //Skip over this.
          return false;
        }
        else if(Status == SongSelect)
        {
          i += 1; //Skip over this.
          return false;
        }
        else if(Status == Meta)
        {
          byte MetaStatusType = Data[i++];
          count Length = ReadVariableLength(Data, i);
          if(Length < 0)
          {
            CorruptionDetected = true;
            return false;
          }
          e.Status = Status;
          e.MetaType = MetaStatusType;
          if(MetaStatusType == EndOfTrack)
          {
            /*There is no need to explicitly save the end of track marker here
            since it is a mandantory event that is part of the chunk structure.
            The length is zero, so there is no data to skip over.*/
            return false;
          }
          else if(MetaStatusType == SetTempo)
          {
            count Sum = count(Data[i]) << 16;
            Sum += count(Data[i + 1]) << 8;
            Sum += count(Data[i + 2]);
            e.Tempo = Ratio(Sum);
          }
          else if(MetaStatusType >= TextEvent and
            MetaStatusType <= TextEventLast)
          {
            e.Value = String(&Data[i], Length);

            //Remove any characters outside of the ASCII encoding.
            e.Value.ForceToASCII();
          }
          else if(MetaStatusType == ChannelPrefix)
            e.Channel = Data[i];
          else if(MetaStatusType == KeySignature)
          {
            e.MetaData[0] = Data[i];
            e.MetaData[1] = Data[i + 1];
          }
          else if(MetaStatusType == TimeSignature)
          {
            e.MetaData[0] = Data[i];
            e.MetaData[1] = Data[i + 1];
            e.MetaData[2] = Data[i + 2];
            e.MetaData[3] = Data[i + 3];
          }
          else
          {
            //Unsupported meta event. Skip.
            i += Length;
            return false;
          }
          i += Length;
        }
        else if(StatusMasked == NoteOn or StatusMasked == NoteOff)
        {
          e.Status   = StatusMasked;
          e.Channel  = Channel;
          e.Note     = Data[i++];
          e.Velocity = Data[i++];

          /*Note-on with a zero velocity is equivalent to a note-off. Rewrite as
          note-off.*/
          if(e.Status == NoteOn and e.Velocity == 0)
            e.Status = NoteOff;
        }
        else if(StatusMasked == PolyphonicAftertouch)
        {
          e.Status             = StatusMasked;
          e.Channel            = Channel;
          e.Note               = Data[i++];
          e.AftertouchPressure = Data[i++];
        }
        else if(StatusMasked == ControlChange)
        {
          e.Status          = StatusMasked;
          e.Channel         = Channel;
          e.Controller      = Data[i++];
          e.ControllerValue = Data[i++];
        }
        else if(StatusMasked == PitchWheel)
        {
          e.Status        = StatusMasked;
          e.Channel       = Channel;
          e.PitchWheelLSB = Data[i++];
          e.PitchWheelMSB = Data[i++];
        }
        else if(StatusMasked == ProgramChange)
        {
          e.Status  = StatusMasked;
          e.Channel = Channel;
          e.Program = Data[i++];
        }
        else if(StatusMasked == ChannelAftertouch)
        {
          e.Status             = StatusMasked;
          e.Channel            = Channel;
          e.AftertouchPressure = Data[i++];
        }
        else
        {
          //Unsupported MIDI event. Skip.
          return false;
        }
        return true;
      }

      ///Writes an event to the chunk.
      void WriteEvent(const Event& e, Ratio Time)
      {
        /*For channel-based status, write out the status and the data bytes. For
        meta status, write out the status, event type, length, and data.*/
        if(e.Status >= NoteOff and e.Status <= PitchWheel)
        {
          //Write the delta time for the event.
          WriteVariableLength(Data, Time.To<count>());

          //Write the status byte with channel information.
          Data.Add() = (e.Status & StatusMask) + (e.Channel & ChannelMask);

          //Write out first data byte.
          Data.Add() = e.Note;

          //Write out second data byte if necessary.
          if(e.Status != ProgramChange and e.Status != ChannelAftertouch)
            Data.Add() = e.Velocity;
        }
        else if(e.Status == Meta)
        {
          //Write the delta time for the event.
          WriteVariableLength(Data, Time.To<count>());

          //Write the status byte.
          Data.Add() = e.Status;

          //Write the meta event type.
          Data.Add() = e.MetaType;

          //Write the data out specific to each meta event type.
          if(e.MetaType >= TextEvent and e.MetaType <= TextEventLast)
          {
            String v = e.Value;
            v.ForceToASCII();
            WriteVariableLength(Data, v.n());
            for(count i = 0; i < v.n(); i++)
              Data.Add() = byte(v[i]);
          }
          else if(e.MetaType == SetTempo)
          {
            int32 x = int32(e.Tempo.To<int64>());
            Data.Add() = 3;
            Data.Add() = byte((x >> 16) & 0xff);
            Data.Add() = byte((x >> 8) & 0xff);
            Data.Add() = byte(x & 0xff);
          }
          else if(e.MetaType == ChannelPrefix)
          {
            Data.Add() = 1;
            Data.Add() = e.Channel;
          }
          else if(e.MetaType == KeySignature)
          {
            Data.Add() = 2;
            Data.Add() = e.MetaData[0];
            Data.Add() = e.MetaData[1];
          }
          else if(e.MetaType == TimeSignature)
          {
            Data.Add() = 4;
            Data.Add() = e.MetaData[0];
            Data.Add() = e.MetaData[1];
            Data.Add() = e.MetaData[2];
            Data.Add() = e.MetaData[3];
          }
        }
      }

      ///Reads through the chunk MIDI event stream and generates a track.
      void ReadIntoTrack(Track& t, Ratio DivisionsPerQuarterNote) const
      {
        //Initialize the track.
        t.Events.RemoveAll();
        if(GetType() != TrackChunk()) return;

        //Keep track of state.
        byte RunningStatus = 0;
        Ratio Time = 0;
        bool CorruptionDetected = false;

        for(count i = 0; i < Data.n();)
        {
          //Process the next event.
          Event e;
          if(not ReadNextEvent(e, i, RunningStatus, Time, CorruptionDetected))
          {
            if(CorruptionDetected)
            {
              C::Error() >> "MIDI track appears to be corrupt. Aborting.";
              return;
            }
            continue;
          }

          //Scale the time in the event to be relative to whole notes.
          e.Time /= (DivisionsPerQuarterNote * 4);

          t.Events.Add() = e;
        }
      }

      ///Returns a string representation of this chunk for debugging.
      operator String () const
      {
        String s;
        s >> TotalSize() << " bytes: " << GetType() << " " << Length() << " <";
        s << Length() << " bytes of data...>";
        return s;
      }
    };

    public:

    /**Represents a Standard MIDI File. The SMF has an extension of .mid and
    acts as a container for a real-time MIDI event performance. Use the Read()
    methods to read in existing MIDI files. To create MIDI files from scratch,
    add a Track to Tracks, and add Event objects to Tracks::Events. To write the
    MIDI files back out use the Write() method.

    You can also convert between Type 0 (single interleaved track) and Type 1
    (multi-track) formats using the provided methods.*/
    class File
    {
      /*Since the file is not in real-time, it has a few structural differences:
      - The file is composed of chunks, in the simplest case, a header chunk
        followed by one or more track chunks.
      - The file may be in Type 0 (one track containing merged channel data) or
        Type 1 (multi-track). In the case of Type 1, there are separate track
        chunks for each channel.
      - Events are preceded by a delta time, which indicates in the current
        tempo how long to wait after the preceding event, before performing the
        event.
      - The System Reset 0xff status is replaced by the Meta event status, which
        allows certain information to be encoded into the stream, such as text,
        tempo changes, end of track, and other annotative information.
      - SysEx packets are structured slightly differently.*/

      ///Stores the SMPTE mode type.
      int8 SMPTEType; PRIM_PAD(int8)

      ///Stores the divisions per quarter (or second in the case of SMPTE).
      Ratio DivisionsPerQuarter;

      public:

      ///Stores each track of MIDI events.
      List<Track> Tracks;

      public:

      ///Creates an empty standard MIDI file.
      File() {Clear();}

      ///Clears chunks and always returns false to aid error checking.
      bool Clear()
      {
        Tracks.RemoveAll();
        SMPTEType = QuarterNoteMode;
        DivisionsPerQuarter = 840;
        return false;
      }

      ///Returns whether file is type 0 (one track with merged channel data).
      bool IsType0() const {return Tracks.n() == 1;}

      ///Returns whether file is type 1 (one track per channel).
      bool IsType1() const {return Tracks.n() > 1;}

      ///Returns whether file uses divisions of the quarter note for clocking.
      bool HasQuarterNoteDivisions() const
      {
        return SMPTEType == QuarterNoteMode;
      }

      ///Returns whether the file uses SMPTE time divisions for clocking.
      bool HasSMPTEDivisions() const {return SMPTEType != QuarterNoteMode;}

      /**Sets the number of divisions per quarter note. The number must be in
      the range [1, 32767] inclusive. It is usually best to set this to a
      highly compound number such as 840, so that it can divide evenly with
      common note lengths and tuples.*/
      bool SetDivisionsPerQuarterNote(count Divisions)
      {
        //Must be a positive 15-bit integer.
        if(Divisions > 0 and Divisions < 0x8000)
        {
          SMPTEType = QuarterNoteMode;
          DivisionsPerQuarter = Ratio(Divisions);
          return true;
        }
        return false;
      }

      /**Sets the divisions mode to one of the SMPTE formats. This expresses
      absolute time in terms of frames per second and subdivisions per frame.
      The SMPTE mode may be SMPTE24, SMPTE25, SMPTE29DropFrame (29.97), or
      SMPTE30. The subdivisions may be in the range [1, 255] inclusive. Returns
      whether the new SMPTE mode was set.*/
      bool SetSMPTEMode(int8 NewSMPTEType, count SubdivisionsPerFrame)
      {
        //Make sure the incoming type and subdivisions are valid.
        if(NewSMPTEType != SMPTE24 and NewSMPTEType != SMPTE25 and NewSMPTEType
          != SMPTE29DropFrame and NewSMPTEType != SMPTE30)
            return false;

        //Subdivisions per frame must be a positive 8-bit integer.
        if(SubdivisionsPerFrame < 1 or SubdivisionsPerFrame > 255)
          return false;

        //Set the new type.
        SMPTEType = NewSMPTEType;

        //Set the divisions per quarter.
        if(SMPTEType == SMPTE24)
          DivisionsPerQuarter = 24;
        else if(SMPTEType == SMPTE25)
          DivisionsPerQuarter = 25;
        else if(SMPTEType == SMPTE29DropFrame)
          DivisionsPerQuarter = Ratio(30000, 1001);
        else if(SMPTEType == SMPTE30)
          DivisionsPerQuarter = 30;
        DivisionsPerQuarter *= Ratio(SubdivisionsPerFrame, 4);

        return true;
      }

      /**Returns the divisions per quarter note. In the case of SMPTE division
      mode, the quarter note is assumed to be at a rate of 240 BPM, so that a
      quarter note equals a quarter of a second.*/
      Ratio GetDivisionsPerQuarterNote() const {return DivisionsPerQuarter;}

      private:

      /**Returns whether this meta event is global. This indicates whether an
      event is part of the meta-event map and should therefore only be encoded
      on the first track. The meta-event map is a generalization (specific to
      this class) of the tempo map described in the spec.*/
      static bool IsGlobalMetaEvent(const Event& e)
      {
        return e.Status == Meta and e.MetaType != TrackName and
          e.MetaType != KeySignature;
      }

      public:

      /**Returns a list of all the meta events. This would contain for example,
      tempo and time signature, changes as well as some types of text. Note that
      the meta-event map is not stored in a separate structure; it just refers
      to all of the global meta events.*/
      List<Event> GetMetaEventMap() const
      {
        List<Event> e;
        for(count i = 0; i < Tracks.n(); i++)
          for(count j = 0; j < Tracks[i].Events.n(); j++)
            if(IsGlobalMetaEvent(Tracks[i].Events[j]))
                e.Add() = Tracks[i].Events[j];
        return e;
      }

      /**Replaces the meta event map with a new list of events. Note that the
      meta-event map is not stored in a separate structure; it just refers to
      all of the global meta events. This will cause track events to be sorted
      as well.*/
      void ReplaceMetaEventMap(const List<Event>& e)
      {
        //Make sure there is at least one track.
        if(not Tracks.n())
          Tracks.Add();

        //Remove all meta events.
        for(count i = 0; i < Tracks.n(); i++)
          for(count j = 0; j < Tracks[i].Events.n(); j++)
            if(IsGlobalMetaEvent(Tracks[i].Events[j]))
                Tracks[i].Events.Remove(j--);

        //Add the incoming meta event list.
        for(count i = 0; i < e.n(); i++)
          if(IsGlobalMetaEvent(e[i]))
            Tracks.a().Events.Add() = e[i];

        //Sort as appropriate.
        SortTrackEvents();
      }

      /**Sorts and sanitizes the event streams in each track. This places all
      meta events on the first track and sorts the events in each track by time,
      status, channel, and data.*/
      void OrderEventsCanonically()
      {
        //Restore the meta-event map (this also sorts the track events).
        ReplaceMetaEventMap(GetMetaEventMap());

        //Remove any events and tracks that are empty.
        for(count i = 0; i < Tracks.n(); i++)
        {
          for(count j = 0; j < Tracks[i].Events.n(); j++)
            if(not Tracks[i].Events[j].Status)
              Tracks[i].Events.Remove(j--);
          if(not Tracks[i].Events.n())
            Tracks.Remove(i--);
        }
      }

      ///Creates a generic text event.
      void TextEvent(Ratio Time, String Text)
      {
        if(not Tracks.n()) Tracks.Add();
        Tracks.a().Events.Add() = Event::TextEvent(Time, Text);
      }

      ///Creates a copyright notice at the beginning of the file.
      void CopyrightNotice(String Text)
      {
        if(not Tracks.n()) Tracks.Add();
        Tracks.a().Events.Add() = Event::CopyrightNotice(Text);
      }

      ///Creates an instrument name text event for a given channel [1-16].
      void InstrumentName(String Text, count Channel)
      {
        if(not Tracks.n()) Tracks.Add();
        Tracks.a().Events.Add() = Event::ChannelPrefix(0, Channel);
        Tracks.a().Events.Add() = Event::InstrumentName(Text);
      }

      ///Creates a lyric text event.
      void Lyric(Ratio Time, String Text)
      {
        if(not Tracks.n()) Tracks.Add();
        Tracks.a().Events.Add() = Event::Lyric(Time, Text);
      }

      ///Creates a marker text event.
      void Marker(Ratio Time, String Text)
      {
        if(not Tracks.n()) Tracks.Add();
        Tracks.a().Events.Add() = Event::Marker(Time, Text);
      }

      ///Creates a cue point text event.
      void CuePoint(Ratio Time, String Text)
      {
        if(not Tracks.n()) Tracks.Add();
        Tracks.a().Events.Add() = Event::CuePoint(Time, Text);
      }

      /**Creates a set tempo meta event. The tempo is expressed in quarter notes
      per minute. This tempo value must be between [1, 1000] inclusive.*/
      void SetTempo(Ratio Time, Ratio QuarterNotesPerMinute)
      {
        if(not Tracks.n()) Tracks.Add();
        Tracks.a().Events.Add() = Event::SetTempo(Time, QuarterNotesPerMinute);
      }

      /**Creates a time signature event. The numerator must be between [1, 64]
      inclusive and the denominator must be a power of two [1, 64] inclusive.
      The rhythmic value to be used as the metronomic beat must be one of: 1/16,
      1/8, 1/4, 3/8, 1/2, or 1 and defaults to a quarter note (1/4).*/
      void TimeSignature(Ratio Time, count Numerator, count Denominator,
        Ratio Beat = Ratio(1, 4))
      {
        if(not Tracks.n()) Tracks.Add();
        Tracks.a().Events.Add() = Event::TimeSignature(Time, Numerator,
          Denominator, Beat);
      }

      ///Reads in a MIDI file give a pointer to a byte array.
      bool Read(const byte* MIDIFileData, count MIDIFileLength)
      {
        ///Stores the chunks in the MIDI file.
        Array<Chunk> Chunks;

        Clear();
        for(count i = 0; i < MIDIFileLength; i += Chunks.z().TotalSize())
        {
          //First check for a corrupt header size.
          if(i + 8 > MIDIFileLength)
            return Clear();

          //Get a pointer to the start of the current chunk.
          const byte* ChunkStart = &MIDIFileData[i];

          //Read the chunk type.
          String ChunkType;
          ChunkType << ascii(ChunkStart[0]) << ascii(ChunkStart[1]) <<
            ascii(ChunkStart[2]) << ascii(ChunkStart[3]);

          //Read the chunk length.
          count ChunkLength = count(Read32BitInteger(ChunkStart[4],
            ChunkStart[5], ChunkStart[6], ChunkStart[7]));

          //Now check to see if the chunk is complete.
          if(i + 8 + ChunkLength > MIDIFileLength)
            return Clear();

          //Add a chunk and import the data into the chunk.
          Chunks.Add().ImportChunkData(&ChunkStart[8], ChunkLength);
          Chunks.z().SetType(ChunkType);
        }

        /*Do some sanity checks before returning in case of empty or really
        malformed files.*/
        {
          count TrackCount = 0;
          if(Chunks.n() < 2) return Clear();
          if(Chunks.a().GetType() != HeaderChunk()) return Clear();
          if(Chunks.a().Data.n() != 6) return Clear();
          for(count i = 0; i < Chunks.n(); i++)
            if(Chunks[i].GetType() == TrackChunk())
              TrackCount++;
          if(not TrackCount) return Clear();
        }

        /*Read the type. Note: type 2 never really caught on and so it is
        unsupported for now.

        Spec: "0) the file contains a single multi-channel track; 1) the file
        contains one or more simultaneous tracks (or MIDI outputs) of a
        sequence; 2) the file contains one or more sequentially independent
        single-track patterns."*/

        uint16 Type = Read16BitInteger(Chunks.a().Data[0], Chunks.a().Data[1]);
        if(Type != 0 and Type != 1) return Clear();

        /*Though the number of tracks is encoded, there is no reason to read
        this as it can be derived automatically.*/

        /*Read the clock divisions.

        Spec: "If bit 15 of <division> is zero, the bits 14 thru 0 represent the
        number of delta time ticks which make up a quarter-note. For instance,
        if the division is 96, then the time interval of an eighth-note between
        two events in the file would be 48.

        If bit 15 of <division> is a one, delta times in a file correspond to
        subdivisions of a second, in a way consistent with SMPTE and MIDI Time
        Code. Bits 14 thru 8 contain one of the four values -24, -25, -29, or
        -30, corresponding to the four standard SMPTE and MIDI Time Code formats
        (-29 corresponds to 30 drop frame), and represents the number of frames
        per second. These negative numbers are stored in two's complement form.
        The second byte (stored positive) is the resolution within a frame:
        typical values may be 4 (MIDI Time Code resolution), 8, 10, 80 (bit
        resolution), or 100."*/
        int8  DivisionsByteOne = int8(Chunks.a().Data[4]);
        uint8 DivisionsByteTwo = Chunks.a().Data[5];
        if(DivisionsByteOne < 0)
        {
          if(not SetSMPTEMode(DivisionsByteOne, count(DivisionsByteTwo)))
            return Clear();
        }
        else
        {
          if(not SetDivisionsPerQuarterNote(((count(DivisionsByteOne)) << 8) +
            count(DivisionsByteTwo)))
              return Clear();
        }

        ImportTrackChunks(Chunks);

        return true;
      }

      private:

      ///Imports each of the track chunks.
      void ImportTrackChunks(const Array<Chunk>& Chunks)
      {
        //Clear any existing tracks.
        Tracks.RemoveAll();

        //For each track chunk, import the track.
        for(count i = 0; i < Chunks.n(); i++)
          if(Chunks[i].GetType() == TrackChunk())
            Chunks[i].ReadIntoTrack(Tracks.Add(), DivisionsPerQuarter);

        //Reorder the event stream to be canonical.
        OrderEventsCanonically();
      }

      ///Sorts all the track events so that they appear in a canonical order.
      void SortTrackEvents()
      {
        for(count i = 0, Order = 1; i < Tracks.n(); i++)
        {
          //Tag events with a tie-break order increasing monotonically from 1.
          for(count j = 0; j < Tracks[i].Events.n(); j++)
            Tracks[i].Events[j].TieBreakOrder = Order++;

          //Sort the track.
          Tracks[i].Events.Sort();

          //Return the tie break order to zero.
          for(count j = 0; j < Tracks[i].Events.n(); j++)
            Tracks[i].Events[j].TieBreakOrder = 0;
        }
      }

      public:

      /**Converts the MIDI file to Type 0. This has the effect of merging all
      the messages on each track into a single interleaved track. This file
      format is best for MIDI players, since they do not need to read mutiple
      tracks in parallel.*/
      void ConvertToType0()
      {
        if(IsType0()) return;
        for(count i = 1; i < Tracks.n(); i++)
        {
          for(count j = 0; j < Tracks[i].Events.n(); j++)
            Tracks[0].Events.Add() = Tracks[i].Events[j];
          Tracks.Remove(i--);
        }

        //Reorder the event stream to be canonical.
        OrderEventsCanonically();
      }

      /**Converts the MIDI file to Type 1. This has the effect of separating all
      the messages into separate channel-based tracks. Note that if the file
      only contains one track, then it will still remain Type 0. The File class
      always writes a Type 0 file if it can. This format is best for using to
      import into notation programs or sequencers where the track structure may
      be important.*/
      void ConvertToType1()
      {
        if(IsType1()) return;

        //Make sure there are sixteen tracks, one for each channel.
        for(count i = Tracks.n() + 1; i < ChannelMaximum; i++)
          Tracks.Add();

        //Move the channeled status events to their respective tracks.
        for(count i = 0; i < Tracks.a().Events.n(); i++)
        {
          Event e = Tracks.a().Events[i];
          if(e.Status >= NoteOff and e.Status <= PitchWheel and e.Channel > 0)
          {
            Tracks[e.Channel & ChannelMask].Events.Add() = e;
            Tracks.a().Events.Remove(i--);
          }
        }

        //Reorder the event stream to be canonical.
        OrderEventsCanonically();
      }

      ///Reads in a MIDI file given a byte array.
      bool Read(const Array<byte>& MIDIFileData)
      {
        return Read(&MIDIFileData.a(), MIDIFileData.n());
      }

      ///Reads in a MIDI file given a filename.
      bool Read(String MIDIFilename)
      {
        Array<byte> MIDIFileData;
        PRIM_NAMESPACE::File::Read(MIDIFilename, MIDIFileData);
        return Read(MIDIFileData);
      }

      ///Determines whether the MIDI file is empty.
      bool IsEmpty() const
      {
        for(count i = 0; i < Tracks.n(); i++)
          if(Tracks[i].Events.n())
            return false;
        return true;
      }

      ///Writes the MIDI file to a filename.
      bool Write(String MIDIFilename)
      {
        Array<byte> MIDIFileData;
        MIDI::File::Write(MIDIFileData);
        if(MIDIFileData.n())
          return PRIM_NAMESPACE::File::Write(MIDIFilename, MIDIFileData);
        return false;
      }

      ///Writes the MIDI file to a byte array.
      void Write(Array<byte>& MIDIFileData)
      {
        OrderEventsCanonically();
        MIDIFileData.Clear();
        if(IsEmpty()) return;
        if(Tracks.n() > 0xffff) return;

        List<Chunk> Chunks;

        //Write the header.
        Chunk& Header = Chunks.Add();
        Header.SetType(HeaderChunk());

        //Format
        Header.Data.Add() = 0;
        Header.Data.Add() = (IsType0() ? 0 : 1);

        //Number of tracks
        Header.Data.Add() = byte(Tracks.n() >> 8);
        Header.Data.Add() = byte(Tracks.n() & 0xff);

        //Division type
        if(SMPTEType < 0)
        {
          Ratio x = 0;
          if(SMPTEType == SMPTE24)
            x = DivisionsPerQuarter / Ratio(24);
          else if(SMPTEType == SMPTE25)
            x = DivisionsPerQuarter / Ratio(25);
          else if(SMPTEType == SMPTE29DropFrame)
            x = DivisionsPerQuarter / Ratio(30000, 1001);
          else if(SMPTEType == SMPTE30)
            x = DivisionsPerQuarter / Ratio(30);
          integer y = x.To<integer>();
          Header.Data.Add() = byte(SMPTEType);
          Header.Data.Add() = byte(y & 0xff);
        }
        else
        {
          integer x = DivisionsPerQuarter.To<integer>();
          Header.Data.Add() = byte(x >> 8);
          Header.Data.Add() = byte(x & 0xff);
        }

        //Create track chunks for each track.
        Ratio TimeScalar = DivisionsPerQuarter * 4;
        for(count i = 0; i < Tracks.n(); i++)
        {
          //Write the events out in each track.
          Chunks.Add().SetType(TrackChunk());
          Ratio LastTime = 0;
          for(count j = 0; j < Tracks[i].Events.n(); j++)
          {
            const Event& e = Tracks[i].Events[j];
            Chunks.z().WriteEvent(e, (e.Time - LastTime) * TimeScalar);
            LastTime = e.Time;
          }
          //Add end of track marker (0x0) (0xff 0x2f 0x00)
          Chunks.z().Data.Add() = 0;
          Chunks.z().Data.Add() = Meta;
          Chunks.z().Data.Add() = EndOfTrack;
          Chunks.z().Data.Add() = 0;
        }

        //Combine all the chunks together.
        for(count i = 0; i < Chunks.n(); i++)
        {
          //Add the 4-byte type.
          String t = Chunks[i].GetType();
          for(count k = 0; k < t.n(); k++)
            MIDIFileData.Add() = byte(t[k]);

          //Add the 4-byte length.
          uint32 Length = uint32(Chunks[i].Data.n());
          MIDIFileData.Add() = byte((Length >> 24) & 0xff);
          MIDIFileData.Add() = byte((Length >> 16) & 0xff);
          MIDIFileData.Add() = byte((Length >> 8) & 0xff);
          MIDIFileData.Add() = byte(Length & 0xff);

          //Add the chunk data.
          for(count j = 0; j < Chunks[i].Data.n(); j++)
            MIDIFileData.Add() = Chunks[i].Data[j];
        }
      }

      ///Returns a string representation of this file for debugging.
      operator String () const
      {
        String s;
        if(not Tracks.n())
          return "Empty MIDI File";
        s >> "Type 0: " << IsType0();
        s >> "Type 1: " << IsType1();
        s >> "SMPTE:  " << HasSMPTEDivisions();
        s >> "Div/QN: " << DivisionsPerQuarter;
        s >> "Tracks: " << Tracks.n();
        for(count i = 0; i < Tracks.n(); i++)
          s >> "" >> "Track " << i >> Tracks[i];
        s >> "" >> "Meta Event Map:";
        List<Event> e = GetMetaEventMap();
        for(count i = 0; i < e.n(); i++)
          s >> e[i];
        return s;
      }
    };

    class Synthesizer
    {
      Random DitherGenerator; PRIM_PAD(Random)

      int32 TriangleDither(float64 SampleValue)
      {
        SampleValue = Clip(SampleValue, -32768.0, 32767.0);

        const float64 DitherBits = 1.0;

        //In range [1, 2)
        float64 Pick1 = DitherGenerator.NextRawFloat64();
        float64 Pick2 = DitherGenerator.NextRawFloat64();

        //In range [-1, 1)
        float64 TriangularPick = Pick1 + Pick2 - 3.;
        TriangularPick *= DitherBits;
        float64 DitheredSampleValue = SampleValue + TriangularPick;
        return int32(Round(DitheredSampleValue));
      }

      void Normalize(AIFF::Multichannel16Bit<int32>& Destination)
      {
        int32 MaxSample = 100;
        for(count c = 0; c < Destination.GetChannelCount(); c++)
        {
          int32* d = Destination.GetChannel(c)->GetRawSamples();
          for(count i = 0; i < Destination.GetSampleCount(); i++)
            if(Abs(d[i]) > MaxSample)
              MaxSample = Abs(d[i]);
        }

        for(count c = 0; c < Destination.GetChannelCount(); c++)
        {
          float64 Multiplier = 32000.f / float64(MaxSample);
          int32* d = Destination.GetChannel(c)->GetRawSamples();
          for(count i = 0; i < Destination.GetSampleCount(); i++)
            d[i] = TriangleDither(float64(d[i]) * Multiplier);
        }
      }

      Tree<String, Pointer<AIFF::Multichannel16Bit<int16> > > SampleCache;
      Value SampleCacheHistogram;

      count MaximumSamplesToCache;

      Pointer<AIFF::Multichannel16Bit<int16> > GetSample(String SampleFolder,
        count Key, count Volume)
      {
        String k = NameOfSample(SampleFolder, Key, Volume);
        Pointer<AIFF::Multichannel16Bit<int16> > Sample;
        if(not SampleCache.Contains(k))
        {
          //Prune sample cache
          if(SampleCache.n() >= MaximumSamplesToCache)
          {
            Value HistogramKeys = SampleCacheHistogram.Keys();
            String MinKey = HistogramKeys[0].AsString();
            count MinCache = SampleCacheHistogram[HistogramKeys[0]].AsCount();
            for(count i = 1; i < HistogramKeys.n(); i++)
            {
              count Current = SampleCacheHistogram[HistogramKeys[i]].AsCount();
              if(Current < MinCache)
              {
                MinCache = Current;
                MinKey = HistogramKeys[i].AsString();
              }
            }
            SampleCacheHistogram[Value(MinKey)] = Value();
            SampleCacheHistogram.Prune();
            SampleCache.Remove(MinKey);
          }

          SampleCache[k] = Sample.New();
          Sample->Assume16BitStereoAndReadFromFile(k);
          SampleCacheHistogram[k] = 0;
        }
        SampleCacheHistogram[k] = SampleCacheHistogram[k].AsCount() + 1;
        return SampleCache[k];
      }

      static String NameOfSample(String SampleFolder, count KeyNumber,
        count Volume)
      {
        String s = SampleFolder;
        s << "/v";
        if(Volume < 10) s << "0";
        s << Volume << "_k";
        if(KeyNumber < 10) s << "00";
        else if(KeyNumber < 100) s << "0";
        s << KeyNumber << ".aiff";
        return s;
      }

      static Array<number> GetEventTimes(const Track& t)
      {
        Array<number> EventTimes;
        EventTimes.n(t.Events.n());
        EventTimes.Zero();
        Ratio MicrosecondsPerQuarterNote = 416666;
        Ratio OffsetLocation;
        number CurrentOffset = 0.f;
        for(count i = 0; i < t.Events.n(); i++)
        {
          Event e = t.Events[i];
          number ActualTime = CurrentOffset +
            ((e.Time - OffsetLocation) * MicrosecondsPerQuarterNote *
            Ratio(4, 1000000)).To<number>();
          EventTimes[i] = ActualTime;
          if(e.Status == MIDI::Meta and e.MetaType == MIDI::SetTempo)
          {
            MicrosecondsPerQuarterNote = e.Tempo;
            CurrentOffset = ActualTime;
            OffsetLocation = e.Time;
          }
        }
        return EventTimes;
      }

      public:

      static Value GetNoteList(const Track& t)
      {
        Array<number> EventTimes = GetEventTimes(t);
        Array<count> ProgramState(16);
        ProgramState.Zero();

        Value NoteList;
        for(count i = 0; i < t.Events.n(); i++)
        {
          Event e = t.Events[i];
          if(e.Status == MIDI::NoteOn)
          {
            for(count j = i + 1; j < t.Events.n(); j++)
            {
              Event e2 = t.Events[j];
              if((e2.Status == MIDI::NoteOff or e2.Status == MIDI::NoteOn) and
                e.Channel == e2.Channel and e.Note == e2.Note)
              {
                number Duration = EventTimes[j] - EventTimes[i];
                if(Duration > 0.05f and Duration < 60.f)
                {
                  Value v;
                  v["Key"] = count(e.Note);
                  v["Velocity"] = number(e.Velocity) / 128.f;
                  v["Program"] = ProgramState[e.Channel];
                  v["Start"] = EventTimes[i];
                  v["Duration"] = Duration;
                  v["Channel"] = e.Channel;
                  NoteList.Add() = v;
                }
                j = t.Events.n();
              }
            }
          }
          else if(e.Status == MIDI::ProgramChange)
          {
            ProgramState[Min(Max(count(e.Channel), count(0)), count(15))] =
              e.Program;
          }
        }
        return NoteList;
      }

      private:

      static void MixSample(AIFF::Multichannel16Bit<int32>& Destination,
        Pointer<AIFF::Multichannel16Bit<int16> > Sample, number Offset,
        number Duration, number Decay = 0.1f)
      {
        if(Offset < 0.f or Duration < 0.f or Decay < 0.f or not Sample)
          return;
        Duration += Decay;
        number SampleRate = number(Destination.GetSampleRate());
        count SampleDuration = count(Duration * SampleRate);
        if(SampleDuration > Sample->GetSampleCount())
          SampleDuration = Sample->GetSampleCount();
        count SampleStart = count(Offset * SampleRate);
        count SampleDecayEnd = SampleStart + SampleDuration - 1;
        count SampleDecayStart = SampleDecayEnd - count(Decay * SampleRate);

        if(SampleDecayEnd >= Destination.GetSampleCount() or
          SampleDecayStart + 2 >= SampleDecayEnd or SampleDuration < 2)
            return;

        for(count c = 0; c < Destination.GetChannelCount(); c++)
        {
          int32* d = Destination.GetChannel(c)->GetRawSamples();
          const int16* s = Sample->GetChannel(c)->GetRawSamples();

          for(count i = SampleStart; i < SampleDecayStart; i++)
            d[i] += int32(s[i - SampleStart]);
          for(count i = SampleDecayStart; i <= SampleDecayEnd; i++)
          {
            number x = number(s[i - SampleStart]);
            number a = number(i - SampleDecayStart);
            number b = number(SampleDecayEnd - SampleDecayStart);
            d[i] += int32((1.f - a / b) * x);
          }
        }
      }

      public:

      static Array<count> GetAvailableVolumes(String SampleFolder)
      {
        Array<count> Volumes;
        for(count i = 0; i < 100; i++)
          if(PRIM_NAMESPACE::File::Length(
            NameOfSample(SampleFolder, 60, i)) > 0)
              Volumes.Add() = i;
        return Volumes;
      }

      bool Synthesize(const Array<byte>& MIDIData, String SampleFolder,
        String OutputFilename, count MaximumSamplesToCache_ = 20)
      {
        //Set limits on sample caching.
        MaximumSamplesToCache = MaximumSamplesToCache_;

        //Stabilize results by using consistent seed for dither generator.
        DitherGenerator.PickSequence(0xB00DA);

        //Check which levels are available.
        Array<count> Volumes = GetAvailableVolumes(SampleFolder);

        //If no sounds available, then abort.
        if(not Volumes.n())
          return false;

        //Read in MIDI data.
        File MIDIFile;
        MIDIFile.Read(MIDIData);

        //Convert to single-stream type 0 format so all events are on one track.
        MIDIFile.ConvertToType0();

        //If no tracks, abort.
        if(not MIDIFile.Tracks.n())
          return false;

        Value Notes = GetNoteList(MIDIFile.Tracks.a());
        number MaxDuration = +Notes.z()["Start"] + +Notes.z()["Duration"] + 1.f;
        AIFF::Multichannel16Bit<int32> Out(2, 44100, MaxDuration);

        if(not Notes.n())
          return false;

        for(count i = 0; i < Notes.n(); i++)
        {
          Value s = Notes[i];
          Pointer<AIFF::Multichannel16Bit<int16> > Sample =
            GetSample(SampleFolder, s["Key"].AsCount(),
              Volumes[count(+s["Velocity"] * number(Volumes.n()))]);
          MixSample(Out, Sample, +s["Start"], +s["Duration"]);
        }
        Normalize(Out);

        if(OutputFilename.EndsWith(".aif") or OutputFilename.EndsWith(".aiff"))
          Out.WriteToFile(OutputFilename);
        else if(OutputFilename.EndsWith(".wav"))
          Out.WriteToWAVFile(OutputFilename);
        else
          return false;

        return true;
      }
    };
  };
}
#endif
#endif
