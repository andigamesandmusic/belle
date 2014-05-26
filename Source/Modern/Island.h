/*
  ==============================================================================

  Copyright 2007-2013 William Andrew Burnson. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY WILLIAM ANDREW BURNSON ''AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
  EVENT SHALL WILLIAM ANDREW BURNSON OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  ------------------------------------------------------------------------------

  This file is part of Belle, Bonne, Sage --
    The 'Beautiful, Good, Wise' C++ Vector-Graphics Library for Music Notation 

  ==============================================================================
*/

#ifndef BELLE_MODERN_ISLAND_H
#define BELLE_MODERN_ISLAND_H

#include "Directory.h"

#include "Barline.h"
#include "Beaming.h"
#include "Chord.h"
#include "Clef.h"
#include "Custom.h"
#include "KeySignature.h"
#include "Meter.h"
#include "State.h"

namespace BELLE_NAMESPACE { namespace modern
{
  ///Algorithms for typesetting an island.
  struct IslandEngraver : public DirectoryHandler
  {     
    ///Constructor to assign external resources to this engraver.
    IslandEngraver(Directory& d) : DirectoryHandler(d) {}
    
    ///Engraves the island.
    void Engrave(graph::Music& g, graph::MusicNode n, Stamp& s)
    {
      //Get all the tokens belonging to the island.
      prim::Array<graph::MusicNode> Tokens = g.Children(n,
        graph::MusicLabel::Token()), TokenArray = Tokens;
      UpdateStemState(g, TokenArray);
      
      //Engrave each token.
      for(prim::count i = 0; i < Tokens.n(); i++)
        EngraveToken(g, Tokens[i], s);
    }
    
    ///Updates the current stem state.
    void UpdateStemState(graph::Music& g, prim::Array<graph::MusicNode>& Tokens)
    {
      //Get references to the stem states.
      Chord::State& Current = d.s.Current;
      Chord::State& Previous = d.s.Previous;
      
      //Grab all the chord tokens.
      Current.n(0);
      bool ContainsChords = false;
      for(prim::count i = 0; i < Tokens.n(); i++)
      {
        if(Tokens[i]->Get(mica::Value) == mica::Chord)
        {
          ContainsChords = true;
          Current.Add().c = Tokens[i];
          Current.z().UpdateTessitura(g);
          Current.z().p = g.Previous(Current.z().c,
            graph::MusicLabel::Continuity());
        }
        else
        {
          Current.Add().c = 0;
          Current.z().Tessitura = 999.0;
        }
      }
      
      //If there are no chords in the island, then there is nothing to process.
      if(!ContainsChords) return;
      
      //Sort from bottom to top.
      Current.Sort();
      
      //Determine the number of new voices.
      prim::count NewVoices = 0;
      for(prim::count i = 0; i < Current.n(); i++)
      {
        Current[i].NewVoice = true;
        NewVoices++;
        if(graph::MusicNode Needle = Current[i].p)
        {
          for(prim::count j = 0; j < Previous.n(); j++)
          {
            if(Needle == Previous[j].c)
            {
              Current[i].NewVoice = false;
              Current[i].pd = Previous[j].d;
              NewVoices--;
              break;
            }
          }
        }
      }
      
      //By default use same stem arrangements as previous island.
      for(prim::count i = 0; i < Current.n(); i++)
        Current[i].d = Current[i].pd;
          
      //Determine stems.
      if((Previous.n() == 1 && Current.n() > 1) || NewVoices)
      {
        /*There are some new voices and some preexisting voices. Any voices
        which already exist use their existing stem arrangements.*/
        
        //Alternate directions stem directions starting with down on low voice.
        for(prim::count i = 0; i < Current.n(); i++)
        {
          if((Previous.n() == 1 && Current.n() > 1) || Current[i].NewVoice)
          {
            Current[i].d =
              (i % 2 == 0 ? Chord::StateInfo::Down : Chord::StateInfo::Up);
          }
        }
      }
      
      //Recycle stem state.
      if(d.s.Current.n())
        d.s.Previous = d.s.Current;
    }

    ///Engraves the token.
    void EngraveToken(graph::Music& g, graph::MusicNode Token, Stamp& s)
    {
      mica::Concept TokenType = Token->Get(mica::Kind);
      if(TokenType == mica::Chord)
      {
        Chord ChordInfo;
        ChordInfo.Import(g, Token);
        
        /*Need to detect any unisons or octaves with different accidentals and
        clear their state if they have differing accidentals. Also octaves
        with same accidentals need to report accidentals if not in active
        state.*/
        
        //Determine the accidental state.
        for(prim::count j = 0; j < ChordInfo.StaffNotes.n(); j++)
        {
          Chord::StaffNote& sn = ChordInfo.StaffNotes[j];
          sn.Accidental =
            d.s.ConsumeAccidental(0, sn.Accidental); //FIX ME: Need to consume
          //pitch instead of line space.
        }
        
        //Read in the stem state.
        //ChordInfo.DetermineStemDirectionByPosition();

        prim::count k = -1;
        for(prim::count i = 0; i < d.s.Current.n(); i++)
        {
          if(d.s.Current[i].c == Token)
          {
            k = i;
            break;
          }
        }
        if(k >= 0)
        {
          if(d.s.Current[k].d == Chord::StateInfo::PositionBased)  
          {
            ChordInfo.DetermineStemDirectionByPosition();
            prim::c >> "Position based used..";
          }
          else
            ChordInfo.StemUp = d.s.Current[k].d == Chord::StateInfo::Up;
        }
        else
        {
          //prim::c >> "ChordState not found";
        }
        
        //Engrave the chord.
        ChordInfo.Engrave(s, d.h, d.c, d.t, d.f);
      }
      else if(TokenType == mica::Clef)
      {
        prim::number ClefSize = d.s.ActiveClef == mica::Undefined ?
          1.0 : d.h.NonInitialClefSize;
        ClefSize = 1.0; //For now this is hacked after the fact.
        d.s.ActiveClef = Token->Get(mica::Value);
        Clef::Engrave(d, s, d.s.ActiveClef, ClefSize);
      }
      else if(TokenType == mica::KeySignature)
      {
        mica::Concept k = Token->Get(mica::Value);
        d.s.SetKeySignature(k);
        d.s.ResetActiveAccidentalsToKeySignature();
        KeySignature::Engrave(d, s, Token);
      }
      else if(TokenType == mica::TimeSignature)
      {
        Meter::Engrave(d, s, Token);
      }
      else if(TokenType == mica::Barline)
      {
        Barline::Engrave(d, s, Token);
        d.s.ResetActiveAccidentalsToKeySignature();
      }
      else
      {
        Custom::Engrave(Token, s, d);
      }
    }
  };
}}
#endif
