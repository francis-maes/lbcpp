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

BatchLearnerPtr StochasticGDParameters::createBatchLearner(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables) const
{
  return stochasticBatchLearner(maxIterations, randomizeExamples);
}

OnlineLearnerPtr StochasticGDParameters::createOnlineLearner(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables) const
{
  std::vector<OnlineLearnerPtr> learners;

  FunctionPtr lossFunction = this->lossFunction;
  if (!lossFunction)
  {
    TypePtr supervisionType = inputVariables[1]->getType();
    if (supervisionType == booleanType)
      lossFunction = hingeDiscriminativeLossFunction();
    else if (supervisionType == doubleType)
      lossFunction = squareRegressionLossFunction();
    else if (supervisionType->inheritsFrom(enumValueType))
      lossFunction = oneAgainstAllMultiClassLossFunction(hingeDiscriminativeLossFunction());
    else
    {
      context.errorCallback(T("Could not create default loss function for type ") + supervisionType->getName());
      return OnlineLearnerPtr();
    }
  }

  learners.push_back(doPerEpisodeUpdates
    ? perEpisodeGDOnlineLearner(lossFunction, learningRate, normalizeLearningRate)
    : stochasticGDOnlineLearner(lossFunction, learningRate, normalizeLearningRate));

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
SupervisedNumericalFunction::SupervisedNumericalFunction(LearnerParametersPtr learnerParameters)
  : learnerParameters(learnerParameters)
{
}

// TODO: replace by "ComposeFunction"
TypePtr SupervisedNumericalFunction::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());

  frameClass = new FrameClass(getClassName() + T("Frame"));
  frameClass->addMemberVariable(context, inputVariables[0]->getType(), T("input"));             // 0: input
  frameClass->addMemberVariable(context, inputVariables[1]->getType(), T("supervision"));       // 1: supervision

  FunctionPtr learnableFunction = createLearnableFunction();
  learnableFunction->setOnlineLearner(learnerParameters->createOnlineLearner(context, inputVariables));
  frameClass->addMemberOperator(context, learnableFunction, 0, 1);                              // 2: linearFunction(0,1)

  FunctionPtr postProcessing = createPostProcessing();
  if (postProcessing)
    frameClass->addMemberOperator(context, postProcessing, 2);                                  // 3: postProcess(2)

  setBatchLearner(learnerParameters->createBatchLearner(context, inputVariables));
  return FrameBasedFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
}
