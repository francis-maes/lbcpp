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

class RankingBasedExtractionInference : public VectorSequentialInference
{
public:
  RankingBasedExtractionInference(const String& name, InferencePtr rankingInference, InferencePtr cutoffInference)
    : VectorSequentialInference(name)
  {
    appendInference(rankingInference);
    appendInference(cutoffInference);
  }

  virtual TypePtr getInputType() const
    {return subInferences[0]->getInputType();}

  virtual TypePtr getSupervisionType() const
    {return subInferences[0]->getSupervisionType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return subInferences[0]->getOutputType(inputType);}

  virtual ContainerPtr createRankingInputs(const Variable& input) const;
  virtual ContainerPtr createRankingCosts(const Variable& supervision) const = 0;
  virtual Variable createCutoffInput(const Variable& input) const
    {return input;}
  virtual double computeBestCutoff(const ContainerPtr& scores, const ContainerPtr& costs) const = 0;

  virtual Variable computeOutput(const ContainerPtr& scores, double cutoff) const = 0;

  struct State : public SequentialInferenceState
  {
    State(const Variable& input, const Variable& supervision)
      : SequentialInferenceState(input, supervision) {}

    ContainerPtr scores;
  };

  typedef ReferenceCountedObjectPtr<State> StatePtr;

  virtual SequentialInferenceStatePtr prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    SequentialInferenceStatePtr state = new State(input, supervision);
    state->setSubInference(subInferences[0], createRankingInputs(input), supervision.exists() ? createRankingCosts(supervision) : ContainerPtr());
    return state;
  }

  virtual void prepareSubInference(InferenceContextPtr context, SequentialInferenceStatePtr s, size_t index, ReturnCode& returnCode)
  {
    jassert(index == 1);
    const StatePtr& state = s.staticCast<State>();
    state->scores = state->getSubOutput().getObjectAndCast<Container>();
    jassert(state->scores);

    Variable supervision;
    if (state->getSupervision().exists())
      supervision = computeBestCutoff(state->getSubOutput().getObjectAndCast<Container>(), state->getSubSupervision().getObjectAndCast<Container>());
    state->setSubInference(subInferences[1], createCutoffInput(state->getInput()), supervision);
  }

  virtual Variable finalizeInference(const InferenceContextPtr& context, SequentialInferenceStatePtr finalState, ReturnCode& returnCode)
  {
    const StatePtr& state = finalState.staticCast<State>();
    Variable predictedCutoff = state->getSubOutput();
    double cutoff = predictedCutoff.exists() ? predictedCutoff.getDouble() : 0.0;
    return computeOutput(state->scores, cutoff);
  }
};

class DisorderedRegionInference : public RankingBasedExtractionInference
{
public:
  DisorderedRegionInference(const String& name, InferencePtr rankingInference, InferencePtr cutoffInference)
    : RankingBasedExtractionInference(name, rankingInference, cutoffInference)
  {
    checkInheritance(rankingInference->getInputType(), containerClass(pairClass(proteinClass, positiveIntegerType)));
    checkInheritance(cutoffInference->getInputType(), proteinClass);
  }

  virtual TypePtr getInputType() const
    {return proteinClass;}

  virtual TypePtr getSupervisionType() const
    {return containerClass(probabilityType);}

  virtual TypePtr getOutputType() const
    {return containerClass(probabilityType);}

  virtual ContainerPtr createRankingInputs(const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>();
    size_t n = protein->getLength();

    TypePtr elementsType = pairClass(proteinClass, positiveIntegerType);
    ContainerPtr res = objectVector(elementsType, n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, Variable::pair(input, i, elementsType));
    return res;
  }

  virtual ContainerPtr createRankingCosts(const Variable& sup) const
  {
    const ContainerPtr& supervision = sup.getObjectAndCast<Container>();
    size_t n = supervision->getNumElements();
    ContainerPtr res = vector(doubleType, n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, supervision->getElement(i).getDouble() < 0.5 ? 1.0 : 0.0);
    return res;
  }

  virtual Variable createCutoffInput(const Variable& input) const
    {return input;}

  virtual double computeBestCutoff(const ContainerPtr& scores, const ContainerPtr& costs) const
  {
    ROCAnalyse roc;
    size_t n = scores->getNumElements();
    jassert(n == costs->getNumElements());
    for (size_t i = 0; i < n; ++i)
      roc.addPrediction(scores->getElement(i).getDouble(), costs->getElement(i).getDouble() == 0.0);
    double bestMCC;
    double res = roc.findThresholdMaximisingMCC(bestMCC);
    MessageCallback::info(T("computeBestCutoff: ") + String(res) + T(" (MCC = ") + String(bestMCC) + T(")"));
    return res;
  }

  virtual Variable computeOutput(const ContainerPtr& scores, double cutoff) const
  {
    size_t n = scores->getNumElements();
    VectorPtr res = vector(probabilityType, n);
    for (size_t i = 0; i < n; ++i)
    {
      static const double temperature = 1.0;
      double score = scores->getElement(i).getDouble() - cutoff;
      double probability = 1.0 / (1.0 + exp(-score * temperature));
      res->setElement(i, probability);
    }
    return res;
  }
};

///////////////////////////////////////////////

InferenceContextPtr createInferenceContext()
{
  return multiThreadedInferenceContext(new ThreadPool(7, false));
}

class ExtraTreeProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  virtual PerceptionPtr createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const
  {
    PerceptionPtr res = ProteinInferenceFactory::createPerception(targetName, is1DTarget, is2DTarget);
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
  virtual InferencePtr createTargetInference(const String& targetName) const
  {
    InferencePtr res = ProteinInferenceFactory::createTargetInference(targetName);
    //res->setBatchLearner(onlineToBatchInferenceLearner());
    return res;
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

  virtual PerceptionPtr createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const
  {
    static int count = 1;
    if (count++ == 1)
      return ProteinInferenceFactory::createPerception(targetName, is1DTarget, is2DTarget);
    else
    {
      PerceptionPtr res = collapsePerception(ProteinInferenceFactory::createPerception(targetName, is1DTarget, is2DTarget));
      std::vector<std::vector<size_t> > conjunctions(res->getNumOutputVariables());
      for (size_t i = 0; i < conjunctions.size(); ++i)
        conjunctions[i].push_back(i);
      return selectAndMakeConjunctionFeatures(res, conjunctions);
    }
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
    StaticDecoratorInferencePtr res = binaryLinearSVMInference(perception, createOnlineLearner(targetName), targetName);
    //if (targetName.startsWith(T("contactMap")) || targetName == T("disorderRegions"))
    //  res->setSubInference(addBiasInference(targetName, res->getSubInference()));
    res->setBatchLearner(onlineToBatchInferenceLearner());
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

   // return multiClassLinearSVMInference(perception, classes, createOnlineLearner(targetName, 0.5), false, targetName);
*/
  
    //InferencePtr binaryClassifier = createBinaryClassifier(targetName, perception);
    //InferencePtr res = oneAgainstAllClassificationInference(targetName, classes, binaryClassifier);
    //return res;

    InferencePtr rankingInference = allPairsRankingLinearSVMInference(inputLabelPairPerception(perception, classes), createOnlineLearner(targetName), targetName);
    return rankingBasedClassificationInference(targetName, rankingInference, classes);
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
        InferenceOnlineLearner::perPass, //perStepMiniBatch1000,                                                 // randomization
        InferenceOnlineLearner::perStep, invLinearIterationFunction(1.0, (size_t)5e6), true, // learning steps
        InferenceOnlineLearner::perStepMiniBatch20, l2RegularizerFunction(1e-8));         // regularizer

    res->getLastLearner()->setNextLearner(stoppingCriterionOnlineLearner(InferenceOnlineLearner::perPass, maxIterationsStoppingCriterion(1), true)); // stopping criterion
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
    //if (inferenceName == T("LearningPass"))
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
  size_t maxCount = 500;
#endif // JUCE_DEBUG
  //return directoryFileStream(directory)->load(maxCount)->apply(loadFromFileFunction(proteinClass), pool)
//    ->apply(proteinToInputOutputPairFunction(), false)->randomize();

  return directoryPairFileStream(inputDirectory, supervisionDirectory)->load(maxCount)
      ->apply(loadFromFilePairFunction(proteinClass, proteinClass), pool)->randomize();
}

void initializeLearnerByCloning(InferencePtr inference, InferencePtr inferenceToClone)
{
  inference->setBatchLearner(multiPassInferenceLearner(initializeByCloningInferenceLearner(inferenceToClone), inference->getBatchLearner()));
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

  ContainerPtr trainProteins = loadProteins(workingDirectory.getChildFile(T("trainPass1")), workingDirectory.getChildFile(T("train")), pool);
  ContainerPtr testProteins = loadProteins(workingDirectory.getChildFile(T("testPass1")), workingDirectory.getChildFile(T("test")), pool);
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

  //InferencePtr lastStep = factory->createInferenceStep(T("secondaryStructure"));
  //inference->appendInference(lastStep);
  /*for (int i = 1; i < 2; ++i)
  {
    InferencePtr step = factory->createInferenceStep(T("secondaryStructure"));
    //initializeLearnerByCloning(step, lastStep);
    inference->appendInference(step);
    lastStep = step;
  } */

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
  //return 0;
  

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
