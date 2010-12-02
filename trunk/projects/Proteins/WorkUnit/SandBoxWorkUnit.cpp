/*-----------------------------------------.---------------------------------.
| Filename: SandBoxWorkUnit.cpp            | Sand Box Work Unit              |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 13:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "SandBoxWorkUnit.h"
using namespace lbcpp;

ExecutionContextPtr createInferenceContext()
{
  return defaultConsoleExecutionContext();
}

class ExtraTreeProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  ExtraTreeProteinInferenceFactory(ExecutionContext& context)
    : ProteinInferenceFactory(context) {}

  virtual PerceptionPtr createPerception(const String& targetName, PerceptionType type) const
  {
    PerceptionPtr res = ProteinInferenceFactory::createPerception(targetName, type);
    return res ? flattenPerception(res) : PerceptionPtr();
  }

  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
    {return binaryClassificationExtraTreeInference(context, targetName, perception, 2, 3);}

  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
    {return classificationExtraTreeInference(context, targetName, perception, classes, 2, 3);}
};

class SandBoxNumericalProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  SandBoxNumericalProteinInferenceFactory(ExecutionContext& context)
    : ProteinInferenceFactory(context) {}

  virtual PerceptionPtr createPerception(const String& targetName, PerceptionType type) const
  {
    PerceptionPtr res = ProteinInferenceFactory::createPerception(targetName, type);
    return res ? flattenPerception(res) : PerceptionPtr();
  }
/*
  virtual InferencePtr createTargetInference(const String& targetName) const
  {
    InferencePtr res = ProteinInferenceFactory::createTargetInference(targetName);
    res->setBatchLearner(stochasticInferenceLearner());
    return res;
  }*/
 
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

/*
  std::vector<size_t> makeBinaryConjunction(size_t index1, size_t index2) const
    {std::vector<size_t> res(2); res[0] = index1; res[1] = index2; return res;}
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
    NumericalSupervisedInferencePtr res = binaryLinearSVMInference(targetName, perception);
    res->setStochasticLearner(createOnlineLearner(targetName));

    if (targetName.startsWith(T("contactMap")) || targetName == T("disorderRegions") || targetName == T("solventAccessibilityAt20p") || targetName == T("disulfideBonds"))
    {
      VectorSequentialInferencePtr sequentialInference = new VectorSequentialInference(targetName);
      sequentialInference->appendInference(res->getSubInference());
      sequentialInference->appendInference(addBiasInference(targetName + T(" bias")));
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
      multiLinearInference->addOnlineLearner(stoppingCriterionOnlineLearner(maxIterationsStoppingCriterion(15), true));
    else
    {
      multiLinearInference->addOnlineLearner(graftingOnlineLearner(perception, multiLinearInference));
      multiLinearInference->addOnlineLearner(stoppingCriterionOnlineLearner(maxIterationsStoppingCriterion(1000), true));
    }
    return res;

    
*/
    NumericalSupervisedInferencePtr svm = multiClassLinearSVMInference(targetName, perception, classes);
    svm->setStochasticLearner(createOnlineLearner(targetName, 0.5, classificationAccuracyEvaluator()), true, true);
  
    /*svm->getSubInference()->setBatchLearner(
      precomputePerceptionsNumericalInferenceLearner(
            new OptimizerInferenceLearner(new AlaRacheOptimizer(), stochasticInferenceLearner(true))));*/
    return svm;
  
    /*InferencePtr binaryClassifier = createBinaryClassifier(targetName, perception);
    InferencePtr res = oneAgainstAllClassificationInference(targetName, classes, binaryClassifier);
    return res;*/

    //InferencePtr rankingInference = allPairsRankingInference(linearInference(targetName, inputLabelPairPerception(perception, classes)), hingeLossFunction(true), 
    //  createOnlineLearner(targetName), targetName);

      //mostViolatedPairRankingLinearSVMInference(inputLabelPairPerception(perception, classes), createOnlineLearner(targetName), targetName);
    //return rankingBasedClassificationInference(targetName, rankingInference, classes);
  }

protected:
  InferenceOnlineLearnerPtr createOnlineLearner(const String& targetName, double initialLearningRate = 1.0, EvaluatorPtr evaluator = EvaluatorPtr()) const
  {
    InferenceOnlineLearnerPtr res, lastLearner;
    if (targetName.startsWith(T("contactMap")))
    {
      res = randomizerOnlineLearner(perEpisode);
      lastLearner = res->setNextLearner(gradientDescentOnlineLearner(
        perStep, invLinearIterationFunction(initialLearningRate, 100000), true, // learning steps
        perStepMiniBatch20, l2RegularizerFunction(0.0)));         // regularizer
    }
    else
      res = lastLearner = gradientDescentOnlineLearner(
        perStep, constantIterationFunction(0.1), true, //  invLinearIterationFunction(initialLearningRate, (size_t)5e6), // learning steps
        perStepMiniBatch20, l2RegularizerFunction(1e-8));         // regularizer

    if (evaluator)
    {
      EvaluatorPtr trainEvaluator = evaluator->cloneAndCast<Evaluator>(context);
      trainEvaluator->setName(T("trainScore"));
      lastLearner = lastLearner->setNextLearner(computeEvaluatorOnlineLearner(trainEvaluator, false));

      EvaluatorPtr validationEvaluator = evaluator->cloneAndCast<Evaluator>(context);
      validationEvaluator->setName(T("validationScore"));
      lastLearner = lastLearner->setNextLearner(computeEvaluatorOnlineLearner(validationEvaluator, true));
    }

    StoppingCriterionPtr stoppingCriterion = logicalOr(maxIterationsStoppingCriterion(3), maxIterationsWithoutImprovementStoppingCriterion(5));
    lastLearner = lastLearner->setNextLearner(stoppingCriterionOnlineLearner(stoppingCriterion, true)); // stopping criterion

    //File workingDirectory(T("C:\\Projets\\lbcpp\\projects\\temp\\psipred"));
    //lastLearner = lastLearner->setNextLearner(saveScoresToGnuPlotFileOnlineLearner(workingDirectory.getChildFile(T("results.txt"))));
    return res;
  }
};

/////////////////////////////////////////

class MyInferenceCallback : public ExecutionCallback
{
public:
  MyInferenceCallback(InferencePtr inference, ContainerPtr trainingData, ContainerPtr testingData)
    : inference(inference), trainingData(trainingData), testingData(testingData), iterationNumber(0) {}

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit)
  {
/*
    InferenceWorkUnitPtr inferenceWorkUnit = workUnit.dynamicCast<InferenceWorkUnit>();
    if (!inferenceWorkUnit)
      return;

    InferenceBatchLearnerInputPtr learnerInput = inferenceWorkUnit->getInput().dynamicCast<InferenceBatchLearnerInput>();
    if (learnerInput)
    {
      String inputTypeName = learnerInput->getTrainingExamples()->getElementsType()->getTemplateArgument(0)->getName();

      String info = T("=== Learning ") + learnerInput->getTargetInference()->getName() + T(" with ");
      info += String((int)learnerInput->getNumTrainingExamples());
      if (learnerInput->getNumValidationExamples())
        info += T(" + ") + String((int)learnerInput->getNumValidationExamples());

      info += T(" ") + inputTypeName + T("(s) ===");
      getContext().informationCallback(info);
    }*/
  }

  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit, bool result)
  {
    ExecutionContext& context = getContext();
    
    InferenceWorkUnitPtr inferenceWorkUnit = workUnit.dynamicCast<InferenceWorkUnit>();
    if (!inferenceWorkUnit)
      return;
    const InferencePtr& inference = inferenceWorkUnit->getInference();
    const Variable& input = inferenceWorkUnit->getInput();

    String inferenceName = inference->getName();

    //if (stack->getDepth() == 1) // 
    //if (context.getCurrentFunction()->getClassName() == T("RunSequentialInferenceStepOnExamples"))
    if (inferenceName.startsWith(T("Learning Iteration")))
    {
      // end of learning iteration
      //context.informationCallback(T("====================================================="));
      //context.informationCallback(T("================ EVALUATION =========================  ") + String((Time::getMillisecondCounter() - startingTime) / 1000) + T(" s"));
      //context.informationCallback(T("====================================================="));

      InferenceBatchLearnerInputPtr learnerInput = input.dynamicCast<InferenceBatchLearnerInput>();
      if (learnerInput)
      {
        const InferencePtr& targetInference = learnerInput->getTargetInference();
        if (targetInference && targetInference->getOnlineLearner())
        {
          std::vector< std::pair<String, double> > scores;
          targetInference->getLastOnlineLearner()->getScores(scores);
          String info;
          for (size_t i = 0; i < scores.size(); ++i)
            info += T("Score ") + scores[i].first + T(": ") + String(scores[i].second) + T("\n");
          context.informationCallback(info);
        }
      }
/*
      ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
      inference->evaluate(context, trainingData, evaluator);
      processResults(evaluator, true);

      evaluator = new ProteinEvaluator();
      inference->evaluate(context, testingData, evaluator);
      processResults(evaluator, false);*/

      //context.informationCallback(T("====================================================="));
    }

  }

  void processResults(ProteinEvaluatorPtr evaluator, bool isTrainingData)
    {std::cout << " == " << (isTrainingData ? "Training" : "Testing") << " Scores == " << std::endl << evaluator->toString() << std::endl;}

private:
  InferencePtr inference;
  ContainerPtr trainingData, testingData;
  size_t iterationNumber;
};

VectorPtr SandBoxWorkUnit::loadProteins(ExecutionContext& context, const String& workUnitName, const File& inputDirectory, const File& supervisionDirectory)
{
#ifdef JUCE_DEBUG
  size_t maxCount = 2;
#else
  size_t maxCount = 500;
#endif // JUCE_DEBUG
  if (inputDirectory.exists())
    return directoryPairFileStream(inputDirectory, supervisionDirectory)->load(context, maxCount)
      ->apply(context, loadFromFilePairFunction(proteinClass, proteinClass), Container::parallelApply, workUnitName)->randomize();
  else
    return directoryFileStream(supervisionDirectory)->load(context, maxCount)
      ->apply(context, composeFunction(loadFromFileFunction(proteinClass), proteinToInputOutputPairFunction(false)), Container::parallelApply, workUnitName)
      ->randomize();
}

void SandBoxWorkUnit::initializeLearnerByCloning(InferencePtr inference, InferencePtr inferenceToClone)
{
  inference->setBatchLearner(multiPassInferenceLearner(initializeByCloningInferenceLearner(inferenceToClone), inference->getBatchLearner()));
}

bool SandBoxWorkUnit::run(ExecutionContext& context)
{
#ifdef JUCE_WIN32
  File workingDirectory(T("C:\\Projets\\lbcpp\\projects\\temp\\psipred"));
#else
  File workingDirectory(T("/data/PDB/PDB30Medium"));
#endif
  bool inputOnly = true;
  ContainerPtr trainProteins = loadProteins(context, T("Loading training proteins"), inputOnly ? File::nonexistent : workingDirectory.getChildFile(T("trainCO")), workingDirectory.getChildFile(T("train")));
  ContainerPtr testProteins = loadProteins(context, T("Loading testing proteins"), inputOnly ? File::nonexistent : workingDirectory.getChildFile(T("testCO")), workingDirectory.getChildFile(T("test")));
  ContainerPtr validationProteins = trainProteins->fold(0, 3);
  trainProteins = trainProteins->invFold(0, 3);

  context.informationCallback(String((int)trainProteins->getNumElements()) + T(" training proteins, ")  +
    String((int)validationProteins->getNumElements()) + T(" validation proteins, ")  +
    String((int)testProteins->getNumElements()) + T(" testing proteins"));

  //ProteinInferenceFactoryPtr factory = new ExtraTreeProteinInferenceFactory(context);
  ProteinInferenceFactoryPtr factory = new SandBoxNumericalProteinInferenceFactory(context);

  //ProteinParallelInferencePtr inference = new ProteinParallelInference();
  //inference->setProteinDebugDirectory(workingDirectory.getChildFile(T("proteins")));
  //inference->appendInference(factory->createInferenceStep(T("contactMap8Ca")));

  //inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  /*inference->appendInference(factory->createInferenceStep(T("dsspSecondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("solventAccessibilityAt20p")));
  inference->appendInference(factory->createInferenceStep(T("disorderRegions")));
  inference->appendInference(factory->createInferenceStep(T("structuralAlphabetSequence")));*/

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
  //inference->appendInference(factory->createInferenceStep(T("disulfideBonds")));
  //inference->appendInference(factory->createInferenceStep(T("disulfideBonds")));
  //inference->appendInference(factory->createInferenceStep(T("disulfideBonds")));
  //inference->appendInference(factory->createInferenceStep(T("disulfideBonds")));
//  inference->appendInference(factory->createInferenceStep(T("disulfideBonds")));

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

  ProteinEvaluatorPtr evaluator = new ProteinEvaluator();

  /*
  ReferenceCountedObject::resetRefCountDebugInfo();
  context->crossValidate(inference, proteins, evaluator, 2);
  std::cout << evaluator->toString() << std::endl;
  ReferenceCountedObject::displayRefCountDebugInfo(std::cout);
  return true;*/

  {
    ExecutionCallbackPtr trainingCallback = new MyInferenceCallback(inference, trainProteins, testProteins);
    context.appendCallback(trainingCallback);
    inference->train(context, trainProteins, validationProteins, T("Training"));
    context.removeCallback(trainingCallback);
  }

  /*
  std::cout << "Making and saving train predicions..." << std::endl;
  inference->evaluate(context, trainProteins, saveToDirectoryEvaluator(workingDirectory.getChildFile(T("trainCO"))));
  std::cout << "Making and saving test predicions..." << std::endl;
  inference->evaluate(context, testProteins, saveToDirectoryEvaluator(workingDirectory.getChildFile(T("testCO"))));
  */

  {
    evaluator = new ProteinEvaluator();
    inference->evaluate(context, trainProteins, evaluator, T("Evaluating on training data"));
    std::cout << evaluator->toString() << std::endl << std::endl;

    evaluator = new ProteinEvaluator();
    inference->evaluate(context, validationProteins, evaluator, T("Evaluating on validation data"));
    std::cout << evaluator->toString() << std::endl << std::endl;

    EvaluatorPtr evaluator = new ProteinEvaluator();
    inference->evaluate(context, testProteins, evaluator, T("Evaluating on testing data"));
    std::cout << evaluator->toString() << std::endl << std::endl;
  }

  context.informationCallback(T("Saving inference"));
  inference->saveToFile(context, workingDirectory.getChildFile(T("NewStyleInference.xml")));
  return true;
  
  std::cout << "Loading..." << std::flush;
  inference = Inference::createFromFile(context, workingDirectory.getChildFile(T("NewStyleInference.xml")));
  std::cout << "ok." << std::endl;

  for (size_t i = 7; i <= 7; i += 1)
  {
    std::cout << "Check Evaluating with " << (i ? i : 1) << " threads ..." << std::endl;
    EvaluatorPtr evaluator = new ProteinEvaluator();
    ExecutionContextPtr context = multiThreadedExecutionContext(i ? i : 1);
    context->appendCallback(new MyInferenceCallback(inference, trainProteins, testProteins));
    inference->evaluate(*context, trainProteins, evaluator);
  //  inference->crossValidate(context, proteins, evaluator, 2);
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
  loadedInference->evaluate(context, trainProteins, evaluator);
  std::cout << "ok." << std::endl;
  std::cout << "============================" << std::endl << std::endl;
  std::cout << evaluator->toString() << std::endl << std::endl;
#endif // 0
  return true;
}
