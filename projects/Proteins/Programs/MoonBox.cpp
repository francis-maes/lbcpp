/*
 *  MoonBox.lcpp
 *  LBCpp
 *
 *  Created by Becker Julien on 15/07/10.
 *  Copyright 2010 University of Liège. All rights reserved.
 *
 */

#include <lbcpp/lbcpp.h>

#include "Programs/ArgumentSet.h"

#include "Data/Protein.h"

#include "Inference/ProteinInferenceFactory.h"
#include "Inference/ProteinInference.h"
#include "Evaluator/ProteinEvaluator.h"


using namespace lbcpp;

extern void declareLBCppClasses();
extern void declareProteinClasses();

struct DefaultParameters
{
  static bool   useExtraTrees;
  static double learningRate;
  static size_t learningRateUpdate;
  static bool   useConstantLearning; // TODO
  static double regularizer;
  static size_t stoppingIteration;
  static bool   forceUse; // TODO
  static bool   saveIterations;
  static size_t numThreads;
};

ThreadPoolPtr createThreadPool()
{
  ThreadPoolPtr threadPool = new ThreadPool(DefaultParameters::numThreads, false);
  return threadPool;
}

InferenceContextPtr createInferenceContext()
{
  InferenceContextPtr context = multiThreadedInferenceContext(createThreadPool());
  return context;
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
  NumericalProteinInferenceFactory(size_t windowSize)
  : windowSize(windowSize) {}

  virtual void getPerceptionRewriteRules(PerceptionRewriterPtr rewriter) const
  {
    rewriter->addRule(booleanType, booleanFeatures());
    rewriter->addEnumValueFeaturesRule();
    rewriter->addRule(negativeLogProbabilityType, defaultPositiveDoubleFeatures(30, -3, 3));
    rewriter->addRule(probabilityType, defaultProbabilityFeatures());
    rewriter->addRule(positiveIntegerType, defaultPositiveIntegerFeatures());
    rewriter->addRule(integerType, defaultIntegerFeatures());
    
    // all other features
    rewriter->addRule(doubleType, identityPerception());
  }

/*  
  virtual PerceptionPtr createLabelSequencePerception(const String& targetName) const
  {
    TypePtr targetType = getTargetType(targetName)->getTemplateArgument(0);
    return applyPerceptionOnProteinVariable(targetName, windowPerception(targetType, windowSize));
  }
*/
  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
  {
    return binaryLinearSVMInference(perception, createOnlineLearner(targetName + T(" Learner")), targetName + T(" Classifier"));
  }

  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
  {
    //InferencePtr binaryClassifier = createBinaryClassifier(targetName, perception);
    //InferencePtr res = oneAgainstAllClassificationInference(targetName, classes, binaryClassifier);
    
    InferencePtr res = multiClassLinearSVMInference(perception, classes, createOnlineLearner(targetName), true, targetName);
    if (DefaultParameters::saveIterations)
      res->setBatchLearner(onlineToBatchInferenceLearner());
    return res;
  }

protected:
  InferenceOnlineLearnerPtr createOnlineLearner(const String& targetName) const
  {
    double learningRate = DefaultParameters::learningRate;
    size_t learningRateUpdate = DefaultParameters::learningRateUpdate;
    double regularizer = DefaultParameters::regularizer;
    if (targetName == T("secondaryStructure"))
    {
      learningRate = 2;
      learningRateUpdate = (size_t)5e4;
      regularizer = 1e-5;
    }
    else if (targetName == T("dsspSecondaryStructure"))
    {
      learningRate = 2.0;
      learningRateUpdate = (size_t)1e5;
      regularizer = 1e-7;
    }
    else if (targetName == T("solventAccessibilityAt20p"))
    {
      learningRate = 1.0;
      learningRateUpdate = (size_t)2e5;
      regularizer = 1e-5;
    }
    else if (targetName == T("disorderRegions"))
    {
      learningRate = 2.0;
      learningRateUpdate = (size_t)2e7;
      regularizer = 1e-8;
    }
    else if (targetName == T("structuralAlphabetSequence"))
    {
      learningRate = 0.1;
      learningRateUpdate = (size_t)1e4;
      regularizer = 0;
    }

    StoppingCriterionPtr stoppingCriterion = logicalOr(maxIterationsStoppingCriterion(DefaultParameters::stoppingIteration),
                                                       maxIterationsWithoutImprovementStoppingCriterion(4));

    if (DefaultParameters::forceUse)
      stoppingCriterion = maxIterationsStoppingCriterion(DefaultParameters::stoppingIteration);

    IterationFunctionPtr learningStepFunction = DefaultParameters::useConstantLearning ? constantIterationFunction(DefaultParameters::learningRate)
                                                                              : invLinearIterationFunction(learningRate, learningRateUpdate);



    if (targetName.startsWith(T("contactMap")))
      return gradientDescentInferenceOnlineLearner(
                                                   InferenceOnlineLearner::perEpisode,                                                 // randomization
                                                   InferenceOnlineLearner::perStep, invLinearIterationFunction(DefaultParameters::learningRate, 100000), true, // learning steps
                                                   InferenceOnlineLearner::perStepMiniBatch20, l2Regularizer(0.0),         // regularizer
                                                   InferenceOnlineLearner::perPass, stoppingCriterion, true);                     // stopping criterion
    else
      return gradientDescentInferenceOnlineLearner(
                                                   InferenceOnlineLearner::perPass,                                                 // randomization
                                                   InferenceOnlineLearner::perStep, learningStepFunction, true, // learning steps
                                                   InferenceOnlineLearner::perStepMiniBatch20, l2Regularizer(regularizer),         // regularizer
                                                   InferenceOnlineLearner::perPass, stoppingCriterion, true);                     // stopping criterion
  }
  
private:
  size_t windowSize;
};

/////////////////////////////////////////

class StackPrinterCallback : public InferenceCallback
{
public:
  virtual void preInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    ScopedLock _(lock);
    const InferencePtr& currentInference = stack->getCurrentInference();
    if (stack->getDepth() > 6)
      return;
    String line;
    for (size_t i = 0; i < stack->getDepth(); ++i)
      line += T("    ");
    line += currentInference->getClassName() + T(" -> ") + currentInference->getName();
    MessageCallback::info(line);
  }
  
  virtual void postInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    ScopedLock _(lock);
    const InferencePtr& currentInference = stack->getCurrentInference();
    if (stack->getDepth() > 6)
      return;
    String line = T("END ");
    for (size_t i = 0; i < stack->getDepth() - 1; ++i)
      line += T("    ");
    line += currentInference->getClassName() + T(" -> ") + currentInference->getName() + T("\n");
    MessageCallback::info(line);
  }

private:
  CriticalSection lock;
};

class WrappedInferenceCallback : public InferenceCallback
{
public:
  WrappedInferenceCallback() : newEvaluators(false) {}
  
  void setTrainingEvaluator(ProteinEvaluatorPtr trainingEvaluator)
  {
    this->trainingEvaluator = trainingEvaluator;
    newEvaluators = true;
  }

  void setTestingEvaluator(ProteinEvaluatorPtr testingEvaluator)
  {
    this->testingEvaluator = testingEvaluator;
    newEvaluators = true;
  }

protected:
  ProteinEvaluatorPtr getTrainingEvaluator()
    {return trainingEvaluator;}

  ProteinEvaluatorPtr getTestingEvaluator()
    {return testingEvaluator;}
  
  bool hasNewEvaluators()
  {
    bool res = newEvaluators;
    newEvaluators = false;
    return res;
  }

private:
  ProteinEvaluatorPtr trainingEvaluator;
  ProteinEvaluatorPtr testingEvaluator;
  bool newEvaluators;
};

typedef ReferenceCountedObjectPtr<WrappedInferenceCallback> WrappedInferenceCallbackPtr;

class WrapperInferenceCallback : public InferenceCallback
{
public:
  WrapperInferenceCallback(InferencePtr inference, ContainerPtr trainingData, ContainerPtr testingData, const String& inferenceNameToEvaluate)
  : inference(inference), trainingData(trainingData), testingData(testingData), inferenceNameToEvaluate(inferenceNameToEvaluate) {}

  void appendCallback(WrappedInferenceCallbackPtr callback)
  {
    callbacks.push_back(callback);
  }

  virtual void preInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
      callbacks[i]->preInferenceCallback(context, stack, input, supervision, output, returnCode);
  }

  virtual void postInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    InferencePtr currentInference = stack->getCurrentInference();
    if (currentInference->getName() == inferenceNameToEvaluate) // T("Pass learner")
    {
      ProteinEvaluatorPtr trainingEvaluator = new ProteinEvaluator();
      context->evaluate(inference, trainingData, trainingEvaluator);

      ProteinEvaluatorPtr testingEvaluator = new ProteinEvaluator();
      context->evaluate(inference, testingData, testingEvaluator);

      for (size_t i = 0; i < callbacks.size(); ++i)
      {
        callbacks[i]->setTrainingEvaluator(trainingEvaluator);
        callbacks[i]->setTestingEvaluator(testingEvaluator);
      }
    }

    for (size_t i = 0; i < callbacks.size(); ++i)
      callbacks[i]->postInferenceCallback(context, stack, input, supervision, output, returnCode);
  }

private:
  InferencePtr inference;
  ContainerPtr trainingData;
  ContainerPtr testingData;
  std::vector<WrappedInferenceCallbackPtr> callbacks;
  const String inferenceNameToEvaluate;
};

typedef ReferenceCountedObjectPtr<WrapperInferenceCallback> WrapperInferenceCallbackPtr;

class StandardOutputInferenceCallback : public WrappedInferenceCallback
{
public:
  virtual void preInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
    {
      // top-level learning is beginning
      startingTime = Time::getMillisecondCounter();
      iterationNumber = 0;
    }

    InferencePtr currentInference = stack->getCurrentInference();
    if (currentInference->getClassName().contains(T("Learner")) && input.size() == 2)
    {
      TypePtr trainingExamplesType = input[1].getObjectAndCast<Container>()->getElementsType();
      jassert(trainingExamplesType->getNumTemplateArguments() == 2);
      String inputTypeName = trainingExamplesType->getTemplateArgument(0)->getName();
      MessageCallback::info(T("=== Learning ") + input[0].getObject()->getName() + T(" with ") + String((int)input[1].size()) + T(" ") + inputTypeName + T("(s) ==="));
    }
  }

  virtual void postInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (hasNewEvaluators())
    {
      // end of learning pass
      MessageCallback::info(String("\n====================================================\n") + 
        T("===================  EVALUATION  ===================  ") + String((Time::getMillisecondCounter() - startingTime) / 1000) + T(" s\n") +
        T("====================================================\n"));

      ProteinEvaluatorPtr evaluator = getTrainingEvaluator();
      processResults(evaluator, true);

      evaluator = getTestingEvaluator();
      processResults(evaluator, false);

      MessageCallback::info(T("=====================================================\n"));
    }
    else if (stack->getDepth() == 1)
    {
      MessageCallback::info(T("Bye."));
    }
  }

  void processResults(ProteinEvaluatorPtr evaluator, bool isTrainingData)
    {MessageCallback::info(String(T(" == ")) + (isTrainingData ? T("Training") : T("Testing")) + T(" Scores = \n") + evaluator->toString());}

private:
  InferencePtr inference;
  ContainerPtr trainingData, testingData;
  size_t iterationNumber;
  juce::uint32 startingTime;
};

class GnuPlotInferenceCallback : public WrappedInferenceCallback {
public:
  GnuPlotInferenceCallback(const File& prefixFile, std::vector<String> targets)
    : prefixFile(prefixFile), startingTime(0), nbPass(0), targets(targets), files(std::vector<File>(targets.size()))
  {
    for (size_t i = 0; i < targets.size(); ++i)
    {
      files[i] = prefixFile.getFullPathName() + T(".") + targets[i];
      if (files[i].exists())
        files[i].deleteFile();
    }
  }

  virtual void preInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
    {
      // top-level learning is beginning
      startingTime = Time::getMillisecondCounter();
    }
  }

  virtual void postInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (hasNewEvaluators())
    {
      ScopedLock _(fileLock);
      for (size_t i = 0; i < targets.size(); ++i)
      {
        OutputStream* o = files[i].createOutputStream();
        *o << (int)nbPass << '\t'
        << getTrainingEvaluator()->getEvaluatorForTarget(targets[i])->getDefaultScore() << '\t'
        << getTestingEvaluator()->getEvaluatorForTarget(targets[i])->getDefaultScore() << '\t'
        << String((int)(Time::getMillisecondCounter() - startingTime) / 1000) << '\n';
        delete o;
      }

      ++nbPass;
    }
  }

private:
  CriticalSection fileLock;
  File prefixFile;
  juce::uint32 startingTime;
  size_t nbPass;
  std::vector<String> targets;
  std::vector<File> files;
};

/*------------------------------------------------------------------------------
 | Main Function
 -----------------------------------------------------------------------------*/

bool   DefaultParameters::useExtraTrees       = false;
double DefaultParameters::learningRate        = 1.;
size_t DefaultParameters::learningRateUpdate  = 10000;
bool   DefaultParameters::useConstantLearning = false;
double DefaultParameters::regularizer         = 0.;
size_t DefaultParameters::stoppingIteration   = 20;
bool   DefaultParameters::forceUse            = false;
bool   DefaultParameters::saveIterations      = false;
size_t DefaultParameters::numThreads          = 1;

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();

  enum {numFolds = 5};
  /*
  ** Parameters initialization
  */
  File proteinsDirectory;//(T("/Users/jbecker/Documents/Workspace/CASP9/SmallPDB/test_version"));
  File testingProteinsDirectory;
  int numProteinsToLoad = 0;
  std::vector<String> targets;
  String output(T("result"));
  bool generateIntermediate = false;

  size_t windowSize = 15; // TODO

  bool isTestVersion = false;
  String multiTaskFeatures;
  bool isExperimentalMode = false;
  size_t foldCrossValidation = 0;

  ArgumentSet arguments;
  /* Input-Output */
  arguments.insert(new FileArgument(T("ProteinsDirectory"), proteinsDirectory, true, true));
  arguments.insert(new FileArgument(T("TestingProteinsDirectory"), testingProteinsDirectory, true, true));
  arguments.insert(new IntegerArgument(T("NumProteinsToLoad"), numProteinsToLoad));
  arguments.insert(new TargetExpressionArgument(T("Targets"), targets), true);
  arguments.insert(new StringArgument(T("Output"), output));
  arguments.insert(new BooleanArgument(T("GenerateIntermediate"), generateIntermediate));
  arguments.insert(new BooleanArgument(T("SaveIterations"), DefaultParameters::saveIterations));
  /* Learning Parameters */
  arguments.insert(new BooleanArgument(T("useExtraTrees"), DefaultParameters::useExtraTrees));
  arguments.insert(new DoubleArgument(T("LearningRate"), DefaultParameters::learningRate));
  arguments.insert(new IntegerArgument(T("LearningStep"), (int&)DefaultParameters::learningRateUpdate));
  arguments.insert(new DoubleArgument(T("Regularizer"), DefaultParameters::regularizer));
  arguments.insert(new IntegerArgument(T("StoppingIteration"), (int&)DefaultParameters::stoppingIteration));
  /* Perception Parameters */
  arguments.insert(new IntegerArgument(T("WindowSize"), (int&)windowSize));
  // ...
  /* Modes */
  arguments.insert(new BooleanArgument(T("IsTestVersion"), isTestVersion));
  arguments.insert(new BooleanArgument(T("IsExperimentalMode"), isExperimentalMode));
  arguments.insert(new IntegerArgument(T("FoldCrossValidation"), (int&)foldCrossValidation));
  arguments.insert(new IntegerArgument(T("NumThreads"), (int&)DefaultParameters::numThreads));

  if (!arguments.parse(argv, 1, argc-1))
  {
    std::cout << "Usage: " << argv[0] << " " << arguments.toString() << std::endl;
    return -1;
  }

  if (isTestVersion)
  {
    if (!numProteinsToLoad)
      numProteinsToLoad = numFolds;
    DefaultParameters::stoppingIteration = 2;
  }

  if (isExperimentalMode)
  {
    DefaultParameters::useConstantLearning = true;
    DefaultParameters::saveIterations = true;
    DefaultParameters::forceUse = true;
  }

  std::cout << "*---- Program Parameters -----" << std::endl;
  std::cout << arguments;
  std::cout << "*-----------------------------" << std::endl;

  /*
  ** Loading proteins
  */
  ContainerPtr trainingData = directoryFileStream(proteinsDirectory, T("*.xml"))
                            ->load(numProteinsToLoad)
                            ->apply(loadFromFileFunction(proteinClass), createThreadPool())
                            ->apply(proteinToInputOutputPairFunction())
                            ->randomize();
  ContainerPtr testingData;

  if (testingProteinsDirectory != File::nonexistent)
  {
    testingData = directoryFileStream(testingProteinsDirectory, T("*.xml"))
                ->load()
                ->apply(loadFromFileFunction(proteinClass), createThreadPool())
                ->apply(proteinToInputOutputPairFunction());
  }
  
  if (foldCrossValidation && testingProteinsDirectory != File::nonexistent)
  {
    std::cout << "Warning - You are in Cross Validation Mode,"
              << "the testing set isn't taken into account !" << std::endl;
  }
  
  if (!foldCrossValidation && testingProteinsDirectory == File::nonexistent)
  {
    testingData = trainingData->fold(0, numFolds);
    trainingData = trainingData->invFold(0, numFolds);
  }

  if (!foldCrossValidation)
    std::cout << trainingData->getNumElements() << " Training Proteins & "
              << testingData->getNumElements()  << " Testing Proteins" << std::endl;
  else
    std::cout << foldCrossValidation << "-Fold Cross Validation & "
              << trainingData->getNumElements() << " Proteins" << std::endl;

  /*
  ** Selection of the Protein Inference Factory
  */
  ProteinInferenceFactoryPtr factory;
  if (DefaultParameters::useExtraTrees)
    factory = new ExtraTreeProteinInferenceFactory();
  else
    factory = new NumericalProteinInferenceFactory(windowSize);

  /*
  ** Creation of the inference
  */
  ProteinSequentialInferencePtr inference = new ProteinSequentialInference();
  jassert(!(generateIntermediate && foldCrossValidation));
  if (generateIntermediate)
    inference->setProteinDebugDirectory(File::getCurrentWorkingDirectory().getChildFile(output));

  for (size_t i = 0; i < targets.size(); ++i)
  {
    ProteinSequentialInferencePtr inferencePass = new ProteinSequentialInference("Pass"); //new ProteinParallelInference("Pass");
    if (targets[i].contains(T("SS3")))
      inferencePass->appendInference(factory->createInferenceStep(T("secondaryStructure")));
    if (targets[i].contains(T("SS8")))
      inferencePass->appendInference(factory->createInferenceStep(T("dsspSecondaryStructure")));
    if (targets[i].contains(T("SA")))
      inferencePass->appendInference(factory->createInferenceStep(T("solventAccessibilityAt20p")));
    if (targets[i].contains(T("DR")))
      inferencePass->appendInference(factory->createInferenceStep(T("disorderRegions")));
    if (targets[i].contains(T("StAl")))
      inferencePass->appendInference(factory->createInferenceStep(T("structuralAlphabetSequence")));
    inference->appendInference(inferencePass);
  }

  /*
  ** Setting Callbacks
  */
  InferenceContextPtr context = createInferenceContext(); // = singleThreadedInferenceContext();

  /*
  ** Run
  */
  if (foldCrossValidation)
  {
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    context->crossValidate(inference, trainingData, evaluator, foldCrossValidation);
    std::cout << evaluator->toString() << std::endl;
  }
  else
  {
    std::map<String, String> targetsMap;
    for (size_t i = 0; i < targets.size(); ++i)
    {
      if (targets[i].contains(T("SS3")))
        targetsMap[T("SS3")] = T("secondaryStructure");
      if (targets[i].contains(T("SS8")))
        targetsMap[T("SS8")] = T("dsspSecondaryStructure");
      if (targets[i].contains(T("SA")))
        targetsMap[T("SA")] = T("solventAccessibilityAt20p");
      if (targets[i].contains(T("DR")))
        targetsMap[T("DR")] = T("disorderRegions");
      if (targets[i].contains(T("StAl")))
        targetsMap[T("StAl")] = T("structuralAlphabetSequence");
    }
    
    std::vector<String> targetsName;
    for (std::map<String, String>::iterator it = targetsMap.begin(); it != targetsMap.end(); ++it)
      targetsName.push_back(it->second);

    const String inferenceNameToEvaluate = DefaultParameters::saveIterations ? T("LearningPass") : T("Pass learner");
    WrapperInferenceCallbackPtr callbacks = new WrapperInferenceCallback(inference, trainingData, testingData, inferenceNameToEvaluate);
    callbacks->appendCallback(new StandardOutputInferenceCallback());
    callbacks->appendCallback(new GnuPlotInferenceCallback(File::getCurrentWorkingDirectory().getChildFile(output), targetsName));
    //context->appendCallback(new StackPrinterCallback());
    context->appendCallback(callbacks);

    context->train(inference, trainingData);
  }

  lbcpp::deinitialize();
  return 0;
}
