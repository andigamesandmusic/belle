Welcome to the repository of Belle, Bonne, Sage, an experimental music engraver in C++. Currently the project is inactive since June 2012. At this time, the author forked the repository to work on a proprietary project called MusicPal (<http://museami.com/musicpal>). Meanwhile, the project has also been forked by Illiac Software (<http://illiacsoftware.com>) to develop Harmonia.

The goals of Belle, Bonne, Sage are described here: <http://bellebonnesage.sourceforge.net/> ([backup link](https://web.archive.org/web/20140110014642/http://bellebonnesage.sourceforge.net/))

**For newcomers**, I would say this: if you are interested in playing around with music-related vector graphics, then Belle is a great way to do that. If you are looking for a full on engraver, you'll need to keep looking as the current typesetter for open source Belle was just one in a long line of experimental and incomplete typesetters.

If all the above doesn't deter you, here is how you can get started with this project (Mac OS X version):

1) Clone the repo

`git clone git://github.com/burnson/Belle.git`

2) Change directory to Belle

`cd Belle`

3) Run the CI script to build and run the examples

`Scripts/CI`

4) Open the generated example PDFs and have a look

`open *.pdf`

**NOTE**: the typesetter example makes use of the Belle proto-engraver. This engraver can not do anything meaningful in terms of printing music; rather, it was only created to determine how graphs may be useful as a data structure for music. As such, the following code directories are considered deprecated:

 * `Belle/Source/Graph`
 * `Belle/Source/Modern`
