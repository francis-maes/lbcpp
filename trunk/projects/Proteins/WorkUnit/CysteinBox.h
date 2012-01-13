
#include <lbcpp/Core/Function.h>
#include "../Predictor/DecoratorProteinPredictorParameters.h"
#include "../Data/Formats/FASTAFileParser.h"

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
      svmC(4.0), svmGamma(1.0),
      x3Trees(1000), x3Attributes(0), x3Splits(1),
      windowSize(40) {}

  DisulfideBondWorkUnit(double svmC, double svmGamma)
    : learningMachineName(T("LibSVM")), svmC(svmC), svmGamma(svmGamma), windowSize(40) {}

  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr train = Protein::loadProteinsFromDirectoryPair(context, context.getFile(inputDirectory).getChildFile(T("train")), context.getFile(supervisionDirectory).getChildFile(T("train")), 0, T("Loading training proteins"));
    ContainerPtr test = Protein::loadProteinsFromDirectoryPair(context, context.getFile(inputDirectory).getChildFile(T("test")), context.getFile(supervisionDirectory).getChildFile(T("test")), 0, T("Loading testing proteins"));

    if (!train || !test || train->getNumElements() == 0 || test->getNumElements() == 0)
    {
      context.errorCallback(T("No training or testing proteins !"));
      return 100;
    }

    LargeProteinParametersPtr parameter;
    if (parameterFile == File::nonexistent)
      parameter = LargeProteinParameters::createTestObject(windowSize);
    else
    {
      parameter = LargeProteinParameters::createFromFile(context, parameterFile).dynamicCast<LargeProteinParameters>();
      if (!parameter)
      {
        context.errorCallback(T("Invalid LargeProteinParameters file !"));
        return 101;
      }
    }
/*
    context.warningCallback(T("Parameter set to lin09"));
    parameter = new LargeProteinParameters();
    parameter->usePositionDifference = true;
    parameter->useIndexDifference = true;
    parameter->pssmWindowSize = 23;
    parameter->ss3WindowSize = 1;
*/
    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);
    predictor->learningMachineName = learningMachineName;
    // Config ExtraTrees
    predictor->x3Trees = x3Trees;
    predictor->x3Attributes = x3Attributes;
    predictor->x3Splits = x3Splits;
    // Config kNN
    predictor->knnNeighbors = 5;
    // Config LibSVM
    predictor->svmC = svmC;
    predictor->svmGamma = svmGamma;

    ProteinPredictorPtr iteration = new ProteinPredictor(predictor);
    iteration->addTarget(dsbTarget);

    if (!iteration->train(context, train, ContainerPtr(), T("Training")))
      return Variable::missingValue(doubleType);

    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    evaluator->addEvaluator(cbsTarget, containerSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationSensitivityAndSpecificityScore, true)), T("Cystein Bonding States (Sens. & Spec.)"));
    evaluator->addEvaluator(dsbTarget, new DisulfidePatternEvaluator(new GreedyDisulfidePatternBuilder(6, 0.0), 0.0), T("Disulfide Bonds (Greedy L=6)")); 

    CompositeScoreObjectPtr scores = iteration->evaluate(context, test, evaluator, T("Evaluate on test proteins"));
    return evaluator->getScoreObjectOfTarget(scores, dsbTarget)->getScoreToMinimize();
  }

protected:
  friend class DisulfideBondWorkUnitClass;

  String inputDirectory;
  String supervisionDirectory;
  File parameterFile;
  String learningMachineName;
  double svmC;
  double svmGamma;
  size_t x3Trees;
  size_t x3Attributes;
  size_t x3Splits;
  size_t windowSize;
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

    context.informationCallback(T("Output file name: ") + outputPrefix + T(".train.arff"));

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
      *o << "@ATTRIBUTE " << enumeration->getElement(i)->getName() << " NUMERIC\n";
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
      ContainerPtr cbp = Protein::createEmptyCysteinBondingProperty();
      cbp->setElement(isPositiveExamples ? all : none, probability(1.0));
      protein->setCysteinBondingProperty(cbp);
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

};
