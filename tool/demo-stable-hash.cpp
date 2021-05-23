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
      C::Out() >> "Usage: stable-hash <score-file>";
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
  C::Out() >> "Prehash is:" >> MusicIterator::StableHash::Prehash(M);
  C::Out() >> "Hash is:           " << MusicIterator::StableHash::Hash(M);
  Pointer<Music> R;
  R.New()->ImportXML(M->ExportXML());
  C::Out() >> "Roundtrip hash is: " << MusicIterator::StableHash::Hash(R);
  return 0;
}
