
#include <lbcpp/lbcpp.h>
#include "SnowBox.h"
#include "Inference/ProteinInference.h"
#include "Evaluator/ProteinEvaluator.h"

using namespace lbcpp;

/********** ProteinInferenceFactory **********/

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

class NumericalProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  NumericalProteinInferenceFactory(ExecutionContext& context)
    : ProteinInferenceFactory(context), maxIterations(0) {} 
  
  virtual PerceptionPtr createPerception(const String& targetName, PerceptionType type) const
  {
    PerceptionPtr res = ProteinInferenceFactory::createPerception(targetName, type);
    return res ? flattenPerception(res) : PerceptionPtr();
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

  virtual void setParameters(const std::vector<std::pair<String, std::pair<NumericalLearningParameterPtr, NumericalLearningParameterPtr> > >& parameters)
  {
    for (size_t i = 0; i < parameters.size(); ++i)
      this->parameters[parameters[i].first] = parameters[i].second;
  }
  
  virtual void setDefaultParameter(const NumericalLearningParameterPtr parameter)
    {defaultParameter = parameter;}

  virtual void setCurrentPassPointer(const size_t* currentPass)
    {currentPassPointer = currentPass;}
  
  virtual void setMaximumIterations(size_t maxIterations)
    {this->maxIterations = maxIterations;}

protected:  
  void addBiasInferenceIfNeeded(NumericalSupervisedInferencePtr inference, const String& targetName) const
  {
    if (targetName.startsWith(T("contactMap"))
        || targetName == T("disorderRegions")
        || targetName == T("solventAccessibilityAt20p")
        || targetName == T("disulfideBonds"))
    {
      VectorSequentialInferencePtr sequentialInference = new VectorSequentialInference(targetName);
      sequentialInference->appendInference(inference->getSubInference());
      sequentialInference->appendInference(addBiasInference(targetName + T(" bias")));
      inference->setSubInference(sequentialInference);
    }
  }
  
  const NumericalLearningParameterPtr getParameters(const String& targetName, bool contentOnly) const
  {
    if (!parameters.count(targetName))
    {
      jassert(defaultParameter);
      context.informationCallback(T("Default learning parameter used for ") + targetName.quoted()
                + T(" on ") + ((contentOnly) ? T("Content-Only") : T("MultiPass")) + T(" context"));
      return defaultParameter;
    }

    NumericalLearningParameterPtr res = contentOnly ? parameters.find(targetName)->second.first : parameters.find(targetName)->second.second;
    if (!res)
    {
      jassert(defaultParameter);
      context.informationCallback(T("Default learning parameter used for ") + targetName.quoted()
                + T(" on ") + ((contentOnly) ? T("Content-Only") : T("MultiPass")) + T(" context"));
      return defaultParameter;
    }
    return res;
  }
  
  InferenceOnlineLearnerPtr createOnlineLearner(const String& targetName) const
  {
    jassert(!targetName.startsWith(T("contactMap"))); // Not yet implemented
    NumericalLearningParameterPtr parameter = getParameters(targetName, *currentPassPointer == 0);
    
    InferenceOnlineLearnerPtr res, lastLearner;
    /* randomizer */

    /* gradient */
    res = lastLearner = gradientDescentOnlineLearner(perStep, invLinearIterationFunction(parameter->getLearningRate(), parameter->getLearningRateDecrease()), true,
                                                     perStepMiniBatch20, l2RegularizerFunction(parameter->getRegularizer()));
    /* evaluator */
    //if (targetName == T("secondaryStructure"))
    //  lastLearner = lastLearner->setNextLearner(computeEvaluatorOnlineLearner(classificationAccuracyEvaluator(T("SS3")), false));
    //else if (targetName == T("disorderRegions"))
    //  lastLearner = lastLearner->setNextLearner(computeEvaluatorOnlineLearner(binaryClassificationConfusionEvaluator(T("DR")), false));
    
    
    /*
    EvaluatorPtr evaluator = new ProteinEvaluator();
    evaluator->setName(T("trainScore"));
    lastLearner = lastLearner->setNextLearner(computeEvaluatorOnlineLearner(evaluator, false));
    
    evaluator = new ProteinEvaluator();
    evaluator->setName(T("validationScore"));
    lastLearner = lastLearner->setNextLearner(computeEvaluatorOnlineLearner(evaluator, true));
     */
    /* stopping criterion */
    StoppingCriterionPtr stoppingCriterion = logicalOr(maxIterationsStoppingCriterion(maxIterations), 
                                                       maxIterationsWithoutImprovementStoppingCriterion(2));
    lastLearner = lastLearner->setNextLearner(stoppingCriterionOnlineLearner(stoppingCriterion, true));
    
    return res;
  }

private: 
  std::map<String, std::pair<NumericalLearningParameterPtr, NumericalLearningParameterPtr> > parameters;

  NumericalLearningParameterPtr defaultParameter;
  
  const size_t* currentPassPointer;
  
  size_t maxIterations;
};

typedef ReferenceCountedObjectPtr<NumericalProteinInferenceFactory> NumericalProteinInferenceFactoryPtr;

class OneAgainstAllLinearSVMProteinInferenceFactory : public NumericalProteinInferenceFactory
{
public:
  OneAgainstAllLinearSVMProteinInferenceFactory(ExecutionContext& context)
    : NumericalProteinInferenceFactory(context) {}

  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
  {
    NumericalSupervisedInferencePtr res = binaryLinearSVMInference(targetName, perception);
    res->setStochasticLearner(createOnlineLearner(targetName));
    

    addBiasInferenceIfNeeded(res, targetName);
    return res;
  }
  
  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
  {
    InferencePtr binaryClassifier = createBinaryClassifier(targetName, perception);
    return oneAgainstAllClassificationInference(context, targetName, classes, binaryClassifier);
  }
};

class MultiClassLinearSVMProteinInferenceFactory : public NumericalProteinInferenceFactory
{
public:
  MultiClassLinearSVMProteinInferenceFactory(ExecutionContext& context)
    : NumericalProteinInferenceFactory(context) {}

  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
  {
    NumericalSupervisedInferencePtr res = binaryLinearSVMInference(targetName, perception);
    res->setStochasticLearner(createOnlineLearner(targetName));
    
    addBiasInferenceIfNeeded(res, targetName);
    return res;
  }
  
  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
  {
    NumericalSupervisedInferencePtr res = multiClassLinearSVMInference(targetName, perception, classes, (targetName == T("structuralAlphabet")));
    res->setStochasticLearner(createOnlineLearner(targetName));
    return res;
  }
};

/********** InferenceClallback **********/

class StackPrinterCallback : public ExecutionCallback
{
public:
  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit)
  {
    for (size_t i = 0; i < stack->getDepth(); ++i)
      std::cout << "  ";
    std::cout << workUnit->getClassName() << std::endl;
  }
  
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit, bool result)
  {}
};

class EvaluationInferenceCallback : public ExecutionCallback
{
public:
  EvaluationInferenceCallback(InferencePtr inference, ContainerPtr trainingData, ContainerPtr testingData, ContainerPtr validationData, ProteinTargetPtr target, File output)
    : inference(inference), trainingData(trainingData), testingData(testingData), passNumber(0), startingTime(Time::getMillisecondCounter())
  {
    for (size_t i = 0; i < target->getNumPasses(); ++i)
    {
      for (size_t j = 0; j < target->getNumTasks(i); ++j)
      {
        if (outputs.count(target->getTask(i, j)))
          continue;
        File f = output.getFullPathName() + T(".") + target->getTask(i, j);
        if (f.exists())
          f.deleteFile();
        outputs[target->getTask(i, j)] = f;
      }
    }
  }
  
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit, bool result)
  {
    ExecutionContext& context = getContext();

    if (stack->getDepth() == 1 && workUnit->getName().startsWith(T("Learning Passsssssss")))
    {
      InferenceWorkUnitPtr inferenceWorkUnit = workUnit.dynamicCast<InferenceWorkUnit>();
      if (!inferenceWorkUnit || !inferenceWorkUnit->getInput().isObject())
      {
        context.errorCallback(T("EvaluationInferenceCallback::preExecutionCallback"), T("Callback is incorrectly calibrate"));
        return;
      }

      ProteinEvaluatorPtr learningEvaluator = new ProteinEvaluator();
      ProteinEvaluatorPtr testingEvaluator = new ProteinEvaluator();
      ProteinEvaluatorPtr validationEvaluator = new ProteinEvaluator();

      inference->evaluate(context, trainingData, learningEvaluator, T("Evaluating on training data"));

      if (testingData && testingData->getNumElements())
        inference->evaluate(context, testingData, testingEvaluator, T("Evaluating on testing data"));

      if (validationData && validationData->getNumElements())
        inference->evaluate(context, validationData, validationEvaluator, T("Evaluating on validation data"));

      /* GnuPlot File Generation */
      for (std::map<String, File>::iterator it = outputs.begin(); it != outputs.end(); ++it)
      {
        std::vector< std::pair<String, double> > learningScores, testingScores, validationScores;
        learningEvaluator->getScoresForTarget(context, it->first, learningScores);
        testingEvaluator->getScoresForTarget(context, it->first, testingScores);
        validationEvaluator->getScoresForTarget(context, it->first, validationScores);
        jassert(learningScores.size() == testingScores.size() && learningScores.size() == validationScores.size());
        // Header
        OutputStream* o = it->second.createOutputStream();
        if (!passNumber)
        {
          *o << "# GnuPlot File Generated on " << Time::getCurrentTime().toString(true, true, true, true) << "\n";
          *o << "# pass";
          for (size_t i = 0; i < learningScores.size(); ++i)
            *o << "\t" << learningScores[i].first << "(train, test, valid)";
          *o << "\n";
        }        

        *o << (int)passNumber;
        for (size_t i = 0; i < learningScores.size(); ++i)
        {
          *o << "\t" << learningScores[i].second
             << "\t" << testingScores[i].second
             << "\t" << validationScores[i].second;
        }
        *o << "\t" << String((Time::getMillisecondCounter() - startingTime) / 1000) << "\n";
        delete o;
      }

      ++passNumber;
    }

    if (stack->getDepth() == 0)
    {
      context.informationCallback(T("Bye: ") + String((Time::getMillisecondCounter() - startingTime) / 1000.0) + T(" seconds"));
    }
  }

private:
  InferencePtr inference;
  ContainerPtr trainingData, testingData, validationData;
  size_t passNumber;
  juce::uint32 startingTime;
  std::map<String, File> outputs;
};

void initializeLearnerByCloning(InferencePtr inference, InferencePtr inferenceToClone)
{
  inference->setBatchLearner(multiPassInferenceLearner(initializeByCloningInferenceLearner(inferenceToClone), inference->getBatchLearner()));
}

bool NumericalLearningParameter::loadFromString(ExecutionContext& context, const String& value)
{
  StringArray values;
  values.addTokens(value, T(";"), T(""));
  if (values.size() != 3)
  {
    context.errorCallback(T("NumericalLearningParameter::loadFromString"), value.quoted() + T(" is not valid"));
    return false;
  }
  
  Variable vLR  = Variable::createFromString(context, doubleType, values[0]);
  Variable vLRD = Variable::createFromString(context, doubleType, values[1]);
  Variable vR   = Variable::createFromString(context, doubleType, values[2]);
  
  if (vLR.isMissingValue() || vLRD.isMissingValue() || vR.isMissingValue())
  {
    context.errorCallback(T("NumericalLearningParameter::loadFromString"), value.quoted() + T(" is not valid"));
    return false;
  }
  
  learningRate = vLR.getDouble();
  learningRateDecrease = vLRD.getDouble();
  regularizer = vR.getDouble();
  return true;
}

bool ProteinTarget::loadFromString(ExecutionContext& context, const String& value)
{
  tasks.clear();

  for (int begin = 0; begin != -1 && begin < value.length(); )
  {
    int end = value.indexOfChar(begin, T(')'));
    if (value[begin] != T('(') || end == -1)
    {
      context.errorCallback(T("ProteinTarget::loadFromString"), value.quoted() + T(" is not a valid target expression"));
      return false;
    }
    
    StringArray taskValues;
    taskValues.addTokens(value.substring(begin + 1, end), T("-"), T(""));
    
    begin = value.indexOfChar(end, T('('));
    if (begin == -1)
      begin = value.length();
    
    Variable nbPass = Variable::createFromString(context, positiveIntegerType, value.substring(++end, begin));
    if (nbPass.isMissingValue())
    {
      context.errorCallback(T("ProteinTarget::loadFromString"), value.quoted() + T(" is not a valid target expression"));
      return false;
    }
    
    for (size_t n = 0; n < (size_t)nbPass.getInteger(); ++n)
    {
      std::vector<String> pass;
      for (size_t i = 0; i < (size_t)taskValues.size(); ++i)
      {
        if (taskValues[i] == T("SS3"))
          pass.push_back(T("secondaryStructure"));
        else if (taskValues[i] == T("SS8"))
          pass.push_back(T("dsspSecondaryStructure"));
        else if (taskValues[i] == T("SA"))
          pass.push_back(T("solventAccessibilityAt20p"));
        else if (taskValues[i] == T("DR"))
          pass.push_back(T("disorderRegions"));
        else if (taskValues[i] == T("StAl"))
          pass.push_back(T("structuralAlphabetSequence"));
        else
        {
          context.errorCallback(T("ProteinTarget::loadFromString"), taskValues[i].quoted() + T(" is not a valid task"));
          return false;
        }
      }
      
      tasks.push_back(pass);
    }
  }
  return true;
}

ProteinInferenceFactoryPtr SnowBox::createFactory(ExecutionContext& context) const
{
  if (baseLearner == T("ExtraTree"))
    return new ExtraTreeProteinInferenceFactory(context);

  NumericalProteinInferenceFactoryPtr res;
  if (baseLearner == T("OneAgainstAllLinearSVM"))
    res = new OneAgainstAllLinearSVMProteinInferenceFactory(context);
  if (baseLearner == T("MultiClassLinearSVM"))
    res = new MultiClassLinearSVMProteinInferenceFactory(context);
  
  if (res)
  {
    res->setParameters(learningParameters);
    res->setDefaultParameter(defaultParameter);
    res->setCurrentPassPointer(&currentPass);
    res->setMaximumIterations(maxIterations);
    return res;
  }

  return ProteinInferenceFactoryPtr();
}

ContainerPtr SnowBox::loadProteins(ExecutionContext& context, const File& f, size_t maxToLoad) const
{
  if (inputDirectory != File::nonexistent)
    return directoryPairFileStream(inputDirectory, f, T("*.xml"))
      ->load(context, maxToLoad)
      ->apply(context, loadFromFilePairFunction(proteinClass, proteinClass), Container::parallelApply)
      ->randomize();
  
  return directoryFileStream(f, T("*.xml"))
    ->load(context, maxToLoad)
    ->apply(context, loadFromFileFunction(proteinClass), Container::parallelApply)
    ->apply(context, proteinToInputOutputPairFunction(false), Container::parallelApply)
    ->randomize();
}

bool SnowBox::loadData(ExecutionContext& context)
{
  /* learning data */
  if (learningDirectory == File::nonexistent)
  {
    context.errorCallback(T("SnowBox::loadData"), T("No learning directory specified"));
    return false;
  }
  
  learningData = loadProteins(context, learningDirectory, maxProteinsToLoad);
  
  if (!learningData->getNumElements())
  {
    context.errorCallback(T("SnowBox::loadData"), T("No data found in ") + learningDirectory.getFullPathName().quoted());
    return false;
  }
  /* validation data */
  if (validationDirectory == File::nonexistent)
  {
    if (partAsValidation)
    {
      jassert(partAsValidation > 1);
      validationData = learningData->fold(0, partAsValidation);
      learningData = learningData->invFold(0, partAsValidation);
    }
  }
  else
  {
    validationData = loadProteins(context, validationDirectory);
  }
  /* testing data */
  if (testingDirectory == File::nonexistent)
  {
    if (!useCrossValidation)
    {
      testingData = learningData->fold(currentFold, numberOfFolds);
      learningData = learningData->invFold(currentFold, numberOfFolds);
    }
  }
  else
  {
    testingData = loadProteins(context, testingDirectory);
  }
  
  if (!useCrossValidation && !testingData->getNumElements())
  {
    context.errorCallback(T("SnowBox::loadData"), T("No testing data found"));
    return false;
  }
  return true;
}

void SnowBox::printInformation(ExecutionContext& context) const
{
  String txt;

  context.informationCallback(T("SnowBox::Directories"), T("Learning Directory : ") + learningDirectory.getFullPathName());
  if (testingDirectory != File::nonexistent)
    context.informationCallback(T("SnowBox::Directories"), T("Testing Directory : ") + testingDirectory.getFullPathName());
  if (validationDirectory != File::nonexistent)
    context.informationCallback(T("SnowBox::Directories"), T("Validation Directory : ") + validationDirectory.getFullPathName());
  context.informationCallback(T("SnowBox::Directories"), T("Output : ") + output.getFullPathName());
  
  context.informationCallback(T("SnowBox::Data"), T("Learning Proteins : ") + String((int)learningData->getNumElements()));
  txt = String(testingData ? (int)testingData->getNumElements() : 0);
  if (testingDirectory == File::nonexistent && !useCrossValidation)
    txt += T(" (") + String((int)currentFold + 1) + T("/") + String((int)numberOfFolds) + T(")");
  context.informationCallback(T("SnowBox::Data"), T("Testing Proteins : ") + txt);

  txt = String(validationData ? (int)validationData->getNumElements() : 0);
  if (validationDirectory == File::nonexistent && partAsValidation)
    txt = T(" (1/") + String((int)partAsValidation) + T(")");
  context.informationCallback(T("SnowBox::Data"), T("Validation Proteins : ") + txt);

  context.informationCallback(T("SnowBox::Method"), T("Base Learner : ") + baseLearner);

  if (useCrossValidation)
    txt = String((int)numberOfFolds) + T("-fold cross validation");
  else if (testingDirectory != File::nonexistent)
    txt = "Independent testing set";
  else
    txt = T("Separated part of training set (fold ") + String((int)currentFold + 1) + T(" over ") + String((int)numberOfFolds) + T(")");
  context.informationCallback(T("SnowBox::Method"), T("Validation Protocol : ") + txt);

  if (target)
  {
    for (size_t i = 0; i < target->getNumPasses(); ++i)
      for (size_t j = 0; j < target->getNumTasks(i); ++j)
        context.informationCallback(T("SnowBox::Model"), T("Pass ") + String((int)i) + (" - Task : ") + target->getTask(i, j));
  }
}

ProteinSequentialInferencePtr SnowBox::loadOrCreateIfFailInference(ExecutionContext& context) const
{
  if (inferenceFile != File::nonexistent)
  {
    ObjectPtr obj = Object::createFromFile(context, inferenceFile);
    if (obj && obj->getClass()->inheritsFrom(lbcpp::proteinSequentialInferenceClass))
      return obj.staticCast<ProteinSequentialInference>();
  }
  return new ProteinSequentialInference();
}

bool SnowBox::run(ExecutionContext& context)
{
  if (!loadData(context))
  {
    context.errorCallback(T("SnowBox::run"), T("Loading data failed"));
    return false;
  }

  ProteinInferenceFactoryPtr factory = createFactory(context);
  if (!factory)
  {
    context.errorCallback(T("SnowBox::run"), T("Unknown base learned and/or multiclass learner !"));
    return false;
  }

  printInformation(context);

  ProteinSequentialInferencePtr inference = loadOrCreateIfFailInference(context);
  InferencePtr previousInference;
  for (currentPass = 0; currentPass < target->getNumPasses(); ++currentPass)
  {
    ProteinParallelInferencePtr inferencePass = new ProteinParallelInference("Passsssssss");
    for (size_t j = 0; j < target->getNumTasks(currentPass); ++j)
      inferencePass->appendInference(factory->createInferenceStep(target->getTask(currentPass, j)));

    if (previousInference)
      initializeLearnerByCloning(inferencePass, previousInference);

    inference->appendInference(inferencePass);
    previousInference = inferencePass;
  }
  /*
  ExecutionContextPtr inferenceContext = (numberOfThreads == 1)
                              ? singleThreadedExecutionContext()
                              : multiThreadedExecutionContext(numberOfThreads);
  // ExecutionContextPtr inferenceContext = refCountedPointerFromThis(&context); // FIXME !
  inferenceContext->appendCallback(consoleExecutionCallback());
  */
  if (useCrossValidation)
  {
    //inference->crossValidate(*context, learningData, evaluator, numberOfFolds);
    context.errorCallback(T("SnowBox::run"), T("CrossValidation not yet implemented (missing appropriate evaluator)"));
    return false;
  }
  else
  {
    context.appendCallback(new EvaluationInferenceCallback(inference, learningData, testingData, validationData, target, output));
    //context.appendCallback(new StackPrinterCallback());
    inference->train(context, learningData, validationData);

    File outputInferenceFile = output.getFullPathName() + T(".xml");
    context.informationCallback(T("SnowBox::InferenceSavedTo"), outputInferenceFile.getFullPathName());
    inference->saveToFile(context, outputInferenceFile);
  }
  return true;
}
