/*-----------------------------------------.---------------------------------.
| Filename: SandBox.cpp                    | Sand Box                        |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 11:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Data/Protein.h" 
#include "Inference/ProteinInferenceFactory.h"
#include "Inference/ProteinInference.h"
#include "Inference/ContactMapInference.h"
#include "Evaluator/ProteinEvaluator.h"
using namespace lbcpp;

extern void declareProteinClasses();

class ExtraTreeProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  virtual PerceptionPtr createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const
  {
    PerceptionPtr res = ProteinInferenceFactory::createPerception(targetName, is1DTarget, is2DTarget);
    return res ? res->flatten() : PerceptionPtr();
  }

  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
    {return binaryClassificationExtraTreeInference(targetName, perception, 2, 3);}

  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
    {return classificationExtraTreeInference(targetName, perception, classes, 2, 3);}
};

class NumericalProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  virtual PerceptionPtr createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const
  {
    PerceptionPtr res = ProteinInferenceFactory::createPerception(targetName, is1DTarget, is2DTarget);
    return res ? (PerceptionPtr)perceptionToFeatures(res) : PerceptionPtr();
  }
  
public:
  virtual InferencePtr createContactMapInference(const String& targetName) const
  {
    InferencePtr res = ProteinInferenceFactory::createContactMapInference(targetName);
    res->setBatchLearner(onlineToBatchInferenceLearner());
    return res;
  }

  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
  {
    InferencePtr scoreInference = linearInference(targetName + T(" Classifier"), perception);
    scoreInference->setOnlineLearner(createOnlineLearner(targetName + T(" Learner")));
    if (targetName.startsWith(T("contactMap")) || targetName == T("disorderRegions"))
      scoreInference = addBiasInference(targetName, scoreInference);
    return binaryLinearSVMInference(scoreInference);
  }

  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
  {
    InferencePtr binaryClassifier = createBinaryClassifier(targetName, perception);
    InferencePtr res = oneAgainstAllClassificationInference(targetName, classes, binaryClassifier);
    //res->setBatchLearner(onlineToBatchInferenceLearner());
    return res;
  }

protected:
  InferenceOnlineLearnerPtr createOnlineLearner(const String& targetName, double initialLearningRate = 1.0) const
  {
      StoppingCriterionPtr stoppingCriterion = logicalOr(
                                                     maxIterationsStoppingCriterion(5),  
                                                     maxIterationsWithoutImprovementStoppingCriterion(1));

//    StoppingCriterionPtr stoppingCriterion = maxIterationsStoppingCriterion(5);/*logicalOr(
      /*maxIterationsStoppingCriterion(100),  
      maxIterationsWithoutImprovementStoppingCriterion(1));*/

    if (targetName.startsWith(T("contactMap")))
      return gradientDescentInferenceOnlineLearner(
        InferenceOnlineLearner::perEpisode,                                                 // randomization
        InferenceOnlineLearner::perStep, invLinearIterationFunction(initialLearningRate, 100000), true, // learning steps
        InferenceOnlineLearner::perStepMiniBatch20, l2Regularizer(0.0),         // regularizer
        InferenceOnlineLearner::perPass, stoppingCriterion, true);                     // stopping criterion
    else
      return gradientDescentInferenceOnlineLearner(
        InferenceOnlineLearner::never,                                                 // randomization
        InferenceOnlineLearner::perStep, invLinearIterationFunction(initialLearningRate, 10000), true, // learning steps
        InferenceOnlineLearner::perStepMiniBatch20, l2Regularizer(0.01),         // regularizer
        InferenceOnlineLearner::perPass, stoppingCriterion, true);                     // stopping criterion
  }
};

///////////////////////////////////////// 

class MyInferenceCallback : public InferenceCallback
{
public:
  MyInferenceCallback(InferencePtr inference, ContainerPtr trainingData, ContainerPtr testingData)
    : inference(inference), trainingData(trainingData), testingData(testingData) {}

  virtual void preInferenceCallback(InferenceStackPtr stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
    {
      // top-level learning is beginning
      startingTime = Time::getMillisecondCounter();
      iterationNumber = 0;
    }

    String inferenceClassName = stack->getCurrentInference()->getClassName();
    if (inferenceClassName.contains(T("Learner")) && input.size() == 2)
    {
      TypePtr trainingExamplesType = input[1].getObjectAndCast<Container>()->getElementsType();
      jassert(trainingExamplesType->getNumTemplateArguments() == 2);
      String inputTypeName = trainingExamplesType->getTemplateArgument(0)->getName();
      std::cout << "=== Learning " << input[0].getObject()->getName() << " with " << input[1].size() << " " << inputTypeName << "(s) ===" << std::endl;
      //std::cout << "  learner: " << inferenceClassName << " static type: " << input[1].getTypeName() << std::endl
      //  << "  first example type: " << input[1][0].getTypeName() << std::endl << std::endl;
    }
  }
  
  virtual void postInferenceCallback(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    String inferenceName = stack->getCurrentInference()->getName();

    if (stack->getCurrentInference()->getClassName() == T("RunSequentialInferenceStepOnExamples"))
    //if (inferenceName == T("LearningPass"))
    {
      // end of learning iteration
      std::cout << std::endl
                << "=====================================================" << std::endl
                << "================ EVALUATION =========================  " << (Time::getMillisecondCounter() - startingTime) / 1000 << " s" << std::endl
                << "=====================================================" << std::endl;
      
      InferenceContextPtr validationContext = singleThreadedInferenceContext();
      ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
      validationContext->evaluate(inference, trainingData, evaluator);
      processResults(evaluator, true);
      
      evaluator = new ProteinEvaluator();
      validationContext->evaluate(inference, testingData, evaluator);
      processResults(evaluator, false);

      std::cout << "=====================================================" << std::endl << std::endl;
    }
    else if (stack->getDepth() == 1)
    {
      std::cout << "Bye." << std::endl;
    }
  }

  void processResults(ProteinEvaluatorPtr evaluator, bool isTrainingData)
    {std::cout << " == " << (isTrainingData ? "Training" : "Testing") << " Scores == " << std::endl << evaluator->toString() << std::endl;}

private:
  InferencePtr inference;
  ContainerPtr trainingData, testingData;
  size_t iterationNumber;
  juce::uint32 startingTime;
};
///////////////////////////////////////// 

VectorPtr loadProteins(const File& directory, size_t maxCount = 0)
{
#ifdef JUCE_DEBUG
  if (!maxCount) maxCount = 7;
#endif // JUCE_DEBUG
  return directoryFileStream(directory)->apply(loadFromFileFunction(proteinClass()))->load(maxCount);
}

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();
  
  File workingDirectory(T("C:\\Projets\\LBC++\\projects\\temp"));
  //File workingDirectory(T("/Users/francis/tmp"));

  ContainerPtr proteins = loadProteins(workingDirectory.getChildFile(T("PDB30Small/xml")), 100)->apply(proteinToInputOutputPairFunction())->randomize();
  ContainerPtr trainProteins = proteins->invFold(0, 2);
  ContainerPtr testProteins = proteins->fold(0, 2);
  std::cout << trainProteins->getNumElements() << " training proteins, " << testProteins->getNumElements() << " testing proteins" << std::endl;

  ProteinInferenceFactoryPtr factory = new ExtraTreeProteinInferenceFactory();
//  ProteinInferenceFactoryPtr factory = new NumericalProteinInferenceFactory();

  ProteinSequentialInferencePtr inference = new ProteinSequentialInference();
  //inference->setProteinDebugDirectory(workingDirectory.getChildFile(T("proteins")));
  //inference->appendInference(factory->createInferenceStep(T("contactMap8Ca")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  /*inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));*/
  //inference->appendInference(factory->createInferenceStep(T("structuralAlphabetSequence")));
  //inference->appendInference(factory->createInferenceStep(T("solventAccessibilityAt20p")));
  /*
  inference->appendInference(factory->createInferenceStep(T("disorderRegions")));
  inference->appendInference(factory->createInferenceStep(T("disorderRegions")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("dsspSecondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("dsspSecondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("disorderRegions")));*/
  

  std::cout << "Inference: " << std::endl;

  Variable(inference).printRecursively(std::cout, 2);

  InferenceContextPtr context = singleThreadedInferenceContext();
  context->appendCallback(new MyInferenceCallback(inference, trainProteins, testProteins));
  context->train(inference, trainProteins);

  Variable(inference).saveToFile(workingDirectory.getChildFile(T("NewStyleInference.xml")));

  
  {
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    context->evaluate(inference, trainProteins, evaluator);
    std::cout << "Check: " << evaluator->toString() << std::endl;
  }

  Variable v = Variable::createFromFile(workingDirectory.getChildFile(T("NewStyleInference.xml")));
  v.saveToFile(workingDirectory.getChildFile(T("NewStyleInference2.xml")));
  {
    inference = v.getObjectAndCast<Inference>();
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    context->evaluate(inference, trainProteins, evaluator);
    std::cout << "Check2: " << evaluator->toString() << std::endl;
  }
  return 0;
}
