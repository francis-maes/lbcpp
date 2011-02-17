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

BatchLearnerPtr StochasticGDParameters::createBatchLearner() const
{
  return stochasticBatchLearner(maxIterations, randomizeExamples);
}

OnlineLearnerPtr StochasticGDParameters::createOnlineLearner() const
{
  std::vector<OnlineLearnerPtr> learners;

  learners.push_back(doPerEpisodeUpdates ? perEpisodeGDOnlineLearner(learningRate, normalizeLearningRate) : stochasticGDOnlineLearner(learningRate, normalizeLearningRate));
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
SupervisedNumericalFunction::SupervisedNumericalFunction(LearnerParametersPtr learnerParameters, ClassPtr lossFunctionClass)
  : learnerParameters(learnerParameters), lossFunctionClass(lossFunctionClass)
{
}

TypePtr SupervisedNumericalFunction::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());

  frameClass = new FrameClass(getClassName() + T("Frame"));
  frameClass->addMemberVariable(context, inputVariables[0]->getType(), T("input"));             // 0: input
  frameClass->addMemberVariable(context, inputVariables[1]->getType(), T("supervision"));       // 1: supervision
  frameClass->addMemberOperator(context, createObjectFunction(lossFunctionClass), 1);           // 2: loss(supervision)

  FunctionPtr linearFunction = createLearnableFunction();
  linearFunction->setOnlineLearner(learnerParameters->createOnlineLearner());
  frameClass->addMemberOperator(context, linearFunction, 0, 2);                                 // 3: linearFunction(0,2)

  FunctionPtr postProcessing = createPostProcessing();
  if (postProcessing)
    frameClass->addMemberOperator(context, postProcessing, 3);                                  // 4: postProcess(3)          

  setBatchLearner(learnerParameters->createBatchLearner());
  return FrameBasedFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
}
