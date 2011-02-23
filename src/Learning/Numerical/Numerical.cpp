/*-----------------------------------------.---------------------------------.
| Filename: Numerical.cpp                  | Numerical Learning general stuff|
| Author  : Julien Becker                  |                                 |
| Started : 16/02/2011 20:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Learning/Numerical.h>
using namespace lbcpp;

/*
** StochasticGDParameters
*/
StochasticGDParameters::StochasticGDParameters(IterationFunctionPtr learningRate,
                                                StoppingCriterionPtr stoppingCriterion,
                                                size_t maxIterations,
                                                EvaluatorPtr evaluator,
                                                bool doPerEpisodeUpdates,
                                                bool normalizeLearningRate,
                                                bool restoreBestParameters,
                                                bool randomizeExamples)
  : learningRate(learningRate), stoppingCriterion(stoppingCriterion), maxIterations(maxIterations), 
    evaluator(evaluator), doPerEpisodeUpdates(doPerEpisodeUpdates), normalizeLearningRate(normalizeLearningRate), 
    restoreBestParameters(restoreBestParameters), randomizeExamples(randomizeExamples)
{
}

BatchLearnerPtr StochasticGDParameters::createBatchLearner(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, const TypePtr& outputType) const
{
  return stochasticBatchLearner(maxIterations, randomizeExamples);
}

OnlineLearnerPtr StochasticGDParameters::createOnlineLearner(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, const TypePtr& outputType) const
{
  FunctionPtr lossFunction = this->lossFunction;

  if (inputVariables.size() > 1)
  {
    TypePtr supervisionType = inputVariables[1]->getType();

    // get or create loss function
    if (!lossFunction)
    {
      // create default loss function
      if (supervisionType == booleanType || supervisionType == probabilityType)
        lossFunction = hingeDiscriminativeLossFunction();
      else if (supervisionType == doubleType)
        lossFunction = squareRegressionLossFunction();
      else if (supervisionType->inheritsFrom(enumValueType) || supervisionType->inheritsFrom(doubleVectorClass(enumValueType, probabilityType)))
        lossFunction = oneAgainstAllMultiClassLossFunction(hingeDiscriminativeLossFunction());
      else
      {
        context.errorCallback(T("Could not create default loss function for type ") + supervisionType->getName());
        return OnlineLearnerPtr();
      }
    }

    // initialize loss function
    if (!lossFunction->initialize(context, outputType, supervisionType))
      return OnlineLearnerPtr();
  }

  // create gradient descent learner
  std::vector<OnlineLearnerPtr> learners;
  learners.push_back(doPerEpisodeUpdates
    ? perEpisodeGDOnlineLearner(lossFunction, learningRate, normalizeLearningRate)
    : stochasticGDOnlineLearner(lossFunction, learningRate, normalizeLearningRate));

  // create other optionnal online learners
  if (evaluator)
    learners.push_back(evaluatorOnlineLearner(evaluator));
  if (stoppingCriterion)
    learners.push_back(stoppingCriterionOnlineLearner(stoppingCriterion));
  if (restoreBestParameters)
    learners.push_back(restoreBestParametersOnlineLearner());

  return learners.size() == 1 ? learners[0] : compositeOnlineLearner(learners);
}

/*
** SupervisedNumericalFunction
*/
void SupervisedNumericalFunction::buildFunction(CompositeFunctionBuilder& builder)
{
  size_t input = builder.addInput(doubleVectorClass());
  size_t supervision = builder.addInput(anyType);

  FunctionPtr learnableFunctionFunction = createLearnableFunction();
  size_t learnableFunction = builder.addFunction(learnableFunctionFunction, input, supervision);
  learnableFunctionFunction->setOnlineLearner(learnerParameters->createOnlineLearner(builder.getContext(), builder.getProvidedInputs(), builder.getOutputType()));

  FunctionPtr postProcessingFunction = createPostProcessing();
  if (postProcessingFunction)
    builder.addFunction(postProcessingFunction, learnableFunction);

  setBatchLearner(learnerParameters->createBatchLearner(builder.getContext(), builder.getProvidedInputs(), builder.getOutputType()));
}
