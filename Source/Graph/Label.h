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

#ifndef BELLE_GRAPH_LABEL_H
#define BELLE_GRAPH_LABEL_H

#include "Typesetting.h"

namespace BELLE_NAMESPACE { namespace graph
{
  //Class to store music concepts and custom strings
  class MusicLabel
  {
    ///Stores the concepts.
    prim::Table<mica::Concept> Concepts;
    
    ///Stores the strings.
    prim::Table<prim::String> Strings;
  
    public:
    
    ///Stores information related to Typesetting
    prim::Pointer<TypesettingInfo> Typesetting;
  
    private:
    
    ///Converts a string like "Foo Bar" to "FooBar" (no case check though).
    static prim::String ToCamelCase(prim::String s)
    {
      s.Replace(" ", "");
      return s;
    }
  
    /**Converts a string like "FooBar" to "Foo Bar" (any sequence of lowercase
    followed by uppercase has a space inserted between).*/
    static prim::String ToSpaceSeparated(prim::String s)
    {
      for(prim::count i = 0; i < s.n() - 1; i++)
      {
        prim::ascii a = (prim::ascii)s[i];
        prim::ascii b = (prim::ascii)s[i + 1];
        if((a >= 'a' && a <= 'z') && (b >= 'A' && b <= 'Z'))
          s.Insert(" ", i++ + 1);
      }
      return s;
    }

    public:

    ///Const key-value lookup
    const mica::Concept& Get(const mica::Concept& Key) const
    {
      return Concepts[Key];
    }
  
    ///Mutable key-value lookup
    mica::Concept& Set(const mica::Concept& Key) {return Concepts[Key];}

    ///Const key-value lookup
    const prim::String& Get(const prim::ascii* Key) const {return Strings[Key];}
  
    ///Mutable key-value lookup
    prim::String& Set(const prim::ascii* Key) {return Strings[Key];}

    ///Attribute set for XML deserialization.
    void Set(const prim::ascii* Key, const prim::ascii* Value)
    {
      prim::String k = Key, v = Value;
      if(k.StartsWith("data-"))
      {
        k.Erase(0, 4);
        Strings[k] = v;
      }
      else
        Concepts[mica::Concept(ToSpaceSeparated(k).Merge())] =
          mica::Concept(Value);
    }
  
    ///Returns number of attributes for the purpose of XML serialization.
    prim::count Attributes() const
    {
      return Concepts.n() + Strings.n();
    }
  
    ///Returns attribute key for the purpose of XML serialization.
    prim::String AttributeKey(prim::count i) const
    {
      if(i < Concepts.n())
        return ToCamelCase(Concepts.ith(i).Key);
      else
      {
        prim::String s = "data-";
        s << Strings.ith(i - Concepts.n()).Key;
        return s;
      }
    }
  
    ///Returns attribute value for the purpose of XML serialization.
    prim::String AttributeValue(prim::count i) const
    {
      if(i < Concepts.n())
        return Concepts.ith(i).Value;
      else
        return Strings.ith(i - Concepts.n()).Value;
    }

    /**For equivalence, the label is only checked against the items in filter.
    This differs from simply checking for the equivalence of the tables.*/
    bool EdgeEquivalent(const MusicLabel& Filter) const
    {
      //See if the filter concepts match by value.
      for(prim::count i = 0, n = Filter.Concepts.n(); i < n; i++)
      {
        mica::Concept k = Filter.Concepts.ith(i).Key;
        if(Concepts[k] != Filter.Concepts[k])
          return false;
      }
      
      //See if the filter strings match by value.
      for(prim::count i = 0, n = Filter.Strings.n(); i < n; i++)
      {
        prim::String k = Filter.Strings.ith(i).Key;
        if(Strings[k] != Filter.Strings[k])
          return false;
      }
      
      return true;
    }
  
    ///String conversion
    operator prim::String() const
    {
      prim::String s;
      for(prim::count i = 0, n = Attributes(); i < n; i++)
      {
        if(i) s << " ";
        s << AttributeKey(i) << ":" << AttributeValue(i);
      }
      return s;
    }
    
    //-------------//
    //Common Labels//
    //-------------//
    
    static MusicLabel Instantwise()
    {
      MusicLabel L;
      L.Set(mica::Type) = mica::Instantwise;
      return L;
    }
    
    static MusicLabel Partwise()
    {
      MusicLabel L;
      L.Set(mica::Type) = mica::Partwise;
      return L;
    }

    static MusicLabel Token()
    {
      MusicLabel L;
      L.Set(mica::Type) = mica::Token;
      return L;
    }

    static MusicLabel Note()
    {
      MusicLabel L;
      L.Set(mica::Type) = mica::Note;
      return L;
    }
    
    static MusicLabel Continuity()
    {
      MusicLabel L;
      L.Set(mica::Type) = mica::Continuity;
      return L;
    }
  };
  
  //Typedefs for music graphs, nodes, and edges.
  typedef prim::Pointer<prim::GraphT<MusicLabel>::Object> MusicEdge;
  typedef prim::Pointer<prim::GraphT<MusicLabel>::Object> MusicNode;
  typedef prim::Pointer<const prim::GraphT<MusicLabel>::Object> ConstMusicEdge;
  typedef prim::Pointer<const prim::GraphT<MusicLabel>::Object> ConstMusicNode;
}}
#endif
