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

#ifndef BELLE_GRAPH_PROPERTIES_H
#define BELLE_GRAPH_PROPERTIES_H

#include "Music.h"

namespace BELLE_NAMESPACE { namespace graph
{
  ///Static structure to help assign instant properties.
  struct Instant
  {
    struct Properties : public prim::Array<mica::Concept>
    {
      bool IsSystemBreak() const {return Contains(mica::SystemBreak);}
      bool IsOptionalBreak() const {return Contains(mica::OptionalBreak);}
      bool IsRepeatingInstant() const {return Contains(mica::RepeatingInstant);}
      bool IsAbsorbedInstant() const {return Contains(mica::AbsorbedInstant);}
      
      void SetSystemBreak() {if(!IsSystemBreak()) Add(mica::SystemBreak);}
      void SetOptionalBreak() {if(!IsOptionalBreak()) Add(mica::OptionalBreak);}
      void SetRepeatingInstant()
      {
        if(!IsRepeatingInstant()) Add(mica::RepeatingInstant);
      }
      void SetAbsorbedInstant()
      {
        if(!IsAbsorbedInstant()) Add(mica::AbsorbedInstant);
      }
      
      prim::String ToString() const
      {
        prim::String s;
        s >> "System break? " << (IsSystemBreak() ? "yes" : "no");
        s >> "Optional break? " << (IsOptionalBreak() ? "yes" : "no");
        s >> "Repeating instant? " << (IsRepeatingInstant() ? "yes" : "no");
        s >> "Absorbed instant? " << (IsAbsorbedInstant() ? "yes" : "no");
        return s;
      }
    };
    
    /**Sets default properties on score. This method does make a few assumptions
    about the geometry of the score, mostly that it is relatively well-behaved
    as regards the initial material and the appearances of barlines.*/
    static void SetDefaultProperties(Music& g)
    {
      MusicNode n = g.Root();
      bool InitialMaterial = true;
      bool BarlineAppeared = false;
      bool ClefAppeared = false;
      bool KeySignatureAppeared = false;
      while(n)
      {
        MusicNode m = g.Next(n, MusicLabel::Token());
        if(!m)
        {
          n = g.Next(n, MusicLabel::Partwise());
          continue;
        }
        
        Instant::Properties p;

        if(InitialMaterial)
        {
          if(m->Get(mica::Kind) == mica::Barline && !BarlineAppeared)
          {
            p.SetRepeatingInstant();
            BarlineAppeared = true;
          }
          else if(m->Get(mica::Kind) == mica::Clef && !ClefAppeared)
          {
            p.SetRepeatingInstant();
            ClefAppeared = true;
          }
          else if(m->Get(mica::Kind) == mica::KeySignature &&
            !KeySignatureAppeared)
          {
            p.SetRepeatingInstant();
            KeySignatureAppeared = true;
          }
          else
            InitialMaterial = false;
        }
        else
        {
          if(m->Get(mica::Kind) == mica::Barline)
            p.SetOptionalBreak();
        }

        Instant::SetProperties(g, p, n);
        n = g.Next(n, MusicLabel::Partwise());
      }
    }
    
    ///Get properties on the instant.
    static void GetProperties(Music& g, Properties& p, MusicNode n)
    {
      //Go to the top node of the part before retrieving the properties.
      if(!g.RaiseToTopPart(n))
        return;
      
      if(!undefined(n->Get(mica::SystemBreak)))
        p.Add() = mica::SystemBreak;
      if(!undefined(n->Get(mica::OptionalBreak)))
        p.Add() = mica::OptionalBreak;
      if(!undefined(n->Get(mica::RepeatingInstant)))
        p.Add() = mica::RepeatingInstant;
      if(!undefined(n->Get(mica::AbsorbedInstant)))
        p.Add() = mica::AbsorbedInstant;
    }
    
    ///Clear all properties on the instant.
    static void ClearProperties(Music& g, MusicNode n)
    {
      //Go to the top node of the part before retrieving the properties.
      if(!g.RaiseToTopPart(n))
        return;
      n->Set(mica::SystemBreak) = mica::Undefined;
      n->Set(mica::OptionalBreak) = mica::Undefined;
      n->Set(mica::RepeatingInstant) = mica::Undefined;
      n->Set(mica::AbsorbedInstant) = mica::Undefined;
    }
    
    ///Clears all instant properties on the graph.
    static void ClearProperties(Music& g)
    {
      MusicNode n = g.Root();
      while(n)
      {
        ClearProperties(g, n);
        n = g.Next(n, MusicLabel::Partwise());
      }
    }
    
    ///Set properties on the instant.
    static void SetProperties(Music& g, const Properties& p, MusicNode n)
    {
      //Go to the top node of the part before retrieving the properties.
      if(g.RaiseToTopPart(n))
        return;

      //Clear properties first.
      ClearProperties(g, n);
      
      if(p.IsSystemBreak())
        n->Set(mica::SystemBreak) = mica::SystemBreak;
      if(p.IsOptionalBreak())
        n->Set(mica::OptionalBreak) = mica::OptionalBreak;
      if(p.IsRepeatingInstant())
        n->Set(mica::RepeatingInstant) = mica::RepeatingInstant;
      if(p.IsAbsorbedInstant())
        n->Set(mica::AbsorbedInstant) = mica::AbsorbedInstant;
    }
    
    ///Prints the properties for the instant.
    static void PrintProperties(Music& g, MusicNode n)
    {
      Properties p;
      GetProperties(g, p, n);
      prim::c >> p.ToString();
    }
  };
}}
#endif
