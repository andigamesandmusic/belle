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

#ifndef BELLE_GRAPH_UTILITY_H
#define BELLE_GRAPH_UTILITY_H

#include "Music.h"

namespace BELLE_NAMESPACE { namespace graph
{
  class Utility
  {
    public:
    
    /**Utility function to help visualize graphs. It uses the Graphviz DOT
    utility to convert a DOT representation of the graph into a PNG image.*/
    static void OpenGraphVisualization(const Music& m,
      prim::String TempFile = "/tmp/prim_test.png")
    {
    #ifdef PRIM_WITH_SHELL
      prim::String In, Out, Error;
      //Pipe a DOT file to dot and get the result back as PNG.
      prim::Shell::PipeInOut(prim::Shell::GetProcessOnPath("dot"),
        m.ExportDOT(), Out, Error, "-Tpng");

      //Write the PNG to the temp file.
      prim::File::Write(TempFile, Out);
  
      //Open the PNG using the default application.
      prim::Shell::PipeInOut(prim::Shell::GetProcessOnPath("open"),
        In, Out, Error, TempFile);
    #else
      prim::c >> "PRIM_WITH_SHELL not enabled. Can not open 'dot'.";
    #endif
    }
  };
}}
#endif
