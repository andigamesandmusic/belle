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

#ifndef BELLE_ENGRAVER_PUBLISHER_H
#define BELLE_ENGRAVER_PUBLISHER_H

namespace BELLE_NAMESPACE
{
  class Publisher
  {
    public:
    static Value GetCommandLineSchema()
    {
      Value Schema;
      Schema = Arguments::NewSchema("belle",
        "Renders sheet music",
        String::LoremIpsum(),
        "");

      Arguments::AddParameter(Schema, "input", Arguments::InputFile,
        Arguments::Required, "Score file (shorthand, graph, MusicXML)",
        "Input");
      Arguments::AddParameter(Schema, "open", Arguments::Flag,
        Arguments::Optional, "Opens output files",
        "Output");
      Arguments::AddParameter(Schema, "synthesize", Arguments::OutputFile,
        Arguments::Optional, "Audio file (AIFF, FLAC, MIDI, MP3, OGG, WAV)",
        "MIDI Synthesis");
      Arguments::AddParameter(Schema, "instruments", Arguments::PropertyList,
        Arguments::Optional, "General MIDI instruments to use (i.e. piano)",
        "MIDI Synthesis");
      Arguments::AddParameter(Schema, "bpm", Arguments::Property,
        Arguments::Optional, "Tempo in BPM (i.e. 90)",
        "MIDI Synthesis");
      return Schema;
    }

    static void OpenDocument(String Filename)
    {
      String Input, Output, Error;
      Shell::PipeInOut(Shell::GetProcessOnPath("open"), Input, Output, Error,
        Filename);
    }

    static void SynthesizeMIDI(const Music& M, const Value& Args)
    {
      Ratio Tempo = 90, Offset;
      if(Args.Contains("bpm"))
      {
        Tempo = Clip(Args["bpm"].AsRatio(), Ratio(20), Ratio(400));
        C::Out() >> "Selected BPM: " << Tempo;
      }
      if(Args.Contains("instruments"))
      {
        C::Out() >> "Selected MIDI instruments:";
        for(count i = 0; i < Args["instruments"].n(); i++)
        {
          C::Out() >> " * ";
          if(Args["instruments"][i].IsInteger())
            C::Out() << MIDIInstruments()[Clip(
              Args["instruments"][i].AsCount(), count(0), count(127))];
          else
            C::Out() << MIDIInstruments()[MIDILookupInstrument(
              Args["instruments"][i].AsString())];
        }
      }
      Array<byte> MIDIData = MIDIGenerateFromSystem(M, Offset, Tempo,
        Args["instruments"]);
      String Filename = Args["synthesize"].AsString();
      MIDISynthesizeAudio(MIDIData, Filename);

      if(Args.Contains("open"))
        OpenDocument(Filename);
    }

    void Publish(const Value& Args)
    {
      C::Out() >> JSON::Export(Args);
      String ScoreData = File::Read(Args["input"].AsString());
      Music M;
      M.ImportXML(ConvertToXML(ScoreData));
      if(Args.Contains("synthesize"))
        SynthesizeMIDI(M, Args);
    }
  };
}
#endif
