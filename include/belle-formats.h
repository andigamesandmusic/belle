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

/**
@name Formats

Functions that convert files and import music scores of various formats.
@{
*/

#ifdef BELLE_IMPLEMENTATION
String ConvertToXML(const String& Input)
{
  return ConvertToXML(Input, Array<byte>());
}

String ConvertToXML(const String& MusicInput,
  const Array<byte>& MusicXMLValidationZip)
{
  String Input = MusicInput;
  if(Input.StartsWith("PK"))
  {
    bool Unzipped = false;
#ifdef PRIM_WITH_SHELL
    String Filename = "/tmp/belle_musicmxl_import_";
    Filename << UUIDv4() << ".zip";
    File::Write(Filename, Input);
    String ShellIn, ShellOut, ShellError;
    Shell::PipeInOut(Shell::GetProcessOnPath("unzip"),
      ShellIn, ShellOut, ShellError, "-p", Filename);
    File::Write(Filename, "");
    if(not ShellError.n())
    {
      Input = ShellOut;
      Input.ConvertToUTF8();
      count i = Input.Find("<score-partwise");
      if(i >= 0)
        Input.Erase(0, i - 1);
      i = Input.Find("</score-partwise>");
      if(i >= 0)
        Input.Erase(i + String("</score-partwise>").n(), Input.n());
      if(Input.StartsWith("<score-partwise") and
        Input.EndsWith("</score-partwise>"))
          Unzipped = true;
    }
#endif
    if(not Unzipped)
    {
      C::Error()
        >> "Input is a compressed MusicXML file. This build failed to"
        >> "unzip the contents. Make sure PRIM_WITH_SHELL is enabled"
        >> "and the unzip command-line tool is available from the shell.";
      return "";
    }
  }
  else
  {
    Input.ConvertToUTF8();
  }
  return IsGraphXML(Input)  ? Input :
         IsMusicXML(Input)  ? MusicXMLToXML(Input, MusicXMLValidationZip) :
         IsShorthand(Input) ? ShorthandToXML(Input) : String();
}

bool IsGraphXML(const String& Input)
{
  return Input.Contains("<graph>") and Input.Contains("<node") and
    Input.Contains("<edge");
}

bool IsMusicXML(const String& Input)
{
  return Input.Contains("<score-partwise");
}

bool IsShorthand(const String& Input)
{
  return Input.Contains(";Kind:");
}

String MusicXMLToXML(const String& Input)
{
  return MusicXMLToXML(Input, Array<byte>());
}

String MusicXMLToXML(const String& Input,
  const Array<byte>& MusicXMLValidationZip)
{
  if(IsGraphXML(Input))
    return Input;
  else if(IsMusicXML(Input))
  {
    Pointer<Music> M;
    MusicXMLParseScore(M.New(), Input, MusicXMLValidationZip);
    return M->ExportXML();
  }
  return "";
}

void PDFToPNG(String PDFFile, String ImageFile, number PPI, bool Grayscale)
{
  //Get the PPI of the 4x supersample.
  String PPISupersample = PPI * 4.0;

  //Use the shell to run an image processing utility.
  String Input, Output, Error;

  //Set up the arguments for the conversion.
  Value Args;

  //Run the ImageMagick convert utility.
  Args[0] = "convert";

  //Set the raster PPI to supersample at 4x PPI.
  Args[1] = "-density";
  Args[2] = PPISupersample;

  //Set input PDF file.
  Args[3] = PDFFile;

  //Remove transparency layer and flatten to white background.
  Args[4] = "-background";
  Args[5] = "white";
  Args[6] = "-flatten";

  //Downsample from supersampled 4x PPI to 1x PPI.
  Args[7] = "-resize";
  Args[8] = "25%";

  //Exclude the date-time chunk so that the file data will be stable.
  Args[9] = "-define";
  Args[10] = "png:exclude-chunks=date";

  //Convert to grayscale since only one channel is necessary.
  Args[11] = "-colorspace";
  Args[12] = (Grayscale ? "Gray" : "RGB");

  //Output to image file.
  Args[13] = ImageFile;

  //Indicate the command line parameters being used to convert the file.
  C::Cyan();
  C::Out() >> "Exporting PDF as PNG through ImageMagick...";
  C::Magenta();
  String ArgsNice;
  ArgsNice << Args;
  ArgsNice.Replace(", ", " ");
  ArgsNice.EraseBeginning("[");
  ArgsNice.EraseEnding("]");
  C::Out() >> ArgsNice;
  C::Reset();

  //Run the shell command.
  Shell::PipeInOut(Shell::GetProcessOnPath(Args[0]), Input, Output, Error,
    Args[1], Args[2], Args[3], Args[4], Args[5], Args[6], Args[7], Args[8],
    Args[9], Args[10], Args[11], Args[12], Args[13]);

  //If ImageMagick reported any problems, then report them.
  if(Error)
  {
    C::Red();
    C::Error() >> Error;
    C::Reset();
  }
}

String ShorthandToXML(const String& Input)
{
  if(IsGraphXML(Input))
    return Input;
  if(not IsShorthand(Input))
    return "";

  //Tokenize by line.
  String CleanInput = Input;
  CleanInput.LineEndingsToLF();
  List<String> Lines = CleanInput.Tokenize("\n", true);
  if(!Lines.n())
  {
    C::Out() >> "No data to process.";
    return "";
  }

  //Determine the size of the island matrix.
  count Parts = 0, Instants = 0;
  for(count i = 0; i < Lines.n(); i++)
  {
    List<String> Elements = Lines[i].Tokenize(";");
    MakeAtLeast(Parts,
      count(Elements.a().Tokenize(",")[0].ToNumber()));
    MakeAtLeast(Instants,
      count(Elements.a().Tokenize(",")[1].ToNumber()));
  }
  Parts++;
  Instants++;

  //Create the matrix.
  Matrix<Value> M(Parts, Instants);

  //Populate the matrix with the key-value information for each token.
  for(count i = 0; i < Lines.n(); i++)
  {
    List<String> Elements = Lines[i].Tokenize(";", true);
    count Part =
      count(Elements.a().Tokenize(",")[0].ToNumber());
    count Instant =
      count(Elements.a().Tokenize(",")[1].ToNumber());
    List<String> KV = Elements[1].Tokenize(",");
    for(count j = 0; j < KV.n(); j++)
      M(Part, Instant)[KV[j].Tokenize(":").a()] = KV[j].Tokenize(":").z();
    if(Elements.n() == 3)
    {
      List<String> Notes = Elements[2].Tokenize(",");
      for(count j = 0; j < Notes.n(); j++)
        M(Part, Instant)[j] = Notes[j];
    }
  }

  //Write the matrix out to XML.
  String s;
  s >> "<graph>";

  for(count i = 0; i < M.m(); i++)
  {
    for(count j = 0; j < M.n(); j++)
    {
      if(M(i, j).IsNil())
        continue;
      s >> "  <node id='" << i << "," << j << "'";
      if(!i && !j)
        s << " root='root'";
      s << " Type='Island'";
      if(!j)
      {
        s << " data-StaffLines='5'";
        s << " data-StaffOffset='" << (number(i) * -12.f) << "'";
        s << " data-StaffScale='1.0'";
      }
      s << " data-StaffConnects='true'";
      s << ">";

      //Make a brace to group the staves.
      if(!i && !j && M.m() > 1)
        s >> "    <edge to='" << (M.m() - 1) << ",0' StaffBracket='Brace'/>";
      s >> "    <edge to='" << i << "," << j << ",t' Type='Token'/>";

      //Find the next instant
      for(count k = j + 1; k < M.n(); k++)
      {
        if(!M(i, k).IsNil())
        {
          s >> "    <edge to='" << i << "," << k << "' Type='Partwise'/>";
          break;
        }
      }

      //Find the next part
      for(count k = i + 1; k < M.m(); k++)
      {
        if(!M(k, j).IsNil())
        {
          s >> "    <edge to='" << k << "," << j << "' Type='Instant-wise'/>";
          break;
        }
      }

      s >> "  </node>";

      //Create token.
      s >> "  <node id='" << i << "," << j << ",t' Type='Token'";
      Array<Value> M_i_j_Keys;
      M(i, j).EnumerateKeys(M_i_j_Keys);
      for(count k = 0, n = M_i_j_Keys.n(); k < n; k++)
      {
        Value Key = M_i_j_Keys[k];
        if(Key.IsString())
        {
          String KeyStr = Key;
          if(KeyStr == "Beam" || KeyStr == "Slur")
            continue;
          s << " " << Key << "='" << M(i, j)[Key] << "'";
        }
      }
      s << ">";
      for(count k = 0, n = M_i_j_Keys.n(); k < n; k++)
      {
        Value Key = M_i_j_Keys[k];
        String v = M(i, j)[Key];
        if(Key.IsInteger())
          s >> "    <edge to='" << i << "," << j << "," <<
            v.Tokenize("-").a() << "' Type='Note'/>";
      }
      for(count k = 0, n = M_i_j_Keys.n(); k < n; k++)
      {
        Value Key = M_i_j_Keys[k];
        if(Key.IsString())
        {
          String KeyStr = Key;
          if(KeyStr != "Beam" && KeyStr != "Slur")
            continue;
          String ValueStr = M(i, j)[Key];
          count x =
            count(ValueStr.Tokenize("|").a().ToNumber());
          count y =
            count(ValueStr.Tokenize("|").z().ToNumber());
          s >> "    <edge to='" << x << "," << y << ",t' Type='" << KeyStr <<
            "'/>";
        }
      }
      s >> "  </node>";

      //Create notes.
      for(count k = 0, n = M_i_j_Keys.n(); k < n; k++)
      {
        Value Key = M_i_j_Keys[k];
        if(Key.IsInteger())
        {
          String v = M(i, j)[Key];
          List<String> vv = v.Tokenize("-");
          s >> "  <node id='" << i << "," << j << "," << vv.a()
            << "' Type='Note' Value='" << vv.a() << "'";
          if(!M(i, j)["Rest"].IsNil())
            s << " Rest='Rest'";
          if(vv.n() == 2)
          {
            String TieId = vv.z();
            TieId.Replace("|", ",");
            s << ">";
            s >> "    <edge to='" << TieId << "' Type='Tie'/>";
            s >> "  </node>";
          }
          else
            s << "/>";
        }
      }
    }
  }
  s >> "</graph>";
  Music g;
  g.ImportXML(s);
  return g.ExportXML();
}
#endif

String ConvertToXML(const String& Input);
String ConvertToXML(const String& Input,
  const Array<byte>& MusicXMLValidationZip);
bool IsGraphXML(const String& Input);
bool IsMusicXML(const String& Input);
bool IsShorthand(const String& Input);
String MusicXMLToXML(const String& Input);
String MusicXMLToXML(const String& Input,
  const Array<byte>& MusicXMLValidationZip);
void PDFToPNG(String PDFFile, String ImageFile, number PPI, bool Grayscale);
String ShorthandToXML(const String& Input);
Pointer<const Value> SMuFLData(String SMuFLJSON);
