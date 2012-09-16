/*-----------------------------------------.---------------------------------.
| Filename: DiagonalGaussianSampler.h      | Diagonal Gaussian sampler in R^n|
| Author  : Francis Maes                   |                                 |
| Started : 13/09/2012 11:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_SAMPLER_DIAGONAL_GAUSSIAN_H_
# define LBCPP_MOO_SAMPLER_DIAGONAL_GAUSSIAN_H_

# include "MOOCore.h"

namespace lbcpp
{

class DiagonalGaussianSampler : public MOOSampler
{
public:
  DiagonalGaussianSampler(double learningRate = 0.1)
    : learningRate(learningRate), epoch(0) {}

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

    initialStddev = stddev->cloneAndCast<DenseDoubleVector>();
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

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects)
  {
    size_t n = domain->getNumDimensions();
    jassert(mean && n == mean->getNumValues() && stddev && n == stddev->getNumValues());

    std::vector<ScalarVariableMeanAndVariance> stats(domain->getNumDimensions());
    for (size_t i = 0; i < objects.size(); ++i)
    {
      DenseDoubleVectorPtr vector = objects[i].staticCast<DenseDoubleVector>();
      for (size_t j = 0; j < n; ++j)
        stats[j].push(vector->getValue(j));
    }

    for (size_t i = 0; i < n; ++i)
    {
      mean->setValue(i, stats[i].getMean());
      stddev->setValue(i, stats[i].getStandardDeviation());
    } 
  }

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object)
  {
    static const double minStddev = 1e-6;
    DenseDoubleVectorPtr vector = object.staticCast<DenseDoubleVector>();

    size_t n = domain->getNumDimensions();
    jassert(vector->getNumValues() == n);
    for (size_t i = 0; i < n; ++i)
    {
      double value = vector->getValue(i);
      double& mean = this->mean->getValueReference(i);
      double& stddev = this->stddev->getValueReference(i);

      //mean += (value - mean) * learningRate;
      mean = value;
      stddev *= 1.0 - learningRate;
      if (stddev < minStddev)
        stddev = 0.0;
#if 0

      if (stddev == minStddev)
        mean = value; // gaussian has converged
      else
      {
        // gradient step
        double normalizedValue = (value - mean) / stddev;
        double invStddev = 1.0 / stddev;
        double derivativeWrtMean = normalizedValue * invStddev;
        double derivativeWrtStddev = (normalizedValue * normalizedValue - 1.0) * invStddev;

        /*if (i == 0)
        {
          context.enterScope(String((int)epoch));
          context.resultCallback("epoch", epoch);
          context.resultCallback("value", value);
          context.resultCallback("mean", mean);
          context.resultCallback("stddev", stddev);
          context.resultCallback("normalizedValue", normalizedValue);
          context.resultCallback("dmean", learningRate * derivativeWrtMean);
          context.resultCallback("dstddev", learningRate * derivativeWrtStddev);
          context.leaveScope();
          ++epoch;
        }*/

        mean += learningRate * derivativeWrtMean;
        stddev += learningRate * derivativeWrtStddev;
        stddev = juce::jlimit(minStddev, initialStddev->getValue(i), stddev);
      }
#endif // 0
    }
    ++epoch;
  }
  
  virtual bool isDegenerate() const
    {return stddev->l2norm() < 1e-12;}
    
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const ReferenceCountedObjectPtr<DiagonalGaussianSampler>& target = t.staticCast<DiagonalGaussianSampler>();
    target->learningRate = learningRate;
    target->mean = mean ? mean->cloneAndCast<DenseDoubleVector>() : DenseDoubleVectorPtr();
    target->stddev = stddev ? stddev->cloneAndCast<DenseDoubleVector>() : DenseDoubleVectorPtr();
    target->initialStddev = initialStddev;
    target->epoch = epoch;
    target->domain = domain;
  }

protected:
  friend class DiagonalGaussianSamplerClass;

  double learningRate;
  DenseDoubleVectorPtr mean;
  DenseDoubleVectorPtr stddev;

  ContinuousMOODomainPtr domain;

  size_t epoch;
  DenseDoubleVectorPtr initialStddev;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SAMPLER_DIAGONAL_GAUSSIAN_H_
