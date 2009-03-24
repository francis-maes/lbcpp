/*-----------------------------------------.---------------------------------.
| Filename: Random.h                       | Random generator                |
| Author  : Francis Maes                   |                                 |
| Started : 10/03/2009 20:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_RANDOM_H_
# define CRALGO_RANDOM_H_

# include <cmath>
# include <vector>
# include <cassert>

namespace cralgo
{

class Random
{
public:
  static Random& getInstance()
    {static Random instance(1664518616645186LL); return instance;}

  Random(long long seedValue = 0)
    : seed(seedValue) {}
  Random(int seedValue)
    {setSeed(seedValue);}
  
  void setSeed(int seed)
    {this->seed = (long long)seed; sampleInt();}
    
  void setSeed(int seed1, int seed2);
    
  void setSeed(long long seed)
    {this->seed = seed;}

  bool sampleBool()
    {return (sampleInt() & 0x80000000) != 0;}

  bool sampleBool(double probabilityOfTrue)
    {return probabilityOfTrue && sampleDouble() <= probabilityOfTrue;}
    
  // returns a number in interval [0, probabilities.size()[ w.r.t. the probability distribution
  // by default, the probabilities are not normalized
  // probabilitiesSum = 0 => the probabilities sum will be computed
  // probabilitiesSum > 0 => the probabilities sum is known in advance
  // probabilitiesSum = 1 : normalized probability distribution, you may also use sampleWithNormalizedProbabilities()
  size_t sampleWithProbabilities(const std::vector<double>& probabilities, double probabilitiesSum = 0.0);
  
  size_t sampleWithNormalizedProbabilities(const std::vector<double>& probabilities)
    {return sampleWithProbabilities(probabilities, 1.0);}

  int sampleInt(); // any integer value
  
  int sampleInt(int maxValue) // in range [0, maxValue[
    {assert(maxValue > 0); return (sampleInt() & 0x7fffffff) % maxValue;}

  int sampleInt(int minValue, int maxValue) // in range [minValue, maxValue[
    {assert(maxValue > minValue); return (sampleInt() & 0x7fffffff) % (maxValue - minValue) + minValue;}

  size_t sampleSize(size_t maxSize) // in range [0, maxSize[
    {return (size_t)sampleInt((int)maxSize);}

  size_t sampleSize(size_t minSize, size_t maxSize) // in range [minSize, maxSize[
    {assert(maxSize > minSize); return (size_t)sampleInt((int)minSize, (int)maxSize);}
    
  float sampleFloat() // in range [0, 1[
    {return ((unsigned int) sampleInt()) / (float) 0xffffffff;}

  double sampleDouble() // in range [0, 1[
    {return ((unsigned int) sampleInt()) / (double) 0xffffffff;}

  double sampleDouble(double maxValue) // in range [0, maxValue]
    {return sampleDouble() * maxValue;}
    
  double sampleDouble(double minValue, double maxValue)
    {return minValue + sampleDouble() * (maxValue - minValue);}
  
  double sampleDoubleFromGaussian();
  
  double sampleDoubleFromGaussian(double mean, double standardDeviation)
    {return sampleDoubleFromGaussian() * standardDeviation + mean;}

  inline void sampleOrder(size_t size, std::vector<size_t>& res)
    {sampleOrder(0, size, res);}
  
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
  long long seed;
  
  inline void swap(size_t& a, size_t& b)
    {size_t tmp = a; a = b; b = tmp;}
};

}; /* namespace cralgo */

#endif // !CRALGO_RANDOM_H_
