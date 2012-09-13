/*-----------------------------------------.---------------------------------.
| Filename: DiagonalGaussianContinuousSampler.h | Diagonal Gaussian sampler  |
| Author  : Francis Maes                   |  in R^n                         |
| Started : 13/09/2012 11:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_SAMPLER_DIAGONAL_GAUSSIAN_CONTINUOUS_H_
# define LBCPP_MOO_SAMPLER_DIAGONAL_GAUSSIAN_CONTINUOUS_H_

# include "MOOCore.h"

namespace lbcpp
{

class DiagonalGaussianContinuousSampler : public MOOSampler
{
public:
  virtual void initialize(ExecutionContext& context, const MOODomainPtr& d)
  {
    domain = d.staticCast<ContinuousMOODomain>();
    jassert(domain);
    size_t n = domain->getNumDimensions();
  
    if (!mean)
    {
      mean = new DenseDoubleVector(n, 0.0);
      for (size_t i = 0; i < n; ++i)
        mean->setValue(i, (domain->getLowerLimit(i) + domain->getUpperLimit(i)) / 2.0);
    }
    if (!stddev)
    {
      stddev = new DenseDoubleVector(n, 0.0);
      for (size_t i = 0; i < n; ++i)
        stddev->setValue(i, (domain->getUpperLimit(i) - domain->getLowerLimit(i)) / 6.0);
    }
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    size_t n = domain->getNumDimensions();
    jassert(mean && n == mean->getNumValues() && stddev && n == stddev->getNumValues());

    DenseDoubleVectorPtr res = new DenseDoubleVector(n, 0.0);
    for (size_t i = 0; i < n; ++i)
      res->setValue(i, random->sampleDoubleFromGaussian(mean->getValue(i), stddev->getValue(i)));
    return res;
  }

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& solutions)
  {
    size_t n = domain->getNumDimensions();
    jassert(mean && n == mean->getNumValues() && stddev && n == stddev->getNumValues());

    std::vector<ScalarVariableMeanAndVariance> stats(domain->getNumDimensions());
    for (size_t i = 0; i < solutions.size(); ++i)
    {
      DenseDoubleVectorPtr solution = solutions[i].staticCast<DenseDoubleVector>();
      for (size_t j = 0; j < n; ++j)
        stats[j].push(solution->getValue(j));
    }

    for (size_t i = 0; i < n; ++i)
    {
      mean->setValue(i, stats[i].getMean());
      stddev->setValue(i, stats[i].getStandardDeviation());
    }    
  }

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& solution)
    {jassertfalse;}

protected:
  friend class DiagonalGaussianContinuousSamplerClass;

  DenseDoubleVectorPtr mean;
  DenseDoubleVectorPtr stddev;

  ContinuousMOODomainPtr domain;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SAMPLER_DIAGONAL_GAUSSIAN_CONTINUOUS_H_
