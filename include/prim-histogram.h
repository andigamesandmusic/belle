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

#ifndef PRIM_INCLUDE_HISTOGRAM_H
#define PRIM_INCLUDE_HISTOGRAM_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///Bin-count based histogram with analysis functions and pretty-print.
  class Histogram
  {
    Value Bins;

    public:

    ///Gets the bin count of the bin.
    integer BinCount(Value Bin) const
    {
      return Bins.Contains(Bin) and Bins[Bin].IsInteger() ?
        Bins[Bin].AsInteger() : integer(0);
    }

    ///Returns a list of the bins sorted from highest to lowest bin-count.
    List<Value> BinsSortedHighToLow() const
    {
      List<Value> BinKeys;
      List<integer> BinCounts;
      Value Keys = Bins.Keys();
      for(count i = 0; i < Keys.n(); i++)
        BinKeys.Add() = Keys[i], BinCounts.Add() = BinCount(Keys[i]);
      for(count i = 0; i < BinKeys.n() - 1; i++)
        for(count j = i; j < BinKeys.n(); j++)
          if(BinCounts[i] < BinCounts[j])
            BinCounts.Swap(i, j), BinKeys.Swap(i, j);
      return BinKeys;
    }

    ///Returns a list of the bins sorted from lowest to highest bin-count.
    List<Value> BinsSortedLowToHigh() const
    {
      List<Value> BinKeys;
      List<integer> BinCounts;
      Value Keys = Bins.Keys();
      for(count i = 0; i < Keys.n(); i++)
        BinKeys.Add() = Keys[i], BinCounts.Add() = BinCount(Keys[i]);
      for(count i = 0; i < BinKeys.n() - 1; i++)
        for(count j = i; j < BinKeys.n(); j++)
          if(BinCounts[i] > BinCounts[j])
            BinCounts.Swap(i, j), BinKeys.Swap(i, j);
      return BinKeys;
    }

    ///Returns a 2D text chart of the histogram.
    String Chart(count Height = 10, count MaxBinNameLength = 10) const
    {
      Value Keys = Bins.Keys();
      Array<String> KeysTruncated;
      count ActualBinNameLength = 0;
      for(count i = 0; i < Keys.n(); i++)
      {
        String t = Keys[i].AsString();
        if(t.n() > MaxBinNameLength)
          t.Erase(MaxBinNameLength, t.n() - 1);
        ActualBinNameLength = PRIM_NAMESPACE::Max(ActualBinNameLength, t.n());
        KeysTruncated.Add() = t;
      }

      integer MaxFreq = Max();

      String MaxFreqStr = MaxFreq;
      count ChartColumns = MaxFreqStr.n() + Keys.n() * 2  + 1;
      count ChartRows = Height + 1 + ActualBinNameLength;

      Matrix<ascii> C(ChartRows, ChartColumns);
      for(count i = 0; i < ChartRows; i++)
        for(count j = 0; j < ChartColumns; j++)
          C(i, j) = ' ';

      for(count j = MaxFreqStr.n(); j < ChartColumns; j++)
        C(Height, j) = '-';

      {
        String Previous = "";
        for(count i = Height; i >= 0; i--)
        {
          count h = count(Round(number(i) / number(Height) * number(MaxFreq)));
          String n = h;
          if(Previous != n)
          {
            Previous = n;
            for(count j = MaxFreqStr.n() - n.n() - 1; j >= 0; j--)
              n.Prepend(" ");
            for(count j = 0; j < n.n(); j++)
              C(Height - i, j) = n.Merge()[j];
          }
        }
      }

      for(count i = 0; i < KeysTruncated.n(); i++)
        for(count j = 0; j < KeysTruncated[i].n(); j++)
          C(Height + 1 + j, MaxFreqStr.n() + i * 2 + 1) =
            KeysTruncated[i].Merge()[j];
      for(count i = 0; i < Keys.n(); i++)
      {
        integer f = BinCount(Keys[i]);
        count h = count(Round(number(f) / number(MaxFreq) * number(Height)));
        for(count j = 0; j <= h; j++)
          C(Height - j, MaxFreqStr.n() + i * 2 + 1) = j > 0 and j < h ?
            '|' : '*';
      }

      String s;
      if(Bins.n())
      {
        for(count i = 0; i < ChartRows; i++)
          for(count j = 0; j < ChartColumns; j++)
            if(not j)
              s >> C(i, j);
            else
              s << C(i, j);
      }
      return s;
    }

    ///Increments a bin by one point.
    void Increment(Value Bin)
    {
      IncrementBy(Bin, int64(1));
    }

    ///Increments a bin by a certain number of points.
    void IncrementBy(Value Bin, int32 Points)
    {
      IncrementBy(Bin, int64(Points));
    }

    ///Increments a bin by a certain number of points.
    void IncrementBy(Value Bin, int64 Points)
    {
      if(Points >= 1)
        Bins[Bin] = BinCount(Bin) + integer(Points);
    }

    ///Returns the highest bin count.
    integer Max() const
    {
      return BinCount(MaxBin());
    }

    ///Returns the first bin with the highest bin-count.
    Value MaxBin() const
    {
      Value Keys = Bins.Keys();
      Value MaxKey = Keys.a();
      integer MaxIndex = BinCount(MaxKey);
      for(count i = 1; i < Keys.n(); i++) if(BinCount(Keys[i]) > MaxIndex)
        MaxIndex = BinCount(Keys[i]), MaxKey = Keys[i];
      return MaxKey;
    }

    ///Returns the median bin-count.
    Value Median() const
    {
      Value Keys = Bins.Keys();
      Sortable::Array<integer> Frequencies;
      for(count i = 0; i < Keys.n(); i++)
        Frequencies.Add() = BinCount(Keys[i]);
      Frequencies.Sort();
      Value MedianValue;
      count Midpoint = Frequencies.n() / 2;
      if(Frequencies.n() and Frequencies.n() % 2 == 0)
      {
        integer Sum = Frequencies[Midpoint - 1] + Frequencies[Midpoint];
        MedianValue = Sum % 2 == 0 ? Value(Sum / 2) : Value(number(Sum) / 2.f);
      }
      else if(Frequencies.n() % 2 == 1)
        MedianValue = Frequencies[Midpoint];
      return MedianValue;
    }

    ///Returns the least bin-count.
    integer Min() const
    {
      return BinCount(MinBin());
    }

    ///Returns the first bin with the least bin-count.
    Value MinBin() const
    {
      Value Keys = Bins.Keys();
      Value MinKey = Keys.a();
      integer MinIndex = BinCount(MinKey);
      for(count i = 1; i < Keys.n(); i++) if(BinCount(Keys[i]) < MinIndex)
        MinIndex = BinCount(Keys[i]), MinKey = Keys[i];
      return MinKey;
    }

    ///Returns the underlying bins data.
    Value RawBins() const
    {
      return Bins;
    }

    ///Returns a chart and summary of the histogram.
    operator String() const
    {
      String s;
      s >> Chart() >> "" >> Summary();
      return s;
    }

    ///Returns a detailed summary of the histogram.
    String Summary() const
    {
      String s;
      s >> "Min:      " << Min() << " (" << MinBin() << ")";
      s >> "Median:   " << Median();
      s >> "Max:      " << Max() << " (" << MaxBin() << ")";
      s >> "High-low: " << BinsSortedHighToLow();
      s >> "Bins:     ";
      Value Keys = Bins.Keys();
      count MaxLength = 0;
      for(count i = 0; i < Keys.n(); i++)
        MaxLength = PRIM_NAMESPACE::Max(MaxLength, Keys[i].AsString().n());
      for(count i = 0; i < Keys.n(); i++)
      {
        String k = Keys[i].AsString();
        if(not i)
          s << k;
        else
          s >> "          " << k;
        for(count j = MaxLength - k.n(); j >= 0; j--)
          s << " ";
        s << "= " << BinCount(Keys[i]);
      }
      return s;
    }
  };
}
#endif
