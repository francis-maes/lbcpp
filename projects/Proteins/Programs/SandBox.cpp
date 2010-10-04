/*-----------------------------------------.---------------------------------.
| Filename: SandBox.cpp                    | Sand Box                        |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 11:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Data/Protein.h" 
#include "Perception/ProteinPerception.h"
#include "Inference/ProteinInferenceFactory.h"
#include "Inference/ProteinInference.h"
#include "Inference/ContactMapInference.h"
#include "Evaluator/ProteinEvaluator.h"
using namespace lbcpp;

extern void declareProteinClasses();

InferenceContextPtr createInferenceContext()
{
  return multiThreadedInferenceContext(new ThreadPool(14, false));
}

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
  virtual void getPerceptionRewriteRules(PerceptionRewriterPtr rewriter) const
  {
    //rewriter->addRule(biVariableFeaturesPerceptionRewriteRule(hardDiscretizedNumberFeatures(probabilityType(), 10)));

    rewriter->addRule(booleanType(), booleanFeatures());
    //rewriter->addRule(probabilityType(), T("PSSM"), identityFeatures());
    //rewriter->addRule(probabilityType(), defaultProbabilityFeatures());
    rewriter->addRule(positiveIntegerType(), defaultPositiveIntegerFeatures());

    rewriter->addEnumValueFeaturesRule();
    rewriter->addRule(doubleType(), identityPerception());
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
    // FIXME !
    //if (targetName.startsWith(T("contactMap")) || targetName == T("disorderRegions"))
    //  scoreInference = addBiasInference(targetName, scoreInference, 0.0);
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
      StoppingCriterionPtr stoppingCriterion = maxIterationsStoppingCriterion(2);/* logicalOr(
                                                     maxIterationsStoppingCriterion(5),  
                                                     maxIterationsWithoutImprovementStoppingCriterion(1));*/

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

class StdOutPrinter
{
public:
  void print(const String& line)
  {
    ScopedLock _(lock);
    std::cout << line << std::endl;
  }

private:
  CriticalSection lock;
};
static StdOutPrinter stdOutPrinter;

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
      stdOutPrinter.print(T("=== Learning ") + input[0].getObject()->getName() + T(" with ") + String((int)input[1].size()) + T(" ") + inputTypeName + T("(s) ==="));
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
      stdOutPrinter.print(String::empty);
      stdOutPrinter.print(T("====================================================="));
      stdOutPrinter.print(T("================ EVALUATION =========================  ") + String((Time::getMillisecondCounter() - startingTime) / 1000) + T(" s"));
      stdOutPrinter.print(T("====================================================="));
      
      //singleThreadedInferenceContext();
      InferenceContextPtr validationContext =  createInferenceContext();
      ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
      validationContext->evaluate(inference, trainingData, evaluator);
      processResults(evaluator, true);
      
      evaluator = new ProteinEvaluator();
      validationContext->evaluate(inference, testingData, evaluator);
      processResults(evaluator, false);

      stdOutPrinter.print(T("====================================================="));
    }
    else if (stack->getDepth() == 1)
    {
      stdOutPrinter.print(T("Bye: ") + String((Time::getMillisecondCounter() - startingTime) / 1000.0) + T(" seconds"));
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
  
#ifdef JUCE_WIN32
  File workingDirectory(T("C:\\Projets\\LBC++\\projects\\temp"));
#else
  File workingDirectory(T("/data/PDB"));
#endif 

  ContainerPtr proteins = loadProteins(workingDirectory.getChildFile(T("PDB30Small/xml")))->apply(proteinToInputOutputPairFunction())->randomize();
  ContainerPtr trainProteins = proteins->invFold(0, 2);
  ContainerPtr testProteins = proteins->fold(0, 2);
  std::cout << trainProteins->getNumElements() << " training proteins, " << testProteins->getNumElements() << " testing proteins" << std::endl;

  //ProteinInferenceFactoryPtr factory = new ExtraTreeProteinInferenceFactory();
  ProteinInferenceFactoryPtr factory = new NumericalProteinInferenceFactory();

  ProteinParallelInferencePtr inferencePass = new ProteinParallelInference();
  //inference->setProteinDebugDirectory(workingDirectory.getChildFile(T("proteins")));
  //inference->appendInference(factory->createInferenceStep(T("contactMap8Ca")));
  inferencePass->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  /*inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));*/
  inferencePass->appendInference(factory->createInferenceStep(T("structuralAlphabetSequence")));
  inferencePass->appendInference(factory->createInferenceStep(T("solventAccessibilityAt20p")));
  inferencePass->appendInference(factory->createInferenceStep(T("disorderRegions")));
  inferencePass->appendInference(factory->createInferenceStep(T("dsspSecondaryStructure")));
  
  ProteinSequentialInferencePtr inference = new ProteinSequentialInference();
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  /*inference->appendInference(factory->createInferenceStep(T("structuralAlphabetSequence")));
  inference->appendInference(factory->createInferenceStep(T("solventAccessibilityAt20p")));
  inference->appendInference(factory->createInferenceStep(T("disorderRegions")));
  inference->appendInference(factory->createInferenceStep(T("dsspSecondaryStructure")));*/

  /*
  inference->appendInference(inferencePass);
  inference->appendInference(inferencePass->cloneAndCast<Inference>());*/

/*  std::cout << "Inference: " << std::endl;
  Variable(inference).printRecursively(std::cout, 2);*/

  InferenceContextPtr context = createInferenceContext();
  ProteinEvaluatorPtr evaluator = new ProteinEvaluator();

  context->appendCallback(new MyInferenceCallback(inference, trainProteins, testProteins));
 /* context->train(inference, trainProteins);

  std::cout << "Saving inference ..." << std::flush;
  Variable(inference).saveToFile(workingDirectory.getChildFile(T("NewStyleInference.xml")));
  std::cout << "ok." << std::endl;

  
  {
    
    std::cout << "Evaluating..." << std::flush;
    context->evaluate(inference, trainProteins, evaluator);
    std::cout << " " << evaluator->toString() << std::endl;
  }*/

  context->crossValidate(inference, proteins, evaluator, 7); 
  std::cout << "============================" << std::endl << std::endl;
  std::cout << evaluator->toString() << std::endl << std::endl;

  /*
  Variable v = Variable::createFromFile(workingDirectory.getChildFile(T("NewStyleInference.xml")));
  v.saveToFile(workingDirectory.getChildFile(T("NewStyleInference2.xml")));
  {
    inference = v.getObjectAndCast<Inference>();
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    context->evaluate(inference, trainProteins, evaluator);
    std::cout << "Check2: " << evaluator->toString() << std::endl;
  }*/

  std::cout << "Deinitializing..." << std::endl;
  lbcpp::deinitialize();
  std::cout << "Tchao." << std::endl;
  return 0;
}
