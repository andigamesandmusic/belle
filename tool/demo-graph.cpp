#define BELLE_COMPILE_INLINE
#include "belle.h"

int main(int ArgumentCount, const char** ArgumentData)
{
  using namespace belle;
  AutoRelease<Console> ReleasePool;

  List<String> Arguments;
  for(count i = 0; i < ArgumentCount; i++)
    Arguments.Add() = ArgumentData[i];

  if(Arguments.n() == 1)
    C::Out() >>
      "Usage: graph <score-file> # Exports TGF and PDFs of a music graph";
  if(Arguments.n() != 2)
    return 0;

  String Filename = Arguments[1], FileData, Input;
  File::Read(Filename, FileData);
  if(not FileData)
  {
    C::Out() >> "No data in file " << Filename;
    return 0;
  }
  Filename.EraseEnding(".txt");
  Filename.EraseEnding(".xml");
  Input = ConvertToXML(FileData);

  Pointer<Music> g;
  g.New()->ImportXML(Input);
  Pointer<Music> h = g->GeometrySubgraph();

  C::Out() >> String("Writing ") << Filename << ".tgf";
  File::Write(Filename + ".tgf", g->ExportTGF());
  C::Out() >> String("Writing ") << Filename << "-geometry.tgf";
  File::Write(Filename + "-geometry.tgf", h->ExportTGF());
  C::Out() >> String("Writing ") << Filename << ".pdf";
  Utility::OpenGraphVisualization(*g, Filename + ".pdf");
  C::Out() >> String("Writing ") << Filename << "-geometry.pdf";
  Utility::OpenGraphVisualization(*h, Filename + "-geometry.pdf");
  return 0;
}
