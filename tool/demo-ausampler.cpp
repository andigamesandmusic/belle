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
#include "prim.h"
using namespace prim;

static Value GetCommandLineSchema()
{
  Value Schema = Arguments::NewSchema("AUSampler",
    "Creates sample-base zones from an existing .aupreset",
    "",
    "...");
  Arguments::AddParameter(Schema, "in", Arguments::InputFile,
    Arguments::Required, "Input file", "Template .aupreset");
  Arguments::AddParameter(Schema, "samples", Arguments::InputFile,
    Arguments::Required, "Samples", "Samples directory containing m4a folder");
  Arguments::AddParameter(Schema, "out", Arguments::OutputFile,
    Arguments::Required, "Output file", ".aupreset");
  Arguments::AddParameter(Schema, "relative", Arguments::Flag,
    Arguments::Optional, "Relative paths", "Use relative paths to sampler");
  return Schema;
}

static String ZeroPad(count n, count x)
{
  String s = x;
  while(s.n() < n)
    s.Prepend("0");
  return s;
}

int main(int argc, const char** argv)
{
  AutoRelease<Console> ReleasePool;
  if(Value Arguments = Arguments::Process(argc, argv, GetCommandLineSchema()))
  {
    C::Out() >> JSON::Export(Arguments);
    String Template = File::Read(Arguments["in"].AsString());
    bool Relative = Arguments["relative"].AsBoolean();

    //Replace zone information with $1 token.
    {
      String ZoneStartTag = "<key>Zones</key>";
      String ZoneEndTag = "</array>";
      count ZonesStart = Template.Find(ZoneStartTag);
      count ZonesEnd = Template.Find(ZoneEndTag, ZonesStart) +
        ZoneEndTag.n() - 1;
      Template.Erase(ZonesStart, ZonesEnd);
      Template.Insert("$1", ZonesStart);
    }

    //Replace sample file reference information with $2 token.
    {
      String SampleStartTag = "<key>file-references</key>";
      String SampleEndTag = "</dict>";
      count SamplesStart = Template.Find(SampleStartTag);
      count SamplesEnd = Template.Find(SampleEndTag, SamplesStart) +
        SampleEndTag.n() - 1;
      Template.Erase(SamplesStart, SamplesEnd);
      Template.Insert("$2", SamplesStart);
    }

    //Create the zone table that defines how samples are zoned.
    Value ZoneTable;
    String Format = "m4a";
    count j = 0;

    count SemitoneSkip = 1;
    Value Velocities;
    Velocities.Add() = 2;
    Velocities.Add() = 5;
    Velocities.Add() = 8;
    const count DenseRangeMin = 60 - 12;
    const count DenseRangeMax = 60 + 12;
    for(count v = 0; v < Velocities.n(); v++)
    {
      for(count i = 20; i <= 109; i += SemitoneSkip)
      {
        SemitoneSkip = (i >= DenseRangeMin and i <= DenseRangeMax) ? 1 : 2;
        Value Zone;

        String ZoneFileStem = String("v") +
          ZeroPad(2, Velocities[v].AsCount()) +
          String("_k") + ZeroPad(3, i) + String(".") + Format;
        String RelativePath = String("Piano/") + ZoneFileStem;
        String AbsolutePath = Arguments["samples"].AsString() + String("/") +
          Format + String("/") + ZoneFileStem;
        Zone["file"] = Relative ? RelativePath : AbsolutePath;
        if(not File::Length(AbsolutePath))
        {
          C::Error() >> AbsolutePath << " is not a valid sample.";
          return 1;
        }

        Zone["waveform"] = j + 0x10000000;
        Zone["minkey"] = i;
        Zone["maxkey"] = i + SemitoneSkip - 1;
        Zone["minvel"] = Clip(count(v * 128 / Velocities.n()),
          count(0), count(127));
        Zone["maxvel"] = Clip(count((v + 1) * 128 / Velocities.n() - 1),
          count(0), count(127));
        Zone["rootkey"] = i;
        ZoneTable[j++] = Zone;
      }
    }
    C::Out() >> "Created " << j << " zones";

    //Create new zones.
    String NewZones;
    {
      String Indent = "\t\t\t\t";
      String Indentt = "\t\t\t\t\t";
      String Indenttt = "\t\t\t\t\t\t";
      NewZones >> "<key>Zones</key>";
      NewZones >> Indent << "<array>";
      for(count i = 0; i < ZoneTable.n(); i++)
      {
        NewZones >> Indentt << "<dict>";
        NewZones >> Indenttt << "<key>ID</key>";
        NewZones >> Indenttt << "<integer>" << (i + 1) << "</integer>";
        NewZones >> Indenttt << "<key>enabled</key>";
        NewZones >> Indenttt << "<true/>";
        NewZones >> Indenttt << "<key>loop enabled</key>";
        NewZones >> Indenttt << "<false/>";
        NewZones >> Indenttt << "<key>max key</key>";
        NewZones >> Indenttt << "<integer>" <<
          ZoneTable[i]["maxkey"].AsCount() << "</integer>";
        NewZones >> Indenttt << "<key>max vel</key>";
        NewZones >> Indenttt << "<integer>" <<
          ZoneTable[i]["maxvel"].AsCount() << "</integer>";
        NewZones >> Indenttt << "<key>min key</key>";
        NewZones >> Indenttt << "<integer>" <<
          ZoneTable[i]["minkey"].AsCount() << "</integer>";
        NewZones >> Indenttt << "<key>min vel</key>";
        NewZones >> Indenttt << "<integer>" <<
          ZoneTable[i]["minvel"].AsCount() << "</integer>";
        NewZones >> Indenttt << "<key>root key</key>";
        NewZones >> Indenttt << "<integer>" <<
          ZoneTable[i]["rootkey"].AsCount() << "</integer>";
        NewZones >> Indenttt << "<key>waveform</key>";
        NewZones >> Indenttt << "<integer>" <<
          ZoneTable[i]["waveform"].AsCount() << "</integer>";
        NewZones >> Indentt << "</dict>";
      }
      NewZones >> Indent << "</array>";
    }

    //Create new samples.
    String NewSamples;
    {
      String Indent = "\t";
      String Indentt = "\t\t";
      NewSamples >> "<key>file-references</key>";
      NewSamples >> Indent << "<dict>";
      for(count i = 0; i < ZoneTable.n(); i++)
      {
        NewSamples >> Indentt << "<key>Sample:" <<
          ZoneTable[i]["waveform"].AsCount() << "</key>";
        NewSamples >> Indentt << "<string>" <<
          ZoneTable[i]["file"].AsString() << "</string>";
      }
      NewSamples >> Indent << "</dict>";
    }

    //Update template with new zones and new samples.
    Template.Replace("$1", NewZones);
    Template.Replace("$2", NewSamples);

    File::Write(Arguments["out"].AsString(), Template);
    if(not Relative)
    {
      String Path = Arguments["out"].AsString();
      Path.Replace(Path.ToFilename(), "");
      for(count i = 0; i < ZoneTable.n(); i++)
      {
        Array<byte> Data;
        File::Read(ZoneTable[i]["file"].AsString(), Data);
        File::Write(Path + ZoneTable[i]["file"].AsString().ToFilename(), Data);
      }
    }
  }
}
