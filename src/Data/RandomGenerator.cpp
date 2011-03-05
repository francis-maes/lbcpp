/*-----------------------------------------.---------------------------------.
| Filename: RandomGenerator.cpp            | Random Generator                |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 21:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Data/RandomGenerator.h>
using namespace lbcpp;

void RandomGenerator::setSeed(int seed1, int seed2)
{
  this->seed = ((long long)(seed1) * 2654435769UL + (long long)(seed2) * 3373259426UL) >> 5;
  sampleInt();
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
  jassert(false);
  return 0;
}

int RandomGenerator::sampleInt()
{
  seed = (seed * 0x5deece66dLL + 11) & 0xffffffffffffLL;
  return (int)(seed >> 16);
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
