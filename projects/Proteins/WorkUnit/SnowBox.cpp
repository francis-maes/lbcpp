
#include "SnowBox.h"
#include "Evaluator/ProteinEvaluator.h"

using namespace lbcpp;

/********** ProteinInferenceFactory **********/


/********** InferenceClallback **********/

class StackPrinterCallback : public ExecutionCallback
{
public:
  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit)
  {
    for (size_t i = 0; i < stack->getDepth(); ++i)
      std::cout << "  ";
    std::cout << description << std::endl;
  }
  
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit, const Variable& result)
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
  
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit, const Variable& result)
  {
    ExecutionContext& context = getContext();

    if (stack->getDepth() == 1 && description.startsWith(T("Learning Stage")))
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

ProteinInferenceFactoryPtr SnowBox::createFactory(ExecutionContext& context) const
{
  if (baseLearner == T("ExtraTree"))
    return new ExtraTreeProteinInferenceFactory(context, new ExtraTreeLearningParameter(numTrees, numAttributesPerSplit, numForSplitting));

  NumericalProteinInferenceFactoryPtr res;
  if (baseLearner == T("OneAgainstAllLinearSVM"))
    res = new OneAgainstAllLinearSVMProteinInferenceFactory(context);
  else if (baseLearner == T("MultiClassLinearSVM"))
    res = new MultiClassLinearSVMProteinInferenceFactory(context);
  else if (baseLearner == T("MultiClassLinearSVMMostViolated"))
    res = new MultiClassLinearSVMProteinInferenceFactory(context, true);
  if (res)
  {
    //res->setParameters(learningParameters); // FIXME: !
    res->setDefaultParameter(defaultParameter);
    res->setMaximumIterations(maxIterations);
    return res;
  }
  jassertfalse;
  return ProteinInferenceFactoryPtr();
}

bool SnowBox::loadData(ExecutionContext& context)
{
  /* learning data */
  if (learningDirectory == File::nonexistent)
  {
    context.errorCallback(T("SnowBox::loadData"), T("No learning directory specified"));
    return false;
  }
  
  learningData = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, learningDirectory, maxProteinsToLoad, T("Loading training proteins"));
  
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
    validationData = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, validationDirectory, maxProteinsToLoad, T("Loading validation proteins"));
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
    testingData = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, testingDirectory, maxProteinsToLoad, T("Loading testing proteins"));
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

  context.resultCallback(T("Learning Directory"), learningDirectory.getFullPathName());
  if (testingDirectory != File::nonexistent)
    context.resultCallback(T("Testing Directory"), testingDirectory.getFullPathName());
  if (validationDirectory != File::nonexistent)
    context.resultCallback(T("Validation Directory"), validationDirectory.getFullPathName());
  context.resultCallback(T("Output"), output.getFullPathName());
  
  context.resultCallback(T("Learning Proteins"), String((int)learningData->getNumElements()));
  txt = String(testingData ? (int)testingData->getNumElements() : 0);
  if (testingDirectory == File::nonexistent && !useCrossValidation)
    txt += T(" (") + String((int)currentFold + 1) + T("/") + String((int)numberOfFolds) + T(")");
  context.resultCallback(T("Testing Proteins"), txt);

  txt = String(validationData ? (int)validationData->getNumElements() : 0);
  if (validationDirectory == File::nonexistent && partAsValidation)
    txt = T(" (1/") + String((int)partAsValidation) + T(")");
  context.resultCallback(T("Validation Protein"), txt);

  context.resultCallback(T("Base Learner"), baseLearner);

  if (useCrossValidation)
    txt = String((int)numberOfFolds) + T("-fold cross validation");
  else if (testingDirectory != File::nonexistent)
    txt = "Independent testing set";
  else
    txt = T("Separated part of training set (fold ") + String((int)currentFold + 1) + T(" over ") + String((int)numberOfFolds) + T(")");
  context.resultCallback(T("Validation Protocol"), txt);

  if (target)
  {
    for (size_t i = 0; i < target->getNumPasses(); ++i)
      for (size_t j = 0; j < target->getNumTasks(i); ++j)
        context.resultCallback(T("Pass ") + String((int)i) + T(" - Task ") + String((int)j), target->getTask(i, j));
  }
}

ProteinSequentialInferencePtr SnowBox::loadOrCreateIfFailInference(ExecutionContext& context, ParameteredProteinInferenceFactoryPtr factory) const
{
  ProteinSequentialInferencePtr inference;
  if (inferenceFile != File::nonexistent)
  {
    ObjectPtr obj = Object::createFromFile(context, inferenceFile);
    if (obj && obj->getClass()->inheritsFrom(lbcpp::proteinSequentialInferenceClass))
      inference = obj.staticCast<ProteinSequentialInference>();
  }
  
  if (!inference)
    return factory->createInference(target);

  inference->appendInference(factory->createInference(target));
  return inference;
}

static void exportPerceptionsToFile(ExecutionContext& context, ContainerPtr data, PerceptionPtr perception, File output)
{
  if (output.exists())
    output.deleteFile();
  OutputStream* o = output.createOutputStream();
  for (size_t i = 0; i < data->getNumElements(); ++i)
  {
    ProteinPtr protein = data->getElement(i)[0].getObjectAndCast<Protein>();
    for (size_t j = 0; j < protein->getLength(); ++j)
    {
      ObjectPtr obj = perception->computeFunction(context, Variable::pair(protein, j)).getObject();
      for (size_t k = 0; k < obj->getNumVariables(); ++k)
      {
        Variable v = obj->getVariable(k);
        if (v.isDouble())
          *o << v.getDouble();
        else if (v.isInteger())
          *o << v.getInteger();
        else if (v.isEnumeration())
          *o << v.getInteger();
        else if (v.isBoolean())
          *o << (v.getBoolean() ? 1 : 0);
        else
        {
          jassertfalse;
        }
        *o << " ";
      }
      *o << data->getElement(i)[1].getObjectAndCast<Protein>()->getSecondaryStructure()->getElement(j).getInteger();
      *o << "\n";
    }
  }
  delete o;
}

static void exportPerceptionTypeToFile(PerceptionPtr perception, File output)
{
  if (output.exists())
    output.deleteFile();
  OutputStream* o = output.createOutputStream();
  TypePtr outputType = perception->getOutputType();
  for (size_t i = 0; i < outputType->getObjectNumVariables(); ++i)
  {
    TypePtr elementType = outputType->getObjectVariableType(i);
    if (elementType->inheritsFrom(enumValueType))
      *o << (int)elementType.dynamicCast<Enumeration>()->getNumElements() + 1;
    else if (elementType->inheritsFrom(booleanType))
      *o << "2";
    else
      *o << "0";
    *o << " ";
  }
  delete o;
}
/*
double LearningParameterObjectiveFunction::compute(ExecutionContext& context, const Variable& input) const
{
  LearningParameterPtr parameter = input.getObjectAndCast<NumericalLearningParameter>();
  jassert(parameter);
  
  InferencePtr inference;
  { // FIXME: Check if thread safe
    ScopedLock _(lock);
    LearningParameterPtr savedParameter = factory->getParameter(targetName, targetStage);
    factory->setParameter(targetName, targetStage, parameter);
    inference = factory->createInference(target);
    factory->setParameter(targetName, targetStage, savedParameter);
  }
  
  inference->train(context, learningData, validationData);
  
  ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
  inference->evaluate(context, learningData, evaluator, T("Evaluating on training data"));
  
  return evaluator->getEvaluatorForTarget(context, targetName)->getDefaultScore();
}
*/
Variable SnowBox::run(ExecutionContext& context)
{
  if (!loadData(context))
  {
    context.errorCallback(T("SnowBox::run"), T("Loading data failed"));
    return false;
  }

  ParameteredProteinInferenceFactoryPtr factory = createFactory(context);
  if (!factory)
  {
    context.errorCallback(T("SnowBox::run"), T("Unknown base learned and/or multiclass learner !"));
    return false;
  }

  printInformation(context);
  
  if (exportPerceptions)
  {
    PerceptionPtr perception = factory->createPerception(T("secondaryStructure"), ProteinInferenceFactory::residuePerception);
    jassert(perception);
    exportPerceptionTypeToFile(perception, File(output.getFullPathName() + T(".type")));
    if (learningData)
      exportPerceptionsToFile(context, learningData, perception, File(output.getFullPathName() + T(".learningSet")));
    if (testingData)
      exportPerceptionsToFile(context, testingData, perception, File(output.getFullPathName() + T(".testingSet")));
    if (validationData)
      exportPerceptionsToFile(context, validationData, perception, File(output.getFullPathName() + T(".validationSet")));
    return true;
  }

  if (optimizeLearningParameter)
  {
    /*
    LearningParameterPtr parameter = factory->getParameter(targetToOptimize, stageToOptimize);
    ObjectiveFunctionPtr objective = new LearningParameterObjectiveFunction(factory, target,
                                                                            targetToOptimize, stageToOptimize,
                                                                            learningData, validationData);
    OptimizerPtr optimizer = iterativeBracketingOptimizer(3, 2, uniformSampleAndPickBestOptimizer(10));
    Variable bestValue = optimizer->computeFunction(context, new OptimizerInput(objective,
                                                                                parameter->getAprioriDistribution(),
                                                                                parameter));
    std::cout << "Best parameter: " << bestValue.toString() << std::endl;
    return true;
    */
    factory->setTargetInferenceToOptimize(targetToOptimize, stageToOptimize);
  }
  
  ProteinSequentialInferencePtr inference = loadOrCreateIfFailInference(context, factory);
  if (saveIntermediatePredictions)
    inference->setProteinDebugDirectory(File(output.getFullPathName() + T(".debug")));

  if (useCrossValidation)
  {
    //inference->crossValidate(*context, learningData, evaluator, numberOfFolds);
    context.errorCallback(T("SnowBox::run"), T("CrossValidation not yet implemented (missing appropriate evaluator)"));
    return false;
  }
  else
  {
    ExecutionCallbackPtr evaluationCallback = new EvaluationInferenceCallback(inference, learningData, testingData, validationData, target, output);
    context.appendCallback(evaluationCallback);
    //context.appendCallback(new StackPrinterCallback());
    inference->train(context, learningData, validationData);

    File outputInferenceFile = output.getFullPathName() + T(".xml");
    context.informationCallback(T("SnowBox::InferenceSavedTo"), outputInferenceFile.getFullPathName());
    inference->saveToFile(context, outputInferenceFile);
    context.removeCallback(evaluationCallback);
  }
  return true;
}
