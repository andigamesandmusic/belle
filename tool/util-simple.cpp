/*
  ==============================================================================

  Copyright 2023 Andi

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

#define BELLE_COMPILE_INLINE
#include "belle.h"

// Use the example helper
#include "belle-helper.h"

// Import the belle namespace for convenience
using namespace belle;

// Creates a simple music system with ties, slurs, and tuplets.
// For more examples see the programmatic music system creation
// functions in: include/belle-test-suite.h.
static void MakeSimpleMusicSystem(Score& score)
{
  // Create a pointer to hold a reference to the music system graph.
  Pointer<Music> m;

  // Create a new music system graph object to store the nodes and edges.
  m.New();

  // Allocate some node and edge pointers ahead of time to help with
  // linking things together as we go along.
  Music::Node island, chord, note, prevIsland, prevChord, prevNote;
  Music::Edge tupletLink;

  // The following scoped brackets each represent a music island. An
  // island is a container for a barline, clef, key signature, time
  // signature, chord, etc. The islands are linked together with
  // edges using the mica::Partwise type. This is how we define
  // the left-to-right sequence of musical objects in the graph.

  // Create the first barline and add it to a new island.
  {
    island = m->CreateAndAddBarline();
    prevIsland = island;
  }

  // Create a clef and add it to a new island, then make a partwise
  // link from the previous island to this island.
  {
    island = m->CreateAndAddClef(mica::TrebleClef);
    m->Connect(prevIsland, island)->Set(mica::Type) = mica::Partwise;
    prevIsland = island;
  }

  // Create a chord token, set its duration to 1/2 (half note), then
  // add an E5 note and add it to the chord token. Finally, add the
  // chord token to the island, and again link the previous island
  // to this island.
  {
    chord = m->CreateToken(mica::Chord);
    chord->Set(mica::NoteValue) = mica::Concept(Ratio(1, 2));
    note = m->CreateAndAddNote(chord, mica::E5);

    island = m->CreateIsland();
    m->AddTokenToIsland(island, chord);
    m->Connect(prevIsland, island)->Set(mica::Type) = mica::Partwise;
    prevIsland = island;
    prevChord = chord;
    prevNote = note;
  }

  // In this island we link the E5 note from the previous island
  // to the one in this island with a tie. Then we define the
  // start of a 3:2 tuplet and set it display on the beam with the
  // full ratio for clarity. We then link the tuplet to the first
  // chord of the tuplet so the engraver knows where to start the
  // tuplet.
  {
    chord = m->CreateToken(mica::Chord);
    chord->Set(mica::NoteValue) = mica::Concept(Ratio(3, 16));
    note = m->CreateAndAddNote(chord, mica::E5);
    m->Connect(prevNote, note)->Set(mica::Type) = mica::Tie;

    Music::Node tuplet = m->Add();
    tuplet->Set(mica::Type) = mica::Tuplet;
    tuplet->Set(mica::Value) = mica::Concept(Ratio(3, 2));
    tuplet->Set(mica::FullRatio) = mica::FullRatio;
    tuplet->Set(mica::Placement) = mica::Beam;

    tupletLink = m->Connect(tuplet, chord);
    tupletLink->Set(mica::Type) = mica::Tuplet;

    island = m->CreateIsland();
    m->AddTokenToIsland(island, chord);
    m->Connect(prevIsland, island)->Set(mica::Type) = mica::Partwise;
    prevIsland = island;
    prevChord = chord;
    prevNote = note;
  }

  // For the middle chord we now start linking from the previous
  // chord with a beam and a tuplet edge to make sure the engraver
  // knows to beam to this chord and continue the 3:2 tuplet that
  // was defined before
  {
    chord = m->CreateToken(mica::Chord);
    chord->Set(mica::NoteValue) = mica::Concept(Ratio(3, 32));
    note = m->CreateAndAddNote(chord, mica::E5);
    m->Connect(prevNote, note)->Set(mica::Type) = mica::Tie;
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Beam;
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Tuplet;

    island = m->CreateIsland();
    m->AddTokenToIsland(island, chord);
    m->Connect(prevIsland, island)->Set(mica::Type) = mica::Partwise;
    prevIsland = island;
    prevChord = chord;
    prevNote = note;
  }

  // This time instead of creating a tie from the previous note,
  // we create a slur from the previous chord. This is because
  // a slur applies to the chord-level when the notes it connects
  // are different.
  {
    chord = m->CreateToken(mica::Chord);
    chord->Set(mica::NoteValue) = mica::Concept(Ratio(3, 32));
    note = m->CreateAndAddNote(chord, mica::G4);
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Slur;
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Beam;
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Tuplet;

    island = m->CreateIsland();
    m->AddTokenToIsland(island, chord);
    m->Connect(prevIsland, island)->Set(mica::Type) = mica::Partwise;
    prevIsland = island;
    prevChord = chord;
    prevNote = note;
  }

  // We've reached the beginning of the next beam and tuplet group,
  // so this time we won't continue the beam or tuplet edges from
  // the previous chord. But we will tie from the previous note.
  {
    chord = m->CreateToken(mica::Chord);
    chord->Set(mica::NoteValue) = mica::Concept(Ratio(1, 32));
    note = m->CreateAndAddNote(chord, mica::G4);
    m->Connect(prevNote, note)->Set(mica::Type) = mica::Tie;

    Music::Node tuplet = m->Add();
    tuplet->Set(mica::Type) = mica::Tuplet;
    tuplet->Set(mica::Value) = mica::Concept(Ratio(3, 2));
    tuplet->Set(mica::FullRatio) = mica::FullRatio;
    tuplet->Set(mica::Placement) = mica::Beam;

    tupletLink = m->Connect(tuplet, chord);
    tupletLink->Set(mica::Type) = mica::Tuplet;

    island = m->CreateIsland();
    m->AddTokenToIsland(island, chord);
    m->Connect(prevIsland, island)->Set(mica::Type) = mica::Partwise;
    prevIsland = island;
    prevChord = chord;
    prevNote = note;
  }

  // The remaining islands follow the same patterns as set up before.
  {
    chord = m->CreateToken(mica::Chord);
    chord->Set(mica::NoteValue) = mica::Concept(Ratio(1, 8));
    note = m->CreateAndAddNote(chord, mica::C5);
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Slur;
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Beam;
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Tuplet;

    island = m->CreateIsland();
    m->AddTokenToIsland(island, chord);
    m->Connect(prevIsland, island)->Set(mica::Type) = mica::Partwise;
    prevIsland = island;
    prevChord = chord;
    prevNote = note;
  }

  {
    chord = m->CreateToken(mica::Chord);
    chord->Set(mica::NoteValue) = mica::Concept(Ratio(1, 32));
    note = m->CreateAndAddNote(chord, mica::C5);
    m->Connect(prevNote, note)->Set(mica::Type) = mica::Tie;
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Beam;
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Tuplet;

    island = m->CreateIsland();
    m->AddTokenToIsland(island, chord);
    m->Connect(prevIsland, island)->Set(mica::Type) = mica::Partwise;
    prevIsland = island;
    prevChord = chord;
    prevNote = note;
  }

  {
    chord = m->CreateToken(mica::Chord);
    chord->Set(mica::NoteValue) = mica::Concept(Ratio(1, 8));
    note = m->CreateAndAddNote(chord, mica::BFlat4);
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Beam;
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Tuplet;

    island = m->CreateIsland();
    m->AddTokenToIsland(island, chord);
    m->Connect(prevIsland, island)->Set(mica::Type) = mica::Partwise;
    prevIsland = island;
    prevChord = chord;
    prevNote = note;
  }

  {
    chord = m->CreateToken(mica::Chord);
    chord->Set(mica::NoteValue) = mica::Concept(Ratio(1, 32));
    note = m->CreateAndAddNote(chord, mica::BFlat4);
    m->Connect(prevNote, note)->Set(mica::Type) = mica::Tie;
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Beam;
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Tuplet;

    island = m->CreateIsland();
    m->AddTokenToIsland(island, chord);
    m->Connect(prevIsland, island)->Set(mica::Type) = mica::Partwise;
    prevIsland = island;
    prevChord = chord;
    prevNote = note;
  }

  {
    chord = m->CreateToken(mica::Chord);
    chord->Set(mica::NoteValue) = mica::Concept(Ratio(1, 32));
    note = m->CreateAndAddNote(chord, mica::E4);
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Slur;
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Beam;
    m->Connect(prevChord, chord)->Set(mica::Type) = mica::Tuplet;

    island = m->CreateIsland();
    m->AddTokenToIsland(island, chord);
    m->Connect(prevIsland, island)->Set(mica::Type) = mica::Partwise;
    prevIsland = island;
    prevChord = chord;
    prevNote = note;
  }

  // Create the final barline.
  {
    island = m->CreateAndAddBarline();
    m->Connect(prevIsland, island)->Set(mica::Type) = mica::Partwise;
    prevIsland = island;
  }

  // Add this music graph system to the score.
  score.AddSystem(m);
}

int main()
{
  // Create a score and initialize with the font.
  Score myScore;
  myScore.InitializeFont(Helper::ImportNotationFont());

  // Set the measurements of the score.
  number pageWidth = 8.5f;
  number systemLeft = 0.625f / 2.f;
  number firstSystemLeft = systemLeft + 0.4f;
  number pageHeight = 11.f;
  number staffToStaffDistance = 30.f;
  number systemToSystemDistance = 30.f;
  number systemWidth = 4.f;
  number topMargin = 1.f, bottomMargin = 1.f,
    firstMargin = 0.f, firstBottomMargin = 0.f;
  number maxSystemDistanceRelative = 1.5f;
  Inches PageSize(pageWidth, pageHeight);

  // Add music systems to the score.
  MakeSimpleMusicSystem(myScore);

  // Engrave the music systems.
  myScore.SetSystemWidth(systemWidth);
  myScore.SetSystemLeft(firstSystemLeft, systemLeft);
  myScore.Engrave();

  // Layout the score systems onto pages.
  myScore.Layout(PageSize, topMargin, bottomMargin, staffToStaffDistance,
    systemToSystemDistance, systemToSystemDistance * maxSystemDistanceRelative,
    firstMargin, firstBottomMargin);

  // Set the PDF-specific properties.
  String filename = "output.pdf";
  PDF::Properties PDFSpecificProperties(filename);

  // Write the score to PDF.
  myScore.Create<PDF>(PDFSpecificProperties);

  // Dump the score graph XML to the console for debugging.
  for(count i = 0; i < myScore.n(); i++)
  {
    C::Out() >> myScore.ith(i)->ExportXML();
  }

  C::Out() >> "" >> "Wrote: " << filename >> "";

  return 0;
}
