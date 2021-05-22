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

#ifndef PRIM_INCLUDE_AIFF_H
#define PRIM_INCLUDE_AIFF_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  class AIFF
  {
    public:

    ///Representation of the 80-bit float format for use in file serialization.
    class Float80BigEndian
    {
      public:

      ///The 80-bits stored in big-endian format.
      byte Bytes[10];

      ///Initializes number to zero.
      Float80BigEndian() {ConvertFromInt(0);}

      ///Converts a uint32 to an 80-bit IEEE Standard 754 floating point.
      void ConvertFromInt(uint32 IntToConvert)
      {
        //Calculate and store the exponent of the int.
        {
          uint32 Exponent = 0x3FFF; //Bias exponent
          uint32 DividingInt = IntToConvert;
          while(DividingInt /= 2) Exponent++;

          //Set exponent part in bits (64-78, leave sign bit 79 = 0 = positive)
          Bytes[0] = byte((Exponent >> 8) & 0xFF);
          Bytes[1] = byte((Exponent >> 0) & 0xFF);
        }

        /*Convert int to fractional part by multiplying by 2 until highest bit
        becomes high bit.*/
        {
          uint32 FractionalPart = IntToConvert;
          while(FractionalPart and not (FractionalPart & 0x80000000))
            FractionalPart *= 2;

          //Set fractional part in bits (32-63)
          Bytes[2] = byte((FractionalPart >> 24) & 0xFF);
          Bytes[3] = byte((FractionalPart >> 16) & 0xFF);
          Bytes[4] = byte((FractionalPart >>  8) & 0xFF);
          Bytes[5] = byte((FractionalPart >>  0) & 0xFF);
        }

        //Clear remaining fractional part in bits (0-31)
        Bytes[6] = Bytes[7] = Bytes[8] = Bytes[9] = 0;
      }
    };

    ///Represents a 16-bit PCM audio channel.
    template<class T>
    class Channel
    {
      Array<T> Samples;

      public:

      ///Initializes the channel with a certain number of samples.
      Channel(count SampleCount) : Samples(SampleCount) {Samples.Zero();}

      ///Gets a pointer to the sample at a particular location.
      inline T* GetRawSamples(count i = 0) {return &Samples[i];}

      ///Gets the value of a particular sample.
      inline T GetSample(count i) const {return Samples[i];}

      ///Returns the number of samples in this channel.
      inline count GetSampleCount(void) const {return Samples.n();}

      ///Sets the value of a particular sample.
      inline void SetSample(count i, T Value) {Samples[i] = Value;}

      ///Adds a value to a particular sample's existing value.
      inline void SumToSample(count i, T Value) {Samples[i] += Value;}
    };

    ///A wrapper class for quickly writing multi-channel 16-bit AIFF files.
    template <class T>
    class Multichannel16Bit
    {
      Array<Pointer<Channel<T> > > Channels;
      count SampleCount;
      count SampleRate;

      static void Store16BitValue(byte* Destination, int16 IntValue)
      {
        Destination[0] = byte((IntValue >> 8) & 0xFF);
        Destination[1] = byte((IntValue >> 0) & 0xFF);
      }

      static void Store16BitValueLE(byte* Destination, int16 IntValue)
      {
        Destination[0] = byte((IntValue >> 0) & 0xFF);
        Destination[1] = byte((IntValue >> 8) & 0xFF);
      }

      static void Store16BitValue(byte* Destination, uint16 IntValue)
      {
        Store32BitValue(Destination, int16(IntValue));
      }

      static void Store32BitValue(byte* Destination, int32 IntValue)
      {
        Destination[0] = byte((IntValue >> 24) & 0xFF);
        Destination[1] = byte((IntValue >> 16) & 0xFF);
        Destination[2] = byte((IntValue >>  8) & 0xFF);
        Destination[3] = byte((IntValue >>  0) & 0xFF);
      }

      static void Store32BitValueLE(byte* Destination, int32 IntValue)
      {
        Destination[0] = byte((IntValue >>  0) & 0xFF);
        Destination[1] = byte((IntValue >>  8) & 0xFF);
        Destination[2] = byte((IntValue >> 16) & 0xFF);
        Destination[3] = byte((IntValue >> 24) & 0xFF);
      }

      static void Store32BitValue(byte* Destination, uint32 IntValue)
      {
        Store32BitValue(Destination, int32(IntValue));
      }

      public:

      void Configure(count ChannelCount_, count SampleRate_, count Samples)
      {
        SetSampleRate(SampleRate_);
        SampleCount = Samples;
        Channels.n(Max(ChannelCount_, count(1)));
        for(count i = 0; i < Channels.n(); i++)
          Channels[i] = new Channel<T>(SampleCount);
      }

      ///Initializes an empty mono AIFF file.
      Multichannel16Bit() {Configure(1, 44100, 0);}

      ///Initializes the AIFF file with channels, a sample rate, and duration.
      Multichannel16Bit(count ChannelCount_, count SampleRate_,
        number SecondsDuration)
      {
        SetSampleRate(SampleRate_);
        if(not (SecondsDuration > 0.f))
          SecondsDuration = 0.f;
        Configure(ChannelCount_, SampleRate,
          count(Round(number(SampleRate) * SecondsDuration)));
      }

      ///Returns the sample rate associated with this audio data.
      count GetSampleRate() {return SampleRate;}

      ///Returns the number of samples.
      count GetSampleCount() {return SampleCount;}

      ///Returns the number of channels.
      count GetChannelCount() {return Channels.n();}

      ///Changes the internal sample rate without resampling.
      void SetSampleRate(count SampleRate_)
      {
        SampleRate = Min(Max(SampleRate_, count(8000)), count(192000));
      }

      ///Returns a pointer to one of the 16-bit channels.
      Pointer<Channel<T> > GetChannel(count i) {return Channels[i];}

      ///Reads in a 16-bit stereo audio file.
      void Assume16BitStereoAndReadFromFile(const ascii* Filename,
        count AssumedSampleRate = 44100)
      {
        String Data;
        File::Read(Filename, Data);
        count SoundChunk = Data.Find("SSND");
        if(SoundChunk > 32 and SoundChunk < Data.n())
        {
          count SoundChunkSize = SoundChunk + 4;
          count Samples = 0;
          Samples += count(uint32(Data[SoundChunkSize + 0]) << 24);
          Samples += count(uint32(Data[SoundChunkSize + 1]) << 16);
          Samples += count(uint32(Data[SoundChunkSize + 2]) <<  8);
          Samples += count(uint32(Data[SoundChunkSize + 3]) <<  0);
          Samples = (Samples - 8) / 4;
          Configure(2, AssumedSampleRate, Samples);

          for(count c = 0; c < Channels.n(); c++)
          {
            T* Destination = Channels[c]->GetRawSamples();
            const byte* Source = reinterpret_cast<const byte*>(
              &Data.Merge()[SoundChunk + 16 + c * 2]);
            for(count i = 0; i < Samples; i++, Source += 4)
              Destination[i] = T(int16(Source[0]) << 8) + int16(Source[1]);
          }
        }
        else
          Configure(2, AssumedSampleRate, 0);
      }

      // Reads in a 16-bit stereo PCM byte buffer
      void Assume16BitStereoAndReadFromPCMData(const byte* Source,
        count SourceByteCount, count AssumedSampleRate = 44100)
      {
        count Samples = SourceByteCount / 4;
        if(Source and Samples > 0)
        {
          Configure(2, AssumedSampleRate, Samples);
          T* LDestination = Channels[0]->GetRawSamples();
          T* RDestination = Channels[1]->GetRawSamples();
          for(count i = 0; i < Samples; i++, Source += 4)
            LDestination[i] = T((int16(Source[1]) << 8) + int16(Source[0])),
            RDestination[i] = T((int16(Source[3]) << 8) + int16(Source[2]));
        }
        else
          Configure(2, AssumedSampleRate, 0);
      }

      ///Writes this AIFF to file.
      void WriteToFile(const ascii* Filename)
      {
        byte FORMAIFF[12] = {'F', 'O', 'R', 'M', 0, 0, 0, 0,
          'A', 'I', 'F', 'F'};

        byte COMMONCHUNK[26];
        COMMONCHUNK[0] = 'C';
        COMMONCHUNK[1] = 'O';
        COMMONCHUNK[2] = 'M';
        COMMONCHUNK[3] = 'M';

        int32 CHUNKSIZE = 18;
        int16 NUMCHANNELS = int16(Channels.n());
        uint32 NUMSAMPLEFRAMES = uint32(SampleCount);
        int16 SAMPLESIZE = 16;
        Float80BigEndian SAMPLERATE;
        SAMPLERATE.ConvertFromInt(uint32(SampleRate));

        Store32BitValue(&COMMONCHUNK[4], CHUNKSIZE);
        Store16BitValue(&COMMONCHUNK[8], NUMCHANNELS);
        Store32BitValue(&COMMONCHUNK[10], NUMSAMPLEFRAMES);
        Store16BitValue(&COMMONCHUNK[14], SAMPLESIZE);

        for(int i = 0; i < 10; i++)
          COMMONCHUNK[i + 16] = SAMPLERATE.Bytes[i];

        byte SSNDCHUNK[16] = {'S', 'S', 'N', 'D', 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0};

        int32 SSNDCHUNKSIZE = int32(8 + 2 * SampleCount * Channels.n());
        Store32BitValue(&SSNDCHUNK[4], SSNDCHUNKSIZE);

        uint32 FileLength = 4 + 8 + uint32(CHUNKSIZE) + 8 +
          uint32(SSNDCHUNKSIZE);
        Store32BitValue(&FORMAIFF[4], FileLength);

        Array<byte> HEADER(12 + 26 + 16);
        for(count i = 0; i < 12; i++) HEADER[i] = FORMAIFF[i];
        for(count i = 0; i < 26; i++) HEADER[i + 12] = COMMONCHUNK[i];
        for(count i = 0; i < 16; i++) HEADER[i + 12 + 26] = SSNDCHUNK[i];
        File::Write(Filename, HEADER);

        const count BufferSamples = 1024 * 256;
        const count ChannelCount = Channels.n();
        Array<int16> Buffer;
        for(count i = 0; i < SampleCount; i += BufferSamples)
        {
          count CurrentSamples = BufferSamples;
          if(SampleCount - i < BufferSamples)
            CurrentSamples = SampleCount - i;

          Buffer.n(CurrentSamples * ChannelCount);

          for(count c = 0; c < ChannelCount; c++)
          {
            Pointer<Channel<T> > Channel = Channels[c];
            T* Source = Channel->GetRawSamples(i);
            T* SourceEnd = Source + CurrentSamples;
            byte* Destination = reinterpret_cast<byte*>(&Buffer[c]);
            while(Source != SourceEnd)
            {
              uint16 v = uint16(int16(Clip(*Source++, T(-32768), T(32767))));
              Destination[0] = byte((v >> 8) & 0xff);
              Destination[1] = byte((v >> 0) & 0xff);
              Destination += ChannelCount * 2;
            }
          }

          File::Append(Filename, Buffer);
        }
      }

      ///Writes this AIFF to a WAV file.
      void WriteToWAVFile(const ascii* Filename)
      {
        count DataSize = Channels.n() * SampleCount *  2;

        Array<byte> HEADER(44);

        //'RIFF' chunk
        HEADER[0] = byte('R');
        HEADER[1] = byte('I');
        HEADER[2] = byte('F');
        HEADER[3] = byte('F');

        //RIFF chunk size
        Store32BitValueLE(&HEADER[4], int32(36 + DataSize));

        //RIFF format = 'WAVE'
        HEADER[8] = byte('W');
        HEADER[9] = byte('A');
        HEADER[10] = byte('V');
        HEADER[11] = byte('E');

        //'fmt ' chunk
        HEADER[12] = 'f';
        HEADER[13] = 'm';
        HEADER[14] = 't';
        HEADER[15] = ' ';

        //Subchunk 1 size = 16
        Store32BitValueLE(&HEADER[16], int32(16));

        //PCM = 1
        Store16BitValueLE(&HEADER[20], int16(1));

        //Channels
        Store16BitValueLE(&HEADER[22], int16(Channels.n()));

        //Sample rate
        Store32BitValueLE(&HEADER[24], int32(SampleRate));

        //Byte rate
        Store32BitValueLE(&HEADER[28], int32(SampleRate * Channels.n() * 2));

        //Block align
        Store16BitValueLE(&HEADER[32], int16(Channels.n() * 2));

        //Sample bits
        Store16BitValueLE(&HEADER[34], int16(16));

        //'data' chunk
        HEADER[36] = 'd';
        HEADER[37] = 'a';
        HEADER[38] = 't';
        HEADER[39] = 'a';

        //Data chunk size
        Store32BitValueLE(&HEADER[40], int32(DataSize));

        File::Write(Filename, HEADER);

        const count BufferSamples = 1024 * 256;
        const count ChannelCount = Channels.n();
        Array<int16> Buffer;
        for(count i = 0; i < SampleCount; i += BufferSamples)
        {
          count CurrentSamples = BufferSamples;
          if(SampleCount - i < BufferSamples)
            CurrentSamples = SampleCount - i;

          Buffer.n(CurrentSamples * ChannelCount);

          for(count c = 0; c < ChannelCount; c++)
          {
            Pointer<Channel<T> > Channel = Channels[c];
            T* Source = Channel->GetRawSamples(i);
            T* SourceEnd = Source + CurrentSamples;
            byte* Destination = reinterpret_cast<byte*>(&Buffer[c]);
            while(Source != SourceEnd)
            {
              uint16 v = uint16(int16(Clip(*Source++, T(-32768), T(32767))));
              Destination[0] = byte((v >> 0) & 0xff);
              Destination[1] = byte((v >> 8) & 0xff);
              Destination += ChannelCount * 2;
            }
          }

          File::Append(Filename, Buffer);
        }
      }
    };
  };
}
#endif
