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
extern ContainerPtr parseDataFile(const File& f);
}

bool LearnerProgram::loadData()
{
  if (learningFile == File::nonexistent)
  {
    std::cerr << "Error - No learning file found !" << std::endl;
    return false;
  }

  learningData = parseDataFile(learningFile);
  if (!learningData || !learningData->getNumElements())
  {
    std::cerr << "Error - No training data found in " << learningFile.getFullPathName().quoted() << std::endl;
    return false;
  }

  if (testingFile == File::nonexistent)
  {
    testingData = learningData->fold(0, 5);
    learningData = learningData->invFold(0, 5);
    return true;
  }

  testingData = parseDataFile(testingFile);
  if (!testingData->getNumElements())
  {
    std::cerr << "Error - No testing data found in " << testingFile.getFullPathName().quoted() << std::endl;
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
  
  if (!loadData())
    return false;
  
  std::cout << "------------ Data ------------  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
  std::cout << "Learning images : " << learningData->getNumElements() << std::endl;
  std::cout << "Testing images  : " << testingData->getNumElements() << std::endl;

  InferenceContextPtr inferenceContext = singleThreadedInferenceContext();
//  InferenceContextPtr context = multiThreadedInferenceContext(new ThreadPool(10, false));

  /* Perception */
  CompositePerceptionPtr perception = compositePerception(imageClass, T("Image"));
//  perception->addPerception(T("raw data"), imageFunctionToFlattenPerception(identityImageFunction(28, 28)));
//  perception->addPerception(T("binarized"), imageFunctionToFlattenPerception(binarizeImageFunction(28, 28, 0.02)));
//  perception->addPerception(T("maxima"), imageFunctionToFlattenPerception(maximumImageFunction(28, 28, 2)));
  perception->addPerception(T("minima x2"), imageFunctionToFlattenPerception(minimumImageFunction(28, 28, 2)));
//  perception->addPerception(T("minima x4"), imageFunctionToFlattenPerception(minimumImageFunction(28, 28, 4)));
//  perception->addPerception(T("minima x8"), imageFunctionToFlattenPerception(minimumImageFunction(28, 28, 8)));

  /* Inference */
  NumericalSupervisedInferencePtr inference = multiClassLinearSVMInference(T("digit"), rewritePerception(perception), digitTypeEnumeration, false);
  inference->setStochasticLearner(createOnlineLearner());

//  InferencePtr inference = classificationExtraTreeInference(T("digit"), flattenPerception(perception), digitTypeEnumeration, numTrees, numAttr, splitSize);
  
  /* Experiment */
  std::cout << "---------- Learning ----------" << std::endl;
  inferenceContext->train(inference, learningData, ContainerPtr());

  std::cout << "----- Evaluation - Train -----  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
  EvaluatorPtr evaluator = classificationAccuracyEvaluator(T("digit"));
  inferenceContext->evaluate(inference, learningData, evaluator);
  std::cout << evaluator->toString() << std::endl;

  if (testingData && testingData->getNumElements())
  {
    std::cout << "----- Evaluation - Test ------  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
    evaluator = classificationAccuracyEvaluator(T("digit"));
    inferenceContext->evaluate(inference, testingData, evaluator);
    std::cout << evaluator->toString() << std::endl;
  }

  inference->saveToFile(output.getFullPathName() + T(".inference"));
  std::cout << "------------ Bye -------------  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
  return true;
}
