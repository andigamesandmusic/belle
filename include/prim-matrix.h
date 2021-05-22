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

#ifndef PRIM_INCLUDE_MATRIX_H
#define PRIM_INCLUDE_MATRIX_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  ///Efficient two-dimensional matrix.
  template <class T> class Matrix : public Array<T>
  {
    ///Stores the number of columns in the matrix.
    count Columns;

    public:

    ///Default constructor creates an empty matrix.
    Matrix() : Columns(1) {}

    ///Constructs an m x n matrix (rows by columns).
    Matrix(count Rows, count Columns_) {mn(Rows, Columns_);}

    ///Imports an m x n matrix (rows by columns).
    Matrix(const T* Data, count Rows, count Columns_)
    {
      mn(Data, Rows, Columns_);
    }

    ///Imports data from an existing two-dimensional array.
    void mn(const T* Data, count Rows, count Columns_)
    {
      mn(Rows, Columns_);
      Array<T>::CopyFrom(Data, mn());
    }

    ///Clears and resizes the matrix.
    void mn(count Rows, count Columns_)
    {
      //An empty matrix will represented by a 0x1 matrix for convenience.
      if(Rows <= 0 or Columns_ <= 0)
      {
        Rows = 0;
        Columns_ = 1;
      }

      Columns = Columns_;
      Array<T>::n(0);
      Array<T>::n(Rows * Columns_);
    }

    ///Returns the total number of elements in the matrix.
    inline count mn() const
    {
      return Array<T>::n();
    }

    /**Returns the number of columns in the matrix. This method will also hide
    the Array<T>::n() methods to prevent changes to the size of the array
    without going through mn() first.*/
    inline count n() const
    {
      return Columns;
    }

    ///Returns the number of rows in the matrix.
    inline count m() const
    {
      //Note: n() >= 1 as enforced by mn().
      return mn() / n();
    }

    ///Returns a reference to the element i, j.
    T& ij(count i, count j) {return Array<T>::ith(i * Columns + j);}

    ///Returns a const reference to the element i, j.
    const T& ij(count i, count j) const {return Array<T>::ith(i * Columns + j);}

    ///Returns a reference to the element i, j.
    T& operator () (count i, count j) {return ij(i, j);}

    ///Returns a const reference to the element i, j.
    const T& operator () (count i, count j) const {return ij(i, j);}

    ///Clears the matrix.
    void Clear() {Array<T>::Clear(); Columns = 1;}

    ///Clears the matrix after deleting the objects pointed to by each element.
    void ClearAndDeleteAll() {Array<T>::ClearAndDeleteAll(); Columns = 1;}

    /**Solves an augmented matrix using Gaussian Elimination. An augmented
    matrix is a N x N + 1 matrix (last column represents the right hand side of
    the equation). If the matrix can not be solved (is improper size, under- or
    over-determined) then an empty array is returned.*/
    Array<T> LinearSolve() const
    {
      //Copy the matrix so that the current one is not manipulated.
      Matrix<T> M = *this;

      //Determine the rank and make sure this is an augmented matrix.
      count Rank = M.m(), RHS = Rank;
      if(Rank < 1 or M.n() != Rank + 1)
      {
        Array<T> NoAnswer;
        return NoAnswer;
      }

      //Convert to upper triangular form.
      for(count k = 0; k < Rank - 1; k++)
      {
        T M_k_k = M(k, k);

        if(Limits<T>::IsZero(Chop(M_k_k, T(1.e-10))))
        {
          // Zero pivot found in line
          Array<T> NoAnswer;
          return NoAnswer;
        }

        for(count i = k + 1; i < Rank; i++)
        {
          T x = M(i, k) / M_k_k;
          for(count j = k + 1; j < Rank; j++)
            M(i, j) -= M(k, j) * x;
          M(i, RHS) -= M(k, RHS) * x;
        }
      }

      //Ensure that no diagonals contain zeros.
      for(count i = 0; i < Rank; i++)
      {
        if(Limits<T>::IsZero(Chop(M(i, i), T(1.e-10))))
        {
          // Zero diagonal found in line
          Array<T> NoAnswer;
          return NoAnswer;
        }
      }

      //Solve via back substitution.
      M(Rank - 1, RHS) /= M(Rank - 1, Rank - 1);
      for(count i = Rank - 2; i >= 0; i--)
      {
        T Sum = M(i, RHS);
        for(count j = i + 1; j < Rank; j++)
          Sum = Sum - M(i, j) * M(j, RHS);
        M(i, RHS) = Sum / M(i, i);
      }

      //Copy solution to array.
      Array<T> Solution;
      Solution.n(Rank);
      for(count i = 0; i < Rank; i++)
        Solution[i] = M(i, RHS);
      return Solution;
    }

    //Hide methods from Array<T> that are not useful for Matrix<T>.
    private:
    const T* n(count NewSize);
    T& Add();
    void Add(const T& NewElement);
    void CopyFrom(const T* OtherArray, count NumberOfElementsToTake);
    void CopyFrom(const Array<T>& Other);
    void CopyMemoryFrom(const T* OtherArray, count NumberOfElementsToTake);
    void CopyMemoryFrom(const Array<T>& Other);
    void EncodeAsASCIIHex(Array<byte>& HexArray) const;
    void SwapWith(Array<T>& Other);
    void ZeroFrom(count Index);
  };
}
#endif
