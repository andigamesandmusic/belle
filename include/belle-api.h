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

namespace BELLE_NAMESPACE
{
  class Stamp;

  #include "belle-accidentals.h"
  #include "belle-annotations.h"
  #include "belle-accidental-state.h"
  #include "belle-articulations.h"
  #include "belle-barline-state.h"
  #include "belle-chord-state.h"
  #include "belle-clef-state.h"
  #include "belle-dots.h"
  #include "belle-expressions.h"
  #include "belle-filter-autocorrect.h"
  #include "belle-filter-beaming.h"
  #include "belle-filter-harmonization.h"
  #include "belle-filter-interval.h"
  #include "belle-filter-pitch.h"
  #include "belle-filter-rhythm.h"
  #include "belle-filter-staff-select.h"
  #include "belle-filter-time.h"
  #include "belle-filter-traversal.h"
  #include "belle-flags.h"
  #include "belle-formats.h"
  #include "belle-general.h"
  #include "belle-incipits.h"
  #include "belle-key-signature-state.h"
  #include "belle-ledger-lines.h"
  #include "belle-measure-rest.h"
  #include "belle-midi.h"
  #include "belle-multivoice.h"
  #include "belle-musicxml.h"
  #include "belle-notes.h"
  #include "belle-octave-transposition.h"
  #include "belle-part-state.h"
  #include "belle-pedal-markings.h"
  #include "belle-performance.h"
  #include "belle-rests.h"
  #include "belle-search.h"
  #include "belle-stems.h"
  #include "belle-time-signature.h"
  #include "belle-time-signature-state.h"
  #include "belle-tuplets.h"
  #include "belle-voicing.h"
  #include "belle-wrap.h"
}
