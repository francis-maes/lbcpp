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
};

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
  NumericalProteinInferenceFactory(size_t windowSize)
  : windowSize(windowSize) {}

  virtual void getPerceptionRewriteRules(PerceptionRewriterPtr rewriter) const
  {
    rewriter->addRule(biVariableFeaturesPerceptionRewriteRule(hardDiscretizedNumberFeatures(probabilityType(), 0.0, 1.0, 10, false)));

    rewriter->addEnumValueFeaturesRule();

    rewriter->addRule(probabilityType(), T("POSITION"), hardDiscretizedNumberFeatures(probabilityType(), 0.0, 1.0, 10, false));
    rewriter->addRule(probabilityType(), T("TERMINUS"), hardDiscretizedNumberFeatures(probabilityType(), 0.0, 1.0, 10, false));
    rewriter->addRule(probabilityType(), T("HISTOGRAM"), hardDiscretizedNumberFeatures(probabilityType(), 0.0, 1.0, 10, false));
    rewriter->addRule(doubleType(), identityPerception());
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
    InferencePtr binaryClassifier = createBinaryClassifier(targetName, perception);
    InferencePtr res = oneAgainstAllClassificationInference(targetName, classes, binaryClassifier);
    if (DefaultParameters::saveIterations)
      res->setBatchLearner(onlineToBatchInferenceLearner());
    return res;
  }

protected:
  InferenceOnlineLearnerPtr createOnlineLearner(const String& targetName, double initialLearningRate = 1.0) const
  {
    StoppingCriterionPtr stoppingCriterion = logicalOr(maxIterationsStoppingCriterion(DefaultParameters::stoppingIteration),
                                                       maxIterationsWithoutImprovementStoppingCriterion(1));

    if (DefaultParameters::forceUse)
      stoppingCriterion = maxIterationsStoppingCriterion(DefaultParameters::stoppingIteration);

    IterationFunctionPtr learningStepFunction = DefaultParameters::useConstantLearning ? constantIterationFunction(DefaultParameters::learningRate)
                                                                              : invLinearIterationFunction(DefaultParameters::learningRate,
                                                                                                           DefaultParameters::learningRateUpdate);

    if (targetName.startsWith(T("contactMap")))
      return gradientDescentInferenceOnlineLearner(
                                                   InferenceOnlineLearner::perEpisode,                                                 // randomization
                                                   InferenceOnlineLearner::perStep, invLinearIterationFunction(initialLearningRate, 100000), true, // learning steps
                                                   InferenceOnlineLearner::perStepMiniBatch20, l2Regularizer(0.0),         // regularizer
                                                   InferenceOnlineLearner::perPass, stoppingCriterion, true);                     // stopping criterion
    else
      return gradientDescentInferenceOnlineLearner(
                                                   InferenceOnlineLearner::never,                                                 // randomization
                                                   InferenceOnlineLearner::perStep, learningStepFunction, true, // learning steps
                                                   InferenceOnlineLearner::perStepMiniBatch20, l2Regularizer(DefaultParameters::regularizer),         // regularizer
                                                   InferenceOnlineLearner::perPass, stoppingCriterion, true);                     // stopping criterion
  }
  
private:
  size_t windowSize;
};

/////////////////////////////////////////
class WrappedInferenceCallback : public InferenceCallback
{
public:
  void setTargetName(const String& targetName)
    {this->targetName = targetName;}

  void setTrainingEvaluator(ProteinEvaluatorPtr trainingEvaluator)
    {this->trainingEvaluator = trainingEvaluator;}

  void setTestingEvaluator(ProteinEvaluatorPtr testingEvaluator)
    {this->testingEvaluator = testingEvaluator;}

  String getTargetName()
    {return targetName;}

  ProteinEvaluatorPtr getTrainingEvaluator()
    {return trainingEvaluator;}

  ProteinEvaluatorPtr getTestingEvaluator()
    {return testingEvaluator;}

private:
  String targetName;
  ProteinEvaluatorPtr trainingEvaluator;
  ProteinEvaluatorPtr testingEvaluator;
};

typedef ReferenceCountedObjectPtr<WrappedInferenceCallback> WrappedInferenceCallbackPtr;

class WrapperInferenceCallback : public InferenceCallback
{
public:
  WrapperInferenceCallback(InferencePtr inference, ContainerPtr trainingData, ContainerPtr testingData, const String& classNameWhichContainTargetName, const String& classNameToEvaluate)
  : inference(inference), trainingData(trainingData), testingData(testingData), targetName(String::empty), classNameWhichContainTargetName(classNameWhichContainTargetName), classNameToEvaluate(classNameToEvaluate) {}

  void appendCallback(WrappedInferenceCallbackPtr callback)
  {
    callbacks.push_back(callback);
  }

  virtual void preInferenceCallback(InferenceStackPtr stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    String inferenceClassName = stack->getCurrentInference()->getClassName();
//    std::cout << inferenceClassName << " - " << stack->getCurrentInference()->getName() << std::endl;
    if (inferenceClassName == classNameWhichContainTargetName)
    {
      String currentTargetName = stack->getCurrentInference()->getName();
      if (currentTargetName != targetName)
      {
        targetName = currentTargetName;
        for (size_t i = 0; i < callbacks.size(); ++i)
          callbacks[i]->setTargetName(targetName);
      }
    }

    for (size_t i = 0; i < callbacks.size(); ++i)
      callbacks[i]->preInferenceCallback(stack, input, supervision, output, returnCode);
  }

  virtual void postInferenceCallback(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    String inferenceClassName = stack->getCurrentInference()->getClassName();
    if (inferenceClassName == classNameToEvaluate)
    {
      InferenceContextPtr validationContext = singleThreadedInferenceContext();

      ProteinEvaluatorPtr trainingEvaluator = new ProteinEvaluator();
      validationContext->evaluate(inference, trainingData, trainingEvaluator);

      ProteinEvaluatorPtr testingEvaluator = new ProteinEvaluator();
      validationContext->evaluate(inference, testingData, testingEvaluator);

      for (size_t i = 0; i < callbacks.size(); ++i)
      {
        callbacks[i]->setTrainingEvaluator(trainingEvaluator);
        callbacks[i]->setTestingEvaluator(testingEvaluator);
      }
    }

    for (size_t i = 0; i < callbacks.size(); ++i)
      callbacks[i]->postInferenceCallback(stack, input, supervision, output, returnCode);
  }

private:
  InferencePtr inference;
  ContainerPtr trainingData;
  ContainerPtr testingData;
  String targetName;
  std::vector<WrappedInferenceCallbackPtr> callbacks;
  String classNameWhichContainTargetName;
  String classNameToEvaluate;
};

typedef ReferenceCountedObjectPtr<WrapperInferenceCallback> WrapperInferenceCallbackPtr;

class StandardOutputInferenceCallback : public WrappedInferenceCallback
{
public:
  StandardOutputInferenceCallback(const String& classNameToEvaluate) : classNameToEvaluate(classNameToEvaluate) {}

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
    String inferenceClassName = stack->getCurrentInference()->getClassName();

    if (inferenceClassName == classNameToEvaluate)
    {
      // end of learning iteration
      std::cout << std::endl
      << "=====================================================" << std::endl
      << "================ EVALUATION =========================  " << (Time::getMillisecondCounter() - startingTime) / 1000 << " s" << std::endl
      << "=====================================================" << std::endl;

      ProteinEvaluatorPtr evaluator = getTrainingEvaluator();
      processResults(evaluator, true);

      evaluator = getTestingEvaluator();
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
  String classNameToEvaluate;
};

class GnuPlotInferenceCallback : public WrappedInferenceCallback {
public:
  GnuPlotInferenceCallback(const File& prefixFile, const String& classNameToEvaluate)
  : prefixFile(prefixFile), startingTime(0), classNameToEvaluate(classNameToEvaluate) {}

  virtual void preInferenceCallback(InferenceStackPtr stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
    {
      // top-level learning is beginning
      startingTime = Time::getMillisecondCounter();
    }
  }

  virtual void postInferenceCallback(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    String inferenceClassName = stack->getCurrentInference()->getClassName();

    if (inferenceClassName == classNameToEvaluate)
    {
      String targetName = getTargetName();
      File dst = prefixFile.getFullPathName() + T(".") + targetName;
      if (!nbIterations[targetName])
      {
        nbIterations[targetName] = 0;
        if (dst.exists())
          dst.deleteFile();
      }

      OutputStream* o = dst.createOutputStream();
      *o << (int)nbIterations[targetName] << '\t'
         << getTrainingEvaluator()->getEvaluatorForTarget(getTargetName())->getDefaultScore() << '\t'
         << getTestingEvaluator()->getEvaluatorForTarget(getTargetName())->getDefaultScore() << '\t'
         << String((int)(Time::getMillisecondCounter() - startingTime) / 1000) << '\n';
      delete o;

      ++nbIterations[targetName];
    }
  }

private:
  File prefixFile;
  juce::uint32 startingTime;
  std::map<String, size_t> nbIterations;
  String classNameToEvaluate;
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
                            ->apply(loadFromFileFunction(proteinClass()))
                            ->load(numProteinsToLoad)
                            ->apply(proteinToInputOutputPairFunction())
                            ->randomize();
  ContainerPtr testingData;
  jassert(foldCrossValidation == 0 || testingProteinsDirectory == File::nonexistent);
  if (!foldCrossValidation)
  {
    if (testingProteinsDirectory == File::nonexistent)
    {
      testingData = trainingData->fold(0, numFolds);
      trainingData = trainingData->invFold(0, numFolds);
    }
    else
    {
      testingData = directoryFileStream(testingProteinsDirectory, T("*.xml"))
                  ->apply(loadFromFileFunction(proteinClass()))
                  ->load()
                  ->apply(proteinToInputOutputPairFunction());
    }
  }

  std::cout << trainingData->getNumElements() << " Training Proteins & "
            << testingData->getNumElements()  << " Testing Proteins" << std::endl;

  if (trainingData->isEmpty() || testingData->isEmpty())
  {
    std::cout << "The training set or the testing set is empty." << std::endl;
    return 0;
  }

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
  if (generateIntermediate)
    inference->setProteinDebugDirectory(File::getCurrentWorkingDirectory().getChildFile(output));

  for (size_t i = 0; i < targets.size(); ++i)
  {
    if (targets[i].contains(T("SS3")))
      inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
    if (targets[i].contains(T("SS8")))
      inference->appendInference(factory->createInferenceStep(T("dsspSecondaryStructure")));
    if (targets[i].contains(T("SA")))
      inference->appendInference(factory->createInferenceStep(T("solventAccessibilityAt20p")));
    if (targets[i].contains(T("DR")))
      inference->appendInference(factory->createInferenceStep(T("disorderRegions")));
    if (targets[i].contains(T("StAl")))
      inference->appendInference(factory->createInferenceStep(T("structuralAlphabetSequence")));
  }

  /*
  ** Setting Callbacks
  */
  InferenceContextPtr context = singleThreadedInferenceContext();

  const String classNameToEvaluate = DefaultParameters::saveIterations ? T("RunOnSupervisedExamplesInference") : T("RunSequentialInferenceStepOnExamples");
  const String classNameWhichContainTargetName = DefaultParameters::saveIterations ? T("OneAgainstAllClassificationInference") : T("ProteinInferenceStep");

  WrapperInferenceCallbackPtr callbacks = new WrapperInferenceCallback(inference, trainingData, testingData, classNameWhichContainTargetName, classNameToEvaluate);
  callbacks->appendCallback(new StandardOutputInferenceCallback(classNameToEvaluate));
  callbacks->appendCallback(new GnuPlotInferenceCallback(File::getCurrentWorkingDirectory().getChildFile(output), classNameToEvaluate));

  context->appendCallback(callbacks);

  /*
  ** Run
  */
  context->train(inference, trainingData);
  return 0;
}
