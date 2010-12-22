/*-----------------------------------------.---------------------------------.
| Filename: DigiBox.cpp                    | Digi Box                        |
| Author  : Julien Becker                  |                                 |
| Started : 08/11/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "LearnerProgram.h"
#include "../Image/Image.h"

using namespace lbcpp;

namespace lbcpp
{
extern EnumerationPtr digitTypeEnumeration;
extern ContainerPtr parseDataFile(ExecutionContext& context, const File& f);
}

bool LearnerProgram::loadData(ExecutionContext& context)
{
  if (learningFile == File::nonexistent)
  {
    context.errorCallback(T("Error - No learning file found !"));
    return false;
  }

  context.informationCallback(T("Loading learning data from ") + learningFile.getFileName() + T("..."));
  learningData = parseDataFile(context, learningFile);
  if (!learningData || !learningData->getNumElements())
  {
    context.errorCallback(T("Error - No training data found in ") + learningFile.getFullPathName().quoted());
    return false;
  }
  context.informationCallback(String((int)learningData->getNumElements()) + T(" learning examples"));

  if (testingFile == File::nonexistent)
  {
    testingData = learningData->fold(0, 5);
    learningData = learningData->invFold(0, 5);
    return true;
  }

  context.informationCallback(T("Loading testing data from ") + testingFile.getFileName() + T("..."));
  testingData = parseDataFile(context, testingFile);
  if (!testingData->getNumElements())
  {
    context.errorCallback(T("Error - No testing data found in ") + testingFile.getFullPathName().quoted());
    return false;
  }

  return true;
}

PerceptionPtr rewritePerception(PerceptionPtr perception)
{
  PerceptionRewriterPtr rewriter = new PerceptionRewriter(false);

  rewriter->addRule(booleanType, booleanFeatures());
  //rewriter->addRule(enumValueFeaturesPerceptionRewriteRule());
  
  //rewriter->addRule(negativeLogProbabilityType, defaultPositiveDoubleFeatures(30, -3, 3));
  rewriter->addRule(probabilityType, defaultProbabilityFeatures());
  //rewriter->addRule(positiveIntegerType, defaultPositiveIntegerFeatures());
  //rewriter->addRule(integerType, defaultIntegerFeatures());
  
  rewriter->addRule(doubleType, identityPerception());
  return rewriter->rewrite(perception);
}

InferenceOnlineLearnerPtr LearnerProgram::createOnlineLearner() const
{
  InferenceOnlineLearnerPtr learner, lastLearner;
  learner = lastLearner = gradientDescentOnlineLearner(perStep, constantIterationFunction(1.0),
                                                       true, perStepMiniBatch20,
                                                       l2RegularizerFunction(regularizer));
  
  lastLearner = lastLearner->setNextLearner(computeEvaluatorOnlineLearner(classificationAccuracyEvaluator(T("digit")), false));
  lastLearner = lastLearner->setNextLearner(saveScoresToGnuPlotFileOnlineLearner(output.getFullPathName() + T(".gnuplot")));
  
  StoppingCriterionPtr stoppingCriterion = maxIterationsStoppingCriterion(numIterations);
  lastLearner->setNextLearner(stoppingCriterionOnlineLearner(stoppingCriterion, true));
  
  return learner;
}

bool LearnerProgram::run(ExecutionContext& context)
{
  juce::uint32 startingTime = Time::getMillisecondCounter();
  
  if (!loadData(context))
    return false;
  
  std::cout << "------------ Data ------------  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
  std::cout << "Learning images : " << learningData->getNumElements() << std::endl;
  std::cout << "Testing images  : " << testingData->getNumElements() << std::endl;


  /* Perception */
  CompositePerceptionPtr perception = compositePerception(imageClass, T("Image"));
//  perception->addPerception(T("raw data"), imageFunctionToFlattenPerception(identityImageFunction(28, 28)));
//  perception->addPerception(T("binarized"), imageFunctionToFlattenPerception(binarizeImageFunction(28, 28, 0.02)));
//  perception->addPerception(T("maxima"), imageFunctionToFlattenPerception(maximumImageFunction(28, 28, 2)));
  perception->addPerception(T("minima x2"), imageFunctionToFlattenPerception(minimumImageFunction(28, 28, 2)));
//  perception->addPerception(T("minima x4"), imageFunctionToFlattenPerception(minimumImageFunction(28, 28, 4)));
//  perception->addPerception(T("minima x8"), imageFunctionToFlattenPerception(minimumImageFunction(28, 28, 8)));

  /* Inference */
  //  NumericalSupervisedInferencePtr inference = multiClassLinearSVMInference(T("digit"), rewritePerception(perception), digitTypeEnumeration, false);
  //inference->setStochasticLearner(createOnlineLearner());

  InferencePtr inference = classificationExtraTreeInference(context, T("digit"), flattenPerception(perception), digitTypeEnumeration, numTrees, numAttr, splitSize);
  
  /* Experiment */
  //context.informationCallback(T("---------- Learning ----------"));
  inference->train(context, learningData, ContainerPtr());

  //std::cout << "----- Evaluation - Train -----  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
  EvaluatorPtr evaluator = classificationAccuracyEvaluator(T("digit"));
  inference->evaluate(context, learningData, evaluator, T("Evaluating on training data"));

  if (testingData && testingData->getNumElements())
  {
    //std::cout << "----- Evaluation - Test ------  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
    evaluator = classificationAccuracyEvaluator(T("digit"));
    inference->evaluate(context, testingData, evaluator, T("Evaluating on testing data"));
  }

  inference->saveToFile(context, output.getFullPathName() + T(".inference"));
  //std::cout << "------------ Bye -------------  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
  return true;
}
