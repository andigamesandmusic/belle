#define BELLE_COMPILE_INLINE
#include "belle.h"

int main(int ArgumentCount, const char** ArgumentData)
{
  using namespace belle;
  AutoRelease<Console> ReleasePool;

  String Input;
  {
    List<String> Arguments;
    for(count i = 0; i < ArgumentCount; i++)
      Arguments.Add() = ArgumentData[i];

    if(Arguments.n() == 1)
      C::Out() >> "Usage: iterate <score-file>";
    if(Arguments.n() != 2)
      return 1;

    String Filename = Arguments[1], FileData;
    File::Read(Filename, FileData);
    if(not FileData)
    {
      C::Out() >> "No data in file " << Filename;
      return 1;
    }
    Input = ConvertToXML(FileData);
  }

  Pointer<Music> M;
  M.New()->ImportXML(Input);

  Pointer<Geometry> G;
  G.New()->Parse(*M);

  MusicIterator Iterator;
  String A, B;
  {
    {
      A >> "Iterating through each island...";
      Iterator.Start(M.Const());
      while(Iterator.NextIsland())
        A >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Island: " << Iterator.Island().Raw();
    }
    {
      A >> "Iterating through each barline...";
      Iterator.Start(M.Const());
      while(Iterator.NextBarline())
        A >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Barline: " << Iterator.Barline()->Get(mica::Value);
    }
    {
      A >> "Iterating through each clef...";
      Iterator.Start(M.Const());
      while(Iterator.NextClef())
        A >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Clef: " << Iterator.Clef()->Get(mica::Value);
    }
    {
      A >> "Iterating through each key signature...";
      Iterator.Start(M.Const());
      while(Iterator.NextKeySignature())
        A >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Key Signature: " << Iterator.KeySignature()->Get(mica::Value);
    }
    {
      A >> "Iterating through each time signature...";
      Iterator.Start(M.Const());
      while(Iterator.NextTimeSignature())
        A >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Time Signature: " << Iterator.TimeSignature()->Get(mica::Value);
    }
    {
      A >> "Iterating through each chord...";
      Iterator.Start(M.Const());
      while(Iterator.NextChord())
        A >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Chord: " << Iterator.Chord()->Get(mica::NoteValue);
    }
    {
      A >> "Iterating through each note...";
      Iterator.Start(M.Const());
      while(Iterator.NextNote())
        A >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Note: " << Iterator.Note()->Get(mica::Value);
    }
  }
  {
    {
      B >> "Iterating through each island...";
      Iterator.Start(G.Const());
      while(Iterator.NextIsland())
        B >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Island: " << Iterator.Island().Raw();
    }
    {
      B >> "Iterating through each barline...";
      Iterator.Start(G.Const());
      while(Iterator.NextBarline())
        B >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Barline: " << Iterator.Barline()->Get(mica::Value);
    }
    {
      B >> "Iterating through each clef...";
      Iterator.Start(G.Const());
      while(Iterator.NextClef())
        B >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Clef: " << Iterator.Clef()->Get(mica::Value);
    }
    {
      B >> "Iterating through each key signature...";
      Iterator.Start(G.Const());
      while(Iterator.NextKeySignature())
        B >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Key Signature: " << Iterator.KeySignature()->Get(mica::Value);
    }
    {
      B >> "Iterating through each time signature...";
      Iterator.Start(G.Const());
      while(Iterator.NextTimeSignature())
        B >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Time Signature: " << Iterator.TimeSignature()->Get(mica::Value);
    }
    {
      B >> "Iterating through each chord...";
      Iterator.Start(G.Const());
      while(Iterator.NextChord())
        B >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Chord: " << Iterator.Chord()->Get(mica::NoteValue);
    }
    {
      B >> "Iterating through each note...";
      Iterator.Start(G.Const());
      while(Iterator.NextNote())
        B >> "P: " << Iterator.Part() << " I: " << Iterator.Instant() <<
          " Note: " << Iterator.Note()->Get(mica::Value);
    }
  }

  if(A != B)
  {
    C::Out() >> "===Graph-based iteration===";
    C::Out() >> A;
    C::Out() >> "===Geometry-based iteration===";
    C::Out() >> B;
    C::Error() >>
      "Error: graph-based iteration did not match geometry-based iteration";
    File::Write("/tmp/a.txt", A);
    File::Write("/tmp/b.txt", B);
    String In, Out, Error;
    Shell::PipeInOut("/usr/bin/diff", In, Out, Error,
      "/tmp/a.txt", "/tmp/b.txt");
    C::Error() >> Out;
    return 1;
  }
  else
    C::Out() >> A;

  return 0;
}
