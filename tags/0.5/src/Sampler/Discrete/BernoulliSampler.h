/*-----------------------------------------.---------------------------------.
| Filename: BernoulliSampler.h             | Bernoulli Sampler               |
| Author  : Francis Maes                   |                                 |
| Started : 17/05/2011 12:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_DISCRETE_BERNOULLI_H_
# define LBCPP_SAMPLER_DISCRETE_BERNOULLI_H_

# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class BernoulliSampler : public DiscreteSampler
{
public:
  BernoulliSampler(double p = 0.5, double minProbability = 0.0, double maxProbability = 1.0)
    : probability(p), minProbability(minProbability), maxProbability(maxProbability) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
    {return random->sampleBool(probability);}

  virtual Variable computeExpectation(const Variable* inputs = NULL) const
    {return Variable(probability, probabilityType);}

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                    const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
  {
    size_t n = trainingSamples->getNumElements();
    jassert(n);

    size_t numTrue = 0;

    for (size_t i = 0; i < n; i++)
    {
      bool value = trainingSamples->getElement(i).getBoolean();
      if (value)
        ++numTrue;
    }
    probability = juce::jlimit(minProbability, maxProbability, numTrue / (double)n);
  }

protected:
  friend class BernoulliSamplerClass;

  double probability;
  double minProbability;
  double maxProbability;
};

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_DISCRETE_BERNOULLI_H_
