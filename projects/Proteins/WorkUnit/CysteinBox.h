
#include <lbcpp/Core/Function.h>
#include "../Predictor/DecoratorProteinPredictorParameters.h"

namespace lbcpp
{

class StreamBasedOptimizationProblem : public OptimizationProblem
{
public:
  StreamBasedOptimizationProblem(const StreamPtr& stream, const FunctionPtr& objective, const Variable& initialGuess = Variable(), const SamplerPtr& sampler = SamplerPtr(), const FunctionPtr& validation = FunctionPtr())
    : OptimizationProblem(objective, initialGuess, sampler, validation), stream(stream)
  {}

  StreamPtr getStream() const
    {return stream;}

protected:
  friend class StreamBasedOptimizationProblemClass;

  StreamBasedOptimizationProblem() {}

  StreamPtr stream;
};

typedef ReferenceCountedObjectPtr<StreamBasedOptimizationProblem> StreamBasedOptimizationProblemPtr;
extern ClassPtr streamBasedOptimizationProblemClass;

class SimpleStreamBasedOptimizerState : public OptimizerState, public ExecutionContextCallback
{
public:
  void setResult(size_t index, double value)
  {
    if (results.size() <= index)
      results.resize(index + 1, Variable::missingValue(doubleType).getDouble());
    results[index] = value;
  }

  bool hasResult(size_t index) const
  {
    if (index >= results.size())
      return false;
    return results[index] != Variable::missingValue(doubleType).getDouble();
  }

  void addWorkUnit(const WorkUnitPtr& workUnit, size_t index)
  {
    workUnitMap.insert(std::make_pair(workUnit, index));
  }

  void computeScore()
  {
    double sum = 0.f;
    for (size_t i = 0; i < results.size(); ++i)
      sum += results[i];
    submitSolution(Variable(), results.size() == 0 ? 0 : sum / (double)results.size());
  }

  size_t getNumWorkUnitInProcess() const
    {return workUnitMap.size();}

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    jassert(workUnitMap.count(workUnit) == 1);
    setResult(workUnitMap[workUnit], result.getDouble());
    workUnitMap.erase(workUnit);
  }

protected:
  friend class SimpleStreamBasedOptimizerStateClass;

  std::vector<double> results;

private:
  std::map<WorkUnitPtr, size_t> workUnitMap;
};

typedef ReferenceCountedObjectPtr<SimpleStreamBasedOptimizerState> SimpleStreamBasedOptimizerStatePtr;
extern ClassPtr simpleStreamBasedOptimizerStateClass;

class StreamBasedOptimizer : public Optimizer
{
public:
  StreamBasedOptimizer(const File& optimizerStateFile = File::nonexistent)
    : Optimizer(optimizerStateFile) {}

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context, const OptimizationProblemPtr& problem) const
    {return new SimpleStreamBasedOptimizerState();}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    if (!problem->getClass()->inheritsFrom(streamBasedOptimizationProblemClass) || !optimizerState->getClass()->inheritsFrom(simpleStreamBasedOptimizerStateClass))
    {
      context.errorCallback(T("StreamBasedOptimizer::optimize"), T("You need a StreamBasedOptimizationProblem && a SimpleStreamBasedOptimizerState !"));
      return OptimizerStatePtr();
    }

    StreamBasedOptimizationProblemPtr streamedProblem = problem.staticCast<StreamBasedOptimizationProblem>();
    SimpleStreamBasedOptimizerStatePtr streamedState = optimizerState.staticCast<SimpleStreamBasedOptimizerState>();

    size_t currentIndex = 0;
    StreamPtr stream = streamedProblem->getStream();
    stream->rewind();

    size_t numWorkUnitPushed = 0;
    while (!stream->isExhausted())
    {
      Variable value = stream->next();
      if (!streamedState->hasResult(currentIndex))
      {
        std::vector<Variable> inputs(2);
        inputs[0] = value;
        inputs[1] = problem->getInitialGuess();

        WorkUnitPtr wu = new FunctionWorkUnit(problem->getObjective(), inputs);
        streamedState->addWorkUnit(wu, currentIndex);
        context.pushWorkUnit(wu, streamedState.get());
        ++numWorkUnitPushed;
      }
      ++currentIndex;
    }

    size_t previousNumWorkUnitInProgress = (size_t)-1;
    while (previousNumWorkUnitInProgress != 0)
    {
      context.flushCallbacks();
      if (streamedState->getNumWorkUnitInProcess() != previousNumWorkUnitInProgress)
      {
        saveOptimizerState(context, optimizerState);
        previousNumWorkUnitInProgress = streamedState->getNumWorkUnitInProcess();
      }
      context.progressCallback(new ProgressionState(numWorkUnitPushed - previousNumWorkUnitInProgress, numWorkUnitPushed, T("Evaluation")));
      juce::Thread::sleep(500);
    }
    context.progressCallback(new ProgressionState(numWorkUnitPushed, numWorkUnitPushed, T("Evaluation")));

    streamedState->computeScore();
    saveOptimizerState(context, optimizerState);

    return streamedState;
  }
};

class TestStreamBasedOptimizerFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? positiveIntegerType : stringType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    std::cout << "TestStreamBasedOptimizerFunction - " <<  inputs[1].toString() << " - " << inputs[0].toString() << std::endl;
    juce::Thread::sleep(context.getRandomGenerator()->sampleSize(3000));
    return Variable((double)inputs[0].getInteger() + 10.f, doubleType);
  }
};

class TestStreamBasedOptimizer : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    Variable v = Variable(String("Hello "), stringType);
    
    std::vector<int> values(5);
    for (size_t i = 0; i < values.size(); ++i)
      values[i] = i + 1;

    OptimizationProblemPtr problem = new StreamBasedOptimizationProblem(integerStream(positiveIntegerType, values), new TestStreamBasedOptimizerFunction(), v);
    OptimizerPtr optimizer = new StreamBasedOptimizer(context.getFile(T("testSimpleStreamBasedOptimizerState.txt")));

    OptimizerStatePtr state = optimizer->compute(context, problem).getObjectAndCast<OptimizerState>();
    std::cout << "Result: " << state->getBestScore() << std::endl;

    return true;
  }
};

class DisulfideBondLearner : public Function
{
public:
  DisulfideBondLearner(const ContainerPtr& trainingProteins, const ContainerPtr& testingProteins, const LargeProteinPredictorParametersPtr& predictor)
    : trainingProteins(trainingProteins), testingProteins(testingProteins), predictor(predictor) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return largeProteinParametersClass;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    LargeProteinParametersPtr parameters = input.getObjectAndCast<LargeProteinParameters>(context);

    LargeProteinPredictorParametersPtr clone = predictor->cloneAndCast<LargeProteinPredictorParameters>(context);
    clone->setParameters(parameters);

    ProteinPredictorPtr iteration = new ProteinPredictor(clone);
    iteration->addTarget(dsbTarget);

    if (!iteration->train(context, trainingProteins, ContainerPtr(), T("Training")))
      return 103.f;

    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    CompositeScoreObjectPtr scores = iteration->evaluate(context, testingProteins, evaluator, T("Evaluate on test proteins"));
    return evaluator->getScoreObjectOfTarget(scores, dsbTarget)->getScoreToMinimize();
  }

protected:
  friend class DisulfideBondLearnerClass;

  DisulfideBondLearner() {}

  ContainerPtr trainingProteins;
  ContainerPtr testingProteins;

  LargeProteinPredictorParametersPtr predictor;
};

class DisulfideBondByGreedyLeaveOneOut : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (supervisionDirectory == File::nonexistent)
    {
      context.errorCallback(T("No directory specified"));
      return false;
    }

    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, 0, T("Loading proteins"));
    if (!proteins || proteins->getNumElements() == 0)
    {
      context.errorCallback(T("No protein found !"));
      return false;
    }

    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters();
    predictor->learningMachineName = T("kNN-LOO");
    predictor->knnNeighbors = 5;

    OptimizationProblemPtr problem = new OptimizationProblem(new DisulfideBondLearner(proteins, proteins, predictor), new LargeProteinParameters());
    OptimizerPtr optimizer = bestFirstSearchOptimizer(LargeProteinParameters::createStreams(), optimizerStateFile);

    return optimizer->compute(context, problem);
  }

protected:
  friend class DisulfideBondByGreedyLeaveOneOutClass;

  File inputDirectory;
  File supervisionDirectory;
  File optimizerStateFile;
};

class DisulfideBondTestWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    //ContainerPtr trainingProteins = Protein::loadProteinsFromDirectoryPair(context, File(), inputDirectory.getChildFile(T("train/")), 0, T("Loading training proteins"));
    //ContainerPtr testingProteins = Protein::loadProteinsFromDirectoryPair(context, File(), inputDirectory.getChildFile(T("test/")), 0, T("Loading testing proteins"));
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, 0, T("Loading proteins"));

    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters();
    predictor->learningMachineName = T("kNN-LOO");
    predictor->knnNeighbors = 5;
    predictor->useFisherFilter = true;
    predictor->numFisherFeatures = 100;

    LargeProteinParametersPtr parameter = LargeProteinParameters::createTestObject(15);
    FunctionPtr learner = new DisulfideBondLearner(proteins, proteins, predictor);

    return learner->compute(context, parameter);
  }

protected:
  friend class DisulfideBondTestWorkUnitClass;

  File inputDirectory;
  File supervisionDirectory;
};

class GaussianSVMOptimizerState : public OptimizerState, public ExecutionContextCallback
{
public:
  void appendWorkUnit(const WorkUnitPtr& workUnit)
  {
    results.push_back(Variable::missingValue(doubleType).toDouble());
    workUnits[workUnit] = results.size() - 1;
  }

  bool hasResult(size_t index) const
  {
    if (index >= results.size() || results[index] == Variable::missingValue(doubleType).toDouble())
      return false;
    return true;
  }

  double getResult(size_t index) const
    {return results[index];}

  size_t getNumWorkUnitInProcess() const
    {return workUnits.size();}

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    results[workUnits[workUnit]] = result.getDouble();
    workUnits.erase(workUnit);
  }

protected:
  friend class GaussianSVMOptimizerStateClass;

  std::vector<double> results;

private:
  std::map<WorkUnitPtr, size_t> workUnits;
};

typedef ReferenceCountedObjectPtr<GaussianSVMOptimizerState> GaussianSVMOptimizerStatePtr;

class GaussianSVMOptimizer : public Optimizer
{
public:
  GaussianSVMOptimizer(const File& optimizerStateFile = File::nonexistent)
    : Optimizer(optimizerStateFile) {}

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context, const OptimizationProblemPtr& problem) const
  {
    return new GaussianSVMOptimizerState();
  }  

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    GaussianSVMOptimizerStatePtr state = optimizerState.staticCast<GaussianSVMOptimizerState>();

    std::vector<double> regularizer;
    for (double i = 1.f; i <= 7.f; i += 2.f)
      regularizer.push_back(i);
    std::vector<double> gamma;
    for (double i = -5.f; i <= 5; i += 2.f)
      gamma.push_back(i);

    // Send
    size_t index = 0;
    size_t numPushedWorkUnits = 0;
    for (size_t i = 0; i < regularizer.size(); ++i)
      for (size_t j = 0; j < gamma.size(); ++j)
      {
        if (!state->hasResult(index))
        {
          WorkUnitPtr wu = new FunctionWorkUnit(problem->getObjective(), regularizer[i], gamma[j]);
          state->appendWorkUnit(wu);
          context.pushWorkUnit(wu, state.get());
          ++numPushedWorkUnits;
        }
        ++index;
      }

    // Wait
    size_t previousNumWorkUnitInProgress = (size_t)-1;
    while (previousNumWorkUnitInProgress != 0)
    {
      context.flushCallbacks();
      if (state->getNumWorkUnitInProcess() != previousNumWorkUnitInProgress)
      {
        saveOptimizerState(context, optimizerState);
        previousNumWorkUnitInProgress = state->getNumWorkUnitInProcess();
      }
      context.progressCallback(new ProgressionState(numPushedWorkUnits - previousNumWorkUnitInProgress, numPushedWorkUnits, T("Evaluation")));
      juce::Thread::sleep(500);
    }
    context.progressCallback(new ProgressionState(numPushedWorkUnits, numPushedWorkUnits, T("Evaluation")));

    // Analyse
    index = 0;
    for (size_t i = 0; i < regularizer.size(); ++i)
    {
      context.enterScope(T("Regularizer ") + String(regularizer[i]));
      for (size_t j = 0; j < gamma.size(); ++j)
      {
        context.enterScope(T("Gamma ") + String(gamma[j]));
        context.resultCallback(T("Gamma"), Variable(gamma[j], doubleType));
        context.resultCallback(T("Result"), Variable(state->getResult(index), doubleType));
        context.leaveScope(state->getResult(index));

        state->submitSolution(new Pair(Variable(regularizer[i], doubleType), Variable(gamma[j], doubleType)), state->getResult(index));
        ++index;
      }
      context.leaveScope();
    }

    return state;
  }
};

class CrossValidationFunction : public Function
{
public:
  CrossValidationFunction(const FunctionPtr& objectiveFunction, const size_t numFolds = 10, const ExecutionContextPtr foldContext = ExecutionContextPtr())
    : objectiveFunction(objectiveFunction), numFolds(numFolds), foldContext(foldContext) {}

  virtual size_t getMinimumNumRequiredInputs() const
    {return 0;}
  
  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return anyType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    std::vector<Variable> subInputs(getNumInputs() + 1);
    for (size_t i = 0; i < getNumInputs(); ++i)
      subInputs[i + 1] = inputs[i];

    CompositeWorkUnitPtr workUnits = new CompositeWorkUnit(T("Folds"), numFolds);
    for (size_t i = 0; i < numFolds; ++i)
    {
      subInputs[0] = Variable(i, positiveIntegerType);
      WorkUnitPtr wu = new FunctionWorkUnit(objectiveFunction, subInputs);
      workUnits->setWorkUnit(i, wu);
    }

    ContainerPtr results = foldContext ? foldContext->run(workUnits).getObjectAndCast<Container>()
                                       : context.run(workUnits).getObjectAndCast<Container>();
    double sum = 0.f;
    for (size_t i = 0; i < numFolds; ++i)
      sum += results->getElement(i).getDouble();
    return Variable(sum / (double)numFolds, doubleType);
  }

protected:
  friend class CrossValidationFunctionClass;

  FunctionPtr objectiveFunction;
  size_t numFolds;
  ExecutionContextPtr foldContext;

  CrossValidationFunction() {}
};

class DisulfideBondGaussianSVM : public Function
{
public:
  DisulfideBondGaussianSVM(const String& inputDirectory, const String& supervisionDirectory, size_t numFolds)
    : inputDirectory(inputDirectory), supervisionDirectory(supervisionDirectory), numFolds(numFolds) {}

  virtual size_t getNumRequiredInputs() const
    {return 3;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? positiveIntegerType : doubleType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const size_t fold = inputs[0].getInteger();
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, context.getFile(inputDirectory), context.getFile(supervisionDirectory), 0, T("Loading proteins"));
    ContainerPtr train = proteins->invFold(fold, numFolds);
    ContainerPtr test = proteins->fold(fold, numFolds);

    LargeProteinParametersPtr parameter = new LargeProteinParameters();
    parameter->separationProfilSize = 9;
    parameter->saLocalHistogramSize = 80;
    parameter->useRelativeCysteinIndex = true;
    parameter->drWindowSize = 1;
    parameter->useCysteinDistance = true;

    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);
    predictor->learningMachineName = T("LibSVM");
    predictor->svmC = inputs[1].getDouble();
    predictor->svmGamma = inputs[2].getDouble();

    ProteinPredictorPtr iteration = new ProteinPredictor(predictor);
    iteration->addTarget(dsbTarget);
    
    if (!iteration->train(context, train, ContainerPtr(), T("Training")))
      return Variable::missingValue(doubleType);

    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    CompositeScoreObjectPtr scores = iteration->evaluate(context, test, evaluator, T("Evaluate on test proteins"));
    return evaluator->getScoreObjectOfTarget(scores, dsbTarget)->getScoreToMinimize();
  }

protected:
  friend class DisulfideBondGaussianSVMClass;

  String inputDirectory;
  String supervisionDirectory;
  size_t numFolds;

  DisulfideBondGaussianSVM() {}
};

class GaussianSVMWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    enum {numFolds = 10};
    std::vector<String> to(3);
    to[0] = T("jbecker@nic3");
    to[1] = T("fmaes@nic3");
    to[2] = T("amarcos@nic3");
    ExecutionContextPtr ctx = distributedExecutionContext(context, T("monster24.montefiore.ulg.ac.be"), 1664,
                                                          T("ESANN12"), T("jbecker@monster24"), to,
                                                          fixedResourceEstimator(1, 4 * 1024, 7 * 24));

    OptimizationProblemPtr problem = new OptimizationProblem(new CrossValidationFunction(new DisulfideBondGaussianSVM(inputDirectory, supervisionDirectory, numFolds), numFolds, ctx)); 

    OptimizerPtr optimizer = new GaussianSVMOptimizer(optimizerStateFile);
    return optimizer->compute(context, problem);
  }

protected:
  friend class GaussianSVMWorkUnitClass;

  File optimizerStateFile;
  String inputDirectory;
  String supervisionDirectory;
};

};
