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

#ifndef PRIM_INCLUDE_RANDOM_H
#define PRIM_INCLUDE_RANDOM_H

#ifndef PRIM_LIBRARY
#error This file can not be included individually. Include prim.h instead.
#endif

namespace PRIM_NAMESPACE
{
  /**A random number generator using the multiply-with-carry algorithm. This
  algorithm produces an extremely uniform and uncorrelated distribution and has
  an extremely long period somewhere on the order of 2^64. This particular
  version of the number generator has been evaluated with the Dieharder random
  number generator test battery and performs extremely well. The generator
  defaults to seeding with the system clock, so you only need to seed it if you
  want to be able to reproduce a given random sequence.*/
  class Random
  {
    ///Stores the state of the random number generator.
    uint32 History[5];

    public:

    //Returns the next uniform 32-bit random number.
    uint32 Next()
    {
      uint64 Sum = uint64(2111111111UL) * uint64(History[3]) + //
                   uint64(1492) * uint64(History[2]) +         //
                   uint64(1776) * uint64(History[1]) +         //
                   uint64(5115) * uint64(History[0]) +         //
                   uint64(History[4]);                         //

      History[3] = History[2];
      History[2] = History[1];
      History[1] = History[0];
      History[4] = uint32(Sum >> uint64(32));
      History[0] = uint32(Sum);

      return History[0];
    }

    ///Returns a uniformly random signed 64-bit integer.
    int64 NextSignedInt64()
    {
      uint64 RandomInteger = (uint64(Next()) << uint64(32)) + uint64(Next());
      return int64(RandomInteger);
    }

    ///Returns a uniformly random unsigned 64-bit integer.
    uint64 NextUnsignedInt64()
    {
      return uint64(NextSignedInt64());
    }

    ///Returns a uniform random integer.
    integer NextInteger()
    {
      return integer(NextSignedInt64());
    }

    /**Returns a uniformly random 64-bit float over [1.0, 2.0). It is
    calculated by fixing the floating-point exponent and dumping random bits
    into the mantissa.*/
    float64 NextRawFloat64()
    {
      float64 Float = 0;
      void* FloatVoid = reinterpret_cast<void*>(&Float);
      uint64* BinaryFloat = reinterpret_cast<uint64*>(FloatVoid);
      *BinaryFloat = uint64(1023) << uint64(52);
      uint64 RandomMantissa = (uint64(Next()) << uint64(32)) + uint64(Next());
      RandomMantissa = RandomMantissa >> uint64(12);
      *BinaryFloat += RandomMantissa;
      return Float;
    }

    /**Returns a uniformly random number over [0.0, 1.0). In theory, the
    floating-point subtraction used in this method will not cause any loss of
    precision and so it should not introduce correlation.*/
    number NextNumber()
    {
      return number(NextRawFloat64() - 1.);
    }

    /**Returns a random integer in a particular range. The order of the
    arguments does not make a difference. Note that the upper integer is
    excluded. In other words the interval is [Low, High).*/
    int64 NextIntegerInRange(int64 RangeBound1, int64 RangeBound2 = 0)
    {
      //If bounds are equal, return the bound.
      if(RangeBound1 == RangeBound2)
        return RangeBound1;

      //Determine floating point range.
      float64 Low, High;
      int64 LowInteger, HighInteger;
      if(RangeBound1 < RangeBound2)
      {
        Low = float64(RangeBound1);
        LowInteger = RangeBound1;
        High = float64(RangeBound2);
        HighInteger = RangeBound2;
      }
      else
      {
        Low = float64(RangeBound2);
        LowInteger = RangeBound2;
        High = float64(RangeBound1);
        HighInteger = RangeBound1;
      }

      //Generate a random number in the interval [1.0, 2.0).
      float64 Float = NextRawFloat64();

      //Set the range.
      Float = Float * High - Float * Low - High + 2.0 * Low;

      //Convert to integer.
      int64 Integer = int64(Float);

      //Clamp the range in case of any floating-point arithmetic errors.
      if(Integer < LowInteger)
        Integer = LowInteger;
      else if(Integer >= HighInteger)
        Integer = HighInteger - 1;

      return Integer;
    }

    /**Returns a random number in a particular range. The order of the arguments
    does not make a difference. Note that the upper number is excluded. In other
    words the interval is [Low, High).*/
    float64 NextNumberInRange(float64 RangeBound1, float64 RangeBound2 = 0)
    {
      //Determine floating point range.
      float64 Low, High;
      if(RangeBound1 < RangeBound2)
      {
        Low = RangeBound1;
        High = RangeBound2;
      }
      else
      {
        Low = RangeBound2;
        High = RangeBound1;
      }

      //Generate a random number in the interval [1.0, 2.0).
      float64 Float = NextRawFloat64();

      //Set the range.
      Float = Float * High - Float * Low - High + 2.0 * Low;

      //Clamp the range in case of floating-point arithmetic errors.
      if(Float < Low)
        Float = Low;
      if(Float > High)
        Float = High;
      return Float;
    }

    ///Pick a random sequence using a 32-bit seed.
    void PickSequence(uint32 Seed)
    {
      //Use recommended MCA initialization arithmetic to generate initial state.
      for(count i = 0; i < 5; i++)
      {
        Seed *= 29943829;
        Seed -= 1;
        History[i] = Seed;
      }

      //Break in the sequence by discarding the first 100 values.
      for(count i = 0; i < 100; i++)
        Next();
    }

    ///Picks a random sequence using the current state and the system clock.
    void PickRandomSequence()
    {
      History[0] = SystemNoise();
      History[1] = SystemNoise();
      History[2] = SystemNoise();
      History[3] = SystemNoise();
      History[4] = SystemNoise();

      //Break in the sequence by discarding the first 100 values.
      for(count i = 0; i < 100; i++)
        Next();
    }

    ///Initializes the random number generator using system noise.
    Random()
    {
      PickRandomSequence();
    }

    ///Initialize the random number generator with a 32-bit seed.
    Random(uint32 Seed)
    {
      PickSequence(Seed);
    }

    ///Returns a uniformly random number [0, 1).
    number Between()
    {
      return NextNumber();
    }

    ///Returns a uniformly random number [0, Max).
    float32 Between(float32 Max)
    {
      return float32(NextNumberInRange(float64(Max)));
    }

    ///Returns a uniformly random number [0, Max).
    float64 Between(float64 Max)
    {
      return NextNumberInRange(Max);
    }

    ///Returns a uniformly random number [Min, Max).
    float32 Between(float32 Min, float32 Max)
    {
      return float32(NextNumberInRange(float64(Min), float64(Max)));
    }

    ///Returns a uniformly random number [Min, Max).
    float64 Between(float64 Min, float64 Max)
    {
      return NextNumberInRange(Min, Max);
    }

    ///Returns a uniformly random integer [0, Max).
    int32 Between(int32 Max)
    {
      return int32(NextIntegerInRange(int64(Max)));
    }

    ///Returns a uniformly random integer [0, Max).
    int64 Between(int64 Max)
    {
      return NextIntegerInRange(Max);
    }

    ///Returns a uniformly random integer [Min, Max).
    int32 Between(int32 Min, int32 Max)
    {
      return int32(NextIntegerInRange(int64(Min), int64(Max)));
    }

    ///Returns a uniformly random integer [Min, Max).
    int64 Between(int64 Min, int64 Max)
    {
      return NextIntegerInRange(Min, Max);
    }

    ///Returns system noise using an entropy pool.
    static uint32 SystemNoise();
  };
#ifdef PRIM_COMPILE_INLINE
uint32 Random::SystemNoise()
{
#if defined(PRIM_ENVIRONMENT_UNIX_LIKE) and defined(PRIM_USE_DEV_RANDOM)
  {
    uint32 R;
    std::ifstream FileStream;
    FileStream.open("/dev/random", std::ios::in | std::ios::binary);
    if(FileStream.is_open())
    {
      FileStream.read(reinterpret_cast<ascii*>(&R), sizeof(R));
      return R;
    }
  }
#endif
  {
    //Keep a noise generator around to help seed things.
    static Random NoiseState(123);

    //Gather bits into an entropy pool.
    const count EntropyFields = 5;
    uint32 Entropy[EntropyFields];
    count EntropyField = 0;

    //Clock ticks since program start.
    Entropy[EntropyField++] = uint32(clock());

    //Current time in seconds.
    Entropy[EntropyField++] = uint32(time(0));

    //Address of entropy field on stack.
    Entropy[EntropyField++] = uint32(reinterpret_cast<uintptr>(Entropy));

    //Address of new block of memory and XORed data in that memory.
    {
      const count n = 1024;
      uint32* x = new uint32[n];
      Entropy[EntropyField++] = uint32(reinterpret_cast<uintptr>(x));
      uint32 s = 0;
      for(count i = 0; i < n; i++)
      {
        s ^= x[i];

        /*Thinking ahead: generate random data into the memory in case
        this region of memory is reused next time the program runs.*/
        x[i] = NoiseState.Next();
      }
      Entropy[EntropyField++] = s;
      delete [] x;
    }

    /*XOR the entropy fields together with the next value out of a static PRNG,
    and reseed the PRNG with the result.*/
    uint32 R = 0;
    {
      for(count i = 0; i < EntropyFields; i++)
        R ^= Entropy[i] * uint32(29943829) - uint32(1);
      R ^= NoiseState.Next();
      NoiseState.PickSequence(R);
    }
    return R;
  }
}
#endif
}
#endif
