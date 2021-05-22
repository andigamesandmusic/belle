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

#ifndef BELLE_ENGRAVER_TESTSUITEDATA_H
#define BELLE_ENGRAVER_TESTSUITEDATA_H

namespace BELLE_NAMESPACE
{
  /**Functions to retrieve test suite data. Functions beginning with BehindBars
  are adapted from examples printed in Behind Bars (c) Elaine Gould 2011. The
  numbers in the function name indicate the page numbers the examples are
  sourced from. Only the relevant symbolic information is being taken from the
  examples so that Belle can produce new typesettings using similar information
  to compare against the reference settings in the book. This usage falls under
  fair use since it is transformative and does not intend to supersede the
  original.*/
  class TestSuiteData
  {
    public:

    class ChordData
    {
      public:
      Value StemUp;
      Ratio Duration;
      List<mica::Concept> Notes;
    };

    static List<List<List<ChordData> > > BehindBars_047_047_ChordTests()
    {
      List<List<List<ChordData> > > L;

      /*
      //Behind Bars p. 47-1
      L.Add().Add().Add().Duration = Ratio(7, 16);
      L.z().z().z().Notes.Add() = mica::D4;
      L.z().z().z().Notes.Add() = mica::F4;
      L.z().z().z().Notes.Add() = mica::A4;
      L.z().z().z().Notes.Add() = mica::D5;
      L.z().Add().Add().Duration = Ratio(1, 16);
      L.z().z().z().Notes.Add() = mica::D4;
      L.z().z().z().Notes.Add() = mica::F4;
      L.z().z().z().Notes.Add() = mica::A4;
      L.z().z().z().Notes.Add() = mica::D5;
      L.z().Add().Add().Duration = Ratio(1, 2);
      L.z().z().z().Notes.Add() = mica::A4;
      L.z().z().z().Notes.Add() = mica::D5;
      L.z().z().z().Notes.Add() = mica::F5;
      L.z().z().z().Notes.Add() = mica::A5;
      */

      //Behind Bars p. 47-3
      L.Add().Add().Add().Duration = Ratio(1, 2);
      L.z().z().z().Notes.Add() = mica::D4;
      L.z().z().z().StemUp = false;
      L.z().z().Add().Duration = Ratio(7, 16);
      L.z().z().z().Notes.Add() = mica::F4;
      L.z().z().z().Notes.Add() = mica::A4;
      L.z().z().z().Notes.Add() = mica::D5;
      L.z().z().z().StemUp = true;
      L.z().Add().Add().Duration = Ratio(1, 16);
      L.z().z().z().Notes.Add() = mica::F4;
      L.z().z().z().Notes.Add() = mica::A4;
      L.z().z().z().Notes.Add() = mica::D5;
      L.z().z().z().StemUp = true;
      L.z().Add().Add().Duration = Ratio(1, 2);
      L.z().z().z().Notes.Add() = mica::A4;
      L.z().z().z().Notes.Add() = mica::D5;
      L.z().z().z().Notes.Add() = mica::F5;
      L.z().z().z().StemUp = false;
      L.z().z().Add().Duration = Ratio(1, 4);
      L.z().z().z().Notes.Add() = mica::A5;
      L.z().z().z().StemUp = true;
      L.z().Add().Add().Duration = Ratio(1, 4);
      L.z().z().z().Notes.Add() = mica::Rest;

      return L;
    }

    static List<List<mica::Concept> >  BehindBars_087_090_AccidentalTests()
    {
      List<List<mica::Concept> > L;

      //Behind Bars p. 87
      L.Add().Add() = mica::GSharp4;
      L.z().Add()   = mica::ASharp4;
      L.z().Add()   = mica::B4;
      L.z().Add()   = mica::CSharp5;
      L.z().Add()   = mica::DSharp5;

      //Behind Bars p. 88
      L.Add().Add() = mica::EFlat5;
      L.z().Add()   = mica::FFlat4;

      L.Add().Add() = mica::EFlat5;
      L.z().Add()   = mica::F4;

      L.Add().Add() = mica::EFlat5;
      L.z().Add()   = mica::FSharp4;

      L.Add().Add() = mica::E5;
      L.z().Add()   = mica::FFlat4;

      L.Add().Add() = mica::E5;
      L.z().Add()   = mica::F4;

      L.Add().Add() = mica::E5;
      L.z().Add()   = mica::FSharp4;

      L.Add().Add() = mica::ESharp5;
      L.z().Add()   = mica::FFlat4;

      L.Add().Add() = mica::ESharp5;
      L.z().Add()   = mica::F4;

      L.Add().Add() = mica::ESharp5;
      L.z().Add()   = mica::FSharp4;

      L.Add().Add() = mica::EFlat5;
      L.z().Add()   = mica::GFlat4;

      L.Add().Add() = mica::EFlat5;
      L.z().Add()   = mica::G4;

      L.Add().Add() = mica::EFlat5;
      L.z().Add()   = mica::GSharp4;

      L.Add().Add() = mica::ESharp5;
      L.z().Add()   = mica::GSharp4;

      L.Add().Add() = mica::DFlat5;
      L.z().Add()   = mica::GFlat4;

      L.Add().Add() = mica::D5;
      L.z().Add()   = mica::G4;

      L.Add().Add() = mica::EFlat5;
      L.z().Add()   = mica::BFlat4;

      L.Add().Add() = mica::B4;
      L.z().Add()   = mica::FFlat4;

      L.Add().Add() = mica::B4;
      L.z().Add()   = mica::F4;

      L.Add().Add() = mica::BFlat4;
      L.z().Add()   = mica::F4;

      //Behind Bars p. 89
      L.Add().Add() = mica::AFlat4;
      L.z().Add()   = mica::F4;
      L.z().Add()   = mica::DFlat4;

      L.Add().Add() = mica::GSharp5;
      L.z().Add()   = mica::FSharp5;
      L.z().Add()   = mica::ESharp5;
      L.z().Add()   = mica::DSharp5;
      L.z().Add()   = mica::CSharp5;

      L.Add().Add() = mica::E5;
      L.z().Add()   = mica::AFlat4;
      L.z().Add()   = mica::F4;

      L.Add().Add() = mica::DSharp5;
      L.z().Add()   = mica::ASharp4;
      L.z().Add()   = mica::FSharp4;
      L.z().Add()   = mica::DSharp4;

      L.Add().Add() = mica::F5;
      L.z().Add()   = mica::E5;
      L.z().Add()   = mica::D5;
      L.z().Add()   = mica::B4;
      L.z().Add()   = mica::AFlat4;
      L.z().Add()   = mica::GSharp4;

      L.Add().Add() = mica::A5;
      L.z().Add()   = mica::FSharp5;
      L.z().Add()   = mica::G4;
      L.z().Add()   = mica::E4;

      L.Add().Add() = mica::ESharp5;
      L.z().Add()   = mica::CSharp5;
      L.z().Add()   = mica::FSharp4;
      L.z().Add()   = mica::DSharp4;

      //Behind Bars p. 90
      L.Add().Add() = mica::FSharp5;
      L.z().Add()   = mica::FSharp4;
      L.z().Add()   = mica::DSharp4;

      L.Add().Add() = mica::AFlat5;
      L.z().Add()   = mica::FSharp5;
      L.z().Add()   = mica::AFlat4;
      L.z().Add()   = mica::FSharp4;

      L.Add().Add() = mica::BFlat5;
      L.z().Add()   = mica::A5;
      L.z().Add()   = mica::C5;
      L.z().Add()   = mica::BFlat4;
      L.z().Add()   = mica::F4;

      L.Add().Add() = mica::GSharp5;
      L.z().Add()   = mica::FSharp4;
      L.z().Add()   = mica::ESharp4;
      L.z().Add()   = mica::C4;
      L.z().Add()   = mica::Accidental; //Hide natural signs for this chord.

      L.Add().Add() = mica::F5;
      L.z().Add()   = mica::AFlat4;
      L.z().Add()   = mica::GFlat4;

      L.Add().Add() = mica::FSharp5;
      L.z().Add()   = mica::E5;
      L.z().Add()   = mica::DSharp4;
      L.z().Add()   = mica::CSharp4;

      L.Add().Add() = mica::AFlat5;
      L.z().Add()   = mica::GFlat5;
      L.z().Add()   = mica::C5;
      L.z().Add()   = mica::BFlat4;
      L.z().Add()   = mica::EFlat4;
      L.z().Add()   = mica::DFlat4;

      L.Add().Add() = mica::C5;
      L.z().Add()   = mica::BFlat4;
      L.z().Add()   = mica::F4;

      L.Add().Add() = mica::FSharp5;
      L.z().Add()   = mica::E5;
      L.z().Add()   = mica::A4;

      L.Add().Add() = mica::FSharp5;
      L.z().Add()   = mica::E5;
      L.z().Add()   = mica::GSharp4;
      L.z().Add()   = mica::Accidental; //Hide natural signs for this chord.

      L.Add().Add() = mica::A5;
      L.z().Add()   = mica::DFlat5;
      L.z().Add()   = mica::C5;
      L.z().Add()   = mica::DFlat4;
      L.z().Add()   = mica::Accidental; //Hide natural signs for this chord.

      L.Add().Add() = mica::G5;
      L.z().Add()   = mica::EFlat5;
      L.z().Add()   = mica::F4;
      L.z().Add()   = mica::EFlat4;
      L.z().Add()   = mica::Accidental; //Hide natural signs for this chord.

      return L;
    }

    static List<List<mica::Concept> > SmallClusters()
    {
      List<List<mica::Concept> > L;

      L.Add().Add() = mica::CFlat3;
      L.z().Add()   = mica::GSharp3;

      L.Add().Add() = mica::CFlat3;
      L.z().Add()   = mica::FSharp3;

      L.Add().Add() = mica::CFlat3;
      L.z().Add()   = mica::ESharp3;

      L.Add().Add() = mica::CFlat3;
      L.z().Add()   = mica::DSharp3;

      L.Add().Add() = mica::CFlat3;
      L.z().Add()   = mica::CSharp3;

      L.Add().Add() = mica::CFlat3;
      L.z().Add()   = mica::D3;
      L.z().Add()   = mica::GSharp3;

      L.Add().Add() = mica::CFlat3;
      L.z().Add()   = mica::E3;
      L.z().Add()   = mica::GSharp3;

      L.Add().Add() = mica::CFlat3;
      L.z().Add()   = mica::F3;
      L.z().Add()   = mica::GSharp3;

      L.Add().Add() = mica::CFlat3;
      L.z().Add()   = mica::D3;
      L.z().Add()   = mica::ESharp3;

      L.Add().Add() = mica::CFlat3;
      L.z().Add()   = mica::D3;
      L.z().Add()   = mica::FSharp3;

      L.Add().Add() = mica::CFlat3;
      L.z().Add()   = mica::E3;
      L.z().Add()   = mica::FSharp3;

      L.Add().Add() = mica::CDoubleFlat3;
      L.z().Add()   = mica::DFlat3;
      L.z().Add()   = mica::ESharp3;
      L.z().Add()   = mica::GDoubleSharp3;

      L.Add().Add() = mica::CDoubleFlat3;
      L.z().Add()   = mica::DFlat3;
      L.z().Add()   = mica::FSharp3;
      L.z().Add()   = mica::GDoubleSharp3;

      L.Add().Add() = mica::CDoubleFlat3;
      L.z().Add()   = mica::EFlat3;
      L.z().Add()   = mica::FSharp3;
      L.z().Add()   = mica::GDoubleSharp3;

      L.Add().Add() = mica::CDoubleFlat3;
      L.z().Add()   = mica::DFlat3;
      L.z().Add()   = mica::ESharp3;
      L.z().Add()   = mica::FDoubleSharp3;

      L.Add().Add() = mica::CDoubleFlat3;
      L.z().Add()   = mica::DFlat3;
      L.z().Add()   = mica::E3;
      L.z().Add()   = mica::FSharp3;
      L.z().Add()   = mica::GDoubleSharp3;

      L.Add().Add() = mica::CDoubleFlat3;
      L.z().Add()   = mica::DFlat3;
      L.z().Add()   = mica::ESharp3;
      L.z().Add()   = mica::FSharp3;
      L.z().Add()   = mica::GDoubleSharp3;
      L.z().Add()   = mica::B2;
      L.z().Add()   = mica::Accidental;

      return L;
    }
  };
}
#endif
