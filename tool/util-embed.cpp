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
*/

#define PRIM_WITH_DIRECTORY
#define PRIM_COMPILE_INLINE
#include "prim.h"
using namespace prim;

int main(int ArgumentCount, const char** Arguments)
{
  //Read in the arguments list and check for usage.
  Array<String> Args(Arguments, ArgumentCount);
  if(Args.n() < 3)
    return C::Out() >> "Usage: " << Arguments[0] <<
      " [InDirs ...] [OutName]" << String::Newline;

  //Get the directory of files.
  Array<String> Files;
  for(count i = 1; i < Args.n() - 1; i++)
  {
    Array<String> FilesInDir;
    Directory::Files(Args[i], FilesInDir);
    Files.Append(FilesInDir);
  }

  //Read in all the files.
  C::Out() >> "Reading files...";
  Array<Array<byte> > FileData;
  for(count i = 0; i < Files.n(); i++)
  {
    C::Out() >> "  " << Files[i];
    File::Read(Files[i], FileData.Add());
  }
  C::Out() >> "Processing...";

  //Get the sanitized output name.
  String Name = String(Args.z()).ToIdentifier();

  //Create the preprocessor name.
  String PreprocessorName = Name.ToUpper();

  /*This ID depends on the file list. As long as the file list stays the same,
  the ID stays the same.*/
  String ID = String(UUIDv4(UUIDv4("1a324b65-c752-4a53-8bd3-5035c239b5ce"),
    (String(Files)))).ToUpper();

  //Create the include guard.
  String GuardName;
  GuardName << PreprocessorName << "_" << ID << "_H";
  GuardName = GuardName.ToIdentifier();

  //Create the compile inline define.
  String CompileInlineName;
  CompileInlineName << PreprocessorName << "_COMPILE_INLINE";

  //Write the how-to description.
  String Out;
  Out >> "/*";
  Out++;
  Out >> "This embedded binary resource contains the following files:";
  for(count i = 0; i < Files.n(); i++)
    Out >> "  - " << Files[i].ToFilename();
  Out >> "  ";
  Out >> "Include this file in your main source with:";
  Out >> "#define PRIM_COMPILE_INLINE";
  Out >> "#include \"prim.h\";";
  Out >> "#include \"" << Name << ".h\"";
  Out++;
  Out >> "Or if you are not using prim, then:";
  Out >> "#define " << CompileInlineName;
  Out >> "#include \"" << Name << ".h\"";
  Out++;
  Out >> "And in all other files that need the resources with:";
  Out >> "#include \"" << Name << ".h\"";
  Out++;
  Out >> "Resources may be loaded in prim with:";
  Out >> "Array<byte> x = " << Name << "::Load(\"resource-name\");";
  Out++;
  Out >> "Or:";
  Out >> "Array<byte> x;";
  Out >> Name << "::Load(\"resource-name\", x);";
  Out++;
  Out >> "*/";
  Out++;

  //Write the include guard and declare the loading functions.
  Out >> "//The following identifier is unique to the file list.";
  Out >> "#ifndef " << GuardName >> "#define " << GuardName;
  Out >> "namespace " << Name >> "{";
  Out >> "#ifdef PRIM_LIBRARY";
  Out >> "  ///Loads a resource given its original file name as a string.";
  Out >> "  prim::String Load(const prim::ascii* ResourceName);";
  Out >> "  ";
  Out >> "  ///Loads a resource given its original file name and an "
    "output array.";
  Out >> "  void Load(const prim::ascii* ResourceName, "
    "prim::Array<prim::byte>& Out);";
  Out >> "#endif";

  //Write the declarations.
  for(count i = 0; i < Files.n(); i++)
  {
    String Variable = Files[i].ToFilename().ToIdentifier();
    if(i)
      Out >> "  ";
    else
      Out++;
    Out >> "  //" << Files[i].ToFilename();
    Out >> "  extern const unsigned char* " << Variable << ";";
    Out >> "  extern const int            " << Variable << "_n; // = "
      << FileData[i].n() << ";";
  }
  Out >> "}";
  Out++;

  //Write the definitions.
  Out >> "#if defined(" << CompileInlineName <<
    ") || defined(PRIM_COMPILE_INLINE)";
  for(count i = 0; i < Files.n(); i++)
  {
    //Add a separator between data arrays.
    if(i)
      Out >> "    ";

    String ResourceID = "Resource_";
    ResourceID << i;
    ResourceID << "_";
    ResourceID << ID.Substring(0, 7);

    //Write the data out line by line.
    count Length = FileData[i].n(), Last = Length - 1;
    count j = 0;
    while(j <= Last)
    {
      //Write the line header.
      String CurrentLine;
      if(!j)
      {
        Out >> "  const int Resources::" << Files[i].ToFilename().ToIdentifier()
          << "_n = " << Length << ";";
        CurrentLine << "  const unsigned char ";
        CurrentLine << ResourceID;
        CurrentLine << "[";
        CurrentLine << Length;
        CurrentLine << "] = {";
      }
      else
        CurrentLine << "    ";

      //Build up a single line.
      for(; j <= Last; j++)
      {
        //Use an initial space if not the first in the line.
        String CurrentNumber;

        //Append the value to the current token.
        CurrentNumber << FileData[i][j] << (j == Last ? "};" : ",");

        //If it spills over the limit, then break the line, otherwise append.
        if(CurrentLine.n() + CurrentNumber.n() > 80)
          break;
        else
          CurrentLine << CurrentNumber;
      }

      //Flush the line.
      Out >> CurrentLine;
    }

    //Assign the data to the nice ID.
    Out >> "  const unsigned char* " << Name << "::" <<
      Files[i].ToFilename().ToIdentifier() << " =" >>
      "    " << ResourceID << ";";
  }

  //Write the load helper.
  Out >> "    ";
  Out >> "#ifdef PRIM_LIBRARY";
  Out >> "namespace " << Name >> "{";

  Out >> "  prim::String Load(const prim::ascii* ResourceName)";
  Out >> "  {";
  Out >> "    prim::Array<prim::byte> x;";
  Out >> "    Load(ResourceName, x);";
  Out >> "    return prim::String(&x.a(), x.n());";
  Out >> "  }";
  Out >> "  ";
  Out >> "  void Load(const prim::ascii* ResourceName, "
             "prim::Array<prim::byte>& Out)";
  Out >> "  {";
  Out >> "    Out.Clear();";
  Out >> "    prim::String r = ResourceName;";
  Out >> "    ";
  for(count i = 0; i < Files.n(); i++)
  {
    String Variable = Files[i].ToFilename().ToIdentifier();
    Out >> "    ";
    Out << "if(r == \"" << Files[i].ToFilename() << "\")";
    Out >> "      Out.CopyMemoryFrom(" << Variable << "," >> "        " <<
      Variable << "_n);";
  }
  Out >> "  }";
  Out >> "}" >> "#endif" >> "#endif" >> "#endif";
  Out++;

  //Write the header file.
  String HeaderFile = Name;
  HeaderFile << ".h";
  File::Write(HeaderFile, Out);
  C::Out() >> "Wrote " << HeaderFile;
  return AutoRelease<Console>();
}
