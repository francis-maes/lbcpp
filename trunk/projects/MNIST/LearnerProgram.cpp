/*-----------------------------------------.---------------------------------.
| Filename: DigiBox.cpp                    | Digi Box                        |
| Author  : Julien Becker                  |                                 |
| Started : 08/11/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "LearnerProgram.h"
#include "MNISTImage.h"
#include "MNISTPerception.h"
#include "MatlabFileParser.h"

using namespace lbcpp;

class InputOutputPairFunction : public Function
{
  virtual TypePtr getInputType() const
    {return mnistImageClass;}
  
  virtual TypePtr getOutputType(TypePtr ) const
    {return pairClass(mnistImageClass, digitTypeEnumeration);}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    MNISTImagePtr image = input.getObjectAndCast<MNISTImage>();
    jassert(image);
    MNISTImagePtr inputImage = new MNISTImage();
    inputImage->setPixels(image->getPixels());
    Variable digit = image->getDigit();
    jassert(!digit.isNil());
    return Variable::pair(inputImage, digit);
  }
};

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
  else
  {
    learningData = learningData->apply(FunctionPtr(new InputOutputPairFunction()), false);
  }

  
  if (testingFile == File::nonexistent)
  {
    testingData = learningData->fold(0, 5);
    learningData = learningData->invFold(0, 5);
    return true;
  }
  
  testingData = parseDataFile(testingFile)->apply(FunctionPtr(new InputOutputPairFunction()), false);
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

int LearnerProgram::runProgram(MessageCallback& callback)
{
  juce::uint32 startingTime = Time::getMillisecondCounter();
  
  if (!loadData())
    return -1;
  
  std::cout << "------------ Data ------------  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
  std::cout << "Learning images : " << learningData->getNumElements() << std::endl;
  std::cout << "Testing images  : " << testingData->getNumElements() << std::endl;
  
  InferenceContextPtr context = singleThreadedInferenceContext();

  /* Perception */
  CompositePerceptionPtr perception = compositePerception(mnistImageClass, T("Image"));
  perception->addPerception(T("raw data"), imageFlattenPerception());
  perception->addPerception(T("binarized data"), binarizeImagePerception(binarizationThreshold));

  /* Inference */
  NumericalSupervisedInferencePtr inference = multiClassLinearSVMInference(T("digit"), rewritePerception(perception), digitTypeEnumeration, false);
  inference->setStochasticLearner(createOnlineLearner());
  
  //InferencePtr inference = classificationExtraTreeInference(T("digit"), perception, digitTypeEnumeration, 10, 26, 1);
  
  /* Experiment */
  std::cout << "---------- Learning ----------" << std::endl;
  context->train(inference, learningData, ContainerPtr());

  std::cout << "----- Evaluation - Train -----  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
  EvaluatorPtr evaluator = classificationAccuracyEvaluator(T("digit"));
  context->evaluate(inference, learningData, evaluator);
  std::cout << evaluator->toString() << std::endl;

  if (testingData && testingData->getNumElements())
  {
    std::cout << "----- Evaluation - Test ------  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
    evaluator = classificationAccuracyEvaluator(T("digit"));
    context->evaluate(inference, testingData, evaluator);
    std::cout << evaluator->toString() << std::endl;
  }

  inference->saveToFile(output.getFullPathName() + T(".inference"));
  std::cout << "------------ Bye -------------  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
  return 0;
}
