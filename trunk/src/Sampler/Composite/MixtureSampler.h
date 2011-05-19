/*-----------------------------------------.---------------------------------.
| Filename: MixtureSampler.h               | Mixture Sampler                 |
| Author  : Francis Maes                   |                                 |
| Started : 13/05/2011 20:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_COMPOSITE_MIXTURE_H_
# define LBCPP_SAMPLER_COMPOSITE_MIXTURE_H_

# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class MixtureSampler : public CompositeSampler
{
public:
  MixtureSampler(const DenseDoubleVectorPtr& probabilities, const std::vector<SamplerPtr>& samplers)
    : CompositeSampler(samplers), probabilities(probabilities) {}
  MixtureSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    jassert(probabilities->getNumValues() == samplers.size());
    size_t index = random->sampleWithProbabilities(probabilities->getValues());
    return samplers[index]->sample(context, random, inputs);
  }

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, const DenseDoubleVectorPtr& weights,
                               std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
  {
    // FIXME
  }

  double computeLogLikelihood(const std::vector<DenseDoubleVectorPtr>& rawProbabilities)
  {
    jassert(rawProbabilities.size());
    size_t numSamples = rawProbabilities[0]->getNumElements();
    double ll = 0;
    for (size_t i = 0; i < numSamples; i++)
    {
      double accumulator = 0;
      for (size_t j = 0; j < rawProbabilities.size(); j++)
        accumulator += rawProbabilities[j]->getValue(i);
      ll += std::log(accumulator);
    }
    ll = ll / (double)numSamples;
    return ll;
  }

  void computeProbabilitiesForAllModels(const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples,
      std::vector<DenseDoubleVectorPtr>& rawProbabilities)
  {
    for (size_t j = 0; j < samplers.size(); j++)
    {
      rawProbabilities[j] = samplers[j]->computeProbabilities(trainingInputs, trainingSamples);

      for (size_t i = 0; i < trainingSamples->getNumElements(); i++)
        rawProbabilities[j]->setValue(i, rawProbabilities[j]->getValue(i) * probabilities->getValue(j));
    }
  }

  void normalizeProbabilitiesForAllModels(std::vector<DenseDoubleVectorPtr>& rawProbabilities)
  {
    jassert(rawProbabilities.size());
    size_t numSamples = rawProbabilities[0]->getNumElements();
    for (size_t i = 0; i < numSamples; i++)
    {
      double Z = 0;
      for (size_t j = 0; j < samplers.size(); j++)
        Z += rawProbabilities[j]->getValue(i);
      Z = 1.0 / Z;
      for (size_t j = 0; j < samplers.size(); j++)
        rawProbabilities[j]->setValue(i, rawProbabilities[j]->getValue(i) * Z);
    }
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                    const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& validationWeights)
  {
    size_t numSamples = trainingSamples->getNumElements();
    jassert(numSamples > 0);
    std::vector<DenseDoubleVectorPtr> modelsProbabilities(samplers.size());

    // compute raw probabilities
    computeProbabilitiesForAllModels(trainingInputs, trainingSamples, modelsProbabilities);

    double tolerance = 0.01;
    size_t maxIterations = 1000;
    double oldLL = computeLogLikelihood(modelsProbabilities);
    double newLL = oldLL + 2 * oldLL * tolerance;

    // EM
    for (size_t k = 0; (k < maxIterations) && (std::abs((oldLL - newLL) / oldLL) > tolerance); k++)
    {
      // normalize probabilities for a sample
      normalizeProbabilitiesForAllModels(modelsProbabilities);

      // update parameters of models
      for (size_t j = 0; j < samplers.size(); j++)
        samplers[j]->learn(context, trainingInputs, trainingSamples, modelsProbabilities[j], validationInputs,
            validationSamples, validationWeights);

      // update probabilities of mixture
      for (size_t j = 0; j < samplers.size(); j++)
      {
        double accumulatedProbabilities = 0;
        for (size_t i = 0; i < numSamples; i++)
          accumulatedProbabilities += modelsProbabilities[j]->getValue(i);
        probabilities->setValue(j, accumulatedProbabilities / (double)numSamples);
      }

      computeProbabilitiesForAllModels(trainingInputs, trainingSamples, modelsProbabilities);
      oldLL = newLL;
      newLL = computeLogLikelihood(modelsProbabilities);
    }
  }

  virtual DenseDoubleVectorPtr computeProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(samples->getNumElements(), 0);
    for (size_t i = 0; i < probabilities->getNumElements(); i++)
    {
      DenseDoubleVectorPtr tempProbabilities = samplers[i]->computeProbabilities(inputs, samples);
      tempProbabilities->addWeightedTo(result, 0, probabilities->getValue(i));
    }
    return result;
  }

protected:
  friend class MixtureSamplerClass;

  DenseDoubleVectorPtr probabilities;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_COMPOSITE_MIXTURE_H_
