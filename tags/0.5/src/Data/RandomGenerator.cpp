/*-----------------------------------------.---------------------------------.
| Filename: RandomGenerator.cpp            | Random Generator                |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 21:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Variable.h>
#include <lbcpp/Data/RandomGenerator.h>
#include <lbcpp/Lua/Lua.h>
using namespace lbcpp;

//////////////////////////////////////////////////////////////////////////////

// Mersenne Twister with improved initialization ///
// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/emt19937ar.html

/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

/* Period parameters */  
//#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

void RandomGenerator::setSeed(juce::uint32 seed)
{
  /* initializes mt[N] with a seed */
  mt[0]= seed & 0xffffffffUL;
  for (mti=1; mti<N; mti++)
  {
    mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
    /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
    /* In the previous versions, MSBs of the seed affect   */
    /* only MSBs of the array mt[].                        */
    /* 2002/01/09 modified by Makoto Matsumoto             */
    mt[mti] &= 0xffffffffUL;
    /* for >32 bit machines */
  }
}

juce::uint32 RandomGenerator::sampleUint32()
{
  juce::uint32 y;
  static juce::uint32 mag01[2]={0x0UL, MATRIX_A};
  /* mag01[x] = x * MATRIX_A  for x=0,1 */

  if (mti >= N)
  { /* generate N words at one time */
    int kk;

    for (kk=0;kk<N-M;kk++) {
        y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
        mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    for (;kk<N-1;kk++) {
        y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
        mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
    mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

    mti = 0;
  }

  y = mt[mti++];

  /* Tempering */
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680UL;
  y ^= (y << 15) & 0xefc60000UL;
  y ^= (y >> 18);
  return y;
}

//////////////////////////////////////////////////////////////////////////////

juce::uint32 RandomGenerator::defaultSeed[RandomGenerator::N];

void RandomGenerator::initializeRandomGenerator()
{
  RandomGenerator gen(16645186);
  memcpy(defaultSeed, gen.mt, sizeof (juce::uint32) * N);
}

RandomGenerator::RandomGenerator(juce::uint32 seedValue)
  : Object(randomGeneratorClass)
  {setSeed(seedValue);}

RandomGenerator::RandomGenerator()
  : Object(randomGeneratorClass)
{
  memcpy(mt, defaultSeed, sizeof (juce::uint32) * N);
  mti = N;
}

ObjectPtr RandomGenerator::clone(ExecutionContext& context) const
{
  ObjectPtr res = new RandomGenerator((void* )0); // use private empty constructor
  clone(context, res);
  return res;
}

void RandomGenerator::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const RandomGeneratorPtr& target = t.staticCast<RandomGenerator>();
  memcpy(target->mt, mt, sizeof (juce::uint32) * N);
  target->mti = mti;
}

size_t RandomGenerator::sampleWithProbabilities(const std::vector<double>& probabilities, double probabilitiesSum)
{
  jassert(probabilities.size());
  if (!probabilitiesSum)
    for (size_t i = 0; i < probabilities.size(); ++i)
      probabilitiesSum += probabilities[i];
  double number = sampleDouble(probabilitiesSum);
  for (size_t i = 0; i < probabilities.size(); ++i)
  {
    double prob = probabilities[i];
    if (number <= prob)
      return i;
    number -= prob;
  }
  return probabilities.size() - 1; // this might happen in case in numerical imprecisions (sum of probabilities which do not perfectly match probabilitiesSum)
}

double RandomGenerator::sampleDoubleFromGaussian()
{
  // from http://www.taygeta.com/random/gaussian.html
  double x1, x2, w;
  do
  {
    x1 = sampleDouble(-1.0, 1.0);
    x2 = sampleDouble(-1.0, 1.0);
    w = x1 * x1 + x2 * x2;
  } while (w >= 1.0);

  w = sqrt((-2.0 * log(w)) / w);
  return x1 * w; // (x2 * w) is another sample from the same gaussian
}

int RandomGenerator::create(LuaState& state)
{
  int seed = state.checkInteger(1);
  state.pushObject(new RandomGenerator(seed));
  return 1;
}

int RandomGenerator::sample(LuaState& state)
{
  RandomGeneratorPtr random = state.checkObject(1, randomGeneratorClass).staticCast<RandomGenerator>();
  state.pushNumber(random->sampleDouble());
  return 1;
}

/*
** sampleFromGamma() and sampleFromBeta() are translated from Java from
** "MALLET" (MAchine Learning for LanguagE Toolkit). http://www.cs.umass.edu/~mccallum/mallet
** Author: Andrew McCallum 
*/
double RandomGenerator::sampleFromGamma(double alpha, double beta, double lambda) // not tested
{
  double gamma = 0;
  jassert(alpha > 0 && beta > 0);
  if (alpha < 1)
  {
    double b = 1 + alpha * exp(-1.0);
    while (true)
    {
      double p = b * sampleDouble();
      if (p>1)
      {
        gamma = -log((b-p)/alpha);
        if (sampleDouble() <= pow(gamma, alpha-1))
          break;
      }
      else
      {
        gamma = pow(p,1/alpha);
        if (sampleDouble() <= exp(-gamma))
          break;
      }
    }
  }
  else if (alpha == 1)
    gamma = -log(sampleDouble());
  else
  {
    double y = -log(sampleDouble());
    while (sampleDouble() > pow(y * exp(1 - y), alpha - 1))
      y = -log(sampleDouble());
    gamma = alpha * y;
  }
  return beta * gamma + lambda;
}

double RandomGenerator::sampleFromBeta(double alpha, double beta)
{
  jassert(alpha > 0.0 && beta > 0.0);
  if (alpha == 1 && beta == 1)
    return sampleDouble();
  else if (alpha >= 1 && beta >= 1)
  {
    double A = alpha - 1, B = beta - 1;
    double C = A + B;
    double L = C * log (C);
    double mu = A / C;
    double sigma = 0.5 / sqrt(C);
    double x, y;
    do
    {
      y = sampleDoubleFromGaussian();
      x = sigma * y + mu;
    }
    while (x < 0 || x > 1);
    double u = sampleDouble();
    while (log(u) >= A * log(x / A) + B * log((1 - x) / B) + L + 0.5 * y * y)
    {
      do
      {
        y = sampleDoubleFromGaussian();
        x = sigma * y + mu;
      }
      while (x < 0 || x > 1);
      u = sampleDouble();
    }
    return x;
  }
  else
  {
    double v1, v2;
    do
    {
      v1 = pow(sampleDouble(), 1 / alpha);
      v2 = pow(sampleDouble(), 1 / beta);
    } while (v1 + v2 > 1);
    return v1 / (v1 + v2);
  }
}
