/*-----------------------------------------.---------------------------------.
| Filename: UniformScalarSampler.h         | Uniform Scalar Sampler          |
| Author  : Francis Maes                   |                                 |
| Started : 17/05/2011 13:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_CONTINUOUS_UNIFORM_SCALAR_H_
# define LBCPP_SAMPLER_CONTINUOUS_UNIFORM_SCALAR_H_

# include <lbcpp/Sampler/Sampler.h>
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class UniformScalarSampler : public ScalarContinuousSampler
{
public:
  UniformScalarSampler(double minValue = 0.0, double maxValue = 1.0)
    : minValue(minValue), maxValue(maxValue) {}

  virtual Variable computeExpectation(const Variable* inputs = NULL) const
    {return (minValue + maxValue) / 2.0;}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
    {return random->sampleDouble(minValue, maxValue);}

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, 
                                                const ContainerPtr& validationInputs, const ContainerPtr& validationSamples)
  {
    size_t n = trainingSamples->getNumElements();

    minValue = DBL_MAX;
    maxValue = -DBL_MAX;
    for (size_t i = 0; i < n; i++)
    {
      double v = trainingSamples->getElement(i).getDouble();
      if (v < minValue)
        minValue = v;
      if (v > maxValue)
        maxValue = v;
    }
  }

protected:
  friend class UniformScalarSamplerClass;

  double minValue;
  double maxValue;
};

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_CONTINUOUS_UNIFORM_SCALAR_H_
