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
**@brief  Random generator singleton.
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
** @brief Random generator singleton.
*/
class Random
{
public:
  /*!
  ** Singleton instance getter.
  **
  ** @return a reference on the Random singleton.
  */
  static Random& getInstance()
    {static Random instance(1664518616645186LL); return instance;}

  /*!
  ** Constructor.
  **
  ** @param seedValue : random seed.
  */
  Random(long long seedValue = 0)
    : seed(seedValue) {}

  /*!
  ** Constructor.
  **
  ** @param seedValue : random seed.
  */
  Random(int seedValue)
    {setSeed(seedValue);}

  /*!
  ** Seed setter.
  **
  ** @param seed : random seed.
  */
  void setSeed(int seed)
    {this->seed = (long long)seed; sampleInt();}

  /*!
  ** Seed setter.
  **
  ** @param seed1 : random seed.
  ** @param seed2 : random seed.
  */
  void setSeed(int seed1, int seed2);

  /*!
  ** Seed setter.
  **
  ** @param seed : random seed.
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


  /*!
  ** Returns a number in interval [0, probabilities.size()[ w.r.t. the probability distribution
  ** by default, the probabilities are not normalized
  ** probabilitiesSum = 0 => the probabilities sum will be computed
  ** probabilitiesSum > 0 => the probabilities sum is known in advance
  ** probabilitiesSum = 1 : normalized probability distribution, you may also use sampleWithNormalizedProbabilities()
  **
  ** @param probabilities : #FIXME
  ** @param probabilitiesSum : #FIXME
  **
  ** @return #FIXME
  */
  size_t sampleWithProbabilities(const std::vector<double>& probabilities, double probabilitiesSum = 0.0);

  /*!
  ** #FIXME
  **
  ** @param probabilities
  **
  ** @return
  */
  size_t sampleWithNormalizedProbabilities(const std::vector<double>& probabilities)
    {return sampleWithProbabilities(probabilities, 1.0);}

  /*!
  ** Return any integer value.
  **
  ** @return any integer value.
  */
  int sampleInt();

  /*!
  ** Return any interger value in range [0, @a maxValue[
  **
  ** @param maxValue : upper bound.
  **
  ** @return any interger value in range [0, maxValue[
  */
  int sampleInt(int maxValue)
    {assert(maxValue > 0); return (sampleInt() & 0x7fffffff) % maxValue;}

  /*!
  ** Return any integer value in range [@a minValue, @a maxValue[
  **
  ** @param minValue : lower bound.
  ** @param maxValue : upper bound.
  **
  ** @return any integer value in range [minValue, maxValue[
  */
  int sampleInt(int minValue, int maxValue)
    {assert(maxValue > minValue); return (sampleInt() & 0x7fffffff) % (maxValue - minValue) + minValue;}

  /*!
  ** Return any size_t type value in range [0, @a maxSize[
  **
  ** @param maxSize : upper bound.
  **
  ** @return any size_t type value in range [0, maxSize[
  */
  size_t sampleSize(size_t maxSize)
    {return (size_t)sampleInt((int)maxSize);}

  /*!
  ** Return any size_t type value in range [@a minSize, @a maxSize[
  **
  ** @param minSize : lower bound.
  ** @param maxSize : upper bound.
  **
  ** @return any size_t type value in range [minSize, maxSize[
  */
  size_t sampleSize(size_t minSize, size_t maxSize)
    {assert(maxSize > minSize); return (size_t)sampleInt((int)minSize, (int)maxSize);}

  /*!
  ** Return any float value in range [0, 1[
  **
  ** @return any float value in range [0, 1[
  */
  float sampleFloat()
    {return ((unsigned int) sampleInt()) / (float) 0xffffffff;}

  /*!
  ** Return any float value in range [0, @a maxValue[
  **
  ** @param maxValue : upper bound.
  **
  ** @return any float value in range [0, maxValue[
  */
  float sampleFloat(float maxValue)
    {return sampleFloat() * maxValue;}

  /*!
  ** Return any float value in range [@a minValue, @a maxValue[
  **
  ** @param minValue : lower bound.
  ** @param maxValue : upper bound.
  **
  ** @return any float value in range [minValue, maxValue[
  */
  float sampleFloat(float minValue, float maxValue)
    {return minValue + sampleFloat() * (maxValue - minValue);}

  /*!
  ** Return any double value in range [0, 1[
  **
  ** @return any double value in range [0, 1[
  */
  double sampleDouble()
    {return ((unsigned int) sampleInt()) / (double) 0xffffffff;}

  /*!
  ** Return any double value in range [0, @a maxValue[
  **
  ** @param maxValue : upper bound.
  **
  ** @return any double value in range [0, maxValue[
  */
  double sampleDouble(double maxValue)
    {return sampleDouble() * maxValue;}

  /*!
  ** Return any double value in range [@a minValue, @a maxValue[
  **
  ** @param minValue : lower bound.
  ** @param maxValue : upper bound.
  **
  ** @return any double value in range [minValue, maxValue[
  */
  double sampleDouble(double minValue, double maxValue)
    {return minValue + sampleDouble() * (maxValue - minValue);}

  /*!
  ** Return any double value in range [0, 1[ from a Gaussian distribution.
  **
  ** @return any double value in range [0, 1[ from a Gaussian distribution.
  */
  double sampleDoubleFromGaussian();

  /*!
  ** Return any double value in range [0, 1[ from a Gaussian
  ** distribution
  * (mean = @a mean, standard deviation = @a standardDeviation)
  **
  ** @param mean : gaussian mean.
  ** @param standardDeviation : gaussian standard deviation.
  **
  ** @return any double value in range [0, 1[ from a Gaussian distribution.
  */
  double sampleDoubleFromGaussian(double mean, double standardDeviation)
    {return sampleDoubleFromGaussian() * standardDeviation + mean;}

  /*!
  ** Mix @a res items.
  **
  ** @param size : vector size.
  ** @param res : vector to mix.
  */
  inline void sampleOrder(size_t size, std::vector<size_t>& res)
    {sampleOrder(0, size, res);}

  /*!
  ** Mix @a res items from @a begin to @a end.
  **
  ** @param begin : first item index.
  ** @param end : last item index.
  ** @param res : vector to mix.
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
  long long seed;               /*!< Random seed. */

  /*!
  ** Swap @a a and @a b content.
  **
  ** @param a : first item.
  ** @param b : second item.
  */
  inline void swap(size_t& a, size_t& b)
    {size_t tmp = a; a = b; b = tmp;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_RANDOM_H_
