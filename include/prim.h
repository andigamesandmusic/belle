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

  To use:

  1) In one and only one source file:
     #define PRIM_COMPILE_INLINE
     #include "prim.h"

  2) In all other files that use prim:
     #include "prim.h"

  ==============================================================================
*/

#ifndef PRIM_LIBRARY
#define PRIM_LIBRARY

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-platform.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-configuration.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-includes.h"
#include "prim-language.h"
#include "prim-singleton.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-environment.h"
#include "prim-types.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-counter.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-boolean-pointer.h"
#include "prim-constants.h"
#include "prim-endian.h"
#include "prim-math.h"
#include "prim-memory.h"
#include "prim-mod-thread.h"
#include "prim-mod-timer.h"
#include "prim-pointer.h"
#include "prim-random.h"
#include "prim-unicode.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-array.h"
#include "prim-complex.h"
#include "prim-list.h"
#include "prim-mod-tinyxml.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-matrix.h"
#include "prim-mod-fft.h"
#include "prim-sortable.h"
#include "prim-tree.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-string.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-console.h"
#include "prim-encoding.h"
#include "prim-file.h"
#include "prim-md5.h"
#include "prim-planar.h"
#include "prim-rational.h"
#include "prim-table.h"
#include "prim-time.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-aes.h"
#include "prim-aiff.h"
#include "prim-mod-memory-map.h"
#include "prim-uuid.h"
#include "prim-value.h"
#include "prim-xml.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-arguments.h"
#include "prim-graph-t.h"
#include "prim-histogram.h"
#include "prim-json.h"
#include "prim-mod-midi.h"
#include "prim-mod-queue.h"
#include "prim-serialization.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-mod-javascript.h"
#include "prim-mod-shell.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#include "prim-mod-directory.h"
#include "prim-mod-python.h"

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . //

#endif
