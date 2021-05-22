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

/**
@name Rests

Functions that implement construction of rests.
@{
*/

#ifdef BELLE_IMPLEMENTATION
/**Returns whether a chord token is a rest. A rest is indicated by a chord
token with no note nodes. In the past, the way to define a chord as a rest was
not well-defined. Setting a Rest property on the chord token or a note would
act as a rest signifier, so this function also takes into account the old
form.*/
bool IsRest(Music::ConstNode Chord)
{
  Array<Music::ConstNode> Notes = NotesOfChord(Chord);
  bool NoteAsRest = false;
  for(count i = 0; not NoteAsRest and i < Notes.n(); i++)
    if(Notes[i]->Get(mica::Rest) != mica::Undefined)
      NoteAsRest = true;
  return not Notes.n() or NoteAsRest or (IsChord(Chord) and
    Chord->Get(mica::Rest) != mica::Undefined);
}

void EngraveRest(Music::ConstNode Island, Music::ConstNode Chord)
{
  Pointer<Stamp> IslandStamp = StampForIsland(Island);
  Ratio Duration = IntrinsicDurationOfChord(Chord);

  Stamp RestStamp;
  Pointer<const Glyph> Rest = RestGlyph(FontFromIsland(Island), Duration);
  if(!Rest)
    return;
  RestStamp.Add()->p = Rest;
  RestStamp.z()->Context = Chord;
  Vector InternalLocation = UndottedDuration(Duration) == Ratio(1, 1) ?
    Vector(0, 1) : Vector();
  RestStamp.z()->a = Affine::Translate(InternalLocation);
  Box RestBounds = RestStamp.Bounds();

  Value Locations;
  Locations.a()["Dots"] = DurationDots(Duration);
  Locations.a()["StaffPosition"] = 1 + Max(count(0),
    (RestIndexGivenDuration(Duration) - 5) / 2) * 2;
  Locations.a()["Rest"] = Chord;
  //Override for 1024th rest:
  if(RestIndexGivenDuration(Duration) == 12)
    Locations.a()["StaffPosition"] = 1 + 4 * 2;
  number DotStartX = RestBounds.IsEmpty() ? number(0.f) : RestBounds.Right();
  EngraveDotsFromList(Island, Locations, DotStartX, RestStamp);

  number YPosition = 0;
  if(not IslandStamp->IsEmpty())
  {
    Box ExistingBounds = IslandStamp->Bounds();
    bool Above = StrandIDOfChord(Chord) % 2 == 0;
    if(Above)
      YPosition = number(Ceiling(ExistingBounds.Top() - RestBounds.Bottom()));
    else
      YPosition = number(Ceiling(ExistingBounds.Bottom() - RestBounds.Top()));
  }
  Vector RestLocation(-RestBounds.Center().x, YPosition);
  RestStamp.a = Affine::Translate(RestLocation);
  IslandStamp->AccumulateGraphics(RestStamp);
}

count RestIndexGivenDuration(Ratio r)
{
  r = UndottedDuration(r);
  count RestIndex = 0;
  while(r > 0 and r < Ratio(4, 1))
    r *= 2, RestIndex++;
  return RestIndex;
}

Pointer<const Glyph> RestGlyph(Pointer<const Font> f, Ratio Duration)
{
  return SMuFLGlyph(f, mica::map(mica::SMuFL, mica::item(
    mica::Rests, int64(RestIndexGivenDuration(Duration)))));
}
#endif
///@}

//Declarations
bool IsRest(Music::ConstNode Chord);
Pointer<const Glyph> RestGlyph(Pointer<const Font> f, Ratio Duration);
count RestIndexGivenDuration(Ratio r);
void EngraveRest(Music::ConstNode Island, Music::ConstNode Chord);
