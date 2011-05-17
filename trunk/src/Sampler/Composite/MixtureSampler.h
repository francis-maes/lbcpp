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

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples) const
  {
    // FIXME
  }

  double computeLogLikelihood(const DoubleMatrixPtr& rawProbabilities)
  {
    double ll = 0;
    for (size_t i = 0; i < rawProbabilities->getNumRows(); i++)
    {
      double accumulator = 0;
      for (size_t j = 0; j < rawProbabilities->getNumColumns(); j++)
        accumulator += rawProbabilities->getValue(i, j);
      ll += std::log(accumulator);
    }
    return ll;
  }

  void computeModelsProbabilities(const ContainerPtr& trainingSamples, DoubleMatrixPtr& rawProbabilities)
  {
    for (size_t j = 0; j < samplers.size(); j++)
    {
      (samplers[j].staticCast<ScalarContinuousSampler> ())->computeProbabilities(trainingSamples,
          rawProbabilities, j);

      for (size_t i = 0; i < trainingSamples->getNumElements(); i++)
        rawProbabilities->setValue(i, j, rawProbabilities->getValue(i, j)
            * probabilities->getValue(j));
    }
  }

  void normalizeModelsProbabilities(DoubleMatrixPtr& rawProbabilities)
  {
    for (size_t i = 0; i < rawProbabilities->getNumRows(); i++)
    {
      double Z = 0;
      for (size_t j = 0; j < samplers.size(); j++)
        Z += rawProbabilities->getValue(i, j);
      Z = 1 / Z;
      for (size_t j = 0; j < samplers.size(); j++)
        rawProbabilities->setValue(i, j, rawProbabilities->getValue(i, j) * Z);
    }
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, 
                                                const ContainerPtr& validationInputs, const ContainerPtr& validationSamples)
  {
    jassert(trainingSamples->getNumElements() > 1);
    DoubleMatrixPtr modelsProbabilities = new DoubleMatrix(trainingSamples->getNumElements(),
        samplers.size(), 0);

    // compute raw probabilities
    computeModelsProbabilities(trainingSamples, modelsProbabilities);

    double tolerance = 0.01;
    size_t maxIterations = 1000;
    double oldLL = computeLogLikelihood(modelsProbabilities);
    double newLL = oldLL + 2 * oldLL * tolerance;

    // EM
    for (size_t k = 0; (k < maxIterations) && (std::abs((oldLL - newLL) / oldLL) > tolerance); k++)
    {
      // normalize probabilities for a sample
      normalizeModelsProbabilities(modelsProbabilities);

      // update parameters of models
      for (size_t j = 0; j < samplers.size(); j++)
        (samplers[j].staticCast<ScalarContinuousSampler> ())->updateParameters(trainingSamples,
            modelsProbabilities, j);

      // update probabilities of mixture
      for (size_t j = 0; j < samplers.size(); j++)
      {
        double accumulatedProbabilities = 0;
        for (size_t i = 0; i < modelsProbabilities->getNumRows(); i++)
          accumulatedProbabilities += modelsProbabilities->getValue(i, j);
        probabilities->setValue(j, accumulatedProbabilities / modelsProbabilities->getNumRows());
      }

      computeModelsProbabilities(trainingSamples, modelsProbabilities);
      oldLL = newLL;
      newLL = computeLogLikelihood(modelsProbabilities);
    }
  }

protected:
  friend class MixtureSamplerClass;

  DenseDoubleVectorPtr probabilities;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_COMPOSITE_MIXTURE_H_
