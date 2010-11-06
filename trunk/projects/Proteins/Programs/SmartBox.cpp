
#include <lbcpp/lbcpp.h>
#include "Programs/ProgramDeclarations.h"
#include "Inference/ProteinInference.h"
#include "Evaluator/ProteinEvaluator.h"

using namespace lbcpp;

/********** ProteinInferenceFactory **********/

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
  NumericalProteinInferenceFactory() : maxIterations(0) {} 
  
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
  
  const NumericalLearningParameterPtr getParamerters(const String& targetName, bool contentOnly) const
  {
    if (!parameters.count(targetName))
    {
      jassert(defaultParameter);
      std::cout << "Default learning parameter used for " << targetName.quoted()
                << " on " << ((contentOnly) ? "Content-Only" : "MultiPass") << " context" << std::endl;
      return defaultParameter;
    }

    NumericalLearningParameterPtr res = contentOnly ? parameters.find(targetName)->second.first : parameters.find(targetName)->second.second;
    if (!res)
    {
      jassert(defaultParameter);
      std::cout << "Default learning parameter used for " << targetName.quoted()
                << " on " << ((contentOnly) ? "Content-Only" : "MultiPass") << " context" << std::endl;
      return defaultParameter;
    }
    return res;
  }
  
  InferenceOnlineLearnerPtr createOnlineLearner(const String& targetName) const
  {
    jassert(!targetName.startsWith(T("contactMap"))); // Not yet implemented
    NumericalLearningParameterPtr parameter = getParamerters(targetName, *currentPassPointer == 0);
    
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
    return oneAgainstAllClassificationInference(targetName, classes, binaryClassifier);
  }
};

class MultiClassLinearSVMProteinInferenceFactory : public NumericalProteinInferenceFactory
{
public:
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

class StackPrinterCallback : public InferenceCallback
{
public:
  virtual void preInferenceCallback(InferenceContextWeakPtr context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    ScopedLock _(lock);
    const InferencePtr& currentInference = stack->getCurrentInference();
    if (stack->getDepth() > 3
        || currentInference->getClassName() == T("BinaryLinearSVMInference")
        || currentInference->getClassName() == T("OneAgainstAllClassificationInference"))
      return;
    String line(T("#"));
    for (size_t i = 1; i < stack->getDepth(); ++i)
      line += T("    ");
    line += currentInference->getClassName() + T(" -> ") + currentInference->getName();
    MessageCallback::info(line);
  }
  
  virtual void postInferenceCallback(InferenceContextWeakPtr context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    return;
    ScopedLock _(lock);
    const InferencePtr& currentInference = stack->getCurrentInference();
    if (stack->getDepth() > 2
        || currentInference->getClassName() == T("BinaryLinearSVMInference")
        || currentInference->getClassName() == T("OneAgainstAllClassificationInference"))
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

class MyInferenceCallback : public InferenceCallback
{
public:
  MyInferenceCallback(InferencePtr inference, ContainerPtr trainingData, ContainerPtr testingData, ContainerPtr validationData, ProteinTargetPtr target, File output)
    : inference(inference), trainingData(trainingData), testingData(testingData)
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
  
  virtual void preInferenceCallback(InferenceContextWeakPtr context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
    {
      // top-level learning is beginning
      startingTime = Time::getMillisecondCounter();
      passNumber = 0;
    }
    
    if (!input.isObject())
      return;

    InferenceBatchLearnerInputPtr learnerInput = input.dynamicCast<InferenceBatchLearnerInput>();
    if (learnerInput)
    {
      String inputTypeName = learnerInput->getTrainingExamples()->computeElementsCommonBaseType()->getTemplateArgument(0)->getName();
      
      String info = T("=== Learning ") + learnerInput->getTargetInference()->getName() + T(" with ");
      info += String((int)learnerInput->getNumTrainingExamples());
      if (learnerInput->getNumValidationExamples())
        info += T(" + ") + String((int)learnerInput->getNumValidationExamples());
      
      info += T(" ") + inputTypeName + T("(s) ===");
      MessageCallback::info(info);
    }
  }
  
  virtual void postInferenceCallback(InferenceContextWeakPtr context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    //String inferenceName = stack->getCurrentInference()->getName();

    if (stack->getDepth() == 2 && stack->getCurrentInference()->getClassName() == T("StaticParallelInferenceLearner"))
    {
      MessageCallback::info(T("===================== EVALUATION ====================="));

      ProteinEvaluatorPtr learningEvaluator = new ProteinEvaluator();
      ProteinEvaluatorPtr testingEvaluator = new ProteinEvaluator();
      ProteinEvaluatorPtr validationEvaluator = new ProteinEvaluator();
      
      MessageCallback::info(T("================== Train Evaluation ==================  ")
                            + String((Time::getMillisecondCounter() - startingTime) / 1000)
                            + T(" s"));
      context->evaluate(inference, trainingData, learningEvaluator);
      MessageCallback::info(learningEvaluator->toString());

      if (testingData && testingData->getNumElements())
      {
        MessageCallback::info(T("=================== Test Evaluation ==================  ")
                              + String((Time::getMillisecondCounter() - startingTime) / 1000)
                              + T(" s"));
        context->evaluate(inference, testingData, testingEvaluator);
        MessageCallback::info(testingEvaluator->toString());
      }

      if (validationData && validationData->getNumElements())
      {
        MessageCallback::info(T("============== Validation Evaluation ===============  ")
                              + String((Time::getMillisecondCounter() - startingTime) / 1000)
                              + T(" s"));
        context->evaluate(inference, validationData, validationEvaluator);
        MessageCallback::info(validationEvaluator->toString());
      }

      MessageCallback::info(T("======================================================"));

      std::vector< std::pair<String, double> > learningScores, testingScores, validationScores;
      learningEvaluator->getScores(learningScores);
      testingEvaluator->getScores(testingScores);
      validationEvaluator->getScores(validationScores);
      
      std::map<String, size_t> scoreNameToIndex;
      for (size_t i = 0; i < learningScores.size(); ++i)
        scoreNameToIndex[learningScores[i].first] = i;
      
      std::map<String, String> scoresToUse;
      scoresToUse[T("secondaryStructure")] = T("secondaryStructure[Accuracy]");
      scoresToUse[T("dsspSecondaryStructure")] = T("dsspSecondaryStructure[Accuracy]");
      scoresToUse[T("disorderRegions")] = T("disorderRegions[MCC]");
      scoresToUse[T("solventAccessibilityAt20p")] = T("solventAccessibilityAt20p[Accuracy]");
      scoresToUse[T("structuralAlphabetSequence")] = T("structuralAlphabetSequence[Accuracy]");
      
      for (std::map<String, File>::iterator it = outputs.begin(); it != outputs.end(); ++it)
      {
        String targetName = it->first;
        OutputStream* o = it->second.createOutputStream();
        size_t index = scoreNameToIndex[scoresToUse[targetName]];
        *o << (int)passNumber << '\t'
           << learningScores[index].second << '\t'
           << testingScores[index].second << '\t'
           << validationScores[index].second << '\t'
           << String((int)(Time::getMillisecondCounter() - startingTime) / 1000) << '\n';
        delete o;
      }
      
      // Mouais ... -_-"
      /*InferenceBatchLearnerInputPtr learnerInput = input.dynamicCast<InferenceBatchLearnerInput>();
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
          MessageCallback::info(info);
        }
      }*/

      MessageCallback::info(T("======================================================"));
      ++passNumber;
    }

    if (stack->getDepth() == 1)
    {
      MessageCallback::info(T("Bye: ") + String((Time::getMillisecondCounter() - startingTime) / 1000.0) + T(" seconds"));
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

bool NumericalLearningParameter::loadFromString(const String& value, MessageCallback& callback)
{
  StringArray values;
  values.addTokens(value, T(";"), T(""));
  if (values.size() != 3)
  {
    callback.errorMessage(T("NumericalLearningParameter::loadFromString"), value.quoted() + T(" is not valid"));
    return false;
  }
  
  Variable vLR  = Variable::createFromString(doubleType, values[0], callback);
  Variable vLRD = Variable::createFromString(doubleType, values[1], callback);
  Variable vR   = Variable::createFromString(doubleType, values[2], callback);
  
  if (vLR.isMissingValue() || vLRD.isMissingValue() || vR.isMissingValue())
  {
    callback.errorMessage(T("NumericalLearningParameter::loadFromString"), value.quoted() + T(" is not valid"));
    return false;
  }
  
  learningRate = vLR.getDouble();
  learningRateDecrease = vLRD.getDouble();
  regularizer = vR.getDouble();

  return true;
}

bool ProteinTarget::loadFromString(const String& value, MessageCallback& callback)
{
  tasks.clear();

  for (int begin = 0; begin != -1 && begin < value.length(); )
  {
    int end = value.indexOfChar(begin, T(')'));
    if (value[begin] != T('(') || end == -1)
    {
      callback.errorMessage(T("ProteinTarget::loadFromString"), value.quoted() + T(" is not a valid target expression"));
      return false;
    }
    
    StringArray taskValues;
    taskValues.addTokens(value.substring(begin + 1, end), T("-"), T(""));
    
    begin = value.indexOfChar(end, T('('));
    if (begin == -1)
      begin = value.length();
    
    Variable nbPass = Variable::createFromString(positiveIntegerType, value.substring(++end, begin), callback);
    if (nbPass.isMissingValue())
    {
      callback.errorMessage(T("ProteinTarget::loadFromString"), value.quoted() + T(" is not a valid target expression"));
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
          callback.errorMessage(T("ProteinTarget::loadFromString"), taskValues[i].quoted() + T(" is not a valid task"));
          return false;
        }
      }
      
      tasks.push_back(pass);
    }
  }

  return true;
}

ProteinInferenceFactoryPtr SnowBox::createFactory() const
{
  if (baseLearner == T("ExtraTree"))
    return new ExtraTreeProteinInferenceFactory();

  NumericalProteinInferenceFactoryPtr res;
  if (baseLearner == T("OneAgainstAllLinearSVM"))
    res = new OneAgainstAllLinearSVMProteinInferenceFactory();
  if (baseLearner == T("MultiClassLinearSVM"))
    res = new MultiClassLinearSVMProteinInferenceFactory();
  
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

ContainerPtr SnowBox::loadProteins(const File& f, size_t maxToLoad) const
{
  static ThreadPoolPtr pool = new ThreadPool(numberOfThreads, false);
  return directoryFileStream(f, T("*.xml"))
    ->load(maxToLoad)
    ->apply(loadFromFileFunction(proteinClass), pool)
    ->apply(proteinToInputOutputPairFunction(true))
    ->randomize();
}

bool SnowBox::loadData(MessageCallback& callback)
{
  /* learning data */
  if (learningDirectory == File::nonexistent)
  {
    callback.errorMessage(T("SnowBox::loadData"), T("No learning directory specified"));
    return false;
  }
  
  learningData = loadProteins(learningDirectory, maxProteinsToLoad);
  
  if (!learningData->getNumElements())
  {
    callback.errorMessage(T("SnowBox::loadData"), T("No data found in ") + learningDirectory.getFullPathName().quoted());
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
    validationData = loadProteins(validationDirectory);
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
    testingData = loadProteins(testingDirectory);
  }
  
  if (!useCrossValidation && !testingData->getNumElements())
  {
    callback.errorMessage(T("SnowBox::loadData"), T("No testing data found"));
    return false;
  }
  return true;
}

void SnowBox::printInformation() const
{
  std::cout << "* -------------- Directories -------------- *" << std::endl;
  std::cout << "learningDirectory   : " << learningDirectory.getFullPathName() << std::endl;
  if (testingDirectory != File::nonexistent)
    std::cout << "testingDirectory    : " << testingDirectory.getFullPathName() << std::endl;
  if (validationDirectory != File::nonexistent)
    std::cout << "validationDirectory : " << validationDirectory.getFullPathName() << std::endl;
  std::cout << "output              : " << output.getFullPathName() << std::endl;
  
  std::cout << "* ----------------- Data ------------------ *" << std::endl;
  std::cout << "Learning proteins   : " << learningData->getNumElements() << std::endl;
  std::cout << "Testing proteins    : " << ((testingData) ? testingData->getNumElements() : 0);
  if (testingDirectory == File::nonexistent && !useCrossValidation)
    std::cout << " (" << (currentFold+1) << "/" << numberOfFolds << ")";
  std::cout << std::endl;
  std::cout << "Validation proteins : " << ((validationData) ? validationData->getNumElements() : 0);
  if (validationDirectory == File::nonexistent && partAsValidation)
    std::cout << " (1/" << partAsValidation << ")";
  std::cout << std::endl;
  
  std::cout << "* ---------------- Method ----------------- *" << std::endl;
  std::cout << "baseLearner         : " << baseLearner << std::endl;
  std::cout << "Validation protocol : ";
  if (useCrossValidation)
    std::cout << numberOfFolds << "-fold cross validation";
  else if (testingDirectory != File::nonexistent)
    std::cout << "Independent testing set";
  else
    std::cout << "Separated part of training set (fold " << (currentFold+1) << " over " << numberOfFolds << ")";
  std::cout << std::endl;
  
  std::cout << "* ------------------ Model ---------------- *" << std::endl;
  for (size_t i = 0; i < target->getNumPasses(); ++i)
  {
    std::cout << "Pass " << i << std::endl;
    for (size_t j = 0; j < target->getNumTasks(i); ++j)
      std::cout << "|-> " << target->getTask(i, j) << std::endl;
  }
  std::cout << "* ----------------------------------------- *" << std::endl;
  std::cout << std::endl;
}

int SnowBox::runProgram(MessageCallback& callback)
{
  if (!loadData(callback))
  {
    callback.errorMessage(T("SnowBox::runProgram"), T("Loading data failed"));
    return -1;
  }

  ProteinInferenceFactoryPtr factory = createFactory();
  if (!factory)
  {
    callback.errorMessage(T("SnowBox::runProgram"), T("Unknown base learned and/or multiclass learner !"));
    return -1;
  }

  printInformation();

  ProteinSequentialInferencePtr inference = new ProteinSequentialInference();
  InferencePtr previousInference;
  for (currentPass = 0; currentPass < target->getNumPasses(); ++currentPass)
  {
    ProteinParallelInferencePtr inferencePass = new ProteinParallelInference("Passsssssss");
    for (size_t j = 0; j < target->getNumTasks(currentPass); ++j)
      inferencePass->appendInference(factory->createInferenceStep(target->getTask(currentPass, j)));

    if (previousInference)
      initializeLearnerByCloning(inferencePass, previousInference);

    inference->appendInference(inferencePass);
    //previousInference = inferencePass;
  }
  
  InferenceContextPtr context = (numberOfThreads == 1)
                              ? singleThreadedInferenceContext()
                              : multiThreadedInferenceContext(new ThreadPool(numberOfThreads, false));
  
  if (useCrossValidation)
  {
    //context->crossValidate(inference, learningData, evaluator, numberOfFolds);
    callback.errorMessage(T("SnowBox::runProgram"), T("CrossValidation not yet implemented (missing appropriate evaluator)"));
    return -1;
  }
  else
  {
    context->appendCallback(new MyInferenceCallback(inference, learningData, testingData, validationData, target, output));
    //context->appendCallback(new StackPrinterCallback());
    context->train(inference, learningData, validationData);

    File outputInferenceFile = output.getFullPathName() + T(".xml");
    inference->saveToFile(outputInferenceFile);
    std::cout << "Save inference : " << outputInferenceFile.getFullPathName() << std::endl;
  }
  
  return 0;
}
