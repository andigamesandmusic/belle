int main() {
  // stub for now
}

#if 0
String f = "ChromaticPitches.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";
s >> "  <category>";
s >> "    <definition lang=\"en\" " <<
  "name=\"Chromatic Pitches\">Contains chromatic";
s >> "    pitches over a large practical range.</definition>";
s++;

//------------------------------------------------------------------------------

String Letters[7] = {"C", "D", "E", "F", "G", "A", "B"};
String Identifiers[7] = {"TripleFlat", "DoubleFlat", "Flat", "",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Definitions[7] = {" triple-flat", " double-flat", " flat", "",
  " sharp", " double-sharp", " triple-sharp"};

count OctaveMin = -1;
count OctaveMax = 10;
String seq;
for(count o = OctaveMin; o <= OctaveMax; o++)
{
  for(count l = 0; l < 7; l++)
  {
    for(count a = 0; a < 7; a++)
    {
      String x = Definitions[a];
      String oid;
      oid << o;
      oid.Replace("-", "_");
      x.Replace("-", " ");
      if(a < 3) x.Prepend(" lowered by a");
      else if(a > 3) x.Prepend(" raised by a");
      s >> "    <concept uuid=\"en:" << Letters[l] << Identifiers[a] << oid
        << "\">";
      s >> "      <identifier lang=\"en\">" << Letters[l] << Identifiers[a] <<
        oid << "</identifier>";
      s >> "      <definition lang=\"en\" name=\"" << Letters[l] << o <<
        Definitions[a] << "\">Represents the pitch"
        >> "        " << Letters[l] << o <<
        x << ".</definition>"
        >> "    </concept>";
      seq << " en:" << Letters[l] << Identifiers[a] << oid;
      s++;
    }
  }
}

seq.Trim();
s >> "  <concept uuid=\"en:Pitches\">";
s >> "    <identifier lang=\"en\">Pitches</identifier>";
s >> "    <definition lang=\"en\" name=\"Pitches\">Ordered sequence of all the";
s >> "      defined chromatic pitches.</definition>";
s >> "    <sequence>" << seq << "</sequence>";
s >> "  </concept>";

//------------------------------------------------------------------------------

s >> "  </category>" >> "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif

#if 0
String f = "Intervals.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";
s >> "  <category>";
s >> "  <definition lang=\"en\" name=\"Intervals\">" <<
  "Contains intervals.</definition>";

//------------------------------------------------------------------------------

String Distance[8] = {"Unison", "Second", "Third", "Fourth", "Fifth", "Sixth",
  "Seventh", "Octave"};
String DistanceDef[8] = {"unison", "second", "third", "fourth", "fifth",
  "sixth", "seventh", "octave"};

String Quality[7] = {"DoublyDiminished", "Diminished", "Minor",
  "Perfect", "Major", "Augmented", "DoublyAugmented"};
String QualityDefName[7] = {"Doubly-Diminished", "Diminished", "Minor",
  "Perfect", "Major", "Augmented", "Doubly-Augmented"};
String QualityDef[7] = {"a doubly-diminished", "a diminished", "a minor",
  "a perfect", "a major", "an augmented", "a doubly-augmented"};

for(count d = 0; d < 8; d++)
{
  for(count q = 0; q < 7; q++)
  {
    if(d == 0 && q <= 2) continue;
    if(d == 0 || d == 3 || d == 4 || d == 7)
    {
      if(q == 2 || q == 4) continue;
    }
    else
    {
      if(q == 3) continue;
    }
    String qd = Quality[q] + Distance[d];
    String q_d = QualityDefName[q] + " " + Distance[d];

    s >> "  <concept uuid=\"en:" << qd << "\">";
    s >> "    <identifier lang=\"en\">" << qd << "</identifier>";
    s >> "    <definition lang=\"en\" name=\"" << q_d << "\">Represents the";
    s >> "      interval of " << QualityDef[q] << " " << DistanceDef[d] <<
      ".</definition>";
    s >> "  </concept>";
  }
}
s >> "  </category>";
s++;

for(count d = 0; d < 8; d++)
{
  for(count q = 0; q < 7; q++)
  {
    if(d == 0 && q <= 2) continue;
    if(d == 0 || d == 3 || d == 4 || d == 7)
    {
      if(q == 2 || q == 4) continue;
    }
    else
    {
      if(q == 3) continue;
    }
    String qd = Quality[q] + Distance[d];
    s >> "  <map>en:" << Quality[q] << " en:" << Distance[d] << " en:" <<
      qd << "</map>";
    s >> "  <map>en:" << qd << " en:Distance en:" << Distance[d] << "</map>";
    s >> "  <map>en:" << qd << " en:Quality en:" << Quality[q] << "</map>";
    s++;
  }
}

//------------------------------------------------------------------------------

s >> "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif

#if 0
String f = "JustIntervals.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";
s >> "<category>";
s >> "  <definition lang=\"en\" name=\"Just Intervals\">";
s >> " Common just intonation intervals " <<
  "[http://www.huygens-fokker.org/docs/intervals.html " <<
  "compiled by Manuel Op de Coul].";
s >> "  </definition>";
s++;

//------------------------------------------------------------------------------

struct Entry
{
  String Identifier;
  String Name;
  String Interval;
  String Definition;
};

Entry e[]={

{"JustUnison", "Just Unison", "1/1", ""},
{"JustOctave", "Just Octave", "2/1", ""},
{"JustFifth", "Just Fifth", "3/2", ""},
{"JustFourth", "Just Fourth", "4/3", ""},
{"JustMajorSixth", "Just Major Sixth", "5/3", "the BP sixth"},
{"JustMajorThird", "Just Major Third", "5/4", ""},
{"JustMinorThird", "Just Minor Third", "6/5", ""},
{"MinimalTenth", "Minimal Tenth", "7/3", "the BP tenth"},
{"HarmonicSeventh", "Harmonic Seventh", "7/4", ""},
{"SeptimalTritone", "Septimal Tritone", "7/5",
  "Huygens' tritone or the BP fourth"},
{"SeptimalMinorThird", "Septimal Minor Third", "7/6", ""},
{"JustMinorSixth", "Just Minor Sixth", "8/5", ""},
{"SeptimalWholeTone", "Septimal Whole Tone", "8/7", ""},
{"JustMajorNinth", "Just Major Ninth", "9/4", ""},
{"JustMinorSeventh", "Just Minor Seventh", "9/5", "the BP seventh"},
{"SeptimalMajorThird", "Septimal Major Third", "9/7", "the BP third"},
{"MajorWholeTone", "Major Whole Tone", "9/8", ""},
{"EulersTritone", "Euler's Tritone", "10/7", ""},
{"MinorWholeTone", "Minor Whole Tone", "10/9", ""},
{"NeutralNinth", "Neutral Ninth", "11/5", ""},
{"UndecimalNeutralSeventh", "Undecimal Neutral Seventh", "11/6", "a 21/4-tone"},
{"UndecimalAugmentedFifth", "Undecimal Augmented Fifth", "11/7", ""},
{"UndecimalSemiAugmentedFourth", "Undecimal Semi-Augmented Fourth", "11/8", ""},
{"UndecimalNeutralThird", "Undecimal Neutral Third", "11/9", ""},
{"PtolemysSecond", "Ptolemy's Second", "11/10", "a 4/5-tone"},
{"SeptimalMajorSixth", "Septimal Major Sixth", "12/7", ""},
{"UndecimalNeutralSecond", "Undecimal Neutral Second", "12/11", "a 3/4-tone"},
{"SixteenThirdsTone", "Sixteen Thirds Tone", "13/7", ""},
{"TridecimalNeutralSixth", "Tridecimal Neutral Sixth", "13/8", ""},
{"TridecimalDiminishedFifth", "Tridecimal Diminished Fifth", "13/9", ""},
{"TridecimalSemiDiminishedFourth", "Tridecimal Semi-Diminished Fourth",
  "13/10", ""},
{"TridecimalMinorThird", "Tridecimal Minor Third", "13/11", ""},
{"TridecimalTwoThirdsTone", "Tridecimal Two-Thirds Tone", "13/12", ""},
{"SeptimalMinorSixth", "Septimal Minor Sixth", "14/9", ""},
{"UndecimalMajorThird", "Undecimal Major Third", "14/11",
  "the undecimal diminished fourth"},
{"TwoThirdsTone", "Two-Thirds Tone", "14/13", ""},
{"SeptimalMinorNinth", "Septimal Minor Ninth", "15/7", "the BP ninth"},
{"ClassicMajorSeventh", "Classic Major Seventh", "15/8", ""},
{"UndecimalAugmentedFourth", "Undecimal Augmented Fourth", "15/11", ""},
{"TridecimalFiveFourthsTone", "Tridecimal Five-Fourths Tone", "15/13", ""},
{"MajorDiatonicSemitone", "Major Diatonic Semitone", "15/14", ""},
{"SeptimalMajorNinth", "Septimal Major Ninth", "16/7", ""},
{"PythagoreanMinorSeventh", "Pythagorean Minor Seventh", "16/9", ""},
{"UndecimalSemiDiminishedFifth", "Undecimal Semi-Diminished Fifth",
  "16/11", ""},
{"TridecimalNeutralThird", "Tridecimal Neutral Third", "16/13", ""},
{"MinorDiatonicSemitone", "Minor Diatonic Semitone", "16/15", ""},
{"SeptendecimalMinorNinth", "Septendecimal Minor Ninth", "17/8", ""},
{"SeptendecimalMajorSeventh", "Septendecimal Major Seventh", "17/9", ""},
{"SeptendecimalDiminishedSeventh", "Septendecimal Diminished Seventh",
  "17/10", ""},
{"SecondSeptendecimalTritone", "Second Septendecimal Tritone", "17/12", ""},
{"SupraminorThird", "Supraminor Third", "17/14", ""},
{"SeventeenthHarmonic", "Seventeenth Harmonic", "17/16", ""},
{"UndecimalNeutralSixth", "Undecimal Neutral Sixth", "18/11", ""},
{"TridecimalAugmentedFourth", "Tridecimal Augmented Fourth", "18/13", ""},
{"ArabicLuteIndexFinger", "Arabic Lute Index Finger", "18/17", ""},
{"UndevicesimalMajorSeventh", "Undevicesimal Major Seventh", "19/10", ""},
{"UndevicesimalMinorSixth", "Undevicesimal Minor Sixth", "19/12", ""},
{"UndevicesimalDitone", "Undevicesimal Ditone", "19/15", ""},
{"NineteenthHarmonic", "Nineteenth Harmonic", "19/16", ""},
{"QuasiMeantone", "Quasi-Meantone", "19/17", ""},
{"UndevicesimalSemitone", "Undevicesimal Semitone", "19/18", ""},
{"SmallNinth", "Small Ninth", "20/9", ""},
{"LargeMinorSeventh", "Large Minor Seventh", "20/11", ""},
{"TridecimalSemiAugmentedFifth", "Tridecimal Semi-Augmented Fifth",
  "20/13", ""},
{"SeptendecimalAugmentedSecond", "Septendecimal Augmented Second", "20/17", ""},
{"SmallUndevicesimalSemitone", "Small Undevicesimal Semitone", "20/19", ""},
{"UndecimalMajorSeventh", "Undecimal Major Seventh", "21/11", ""},
{"NarrowFourth", "Narrow Fourth", "21/16", ""},
{"SubmajorThird", "Submajor Third", "21/17", ""},
{"MinorSemitone", "Minor Semitone", "21/20", ""},
{"TridecimalMajorSixth", "Tridecimal Major Sixth", "22/13", ""},
{"UndecimalDiminishedFifth", "Undecimal Diminished Fifth", "22/15", ""},
{"UndecimalMinorSemitone", "Undecimal Minor Semitone", "22/21", ""},
{"VicesimotertialMajorSeventh", "Vicesimotertial Major Seventh", "23/12", ""},
{"TwentyThirdHarmonic", "Twenty-Third Harmonic", "23/16", ""},
{"VicesimotertialMajorThird", "Vicesimotertial Major Third", "23/18", ""},
{"TridecimalNeutralSeventh", "Tridecimal Neutral Seventh", "24/13", ""},
{"FirstSeptendecimalTritone", "First Septendecimal Tritone", "24/17", ""},
{"SmallerUndevicesimalMajorThird", "Smaller Undevicesimal Major Third",
  "24/19", ""},
{"VicesimotertialMinorSemitone", "Vicesimotertial Minor Semitone", "24/23", ""},
{"ClassicAugmentedEleventh", "Classic Augmented Eleventh", "25/9",
  "the BP twelfth"},
{"ClassicAugmentedOctave", "Classic Augmented Octave", "25/12", ""},
{"MiddleMinorSeventh", "Middle Minor Seventh", "25/14", ""},
{"ClassicAugmentedFifth", "Classic Augmented Fifth", "25/16", ""},
{"ClassicAugmentedFourth", "Classic Augmented Fourth", "25/18", ""},
{"QuasiTemperedMinorThird", "Quasi-Tempered Minor Third", "25/21",
  "the BP second"},
{"UndecimalAcuteWholeTone", "Undecimal Acute Whole Tone", "25/22", ""},
{"ClassicChromaticSemitone", "Classic Chromatic Semitone", "25/24",
  "the minor chroma"},
{"TridecimalSemiAugmentedSixth", "Tridecimal Semi-Augmented Sixth",
  "26/15", ""},
{"TridecimalThirdTone", "Tridecimal Third Tone", "26/25", ""},
{"SeptimalMajorSeventh", "Septimal Major Seventh", "27/14", ""},
{"PythagoreanMajorSixth", "Pythagorean Major Sixth", "27/16", ""},
{"SeptendecimalMinorSixth", "Septendecimal Minor Sixth", "27/17", ""},
{"AcuteFourth", "Acute Fourth", "27/20", ""},
{"NeutralThird", "Neutral Third", "27/22", "the Zalzal Wosta of Al-Farabi"},
{"VicesimotertialMinorThird", "Vicesimotertial Minor Third", "27/23", ""},
{"LargeLimma", "Large Limma", "27/25", "the BP small semitone"},
{"TridecimalComma", "Tridecimal Comma", "27/26", ""},
{"GraveMajorSeventhAlternate", "Grave Major Seventh Alternate", "28/15", ""},
{"SubmajorSixth", "Submajor Sixth", "28/17", ""},
{"MiddleSecond", "Middle Second", "28/25", ""},
{"ArchytasThirdTone", "Archytas' Third Tone", "28/27", ""},
{"TwentyNinthHarmonic", "Twenty-Ninth Harmonic", "29/16", ""},
{"SmallerUndevicesimalMinorSixth", "Smaller Undevicesimal Minor Sixth",
  "30/19", ""},
{"ThirtyFirstHarmonic", "Thirty-First Harmonic", "31/16", ""},
{"ThirtyFirstPartialChroma", "Thirty-First Partial Chroma", "31/30", ""},
{"MinorNinth", "Minor Ninth", "32/15", ""},
{"SeventeenthSubharmonic", "Seventeenth Subharmonic", "32/17", ""},
{"NineteenthSubharmonic", "Nineteenth Subharmonic", "32/19", ""},
{"WideFifth", "Wide Fifth", "32/21", ""},
{"TwentyThirdSubharmonic", "Twenty-Third Subharmonic", "32/23", ""},
{"ClassicDiminishedFourth", "Classic Diminished Fourth", "32/25", ""},
{"PythagoreanMinorThird", "Pythagorean Minor Third", "32/27", ""},
{"TwentyNinthSubharmonic", "Twenty-Ninth Subharmonic", "32/29", ""},
{"GreekEnharmonicQuartertone", "Greek Enharmonic Quartertone", "32/31", ""},
{"TwoPentatones", "Two Pentatones", "33/25", ""},
{"TridecimalMajorThird", "Tridecimal Major Third", "33/26", ""},
{"UndecimalMinorThird", "Undecimal Minor Third", "33/28", ""},
{"UndecimalComma", "Undecimal Comma", "33/32", "Al-Farabi's quartertone"},
{"SupraminorSixth", "Supraminor Sixth", "34/21", ""},
{"SeptendecimalMajorThird", "Septendecimal Major Third", "34/27", ""},
{"SeptimalSemiDiminishedOctave", "Septimal Semi-Diminished Octave", "35/18",
  ""},
{"SeptimalSemiDiminishedFifth", "Septimal Semi-Diminished Fifth", "35/24", ""},
{"SeptimalSemiDiminishedFourth", "Septimal Semi-Diminished Fourth", "35/27",
  "the 9/4-tone"},
{"SeptimalNeutralSecond", "Septimal Neutral Second", "35/32", ""},
{"SeptendecimalQuartertone", "Septendecimal Quartertone", "35/34", ""},
{"SmallerUndevicesimalMajorSeventh", "Smaller Undevicesimal Major Seventh",
  "36/19", ""},
{"ClassicDiminishedFifth", "Classic Diminished Fifth", "36/25", ""},
{"SeptimalDiesis", "Septimal Diesis", "36/35", "a quartertone"},
{"ThirtySeventhHarmonic", "Thirty-Seventh Harmonic", "37/32", ""},
{"ThirtyNinthHarmonic", "Thirty-Ninth Harmonic", "39/32",
  "the Zalzal Wosta of Ibn Sina"},
{"AcuteMajorSeventh", "Acute Major Seventh", "40/21", ""},
{"GraveFifth", "Grave Fifth", "40/27", ""},
{"TridecimalMinorDiesis", "Tridecimal Minor Diesis", "40/39", ""},
{"QuasiTemperedMajorSixth", "Quasi-Tempered Major Sixth", "42/25", ""},
{"UndecimalGraveMinorSeventh", "Undecimal Grave Minor Seventh", "44/25", ""},
{"NeutralSixth", "Neutral Sixth", "44/27", ""},
{"DiatonicTritone", "Diatonic Tritone", "45/32", ""},
{"FifthTone", "Fifth-Tone", "45/44", ""},
{"TwentyThirdPartialChroma", "Twenty-Third Partial Chroma", "46/45", ""},
{"ClassicDiminishedOctave", "Classic Diminished Octave", "48/25", ""},
{"SeptimalSemiAugmentedFourth", "Septimal Semi-Augmented Fourth", "48/35", ""},
{"BPEighth", "BP Eighth", "49/25", ""},
{"LargeApproximationNeutralSixth", "Large Approximation Neutral Sixth",
  "49/30", ""},
{"ArabicLuteAcuteFourth", "Arabic Lute Acute Fourth", "49/36", ""},
{"LargeApproximationNeutralThird", "Large Approximation Neutral Third",
  "49/40", ""},
{"BPMinorSemitone", "BP Minor Semitone", "49/45", ""},
{"SeptimalSixthTone", "Septimal Sixth-Tone", "49/48", "the slendro diesis "},
{"GraveMajorSeventh", "Grave Major Seventh", "50/27", ""},
{"ThreePentatones", "Three Pentatones", "50/33", ""},
{"ErlichsDecatonicComma", "Erlich's Decatonic Comma", "50/49",
  "the tritonic diesis"},
{"SeventeenthPartialChroma", "Seventeenth-Partial Chroma", "51/50", ""},
{"TridecimalMinorSixth", "Tridecimal Minor Sixth", "52/33", ""},
{"SeptimalSemiAugmentedFifth", "Septimal Semi-Augmented Fifth", "54/35", ""},
{"ZalzalsMujannab", "Zalzal's Mujannab", "54/49", ""},
{"UndecimalSemiAugmentedFifth", "Undecimal Semi-Augmented Fifth", "55/36", ""},
{"UndecimalSemiAugmentedWholeTone", "Undecimal Semi-Augmented Whole Tone",
  "55/48", ""},
{"QuasiEqualMajorSecond", "Quasi-Equal Major Second", "55/49", ""},
{"SmallerApproximationNeutralThird", "Smaller Approximation Neutral Third",
  "60/49", ""},
{"QuasiEqualMajorTenth", "Quasi-Equal Major Tenth", "63/25", "the BP eleventh"},
{"OctaveMinusSeptimalComma", "Octave Minus Septimal Comma", "63/32", ""},
{"NarrowMinorSixth", "Narrow Minor Sixth", "63/40", ""},
{"QuasiEqualMajorThird", "Quasi-Equal Major Third", "63/50", ""},
{"ThirtyThirdSubharmonic", "Thirty-Third Subharmonic", "64/33", ""},
{"SeptimalNeutralSeventh", "Septimal Neutral Seventh", "64/35", ""},
{"ThirtySeventhSubharmonic", "Thirty-Seventh Subharmonic", "64/37", ""},
{"ThirtyNinthSubharmonic", "Thirty-Ninth Subharmonic", "64/39", ""},
{"SecondTritone", "Second Tritone", "64/45", ""},
{"SeptatonicMajorThird", "Septatonic Major Third", "64/49", "two septatones "},
{"SeptimalComma", "Septimal Comma", "64/63", "Archytas' comma"},
{"ThirteenthPartialChroma", "Thirteenth-Partial Chroma", "65/64", ""},
{"TwentyThreeFourthsTone", "Twenty-Three Fourths Tone", "68/35", ""},
{"ArabicLuteGraveFifth", "Arabic Lute Grave Fifth", "72/49", ""},
{"UndecimalSemiDiminishedFourth", "Undecimal Semi-Diminished Fourth",
  "72/55", ""},
{"IbnSinasNeutralThird", "Ibn Sina's Neutral Third", "72/59", ""},
{"BPFifth", "BP Fifth", "75/49", ""},
{"ClassicAugmentedSecond", "Classic Augmented Second", "75/64", ""},
{"FiftyThreeToneCommaApproximation", "Fifty-Three Tone Comma Approximation",
  "77/76", ""},
{"SmallApproximationNeutralSixth", "Small Approximation Neutral Sixth",
  "80/49", ""},
{"WideMajorThird", "Wide Major Third", "80/63", ""},
{"SecondUndecimalNeutralSeventh", "Second Undecimal Neutral Seventh",
  "81/44", ""},
{"AcuteMinorSixth", "Acute Minor Sixth", "81/50", ""},
{"PythagoreanMajorThird", "Pythagorean Major Third", "81/64", ""},
{"PersianWosta", "Persian Wosta", "81/68", ""},
{"AlHwarizmisLuteMiddleFinger", "Al-Hwarizmi's Lute Middle Finger",
  "81/70", ""},
{"SyntonicComma", "Syntonic Comma", "81/80", "the Didymus comma"},
{"SecondUndecimalNeutralSecond", "Second Undecimal Neutral Second",
  "88/81", ""},
{"QuasiEqualSemitone", "Quasi-Equal Semitone", "89/84", ""},
{"FifteenFourthsTone", "Fifteen-Fourths Tone", "91/59", ""},
{"MediumTridecimalComma", "Medium Tridecimal Comma", "91/90", ""},
{"NineteenthPartialChroma", "Nineteenth-Partial Chroma", "96/95", ""},
{"QuasiEqualMinorSeventh", "Quasi-Equal Minor Seventh", "98/55", ""},
{"SecondQuasiEqualTritone", "Second Quasi-Equal Tritone", "99/70", ""},
{"SmallUndecimalComma", "Small Undecimal Comma", "99/98", ""},
{"QuasiEqualMinorSixth", "Quasi-Equal Minor Sixth", "100/63", ""},
{"GraveMajorThird", "Grave Major Third", "100/81", ""},
{"PtolemysComma", "Ptolemy's Comma", "100/99", ""},
{"SeptimalNeutralSixth", "Septimal Neutral Sixth", "105/64", ""},
{"SmallTridecimalComma", "Small Tridecimal Comma", "105/104", ""},
{"UndecimalSecondsComma", "Undecimal Seconds Comma", "121/120", ""},
{"ClassicAugmentedSeventh", "Classic Augmented Seventh", "125/64",
  "an octave minus a minor diesis"},
{"ClassicAugmentedSixth", "Classic Augmented Sixth", "125/72", ""},
{"ClassicAugmentedThird", "Classic Augmented Third", "125/96", ""},
{"SemiAugmentedWholeTone", "Semi-Augmented Whole Tone", "125/108", ""},
{"ClassicAugmentedSemitone", "Classic Augmented Semitone", "125/112", ""},
{"SmallSeptimalComma", "Small Septimal Comma", "126/125", ""},
{"JustDiminishedSeventh", "Just Diminished Seventh", "128/75", ""},
{"PythagoreanMinorSixth", "Pythagorean Minor Sixth", "128/81", ""},
{"SeptimalNeutralThird", "Septimal Neutral Third", "128/105", ""},
{"UndecimalSemitone", "Undecimal Semitone", "128/121", ""},
{"MinorDiesis", "Minor Diesis", "128/125", "the diesis"},
{"ThirteenFourthsTone", "Thirteen-Fourths Tone", "131/90", ""},
{"MajorChroma", "Major Chroma", "135/128", "the major limma"},
{"QuasiEqualTritone", "Quasi-Equal Tritone", "140/99", ""},
{"ClassicDiminishedThird", "Classic Diminished Third", "144/125", ""},
{"TwentyNinthPartialChroma", "Twenty-Ninth Partial Chroma", "145/144", ""},
{"SevenFourthsTone", "Seven-Fourths Tone", "153/125", ""},
{"OctaveMinusSyntonicComma", "Octave Minus Syntonic Comma", "160/81", ""},
{"NineteenFourthsTone", "Nineteen-Fourths Tone", "161/93", ""},
{"PersianNeutralSecond", "Persian Neutral Second", "162/149", ""},
{"QuasiEqualMajorSeventh", "Quasi-Equal Major Seventh", "168/89", ""},
{"Valinorsma", "Valinorsma", "176/175", ""},
{"ClassicDiminishedSixth", "Classic Diminished Sixth", "192/125", ""},
{"SemiAugmentedSixth", "Semi-Augmented Sixth", "216/125", ""},
{"JustAugmentedSixth", "Just Augmented Sixth", "225/128", ""},
{"SeptimalKleisma", "Septimal Kleisma", "225/224", ""},
{"FiveFourthsTone", "Five-Fourths Tone", "231/200", ""},
{"MeshaqahsThreeFourthsTone", "Meshaqah's Three-Fourths Tone", "241/221", ""},
{"OctaveMinusMaximalDiesis", "Octave Minus Maximal Diesis", "243/125", ""},
{"PythagoreanMajorSeventh", "Pythagorean Major Seventh", "243/128", ""},
{"AcuteFifth", "Acute Fifth", "243/160", ""},
{"AcuteMinorThird", "Acute Minor Third", "243/200", ""},
{"NeutralThirdComma", "Neutral Third Comma", "243/242", ""},
{"MinorBPDiesis", "Minor BP Diesis", "245/243", ""},
{"MeshaqahsQuartertone", "Meshaqah's Quartertone", "246/239", ""},
{"TricesoprimalComma", "Tricesoprimal Comma", "248/243", ""},
{"SeventeenFourthsTone", "Seventeen-Fourths Tone", "250/153", ""},
{"MaximalDiesis", "Maximal Diesis", "250/243", ""},
{"OctaveMinusMajorChroma", "Octave Minus Major Chroma", "256/135", ""},
{"JustDiminishedThird", "Just Diminished Third", "256/225", ""},
{"PythagoreanMinorSecond", "Pythagorean Minor Second", "256/243", "the limma"},
{"SeptimalMinorSemitone", "Septimal Minor Semitone", "256/245", ""},
{"SeptendecimalKleisma", "Septendecimal Kleisma", "256/255", ""},
{"VicesimononalComma", "Vicesimononal Comma", "261/256", ""},
{"KirnbergersSixth", "Kirnberger's Sixth", "270/161", ""},
{"PersianWholeTone", "Persian Whole Tone", "272/243", ""},
{"IbnSinasMinorSecond", "Ibn Sina's Minor Second", "273/256", ""},
{"GraveFourth", "Grave Fourth", "320/243", ""},
{"Minthma", "Minthma", "352/351", ""},
{"JustDoublyAugmentedFourth", "Just Doubly Augmented Fourth", "375/256", ""},
{"BPMajorSemitone", "BP Major Semitone", "375/343", "the minor BP chroma"},
{"UndecimalKleisma", "Undecimal Kleisma", "385/384", ""},
{"GraveMajorSixth", "Grave Major Sixth", "400/243", ""},
{"WideAugmentedFifth", "Wide Augmented Fifth", "405/256", ""},
{"WerckmeistersUndecimalSeptenarianSchisma",
  "Werckmeister's Undecimal Septenarian Schisma", "441/440", ""},
{"SeptatonicFifth", "Septatonic Fifth", "512/343", "three septatones"},
{"JustDoublyDiminishedFifth", "Just Doubly Diminished Fifth", "512/375", ""},
{"NarrowDiminishedFourth", "Narrow Diminished Fourth", "512/405", ""},
{"UndevicesimalComma", "Undevicesimal Comma", "513/512", "the Boethius' comma"},
{"AvicennaEnharmonicDiesis", "Avicenna Enharmonic Diesis", "525/512", ""},
{"SwetsComma", "Swets' Comma", "540/539", ""},
{"OctaveMinusMajorDiesis", "Octave Minus Major Diesis", "625/324", ""},
{"BPGreatSemitone", "BP Great Semitone", "625/567", "the major BP chroma"},
{"Huntma", "Huntma", "640/637", ""},
{"MajorDiesis", "Major Diesis", "648/625", ""},
{"WideAugmentedThird", "Wide Augmented Third", "675/512", ""},
{"IslandComma", "Island Comma", "676/675", ""},
{"Senga", "Senga", "686/675", ""},
{"ElevenFourthsTone", "Eleven-Fourths Tone", "687/500", ""},
{"SeptendecimalBridgeComma", "Septendecimal Bridge Comma", "715/714", ""},
{"AcuteMinorSeventh", "Acute Minor Seventh", "729/400", ""},
{"PythagoreanTritone", "Pythagorean Tritone", "729/512", ""},
{"AcuteMajorSecond", "Acute Major Second", "729/640", ""},
{"UndecimalMajorDiesis", "Undecimal Major Diesis", "729/704", ""},
{"VicesimotertialComma", "Vicesimotertial Comma", "736/729", ""},
{"AncientChineseQuasiEqualFifth", "Ancient Chinese Quasi-Equal Fifth",
  "749/500", ""},
{"AncientChineseTempering", "Ancient Chinese Tempering", "750/749", ""},
{"GraveWholeTone", "Grave Whole Tone", "800/729", ""},
{"Keema", "Keema", "875/864", ""},
{"UndecimalSemicomma", "Undecimal Semicomma", "896/891", ""},
{"NarrowDiminishedSixth", "Narrow Diminished Sixth", "1024/675", ""},
{"PythagoreanDiminishedFifth", "Pythagorean Diminished Fifth", "1024/729", ""},
{"GamelanResidue", "Gamelan Residue", "1029/1024", ""},
{"TridecimalMajorDiesis", "Tridecimal Major Diesis", "1053/1024", ""},
{"DoublyAugmentedPrime", "Doubly Augmented Prime", "1125/1024", ""},
{"KestrelComma", "Kestrel Comma", "1188/1183", ""},
{"WideAugmentedSecond", "Wide Augmented Second", "1215/1024", ""},
{"EratosthenesComma", "Eratosthenes' Comma", "1216/1215", ""},
{"GraveMinorSeventh", "Grave Minor Seventh", "1280/729", ""},
{"Triaphonisma", "Triaphonisma", "1288/1287", ""},
{"Nicola", "Nicola", "1575/1573", ""},
{"OrwellComma", "Orwell Comma", "1728/1715", ""},
{"OneCentApproximation", "One Cent Approximation", "1732/1731", ""},
{"JustDoublyAugmentedSixth", "Just Doubly Augmented Sixth", "1875/1024", ""},
{"TwoTritones", "Two Tritones", "2025/1024", ""},
{"JustDoublyDiminishedOctave", "Just Doubly Diminished Octave", "2048/1125",
  ""},
{"NarrowDiminishedSeventh", "Narrow Diminished Seventh", "2048/1215", ""},
{"JustDoublyDiminishedThird", "Just Doubly Diminished Third", "2048/1875", ""},
{"Diaschisma", "Diaschisma", "2048/2025", ""},
{"Xenisma", "Xenisma", "2058/2057", ""},
{"AcuteMajorSixth", "Acute Major Sixth", "2187/1280", ""},
{"Apotome", "Apotome", "2187/2048", ""},
{"SeptendecimalComma", "Septendecimal Comma", "2187/2176", ""},
{"ParizekComma", "Parizek Comma", "2200/2197", ""},
{"Breedsma", "Breedsma", "2401/2400", ""},
{"NuwellComma", "Nuwell Comma", "2430/2401", ""},
{"GraveMinorThird", "Grave Minor Third", "2560/2187", ""},
{"Lehmerisma", "Lehmerisma", "3025/3024", ""},
{"SmallDiesis", "Small Diesis", "3125/3072", ""},
{"MajorBPDiesis", "Major BP Diesis", "3125/3087", ""},
{"MiddleSecondComma", "Middle Second Comma", "3136/3125", ""},
{"JustDoublyAugmentedFifth", "Just Doubly Augmented Fifth", "3375/2048", ""},
{"SeptimalSemicomma", "Septimal Semicomma", "4000/3969", ""},
{"UndecimalSchisma", "Undecimal Schisma", "4000/3993", ""},
{"PythagoreanDiminishedOctave", "Pythagorean Diminished Octave",
  "4096/2187", ""},
{"SeptatonicMajorSixth", "Septatonic Major Sixth", "4096/2401",
  "four septatones"},
{"JustDoublyDiminishedFourth", "Just Doubly Diminished Fourth",
  "4096/3375", ""},
{"TridecimalSchisma", "Tridecimal Schisma", "4096/4095",
  "the Sagittal schismina"},
{"Ragisma", "Ragisma", "4375/4374", ""},
{"ArabicNeutralSecond", "Arabic Neutral Second", "4608/4235", ""},
{"GaribaldiComma", "Garibaldi Comma", "5120/5103", "Beta 5"},
{"JustDoublyAugmentedThird", "Just Doubly Augmented Third", "5625/4096", ""},
{"OctaveMinusSmallDiesis", "Octave Minus Small Diesis", "6144/3125", ""},
{"PorwellComma", "Porwell Comma", "6144/6125", ""},
{"PythagoreanAugmentedFifth", "Pythagorean Augmented Fifth", "6561/4096", ""},
{"AcuteMajorThird", "Acute Major Third", "6561/5120", ""},
{"BPMajorLink", "BP Major Link", "6561/6125", ""},
{"MathieuSuperdiesis", "Mathieu Superdiesis", "6561/6400", ""},
{"JustDoublyDiminishedSixth", "Just Doubly Diminished Sixth", "8192/5625", ""},
{"PythagoreanDiminishedFourth", "Pythagorean Diminished Fourth",
  "8192/6561", ""},
{"UndecimalMinorDiesis", "Undecimal Minor Diesis", "8192/8019", ""},
{"Kalisma", "Kalisma", "9801/9800", "Gauss' comma"},
{"JustDoublyAugmentedSecond", "Just Doubly Augmented Second", "10125/8192", ""},
{"GraveMinorSixth", "Grave Minor Sixth", "10240/6561", ""},
{"Harmonisma", "Harmonisma", "10648/10647", ""},
{"FourthPlusSchisma", "Fourth Plus Schisma", "10935/8192",
  "the 5-limit approximation to ET perfect fourth"},
{"Hemimage", "Hemimage", "10976/10935", ""},
{"GreatBPDiesis", "Great BP Diesis", "15625/15309", ""},
{"Kleisma", "Kleisma", "15625/15552", "the semicomma majeur"},
{"JustDoublyDiminishedSeventh", "Just Doubly Diminished Seventh",
  "16384/10125", ""},
{"FifthMinusSchisma", "Fifth Minus Schisma", "16384/10935",
  "the 5-limit approximation to ET perfect fifth"},
{"DoubleAugmentationDiesis", "Double Augmentation Diesis", "16875/16384", ""},
{"SmallBPDiesis", "Small BP Diesis", "16875/16807", ""},
{"SeptimalMajorDiesis", "Septimal Major Diesis", "17496/16807", ""},
{"MinimalBPChroma", "Minimal BP Chroma", "18225/16807", ""},
{"GreaterHarmonisma", "Greater Harmonisma", "19657/19656", ""},
{"OctaveMinusMinimalDiesis", "Octave Minus Minimal Diesis", "19683/10000", ""},
{"AcuteMajorSeventhAlternate", "Acute Major Seventh Alternate",
  "19683/10240", ""},
{"PythagoreanAugmentedSecond", "Pythagorean Augmented Second",
  "19683/16384", ""},
{"MinimalDiesis", "Minimal Diesis", "20000/19683", ""},
{"GraveMinorSecond", "Grave Minor Second", "20480/19683", ""},
{"MaximalBPChroma", "Maximal BP Chroma", "21875/19683", ""},
{"LesserHarmonisma", "Lesser Harmonisma", "23232/23231", ""},
{"OctaveDoubleAugmentationDiesis", "Octave-Double Augmentation Diesis",
  "32768/16875", ""},
{"PythagoreanDiminishedSeventh", "Pythagorean Diminished Seventh",
  "32768/19683", ""},
{"SeptatonicDiminishedOctave", "Septatonic Diminished Octave",
  "32768/16807", "five septatones"},
{"Schisma", "Schisma", "32805/32768", ""},
{"MirwomoComma", "Mirwomo Comma", "33075/32768", ""},
{"Trimyna", "Trimyna", "50421/50000", ""},
{"MersennesQuasiEqualSemitone", "Mersenne's Quasi-Equal Semitone",
  "52973/50000", ""},
{"PythagoreanAugmentedSixth", "Pythagorean Augmented Sixth",
  "59049/32768", ""},
{"HarrisonsComma", "Harrison's Comma", "59049/57344", ""},
{"OctaveMinusSchisma", "Octave Minus Schisma", "65536/32805", ""},
{"PythagoreanDiminishedThird", "Pythagorean Diminished Third",
  "65536/59049", ""},
{"Orgonisma", "Orgonisma", "65536/65219", ""},
{"HorwellComma", "Horwell Comma", "65625/65536", ""},
{"MediumSemicomma", "Medium Semicomma", "78732/78125", ""},
{"BPMinorLink", "BP Minor Link", "83349/78125", ""},
{"Stearnsma", "Stearnsma", "118098/117649", ""},
{"PythagoreanAugmentedThird", "Pythagorean Augmented Third",
  "177147/131072", ""},
{"LandscapeComma", "Landscape Comma", "250047/250000", ""},
{"PythagoreanDiminishedSixth", "Pythagorean Diminished Sixth",
  "262144/177147", ""},
{"OctaveMinusWurschmidtsComma", "Octave Minus Würschmidt's Comma",
  "390625/196608", ""},
{"DimcompComma", "Dimcomp Comma", "390625/388962", ""},
{"WurschmidtsComma", "Würschmidt's Comma", "393216/390625", ""},
{"BPSmallLink", "BP Small Link", "413343/390625", ""},
{"PythagoreanAugmentedSeventh", "Pythagorean Augmented Seventh",
  "531441/262144", ""},
{"PythagoreanComma", "Pythagorean Comma", "531441/524288",
  "the ditonic comma"},
{"PythagoreanDiminishedNinth", "Pythagorean Diminished Ninth",
  "1048576/531441", ""},
{"PythagoreanDoublyAugmentedFourth", "Pythagorean Doubly Augmented Fourth",
  "1594323/1048576", ""},
{"UnicornComma", "Unicorn Comma", "1594323/1562500", ""},
{"KleismaMinusSchisma", "Kleisma Minus Schisma", "1600000/1594323", ""},
{"PythagoreanDoublyDiminishedFifth", "Pythagorean Doubly Diminished Fifth",
  "2097152/1594323", ""},
{"Semicomma", "Semicomma", "2109375/2097152", "Fokker's comma"},
{"PythagoreanDoublyAugmentedPrime", "Pythagorean Doubly Augmented Prime",
  "4782969/4194304", ""},
{"PythagoreanDoublyDiminishedOctave", "Pythagorean Doubly Diminished Octave",
  "8388608/4782969", ""},
{"PythagoreanDoublyAugmentedFifth", "Pythagorean Doubly Augmented Fifth",
  "14348907/8388608", ""},
{"PythagoreanDoublyDiminishedFourth", "Pythagorean Doubly Diminished Fourth",
  "16777216/14348907", ""},
{"SeptimalSchisma", "Septimal Schisma", "33554432/33480783", "Beta 2"},
{"AmpersandsComma", "Ampersand's Comma", "34171875/33554432", ""},
{"PythagoreanDoublyAugmentedSecond", "Pythagorean Doubly Augmented Second",
  "43046721/33554432", ""},
{"SycamoreComma", "Sycamore Comma", "48828125/47775744", ""},
{"PythagoreanDoublyDiminishedSeventh", "Pythagorean Doubly Diminished Seventh",
  "67108864/43046721", ""},
{"DiaschismaMinusSchisma", "Diaschisma Minus Schisma", "67108864/66430125",
  "the Misty comma"},
{"PythagoreanDoublyAugmentedSixth", "Pythagorean Doubly Augmented Sixth",
  "129140163/67108864", ""},
{"WholeToneMinusTwoSchismas", "Whole-Tone Minus Two Schismas",
  "134217728/119574225", "the 5-limit approximation to ET whole tone"},
{"PythagoreanDoublyDiminishedThird", "Pythagorean Doubly Diminished Third",
  "134217728/129140163", ""},
{"PythagoreanDoublyAugmentedThird", "Pythagorean Doubly Augmented Third",
  "387420489/268435456", ""},
{"PythagoreanDoublyDiminishedSixth", "Pythagorean Doubly Diminished Sixth",
  "536870912/387420489", ""},
{"PythagoreanDoublyAugmentedSeventh", "Pythagorean Doubly Augmented Seventh",
  "1162261467/536870912", ""},
{"PythagoreanNineteenComma", "Pythagorean Nineteen-Comma",
  "1162261467/1073741824", ""},
{"Parakleisma", "Parakleisma", "1224440064/1220703125", ""},
{"VishnuComma", "Vishnu Comma", "6115295232/6103515625", ""},
{"SemithirdsComma", "Semithirds Comma", "274877906944/274658203125", ""},
{"EnnealimmalComma", "Ennealimmal Comma", "7629394531250/7625597484987", ""},
{"NineteenToneComma", "Nineteen-Tone Comma", "19073486328125/19042491875328",
  ""},
{"Monzisma", "Monzisma", "450359962737049600/450283905890997363", ""},
{"FortyOneToneComma", "Forty-One Tone Comma",
  "36893488147419103232/36472996377170786403", ""},
{"MercatorsComma", "Mercator's Comma",
  "19383245667680019896796723/19342813113834066795298816", ""}

};
count n = sizeof(e) / sizeof(Entry);
String m;
Table<String, String> rm;
for(count i = 0; i < n; i++)
{
  c >> e[i].Interval;
  Ratio r = e[i].Interval;
  String Primes;
  if(r.IsEmpty())
  {
    if(e[i].Identifier == "FortyOneToneComma")
    {
      for(count j=0;j<65;j++) Primes << " 2";
      for(count j=0;j<41;j++) Primes << " 1/3";
    }
    else if(e[i].Identifier == "MercatorsComma")
    {
      for(count j=0;j<53;j++) Primes << " 3";
      for(count j=0;j<84;j++) Primes << " 1/2";
    }
  }
  else
  {
    rm[r] = e[i].Identifier;
    integer n = r.Numerator();
    if(n == 1)
      Primes << " 1";
    else while(n != 1)
    {
      for(count p = 2; p <= n; p++)
      {
        if(n % p == 0)
        {
          n /= p;
          Primes << " " << p;
          break;
        }
      }
    }
    integer d = r.Denominator();
    if(d == 1)
      Primes << " ";
    else while(d != 1)
    {
      for(count p = 2; p <= d; p++)
      {
        if(d % p == 0)
        {
          d /= p;
          Primes << " 1/" << p;
          break;
        }
      }
    }
    m >> "<map>en:" << e[i].Identifier << " en:Ratio " << r << "</map>";
    m >> "<map>" << r << " en:JustInterval en:" << e[i].Identifier << "</map>";
  }
  Primes.Trim();
  String rc = e[i].Interval;
  rc.Replace("/", ":");
  number cents = 0.0;
  if(rc != "1:1")
  {
    cents = Log2((number)r.Numerator() / (number)r.Denominator()) * 1200.0;
    if(Limits<number>::IsNaN(cents))
      cents = 0.0;
  }
  s >> "<concept uuid=\"en:" << e[i].Identifier << "\">";
  s >> "  <sequence>" << Primes << "</sequence>";
  s >> "  <identifier lang=\"en\">" << e[i].Identifier << "</identifier>";
  s >> "  <definition lang=\"en\" name=\"" << e[i].Name << "\">" <<
    "Interval with ratio " << rc;
  if(cents > 0)
    s << " (" << cents << " cents)";
  if(e[i].Definition)
    s << " also known as " << e[i].Definition;
  s << ".</definition>";
  s >> "</concept>";
}
c++;
count t = 0;
for(count i = 0; i < rm.n(); i++)
{
  for(count j = i; j < rm.n(); j++)
  {
    Ratio r1 = rm.ith(i).Key;
    Ratio r2 = rm.ith(j).Key;
    if(r1 > r2)
      Swap(r1, r2);

    {
      Ratio r = r1 * r2;
      if(r.IsEmpty()) continue;
      while(r.Numerator() >= r.Denominator() * 2 &&
        r.Denominator() < 1000000000 * 1000000000) r /= 2;
      if(rm[r])
      {
        m >> "<map>en:" << rm.ith(i).Value << " en:" << rm.ith(j).Value <<
          " en:" << rm[r] << "</map>";
        t++;
      }
    }
    {
      Ratio r = r2 / r1;
      if(r.IsEmpty()) continue;
      while(r.Numerator() >= r.Denominator() * 2 &&
        r.Denominator() < 1000000000 * 1000000000) r /= 2;
      if(rm[r])
      {
        m >> "<map>en:" << rm.ith(i).Value << " en:" << rm.ith(j).Value <<
          " en:Difference en:" << rm[r] << "</map>";
        t++;
      }
    }

  }
}
c >> t;

s >> "</category>";
s >> m;
//------------------------------------------------------------------------------

s >> "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif

#if 0
String f = "MapChromaticNoteToAccidental.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";

//------------------------------------------------------------------------------

String Letters[7] = {"C", "D", "E", "F", "G", "A", "B"};
String Identifiers[7] = {"TripleFlat", "DoubleFlat", "Flat", "",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Accidentals[7] = {"TripleFlat", "DoubleFlat", "Flat", "Natural",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Definitions[7] = {" triple-flat", " double-flat", " flat", "",
  " sharp", " double-sharp", " triple-sharp"};
count OctaveMin = -1;
count OctaveMax = 10;

for(count l = 0; l < 7; l++)
{
  for(count a = 0; a < 7; a++)
  {
    s >> "  <map>en:" << Letters[l] << Identifiers[a] <<
      " en:Accidental en:" << Accidentals[a] << "</map>";
  }
  s++;
}

//------------------------------------------------------------------------------

s << "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif

#if 0
String f = "MapChromaticNoteToLetter.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";

//------------------------------------------------------------------------------

String Letters[7] = {"C", "D", "E", "F", "G", "A", "B"};
String Identifiers[7] = {"TripleFlat", "DoubleFlat", "Flat", "",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Accidentals[7] = {"TripleFlat", "DoubleFlat", "Flat", "Natural",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Definitions[7] = {" triple-flat", " double-flat", " flat", "",
  " sharp", " double-sharp", " triple-sharp"};
count OctaveMin = -1;
count OctaveMax = 10;

for(count l = 0; l < 7; l++)
{
  for(count a = 0; a < 7; a++)
  {
    s >> "  <map>en:" << Letters[l] << Identifiers[a] <<
      " en:Letter en:" << Letters[l] << "</map>";
  }
  s++;
}

//------------------------------------------------------------------------------

s << "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif

#if 0
String f = "MapChromaticNote.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";

//------------------------------------------------------------------------------

String Letters[7] = {"C", "D", "E", "F", "G", "A", "B"};
String Identifiers[7] = {"TripleFlat", "DoubleFlat", "Flat", "",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Definitions[7] = {" triple-flat", " double-flat", " flat", "",
  " sharp", " double-sharp", " triple-sharp"};
count OctaveMin = -1;
count OctaveMax = 10;

for(count o = OctaveMin; o <= OctaveMax; o++)
{
  for(count l = 0; l < 7; l++)
  {
    for(count a = 0; a < 7; a++)
    {
      String oid;
      oid << o;
      oid.Replace("-", "_");
      s >> "  <map>en:" << Letters[l] << Identifiers[a] << oid <<
        " en:ChromaticNote en:" << Letters[l] << Identifiers[a] << "</map>";
    }
    s++;
  }
}

//------------------------------------------------------------------------------

s << "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif

#if 0
String f = "MapDiatonicPitchAccidental.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";

//------------------------------------------------------------------------------

String Letters[7] = {"C", "D", "E", "F", "G", "A", "B"};
String Identifiers[7] = {"TripleFlat", "DoubleFlat", "Flat", "Natural",
  "Sharp", "DoubleSharp", "TripleSharp"};
String IdentifiersOut[7] = {"TripleFlat", "DoubleFlat", "Flat", "",
  "Sharp", "DoubleSharp", "TripleSharp"};
count OctaveMin = -1;
count OctaveMax = 10;

for(count o = OctaveMin; o <= OctaveMax; o++)
{
  for(count l = 0; l < 7; l++)
  {
    for(count a = 0; a < 7; a++)
    {
      String oid;
      oid << o;
      oid.Replace("-", "_");
      s >> "  <map>en:" << Letters[l] << oid << " en:" << Identifiers[a] <<
        " en:" << Letters[l] << IdentifiersOut[a] << oid << "</map>";
    }
    s++;
  }
}

//------------------------------------------------------------------------------

s << "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif

#if 0
String f = "MapDiatonicPitch.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";

//------------------------------------------------------------------------------

String Letters[7] = {"C", "D", "E", "F", "G", "A", "B"};
String Identifiers[7] = {"TripleFlat", "DoubleFlat", "Flat", "",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Definitions[7] = {" triple-flat", " double-flat", " flat", "",
  " sharp", " double-sharp", " triple-sharp"};
count OctaveMin = -1;
count OctaveMax = 10;

for(count o = OctaveMin; o <= OctaveMax; o++)
{
  for(count l = 0; l < 7; l++)
  {
    for(count a = 0; a < 7; a++)
    {
      String oid;
      oid << o;
      oid.Replace("-", "_");
      s >> "  <map>en:" << Letters[l] << Identifiers[a] << oid <<
        " en:DiatonicPitch en:" << Letters[l] << oid << "</map>";
    }
    s++;
  }
}

//------------------------------------------------------------------------------

s << "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif

#if 0
String f = "MapLetterAccidentalOctave.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";

//------------------------------------------------------------------------------

String Letters[7] = {"C", "D", "E", "F", "G", "A", "B"};
String Identifiers[8] = {"TripleFlat", "DoubleFlat", "Flat", "", "Natural",
  "Sharp", "DoubleSharp", "TripleSharp"};
String IdentifiersOut[8] = {"TripleFlat", "DoubleFlat", "Flat", "", "",
  "Sharp", "DoubleSharp", "TripleSharp"};
count OctaveMin = -1;
count OctaveMax = 10;

for(count o = OctaveMin; o <= OctaveMax; o++)
{
  for(count l = 0; l < 7; l++)
  {
    for(count a = 0; a < 8; a++)
    {
      s >> "  <map>en:" << Letters[l];
      if(a != 3)
        s << " en:" << Identifiers[a];
      String oid;
      oid << o;
      oid.Replace("-", "_");
      s << " " << o << " en:" << Letters[l] << IdentifiersOut[a] << oid <<
        "</map>";
    }
    s++;
  }
}

//------------------------------------------------------------------------------

s << "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif

#if 0
String f = "MapLetterAccidental.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";

//------------------------------------------------------------------------------

String Letters[7] = {"C", "D", "E", "F", "G", "A", "B"};
String Identifiers[7] = {"TripleFlat", "DoubleFlat", "Flat", "",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Definitions[7] = {" triple-flat", " double-flat", " flat", "",
  " sharp", " double-sharp", " triple-sharp"};
count OctaveMin = -1;
count OctaveMax = 10;

for(count l = 0; l < 7; l++)
{
  for(count a = 0; a < 7; a++)
  {
    if(a == 3) continue;
    s >> "  <map>en:" << Letters[l] << " en:" << Identifiers[a] << " en:"
      << Letters[l] << Identifiers[a] << "</map>";
  }
  s++;
}

//------------------------------------------------------------------------------

s << "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif

#if 0
String f = "MapPitchToAccidental.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";

//------------------------------------------------------------------------------

String Letters[7] = {"C", "D", "E", "F", "G", "A", "B"};
String Identifiers[7] = {"TripleFlat", "DoubleFlat", "Flat", "",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Accidentals[7] = {"TripleFlat", "DoubleFlat", "Flat", "Natural",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Definitions[7] = {" triple-flat", " double-flat", " flat", "",
  " sharp", " double-sharp", " triple-sharp"};
count OctaveMin = -1;
count OctaveMax = 10;

for(count o = OctaveMin; o <= OctaveMax; o++)
{
  for(count l = 0; l < 7; l++)
  {
    for(count a = 0; a < 7; a++)
    {
      String oid;
      oid << o;
      oid.Replace("-", "_");
      s >> "  <map>en:" << Letters[l] << Identifiers[a] << oid <<
        " en:Accidental en:" << Accidentals[a] << "</map>";
    }
    s++;
  }
}

//------------------------------------------------------------------------------

s << "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif

#if 0
String f = "MapPitchToLetter.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";

//------------------------------------------------------------------------------

String Letters[7] = {"C", "D", "E", "F", "G", "A", "B"};
String Identifiers[7] = {"TripleFlat", "DoubleFlat", "Flat", "",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Accidentals[7] = {"TripleFlat", "DoubleFlat", "Flat", "Natural",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Definitions[7] = {" triple-flat", " double-flat", " flat", "",
  " sharp", " double-sharp", " triple-sharp"};
count OctaveMin = -1;
count OctaveMax = 10;

for(count o = OctaveMin; o <= OctaveMax; o++)
{
  for(count l = 0; l < 7; l++)
  {
    for(count a = 0; a < 7; a++)
    {
      String oid;
      oid << o;
      oid.Replace("-", "_");
      s >> "  <map>en:" << Letters[l] << Identifiers[a] << oid <<
        " en:Letter en:" << Letters[l] << "</map>";
    }
    s++;
  }
}

//------------------------------------------------------------------------------

s << "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif

#if 0
String f = "MapPitchToMIDI.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";

//------------------------------------------------------------------------------

count Displacement[7] = {0, 2, 4, 5, 7, 9, 11};
String Letters[7] = {"C", "D", "E", "F", "G", "A", "B"};
String Identifiers[7] = {"TripleFlat", "DoubleFlat", "Flat", "",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Accidentals[7] = {"TripleFlat", "DoubleFlat", "Flat", "Natural",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Definitions[7] = {" triple-flat", " double-flat", " flat", "",
  " sharp", " double-sharp", " triple-sharp"};
count OctaveMin = -1;
count OctaveMax = 10;

for(count o = OctaveMin; o <= OctaveMax; o++)
{
  for(count l = 0; l < 7; l++)
  {
    for(count a = 0; a < 7; a++)
    {
      count kn = o * 12 + Displacement[l] + (a - 3) + 12;
      if(kn < 0 || kn > 127) continue; //no midi values outside 0-127
      String oid;
      oid << o;
      oid.Replace("-", "_");
      s >> "  <map>en:" << Letters[l] << Identifiers[a] << oid <<
        " en:MIDIKeyNumber " << kn << "</map>";
    }
  }
  s++;
}

//------------------------------------------------------------------------------

s << "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif

#if 0
String f = "MapPitchToOctave.xml";

String s;
s >> "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
s >> "<!DOCTYPE mica PUBLIC";
s >> "  \"-//MICA//MICA DTD//EN\"";
s >> "  \"https://raw.githubusercontent.com/" <<
  "burnson/belle/master/definitions/mica.dtd\">";
s >> "<mica>";

//------------------------------------------------------------------------------

String Letters[7] = {"C", "D", "E", "F", "G", "A", "B"};
String Identifiers[7] = {"TripleFlat", "DoubleFlat", "Flat", "",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Accidentals[7] = {"TripleFlat", "DoubleFlat", "Flat", "Natural",
  "Sharp", "DoubleSharp", "TripleSharp"};
String Definitions[7] = {" triple-flat", " double-flat", " flat", "",
  " sharp", " double-sharp", " triple-sharp"};
count OctaveMin = -1;
count OctaveMax = 10;

for(count o = OctaveMin; o <= OctaveMax; o++)
{
  for(count l = 0; l < 7; l++)
  {
    for(count a = 0; a < 7; a++)
    {
      String oid;
      oid << o;
      oid.Replace("-", "_");
      s >> "  <map>en:" << Letters[l] << Identifiers[a] << oid <<
        " en:Octave " << o << "</map>";
    }
    s++;
  }
}

//------------------------------------------------------------------------------

s << "</mica>";
s++;
File::Write(f, s);
c >> "Wrote: " << f;
#endif
