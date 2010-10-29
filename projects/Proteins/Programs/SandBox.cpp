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
#include "Inference/DisorderRegionsInference.h"
#include "Evaluator/ProteinEvaluator.h"
using namespace lbcpp;

extern void declareProteinClasses();

///////////////////////////////////////////////

InferenceContextPtr createInferenceContext()
{
  return multiThreadedInferenceContext(new ThreadPool(7, false));
}

class ExtraTreeProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  virtual PerceptionPtr createPerception(const String& targetName, PerceptionType type) const
  {
    PerceptionPtr res = ProteinInferenceFactory::createPerception(targetName, type);
    return res ? flattenPerception(res) : PerceptionPtr();
  }

  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
    {return binaryClassificationExtraTreeInference(targetName, perception, 2, 3);}

  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
    {return classificationExtraTreeInference(targetName, perception, classes, 2, 3);}
};

class NumericalProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  virtual PerceptionPtr createPerception(const String& targetName, PerceptionType type) const
  {
    PerceptionPtr res = ProteinInferenceFactory::createPerception(targetName, type);
    return res ? flattenPerception(res) : PerceptionPtr();
  }

  virtual InferencePtr createTargetInference(const String& targetName) const
  {
    InferencePtr res = ProteinInferenceFactory::createTargetInference(targetName);
    res->setBatchLearner(stochasticInferenceLearner());
    return res;
  }

  virtual InferencePtr createProbabilitySequenceInference(const String& targetName) const
  {
#if 0
    if (targetName == T("disorderRegions"))
    {
/*      InferencePtr rankingInference = allPairsRankingInference(
                                        linearInference(targetName, createPerception(targetName, residuePerception)),
                                        hingeLossFunction(true),
                                        createOnlineLearner(targetName, 1.0),
                                        targetName);*/

      InferencePtr rankingInference = binaryClassificationRankingLinearSVMInference(
          createPerception(targetName, residuePerception), createOnlineLearner(targetName, 0.01), targetName, false);

      InferencePtr cutoffInference;/* = squareRegressionInference(
                                        createPerception(targetName, proteinPerception),
                                        createOnlineLearner(targetName + T(" cutoff"), 0.05),
                                        targetName + T(" cutoff"));*/

      return new DisorderedRegionInference(targetName, rankingInference, cutoffInference);
    }
    else
#endif // 0
      return ProteinInferenceFactory::createProbabilitySequenceInference(targetName);
  }
  
  virtual void getPerceptionRewriteRules(PerceptionRewriterPtr rewriter) const
  {
    rewriter->addRule(booleanType, booleanFeatures());
    rewriter->addRule(enumValueFeaturesPerceptionRewriteRule());
    rewriter->addRule(negativeLogProbabilityType, defaultPositiveDoubleFeatures(30, -3, 3));
    rewriter->addRule(probabilityType, defaultProbabilityFeatures());
    rewriter->addRule(positiveIntegerType, defaultPositiveIntegerFeatures());
    rewriter->addRule(integerType, defaultIntegerFeatures());

    // all other features
    rewriter->addRule(doubleType, identityPerception());
  }

  std::vector<size_t> makeBinaryConjunction(size_t index1, size_t index2) const
    {std::vector<size_t> res(2); res[0] = index1; res[1] = index2; return res;}
/*
  virtual PerceptionPtr createPerception(const String& targetName, PerceptionType type) const
  {
    static int count = 1;
    if (count++ == 1)
      return ProteinInferenceFactory::createPerception(targetName, type);
    else
    {
      PerceptionPtr res = collapsePerception(ProteinInferenceFactory::createPerception(targetName, type));
      std::vector<std::vector<size_t> > conjunctions(res->getNumOutputVariables());
      for (size_t i = 0; i < conjunctions.size(); ++i)
        conjunctions[i].push_back(i);
      return selectAndMakeConjunctionFeatures(res, conjunctions);
    }
  }*/

public:
  virtual InferencePtr createContactMapInference(const String& targetName) const
  {
    InferencePtr res = ProteinInferenceFactory::createContactMapInference(targetName);
    //res->setBatchLearner(stochasticInferenceLearner());
    return res;
  }

  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
  {
    StaticDecoratorInferencePtr res = binaryLinearSVMInference(perception, createOnlineLearner(targetName), targetName);
    if (targetName.startsWith(T("contactMap")) || targetName == T("disorderRegions"))
    {
      VectorSequentialInferencePtr sequentialInference = new VectorSequentialInference(targetName);
      sequentialInference->appendInference(res->getSubInference());
      sequentialInference->appendInference(addBiasInference(targetName));
      res->setSubInference(sequentialInference);
      //res->setBatchLearner(stochasticInferenceLearner());
    }
    return res;
  }

  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
  {
/*    static int count = 1;
    StaticDecoratorInferencePtr res = multiClassLinearSVMInference(perception, classes, createOnlineLearner(targetName, 0.1), true, targetName);
    NumericalInferencePtr multiLinearInference = res->getSubInference();
    if (count++ == 1)
      multiLinearInference->addOnlineLearner(stoppingCriterionOnlineLearner(InferenceOnlineLearner::perPass, maxIterationsStoppingCriterion(15), true));
    else
    {
      multiLinearInference->addOnlineLearner(graftingOnlineLearner(perception, multiLinearInference));
      multiLinearInference->addOnlineLearner(stoppingCriterionOnlineLearner(InferenceOnlineLearner::perPass, maxIterationsStoppingCriterion(1000), true));
    }
    return res;

    
*/
    return multiClassLinearSVMInference(perception, classes, createOnlineLearner(targetName, 0.5), false, targetName);
  
    /*InferencePtr binaryClassifier = createBinaryClassifier(targetName, perception);
    InferencePtr res = oneAgainstAllClassificationInference(targetName, classes, binaryClassifier);
    return res;*/

    //InferencePtr rankingInference = allPairsRankingInference(linearInference(targetName, inputLabelPairPerception(perception, classes)), hingeLossFunction(true), 
    //  createOnlineLearner(targetName), targetName);

      //mostViolatedPairRankingLinearSVMInference(inputLabelPairPerception(perception, classes), createOnlineLearner(targetName), targetName);
    //return rankingBasedClassificationInference(targetName, rankingInference, classes);
  }

protected:
  InferenceOnlineLearnerPtr createOnlineLearner(const String& targetName, double initialLearningRate = 1.0) const
  {
    InferenceOnlineLearnerPtr res;
    if (targetName.startsWith(T("contactMap")))
      res = gradientDescentOnlineLearner(
        InferenceOnlineLearner::perPass,                                                 // randomization
        InferenceOnlineLearner::perStep, invLinearIterationFunction(initialLearningRate, 100000), true, // learning steps
        InferenceOnlineLearner::perStepMiniBatch20, l2RegularizerFunction(0.0));         // regularizer
    else
      res = gradientDescentOnlineLearner(
        InferenceOnlineLearner::never, //perStepMiniBatch1000,                                                 // randomization
        InferenceOnlineLearner::perStep, invLinearIterationFunction(initialLearningRate, (size_t)5e6), true, // learning steps
        InferenceOnlineLearner::perStepMiniBatch20, l2RegularizerFunction(1e-8));         // regularizer

    size_t numIterations = (targetName == T("disorderRegions cutoff") ? 1 : 10);
    res->getLastLearner()->setNextLearner(stoppingCriterionOnlineLearner(InferenceOnlineLearner::perPass,
        maxIterationsStoppingCriterion(numIterations), true)); // stopping criterion
    return res;
  }
};

/////////////////////////////////////////

class MyInferenceCallback : public InferenceCallback
{
public:
  MyInferenceCallback(InferencePtr inference, ContainerPtr trainingData, ContainerPtr testingData)
    : inference(inference), trainingData(trainingData), testingData(testingData) {}

  virtual void preInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
    {
      // top-level learning is beginning
      startingTime = Time::getMillisecondCounter();
      iterationNumber = 0;
    }

    if (input.size() == 2 && input[0].getType()->inheritsFrom(inferenceClass))
    {
      TypePtr trainingExamplesType = input[1].getObjectAndCast<Container>()->getElementsType();
      jassert(trainingExamplesType->getNumTemplateArguments() == 2);
      String inputTypeName = trainingExamplesType->getTemplateArgument(0)->getName();
      MessageCallback::info(T("=== Learning ") + input[0].getObject()->getName() + T(" with ") + String((int)input[1].size()) + T(" ") + inputTypeName + T("(s) ==="));
      //std::cout << "  learner: " << inferenceClassName << " static type: " << input[1].getTypeName() << std::endl
      //  << "  first example type: " << input[1][0].getTypeName() << std::endl << std::endl;
    }
  }

  virtual void postInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    String inferenceName = stack->getCurrentInference()->getName();

    if (stack->getCurrentInference()->getClassName() == T("RunSequentialInferenceStepOnExamples"))
    //if (inferenceName.startsWith(T("LearningPass")))
    {
      // end of learning iteration
      MessageCallback::info(String::empty);
      MessageCallback::info(T("====================================================="));
      MessageCallback::info(T("================ EVALUATION =========================  ") + String((Time::getMillisecondCounter() - startingTime) / 1000) + T(" s"));
      MessageCallback::info(T("====================================================="));

      //singleThreadedInferenceContext();
      //InferenceContextPtr context = multiThreadedInferenceContext(7);
      ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
      context->evaluate(inference, trainingData, evaluator);
      processResults(evaluator, true);

      evaluator = new ProteinEvaluator();
      context->evaluate(inference, testingData, evaluator);
      processResults(evaluator, false);

      MessageCallback::info(T("====================================================="));
    }
    else if (stack->getDepth() == 1)
    {
      MessageCallback::info(T("Bye: ") + String((Time::getMillisecondCounter() - startingTime) / 1000.0) + T(" seconds"));
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

VectorPtr loadProteins(const File& inputDirectory, const File& supervisionDirectory, ThreadPoolPtr pool)
{
#ifdef JUCE_DEBUG
  size_t maxCount = 1;
#else
  size_t maxCount = 50;
#endif // JUCE_DEBUG
  if (inputDirectory.exists())
    return directoryPairFileStream(inputDirectory, supervisionDirectory)->load(maxCount)
        ->apply(loadFromFilePairFunction(proteinClass, proteinClass), pool)->randomize();
  else
    return directoryFileStream(supervisionDirectory)->load(maxCount)->apply(loadFromFileFunction(proteinClass), pool)
      ->apply(proteinToInputOutputPairFunction(false), false)->randomize();
}

void initializeLearnerByCloning(InferencePtr inference, InferencePtr inferenceToClone)
{
  inference->setBatchLearner(multiPassInferenceLearner(initializeByCloningInferenceLearner(inferenceToClone), inference->getBatchLearner()));
}

void getAllObjectsRecursively(const ObjectPtr& object, std::set<ObjectPtr>& res)
{
  if (res.find(object) == res.end())
  {
    res.insert(object);
    size_t n = object->getNumVariables();
    for (size_t i = 0; i < n; ++i)
    {
      Variable v = object->getVariable(i);
      if (v.exists() && v.isObject())
        getAllObjectsRecursively(v.getObject(), res);
    }
    ContainerPtr container = object.dynamicCast<Container>();
    if (container && container->getElementsType()->inheritsFrom(objectClass))
    {
      n = container->getNumElements();
      for (size_t i  = 0; i < n; ++i)
      {
        Variable elt = container->getElement(i);
        if (elt.exists())
          getAllObjectsRecursively(elt.getObject(), res);
      }
    }
  }
}
namespace lbcpp
{
  extern size_t allocatedSize;
}

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();

  ThreadPoolPtr pool = new ThreadPool(7);

#ifdef JUCE_WIN32
  File workingDirectory(T("C:\\Projets\\lbcpp\\projects\\temp\\psipred"));
#else
  File workingDirectory(T("/data/PDB/PDB30Medium"));
#endif

  ContainerPtr trainProteins = loadProteins(File::nonexistent, workingDirectory.getChildFile(T("train")), pool);
  ContainerPtr testProteins = loadProteins(File::nonexistent, workingDirectory.getChildFile(T("test")), pool);
  std::cout << trainProteins->getNumElements() << " training proteins, " << testProteins->getNumElements() << " testing proteins" << std::endl;

  //ProteinInferenceFactoryPtr factory = new ExtraTreeProteinInferenceFactory();
  ProteinInferenceFactoryPtr factory = new NumericalProteinInferenceFactory();

  //ProteinParallelInferencePtr inference = new ProteinParallelInference();
  //inference->setProteinDebugDirectory(workingDirectory.getChildFile(T("proteins")));
  //inference->appendInference(factory->createInferenceStep(T("contactMap8Ca")));

  //inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  //inference->appendInference(factory->createInferenceStep(T("solventAccessibilityAt20p")));
  //inference->appendInference(factory->createInferenceStep(T("disorderRegions")));

  /*
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));*/
  //inferencePass->appendInference(factory->createInferenceStep(T("structuralAlphabetSequence")));
  //inferencePass->appendInference(factory->createInferenceStep(T("solventAccessibilityAt20p")));
  //inferencePass->appendInference(factory->createInferenceStep(T("disorderRegions")));
  //inferencePass->appendInference(factory->createInferenceStep(T("dsspSecondaryStructure")));

  ProteinSequentialInferencePtr inference = new ProteinSequentialInference();
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));

  //inference->appendInference(inferencePass);
  //inference->appendInference(inferencePass->cloneAndCast<Inference>());
/*
  InferencePtr lastStep = factory->createInferenceStep(T("secondaryStructure"));
  inference->appendInference(lastStep);
  for (int i = 1; i < 5; ++i)
  {
    InferencePtr step = factory->createInferenceStep(T("secondaryStructure"));
    //initializeLearnerByCloning(step, lastStep);
    inference->appendInference(step);
    lastStep = step;
  }*/

  //inference->appendInference(factory->createInferenceStep(T("solventAccessibilityAt20p")));
  //inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));

  /*inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));*/

  //inference->appendInference(factory->createInferenceStep(T("structuralAlphabetSequence")));
  
  /*inference->appendInference(factory->createInferenceStep(T("disorderRegions")));
  inference->appendInference(factory->createInferenceStep(T("dsspSecondaryStructure")));*/

  /*
  inference->appendInference(inferencePass);
  inference->appendInference(inferencePass->cloneAndCast<Inference>());*/

/*  std::cout << "Inference: " << std::endl;
  Variable(inference).printRecursively(std::cout, 2);*/


  InferenceContextPtr context = multiThreadedInferenceContext(pool);
  ProteinEvaluatorPtr evaluator = new ProteinEvaluator();

  /*
  ReferenceCountedObject::resetRefCountDebugInfo();
  context->crossValidate(inference, proteins, evaluator, 2);
  std::cout << evaluator->toString() << std::endl;
  ReferenceCountedObject::displayRefCountDebugInfo(std::cout);
  return 0;*/

  context->appendCallback(new MyInferenceCallback(inference, trainProteins, testProteins));
  context->train(inference, trainProteins);

  /*
  std::cout << "Making and saving train predicions..." << std::endl;
  context->evaluate(inference, trainProteins, saveToDirectoryEvaluator(workingDirectory.getChildFile(T("trainPass2"))));
  std::cout << "Making and saving test predicions..." << std::endl;
  context->evaluate(inference, testProteins, saveToDirectoryEvaluator(workingDirectory.getChildFile(T("testPass2"))));
*/
  {
    std::cout << "Check Evaluating..." << std::endl;
    evaluator = new ProteinEvaluator();
    context->evaluate(inference, trainProteins, evaluator);
    std::cout << "============================" << std::endl << std::endl;
    std::cout << evaluator->toString() << std::endl << std::endl;

    EvaluatorPtr evaluator = new ProteinEvaluator();
    context->evaluate(inference, testProteins, evaluator);
    std::cout << "============================" << std::endl << std::endl;
    std::cout << evaluator->toString() << std::endl << std::endl;
  }
  return 0;
  

  std::cout << "Saving inference ..." << std::flush;
  inference->saveToFile(workingDirectory.getChildFile(T("NewStyleInference.xml")));
  std::cout << "ok." << std::endl;

  std::cout << "Loading..." << std::flush;
  inference = Inference::createFromFile(workingDirectory.getChildFile(T("NewStyleInference.xml")));
  std::cout << "ok." << std::endl;

  for (size_t i = 7; i <= 7; i += 1)
  {
    std::cout << "Check Evaluating with " << (i ? i : 1) << " threads ..." << std::endl;
    EvaluatorPtr evaluator = new ProteinEvaluator();
    InferenceContextPtr context = multiThreadedInferenceContext(new ThreadPool(i ? i : 1, false));
    context->appendCallback(new MyInferenceCallback(inference, trainProteins, testProteins));
    context->evaluate(inference, trainProteins, evaluator);
  //  context->crossValidate(inference, proteins, evaluator, 2);
    std::cout << "============================" << std::endl << std::endl;
    std::cout << evaluator->toString() << std::endl << std::endl;
  }

#if 0
  std::cout << "Loading..." << std::flush;
  InferencePtr loadedInference = Inference::createFromFile(workingDirectory.getChildFile(T("NewStyleInference.xml")));
  std::cout << "ok." << std::endl;

  //printDifferencesRecursively(v, inference, T("inference"));

  std::cout << "Re-saving..." << std::flush;
  loadedInference->saveToFile(workingDirectory.getChildFile(T("NewStyleInference2.xml")));
  std::cout << "ok." << std::endl;

  std::cout << "Re-evaluating..." << std::endl;
  evaluator = new ProteinEvaluator();
  context->evaluate(loadedInference, trainProteins, evaluator);
  std::cout << "ok." << std::endl;
  std::cout << "============================" << std::endl << std::endl;
  std::cout << evaluator->toString() << std::endl << std::endl;
#endif // 0

  std::cout << "Tchao." << std::endl;
  return 0;
}
