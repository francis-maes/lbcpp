/*-----------------------------------------.---------------------------------.
| Filename: DiscretizeSampler.h            | DiscretizeSampler               |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 14 mai 2011  18:26:22          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_DISCRETE_DISCRETIZE_H_
# define LBCPP_SAMPLER_DISCRETE_DISCRETIZE_H_

# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class DiscretizeSampler : public DiscreteSampler
{
public:
  DiscretizeSampler(const ContinuousSamplerPtr& sampler, int minValue, int maxValue)
    : sampler(sampler), minValue(minValue), maxValue(maxValue)
  {
  }

  DiscretizeSampler() : minValue(0), maxValue(1) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    double doubleSample = sampler->sample(context, random, inputs).getDouble();
    return juce::jlimit(minValue, maxValue, juce::roundDoubleToInt(doubleSample));
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                    const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
  {
    DenseDoubleVectorPtr doubleTrainingSamples = convertToDouble(trainingSamples);
    if (validationSamples)
    {
      DenseDoubleVectorPtr doubleValidationSamples = convertToDouble(validationSamples);
      sampler->learn(context, trainingInputs, doubleTrainingSamples, trainingWeights, validationInputs, doubleValidationSamples, supervisionWeights);
    }
    else
      sampler->learn(context, trainingInputs, doubleTrainingSamples);
  }

protected:
  friend class DiscretizeSamplerClass;

  ContinuousSamplerPtr sampler;
  int minValue;
  int maxValue;

  static DenseDoubleVectorPtr convertToDouble(const ContainerPtr& samples)
  {
    size_t n = samples->getNumElements();
    DenseDoubleVectorPtr res = new DenseDoubleVector(n, 0.0);
    for (size_t i = 0; i < n; ++i)
      res->setValue(i, samples->getElement(i).toDouble());
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_DISCRETE_DISCRETIZE_H_
