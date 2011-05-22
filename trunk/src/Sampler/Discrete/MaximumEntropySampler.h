/*-----------------------------------------.---------------------------------.
| Filename: MaximumEntropySampler.h        | Maximum Entropy Sampler         |
| Author  : Francis Maes                   |                                 |
| Started : 22/05/2011 21:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_DISCRETE_MAXIMUM_ENTROPY_H_
# define LBCPP_SAMPLER_DISCRETE_MAXIMUM_ENTROPY_H_

# include <lbcpp/Learning/LossFunction.h>
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class MaximumEntropySampler : public DiscreteSampler
{
public:
  MaximumEntropySampler(TypePtr outputType) : outputType(outputType)
  {
    StochasticGDParametersPtr learnerParameters = new StochasticGDParameters(constantIterationFunction(1.0), maxIterationsStoppingCriterion(100));
    learnerParameters->setLossFunction(logBinomialMultiClassLossFunction());
    learnerParameters->setEvaluateAtEachIteration(false);

    predictor = linearMultiClassClassifier(learnerParameters);
    predictor->setEvaluator(EvaluatorPtr()); // todo: log-likelyhood evaluator
  }

  MaximumEntropySampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    DenseDoubleVectorPtr probabilities = predictor->compute(context, inputs[0], Variable::missingValue(outputType)).getObjectAndCast<DenseDoubleVector>();
    if (!probabilities)
      return Variable();
    return Variable(random->sampleWithNormalizedProbabilities(probabilities->getValues()), probabilities->getElementsEnumeration());
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                    const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
  {
    ContainerPtr trainingData = Container::makePairsContainer(trainingInputs, trainingSamples);
    ContainerPtr validationData;
    if (validationSamples)
      validationData = Container::makePairsContainer(validationInputs, validationSamples);
    predictor->train(context, trainingData, validationData, T("Training maxent"));
  }

protected:
  friend class MaximumEntropySamplerClass;

  TypePtr outputType; // tmp, todo: merge with Functions
  FunctionPtr predictor;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_DISCRETE_MAXIMUM_ENTROPY_H_
