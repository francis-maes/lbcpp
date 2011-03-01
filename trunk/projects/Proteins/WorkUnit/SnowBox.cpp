
#include "SnowBox.h"
#include "Evaluator/ProteinEvaluator.h"

using namespace lbcpp;
/*
class LinearMultiClassNumericalProteinPredictorParameters : public NumericalProteinPredictorParameters
{
public:
  LinearMultiClassNumericalProteinPredictorParameters(size_t maxLearningIterations)
    : maxLearningIterations(maxLearningIterations)
  {
  }

  virtual FunctionPtr binaryClassifier(ProteinTarget target) const
    {return linearBinaryClassifier(createLearnerParameters(target));}

  virtual FunctionPtr multiClassClassifier(ProteinTarget target) const
    {return linearMultiClassClassifier(createLearnerParameters(target));}

  virtual FunctionPtr regressor(ProteinTarget target) const
    {return linearRegressor(createLearnerParameters(target));}

  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    jassertfalse;
    return FunctionPtr();
  }

protected:
  size_t maxLearningIterations;

  LearnerParametersPtr createLearnerParameters(ProteinTarget target) const
  {
    StochasticGDParametersPtr res = new StochasticGDParameters(constantIterationFunction(0.1));
    res->setMaxIterations(maxLearningIterations);

    String targetName = proteinClass->getMemberVariableName(target);
    if (target == ss3Target || target == ss8Target || target == stalTarget)
      res->setEvaluator(classificationEvaluator());
    else if (target == drTarget || target == saTarget || target == sa20Target)
      res->setEvaluator(binaryClassificationEvaluator());
    return res;
  }
};*/

ProteinPredictorParametersPtr SnowBox::createParameters(ExecutionContext& context) const
{
  jassert(false); // broken
  return ProteinPredictorParametersPtr();//new LinearMultiClassNumericalProteinPredictorParameters(maxIterations);
  /*
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
   context.errorCallback(T("SnowBox::run"), T("Unknown base learner !"));

  jassertfalse;
  return ProteinInferenceFactoryPtr();
  */
}

Variable SnowBox::run(ExecutionContext& context)
{
  if (!loadData(context))
    return false;

  ProteinPredictorParametersPtr parameters = createParameters(context);
  if (!parameters)
    return false;

  printInformation(context);

  ProteinSequentialPredictorPtr predictors = loadPredictorOrCreateIfFail(context);
  for (size_t i = 0; i < target->getNumStages(); ++i)
  {
    ProteinPredictorPtr predictor = new ProteinPredictor(parameters);
    for (size_t j = 0; j < target->getNumTasks(i); ++j)
      predictor->addTarget(target->getTask(i, j));
    predictors->addPredictor(predictor);
  }

  if (useCrossValidation)
  {
    //inference->crossValidate(*context, learningData, evaluator, numberOfFolds);
    context.errorCallback(T("SnowBox::run"), T("CrossValidation not yet implemented (missing appropriate evaluator)"));
    return false;
  }
  else
  {
    if (!predictors->train(context, learningData, validationData, T("Training"), false))
      return false;
    if (learningData && !predictors->evaluate(context, learningData, new ProteinEvaluator(), T("Evaluate on training data")))
      return false;
    if (testingData && !predictors->evaluate(context, testingData, new ProteinEvaluator(), T("Evaluate on testing data")))
      return false;
  }
  return true;
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
    validationData = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, validationDirectory, maxProteinsToLoad, T("Loading validation proteins"));

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
    testingData = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, testingDirectory, maxProteinsToLoad, T("Loading testing proteins"));

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
    for (size_t i = 0; i < target->getNumStages(); ++i)
      for (size_t j = 0; j < target->getNumTasks(i); ++j)
        context.resultCallback(T("Pass ") + String((int)i) + T(" - Task ") + String((int)j), target->getTaskName(i, j));
  }
}

ProteinSequentialPredictorPtr SnowBox::loadPredictorOrCreateIfFail(ExecutionContext& context) const
{
  return new ProteinSequentialPredictor();
  /*
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
   */
}

/*
 ** ProteinTargetsArgument
 */

bool ProteinTargetsArgument::loadFromString(ExecutionContext& context, const String& value)
{
  stages.clear();
  description = value;
  
  for (int begin = 0; begin != -1 && begin < value.length(); )
  {
    int end = value.indexOfChar(begin, T(')'));
    if (value[begin] != T('(') || end == -1)
    {
      context.errorCallback(T("ProteinTargetsArgument::loadFromString"), value.quoted() + T(" is not a valid target expression"));
      return false;
    }
    
    StringArray taskValues;
    taskValues.addTokens(value.substring(begin + 1, end), T("-"), T(""));
    
    begin = value.indexOfChar(end, T('('));
    if (begin == -1)
      begin = value.length();
    
    Variable numStages = Variable::createFromString(context, positiveIntegerType, value.substring(++end, begin));
    if (numStages.isMissingValue())
    {
      context.errorCallback(T("ProteinTargetsArgument::loadFromString"), value.quoted() + T(" is not a valid target expression"));
      return false;
    }
    
    for (size_t n = 0; n < (size_t)numStages.getInteger(); ++n)
    {
      std::vector<int> stageTargets;
      for (size_t i = 0; i < (size_t)taskValues.size(); ++i)
      {
        String task = taskValues[i].toLowerCase();
        if (task == T("ss3"))
          stageTargets.push_back(ss3Target);
        else if (task == T("ss8"))
          stageTargets.push_back(ss3Target);
        else if (task == T("sa"))
          stageTargets.push_back(sa20Target);
        else if (task == T("dr"))
          stageTargets.push_back(drTarget);
        else if (taskValues[i] == T("stal"))
          stageTargets.push_back(stalTarget);
        else
        {
          context.errorCallback(T("ProteinTargetsArgument::loadFromString"), taskValues[i].quoted() + T(" is not a valid task"));
          return false;
        }
      }
      
      stages.push_back(stageTargets);
    }
  }
  return true;
}
