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

///Comma-separated value import and export according to RFC 4180.
class CSV : public Matrix<String>
{
  ///Reads CSV data into a list-array of strings.
  static void ImportToListList(const String& s, List<List<String> >& ll)
  {
    //Clear out the CSV.
    ll.RemoveAll();

    //If there is no data, return immediately.
    if(not s.n()) return;

    //Set up the decoding stream.
    const byte* Next = reinterpret_cast<const byte*>(s.Merge());
    const byte* End = Next + s.n();

    //Create the first row.
    ll.Add();

    //Parsing variables.
    bool InString = false;
    unicode Codepoint, Peek;
    String Cell;

    //Get the first character.
    Peek = (Next < End ? String::Decode(Next, End) : 0);

    //Start parsing the comma-separated values.
    while(Next - 1 < End and (Codepoint = Peek))
    {
      //Peek at the next character.
      Peek = (Next < End ? String::Decode(Next, End) : 0);

      /*All CRs will be silently skipped. The LF will be used to determine line-
      breaks in both the row structure and within a given cell.*/
      if(Codepoint == 13)
        continue;

      //Parse structural characters and construct cells.
      if(not InString)
      {
        if(Codepoint == '"')
          InString = true;
        else if(Codepoint == 10)
        {
          ll.z().Add() = Cell;
          ll.Add();
          Cell = "";
          if(not Peek)
            return;
        }
        else if(Codepoint == ',')
        {
          ll.z().Add() = Cell;
          Cell = "";
        }
        else
          Cell << Codepoint;
      }
      else
      {
        if(Codepoint == '"')
        {
          if(Peek == '"')
          {
            Cell << '"';
            Peek = (Next < End ? String::Decode(Next, End) : 0);
          }
          else
            InString = false;
        }
        else
          Cell << Codepoint;
      }
    }
    ll.z().Add() = Cell;
  }

  public:

  ///Creates an empty cell matrix.
  CSV() {}

  ///Constructor to read in a CSV file.
  CSV(const String& s)
  {
    Import(s);
  }

  ///Imports CSV data.
  void Import(const String& s)
  {
    //Read in the CSV data into a list of lists.
    List< List<String> > ll;
    ImportToListList(s, ll);

    //Count rows and max columns.
    count RowsCount = ll.n();
    count ColumnsCount = 0;
    for(count i = 0; i < RowsCount; i++)
      ColumnsCount = Max(ColumnsCount, ll[i].n());

    //Copy list-list data into matrix.
    mn(RowsCount, ColumnsCount);
    for(count i = 0; i < RowsCount; i++)
      for(count j = 0; j < ll[i].n(); j++)
        ij(i, j) = ll[i][j];
  }

  /**Exports rows and columns to CSV. Internal double-quotes, commas, and line
  breaks are properly escaped using double-quoting.*/
  String Export()
  {
    String s;
    for(count i = 0; i < m(); i++)
    {
      for(count j = 0; j < n(); j++)
      {
        String Cell = ij(i, j);
        bool MustDoubleQuote =
          Cell.Contains("\n") or Cell.Contains("\r") or Cell.Contains(",") or
          Cell.Contains("\x22");
        if(MustDoubleQuote)
        {
          Cell.Replace("\x22", "\x22\x22");
          Cell.Prepend("\x22");
          Cell.Append("\x22");
        }
        s << (j ? "," : "") << Cell;
      }
      if(i != m() - 1)
        s << "\r\n";
    }
    return s;
  }

  ///Gets the column index of a given header label.
  count GetIndexOfHeader(const String& s)
  {
    if(not m())
      return -1;

    for(count j = 0; j < n(); j++)
      if(ij(0, j) == s)
        return j;

    return -1;
  }
};
