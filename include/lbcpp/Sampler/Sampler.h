/*-----------------------------------------.---------------------------------.
| Filename: Sampler.h                      | Sampler base class              |
| Author  : Francis Maes                   |                                 |
| Started : 13/05/2011 16:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_H_
# define LBCPP_SAMPLER_H_

# include "../Data/RandomGenerator.h"
# include "../Core/Variable.h"

namespace lbcpp
{ 
  
class Sampler;
typedef ReferenceCountedObjectPtr<Sampler> SamplerPtr;

class Sampler : public Object
{
public:
  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const = 0;

  virtual void learn(ExecutionContext& context, const std::vector<std::pair<Variable, Variable> >& dataset) = 0;

protected:
  friend class SamplerClass;
};

class ContinuousSampler;
typedef ReferenceCountedObjectPtr<ContinuousSampler> ContinuousSamplerPtr;

class ContinuousSampler : public Sampler
{
public:/*
  ContinuousSampler()
    : mean(0), std(1)
  {
  }

  ContinuousSampler(double mean, double std)
    : mean(mean), std(std)
  {
  }*/

  /**
   * dataset = first : a Variable of double type containing the data observed.
   *           second : not yet used.
   */
  double getMean(const std::vector<std::pair<Variable, Variable> >& dataset)
  {
    double temporaryMean = 0;
    if (dataset.size() > 0)
    {
      for (size_t i = 0; i < dataset.size(); i++)
        temporaryMean += dataset[i].first.getDouble();
      temporaryMean = temporaryMean / dataset.size();
    }
    return temporaryMean;
  }

  /**
   * dataset = first : a Variable of double type containing the data observed.
   *           second : not yet used.
   */
  double getVariance(const std::vector<std::pair<Variable, Variable> >& dataset, double mean)
  {
    double temporaryVariance = 0;
    if (dataset.size() > 0)
    {
      for (size_t i = 0; i < dataset.size(); i++)
        temporaryVariance += std::pow(dataset[i].first.getDouble() - mean, 2);
      temporaryVariance = temporaryVariance / dataset.size();
    }
    return temporaryVariance;
  }

protected:
//  friend class ContinuousSamplerClass;
//  double mean;
//  double std;
};

extern ContinuousSamplerPtr gaussianSampler(double mean, double stddev);

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_H_
