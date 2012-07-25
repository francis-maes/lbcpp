
#include <lbcpp/Core/Function.h>
#include "../Predictor/DecoratorProteinPredictorParameters.h"
#include "../Data/Formats/FASTAFileParser.h"
#include "../Evaluator/ExhaustiveDisulfidePatternFunction.h"
#include "../Evaluator/GabowPatternFunction.h"
#include "../Evaluator/KolmogorovPerfectMatchingFunction.h"

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
  SimpleStreamBasedOptimizerState(const OptimizationProblemPtr& problem = OptimizationProblemPtr())
    : OptimizerState(problem) {}

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

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result, const ExecutionTracePtr& trace)
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
    {return new SimpleStreamBasedOptimizerState(problem);}

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
  GaussianSVMOptimizerState(const OptimizationProblemPtr& problem = OptimizationProblemPtr())
    : OptimizerState(problem) {}

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

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result, const ExecutionTracePtr& trace)
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
    {return new GaussianSVMOptimizerState(problem);}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    GaussianSVMOptimizerStatePtr state = optimizerState.staticCast<GaussianSVMOptimizerState>();

    std::vector<double> regularizer;
    for (double i = 0.f; i <= 7.f; i += 1.f)
      regularizer.push_back(i);
    std::vector<double> gamma;
    for (double i = 0.f; i <= 5; i += 1.f)
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
      wu->saveToFile(context, context.getFile(generateName(subInputs)));
      workUnits->setWorkUnit(i, wu);
    }
    return Variable(0.f, doubleType);
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

  String generateName(const std::vector<Variable>& inputs) const
  {
    String res;
    for (size_t i = 1; i < inputs.size(); ++i)
      res += T("#") + inputs[i].toString();
    res += T("#Fold") + inputs[0].toString();
    return res;
  }
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

class AverageDisulfideBondResults : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    juce::OwnedArray<File> files;
    directory.findChildFiles(files, File::findFiles, false, T("*#Fold0.trace"));

    std::map<double, std::map<double, double> > scores;
    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      const String prefix = files[i]->getFileName().replace(T("#Fold0.trace"), T(""));
      const String regularizerString = prefix.substring(1, prefix.lastIndexOfChar(T('#')));
      const String gammaString = prefix.substring(prefix.lastIndexOfChar(T('#')) + 1, prefix.length());

      double regularizer = Variable::createFromString(context, doubleType, regularizerString).getDouble();
      double gamma = Variable::createFromString(context, doubleType, gammaString).getDouble();

      scores[regularizer][gamma] = getAverageOverFolds(context, prefix);
    }

    for (std::map<double, std::map<double, double> >::iterator it = scores.begin(); it != scores.end(); ++it)
    {
      context.enterScope(T("Regularizer ") + String(it->first));
      context.resultCallback(T("Regularizer"), it->first);
      double bestScore = DBL_MAX;
      for (std::map<double, double>::iterator itt = it->second.begin(); itt != it->second.end(); ++itt)
      {
        context.enterScope(T("Gamma") + String(itt->first));
        context.resultCallback(T("Gamma"), itt->first);
        context.resultCallback(T("Score"), itt->second);
        if (itt->second < bestScore)
          bestScore = itt->second;
        context.leaveScope(itt->second);
      }
      context.resultCallback(T("BestScore"), bestScore);
      context.leaveScope(bestScore);
    }

    return true;
  }

  double getAverageOverFolds(ExecutionContext& context, const String& prefix) const
  {
    double sum = 0.f;
    size_t numFolds = 0;
    for (size_t j = 0; j < 10; ++j)
    {
      File f = directory.getChildFile(prefix + T("#Fold") + String((int)j) + T(".trace"));
      ExecutionTracePtr trace = ExecutionTrace::createFromFile(context, f).staticCast<ExecutionTrace>();
      if (trace)
      {
        sum += trace->getRootNode()->findFirstNode()->getReturnValue().getDouble();
        ++numFolds;
      }
      else
        context.warningCallback(T("No trace : ") + f.getFileName());
    }
    return  sum / (double)numFolds;
  }

protected:
  friend class AverageDisulfideBondResultsClass;

  File directory;
};

class DisulfideBondWorkUnit : public WorkUnit
{
public:
  DisulfideBondWorkUnit()
    : learningMachineName(T("ExtraTrees")),
      x3Trees(1000), x3Attributes(0), x3Splits(1),
      oxidizedCysteineThreshold(0.5f) {}

  virtual Variable run(ExecutionContext& context)
  {
    size_t numProteinsToLoad = 0;
#if JUCE_MAC && JUCE_DEBUG
    numProteinsToLoad = 20;
#endif
    ContainerPtr train = Protein::loadProteinsFromDirectoryPair(context, context.getFile(inputDirectory).getChildFile(T("train")), context.getFile(supervisionDirectory).getChildFile(T("train")), numProteinsToLoad, T("Loading training proteins"));
    ContainerPtr test = Protein::loadProteinsFromDirectoryPair(context, context.getFile(inputDirectory).getChildFile(T("test")), context.getFile(supervisionDirectory).getChildFile(T("test")), numProteinsToLoad, T("Loading testing proteins"));

    if (!train || !test || train->getNumElements() == 0 || test->getNumElements() == 0)
    {
      context.errorCallback(T("No training or testing proteins !"));
      return 100;
    }

    ProteinSequentialPredictorPtr iterations = new ProteinSequentialPredictor();
    // CBS
    /*
    LargeProteinParametersPtr cbsParameter = LargeProteinParameters::createFromFile(context, cbsParameterFile).dynamicCast<LargeProteinParameters>();
    LargeProteinPredictorParametersPtr cbsPredictor = new LargeProteinPredictorParameters(cbsParameter);
    cbsPredictor->learningMachineName = learningMachineName;
    cbsPredictor->x3Trees = x3Trees;
    cbsPredictor->x3Attributes = x3Attributes;
    cbsPredictor->x3Splits = x3Splits;
    ProteinPredictorPtr cbsIteration = new ProteinPredictor(cbsPredictor);
    cbsIteration->addTarget(cbsTarget);
    iterations->addPredictor(cbsIteration);
     */
    // ODSB
    /*
    LargeProteinParametersPtr odsbParameter = LargeProteinParameters::createFromFile(context, odsbParameterFile).dynamicCast<LargeProteinParameters>();
    LargeProteinPredictorParametersPtr odsbPredictor = new LargeProteinPredictorParameters(odsbParameter, oxidizedCysteineThreshold);
    odsbPredictor->learningMachineName = learningMachineName;
    odsbPredictor->x3Trees = x3Trees;
    odsbPredictor->x3Attributes = x3Attributes;
    odsbPredictor->x3Splits = x3Splits;
    ProteinPredictorPtr odsbIteration = new ProteinPredictor(odsbPredictor);
    odsbIteration->addTarget(odsbTarget);
    iterations->addPredictor(odsbIteration);
    */
    // DSB
    for (size_t i = 0; i < 1; ++i)
    {
      LargeProteinParametersPtr dsbParameter = LargeProteinParameters::createFromFile(context, odsbParameterFile).dynamicCast<LargeProteinParameters>();
      LargeProteinPredictorParametersPtr dsbPredictor = new LargeProteinPredictorParameters(dsbParameter);
      dsbPredictor->learningMachineName = learningMachineName;
      dsbPredictor->x3Trees = x3Trees;
      dsbPredictor->x3Attributes = x3Attributes;
      dsbPredictor->x3Splits = x3Splits;
      ProteinPredictorPtr dsbIteration = new ProteinPredictor(dsbPredictor);
      dsbIteration->addTarget(dsbTarget);
      iterations->addPredictor(dsbIteration);
    }
    // Copy CBS
    copyCysteineBondingStateSupervisons(context, train);
    copyCysteineBondingStateSupervisons(context, test);

    if (!iterations->train(context, train, ContainerPtr(), T("Training")))
      return Variable::missingValue(doubleType);

    if (outputDirectory != File::nonexistent)
    {
      //iteration->evaluate(context, train, saveToDirectoryEvaluator(outputDirectory.getChildFile(T("train")), T(".xml")), T("Saving train predictions to directory"));
      iterations->evaluate(context, test, saveToDirectoryEvaluator(outputDirectory.getChildFile(T("test")), T(".xml")), T("Saving test predictions to directory"));
    }

    ProteinEvaluatorPtr evaluator = createProteinEvaluator();
    CompositeScoreObjectPtr scores = iterations->evaluate(context, test, evaluator, T("EvaluateTest"));
/*
    std::cout << "---------------- Testing proteins --------------------" << std::endl;
    for (size_t i = 0; i < test->getNumElements(); ++i)
    {
      std::cout << "-- Protein " << i << std::endl;
      ProteinPtr p = iterations->compute(context, test->getElement(i).getObjectAndCast<Pair>()->getFirst(), Variable()).getObjectAndCast<Protein>();
      std::cout << "---CBS ---" << std::endl << p->getCysteinBondingStates(context)->toString() << std::endl;
      std::cout << "--- ODSB ---" << std::endl << p->getOxidizedDisulfideBonds(context)->toString() << std::endl;
    }
*/
    return evaluator->getScoreToMinimize(scores);
  }

protected:
  friend class DisulfideBondWorkUnitClass;

  String inputDirectory;
  String supervisionDirectory;
  File outputDirectory;
  File cbsParameterFile;
  File odsbParameterFile;
  String learningMachineName;
  size_t x3Trees;
  size_t x3Attributes;
  size_t x3Splits;
  double oxidizedCysteineThreshold;

  ProteinEvaluatorPtr createProteinEvaluator() const
  {
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator(oxidizedCysteineThreshold);
    evaluator->addEvaluator(cbsTarget, containerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore)), T("CBS"), true);
    evaluator->addEvaluator(cbsTarget, containerSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationAccuracyScore, true)), T("CBS Tuned Q2"));
    evaluator->addEvaluator(cbsTarget, containerSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationSensitivityAndSpecificityScore, false)), T("CBS Tuned S&S"));

    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore)), T("DSB Q2"));
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationAccuracyScore, true)), T("DSB Tuned Q2"));
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationSensitivityAndSpecificityScore, false)), T("DSB Tuned S&S"));

    evaluator->addEvaluator(dsbTarget, new DisulfidePatternEvaluator(), T("DSB QP"));
    evaluator->addEvaluator(dsbTarget, new DisulfidePatternEvaluator(new KolmogorovPerfectMatchingFunction(0.f), 0.f), T("DSB QP Perfect"));

//    evaluator->addEvaluator(odsbTarget, new DisulfidePatternEvaluator(), T("OxyDSB QP"));
//    evaluator->addEvaluator(odsbTarget, new DisulfidePatternEvaluator(new KolmogorovPerfectMatchingFunction(0.f), 0.f), T("OxyDSB QP Perfect"), true);
    
    return evaluator;
  }

  void copyCysteineBondingStateSupervisons(ExecutionContext& context, const ContainerPtr& proteins) const
  {
    for (size_t i = 0; i < proteins->getNumElements(); ++i)
      proteins->getElement(i).dynamicCast<Pair>()->getFirst().getObjectAndCast<Protein>()->setCysteinBondingStates(context, proteins->getElement(i).dynamicCast<Pair>()->getSecond().getObjectAndCast<Protein>()->getCysteinBondingStates(context));
  }
};

class AverageDirectoryScoresWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    juce::OwnedArray<File> files;
    directory.findChildFiles(files, File::findFiles, false, T("*.trace"));

    ScalarVariableMeanAndVariance res;
    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      ExecutionTracePtr trace = ExecutionTrace::createFromFile(context, *files[i]).staticCast<ExecutionTrace>();
      if (trace)
      {
        const double value = trace->getRootNode()->findFirstNode()->getReturnValue().getDouble();
        context.informationCallback(files[i]->getFileName(), String(value));
        res.push(value);
      }
      else
        context.warningCallback(files[i]->getFileName(), T("Not a trace"));
    }
    context.informationCallback(T("Average: ") + String(res.getMean()));
    context.informationCallback(T("Standard Deviation: ") + String(res.getStandardDeviation()));

    return res.getMean();
  }

protected:
  friend class AverageDirectoryScoresWorkUnitClass;

  File directory;
};

class CreateProteinFromPDBTestUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ProteinPtr protein = Protein::createFromPDB(context, pdbFile, true);
    if (!protein)
    {
      context.errorCallback(T("No protein !"));
      return false;
    }
    context.informationCallback(T("Protein's name: ") + protein->getName());
    context.informationCallback(T("Protein's length: ") + String((int)protein->getLength()));

    return protein;
  }

protected:
  friend class CreateProteinFromPDBTestUnitClass;

  File pdbFile;
};

class ExportDisulfideBondFeaturesWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, 0, T("Loading proteins"));
    if (!proteins)
    {
      context.errorCallback(T("ExportDisulfideBondFeaturesWorkUnit::run"), T("No proteins"));
      return false;
    }

    if (outputPrefix == String::empty)
      outputPrefix = T("disulfideBonds");

    LargeProteinParametersPtr parameter = LargeProteinParameters::createTestObject(40);
    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);

    OutputStream* o = context.getFile(outputPrefix + T(".arff")).createOutputStream();
    writeHeader(context, predictor, o);
    writeData(context, proteins, predictor, o);
    delete o;

    context.informationCallback(T("Output file name: ") + outputPrefix + T(".arff"));

    return true;
  }

protected:
  friend class ExportDisulfideBondFeaturesWorkUnitClass;

  File inputDirectory;
  File supervisionDirectory;
  String outputPrefix;

  void writeHeader(ExecutionContext& context, const LargeProteinPredictorParametersPtr& predictor, OutputStream* const o) const
  {
    *o << "@RELATION FeaturesOfDisulfideBonds\n"; 

    FunctionPtr proteinPerception = predictor->createProteinPerception();
    proteinPerception->initialize(context, proteinClass);
    TypePtr perceptionType = proteinPerception->getOutputType();

    FunctionPtr disulfideFunction = predictor->createDisulfideSymmetricResiduePairVectorPerception();
    disulfideFunction->initialize(context, perceptionType);
    TypePtr disulfideType = disulfideFunction->getOutputType();
    EnumerationPtr enumeration = disulfideType->getTemplateArgument(0)->getTemplateArgument(0).staticCast<Enumeration>();
    const size_t n = enumeration->getNumElements();
    context.informationCallback(T("Num. Attributes: ") + String((int)n));
    for (size_t i = 0; i < n; ++i)
      *o << "@ATTRIBUTE " << String((int)i) << "-" << enumeration->getElement(i)->getName().removeCharacters(T(" %#")) << " NUMERIC\n";
    *o << "@ATTRIBUTE class {0,1}\n";
  }

  void writeData(ExecutionContext& context, const ContainerPtr& proteins, const LargeProteinPredictorParametersPtr& predictor, OutputStream* const o) const
  {
    *o << "@DATA\n";

    FunctionPtr proteinPerception = predictor->createProteinPerception();
    FunctionPtr disulfideFunction = predictor->createDisulfideSymmetricResiduePairVectorPerception();

    const size_t n = proteins->getNumElements();
    context.informationCallback(T("Num. Proteins: ") + String((int)n));
    size_t numExamples = 0;
    for (size_t i = 0; i < n; ++i)
    {
      Variable perception = proteinPerception->compute(context, proteins->getElement(i).getObjectAndCast<Pair>()->getFirst());
      SymmetricMatrixPtr featuresVector = disulfideFunction->compute(context, perception).getObjectAndCast<SymmetricMatrix>();
      SymmetricMatrixPtr supervision = proteins->getElement(i).getObjectAndCast<Pair>()->getSecond().getObjectAndCast<Protein>()->getDisulfideBonds(context);
      jassert(featuresVector && supervision && featuresVector->getDimension() == supervision->getDimension());
      const size_t dimension = featuresVector->getDimension();
      for (size_t j = 0; j < dimension; ++j)
        for (size_t k = j + 1; k < dimension; ++k)
        {
          writeFeatures(featuresVector->getElement(j,k).getObjectAndCast<DoubleVector>(), o);
          *o << (supervision->getElement(j,k).getDouble() > 0.5f ? "1" : "0");
          *o << "\n";
          ++numExamples;
        }
    }
    context.informationCallback(T("Num. Examples: ") + String((int)numExamples));
  }

  void writeFeatures(const DoubleVectorPtr& features, OutputStream* const o) const
  {
    DenseDoubleVectorPtr ddv = features->toDenseDoubleVector();
    for (size_t i = 0; i < ddv->getNumValues(); ++i)
      *o << ddv->getValue(i) << ",";
  }
};

class ExportResidueFeaturesWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, 0, T("Loading proteins"));
    if (!proteins)
    {
      context.errorCallback(T("ExportResidueFeaturesWorkUnit::run"), T("No proteins"));
      return false;
    }

    if (outputPrefix == String::empty)
      outputPrefix = proteinClass->getVariableName(target);

    LargeProteinParametersPtr parameter = LargeProteinParameters::createTestObject(40);
    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);

    OutputStream* o = context.getFile(outputPrefix + T(".arff")).createOutputStream();
    writeHeader(context, predictor, o);
    writeData(context, proteins, predictor, o);
    delete o;

    context.informationCallback(T("Output file name: ") + outputPrefix + T(".arff"));

    return true;
  }

protected:
  friend class ExportResidueFeaturesWorkUnitClass;

  File inputDirectory;
  File supervisionDirectory;
  String outputPrefix;
  ProteinTarget target;

  void writeHeader(ExecutionContext& context, const LargeProteinPredictorParametersPtr& predictor, OutputStream* const o) const
  {
    *o << "@RELATION FeaturesOf_" << proteinClass->getVariableName(target) << "\n"; 

    FunctionPtr proteinPerception = predictor->createProteinPerception();
    proteinPerception->initialize(context, proteinClass);
    TypePtr perceptionType = proteinPerception->getOutputType();

    FunctionPtr residueFunction = predictor->createResidueVectorPerception();
    residueFunction->initialize(context, perceptionType);
    TypePtr residueType = residueFunction->getOutputType();
    EnumerationPtr enumeration = residueType->getTemplateArgument(0)->getTemplateArgument(0).staticCast<Enumeration>();
    const size_t n = enumeration->getNumElements();
    context.informationCallback(T("Num. Attributes: ") + String((int)n));
    for (size_t i = 0; i < n; ++i)
      *o << "@ATTRIBUTE " << String((int)i) << "-" << enumeration->getElement(i)->getName() << " NUMERIC\n";
    *o << "@ATTRIBUTE class {";
    TypePtr supervisionType = proteinClass->getMemberVariableType(target);
    if (supervisionType->inheritsFrom(objectVectorClass(doubleVectorClass(enumValueType, probabilityType))))
    {
      EnumerationPtr supEnum = supervisionType->getTemplateArgument(0)->getTemplateArgument(0).dynamicCast<Enumeration>();
      const size_t n = supEnum->getNumElements();
      for (size_t i = 0; i < n; ++i)
        *o << (i != 0 ? "," : "") << supEnum->getElementName(i);
    }
    else if (supervisionType->inheritsFrom(doubleVectorClass(positiveIntegerEnumerationEnumeration, probabilityType)))
      *o << "0,1";
    else
      jassertfalse;
    *o << "}\n";
  }

  void writeData(ExecutionContext& context, const ContainerPtr& proteins, const LargeProteinPredictorParametersPtr& predictor, OutputStream* const o) const
  {
    *o << "@DATA\n";

    FunctionPtr proteinPerception = predictor->createProteinPerception();
    FunctionPtr residueFunction = predictor->createResidueVectorPerception();

    const size_t n = proteins->getNumElements();
    context.informationCallback(T("Num. Proteins: ") + String((int)n));
    size_t numExamples = 0;
    for (size_t i = 0; i < n; ++i)
    {
      Variable perception = proteinPerception->compute(context, proteins->getElement(i).getObjectAndCast<Pair>()->getFirst());
      VectorPtr featuresVector = residueFunction->compute(context, perception).getObjectAndCast<Vector>();
      ContainerPtr supervision = proteins->getElement(i).getObjectAndCast<Pair>()->getSecond().getObjectAndCast<Protein>()->getTargetOrComputeIfMissing(context, target).getObjectAndCast<Container>();
      jassert(featuresVector && supervision && featuresVector->getNumElements() == supervision->getNumElements());
      const size_t n = featuresVector->getNumElements();
      for (size_t j = 0; j < n; ++j)
      {
        if (!supervision->getElement(j).exists())
          continue;
        writeFeatures(featuresVector->getElement(j).getObjectAndCast<DoubleVector>(), o);
        writeSupervision(supervision->getElement(j), o);
        *o << "\n";
        ++numExamples;
      }
    }
    context.informationCallback(T("Num. Examples: ") + String((int)numExamples));
  }

  void writeFeatures(const DoubleVectorPtr& features, OutputStream* const o) const
  {
    DenseDoubleVectorPtr ddv = features->toDenseDoubleVector();
    for (size_t i = 0; i < ddv->getNumValues(); ++i)
      *o << ddv->getValue(i) << ",";
  }

  void writeSupervision(const Variable& v, OutputStream* const o) const
  {
    if (v.inheritsFrom(doubleVectorClass(enumValueType, probabilityType)))
    {
      DoubleVectorPtr dv = v.getObjectAndCast<DoubleVector>();
      String className = dv->getElementsEnumeration()->getElementName(dv->getIndexOfMaximumValue());
      *o << className;
    }
    else if (v.inheritsFrom(probabilityType))
    {
      *o << (v.getDouble() > 0.5 ? "1" : "0");
    }
    else
      jassertfalse;
  }
};

class GenerateLargeProteinExperimentsWorkUnit : public WorkUnit
{
public:
  GenerateLargeProteinExperimentsWorkUnit()
    : numParameters(0), numExperiments(0) {}

  virtual Variable run(ExecutionContext& context)
  {
    std::map<String, LargeProteinParametersPtr> uniqueExperiments;
    for (size_t i = 0; i < numExperiments; ++i)
    {
      LargeProteinParametersPtr res = generateExperiment(context);
      if (uniqueExperiments.count(res->toString()) == 1)
      {
        --i;
        continue;
      }

      String fileName = T("Param-") + String((int)numParameters) + T("_Exp-") + String((int)i) + T(".xml");
      res->saveToFile(context, context.getFile(fileName));
      context.enterScope(fileName);
      context.leaveScope(res);

      uniqueExperiments[res->toString()] = res;
    }
    return true;
  }

protected:
  friend class GenerateLargeProteinExperimentsWorkUnitClass;

  size_t numParameters;
  size_t numExperiments;

  LargeProteinParametersPtr generateExperiment(ExecutionContext& context) const
  {
    LargeProteinParametersPtr res = new LargeProteinParameters();
    const size_t n = largeProteinParametersClass->getNumMemberVariables();
    std::vector<bool> notUsedParameters(n, true);
    for (size_t i = 0; i < numParameters; ++i)
    {
      size_t index;
      do
      {
        index = context.getRandomGenerator()->sampleSize(n);
      } while (!notUsedParameters[index]);
      
      Variable value = sampleValue(context, index);
      res->setVariable(index, value);
      notUsedParameters[index] = false;
    }
    return res;
  }

  Variable sampleValue(ExecutionContext& context, size_t index) const
  {
    const TypePtr varType = largeProteinParametersClass->getMemberVariableType(index);
    const String varName = largeProteinParametersClass->getMemberVariableName(index);
    if (varType->inheritsFrom(booleanType))
      return true;
    if (varName.endsWith(T("WindowSize")))
      return discretizeSampler(gaussianSampler(15, 15), 1, 40)->sample(context, context.getRandomGenerator());
    if (varName.endsWith(T("LocalHistogramSize")))
      return discretizeSampler(gaussianSampler(51, 50), 1, 100)->sample(context, context.getRandomGenerator());
    if (varName == T("separationProfilSize"))
      return discretizeSampler(gaussianSampler(7, 11), 1, 15)->sample(context, context.getRandomGenerator());

    jassertfalse;
    return Variable();
  }
};

class PlotDisulfideBondResultsWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    juce::OwnedArray<File> files;
    directory.findChildFiles(files, File::findFiles, false, T("Param-*_Exp-*.SVM_G-*_C-*.trace"));
    parseSVMFiles(context, files);
    
    files.clear();
    directory.findChildFiles(files, File::findFiles, false, T("Param-*_Exp-*.kNN.trace"));
    parseFiles(context, files);

    files.clear();
    directory.findChildFiles(files, File::findFiles, false, T("Param-*_Exp-*.Dumb.trace"));
    parseFiles(context, files);

    files.clear();
    directory.findChildFiles(files, File::findFiles, false, T("Param-*_Exp-*.x3.trace"));
    parseFiles(context, files);

    return true;
  }

protected:
  friend class PlotDisulfideBondResultsWorkUnitClass;

  File directory;

  size_t parseNumParameters(ExecutionContext& context, const String& str) const
  {
    static const size_t prefixLength = String(T("Param-")).length();
    const int end = str.indexOf(T("_Exp-"));
    jassert(end != -1);
    return Variable::createFromString(context, positiveIntegerType, str.substring(prefixLength, end)).getInteger();
  }

  size_t parseExperimentId(ExecutionContext& context, const String& str) const
  {
    static const size_t prefixLength = String(T("_Exp-")).length();
    const int start = str.indexOf(T("_Exp-")) + prefixLength;
    const int end = str.indexOfChar(start, T('.'));
    jassert(start != -1 && end != -1);
    return Variable::createFromString(context, positiveIntegerType, str.substring(start, end)).getInteger();
  }

  double getReturnValueOfTraceFile(ExecutionContext& context, File f) const
  {
    ExecutionTracePtr trace = ExecutionTrace::createFromFile(context, f).staticCast<ExecutionTrace>();
    jassert(trace);
    const Variable res = trace->getRootNode()->findFirstNode()->getReturnValue();
    if (!res.isDouble())
    {
      context.warningCallback(T("Error in Trace: ") + f.getFileName());
      return DBL_MAX;
    }
    return res.getDouble();
  }

  void parseFiles(ExecutionContext& context, const juce::OwnedArray<File>& files) const
  {
    if (files.size() == 0)
      return;
    // Read files
    typedef std::map<size_t, std::vector<std::pair<size_t, double> > > ScoresMap;
    ScoresMap results;
    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      const String fileName = files[i]->getFileName();
      const size_t numParameters = parseNumParameters(context, fileName);      
      const size_t experimentId = parseExperimentId(context, fileName);
      const double result = getReturnValueOfTraceFile(context, *files[i]);
      if (result == DBL_MAX)
        continue;

      results[numParameters].push_back(std::make_pair(experimentId, result));
    }

    // Print results
    OutputStream* o = context.getFile(T("result.plot")).createOutputStream();
    for (ScoresMap::iterator it = results.begin(); it != results.end(); ++it)
    {
      context.enterScope(T("Num. Parameters: ") + String((int)it->first));
      double sum = 0;
      std::vector<std::pair<size_t, double> > subResults = it->second;
      context.resultCallback(T("Num. Experiments"), subResults.size());
      for (size_t i = 0; i < subResults.size(); ++i)
      {
        context.enterScope(T("Experiment: ") + String((int)subResults[i].first));
        context.resultCallback(T("Experiment"), subResults[i].first);
        context.resultCallback(T("Result"), subResults[i].second);
        sum += subResults[i].second;
        context.leaveScope(subResults[i].second);
        *o << (int)it->first << "\t" << (int)subResults[i].first << "\t" << subResults[i].second << "\n";
      }
      context.leaveScope(sum / ((double)subResults.size()));
    }
    delete o;
  }

  double parseGamma(ExecutionContext& context, const String& str) const
  {
    static const size_t prefixLength = String(T(".SVM_G-")).length();
    const int start = str.indexOf(T(".SVM_G-")) + prefixLength;
    const int end = str.indexOfChar(start, T('_'));
    jassert(start != -1 && end != -1);
    return Variable::createFromString(context, doubleType, str.substring(start, end)).getDouble();
  }

  double parseRegularizer(ExecutionContext& context, const String& str) const
  {
    static const size_t prefixLength = String(T("_C-")).length();
    const int start = str.indexOf(T("_C-")) + prefixLength;
    const int end = str.indexOfChar(start, T('.'));
    return Variable::createFromString(context, doubleType, str.substring(start, end)).getDouble();
  }

  void parseSVMFiles(ExecutionContext& context, const juce::OwnedArray<File>& files) const
  {
    if (files.size() == 0)
      return;
    // Param -> Exp -> Gamma -> Reg -> Result
    typedef std::map<size_t, std::map<size_t, std::map<double, std::map<double, double> > > > ScoresMap;
    ScoresMap results;
    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      const String fileName = files[i]->getFileName();
      const size_t numParameters = parseNumParameters(context, fileName);      
      const size_t experimentId = parseExperimentId(context, fileName);
      const double gamma = parseGamma(context, fileName);
      const double regularizer = parseRegularizer(context, fileName);
      const double result = getReturnValueOfTraceFile(context, *files[i]);
      if (result == DBL_MAX)
        continue;

      results[numParameters][experimentId][gamma][regularizer] = result;
    }

    // Print results
    OutputStream* o = context.getFile(T("result.plot")).createOutputStream();

    typedef std::map<size_t, std::map<double, std::map<double, double> > > SubScoresMap;
    typedef std::map<double, std::map<double, double> > GammaScoresMap;
    typedef std::map<double, double> RegularizerScoresMap;
    for (ScoresMap::iterator it = results.begin(); it != results.end(); ++it)
    {
      for (SubScoresMap::iterator sit = it->second.begin(); sit != it->second.end(); ++sit)
      {
        context.enterScope(T("Param: ") + String((int)it->first) + T(" & Exp: ") + String((int)sit->first));
        double bestGamma = DBL_MAX;
        double bestGammaScore = DBL_MAX;
        for (GammaScoresMap::iterator git = sit->second.begin(); git != sit->second.end(); ++git)
        {
          context.enterScope(T("Gamma: ") + String(git->first));
          context.resultCallback(T("Gamma"), git->first);
          double bestReg = DBL_MAX;
          double bestRegScore = DBL_MAX;
          for (RegularizerScoresMap::iterator rit = git->second.begin(); rit != git->second.end(); ++rit)
          {
            context.enterScope(T("Regularizer: ") + String(rit->first));
            context.resultCallback(T("Regularizer"), rit->first);
            context.resultCallback(T("Result"), rit->second);
            if (rit->second < bestRegScore)
            {
              bestReg = rit->first;
              bestRegScore = rit->second;
            }
            context.leaveScope(rit->first);
          }
          context.resultCallback(T("Best Regularizer Score"), bestRegScore);
          context.resultCallback(T("Best Regularizer"), bestReg);
          if (bestRegScore < bestGammaScore)
          {
            bestGammaScore = bestRegScore;
            bestGamma = git->first;
          }
          context.leaveScope(bestRegScore);
        }
        context.resultCallback(T("Best Gamma"), bestGamma);
        context.resultCallback(T("Best Gamma Score"), bestGammaScore);
        context.leaveScope(bestGammaScore);
        *o << (int)it->first << "\t" << (int)sit->first << "\t" << bestGammaScore << "\n";
      }
    }
    delete o;
  }
};

class CountNumDisulfideBridgePerProtein : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    juce::OwnedArray<File> files;
    directory.findChildFiles(files, File::findFiles, false, T("*.xml"));

    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      ProteinPtr protein = Protein::createFromXml(context, *files[i]);
      protein->getCysteinBondingStates(context);
      size_t numCys = protein->getCysteinIndices().size();
      std::cout << (protein->getNumBondedCysteins() == numCys) << " " << files[i]->getFileName() << std::endl;
    }
    return true;
  }

protected:
  friend class CountNumDisulfideBridgePerProteinClass;

  File directory;
};

class CreateProteinWithCysteinBondingPropertyFromFastaWorkUnit : public WorkUnit
{
public:
  CreateProteinWithCysteinBondingPropertyFromFastaWorkUnit()
    : isPositiveExamples(true) {}

  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = (new FASTAFileParser(context, fastaFile))->load();
    if (!proteins)
      return false;
    const size_t n = proteins->getNumElements();
    context.informationCallback(T("Num. Proteins: ") + String((int)n));
    for (size_t i = 0; i < n; ++i)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>();
      protein->setCysteinBondingProperty(isPositiveExamples ? 1.f : 0.f);
      protein->saveToXmlFile(context, outputDirectory.getChildFile(protein->getName() + T(".xml")));
    }
    return true;
  }

protected:
  friend class CreateProteinWithCysteinBondingPropertyFromFastaWorkUnitClass;

  File fastaFile;
  File outputDirectory;
  bool isPositiveExamples;
};

class CysteinBondingPropertyWorkUnit : public WorkUnit
{
public:
  CysteinBondingPropertyWorkUnit()
    : learningMachineName(T("ExtraTrees")),
      x3Trees(1000), x3Attributes(0), x3Splits(1),
      sgdRate(1.f), sgdIterations(1000),
      useNormalization(false) {}

  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, File::nonexistent, context.getFile(inputDirectory), 0, T("Loading proteins"));

    enum {numFolds = 10};
    ScalarVariableMeanAndVariance stat;
    for (size_t i = 0; i < numFolds; ++i)
    {
      context.enterScope(T("Fold ") + String((int)i + 1) + T(" on ") + String((int)numFolds));
      double result = runOneFold(context, proteins, i, numFolds);
      context.leaveScope(result);
      if (result != DBL_MAX)
        stat.push(result);
    }
    context.resultCallback(T("Mean"), stat.getMean());
    context.resultCallback(T("Std. Dev."), stat.getStandardDeviation());
    context.informationCallback(String(stat.getMean()) + T(" +/- ") + String(stat.getStandardDeviation()));
    return stat.getMean();
  }

protected:
  friend class CysteinBondingPropertyWorkUnitClass;

  String inputDirectory;
  String learningMachineName;
  size_t x3Trees;
  size_t x3Attributes;
  size_t x3Splits;
  double sgdRate;
  size_t sgdIterations;
  bool useNormalization;

  double runOneFold(ExecutionContext& context, const ContainerPtr& proteins, size_t fold, size_t numFolds) const
  {
    ContainerPtr train = proteins->invFold(fold, numFolds);
    ContainerPtr test = proteins->fold(fold, numFolds);

    if (!train || !test || train->getNumElements() == 0 || test->getNumElements() == 0)
    {
      context.errorCallback(T("No training or testing proteins !"));
      return DBL_MAX;
    }

    LargeProteinParametersPtr parameter = new LargeProteinParameters();
    parameter->useProteinLength = true;
    parameter->useNumCysteins = true;
    parameter->useAminoAcidGlobalHistogram = true;
    parameter->usePSSMGlobalHistogram = true;
    
    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);
    predictor->learningMachineName = learningMachineName;
    // Config ExtraTrees
    predictor->x3Trees = x3Trees;
    predictor->x3Attributes = x3Attributes;
    predictor->x3Splits = x3Splits;
    
    // Config SGD
    predictor->sgdRate = sgdRate;
    predictor->sgdIterations = sgdIterations;
    
    // Config kNN
    predictor->knnNeighbors = 5;
    
    predictor->useAddBias = false;
    
    ProteinPredictorPtr iteration = new ProteinPredictor(predictor);
    iteration->addTarget(cbpTarget);
    
    if (!iteration->train(context, train, ContainerPtr(), T("Training")))
      return DBL_MAX;
    
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    //evaluator->addEvaluator(cbpTarget, rocAnalysisEvaluator(binaryClassificationAccuracyScore, true), T("Cystein Bonding Property (Acc.)"));
    evaluator->addEvaluator(cbpTarget, binaryClassificationEvaluator(), T("Cystein Bonding Property (Classif.)"));
    
    CompositeScoreObjectPtr scores = iteration->evaluate(context, test, evaluator, T("Evaluate on test proteins"));
    return evaluator->getScoreObjectOfTarget(scores, cbpTarget)->getScoreToMinimize();
  }
};

class ComputePearsonCorrelationOfDisulfideBondWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, 0, T("Loading proteins"));
    if (!proteins)
    {
      context.errorCallback(T("ComputePearsonCorrelationOfDisulfideBondWorkUnit::run"), T("No proteins"));
      return false;
    }

    EnumerationPtr variableEnumeration;
    SymmetricMatrixPtr correlation;
    {
      std::vector<std::vector<double> > variables; // [variable, examples]
      variableEnumeration = insertExamples(context, proteins, variables);
      //displayStructure(context, variables);
      normalizeExamples(context, variables);
      //displayStructure(context, variables);
      correlation = computePearsonCorrelation(context, variables);
    }
//    rankCorrelations(context, correlation, variableEnumeration);
    meanRankCorrelations(context, correlation, variableEnumeration);

    return true;
  }

protected:
  friend class ComputePearsonCorrelationOfDisulfideBondWorkUnitClass;

  File inputDirectory;
  File supervisionDirectory;

  void displayStructure(ExecutionContext& context, const std::vector<std::vector<double> >& variables) const
  {
    const size_t numVariables = variables.size();
    const size_t numExamples = numVariables == 0 ? 0 : variables[0].size();
    for (size_t i = 0; i < numExamples; ++i)
    {
      for (size_t j = 0; j < numVariables; ++j)
        std::cout << variables[j][i] << " ";
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

  EnumerationPtr insertExamples(ExecutionContext& context, const ContainerPtr& proteins, std::vector<std::vector<double> >& result) const
  {
    EnumerationPtr featureEnumeration; // res
    const size_t n = proteins->getNumElements();
    context.informationCallback(T("Num. Proteins: ") + String((int)n));    

    LargeProteinParametersPtr parameter = LargeProteinParameters::createTestObject(40);
    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);

    FunctionPtr proteinPerception = predictor->createProteinPerception();
    FunctionPtr disulfideFunction = predictor->createDisulfideSymmetricResiduePairVectorPerception();

    for (size_t i = 0; i < n; ++i)
    {
      Variable perception = proteinPerception->compute(context, proteins->getElement(i).getObjectAndCast<Pair>()->getFirst());
      SymmetricMatrixPtr featuresVector = disulfideFunction->compute(context, perception).getObjectAndCast<SymmetricMatrix>();
      //SymmetricMatrixPtr supervision = proteins->getElement(i).getObjectAndCast<Pair>()->getSecond().getObjectAndCast<Protein>()->getDisulfideBonds(context);
      //jassert(featuresVector && supervision && featuresVector->getDimension() == supervision->getDimension());
      const size_t dimension = featuresVector->getDimension();
      for (size_t j = 0; j < dimension; ++j)
        for (size_t k = j + 1; k < dimension; ++k)
        {
          DenseDoubleVectorPtr ddv = featuresVector->getElement(j, k).getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
          if (i == 0 && j == 0 && k == 1) // first iteration
          {
            result.resize(ddv->getNumElements());
            context.informationCallback(T("Num. Attributes: ") + String((int)ddv->getNumElements()));
            featureEnumeration = featuresVector->getElement(j, k).getObjectAndCast<DoubleVector>()->getElementsEnumeration();//disulfideFunction->getOutputType()->getTemplateArgument(0).dynamicCast<Enumeration>();
          }
          jassert(ddv->getNumElements() == ddv->getNumValues());
          for (size_t m = 0; m < ddv->getNumValues(); ++m)
          {
            Variable v = ddv->getElement(m);
            result[m].push_back(v.exists() ? v.getDouble() : 0.f);
          }
        }
    }
    context.informationCallback(T("Num. Examples: ") + String((int)result[0].size()));
    return featureEnumeration;
  }

  void normalizeExamples(ExecutionContext& context, std::vector<std::vector<double> >& result) const
  {
    jassert(result.size() > 0);
    const size_t numExamples = result[0].size();

    std::vector<ScalarVariableMeanAndVariancePtr> meanAndVariances(result.size());      
    for (size_t i = 0; i < result.size(); ++i)
    {
      meanAndVariances[i] = new ScalarVariableMeanAndVariance();
      for (size_t j = 0; j < numExamples; ++j)
        meanAndVariances[i]->push(result[i][j]);
    }

    for (size_t i = 0; i < result.size(); ++i)
    {
      const double mean = meanAndVariances[i]->getMean();
      const double stdDev = meanAndVariances[i]->getStandardDeviation() < 1e-6 ? 1.f : meanAndVariances[i]->getStandardDeviation();
      for (size_t j = 0; j < numExamples; ++j)
        result[i][j] = (result[i][j] - mean) / stdDev;
    }
  }

  SymmetricMatrixPtr computePearsonCorrelation(ExecutionContext& context, const std::vector<std::vector<double> >& variables) const
  {
    jassert(variables.size() > 0);
    const size_t numExamples = variables[0].size();
    SymmetricMatrixPtr res = symmetricMatrix(probabilityType, variables.size());
    for (size_t i = 0; i < variables.size(); ++i)
      for (size_t j = i; j < variables.size(); ++j)
      {
        if (i == j)
          res->setElement(i, j, probability(1.f));
        else
        {
          jassert(variables[i].size() == variables[j].size());
          double sum = 0.f;
          for (size_t k = 0; k < numExamples; ++k)
            sum += variables[i][k] * variables[j][k];
          res->setElement(i, j, probability(sum / (double)numExamples));
        }
      }
    return res;
  }

  void rankCorrelations(ExecutionContext& context, const SymmetricMatrixPtr& correlations, const EnumerationPtr& variableEnumeration) const
  {
    OutputStream* o = context.getFile(T("pearsonCorrelation.plot")).createOutputStream();
    *o << "# Rank InvRank PearsonScore Variables\n";
    typedef std::multimap<double, std::pair<size_t, size_t> > ScoresMap;
    ScoresMap scores;
    const size_t dimension = correlations->getDimension();
    for (size_t i = 0; i < dimension; ++i)
      for (size_t j = i + 1; j < dimension; ++j)
        scores.insert(std::make_pair(fabs(correlations->getElement(i, j).getDouble()), std::make_pair(i, j)));

    size_t rankIndex = 0;
    context.enterScope(T("Ranked variables"));
    for (ScoresMap::reverse_iterator it = scores.rbegin(); it != scores.rend(); ++it, ++rankIndex)
      *o << (int)rankIndex << "\t" << (int)(scores.size() - rankIndex) << "\t" << it->first << "\t"
      << "\"" << variableEnumeration->getElementName(it->second.first) << " & " << variableEnumeration->getElementName(it->second.second) << "\"\n";
    context.leaveScope();
    delete o;
  }

  void meanRankCorrelations(ExecutionContext& context, const SymmetricMatrixPtr& correlations, const EnumerationPtr& variableEnumeration) const
  {
    OutputStream* o = context.getFile(T("pearsonCorrelation.mean.plot")).createOutputStream();
    *o << "# Rank InvRank MeanPearsonScore StdDevpearsonScore MinValue MaxValue Variable\n";
    typedef std::multimap<double, std::pair<size_t, ScalarVariableStatisticsPtr> > ScoresMap;
    ScoresMap scores;
    const size_t dimension = correlations->getDimension();
    for (size_t i = 0; i < dimension; ++i)
    {
      ScalarVariableStatisticsPtr stat = new ScalarVariableStatistics();
      for (size_t j = 0; j < dimension; ++j)
        stat->push(fabs(correlations->getElement(i, j).getDouble()));
      scores.insert(std::make_pair(stat->getMean(), std::make_pair(i, stat)));
    }

    size_t rankIndex = 0;
    context.enterScope(T("Mean Ranked Variables"));
    for (ScoresMap::reverse_iterator it = scores.rbegin(); it != scores.rend(); ++it, ++rankIndex)
      *o << (int)rankIndex << "\t" << (int)(scores.size() - rankIndex) << "\t" << it->first << "\t" << it->second.second->getStandardDeviation() << "\t"
      << it->second.second->getMinimum() << "\t" << it->second.second->getMaximum() << "\t"
      << "\"" << variableEnumeration->getElementName(it->second.first) << "\"\n";
    context.leaveScope();
    delete o;
  }
};

class ParseSP39DataWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    juce::OwnedArray<File> files;
    inputDirectory.findChildFiles(files, File::findFiles, false, T("*"));
    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      context.informationCallback(files[i]->getFileName());
      ProteinPtr protein = StreamPtr(new SP39FileParser(context, *files[i]))->next().getObjectAndCast<Protein>();
      jassert(protein);
      protein->saveToFile(context, outputDirectory.getChildFile(protein->getName() + T(".xml")));
    }
    return true;
  }

protected:
  friend class ParseSP39DataWorkUnitClass;

  File inputDirectory;
  File outputDirectory;
};

class AnalysePatternWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
#if 0 // Stop compilation warnings
    juce::OwnedArray<File> files;
    inputDirectory.findChildFiles(files, File::findFiles, false, T("*.xml"));
    double threshold = 0.f;
    double bestThreshold = 0.f;
    size_t bestErrorPattern = files.size();

    size_t numSupFullyBonded = 0;
    size_t numSupPartiallyBonded = 0;
    size_t numSupNotBonded = 0;
    size_t numSupNotBondedWithPattern = 0;

    for (threshold = 0.f; threshold < 0.3; threshold += 0.05)
    {
      /*
      FunctionPtr patternBuilder = new GreedyDisulfidePatternBuilder(6, threshold);
      patternBuilder->initialize(context, symmetricMatrixClass(probabilityType));
*/
      FunctionPtr kolmogorov = new KolmogorovPerfectMatchingFunction(threshold);
      kolmogorov->initialize(context, doubleSymmetricMatrixClass(doubleType));

      size_t errorNumBonds = 0;
      size_t errorPatterns = 0;
      size_t numDiffPatterns = 0;
      size_t errorKolmogorov = 0;
      size_t samePrediction = 0;
      for (size_t i = 0; i < (size_t)files.size(); ++i)
      {
        //std::cout << "File: " << files[i]->getFileName();

        ProteinPtr protein = Protein::createFromFile(context, *files[i]);
        jassert(protein);
        DoubleSymmetricMatrixPtr matrix = protein->getDisulfideBonds(context);//.dynamicCast<DoubleSymmetricMatrixPtr>();
        jassert(matrix);

        juce::OwnedArray<File> supFiles;
        supervisionDirectory.findChildFiles(supFiles, File::findFiles, true, files[i]->getFileName());
        jassert(supFiles.size() == 1);      

        ProteinPtr supProtein = Protein::createFromFile(context, *supFiles[0]);
        jassert(supProtein);
        DoubleSymmetricMatrixPtr supMatrix = supProtein->getDisulfideBonds(context);
        jassert(supMatrix);
/*
        DoubleSymmetricMatrixPtr pattern = patternBuilder->compute(context, matrix).getObjectAndCast<DoubleSymmetricMatrix>();
        for (size_t i = 0; i < pattern->getDimension(); ++i)
          pattern->setValue(i, i, 0.f);
*/
        DoubleSymmetricMatrixPtr kolPattern = kolmogorov->compute(context, matrix).getObjectAndCast<DoubleSymmetricMatrix>();
  /*
        std::cout << std::endl;
        std::cout << "Original" << std::endl;
        std::cout << matrix->toString() << std::endl;
        std::cout << "Greedy" << std::endl;
        std::cout << pattern->toString() << std::endl;
        std::cout << "Kolmogorov" << std::endl;
        std::cout << kolPattern->toString() << std::endl;
        std::cout << "Supervision" << std::endl;
        std::cout << supMatrix->toString() << std::endl;
        std::cout << std::endl;
  */
        /*
        std::cout << "Greedy Score: " << getScore(pattern) << std::endl;
        std::cout << "Kolmogorov Score : " << getScore(kolPattern) << std::endl;
        jassert(getScore(pattern) < getScore(kolPattern) + 1e-6);
        const size_t supNumBonds = getNumBonds(supMatrix);
        const size_t patNumBonds = getNumBonds(pattern);
        jassert(protein->getCysteinIndices().size() == matrix->getDimension());
         */
  //      std::cout << "\t#Cys: " << matrix->getDimension() << "\t#GreedyBonds: " << patNumBonds << "\t#SupBonds: " << supNumBonds << "\tScore: " << getScore(matrix);
  /*
        std::cout << "Evolution of scores" << std::endl;
        double previousScore = 0.f;
        for (size_t i = 1; i < 26; ++i)
        {
          patternBuilder = new GreedyDisulfidePatternBuilder(i, threshold);
          patternBuilder->initialize(context, symmetricMatrixClass(probabilityType));

          pattern = patternBuilder->compute(context, matrix).getObjectAndCast<DoubleSymmetricMatrix>();
          for (size_t j = 0; j < pattern->getDimension(); ++j)
            pattern->setValue(j, j, 0.f);
          const double score = getScore(pattern);
          std::cout << "L=" << i << "\t\tScore: " << score << std::endl;
          jassert(previousScore <= score);
          previousScore = score;
        } continue;
  */
  /*
        if (patNumBonds != supNumBonds)
        {
  //        std::cout << "\t\t***** Error #Bonds *****";
          ++errorNumBonds;
        }
        if (matrix->getDimension() > 2 * patNumBonds + 1)
        {
  //        std::cout << "\t\t***** Not Full Pattern *****";
        }
  */
        if (!checkPattern(kolPattern, supMatrix))
        {
  //        std::cout << "\t\t***** Error Pattern *****";
          ++errorPatterns;
        }
/*
        bool patError = false;
        if (!checkPattern(pattern, kolPattern))
        {
          ++numDiffPatterns;
          patError = true;
        }
        bool kolError = false;
        if (!checkPattern(kolPattern, supMatrix))
        {
          ++errorKolmogorov;
          kolError = true;
        }
        if (patError == kolError)
          ++samePrediction;
        //std::cout << std::endl;

        if (supMatrix->getDimension() <= (supNumBonds + 1) * 2)
          ++numSupFullyBonded;
        else if (supNumBonds == 0)
        {
          ++numSupNotBonded;
          if (!checkPattern(pattern, zeroSymmetricMatrix(pattern->getDimension())))
              ++numSupNotBondedWithPattern;
        }
        else
          ++numSupPartiallyBonded;
*/
      }

    std::cout << "Threshold: " << threshold
      << "\t\tError Pattern: " << errorPatterns;
      //<< "\t\tDiff. from Kolmo: " << numDiffPatterns 
      //<< "\t\tError Kolmogorov: " << errorKolmogorov << std::endl;
      //std::cout << "Kolmogorov Qp == Pattern Qp : " << samePrediction;
      std::cout << std::endl;
    if (errorPatterns < bestErrorPattern)
    {
      bestErrorPattern = errorPatterns;
      bestThreshold = threshold;
    }
  }

  std::cout << "Best Threshold: " << bestThreshold << "\t\tError Pattern: " << bestErrorPattern << std::endl;
/*
    std::cout << "Num. Proteins: " << files.size() << std::endl;
    std::cout << "#Full: " << numSupFullyBonded
              << "\t#Partial: " << numSupPartiallyBonded
              << "\t#None: "<< numSupNotBonded << std::endl;
    std::cout << "#None With Pattern: " << numSupNotBondedWithPattern << std::endl;
*/
#endif
    return true;
  }

protected:
  friend class AnalysePatternWorkUnitClass;

  File inputDirectory;
  File supervisionDirectory;

  size_t getNumBonds(const DoubleSymmetricMatrixPtr& matrix) const
  {
    size_t numBonds = 0;
    for (size_t i = 0; i < matrix->getDimension(); ++i)
      for (size_t j = i; j < matrix->getDimension(); ++j)
        if (matrix->getValue(i, j) > 0.f)
          ++numBonds;
    return numBonds;
  }

  double getScore(const DoubleSymmetricMatrixPtr& matrix) const
  {
    double score = 0.f;
    for (size_t i = 0; i < matrix->getDimension(); ++i)
      for (size_t j = i; j < matrix->getDimension(); ++j)
        if (matrix->getValue(i, j) > 0.f)
          score += matrix->getValue(i, j);
    return score;
  }

  bool checkPattern(const DoubleSymmetricMatrixPtr& pattern, const DoubleSymmetricMatrixPtr& supervision) const
  {
    const size_t dimension = pattern->getDimension();
    for (size_t i = 0; i < dimension; ++i)
      for (size_t j = i; j < dimension; ++j)
        if ((supervision->getValue(i, j) > 0.f) != (pattern->getValue(i, j) > 0.f))
          return false;
    return true;
  }
};

class TestGabowAlgorithmWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    DoubleSymmetricMatrixPtr graph = new DoubleSymmetricMatrix(doubleType, 10, 0.f);
    graph->setValue(0, 1, 1.f);
    graph->setValue(0, 2, 1.f);
    graph->setValue(0, 9, 1.f);
    graph->setValue(1, 2, 1.f);
    graph->setValue(2, 3, 1.f);
    graph->setValue(2, 8, 1.f);
    graph->setValue(3, 6, 1.f);
    graph->setValue(3, 7, 1.f);
    graph->setValue(4, 5, 1.f);
    graph->setValue(4, 8, 1.f);
    graph->setValue(5, 6, 1.f);
    graph->setValue(6, 7, 1.f);

/*
    DoubleSymmetricMatrixPtr graph = new DoubleSymmetricMatrix(doubleType, 6, 0.f);
    graph->setValue(0, 1, 1.f);
    graph->setValue(1, 2, 1.f);
    graph->setValue(2, 3, 1.f);
    graph->setValue(3, 0, 1.f);

    graph->setValue(0, 4, 1.f);
    graph->setValue(1, 5, 1.f);
*/
    FunctionPtr kolmogorov = new KolmogorovPerfectMatchingFunction();
    SymmetricMatrixPtr res = kolmogorov->compute(context, graph).getObjectAndCast<SymmetricMatrix>();
    std::cout << res->toString() << std::endl;
    return res;
/*
    FunctionPtr gabow = new GabowPatternFunction();
    SymmetricMatrixPtr res = gabow->compute(context, graph).getObjectAndCast<SymmetricMatrix>();
    std::cout << res->toString() << std::endl;
    return res;
*/
  }
};

class LowMemoryDisulfideBondWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    size_t numProteinsToLoad = 0;
#ifdef JUCE_DEBUG
    numProteinsToLoad = 10;
    context.warningCallback(T("Num. Proteins to load was set to ") + String((int)numProteinsToLoad));
#endif // !JUCE_DEBUG
    ContainerPtr trainingProteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory.getChildFile(T("train")), supervisionDirectory.getChildFile(T("train")), numProteinsToLoad, T("Loading training proteins"));
    ContainerPtr testingProteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory.getChildFile(T("test")), supervisionDirectory.getChildFile(T("test")), numProteinsToLoad, T("Loading testing proteins"));
    if (!trainingProteins && !testingProteins)
    {
      context.errorCallback(T("ExportDisulfideBondFeaturesWorkUnit::run"), T("No proteins"));
      return false;
    }

    LargeProteinParametersPtr parameter = LargeProteinParameters::createTestObject(19);
    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);

    ContainerPtr trainData = getData(context, trainingProteins, predictor);
    ContainerPtr testData = getData(context, testingProteins, predictor);

    FunctionPtr x3Function = lowMemoryRTreeFunction(4, 20, 1);
    ContainerPtr result = x3Function->compute(context, trainData, testData).getObjectAndCast<Container>();

    ContainerPtr predictedProteins = addPredictions(context, testingProteins, result);
    EvaluatorPtr evaluator = createProteinEvaluator();
    ScoreObjectPtr res = evaluator->createEmptyScoreObject(context, FunctionPtr());
    const size_t numTest = testingProteins->getNumElements();
    for (size_t i = 0; i < numTest; ++i)
      evaluator->updateScoreObject(context, res, testingProteins->getElement(i).getObject(), predictedProteins->getElement(i));
    evaluator->finalizeScoreObject(res, FunctionPtr());

    context.enterScope(T("EvaluateTest"));
    context.leaveScope(res);
    return true;
  }

protected:
  friend class LowMemoryDisulfideBondWorkUnitClass;

  File inputDirectory;
  File supervisionDirectory;

  ContainerPtr getData(ExecutionContext& context, const ContainerPtr& proteins, const LargeProteinPredictorParametersPtr& predictor) const
  {
    FunctionPtr proteinPerception = predictor->createProteinPerception();
    proteinPerception->initialize(context, proteinClass);
    FunctionPtr disulfideFunction = predictor->createDisulfideSymmetricResiduePairVectorPerception();
    disulfideFunction->initialize(context, proteinPerception->getOutputType());

    const TypePtr pairType = pairClass(disulfideFunction->getOutputType(), probabilityType);
    VectorPtr res = vector(pairType);
    const size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      const ProteinPtr inputProtein = proteins->getElement(i).getObjectAndCast<Pair>()->getFirst().getObjectAndCast<Protein>();
      const ProteinPtr supervisionProtein = proteins->getElement(i).getObjectAndCast<Pair>()->getSecond().getObjectAndCast<Protein>();

      Variable perception = proteinPerception->compute(context, inputProtein);
      SymmetricMatrixPtr featuresVector = disulfideFunction->compute(context, perception).getObjectAndCast<SymmetricMatrix>();
      SymmetricMatrixPtr supervision = supervisionProtein->getDisulfideBonds(context);
      jassert(featuresVector && supervision && featuresVector->getDimension() == supervision->getDimension());

      const size_t dimension = featuresVector->getDimension();
      for (size_t j = 0; j < dimension; ++j)
        for (size_t k = j + 1; k < dimension; ++k)
          res->append(new Pair(pairType, featuresVector->getElement(j,k), supervision->getElement(j,k)));
    }
    return res;
  }

  ContainerPtr addPredictions(ExecutionContext& context, const ContainerPtr& proteins, const ContainerPtr& predictions) const
  {
    size_t currentIndex = 0;
    VectorPtr res = vector(proteinClass);
    const size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ProteinPtr inputProtein = proteins->getElement(i).getObjectAndCast<Pair>()->getFirst().getObjectAndCast<Protein>();
      const ProteinPtr supervisionProtein = proteins->getElement(i).getObjectAndCast<Pair>()->getSecond().getObjectAndCast<Protein>();
      
      const size_t dimension = inputProtein->getCysteinIndices().size();
      jassert(currentIndex + dimension <= predictions->getNumElements()); 
      DoubleSymmetricMatrixPtr prediction = new DoubleSymmetricMatrix(probabilityType, dimension, 0.f);
      for (size_t j = 0; j < dimension; ++j)
        for (size_t k = j + 1; k < dimension; ++k)
          prediction->setValue(j, k, predictions->getElement(currentIndex++).getDouble());
      inputProtein->setDisulfideBonds(prediction);
      res->append(inputProtein);
    }
    jassert(res->getNumElements() == proteins->getNumElements());
    jassert(currentIndex == predictions->getNumElements());
    return res;
  }

  ProteinEvaluatorPtr createProteinEvaluator() const
  {
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    
    // TODO Add CBS evaluator
    evaluator->addEvaluator(cbsTarget, containerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore)), T("CBS"));
    evaluator->addEvaluator(cbsTarget, containerSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationAccuracyScore, true)), T("CBS Tuned Q2"));
    evaluator->addEvaluator(cbsTarget, containerSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationSensitivityAndSpecificityScore, false)), T("CBS Tuned S&S"));
    
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore)), T("DSB Q2"));
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationAccuracyScore, true)), T("DSB Tuned Q2"));
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationSensitivityAndSpecificityScore, false)), T("DSB Tuned S&S"));
    
    evaluator->addEvaluator(dsbTarget, new DisulfidePatternEvaluator(), T("DSB QP"));
    evaluator->addEvaluator(dsbTarget, new DisulfidePatternEvaluator(new KolmogorovPerfectMatchingFunction(0.f), 0.f), T("DSB QP Perfect"), true);
    
    return evaluator;
  }
};

class DSBLearnerFunction : public Function
{
public:
  DSBLearnerFunction(const String& inputDirectory, const String& supervisionDirectory, bool isValidation = false)
    : inputDirectory(inputDirectory), supervisionDirectory(supervisionDirectory), isValidation(isValidation) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return largeProteinParametersClass;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return scalarVariableMeanAndVarianceClass;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ContainerPtr train = Protein::loadProteinsFromDirectoryPair(context, context.getFile(inputDirectory).getChildFile(T("train/")), context.getFile(supervisionDirectory).getChildFile(T("train/")), 0, T("Loading proteins"));
    ScalarVariableMeanAndVariancePtr res = new ScalarVariableMeanAndVariance();

    if (isValidation)
    {
      ContainerPtr test = Protein::loadProteinsFromDirectoryPair(context, context.getFile(inputDirectory).getChildFile(T("test/")), context.getFile(supervisionDirectory).getChildFile(T("test/")), 0, T("Loading proteins"));
      res->push(computeFold(context, input, train, test));
    }
    else
    {
      for (size_t i = 0; i < 10; ++i)
        res->push(computeFold(context, input, train->invFold(i, 10), train->fold(i, 10)));
    }

    return res;
  }

  double computeFold(ExecutionContext& context, const Variable& input, const ContainerPtr& train, const ContainerPtr& test) const
  {
    ProteinSequentialPredictorPtr iterations = new ProteinSequentialPredictor();
    // CBS
    /*
    LargeProteinParametersPtr cbsParameter = new LargeProteinParameters();
    cbsParameter->pssmLocalHistogramSize = 75;
    cbsParameter->saLocalHistogramSize = 10;
    cbsParameter->useSAGlobalHistogram = true;
    LargeProteinPredictorParametersPtr cbsPredictor = new LargeProteinPredictorParameters(cbsParameter);
    cbsPredictor->learningMachineName = T("ExtraTrees");
    cbsPredictor->x3Trees = 1000;
    cbsPredictor->x3Attributes = 0;
    cbsPredictor->x3Splits = 1;
    ProteinPredictorPtr cbsIteration = new ProteinPredictor(cbsPredictor);
    cbsIteration->addTarget(cbsTarget);
    iterations->addPredictor(cbsIteration);
    */
    // DSB
    LargeProteinParametersPtr dsbParameter = input.getObjectAndCast<LargeProteinParameters>(context);
    LargeProteinPredictorParametersPtr dsbPredictor = new LargeProteinPredictorParameters(dsbParameter);
    dsbPredictor->learningMachineName = T("ExtraTrees");
    dsbPredictor->x3Trees = 1000;
    dsbPredictor->x3Attributes = 0;
    dsbPredictor->x3Splits = 1;
    ProteinPredictorPtr dsbIteration = new ProteinPredictor(dsbPredictor);
    dsbIteration->addTarget(dsbTarget);
    iterations->addPredictor(dsbIteration);

    // Copy CBS
    copyCysteineBondingStateSupervisons(context, train);
    copyCysteineBondingStateSupervisons(context, test);    
    
    if (!iterations->train(context, train, ContainerPtr(), T("Training")))
      return 101.f;

    ProteinEvaluatorPtr evaluator = createProteinEvaluator();
    CompositeScoreObjectPtr scores = iterations->evaluate(context, test, evaluator, T("EvaluateTest"));
    return evaluator->getScoreToMinimize(scores);
  }

  void copyCysteineBondingStateSupervisons(ExecutionContext& context, const ContainerPtr& proteins) const
  {
    for (size_t i = 0; i < proteins->getNumElements(); ++i)
      proteins->getElement(i).dynamicCast<Pair>()->getFirst().getObjectAndCast<Protein>()->setCysteinBondingStates(context, proteins->getElement(i).dynamicCast<Pair>()->getSecond().getObjectAndCast<Protein>()->getCysteinBondingStates(context));
  }

protected:
  friend class DSBLearnerFunctionClass;

  String inputDirectory;
  String supervisionDirectory;
  bool isValidation;

  DSBLearnerFunction() {}

  ProteinEvaluatorPtr createProteinEvaluator() const
  {
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    //evaluator->addEvaluator(cbsTarget, containerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore)), T("CBS"), true);
    evaluator->addEvaluator(dsbTarget, new DisulfidePatternEvaluator(new KolmogorovPerfectMatchingFunction(0.f), 0.f), T("DSB QP Perfect"), true);
    return evaluator;
  }
};

class BFSOptimizeDSBWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ExecutionContextPtr remoteContext = distributedExecutionContext(context, T("monster24.montefiore.ulg.ac.be"), 1664,
                                                                    T("120725-BFS-TrueCBS-DSB"), T("jbecker@screen"), T("jbecker@giga"),
                                                                    fixedResourceEstimator(1, 12 * 1024, 100), false);
    LargeProteinParametersPtr initialParameters = new LargeProteinParameters();
    OptimizationProblemPtr problem = new OptimizationProblem(new DSBLearnerFunction(inputDirectory, supervisionDirectory),
                                                             initialParameters, SamplerPtr(),
                                                             new DSBLearnerFunction(inputDirectory, supervisionDirectory, true));
    OptimizerPtr optimizer = bestFirstSearchOptimizer(LargeProteinParameters::createStreams(), context.getFile(optimizerStateFile));

    return optimizer->compute(*remoteContext.get(), problem);
  }

protected:
  friend class BFSOptimizeDSBWorkUnitClass;

  String inputDirectory;
  String supervisionDirectory;
  String optimizerStateFile;
};

class BFSDebugWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    FunctionPtr f = new DSBLearnerFunction(T("/Users/jbecker/Documents/Workspace/Data/Proteins/SPX/FromESANN/5FCV_Fold_0"),
                                           T("/Users/jbecker/Documents/Workspace/Data/Proteins/SPX/FromBoth/5FCV_Fold_0"));
    LargeProteinParametersPtr p = new LargeProteinParameters();
    p->useRelativePosition = true;

    std::cout << p->toString() << std::endl;

    return f->compute(context, p);
  }
};

};
