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
@name Ledger Lines

Functions that implement ledger line construction.
@{
*/

#ifdef BELLE_IMPLEMENTATION

///Merges the ledger line info into the ledger-line table.
void MergeLedgerLine(count Position, number Left, number Right,
  Value& LedgerLines)
{
  LedgerLines[Position]["Left"] = LedgerLines[Position]["Left"].IsNil() ?
    Left : Min(Left, +LedgerLines[Position]["Left"]);
  LedgerLines[Position]["Right"] = LedgerLines[Position]["Right"].IsNil() ?
    Right : Max(Right, +LedgerLines[Position]["Right"]);
}

///Accumulates the necessary ledger lines for a given line number and column.
void AccumulateLedgerLineForPositionAndColumn(Music::ConstNode Island,
  count LineNumber, const Value& Note, Value& LedgerLines)
{
  number ColumnWidth = Note["NoteheadWidth"];
  number LedgerLineExtra = Property(Island, "LedgerLineExtension");
  number Extension = ColumnWidth / 2.f + LedgerLineExtra;
  MergeLedgerLine(LineNumber,
    Min(+Note["Column"], number(0)) * ColumnWidth - Extension,
    Max(+Note["Column"], number(0)) * ColumnWidth + Extension, LedgerLines);
}

///Accumulates the necessary ledger lines for a give note.
void AccumulateLedgerLinesForNote(Music::ConstNode Island, const Value& Note,
  Value& LedgerLines)
{
  count LineNumber = Note["StaffPosition"].AsCount() / 2;
  for(count i = 3; i <= LineNumber; i++)
    AccumulateLedgerLineForPositionAndColumn(Island, i, Note, LedgerLines);
  for(count i = -3; i >= LineNumber; i--)
    AccumulateLedgerLineForPositionAndColumn(Island, i, Note, LedgerLines);
}

///Creates a table of ledger lines.
Value LedgerLinesForMultichord(Music::ConstNode Island, const Value& Multichord)
{
  Value LedgerLines;
  LedgerLines.NewTree();
  for(count i = 0; i < Multichord.n(); i++)
    for(count j = 0; j < Multichord[i].n(); j++)
      for(count k = 0; k < Multichord[i][j].n(); k++)
          AccumulateLedgerLinesForNote(Island, Multichord[i][j][k],
            LedgerLines);
  return LedgerLines;
}

Pointer<Path> LedgerLinePathForMultichord(Music::ConstNode Island,
  const Value& Multichord)
{
  Pointer<Path> p = new Path;
  Value Lines = LedgerLinesForMultichord(Island, Multichord);
  Array<Value> Positions;
  Lines.EnumerateKeys(Positions);
  for(count i = 0; i < Positions.n(); i++)
    Shapes::AddLine(*p, Vector(Lines[Positions[i]]["Left"], Positions[i]),
      Vector(Lines[Positions[i]]["Right"], Positions[i]),
      +Property(Island, "StaffLineThickness") *
      +Property(Island, "LedgerLineRelativeThickness"), true, true, true,
      0.5f);
  return p;
}

#endif

///@}

//Declarations
void MergeLedgerLine(count LineNumber, number Left, number Right,
  Value& LedgerLines);
void AccumulateLedgerLineForPositionAndColumn(Music::ConstNode Island,
  count LineNumber, const Value& Note, Value& LedgerLines);
void AccumulateLedgerLinesForNote(Music::ConstNode Island, const Value& Note,
  Value& LedgerLines);
Pointer<Path> LedgerLinePathForMultichord(Music::ConstNode Island,
  const Value& Multichord);
Value LedgerLinesForMultichord(Music::ConstNode Island,
  const Value& Multichord);
