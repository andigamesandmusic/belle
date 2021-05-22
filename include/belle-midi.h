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

String MIDIConfigForSoundFont(String s);
String MIDIDefaultSoundFontLocation();
void MIDIDownloadSoundFontIfNecessary();
void MIDIDownloadSoundFontIfNecessary(String Location, String Resource);
Array<Value> MIDIGenerateFromSystem(MIDI::File& f, const Music& M,
  Ratio& Offset, Ratio Tempo, Value MIDIInstrumentsForEachStaff);
Array<Value> MIDIGenerateFromSystem(MIDI::File& f, const Music& M,
  Ratio& Offset, Ratio Tempo, Value MIDIInstrumentsForEachStaff,
  bool WithExpression);
Array<byte> MIDIGenerateFromSystem(const Music& M, Ratio& Offset, Ratio Tempo,
  Value MIDIInstrumentsForEachStaff);
Array<byte> MIDIGenerateFromSystem(const Music& M, Ratio& Offset, Ratio Tempo,
  Value MIDIInstrumentsForEachStaff, bool WithExpression);
Value MIDIInstruments();
bool MIDIIsValidAudioFormat(const String& Filename);
count MIDILookupInstrument(String x);
String MIDIShortName(String x);
String MIDISoundFontResource();
count MIDISoundFontResourceLength();
void MIDISynthesizeAudio(String MIDIFilename, String AudioFilename);
void MIDISynthesizeAudio(MIDI::File& MIDIFile, String AudioFilename);
void MIDISynthesizeAudio(const Array<byte>& MIDI, String AudioFilename);

#ifdef BELLE_IMPLEMENTATION

String MIDIConfigForSoundFont(String SoundFontFilename)
{
  SoundFontFilename.Replace(".sf2", "");
  return SoundFontFilename + ".cfg";
}

String MIDIDefaultSoundFontLocation()
{
  return "/tmp/SoundFont.sf2";
}

void MIDIDownloadSoundFontIfNecessary()
{
  MIDIDownloadSoundFontIfNecessary(MIDIDefaultSoundFontLocation(),
    MIDISoundFontResource());
}

void MIDIDownloadSoundFontIfNecessary(String Location, String Resource)
{
  if(File::Length(Location) != MIDISoundFontResourceLength())
  {
    String Config = MIDIConfigForSoundFont(Location);
    File::Write(Config, "");
    C::Out() >> "Downloading MIDI SoundFont... (" <<
      (MIDISoundFontResourceLength() / 1000000) << "MB)";
    String In, Out, Error;
    Shell::PipeInOut(Shell::GetProcessOnPath("curl"), In, Out, Error,
      "-L", "-o", Location, Resource);

    String ConfigData;
    if(File::Length(Location) != MIDISoundFontResourceLength())
      C::Error() >> "Could not download MIDI SoundFont from " << Resource;
    else
    {
      C::Out() >> "SoundFont is located at:  " << Location;
      ConfigData = String("soundfont ") + Location;
    }

    C::Out() >>   "Timidity++ configuration: " << Config;
    File::Write(Config, ConfigData);
  }
}

Array<Value> MIDIGenerateFromSystem(MIDI::File& f, const Music& M,
  Ratio& Offset, Ratio Tempo, Value MIDIInstrumentsForEachStaff)
{
  return MIDIGenerateFromSystem(f, M, Offset, Tempo,
    MIDIInstrumentsForEachStaff, false);
}

Array<Value> MIDIGenerateFromSystem(MIDI::File& f, const Music& M,
  Ratio& Offset, Ratio Tempo, Value MIDIInstrumentsForEachStaff,
  bool WithExpression)
{
  List<Array<Music::ConstNode> > NodeMatrix;
  Matrix<Ratio> RhythmMatrix;

  if(not M.Root())
    return Array<Value>();
  Pointer<Geometry> G = System::Get(M.Root().Const())["Geometry"].
    NewObjectIfEmpty<class Geometry>();
  if(not G or not G->GetNumberOfParts() or not G->GetNumberOfInstants())
  {
    Pointer<Music> S;
    S.New()->ImportXML(M.ExportXML());
    Tree<Music::ConstNode, VectorInt> NodeToIndexLookup;
    GetRhythmicOnsetInfo(S, NodeMatrix, RhythmMatrix, NodeToIndexLookup);
  }
  else
  {
    Rhythm::CreateRhythmOrderedRegion(G, NodeMatrix);
    RhythmMatrix = Rhythm::GetUnpackedRhythmicOnsets(NodeMatrix);
  }

  if(Offset.IsEmpty() or Offset <= 0)
    Offset = 0;

  Ratio EndTime = Offset;
  Array<Value> Moments;
  {
    if(Offset == 0)
      f.SetTempo(0, Tempo);

    count Parts = RhythmMatrix.m();
    for(count p = 0; p < Parts; p++)
    {
      if(p >= f.Tracks.n())
        f.Tracks.Add();
      count MIDIInstrument = 0;
      if(count n = MIDIInstrumentsForEachStaff.n())
      {
        Value MIDIInstrumentVal = MIDIInstrumentsForEachStaff[p % n];
        if(MIDIInstrumentVal.IsInteger())
          MIDIInstrument = MIDIInstrumentVal.AsCount();
        else
          MIDIInstrument = MIDILookupInstrument(MIDIInstrumentVal.AsString());
      }
      f.Tracks[p].ProgramChange(0,
        Clip(MIDIInstrument, count(0), count(127)), (p % 16) + 1);
    }

    RhythmMatrix = Rhythm::ForceAlignRhythmMatrix(RhythmMatrix);
    Moments.n(RhythmMatrix.n());
    for(count Part = 0; Part < Parts; Part++)
    {
      for(count Instant = 0; Instant < RhythmMatrix.n(); Instant++)
      {
        Ratio Onset = RhythmMatrix(Part, Instant);
        if(Onset.IsEmpty())
          continue;
        //Allow some extra time for MIDI program changes to take effect.
        Onset += Offset + Ratio(1, 32);

        Array<Music::ConstNode> Chords = ChordsOfIsland(
          NodeMatrix[Instant][Part]);
        for(count i = 0; i < Chords.n(); i++)
        {
          bool IsBeginningBeamGroup = IsChordBeginningOfBeamGroup(Chords[i]);
          Array<Music::ConstNode> Notes = NotesOfChord(Chords[i]);
          for(count j = 0; j < Notes.n(); j++)
          {
            Ratio Duration = TiedDuration(Notes[j]);
            mica::Concept Pitch = ActualPitchOfNote(Notes[j]);
            mica::Concept NoteNumber = mica::map(Pitch, mica::MIDIKeyNumber);
            if(not undefined(NoteNumber) and not Duration.IsEmpty() and
              Duration > 0)
            {
              count Volume = 60;
              count Channel = (Part % 16) + 1;
              Volume += count((Log2(Duration.To<number>()) + 4.f) * 15.f);
              if(IsBeginningBeamGroup) Volume += 15;
              Ratio ArticulatedDuration = Duration;
              if(WithExpression)
              {
                if(Parts == 2 and Part == 0)
                  Volume += 15;
                if(Parts == 2 and Part == 1)
                  ArticulatedDuration *= Ratio(8, 8), Volume -= 20;
              }
              Volume = Clip(Volume, count(1), count(127));
              Ratio ActualOnset = Onset;
              if(ActualOnset.IsEmpty() or ActualOnset < 0)
                ActualOnset = Ratio(0);
              if(Moments[Instant].IsNil())
                Moments[Instant]["Onset"] = ActualOnset;
              if(not Moments[Instant]["Duration"].IsRatio())
                Moments[Instant]["Duration"] = ArticulatedDuration;
              else if(Moments[Instant]["Duration"].AsRatio() >
                ArticulatedDuration)
                  Moments[Instant]["Duration"] = ArticulatedDuration;
              Moments[Instant]["Notes"].Add() = NoteNumber;
              Moments[Instant]["Islands"][NodeMatrix[Instant][Part]] = true;
              f.Tracks[Part].Note(ActualOnset, ArticulatedDuration,
                count(numerator(NoteNumber)), Volume, Channel);
              if(ActualOnset + Duration > EndTime)
                EndTime = ActualOnset + Duration;
            }
          }
        }
      }
    }
  }
  Offset = EndTime - Ratio(1, 32);
  return Moments;
}

Array<byte> MIDIGenerateFromSystem(const Music& M, Ratio& Offset, Ratio Tempo,
  Value MIDIInstrumentsForEachStaff)
{
  MIDI::File f;
  MIDIGenerateFromSystem(f, M, Offset, Tempo, MIDIInstrumentsForEachStaff);
  Array<byte> Output;
  f.Write(Output);
  return Output;
}

Array<byte> MIDIGenerateFromSystem(const Music& M, Ratio& Offset, Ratio Tempo,
  Value MIDIInstrumentsForEachStaff, bool WithExpression)
{
  MIDI::File f;
  MIDIGenerateFromSystem(f, M, Offset, Tempo, MIDIInstrumentsForEachStaff,
    WithExpression);
  Array<byte> Output;
  f.Write(Output);
  return Output;
}

Value MIDIInstruments()
{
  Value v;
  v[0] = "Piano";
  v[1] = "Bright Acoustic Piano";
  v[2] = "Electric Grand Piano";
  v[3] = "Honky-tonk Piano";
  v[4] = "Electric Piano 1";
  v[5] = "Electric Piano 2";
  v[6] = "Harpsichord";
  v[7] = "Clavinet";
  v[8] = "Celesta";
  v[9] = "Glockenspiel";
  v[10] = "Music Box";
  v[11] = "Vibraphone";
  v[12] = "Marimba";
  v[13] = "Xylophone";
  v[14] = "Tubular Bells";
  v[15] = "Dulcimer";
  v[16] = "Drawbar Organ";
  v[17] = "Percussive Organ";
  v[18] = "Rock Organ";
  v[19] = "Church Organ";
  v[20] = "Reed Organ";
  v[21] = "Accordion";
  v[22] = "Harmonica";
  v[23] = "Tango Accordion";
  v[24] = "Acoustic Guitar (nylon)";
  v[25] = "Acoustic Guitar (steel)";
  v[26] = "Electric Guitar (jazz)";
  v[27] = "Electric Guitar (clean)";
  v[28] = "Electric Guitar (muted)";
  v[29] = "Overdriven Guitar";
  v[30] = "Distortion Guitar";
  v[31] = "Guitar Harmonics";
  v[32] = "Acoustic Bass";
  v[33] = "Electric Bass (finger)";
  v[34] = "Electric Bass (pick)";
  v[35] = "Fretless Bass";
  v[36] = "Slap Bass 1";
  v[37] = "Slap Bass 2";
  v[38] = "Synth Bass 1";
  v[39] = "Synth Bass 2";
  v[40] = "Violin";
  v[41] = "Viola";
  v[42] = "Cello";
  v[43] = "Contrabass";
  v[44] = "Tremolo Strings";
  v[45] = "Pizzicato Strings";
  v[46] = "Orchestral Harp";
  v[47] = "Timpani";
  v[48] = "String Ensemble 1";
  v[49] = "String Ensemble 2";
  v[50] = "Synth Strings 1";
  v[51] = "Synth Strings 2";
  v[52] = "Choir Aahs";
  v[53] = "Voice Oohs";
  v[54] = "Synth Choir";
  v[55] = "Orchestra Hit";
  v[56] = "Trumpet";
  v[57] = "Trombone";
  v[58] = "Tuba";
  v[59] = "Muted Trumpet";
  v[60] = "French Horn";
  v[61] = "Brass Section";
  v[62] = "Synth Brass 1";
  v[63] = "Synth Brass 2";
  v[64] = "Soprano Sax";
  v[65] = "Alto Sax";
  v[66] = "Tenor Sax";
  v[67] = "Baritone Sax";
  v[68] = "Oboe";
  v[69] = "English Horn";
  v[70] = "Bassoon";
  v[71] = "Clarinet";
  v[72] = "Piccolo";
  v[73] = "Flute";
  v[74] = "Recorder";
  v[75] = "Pan Flute";
  v[76] = "Blown bottle";
  v[77] = "Shakuhachi";
  v[78] = "Whistle";
  v[79] = "Ocarina";
  v[80] = "Lead 1 (square)";
  v[81] = "Lead 2 (sawtooth)";
  v[82] = "Lead 3 (calliope)";
  v[83] = "Lead 4 chiff";
  v[84] = "Lead 5 (charang)";
  v[85] = "Lead 6 (voice)";
  v[86] = "Lead 7 (fifths)";
  v[87] = "Lead 8 (bass + lead)";
  v[88] = "Pad 1 (new age)";
  v[89] = "Pad 2 (warm)";
  v[90] = "Pad 3 (polysynth)";
  v[91] = "Pad 4 (choir)";
  v[92] = "Pad 5 (bowed)";
  v[93] = "Pad 6 (metallic)";
  v[94] = "Pad 7 (halo)";
  v[95] = "Pad 8 (sweep)";
  v[96] = "FX 1 (rain)";
  v[97] = "FX 2 (soundtrack)";
  v[98] = "FX 3 (crystal)";
  v[99] = "FX 4 (atmosphere)";
  v[100] = "FX 5 (brightness)";
  v[101] = "FX 6 (goblins)";
  v[102] = "FX 7 (echoes)";
  v[103] = "FX 8 (sci-fi)";
  v[104] = "Sitar";
  v[105] = "Banjo";
  v[106] = "Shamisen";
  v[107] = "Koto";
  v[108] = "Kalimba";
  v[109] = "Bagpipe";
  v[110] = "Fiddle";
  v[111] = "Shanai";
  v[112] = "Tinkle Bell";
  v[113] = "Agogo";
  v[114] = "Steel Drums";
  v[115] = "Woodblock";
  v[116] = "Taiko Drum";
  v[117] = "Melodic Tom";
  v[118] = "Synth Drum";
  v[119] = "Reverse Cymbal";
  v[120] = "Guitar Fret Noise";
  v[121] = "Breath Noise";
  v[122] = "Seashore";
  v[123] = "Bird Tweet";
  v[124] = "Telephone Ring";
  v[125] = "Helicopter";
  v[126] = "Applause";
  v[127] = "Gunshot";
  return v;
}

bool MIDIIsValidAudioFormat(const String& Filename)
{
  return
    Filename.EndsWith(".aif") or
    Filename.EndsWith(".aiff") or
    Filename.EndsWith(".flac") or
    Filename.EndsWith(".mid") or
    Filename.EndsWith(".mp3") or
    Filename.EndsWith(".ogg") or
    Filename.EndsWith(".wav");
}

count MIDILookupInstrument(String x)
{
  x = MIDIShortName(x);
  Value v = MIDIInstruments();
  for(count i = 0; i < v.n(); i++)
    if(MIDIShortName(v[i].AsString()) == x)
      return i;
  for(count i = 0; i < v.n(); i++)
    if(MIDIShortName(v[i].AsString()).Contains(x))
      return i;
  return 0;
}

String MIDIShortName(String x)
{
  x = x.ToLower();
  x.Replace(" ", "");
  x.Replace("+", "");
  x.Replace("-", "");
  x.Replace("(", "");
  x.Replace(")", "");
  return x;
}

String MIDISoundFontResource()
{
  return "specify";
}

count MIDISoundFontResourceLength()
{
  return 0;
}

void MIDISynthesizeAudio(String MIDIFilename, String AudioFilename)
{
  Array<byte> MIDIFile;
  File::Read(MIDIFilename, MIDIFile);
  MIDISynthesizeAudio(MIDIFile, AudioFilename);
}

void MIDISynthesizeAudio(MIDI::File& MIDIFile, String AudioFilename)
{
  Array<byte> MIDIArray;
  MIDIFile.Write(MIDIArray);
  MIDISynthesizeAudio(MIDIArray, AudioFilename);
}

void MIDISynthesizeAudio(const Array<byte>& MIDI, String AudioFilename)
{
  if(not MIDIIsValidAudioFormat(AudioFilename))
  {
    C::Out() >> "Error: audio format is not supported for " << AudioFilename;
    return;
  }
  if(AudioFilename.EndsWith(".mid"))
  {
    File::Write(AudioFilename, MIDI);
    return;
  }
  MIDIDownloadSoundFontIfNecessary();
  String MIDIData(&MIDI.a(), MIDI.n());
  String Timidity = Shell::GetProcessOnPath("timidity");
  String SoX = Shell::GetProcessOnPath("sox");
  if(not Timidity or not SoX)
  {
    C::Error() >> "Error: MIDI synthesis requires Timidity++ and SoX" <<
      " (brew install timidity sox)";
    return;
  }
  C::Out() >> "Synthesizing MIDI with Timidity++ to " << AudioFilename << "...";
  String Out, Error;
  Shell::PipeInOut(Timidity, MIDIData, Out, Error,
    "-c", MIDIConfigForSoundFont(MIDIDefaultSoundFontLocation()),
    "-Oa", "-o", "-", "-");
  String SoXOut;
  Shell::PipeInOut(SoX, Out, SoXOut, Error, "-", "--norm", AudioFilename,
    "reverb", "-w", "30");
  if(File::Length(AudioFilename))
    C::Out() >> "Wrote " << AudioFilename;
  else
    C::Error() >> "Could not write " << AudioFilename;
}

#endif
