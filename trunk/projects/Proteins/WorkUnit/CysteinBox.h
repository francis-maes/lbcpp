
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

};
