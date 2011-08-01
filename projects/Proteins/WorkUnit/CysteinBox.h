
#include <lbcpp/Core/Function.h>
#include "../Predictor/DecoratorProteinPredictorParameters.h"

namespace lbcpp
{

class CysteinLearnerParameters : public Object
{
public:
  ProteinPredictorParametersPtr predictorParameters;
  String foldDirectory;

protected:
  friend class CysteinLearnerParametersClass;
};

typedef ReferenceCountedObjectPtr<CysteinLearnerParameters> CysteinLearnerParametersPtr;
extern ClassPtr cysteinLearnerParametersClass;

class CysteinLearnerFunction : public SimpleUnaryFunction
{
public:
  CysteinLearnerFunction()
    : SimpleUnaryFunction(cysteinLearnerParametersClass, doubleType, T("CysteinLearner"))
    , numStacks(1) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    CysteinLearnerParametersPtr param = input.getObjectAndCast<CysteinLearnerParameters>(context);

    ContainerPtr trainingData = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(param->foldDirectory).getChildFile(T("train/")), 0, T("Loading training proteins"));
    if (!trainingData || !trainingData->getNumElements())
    {
      context.errorCallback(T("No training proteins !"));
      return 101.f;
    }

/*    if (true)
    {
      param->predictorParameters = new GaussianKernelPredictorParameters(context, param->predictorParameters, -4.7, trainingData->fold(0, 5));
      trainingData = trainingData->invFold(0, 5);
    }
*/
    ProteinSequentialPredictorPtr predictor = new ProteinSequentialPredictor();
    for (size_t i = 0; i < numStacks; ++i)
    {
      ProteinPredictorPtr stack = new ProteinPredictor(param->predictorParameters);
      stack->addTarget(dsbTarget);
      predictor->addPredictor(stack);
    }

    if (!predictor->train(context, trainingData, ContainerPtr(), T("Training")))
      return 102.f;

    ContainerPtr testingData = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(param->foldDirectory).getChildFile(T("test/")), 0, T("Loading testing proteins"));
    if (!testingData || !testingData->getNumElements())
    {
      context.warningCallback(T("No testing proteins ! Training score is returned !"));
      return 103.f;
    }

    ProteinEvaluatorPtr testEvaluator = new ProteinEvaluator();
    CompositeScoreObjectPtr testScores = predictor->evaluate(context, testingData, testEvaluator, T("Evaluate on test proteins"));

    return testEvaluator->getScoreObjectOfTarget(testScores, dsbTarget)->getScoreToMinimize();
  }

protected:
  friend class CysteinLearnerFunctionClass;

  size_t numStacks;
};

class CysteinCrossValidationOptimizer : public Optimizer
{
public:
  CysteinCrossValidationOptimizer(ProteinPredictorParametersPtr parameters = ProteinPredictorParametersPtr(), String path = String::empty)
    : parameters(parameters), path(path) {}
  
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {
    enum {numFolds = 5};
    for (size_t i = 0; i < numFolds; ++i)
    {
      CysteinLearnerParametersPtr candidate = new CysteinLearnerParameters();
      candidate->predictorParameters = parameters;
      candidate->foldDirectory = path + String((int)i);

      size_t numAttempts = 0;
      while (!optimizerContext->evaluate(candidate))
      {
        context.informationCallback(T("Evaluation - Attempt ") + String((int)numAttempts));
        Thread::sleep(optimizerContext->getTimeToSleep());
        ++numAttempts;
      }
      
      optimizerState->incTotalNumberOfRequests();
    }
    
    while (!optimizerContext->areAllRequestsProcessed())
      Thread::sleep(optimizerContext->getTimeToSleep());
    
    jassert(optimizerState->getNumberOfProcessedRequests() == numFolds);
    double sum = 0.0;
    size_t numValidScore = 0;
    const std::vector<std::pair<double, Variable> >& results = optimizerState->getProcessedRequests();
    for (size_t i = 0; i < results.size(); ++i)
      if (results[i].first <= 1.f)
      {
        sum += results[i].first;
        ++numValidScore;
      }
    
    if (!numValidScore)
      context.errorCallback(T("CParameterOptimizer"), T("Jobs Failed ! Parameter: ") + parameters->toString());

    optimizerState->setBestScore(sum / (double)numValidScore);
    return Variable(sum / (double)numValidScore, doubleType);
  }
  
protected:
  friend class CysteinCrossValidationOptimizerClass;
  
  ProteinPredictorParametersPtr parameters;
  String path;
};

class CysteinCrossValidationFunction : public SimpleUnaryFunction
{
public:
  CysteinCrossValidationFunction(String path = String::empty)
    : SimpleUnaryFunction(proteinPredictorParametersClass, doubleType, T("CrossValidation"))
    , path(path)
  {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    std::vector<String> destinations;
    destinations.push_back(T("jbecker@nic3"));
    destinations.push_back(T("fmaes@nic3"));
    destinations.push_back(T("amarcos@nic3"));

    FunctionPtr f = new CysteinLearnerFunction();
    OptimizerPtr optimizer = new CysteinCrossValidationOptimizer(input.getObjectAndCast<ProteinPredictorParameters>(context), path);
    OptimizerContextPtr optimizerContext = distributedOptimizerContext(context, f, T("CysBonds_BFS+C+5CV"), T("jbecker@monster24"), destinations, T("localhost"), 1664, 8, 3, 48, 60000);
    OptimizerStatePtr optimizerState = new OptimizerState();

    return optimizer->compute(context, optimizerContext, optimizerState);
  }
  
protected:
  friend class CysteinCrossValidationFunctionClass;

  String path;
};

class GaussianKernelParameters : public Object
{
public:
  GaussianKernelParameters(double gamma = -4.6, double C = 1.4)
    : gamma(gamma), C(C) {}

  std::vector<StreamPtr> createStreams()
  {
    std::vector<StreamPtr> res(2);
    
    std::vector<int> gammaValues;
    for (int i = -15; i <= 15; i += 2)
      gammaValues.push_back(i);
    
    std::vector<int> cValues;
    for (int i = -15; i <= 15; i += 2)
      cValues.push_back(i);

    res[0] = integerStream(positiveIntegerType, gammaValues);
    res[1] = integerStream(positiveIntegerType, cValues);

    return res;
  }

protected:
  friend class GaussianKernelParametersClass;

  double gamma;
  double C;
};

class CParameterOptimizer : public Optimizer
{
public:
  CParameterOptimizer(Lin09ParametersPtr parameters)
    : parameters(parameters) {}
  CParameterOptimizer() {}

  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {
    std::vector<double> values;
    values.push_back(1.4);
    values.push_back(3.4);
    values.push_back(6.4);
    values.push_back(7.4);
    values.push_back(8.4);
    values.push_back(11.4);
    
    for (size_t i = 0; i < values.size(); ++i)
    {
      Lin09PredictorParametersPtr candidate = new Lin09PredictorParameters(parameters);
      candidate->useLibSVM = true;
      candidate->C = values[i];
      
      size_t numAttempts = 0;
      while (!optimizerContext->evaluate(candidate))
      {
        context.informationCallback(T("Evaluation - Attempt ") + String((int)numAttempts));
        Thread::sleep(optimizerContext->getTimeToSleep());
        ++numAttempts;
      }

      optimizerState->incTotalNumberOfRequests();
    }
    
    while (!optimizerContext->areAllRequestsProcessed())
      Thread::sleep(optimizerContext->getTimeToSleep());
    
    jassert(optimizerState->getNumberOfProcessedRequests() == values.size());
    double bestScore = DBL_MAX;
    const std::vector<std::pair<double, Variable> >& results = optimizerState->getProcessedRequests();
    for (size_t i = 0; i < results.size(); ++i)
    {
      context.enterScope(T("Result ") + String((int)i));
      context.resultCallback(T("C"), results[i].second.getObjectAndCast<Lin09PredictorParameters>(context)->C);
      context.resultCallback(T("Score"), results[i].first);
      context.leaveScope();
      if (results[i].first < bestScore)
        bestScore = results[i].first;
    }

    if (bestScore == DBL_MAX)
      context.errorCallback(T("CParameterOptimizer"), T("Jobs Failed ! Parameter: ") + parameters->toString());

    optimizerState->setBestScore(bestScore);
    return bestScore;
  }

protected:
  friend class CParameterOptimizerClass;

  Lin09ParametersPtr parameters;
};

class COptimizerFunction : public SimpleUnaryFunction
{
public:
  COptimizerFunction(String inputDirectory = String::empty)
    : SimpleUnaryFunction(lin09ParametersClass, doubleType, T("COptimizer"))
    , inputDirectory(inputDirectory)
  {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    context.enterScope(T("COptimizing"));
    context.resultCallback(T("Parameters"), input);

    context.appendCallback(consoleExecutionCallback());

    Lin09ParametersPtr parameters = input.getObjectAndCast<Lin09Parameters>(context);

    FunctionPtr f = new CysteinCrossValidationFunction(inputDirectory);
    OptimizerPtr optimizer = new CParameterOptimizer(parameters);
    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, f, FunctionPtr(), 30000);
    OptimizerStatePtr optimizerState = new OptimizerState();

    Variable res = optimizer->compute(context, optimizerContext, optimizerState);
    
    context.leaveScope(res);
    return res;
  }

protected:
  friend class COptimizerFunctionClass;

  String inputDirectory;
};

class BFSCysteinProteinLearner : public WorkUnit
{
public:  
  Variable run(ExecutionContext& context)
  {
    FunctionPtr f = new COptimizerFunction(inputDirectory);

    OptimizerPtr optimizer = bestFirstSearchOptimizer();
    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, f, FunctionPtr(), 30000);
    OptimizerStatePtr optimizerState = streamBasedOptimizerState(context, Lin09Parameters::createInitialObject(), Lin09Parameters::createStreams());

    return optimizer->compute(context, optimizerContext, optimizerState);
  }

protected:
  friend class BFSCysteinProteinLearnerClass;

  String inputDirectory;
};

class CysteinLearnerWorkUnit : public WorkUnit
{
public:
  Variable run(ExecutionContext& context)
  {
    Lin09ParametersPtr lin09 = new Lin09Parameters();
    lin09->pssmWindowSize = 15;
    lin09->separationProfilSize = 9;
    lin09->usePositionDifference = true;
    lin09->pssmLocalHistogramSize = 100;

    Lin09PredictorParametersPtr lin09Pred = new Lin09PredictorParameters(lin09);
    lin09Pred->useLibSVM = true;
    lin09Pred->useLaRank = false;
    lin09Pred->useLibLinear = false;
    lin09Pred->useAddBias = true;
    
    lin09Pred->C = 1.4;
    lin09Pred->kernelGamma = -4.6;

    CysteinLearnerParametersPtr p = new CysteinLearnerParameters();
    p->predictorParameters = lin09Pred;
    p->foldDirectory = inputDirectory.getFullPathName();

    FunctionPtr f = new CysteinLearnerFunction();
    return f->compute(context, p);
  }

protected:
  friend class CysteinLearnerWorkUnitClass;

  File inputDirectory;
};

};
