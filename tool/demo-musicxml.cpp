#define BELLE_COMPILE_INLINE
#include "belle.h"
#include "belle-helper.h"
using namespace belle;

int main(int ArgumentCount, const char** Arguments)
{
  AutoRelease<Console> ReleasePool;

  if(ArgumentCount != 2)
  {
    C::Out() >> "musicxml [file] # Converts MusicXML to Belle at output.xml";
    return 0;
  }

  String Filename = Arguments[1];
  String Data = File::Read(Filename);

  C::Out() >> "Reading " << Filename;
  Array<byte> MusicXMLValidationZip;
  String Result = MusicXMLToXML(Data, MusicXMLValidationZip);

  File::Write("output.xml", Result);
  C::Out() >> "Wrote output.xml";

  if(not Result)
    C::Error() >> "There was an error reading the MusicXML file.";

  return not Result;
}
