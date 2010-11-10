/*-----------------------------------------.---------------------------------.
| Filename: DigiBox.cpp                    | Digi Box                        |
| Author  : Julien Becker                  |                                 |
| Started : 08/11/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "MNISTProgram.h"
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

ContainerPtr MNISTProgram::loadDataFromFile(const File& file)
{
  return StreamPtr(new MatlabFileParser(file))->load()->apply(FunctionPtr(new InputOutputPairFunction()));
}

bool MNISTProgram::loadData()
{
  if (learningFile == File::nonexistent)
  {
    std::cerr << "Error - No learning file found !" << std::endl;
    return false;
  }
  
  learningData = loadDataFromFile(learningFile);
  if (!learningData->getNumElements())
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
  
  testingData = loadDataFromFile(testingFile);
  if (!testingData->getNumElements())
  {
    std::cerr << "Error - No testing data found in " << testingFile.getFullPathName().quoted() << std::endl;
    return false;
  }
  
  return true;
}

int MNISTProgram::runProgram(MessageCallback& callback)
{
  juce::uint32 startingTime = Time::getMillisecondCounter();
  
  if (!loadData())
    return -1;
  
  std::cout << "------------ Data ------------  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
  std::cout << "Learning images : " << learningData->getNumElements() << std::endl;
  std::cout << "Testing images  : " << testingData->getNumElements() << std::endl;
  
  InferenceContextPtr context = singleThreadedInferenceContext();

  /* Perception */
  PerceptionPtr perception = imageFlattenPerception();
  /* Inference */
  InferenceOnlineLearnerPtr learner, lastLearner;
  learner = lastLearner = gradientDescentOnlineLearner(perStep, constantIterationFunction(1.0),
                                                       true, perStepMiniBatch20,
                                                       l2RegularizerFunction(0.0));

  lastLearner = lastLearner->setNextLearner(computeEvaluatorOnlineLearner(classificationAccuracyEvaluator(T("digit")), false));
  lastLearner = lastLearner->setNextLearner(saveScoresToGnuPlotFileOnlineLearner(File::getCurrentWorkingDirectory().getChildFile(T("MNIST.gnuplot"))));
  
  StoppingCriterionPtr stoppingCriterion = maxIterationsStoppingCriterion(100);
  lastLearner->setNextLearner(stoppingCriterionOnlineLearner(stoppingCriterion, true));
  
  NumericalSupervisedInferencePtr inference = multiClassLinearSVMInference(T("digit"), perception, digitTypeEnumeration, false);
  inference->setStochasticLearner(learner);

  /* Experiment */
  std::cout << "---------- Learning ----------" << std::endl;
  context->train(inference, learningData, ContainerPtr());

  std::cout << "----- Evaluation - Train -----  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
  EvaluatorPtr evaluator = classificationAccuracyEvaluator(T("digit"));
  context->evaluate(inference, learningData, evaluator);
  std::cout << evaluator->toString() << std::endl;

  std::cout << "----- Evaluation - Test ------  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
  evaluator = classificationAccuracyEvaluator(T("digit"));
  context->evaluate(inference, testingData, evaluator);
  std::cout << evaluator->toString() << std::endl;

  inference->saveToFile(File::getCurrentWorkingDirectory().getChildFile(T("MNIST.inference")));
  std::cout << "------------ Bye -------------  " << String((Time::getMillisecondCounter() - startingTime) / 1000.0) << std::endl;
  return 0;
}

extern void declareMNISTClasses();

namespace lbcpp
{
  extern ClassPtr mnistProgramClass;
}

int main(int argc, char* argv[])
{
  lbcpp::initialize();
  declareMNISTClasses();
  return ProgramPtr(new MNISTProgram())->main(argc, argv);
}
