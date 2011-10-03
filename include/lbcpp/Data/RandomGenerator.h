/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: RandomGenerator.h              | RandomGenerator generator       |
| Author  : Francis Maes                   |                                 |
| Started : 10/03/2009 20:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_RANDOM_GENERATOR_H_
# define LBCPP_DATA_RANDOM_GENERATOR_H_

# include "../Core/Object.h"
# include "predeclarations.h"

namespace lbcpp
{

extern ClassPtr randomGeneratorClass;

/*!
** @class RandomGenerator
** @brief Pseudo-random number generator
*/
class RandomGenerator : public Object
{
public:
  /** Constructor.
  **
  ** @param seedValue : seed for the random numbers generator.
  */
  RandomGenerator(juce::uint32 seedValue);
  RandomGenerator();


  /** Seed setter.
  **
  ** @param seed : random seed.
  */
  void setSeed(juce::uint32 seed);

  /** Samples an integer value uniformly in [0, 2^32[
  **
  ** @return an integer value.
  */
  juce::uint32 sampleUint32();


  /** Samples a Boolean value uniformly.
  **
  ** Samples True or False according to the following probilities:
  ** \f$ P(True) = 0.5 \f$ et \f$ P(False) = 0.5 \f$.
  **
  **
  ** @return True or False (see above).
  */
  bool sampleBool()
    {return (sampleUint32() & 0x80000000) != 0;}

  /** Samples a Boolean value using a Bernoulli distribution.
  **
  ** Samples True of False according to the following probalities:
  ** \f$ P(True) = probabilityOfTrue \f$ and \f$ P(False) = 1 -
  ** probabilityOfTrue \f$.
  **
  ** @param probabilityOfTrue : probability of True.
  **
  ** @return True or False (see above).
  */
  bool sampleBool(double probabilityOfTrue)
    {return sampleDouble() < probabilityOfTrue;}


  /** Samples a number given a discrete probability distribution.
  **
  ** Samples a number in interval [0, probabilities.size()[ <i>w.r.t.</i>
  ** @a probabilities.
  ** By default, the probabilities are not normalized. If the sum of the 
  ** probabilities is known, it can be given with argument @a probabilitiesSum. 
  ** By default (@a probabilitiesSum = 0), the probabilities sum will be computed
  ** For a normalized probability distribution, use @a probabilitiesSum = 1 or
  ** see the sampleWithNormalizedProbabilities() function.
  **
  *     @code
  *     std::vector<double> probs;
  *     probs.push_back(0.5);
  *     probs.push_back(0.2);
  *     probs.push_back(0.3);
  *     return RandomGenerator::sampleWithProbabilities(probs, 1.0);
  *     @endcode
  *
  *    This examples returns:
  *      - 0 with probability 0.5
  *      - 1 with probability 0.2
  *      - 2 with probability 0.3
  **
  ** @param probabilities : discrete probability distribution.
  ** @param probabilitiesSum : sum of the @a probabilites.
  **
  ** @return a number in interval [0, probabilities.size()[ according
  ** to the probability distribution.
  ** @see sampleWithNormalizedProbabilities
  */
  size_t sampleWithProbabilities(const std::vector<double>& probabilities, double probabilitiesSum = 0.0);

  /** Samples a number given a discrete normalized probability distribution.
  **
  ** Samples a number in interval [0, @a probabilities.size()[ according
  ** to the normalized probability distribution list @a
  ** probabilities.
  **
  ** This function is equivalent to sampleWithProbabilities(@a probabilities, 1.0).
  **
  ** @param probabilities : discrete normalized probability distribution.
  **
  ** @return a number in interval [0, @a probabilities.size()[ according
  ** to the probability distribution @a probabilities.
  ** @see sampleWithProbabilites
  */
  size_t sampleWithNormalizedProbabilities(const std::vector<double>& probabilities)
    {return sampleWithProbabilities(probabilities, 1.0);}

  unsigned char sampleByte()
    {return (unsigned char)sampleInt(256);}

  /** Samples an integer value in range [0, @a maxValue[.
  **
  ** @param maxValue : upper bound.
  **
  ** @return an interger value in range [0, @a maxValue[
  */
  int sampleInt(int maxValue)
    {jassert(maxValue > 0); return (int)(sampleUint32() % maxValue);}

  /** Samples an integer value in range [@a minValue, @a maxValue[.
  **
  ** @param minValue : lower bound.
  ** @param maxValue : upper bound.
  **
  ** @return an integer value in range [@a minValue, @a maxValue[
  */
  int sampleInt(int minValue, int maxValue)
    {jassert(maxValue > minValue); return minValue + sampleInt(maxValue - minValue);}

  /** Samples an unsigned integer value in range [0, @a maxSize[.
  **
  ** @see sampleInt
  ** @param maxSize : upper bound.
  **
  ** @return an size_t type (unsigned) value in range [0, @a maxSize[
  */
  size_t sampleSize(size_t maxSize)
    {return (size_t)sampleInt((int)maxSize);}

  /** Samples an unsigned integer value in range [@a minSize, @a maxSize[.
  **
  ** @see sampleInt
  ** @param minSize : lower bound.
  ** @param maxSize : upper bound.
  **
  ** @return an size_t type (unsigned) value in range [@a minSize, @a maxSize[
  */
  size_t sampleSize(size_t minSize, size_t maxSize)
    {if (minSize == maxSize) return minSize; jassert(maxSize > minSize); return (size_t)sampleInt((int)minSize, (int)maxSize);}

  /** Samples a float value uniformly from range [0, 1[.
  **
  ** @return a float value uniformly from range [0, 1[
  */
  float sampleFloat()
    {return sampleUint32() / (float)0xffffffff;}

  /** Samples a float value uniformly from range [0, @a maxValue[.
  **
  ** @param maxValue : upper bound.
  **
  ** @return any float value uniformly from range [0, @a maxValue[
  */
  float sampleFloat(float maxValue)
    {return sampleFloat() * maxValue;}
  
  /** Samples a float value uniformly from range [@a minValue, @a maxValue[.
  **
  ** @param minValue : lower bound.
  ** @param maxValue : upper bound.
  **
  ** @return a float value uniformly from range [@a minValue, @a maxValue[
  */
  float sampleFloat(float minValue, float maxValue)
    {return minValue + sampleFloat() * (maxValue - minValue);}

  /** Samples a double value uniformly from range [0, 1[.
  **
  ** @return a double value uniformly from range [0, 1[
  */
  double sampleDouble()
    {return sampleUint32() / (double)0xffffffff;}

  /** Samples a double value uniformly from range [0, @a maxValue[.
  **
  ** @param maxValue : upper bound.
  **
  ** @return a double value uniformly from range [0, @a maxValue[
  */
  double sampleDouble(double maxValue)
    {return sampleDouble() * maxValue;}

  /** Samples a double value uniformly from range [@a minValue, @a maxValue[.
  **
  ** @param minValue : lower bound.
  ** @param maxValue : upper bound.
  **
  ** @return a double value uniformly from range [@a minValue, @a maxValue[
  */
  double sampleDouble(double minValue, double maxValue)
    {return minValue + sampleDouble() * (maxValue - minValue);}

  /** Samples a double value from a normal Gaussian distribution.
  **
  ** @return a double value sampled from a Gaussian
  ** distribution which mean is 0 and which standard deviation is 1.
  */
  double sampleDoubleFromGaussian();

  /** Samples a double value from a Gaussian distribution.
  **
  ** @param mean : gaussian mean.
  ** @param standardDeviation : gaussian standard deviation.
  **
  ** @return a double value sampled from the Gaussian distribution.
  */
  double sampleDoubleFromGaussian(double mean, double standardDeviation)
    {return sampleDoubleFromGaussian() * standardDeviation + mean;}

  /** Samples an ordering of fixed size.
  **
  ** This functions fills the vector @a res with all the indices
  ** comprised between 0 and @a size - 1 in a randomly sampled order.
  **
  ** @param size : number of item.
  ** @param res : vector to fill up.
  */
  inline void sampleOrder(size_t size, std::vector<size_t>& res)
    {sampleOrder(0, size, res);}

  /** Samples an ordering of fixed size.
  **
  ** This functions fills the vector @a res with all the indices
  ** comprised between @a begin and @a end - 1 in a randomly sampled order.
  **
  ** @param begin : first item index.
  ** @param end : last item index.
  ** @param res : vector to fill up.
  */
  inline void sampleOrder(size_t begin, size_t end, std::vector<size_t>& res)
  {
    jassert(end >= begin);
    res.resize(end - begin);
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = begin + i;
    for (size_t i = 1; i < res.size(); ++i)
      swap(res[i], res[sampleSize(i + 1)]);
  }

  inline void sampleSubset(const std::vector<size_t>& elements, size_t subsetSize, std::vector<size_t>& res)
  {
    jassert(subsetSize < elements.size());
    std::vector<size_t> order;
    sampleOrder(elements.size(), order);
    res.resize(subsetSize);
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = elements[order[i]];
  }

  inline void sampleSubset(const std::vector<size_t>& elements, size_t subsetSize, std::set<size_t>& res)
  {
    jassert(subsetSize < elements.size());
    std::vector<size_t> order;
    sampleOrder(elements.size(), order);
    for (size_t i = 0; i < subsetSize; ++i)
      res.insert(elements[order[i]]);
  }

  virtual ObjectPtr clone(ExecutionContext& context) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;
  
  static void initializeRandomGenerator();

  // Lua
  static int create(LuaState& state);
  static int sample(LuaState& state);

private:
  friend class RandomGeneratorClass;

  RandomGenerator(void* dummy) {}

  enum {N = 624};
  juce::uint32 mt[N];
  int mti;

  static juce::uint32 defaultSeed[N];

  /**
  ** Swaps @a a and @a b content.
  **
  ** @param a : first item.
  ** @param b : second item.
  */
  inline void swap(size_t& a, size_t& b)
    {size_t tmp = a; a = b; b = tmp;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_RANDOM_GENERATOR_H_
