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

#ifndef PRIM_INCLUDE_MOD_FFT_H
#define PRIM_INCLUDE_MOD_FFT_H
#ifdef PRIM_WITH_FFT //Depends on Thread (optionally).

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

/*Module Configuration
================================================================================

To enable multithreaded computation, enable the thread module.

================================================================================
End Module Configuration*/

namespace PRIM_NAMESPACE
{
  ///Calculates Cos(A / B * Pi).
  template<typename floatT>
  floatT CosRecursive(floatT A, floatT B, count Iterations = 16)
  {
    floatT Angle = A / B * 3.1415926535897932384626433832795029L;
    floatT Angle_2 = Angle * Angle;
    floatT Value = 1.0L;
    for(count M = Iterations * 2 - 1; M > 0; M -= 2)
      Value = 1.0L - Angle_2 * Value / floatT(M * (M + 1));
    return Value;
  }

  ///Calculates Sin(A / B * Pi).
  template<typename floatT>
  floatT SinRecursive(floatT A, floatT B, count Iterations = 16)
  {
    floatT Angle = A / B * 3.1415926535897932384626433832795029L;
    floatT Angle_2 = Angle * Angle;
    floatT Value = 1.0L;
    for(count M = Iterations * 2; M > 0; M -= 2)
      Value = 1.0L - Angle_2 * Value / floatT(M * (M + 1));
    return Angle * Value;
  }

  ///Reverses the bitwise index of the complex array (i.e. 10100 <--> 00101).
  template <typename T>
  void BitReverse(Array<Complex<T> >& Data)
  {
    count n, nn,  m, j, i;

    n = Data.n() * 2;
    nn = n >> 1;
    j = 1;
    for(i = 1; i < n; i += 2)
    {
      if(j > i)
        Swap(Data[(j - 1) >> 1], Data[(i - 1) >> 1]);

      m = nn;

      while(m >= 2 and j > m)
      {
        j -= m;
        m >>= 1;
      }

      j += m;
    }
  }

  ///Normalizes the data to correct for magnitude scaling by the FFT.
  template <typename T>
  void Normalize(Array<Complex<T> >& Data)
  {
    T Scale = T(1) / T(Data.n());
    for(count i = 0; i < Data.n(); i++)
      Data[i] *= Scale;
  }

  ///Generates a complex array of a given size consisting of noise from [-1, 1).
  template <typename T>
  void GenerateWhiteNoise(Array<Complex<T> >& Destination, count FFTSize)
  {
    Random r;
    Destination.n(FFTSize);
    for(count i = 0; i < FFTSize; i++)
      Destination[i] = Complex<T>(r.Between(-1.0, 1.0), r.Between(-1.0, 1.0));
  }

  /**Generates a reusable cosine lookup table for a particular FFT size. The
  typename InternalT controls the float type of intermediate calculations. For
  best results use float80.*/
  template <typename InternalT, typename T>
  void GenerateCosineLookup(Array<T>& LookupToMake, count FFTSize)
  {
    LookupToMake.n(FFTSize + 1);
    for(count i = 0; i < LookupToMake.n(); i++)
      LookupToMake[i] = T(CosRecursive<InternalT>(i * 2, FFTSize, 16));
  }

  ///Calculates the RMS error between two arrays of equal size.
  template <typename T>
  float64 CalculateRMSError(Array<Complex<T> >& Array1,
    Array<Complex<T> >& Array2)
  {
    if(Array1.n() != Array2.n())
      return 0.0;
    float64 Sum = 0.0;
    for(count i = 0; i < Array1.n(); i++)
      Sum += Square(float64(Array1[i].Mag(Array2[i])));
    /*In case of identical match, max out at -64 which is the theoretical
    precision of a long double.*/
    return Max(-64.0, Log2(Sqrt(Sum / float64(Array1.n()))));
  }
}

namespace PRIM_NAMESPACE { namespace meta
{
#ifdef PRIM_INCLUDE_THREAD_H
  ///Internal hook for attaching butterflys to threads.
  template <count FFTSize, count N, count Sign, typename T>
  class ButterflyThreadHook : public Thread
  {
    Complex<T>* Data;
    T* CosLookup;
    count MaxThreadDepth;
    count ThreadDepth;

    public:
    ButterflyThreadHook(Complex<T>* Data, T* CosLookup, count MaxThreadDepth,
      count ThreadDepth) : Data(Data), CosLookup(CosLookup),
      MaxThreadDepth(MaxThreadDepth), ThreadDepth(ThreadDepth) {}

    void Run();
    virtual ~ButterflyThreadHook() {}
  };
#endif

  /**Represents a single type of butterfly. The template parameters, which are
  determined at compile-time, allow the butterfly loop to be unrolled by a
  compiler with speed optimizations turned on.*/
  template <count FFTSize, count N, count Sign, typename T>
  class Butterfly
  {
    Butterfly<FFTSize, N / 2, Sign, T> Next;
  public:
    void Apply(Complex<T>* Data, T* CosLookup, count MaxThreadDepth = 3,
      count ThreadDepth = 0)
    {
#ifdef PRIM_INCLUDE_THREAD_H
      if(ThreadDepth < MaxThreadDepth)
      {
        Thread* t1 = new ButterflyThreadHook<FFTSize, N / 2, Sign, T>(
          Data, CosLookup, MaxThreadDepth, ThreadDepth + 1);
        Thread* t2 = new ButterflyThreadHook<FFTSize, N / 2, Sign, T>(
          Data + (N / 2), CosLookup, MaxThreadDepth, ThreadDepth + 1);
        t1->Begin();
        t2->Begin();
        t1->WaitToEnd();
        t2->WaitToEnd();
      }
      else
#endif
      {
        Next.Apply(Data, CosLookup, MaxThreadDepth, ThreadDepth + 1);
        Next.Apply(Data + (N / 2), CosLookup, MaxThreadDepth, ThreadDepth + 1);
      }

      //Lower power of two, better precision; higher power of two, better speed.
      const count PrecisionSpeedTradeoff = 16;

      //Initialize butterfly.
      count Index0 = 0, Index1 = 0, i = 0, iHalf = 0, Complement = FFTSize / N;
      Complex<T> Temporary, Angle(1.0, 0.0);
      Complex<T> Wp(CosLookup[Complement] - T(1),
                    CosLookup[(FFTSize / 4 - Complement + FFTSize) % FFTSize] *
                    T(-Sign));

      //Do the first half of the butterfly calculation.
      for(; i < N / 2; i += 2, iHalf++)
      {
        //Alternatively: instead of % ==, could try & !=
        if(iHalf % PrecisionSpeedTradeoff == 0)
        {
          //Update the angle with a fully precise value from the lookup table.
          Angle.x = CosLookup[iHalf * Complement];
          Angle.y = CosLookup[FFTSize / 4 - iHalf * Complement] * T(-Sign);
        }
        Index0 = (i >> 1);
        Index1 = ((i + N) >> 1);
        Temporary = Data[Index1] * Angle;
        Data[Index1] = Data[Index0] - Temporary;
        Data[Index0] += Temporary;
        Angle += Angle * Wp;
      }

      /*Do the second half of the butterfly calculation (only difference is
      trig table lookup).*/
      for(; i < N; i += 2, iHalf++)
      {
        if(iHalf % PrecisionSpeedTradeoff == 0)
        {
          //Update the angle with a fully precise value from the lookup table.
          Angle.x = -CosLookup[FFTSize / 2 - iHalf * Complement];
          Angle.y = CosLookup[iHalf * Complement - FFTSize / 4] * T(-Sign);
        }
        Index0 = (i >> 1);
        Index1 = ((i + N) >> 1);
        Temporary = Data[Index1] * Angle;
        Data[Index1] = Data[Index0] - Temporary;
        Data[Index0] += Temporary;
        Angle += Angle * Wp;
      }
    }
  };

  ///Template recursion break for the butterfly
  template <count FFTSize, count Sign, typename T>
  class Butterfly<FFTSize, 1, Sign, T>
  {
  public:
     void Apply(Complex<T>* Data, T* CosLookup, count MaxThreadDepth = 3,
      count ThreadDepth = 0) {(void)Data; (void)CosLookup; (void)MaxThreadDepth;
        (void)ThreadDepth;}
  };

#ifdef PRIM_INCLUDE_THREAD_H
  ///Thread entry-point that triggers a particular butterfly to run.
  template <count FFTSize, count N, count Sign, typename T>
  void ButterflyThreadHook<FFTSize, N, Sign, T>::Run()
  {
    Butterfly<FFTSize, N, Sign, T> f;
    f.Apply(Data, CosLookup, MaxThreadDepth, ThreadDepth);
  }
#endif
}}

namespace PRIM_NAMESPACE
{
  enum FFTDirection
  {
    Forwards = 1,
    Backwards = -1
  };

  /**Calculates a forwards (dsp::Forwards) or backwards (dsp::Backwards) FFT.

  Note that thread depth is a trade-off between the overhead of thread
  management versus the leveraging of multicore environments. The number of
  threads that actually will run will peak at 2^(Depth + 1) - 1. It may be worth
  testing at run-time how various thread depths perform given a particular FFT
  size. In general, small FFTs should have thread depth at 0, and large FFTs on
  a multicore machine may have thread depth up to 3 or 4.

  This FFT implementation has the following advantages and disadvantages:
  <ul>
  <li>Pros
    <ul>
    <li>Compiler-optimizable code. Due to template recursion, a compiler can
    easily unroll loops to create FFT butterflys that spend minimal time in
    for-loop overhead. Note that you must enable full optimization on your
    compiler for this to work.</li>
    <li>Uses (relatively) little memory. While the entire FFT operation is
    performed in memory, its memory consumption is only slightly larger (by an
    eighth) than the footprint of the data itself. The following are the
    memory footprints depending on the float type:
    <ul>
      <li>float32: FFT Size * 9 bytes</li>
      <li>float64: FFT Size * 18 bytes</li>
      <li>float80: FFT Size * 36 bytes (due to 128-bit alignment)</li>
    </ul>
    </li>
    <li>The cosine table which is generated by GenerateCosineLookup() is fully
    accurate for 64-bit floats when the internal type for intermediate
    calculations is set to float80.</li>
    <li>Typical precision (largest FFT size to smallest FFT size):
      <ul>
      <li>float32: 20 to 23 bits</li>
      <li>float64: 49 to 53 bits</li>
      <li>float80: 60 to 64 bits</li>
      </ul>
    </li>
    <li>Built-in multithreading. The thread depth may be specified so that the
    initial butterflys each occupy separate threads and distribute themselves
    across multiple cores in a multicore machine.</li>
    <li>Portable and lightweight (only 300 lines of code). Different platforms
    should compile equally well, and there should be little variation in the
    precision or speed of the results.</li>
    <li>Extremely large FFTs (greater than 64 million points) can be calculated.
    The only real limitation is the availability of system memory.</li>
    <li>Consistent with the prim library, using Array&lt;Complex&lt;T&gt;&gt;
    as the argument type, where T can be any float type (even an
    arbitrary-precision floating point class such as MPFR C++ if it were
    available).</li>
    </ul>
  </li>
  <li>Cons
    <ul>
    <li>Only complex 1-D data can be transformed. There are no savings benefits
    for real-to-complex and complex-to-real data. However, in many applications
    it is possible to do two FFTs for the price of one. For example, you can put
    two real signals into the complex real and imaginary parts of the data, take
    the FFT, multiply by the FFT of a real impulse response, then take the
    inverse FFT. The real and imaginary parts correspond to two separate signals
    convolved with the same real impulse response.</li>
    <li>Thread affinity is not managed which leads to possible duplication of
    threads among the same core yielding performance decrease due to thread
    overhead. Also, each butterfly is atomic to a thread which means the final
    large butterflys will not take advantage of multithreading.</li>
    <li>No smart caching of data for large transforms when page file is
    unavoidable. System will essentially grind to halt if physical memory is
    exhausted.</li>
    <li>Though speed is comparable to that of the FFTW benchmark (within a
    factor of two), there is no substitute for a dedicated library such as FFTW.
    On the other hand, prim is released with looser licensing restrictions
    (BSD 2-clause instead of GPL).</li>
    </ul>
  </li>
  </ul>*/
  template<count Sign, typename T>
  void FFT(Array<Complex<T> >& Data, Array<T>& CosineLookup,
    count ThreadDepth = 3)
  {
    //Create shorthands.
    const count P_0 = 1, P_1 = P_0 * 2, P_2  = P_1  * 2, P_3  = P_2  * 2,
      P_4 = P_3 * 2, P_5 = P_4 * 2, P_6 = P_5 * 2, P_7 = P_6 * 2, P_8 = P_7 * 2,
      P_9  = P_8  * 2, P_10 = P_9  * 2, P_11 = P_10 * 2, P_12 = P_11 * 2,
      P_13 = P_12 * 2, P_14 = P_13 * 2, P_15 = P_14 * 2, P_16 = P_15 * 2,
      P_17 = P_16 * 2, P_18 = P_17 * 2, P_19 = P_18 * 2, P_20 = P_19 * 2,
      P_21 = P_20 * 2, P_22 = P_21 * 2, P_23 = P_22 * 2, P_24 = P_23 * 2,
      P_25 = P_24 * 2, P_26 = P_25 * 2, P_27 = P_26 * 2;
    #if 0 and not defined(PRIM_ENVIRONMENT_ILP32)
    const count P_28 = P_27 * 2,
      P_29 = P_28 * 2, P_30 = P_29 * 2, P_31 = P_30 * 2, P_32 = P_31 * 2,
      P_33 = P_32 * 2, P_34 = P_33 * 2, P_35 = P_34 * 2, P_36 = P_35 * 2,
      P_37 = P_36 * 2, P_38 = P_37 * 2, P_39 = P_38 * 2, P_40 = P_39 * 2;
    #endif
    Array<Complex<T> >& D = Data;
    Array<T>& C = CosineLookup;
    count N = D.n();
    count X = ThreadDepth;
    const count S = Sign;

    //If there is no data just return.
    if(N == 0) return;

    //Bit reverse the output.
    BitReverse(D);

    /*Given the FFT size, find the appropriate butterfly to initiate. Note that
    if a particular range of sizes are not going to be used, then they can be
    block commented out to reduce binary size and speed up compile times.*/
         if(N==P_0) {meta::Butterfly<P_0, P_0, S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_1) {meta::Butterfly<P_1, P_1, S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_2) {meta::Butterfly<P_2, P_2, S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_3) {meta::Butterfly<P_3, P_3, S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_4) {meta::Butterfly<P_4, P_4, S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_5) {meta::Butterfly<P_5, P_5, S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_6) {meta::Butterfly<P_6, P_6, S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_7) {meta::Butterfly<P_7, P_7, S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_8) {meta::Butterfly<P_8, P_8, S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_9) {meta::Butterfly<P_9, P_9, S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_10){meta::Butterfly<P_10,P_10,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_11){meta::Butterfly<P_11,P_11,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_12){meta::Butterfly<P_12,P_12,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_13){meta::Butterfly<P_13,P_13,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_14){meta::Butterfly<P_14,P_14,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_15){meta::Butterfly<P_15,P_15,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_16){meta::Butterfly<P_16,P_16,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_17){meta::Butterfly<P_17,P_17,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_18){meta::Butterfly<P_18,P_18,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_19){meta::Butterfly<P_19,P_19,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_20){meta::Butterfly<P_20,P_20,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_21){meta::Butterfly<P_21,P_21,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_22){meta::Butterfly<P_22,P_22,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_23){meta::Butterfly<P_23,P_23,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_24){meta::Butterfly<P_24,P_24,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_25){meta::Butterfly<P_25,P_25,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_26){meta::Butterfly<P_26,P_26,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_27){meta::Butterfly<P_27,P_27,S,T> f;f.Apply(&D.a(),&C.a(),X);}

    /*Note: the following are not possible on most modern day machines due to
    their extreme memory requirements. In general, the amount of memory consumed
    for an FFT is 18x the number of points (16 for the complex data + 2 for the
    compact trig table).*/

    //Enable for FFT sizes > 2^27. See above to enable power constants.
    #if 0 and not defined(PRIM_ENVIRONMENT_ILP32)
    else if(N==P_28){meta::Butterfly<P_28,P_28,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_29){meta::Butterfly<P_29,P_29,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_30){meta::Butterfly<P_30,P_30,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_31){meta::Butterfly<P_31,P_31,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_32){meta::Butterfly<P_32,P_32,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_33){meta::Butterfly<P_33,P_33,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_34){meta::Butterfly<P_34,P_34,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_35){meta::Butterfly<P_35,P_35,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_36){meta::Butterfly<P_36,P_36,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_37){meta::Butterfly<P_37,P_37,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_38){meta::Butterfly<P_38,P_38,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_39){meta::Butterfly<P_39,P_39,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    else if(N==P_40){meta::Butterfly<P_40,P_40,S,T> f;f.Apply(&D.a(),&C.a(),X);}
    #endif

    //If unable to use correct butterfly, undo bit reverse and return.
    else {BitReverse(Data); return;}
  }
}
#endif
#endif
