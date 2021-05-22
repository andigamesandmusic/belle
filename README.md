# Belle

*Belle, Bonne, Sage,* originally a [stunning piece of *ars subtilior*](http://www.google.com/images?q=belle+bonne+sage+baude+cordier) from the 14th century, and now just *Belle* for short, is the &ldquo;beautiful, good, wise&rdquo; vector-graphics library for music notation written by William Andrew Burnson. The primary purpose of Belle is to engrave a graph representation of sheet music and render it to an output device such as a PDF or a screen. As Belle is a cross-platform software library, it does not have a frontend user interface. However, Belle has been used for many interactive applications: it is the sheet music renderer in [MusicPal](https://itunes.apple.com/us/app/musicpal-snap-hear-play-learn-music/id976261614?mt=8) and [Harmonia](http://illiacsoftware.com).

Belle has been in development now for over a decade. It has been rewritten and refactored multiple times, each iteration progressing a little bit further. (It may yet be rewritten again.) Belle is a strongly independent library: all of the data structures and algorithmic primitives are implemented from whole cloth. The code is written in an expressive subset of C++ that values literacy and consistency.

Writing a sheet music engraver is very challenging, and Belle is nowhere near finished. The long-term goal of the Belle project is not merely to produce another sheet music engraver, but to find the *best possible means by which to engrave music on a computer*. Here are some examples of ways in which Belle is pioneering towards that end:

* Belle is cross-platform and device-agnostic. A music engraver should be able to be used not just for printing sheet music, but interactively for music education, entertainment, publishing, and any other place where music engraving is needed.
* Belle has introduced a graph representation of sheet music that is faithful to its underlying form.
* Belle has introduced the idea of defined musical concepts that have relationships to each other. For example, that C to D is a whole-step is an inherent property of music that can be encoded in data rather than calculated every time. Each musical abstraction is defined in a global web of concepts.
* Belle generalizes where possible and avoids reductive assumptions. By typesetting from a graph representation of sheet music, Belle achieves notations that other engravers struggle with. As a simple example, Belle can typeset complex tuplets that cross barlines.

## Features

Here are some highlights of what Belle can do today:

* Typesets many basic music notation primitives on multiple staves and systems
* Supports [SMuFL](http://www.smufl.org) fonts (e.g. Bravura) for music notation symbols
* Renders basic Unicode text with kerning from imported fonts
* [Spring spaces](https://quod.lib.umich.edu/i/icmc/bbp2372.1995.032/--new-algorithm-for-horizontal-spacing-of-printed-music?view=image) and wraps long systems
* Supports basic MusicXML import
* Outputs to many useful devices: PDF, SVG, Core Graphics, [JUCE](https://www.juce.com), MIDI
* Renders some complex notation such as dense chords, multilevel tuplets, arithmetic time signatures, and arbitrary voices
* Strives to conform to typesetting guidelines set forth in [Behind Bars](http://a.co/1RkHUi7)
* Uses an easy-to-understand and expressive utility library called *prim*
* Defines music concepts as data via an internal library called *MICA* (Music Information Concept Archive)
* Generates an extensive test suite specimen that renders to PDF

---
### Example: Bach Invention in A-minor
![belle-1 0 0-bach-invention](https://user-images.githubusercontent.com/1053998/28297545-62f65b24-6b23-11e7-9f90-10a42f3ea42f.png)
### Example: Test Suite Specimen
![belle-1 0 0](https://user-images.githubusercontent.com/1053998/28297147-2c02ac0a-6b21-11e7-8f3b-de8bcf42eed5.gif)

## Layout

Belle uses a clean, flat folder structure that is easy to navigate.

* `Makefile` — Makefile for Mac and Linux
* `README.md` — this README
* `bin` — output folder created for utilities and demo programs
* `definitions` — MICA music concept definitions
* `include` — the library source
  * `include/belle*.h` — belle source files
  * `include/mica.h` — generated MICA source file
  * `include/prim*.h` — prim source files
  * `include/resources.h` — generated binary resources file
* `resources` — resources and example scores
* `templates` — MICA programming language API templates
* `tool` — source code for utilities and demo programs

## Building
### Mac and Linux
Assuming you have a C++ compiler, run:

```
make # requires an internet connection to download FreeType and fonts
```

To execute the tests including generating the test suite specimen, run:

```
make test
```

To build all of the demos in the `tool` directory, run:

```
make demo
```

To learn more about the engraver's options run it without any arguments:

```
bin/engrave
```

For example to render the Bach Invention MusicXML example to PDF with wrapping enabled, run:

```
bin/engrave resources/bach-invention.xml --wrap
```

To convert the MusicXML file to the native graph format, run:

```
bin/engrave resources/bach-invention.xml --export
```

See the demos in the `tool` directory for more examples. You can create your own `demo-*.cpp` and build it using `make demo`.

### Windows

While there are not presently Visual Studio project files for Belle, it does build nicely on Windows with a little extra work. If you have Cygwin, then you can follow the same steps for Mac and Linux. For Visual Studio, you will first need to generate the `include/mica.h` and `include/resources.h` files (for example by running `make` on a Mac or Linux system). Then it is just a matter of adding all the files from the `include` folder and the source program you want to build into a new project and setting the appropriate header search path.

## MICA
Belle uses an internal library called MICA (Music Information Concept Archive) to store content pertaining to music concepts. MICA defines music concepts as language-agnostic labeled identifiers. The labels are things like `en:C4` or `en:DynamicMarkForte` (the pitch C4 or the dynamic *forte* expressed in English) and the identifiers are UUIDs. MICA allows a given identifier to have more than one label. For example, both `en:EighthNote` and `en-GB:Quaver` refer to the same underlying concept; they simply have different names. Music concepts can relate to each other through one of two built-in data structures *sequences* and *maps*

A concept may optionally contain a sequence. For example, the concept `en:A` contains the letter sequence `en:A en:B en:C en:D en:E en:F en:G`. Through the MICA API, one can thus ask &ldquo;what is the 3rd letter after `en:A`&rdquo; and get back `en:D`.

Maps are global mappings that define a function on an unordered set of concepts. For example, the map `1 en:TrebleClef en:C5` means that result of the staff position 1 (the position just above the middle line) and `en:TrebleClef` is `en:C5`. In other words the staff position 1 on a staff containing a treble clef yields the pitch C5. The MICA API does not use math to calculate this as these relationships are defined in data. The MICA API does however provide some helper functions to help calculate more complex relationships like intervals, but these calculations are themselves just a series of MICA lookups.

MICA uses version 3 UUIDs that are derived from the MD5 of a namespace identifier and a text string, which is often just the English label of the concept. MICA also stores ratios in the same 128-bit UUID space using a big-endian 64-bit signed numerator followed by a big-endian 64-bit signed but always positive denominator. Due to this encoding, a ratio can always be distinguished from a version 3 UUID. The special concept `en:Undefined` is equivalent to `0/0`. The concept `en:Undefined` is returned whenever a MICA operation has no result.

## The Graph Representation of Music

### Overview
Sheet music is represented in Belle using a **graph**.
A [graph](https://en.wikipedia.org/wiki/Graph_(mathematics))
is formally a list of nodes and a list of edges connecting the nodes. Graphs may take many forms. Belle uses a [edge- and node-labeled](http://en.wikipedia.org/wiki/Graph_labeling) [multidigraph](https://en.wikipedia.org/wiki/Multigraph). A multidigraph has directed edges and multiple edges between nodes are permitted. The labels are key-value pairs where the key or value can be a MICA music concept (e.g. `en:Value->en:C4`), ratio (e.g. `en:NoteValue->1/4`), or string (e.g. `"Text"->"Andante"`). These graphs may be thought of as having **structure** (relationships between nodes **content** (information, i.e. labels on the nodes and edges). Since both the structure and content are generalizable, graphs are ideal for representing music notation.

Adding nodes to a graph involves creating the nodes and connecting the nodes by edges. Accessing the graph structure and content is achieved by traversing the graph through its nodes and edges. Graphs may have multiple edges departing from a node, so **filters** are used to decide which paths to take during traversal. A filter is a label with partial information specified, and an edge is followed if the filter matches the edge's label. In Belle, a filter match occurs when the edge label contains all of the keys of the filter. The edge label may contain more information than the filter and still match so long
as each item in the filter is found in the edge label.

The main single-edge traversal functions are **next** and **previous**. These look for a single filter match and return the corresponding node if found. If more than one node
matches the filter, then no node is returned and a multi-node traversal function
must be used.

The two main types of multi-node traversal are **series** and **children**. Series finds a path through the graph matching the filter. Intuitively it is like following *next* and
building a list of the nodes it finds. However, it first backtraces with
*previous*, so no matter which node in the path you call *series* on, it will
return the same path. The *children* function builds a list of all the nodes that match the
filter going in the forwards direction.

Graphs do not store any notion of edge order. Therefore,
though *children* will always return the same objects given the same input
arguments and graph, the order of the objects in the returned array is
necessarily indeterminate and can vary.

### Music Representation

The graphs described above are extremely general data structures. In order to
use them for music, a music-specific structure and labeling paradigm is built
on top of the graph. In order to conveniently represent sheet music, we first
make a few assumptions.

1. The music is represented as a single uninterrupted system instead of several
systems. This assumption helps the representation to be independent of layout
concerns (page dimensions, size of systems, systems per page, measures per
system). The layout is addressed at a higher level.

2. Music has one or more staves; these can be thought of as **parts**.
Moving in the horizontal direction, music has multiple **instants**. Instants
are visually simultaneous objects in the score. Visually simultaneous means that
if you were to stretch the score out horizontally, the visually simultaneous
items would stay together along the same basic vertical line down the score.

Using these two basic premises, we have a notion of objects that can be located
at a specific part and instant. These objects are called **islands** because
if you were to stretch the score out in both the horizontal direction (e.g.
add a lot of space between chords) and the vertical direction (e.g. add a lot of
space between staves) and remove the staff lines you would see clumps like
chords with lots of empty space around them.

Islands are connected by **partwise** and **instant-wise** edges. The structure of the islands in the graph is referred to as the **geometry**
of the graph. In order to layout the music, the graph geometry must be parsed.
Parsing the geometry involves solving for monotonically increasing part and
instant indices. Finding the part indices involves solving a
**transitive closure**. Given relationships between staves like staff A is above staff B and staff
B is above staff C, the order is A > B > C. Finding the instant indices involves
solving the **leading-edge problem**, which means that if you were to sweep
the score from left to right, draw a succession of lines down each instant such
that none of the lines cross.

#### Islands, Tokens, and Notes
Islands contain information about musical objects. An island may contain one or
more **tokens**, where a token may represent a **chord**, **clef**,
**time signature**, **key signature**, or **barline**. Usually, islands
contain one token, but may contain multiple tokens in the case of a
multiple-voice chord in the same staff. Islands connect to tokens by way of
token links. Chord tokens have a membership of one or more **note nodes** connected from
the token to the note by note links.

#### Engraving and Layout
Notating sheet music in Belle happens in two high-level steps:

1. Engraving (typesetting) the islands in an abstract space
2. Laying out (positioning) the islands in a physical page space

Each island node contains a link to a **stamp** that stores the
vector-graphics path engraving of the island. The stamp may also contain
information accumulated during the engraving and layout process.

Each island passes through an **island engraver**, designed for the token
types it contains (chord, clef, etc.). The purpose of the island engraver is to
take the abstract information in the island and engrave a stamp from it.

Island engravers may have **stateless** and **stateful** parts. For
example, a barline engraver is generally stateless; the drawing of the barline
does not depend on any previous information in the graph. On the other hand, a
chord is very stateful; it depends on the clef for its positioning, previously
appearing accidentals that may silence the display of accidentals on the chord,
and so on.

#### Rendering Abstractions
Belle has a double-abstraction rendering layer consisting of a notion of a
document, called a **portfolio** containing multiple pages called **canvases** which is rendered to a vector-graphics
target called a **painter**. The abstraction allows the portfolio and
canvas to be completely independent of the kind of painter, and vice versa and
necessitates that the portfolio-canvas layer communicate to the painter by means
of an abstract bridge.

In practice, the **portfolio** is subclassed as a **score**, and the **canvas** is
subclassed as a **page**. The built-in painters are PDF and SVG. The canvas's paint method callback contains the
specific user code to make calls to the painter class. The painting callbacks
are generated by the painters automatically by creating the painter for the score.

#### Affine Space
In vector graphics, affine space generalizes the notion of physical units into
a transformable two-dimensional space. If you think of a plane on which graphics
are to be drawn, the position of the origin, size of the unit square, and
directions of the x and y axes is sufficient to describe the affine space. In
practice, the x and y axes are kept perpendicular to each other (without skew).
Therefore the main transformations in affine space are translation
(positioning), scale, and rotation. The transformations are not
commutative: moving up by one and scaling by two produces a different
affine space than scaling by two and moving by one.

In Belle, vector graphics are rendered to a canvas whose units are in
inches with the origin at the bottom-left. Belle does not use the conventional
origin at top-left with inverted y-coordinate since this makes many vector operations
assymetric.

This initial space (bottom-left origin in inches) is referred to as
**page space**. In music, however, it is more convenient to work relative
to the **space-height** of the staff. Stamps are therefore engraved in
**stamp space**, in which the middle line (or space) of the staff is
considered the vertical origin and the horizontal origin is typically the
logical horizontal center of the island (e.g. the horizontal center of a note).

Layout occurs in **system space**, which bridges together page space and
stamp space. Given that a staff is positioned on a page at some location (in
inches) and a stamp is positioned relative to the far left point on the middle
line of the top staff (in spaces), the position of the stamp on the page may be
calculated.

When Belle renders to a screen painter, the painter must transform the page
space back into **screen space**. This typically involves moving the origin
from the bottom-left of the page to the top-left of the screen, inverting the
y-axis through an assymetric scale (1, -1) operation, and scaling the page by
a factor to fit the screen. Additional transformations may be used to pan and
zoom the page on the screen in the context of user interfaces.

#### Systems and Graphs
In Belle, a system is represented by a single music graph called **music**. Systems
are placed on the page independently of each other. A system has a width, which
is the distance in page units between the x-coordinates of the first and last
island. The system also has a nominal staff-size expressed by the page unit
height of a staff space.

When a system is engraved, it is engraved in system space. The origin of system
space is always relative to the top-left island (the root of the music graph)
subtracted by its staff vertical offset, and the scale is one unit to the height
of a staff space for a staff of nominal size (a regular staff, not a small or
ossia staff).

## Acknowledgments

Belle would not have been possible without the support of these excellent people:

* [Robert Taub](http://www.roberttaubpianist.com) is a world-renowned concert pianist known for his performances and [numerous recordings of Milton Babbitt, Beethoven, and others](http://www.roberttaubpianist.com/works-commissioned/), his [many publications](http://www.roberttaubpianist.com/publications/), including the Schirmer Performance Edition of the Beethoven Piano Sonatas, and as the founder of the music technology startup MuseAmi. Bob and I crossed paths in the development of [MusicPal](https://itunes.apple.com/us/app/musicpal-snap-hear-play-learn-music/id976261614?mt=8), a mobile app that can turn a picture of sheet music into interactive sheet music. While at MuseAmi, Bob gave me the opportunity to take Belle from prototype to production. **A huge debt of gratitude** is owed to Bob for agreeing to open source two-and-a-half years worth of proprietary improvements to Belle!
* [Lippold Haken](https://www.ece.illinois.edu/directory/profile/l-haken) is the creator of the [LIME](http://www.cerlsoundgroup.org/cgi-bin/Lime/Windows.html) music notation and Braille music notation program, creator of the [Continuum Fingerboard](https://en.wikipedia.org/wiki/Continuum_Fingerboard), and Lecturer at University of Illinois. Dr. Haken was one of my doctoral project advisors on Belle and provided many valuable insights into music notation and representation.
* [Heinrich Taube](https://music.illinois.edu/faculty/heinrich-taube) is the creator of the algorithmic music environment [Common Music](http://commonmusic.sourceforge.net), Professor of Music Composition at University of Illinois, and CEO of Illiac Software Inc. While I was in grad school, he sponsored Belle through multiple research grants involving the development of the [Harmonia](http://illiacsoftware.com) music theory platform that can automagically analyze and grade four-part harmony assignments. Rick was also the chair of my doctoral committee.
* [Barry Hannigan](https://www.bucknell.edu/majors-and-minors/music/faculty-and-staff/barry-hannigan.html) was my piano teacher while I was a music student at Bucknell University. Barry first introduced me to the [eye music of George Crumb](http://www.google.com/images?q=george+crumb+score). After performing *Makrokosmos: Vol. I*, I began to think about how these pieces could be typeset with a program. To see whether it could be done, I created the first version of Belle to typeset my piece [Bike Ride](http://bi.ke-ri.de) (dedicated to Barry and his wife Mary who are avid cyclists).

## License

Belle is distributed under a permissive 2-clause BSD license.

```
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
```

### Font Licenses

Release distributions of Belle contain embedded versions of the [Bravura](https://github.com/steinbergmedia/bravura) and [Merriweather](https://github.com/EbenSorkin/Merriweather) fonts in `include/resources.h` which are each licensed under the SIL Open Font License 1.1.

```
Copyright © 2019, Steinberg Media Technologies GmbH (http://www.steinberg.net/),
with Reserved Font Name "Bravura".
```

```
Copyright 2016 The Merriweather Project Authors (https://github.com/EbenSorkin/Merriweather),
with Reserved Font Name "Merriweather".
```

```
This Font Software is licensed under the SIL Open Font License, Version 1.1.
This license is copied below, and is also available with a FAQ at:
http://scripts.sil.org/OFL

-----------------------------------------------------------
SIL OPEN FONT LICENSE Version 1.1 - 26 February 2007
-----------------------------------------------------------

PREAMBLE
The goals of the Open Font License (OFL) are to stimulate worldwide
development of collaborative font projects, to support the font creation
efforts of academic and linguistic communities, and to provide a free and
open framework in which fonts may be shared and improved in partnership
with others.

The OFL allows the licensed fonts to be used, studied, modified and
redistributed freely as long as they are not sold by themselves. The
fonts, including any derivative works, can be bundled, embedded,
redistributed and/or sold with any software provided that any reserved
names are not used by derivative works. The fonts and derivatives,
however, cannot be released under any other type of license. The
requirement for fonts to remain under this license does not apply
to any document created using the fonts or their derivatives.

DEFINITIONS
"Font Software" refers to the set of files released by the Copyright
Holder(s) under this license and clearly marked as such. This may
include source files, build scripts and documentation.

"Reserved Font Name" refers to any names specified as such after the
copyright statement(s).

"Original Version" refers to the collection of Font Software components as
distributed by the Copyright Holder(s).

"Modified Version" refers to any derivative made by adding to, deleting,
or substituting -- in part or in whole -- any of the components of the
Original Version, by changing formats or by porting the Font Software to a
new environment.

"Author" refers to any designer, engineer, programmer, technical
writer or other person who contributed to the Font Software.

PERMISSION AND CONDITIONS
Permission is hereby granted, free of charge, to any person obtaining
a copy of the Font Software, to use, study, copy, merge, embed, modify,
redistribute, and sell modified and unmodified copies of the Font
Software, subject to the following conditions:

1) Neither the Font Software nor any of its individual components,
in Original or Modified Versions, may be sold by itself.

2) Original or Modified Versions of the Font Software may be bundled,
redistributed and/or sold with any software, provided that each copy
contains the above copyright notice and this license. These can be
included either as stand-alone text files, human-readable headers or
in the appropriate machine-readable metadata fields within text or
binary files as long as those fields can be easily viewed by the user.

3) No Modified Version of the Font Software may use the Reserved Font
Name(s) unless explicit written permission is granted by the corresponding
Copyright Holder. This restriction only applies to the primary font name as
presented to the users.

4) The name(s) of the Copyright Holder(s) or the Author(s) of the Font
Software shall not be used to promote, endorse or advertise any
Modified Version, except to acknowledge the contribution(s) of the
Copyright Holder(s) and the Author(s) or with their explicit written
permission.

5) The Font Software, modified or unmodified, in part or in whole,
must be distributed entirely under this license, and must not be
distributed under any other license. The requirement for fonts to
remain under this license does not apply to any document created
using the Font Software.

TERMINATION
This license becomes null and void if any of the above conditions are
not met.

DISCLAIMER
THE FONT SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF COPYRIGHT, PATENT, TRADEMARK, OR OTHER RIGHT. IN NO EVENT SHALL THE
COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
INCLUDING ANY GENERAL, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF THE USE OR INABILITY TO USE THE FONT SOFTWARE OR FROM
OTHER DEALINGS IN THE FONT SOFTWARE.
```
