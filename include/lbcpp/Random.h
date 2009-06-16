/*-----------------------------------------.---------------------------------.
| Filename: Random.h                       | Random generator                |
| Author  : Francis Maes                   |                                 |
| Started : 10/03/2009 20:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   Random.h
**@author Francis MAES
**@date   Mon Jun 15 23:39:13 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_RANDOM_H_
# define LBCPP_RANDOM_H_

# include <cmath>
# include <vector>
# include <cassert>

namespace lbcpp
{

/*!
** @class Random
** @brief
*/
class Random
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  static Random& getInstance()
    {static Random instance(1664518616645186LL); return instance;}

  /*!
  **
  **
  ** @param seedValue
  */
  Random(long long seedValue = 0)
    : seed(seedValue) {}

  /*!
  **
  **
  ** @param seedValue
  */
  Random(int seedValue)
    {setSeed(seedValue);}

  /*!
  **
  **
  ** @param seed
  */
  void setSeed(int seed)
    {this->seed = (long long)seed; sampleInt();}

  /*!
  **
  **
  ** @param seed1
  ** @param seed2
  */
  void setSeed(int seed1, int seed2);

  /*!
  **
  **
  ** @param seed
  */
  void setSeed(long long seed)
    {this->seed = seed;}

  /*!
  **
  **
  **
  ** @return
  */
  bool sampleBool()
    {return (sampleInt() & 0x80000000) != 0;}

  /*!
  **
  **
  ** @param probabilityOfTrue
  **
  ** @return
  */
  bool sampleBool(double probabilityOfTrue)
    {return probabilityOfTrue && sampleDouble() <= probabilityOfTrue;}

  // returns a number in interval [0, probabilities.size()[ w.r.t. the probability distribution
  // by default, the probabilities are not normalized
  // probabilitiesSum = 0 => the probabilities sum will be computed
  // probabilitiesSum > 0 => the probabilities sum is known in advance
  // probabilitiesSum = 1 : normalized probability distribution, you may also use sampleWithNormalizedProbabilities()
  /*!
  **
  **
  ** @param probabilities
  ** @param probabilitiesSum
  **
  ** @return
  */
  size_t sampleWithProbabilities(const std::vector<double>& probabilities, double probabilitiesSum = 0.0);

  /*!
  **
  **
  ** @param probabilities
  **
  ** @return
  */
  size_t sampleWithNormalizedProbabilities(const std::vector<double>& probabilities)
    {return sampleWithProbabilities(probabilities, 1.0);}

  /*!
  **
  **
  **
  ** @return
  */
  int sampleInt(); // any integer value

  /*!
  **
  **
  ** @param maxValue
  **
  ** @return
  */
  int sampleInt(int maxValue) // in range [0, maxValue[
    {assert(maxValue > 0); return (sampleInt() & 0x7fffffff) % maxValue;}

  /*!
  **
  **
  ** @param minValue
  ** @param maxValue
  **
  ** @return
  */
  int sampleInt(int minValue, int maxValue) // in range [minValue, maxValue[
    {assert(maxValue > minValue); return (sampleInt() & 0x7fffffff) % (maxValue - minValue) + minValue;}

  /*!
  **
  **
  ** @param maxSize
  **
  ** @return
  */
  size_t sampleSize(size_t maxSize) // in range [0, maxSize[
    {return (size_t)sampleInt((int)maxSize);}

  /*!
  **
  **
  ** @param minSize
  ** @param maxSize
  **
  ** @return
  */
  size_t sampleSize(size_t minSize, size_t maxSize) // in range [minSize, maxSize[
    {assert(maxSize > minSize); return (size_t)sampleInt((int)minSize, (int)maxSize);}

  /*!
  **
  **
  **
  ** @return
  */
  float sampleFloat() // in range [0, 1[
    {return ((unsigned int) sampleInt()) / (float) 0xffffffff;}

  /*!
  **
  **
  ** @param maxValue
  **
  ** @return
  */
  float sampleFloat(float maxValue) // in range [0, maxValue[
    {return sampleFloat() * maxValue;}

  /*!
  **
  **
  ** @param minValue
  ** @param maxValue
  **
  ** @return
  */
  float sampleFloat(float minValue, float maxValue) // in range [minValue, maxValue[
    {return minValue + sampleFloat() * (maxValue - minValue);}

  /*!
  **
  **
  **
  ** @return
  */
  double sampleDouble() // in range [0, 1[
    {return ((unsigned int) sampleInt()) / (double) 0xffffffff;}

  /*!
  **
  **
  ** @param maxValue
  **
  ** @return
  */
  double sampleDouble(double maxValue) // in range [0, maxValue[
    {return sampleDouble() * maxValue;}

  /*!
  **
  **
  ** @param minValue
  ** @param maxValue
  **
  ** @return
  */
  double sampleDouble(double minValue, double maxValue) // in range [minValue, maxValue[
    {return minValue + sampleDouble() * (maxValue - minValue);}

  /*!
  **
  **
  **
  ** @return
  */
  double sampleDoubleFromGaussian();

  /*!
  **
  **
  ** @param mean
  ** @param standardDeviation
  **
  ** @return
  */
  double sampleDoubleFromGaussian(double mean, double standardDeviation)
    {return sampleDoubleFromGaussian() * standardDeviation + mean;}

  /*!
  **
  **
  ** @param size
  ** @param res
  */
  inline void sampleOrder(size_t size, std::vector<size_t>& res)
    {sampleOrder(0, size, res);}

  /*!
  **
  **
  ** @param begin
  ** @param end
  ** @param res
  */
  inline void sampleOrder(size_t begin, size_t end, std::vector<size_t>& res)
  {
    assert(end > begin);
    res.resize(end - begin);
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = begin + i;
    for (size_t i = 1; i < res.size(); ++i)
      swap(res[i], res[sampleSize(i + 1)]);
  }

private:
  long long seed;               /*!< */

  /*!
  **
  **
  ** @param a
  ** @param b
  */
  inline void swap(size_t& a, size_t& b)
    {size_t tmp = a; a = b; b = tmp;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_RANDOM_H_
