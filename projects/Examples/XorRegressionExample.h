/*-----------------------------------------.---------------------------------.
| Filename: XorRegressionExample.h         | Illustrates a simple regression |
| Author  : Francis Maes                   |                                 |
| Started : 18/10/2010 19:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXAMPLES_XOR_REGRESSION_H_
# define LBCPP_EXAMPLES_XOR_REGRESSION_H_

# include <lbcpp/Perception/Perception.h>
# include <lbcpp/Inference/Inference.h>
# include <lbcpp/Function/StoppingCriterion.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/NumericalLearning/NumericalLearning.h>
# include <lbcpp/Core/Frame.h>
# include <lbcpp/Data/RandomGenerator.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

class XorFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleType;}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration(T("xor-features"));
    res->addElement(context, T("unit"));
    res->addElement(context, T("x1"));
    res->addElement(context, T("x2"));
    res->addElement(context, T("x1.x2"));
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    double x1 = inputs[0].getDouble();
    double x2 = inputs[1].getDouble();
    callback.sense(0, 1.0);
    callback.sense(1, x1);
    callback.sense(2, x2);
    callback.sense(3, x1 * x2);
  }
};

///////////////////////////////////////////////////////////////

// Function, Container[VariableVector], optional Container[VariableVector] -> Nil
// LearnableFunction, Container[VariableVector], optional Container[VariableVector] -> Parameters
class Learner : public Function
{
public:
  Learner()
    {numInputs = 3;} // tmp: necessaire tant qu'on ne sait pas trop d'ou appeler le Function::initialize

  virtual TypePtr getRequiredFunctionType() const
    {return functionClass;}

  virtual TypePtr getRequiredExamplesType() const
    {return objectClass;}

  // Function
  virtual size_t getMinimumNumRequiredInputs() const
    {return 2;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return 3;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? functionClass : objectVectorClass(getRequiredExamplesType());}

  virtual String getOutputPostFix() const
    {return T("Learned");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return nilType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const FunctionPtr& function = inputs[0].getObjectAndCast<Function>();
    const ObjectVectorPtr& trainingData = inputs[1].getObjectAndCast<ObjectVector>();
    ObjectVectorPtr validationData = (getNumInputs() == 3 ? inputs[2].getObjectAndCast<ObjectVector>() : ObjectVectorPtr());
    train(context, function, trainingData->getObjects(), validationData ? validationData->getObjects() : std::vector<ObjectPtr>());
    return Variable();
  }

  virtual void train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const = 0;
};

typedef ReferenceCountedObjectPtr<Learner> LearnerPtr;

class FrameBasedFunctionSequentialLearner : public Learner
{
public:
  virtual TypePtr getRequiredExamplesType() const
    {return objectClass;} // frameObjectClass

  virtual void train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const FrameBasedFunctionPtr& function = f.staticCast<FrameBasedFunction>();
    FrameClassPtr frameClass = function->getFrameClass();

    // make initial frames
    std::vector<FramePtr> trainingFrames;
    makeInitialFrames(trainingData, trainingFrames);
    std::vector<FramePtr> validationFrames;
    if (validationData.size())
      makeInitialFrames(validationData, validationFrames);

    // for each frame operator:
    for (size_t i = 0; i < frameClass->getNumMemberVariables(); ++i)
    {
      FrameOperatorSignaturePtr signature = frameClass->getMemberVariable(i).dynamicCast<FrameOperatorSignature>();
      if (signature)
      {
        if (signature->getFunction()->hasBatchLearner())
          learnSubFunction(context, frameClass, i, signature, trainingFrames, validationFrames);
        if (trainingFrames.size())
          computeSubFunction(i, trainingFrames);
        if (validationFrames.size())
          computeSubFunction(i, validationFrames);
      }
    }
  }

protected:
  void makeInitialFrames(const std::vector<ObjectPtr>& data, std::vector<FramePtr>& res) const
  {
    size_t n = data.size();
    res.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
      res[i] = data[i].staticCast<Frame>();
      jassert(res[i]);
    }
  }

  ObjectVectorPtr makeSubFrames(const FrameClassPtr& frameClass, const std::vector<FramePtr>& frames, size_t variableIndex) const
  {
    FrameOperatorSignaturePtr signature = frameClass->getMemberVariable(variableIndex).staticCast<FrameOperatorSignature>();
    const FunctionPtr& function = signature->getFunction();
    const FrameClassPtr& subFrameClass = function->getFrameClass();

    size_t n = frames.size();
    size_t numInputs = function->getNumInputs();
    jassert(numInputs == signature->getNumInputs());
    ObjectVectorPtr res = new ObjectVector(subFrameClass, n);

    std::vector<ObjectPtr>& subFrames = res->getObjects();
    for (size_t i = 0; i < n; ++i)
    {
      const FramePtr& frame = frames[i];
      FramePtr subFrame(new Frame(subFrameClass));
      for (size_t j = 0; j < numInputs; ++j)
        subFrame->setVariable(j, frame->getOrComputeVariable(signature->getInputIndex(j)));
      subFrames[i] = subFrame;
    }
    return res;
  }

  void learnSubFunction(ExecutionContext& context, const FrameClassPtr& frameClass, size_t variableIndex, const FrameOperatorSignaturePtr& signature, const std::vector<FramePtr>& trainingFrames, const std::vector<FramePtr>& validationFrames) const
  {
    const FunctionPtr& function = signature->getFunction();
    ObjectVectorPtr subTrainingData = makeSubFrames(frameClass, trainingFrames, variableIndex);
    ObjectVectorPtr subValidationData;
    if (validationFrames.size())
      subValidationData = makeSubFrames(frameClass, validationFrames, variableIndex);
    function->train(context, subTrainingData, subValidationData);
  }

  void computeSubFunction(size_t variableIndex, std::vector<FramePtr>& frames) const
  {
    for (size_t i = 0; i < frames.size(); ++i)
      frames[i]->getOrComputeVariable(variableIndex);
  }
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class StochasticLearner : public Learner
{
public:
  StochasticLearner(const std::vector<FunctionPtr>& functionsToLearn, EvaluatorPtr evaluator,
                    size_t maxIterations = 1000,
                    StoppingCriterionPtr stoppingCriterion = StoppingCriterionPtr(),
                    bool randomizeExamples = true,
                    bool restoreBestParametersWhenDone = true)
    : functionsToLearn(functionsToLearn), evaluator(evaluator), maxIterations(maxIterations),
      randomizeExamples(randomizeExamples), restoreBestParametersWhenDone(restoreBestParametersWhenDone) {}

  virtual TypePtr getRequiredExamplesType() const
    {return objectClass;} // frameObjectClass

  virtual void train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    RunningLearnerVector runningLearners;
    startLearning(function, runningLearners);

    if (stoppingCriterion)
      stoppingCriterion->reset();
    double bestMainScore = -DBL_MAX;
    for (size_t i = 0; runningLearners.size() && (!maxIterations || i < maxIterations); ++i)
    {
      context.enterScope(T("Learning Iteration ") + String((int)i + 1));
      context.resultCallback(T("Iteration"), i + 1);

      // Learning iteration
      startLearningIteration(function, i, maxIterations, runningLearners);

      if (randomizeExamples)
      {
        std::vector<size_t> order;
        RandomGenerator::getInstance()->sampleOrder(trainingData.size(), order);
        for (size_t i = 0; i < order.size(); ++i)
          doEpisode(context, function, trainingData[order[i]], runningLearners);
      }
      else
        for (size_t i = 0; i < trainingData.size(); ++i)
          doEpisode(context, function, trainingData[i], runningLearners);

      bool learningFinished = finishLearningIteration(context, runningLearners);

      // Evaluation
      EvaluatorPtr trainEvaluator = evaluator->cloneAndCast<Evaluator>();
      function->evaluate(context, trainingData, trainEvaluator);
      returnEvaluatorResults(context, trainEvaluator, T("Train"));
      EvaluatorPtr validationEvaluator;
      if (validationData.size())
      {
        validationEvaluator = evaluator->cloneAndCast<Evaluator>();
        function->evaluate(context, validationData, validationEvaluator);
        returnEvaluatorResults(context, validationEvaluator, T("Validation"));
      }

      double mainScore = validationEvaluator ? validationEvaluator->getDefaultScore() : trainEvaluator->getDefaultScore();
      
      context.leaveScope(mainScore);
      context.progressCallback(new ProgressionState(i + 1, maxIterations, T("Learning Iterations")));

      if (mainScore > bestMainScore)
      {
        if (restoreBestParametersWhenDone)
          storeParameters(function);
        bestMainScore = mainScore;
        context.informationCallback(T("Best score: ") + String(mainScore));
      }

      if (learningFinished || (stoppingCriterion && stoppingCriterion->shouldStop(mainScore)))
        break;
    }

    finishLearning();
    
    if (restoreBestParametersWhenDone)
      restoreParameters(function);
  }

  void returnEvaluatorResults(ExecutionContext& context, EvaluatorPtr evaluator, const String& name) const
  {
    std::vector< std::pair<String, double> > results;
    evaluator->getScores(results);
    for (size_t i = 0; i < results.size(); ++i)
      context.resultCallback(name + T(" ") + results[i].first, results[i].second);
  }

protected:
  typedef std::vector< std::pair<FunctionPtr, OnlineLearnerPtr> > RunningLearnerVector;

  std::vector<FunctionPtr> functionsToLearn;
  EvaluatorPtr evaluator;
  size_t maxIterations;
  StoppingCriterionPtr stoppingCriterion;
  bool randomizeExamples;
  bool restoreBestParametersWhenDone;

  void doEpisode(ExecutionContext& context, const FunctionPtr& function, const ObjectPtr& inputs, const RunningLearnerVector& runningLearners) const
  {
    std::vector<Variable> in(function->getNumInputs());
    for (size_t i = 0; i < in.size(); ++i)
      in[i] = inputs->getVariable(i);

    startEpisode(runningLearners);
    function->compute(context, &in[0]);
    finishEpisode(runningLearners);
  }

  void storeParameters(const FunctionPtr& function) const
  {
    // FIXME
  }

  void restoreParameters(const FunctionPtr& function) const
  {
    // FIXME
  }

private:
  void startLearning(const FunctionPtr& function, RunningLearnerVector& runningLearners) const
  {
    for (size_t i = 0; i < functionsToLearn.size(); ++i)
    {
      OnlineLearnerPtr onlineLearner = functionsToLearn[i]->getOnlineLearner();
      jassert(onlineLearner);
      onlineLearner->startLearning(function);
      runningLearners.push_back(std::make_pair(functionsToLearn[i], onlineLearner));
    }
  }

  void finishLearning() const
  {
    for (int i = functionsToLearn.size() - 1; i >= 0; --i)
      functionsToLearn[i]->getOnlineLearner()->finishLearning();
  }

  static void startLearningIteration(const FunctionPtr& function, size_t iteration, size_t maxIterations, const RunningLearnerVector& runningLearners)
  {
    for (size_t i = 0; i < runningLearners.size(); ++i)
      runningLearners[i].second->startLearningIteration(runningLearners[i].first, iteration, maxIterations);
  }

  static bool finishLearningIteration(ExecutionContext& context, RunningLearnerVector& runningLearners)
  {
    bool learningFinished = true;
    for (int i = runningLearners.size() - 1; i >= 0; --i)
    {
      bool learnerFinished = runningLearners[i].second->finishLearningIteration(context, runningLearners[i].first);
      if (learnerFinished)
        runningLearners.erase(runningLearners.begin() + i);
      else
        learningFinished = false;
    }
    return learningFinished;
  }

  static void startEpisode(const RunningLearnerVector& runningLearners)
  {
    for (size_t i = 0; i < runningLearners.size(); ++i)
      runningLearners[i].second->startEpisode(runningLearners[i].first);
  }

  static void finishEpisode(const RunningLearnerVector& runningLearners)
  {
    for (int i = runningLearners.size() - 1; i >= 0; --i)
      runningLearners[i].second->finishEpisode(runningLearners[i].first);
  }
};

///////////////////////////////////////////////////////////////

class LearnableFunction : public Function
{
public:
  const ClassPtr& getParametersClass() const
    {return parametersClass;}

  const ObjectPtr& getParameters() const
    {return parameters;}

  ObjectPtr& getParameters()
    {return parameters;}

protected:
  friend class LearnableFunctionClass;

  //CriticalSection parametersLock;
  ObjectPtr parameters;
  ClassPtr parametersClass;
};

class NumericalLearnableFunction : public LearnableFunction
{
public:
  const DoubleVectorPtr& getParameters() const
    {return parameters.staticCast<DoubleVector>();}

  DoubleVectorPtr& getParameters()
    {return *(DoubleVectorPtr* )&parameters;}

  // returns false if no supervision is available
  virtual bool computeAndAddGradient(const Variable* inputs, const Variable& output, double& exampleLossValue, DoubleVectorPtr& target, double weight) const = 0;
};

typedef ReferenceCountedObjectPtr<NumericalLearnableFunction> NumericalLearnableFunctionPtr;

/////////////////

class GDOnlineLearner : public OnlineLearner, public FunctionCallback
{
public:
  GDOnlineLearner(const IterationFunctionPtr& learningRate, bool normalizeLearningRate)
    : numberOfActiveFeatures(T("NumActiveFeatures"), 100), 
      learningRate(learningRate), normalizeLearningRate(normalizeLearningRate), epoch(0) {}
  GDOnlineLearner() : normalizeLearningRate(true), epoch(0) {}

  virtual void startLearning(const FunctionPtr& function)
  {
    numberOfActiveFeatures.clear();
    lossValue.clear();
    epoch = 0;
  }

  virtual void startLearningIteration(const FunctionPtr& function, size_t iteration, size_t maxIterations)
    {function->addPostCallback(this);}

  virtual void functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output)
  {
    learningStep(function, inputs, output);
    
   /* if (inputs[0].isObject() && inputs[0].dynamicCast<Container>())
    {
      // composite inputs (e.g. ranking)
      const ContainerPtr& inputContainer = inputs[0].getObjectAndCast<Container>(context);
      size_t n = inputContainer->getNumElements();
      for (size_t i = 0; i < n; ++i)
        updateNumberOfActiveFeatures(context, inputContainer->getElement(i).getObjectAndCast<DoubleVector>());
    }
    else*/
    {
      // simple input
      updateNumberOfActiveFeatures(inputs[0].getObjectAndCast<DoubleVector>());
    }
  }

  virtual bool finishLearningIteration(ExecutionContext& context, const FunctionPtr& function)
  {
    function->removePostCallback(this);

    bool isLearningFinished = false;
    if (lossValue.getCount())
    {
      double mean = lossValue.getMean();
      context.resultCallback(T("Empirical Risk"), mean);
      context.resultCallback(T("Mean Active Features"), numberOfActiveFeatures.getMean());
      lossValue.clear();

      if (mean == 0.0)
        isLearningFinished = true;
    }
    return isLearningFinished;
  }

protected:
  ScalarVariableRecentMean numberOfActiveFeatures;
  ScalarVariableMean lossValue;
  IterationFunctionPtr learningRate;
  bool normalizeLearningRate;
  size_t epoch;

  void gradientDescentStep(NumericalLearnableFunctionPtr& function, const DoubleVectorPtr& gradient, double weight = 1.0)
  {
    DoubleVectorPtr& parameters = function->getParameters();
    if (!parameters)
      parameters = new DenseDoubleVector(function->getParametersClass());
    gradient->addWeightedTo(parameters, 0, -computeLearningRate() * weight);
  }

  void computeAndAddGradient(const NumericalLearnableFunctionPtr& function, const Variable* inputs, const Variable& output, DoubleVectorPtr& target, double weight)
  {
     ++epoch;
    double exampleLossValue;
    if (function->computeAndAddGradient(inputs, output, exampleLossValue, target, weight))
      lossValue.push(exampleLossValue);
  }

  double computeLearningRate() const
  {
    double res = 1.0;
    if (learningRate)
      res *= learningRate->compute(epoch);
    if (normalizeLearningRate && numberOfActiveFeatures.getMean())
      res /= numberOfActiveFeatures.getMean();
    jassert(isNumberValid(res));
    return res;
  }

  void updateNumberOfActiveFeatures(const DoubleVectorPtr& input)
  {
    if (normalizeLearningRate && input)
    {
      // computing the l1norm() may be long, so we make more and more sparse sampling of this quantity
      if (!numberOfActiveFeatures.isMemoryFull() || (epoch % 20 == 0))
      {
        double norm = input->l0norm();
        if (norm)
          numberOfActiveFeatures.push(norm);
      }
    }
  }
};

class StochasticGDOnlineLearner : public GDOnlineLearner
{
public:
  StochasticGDOnlineLearner(const IterationFunctionPtr& learningRate, bool normalizeLearningRate = true)
    : GDOnlineLearner(learningRate, normalizeLearningRate) {}
  StochasticGDOnlineLearner() {}

  virtual void learningStep(const FunctionPtr& f, const Variable* inputs, const Variable& output)
  {
    const NumericalLearnableFunctionPtr& function = f.staticCast<NumericalLearnableFunction>();
    computeAndAddGradient(function, inputs, output, function->getParameters(), -computeLearningRate());
  }
};

class PerEpisodeGDOnlineLearner : public GDOnlineLearner
{
public:
  virtual void startEpisode(const FunctionPtr& f, const ObjectPtr& inputs)
  {
    const NumericalLearnableFunctionPtr& function = f.staticCast<NumericalLearnableFunction>();
    episodeGradient =  new DenseDoubleVector(function->getParametersClass());
  }

  virtual void learningStep(const FunctionPtr& f, const Variable* inputs, const Variable& output)
  {
    const NumericalLearnableFunctionPtr& function = f.staticCast<NumericalLearnableFunction>();
    DoubleVectorPtr& parameters = function->getParameters(); // FIXME: unused variable
    computeAndAddGradient(function, inputs, output, episodeGradient, 1.0);
    GDOnlineLearner::learningStep(f, inputs, output);
  }

  virtual void finishEpisode(const FunctionPtr& f)
  {
    NumericalLearnableFunctionPtr function = f.staticCast<NumericalLearnableFunction>();
    gradientDescentStep(function, episodeGradient);
  }

protected:
  DoubleVectorPtr episodeGradient;
};

/////////////////

// DoubleVector<T>, optional LossFunction -> Double
class LearnableLinearFunction : public NumericalLearnableFunction
{
public:
  DenseDoubleVectorPtr getParameters() const
    {return parameters.staticCast<DenseDoubleVector>();}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? functionClass : doubleVectorClass();}

  virtual String getOutputPostFix() const
    {return T("Prediction");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());
    parametersClass = denseDoubleVectorClass(featuresEnumeration);
    outputName = T("prediction");
    outputShortName = T("p");
    setOnlineLearner(new StochasticGDOnlineLearner(constantIterationFunction(0.1)));
    setBatchLearner(new StochasticLearner(std::vector<FunctionPtr>(1, refCountedPointerFromThis(this)), regressionErrorEvaluator(T("toto"))));
    return doubleType;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    DoubleVectorPtr inputVector = inputs[0].getObjectAndCast<DoubleVector>();
    //FunctionPtr supervision = inputs[1].getObjectAndCast<Function>();
    DenseDoubleVectorPtr parameters = getParameters();
    if (!parameters || !inputVector)
      return Variable::missingValue(doubleType);

    double res = inputVector->dotProduct(parameters);
    return isNumberValid(res) ? Variable(res) : Variable::missingValue(doubleType);
  }

  virtual bool computeAndAddGradient(const Variable* inputs, const Variable& prediction, double& exampleLossValue, DoubleVectorPtr& target, double weight) const
  {
    const DoubleVectorPtr& inputVector = inputs[0].getObjectAndCast<DoubleVector>();
    const FunctionPtr& supervision = inputs[1].getObjectAndCast<Function>();

    if (!supervision)
      return false;

    if (supervision.isInstanceOf<ScalarFunction>())
    {
      const ScalarFunctionPtr& loss = supervision.staticCast<ScalarFunction>();
      double lossDerivative;
      loss->compute(prediction.exists() ? prediction.getDouble() : 0.0, &exampleLossValue, &lossDerivative);
      if (!target)
        target = new DenseDoubleVector(parametersClass);
      inputVector->addWeightedTo(target, 0, weight * lossDerivative);
    }
    else
      jassert(false);

    return true;
  }
};


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class MakeRegressionLossFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleType;}

  virtual String getOutputPostFix() const
    {return T("Loss");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return scalarFunctionClass;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return squareLossFunction(inputs[0].getDouble());}
};

class LinearRegressor : public FrameBasedFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? doubleType : (TypePtr)doubleVectorClass();}

  virtual String getOutputPostFix() const
    {return T("Prediction");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());

    frameClass = new FrameClass(T("LinearRegressor"));
    frameClass->addMemberVariable(context, inputVariables[0]->getType(), T("input"));
    frameClass->addMemberVariable(context, inputVariables[1]->getType(), T("supervision"));
    frameClass->addMemberOperator(context, new MakeRegressionLossFunction(), 1);
    FunctionPtr linearFunction = new LearnableLinearFunction();
    frameClass->addMemberOperator(context, linearFunction, 0, 2);

    setBatchLearner(new StochasticLearner(std::vector<FunctionPtr>(1, linearFunction), regressionErrorEvaluator(T("toto"))));

//    setBatchLearner(new FrameBasedFunctionSequentialLearner());
    return FrameBasedFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
  }
};

class XorRegressionExample : public WorkUnit
{
public:
  FrameClassPtr createXorFrameClass(ExecutionContext& context)
  {
    FrameClassPtr res = new FrameClass(T("XorFrame"));
    res->addMemberVariable(context, doubleType, T("x1"));
    res->addMemberVariable(context, doubleType, T("x2"));
    res->addMemberVariable(context, doubleType, T("supervision"));
    res->addMemberOperator(context, new XorFeatureGenerator(), 0, 1, T("features"));
    res->addMemberOperator(context, new LinearRegressor(), 3, 2);
    return res;
  }

  FunctionPtr createXorFunction(ExecutionContext& context, FrameClassPtr xorFrameClass)
  {
    FunctionPtr xorFunction = new FrameBasedFunction(xorFrameClass);
    xorFunction->setBatchLearner(new FrameBasedFunctionSequentialLearner());
    xorFunction->initialize(context, std::vector<TypePtr>(3, doubleType));
    return xorFunction;
  }

  VectorPtr createTrainingExamples(FrameClassPtr frameClass) const
  {
    VectorPtr trainingExamples = vector(frameClass);
    trainingExamples->append(new Frame(frameClass, 0.0, 0.0, 1.0));
    trainingExamples->append(new Frame(frameClass, 1.0, 0.0, 0.0));
    trainingExamples->append(new Frame(frameClass, 0.0, 1.0, 0.0));
    trainingExamples->append(new Frame(frameClass, 1.0, 1.0, 1.0));
    return trainingExamples;
  }

  virtual Variable run(ExecutionContext& context)
  {
    FrameClassPtr xorFrameClass = createXorFrameClass(context);
    VectorPtr trainingExamples = createTrainingExamples(xorFrameClass);
    
    FunctionPtr xorFunction = createXorFunction(context, xorFrameClass);
    if (!xorFunction->train(context, trainingExamples))
      return false;

    // todo: learn and evaluate

#if 0
   // create linear regressor
    InferenceOnlineLearnerPtr learner = gradientDescentOnlineLearner(
            perStep, constantIterationFunction(0.1), true, // learning steps
            never, ScalarObjectFunctionPtr()); // regularizer
    learner->getLastLearner()->setNextLearner(stoppingCriterionOnlineLearner(maxIterationsStoppingCriterion(100), true)); // stopping criterion
    NumericalSupervisedInferencePtr regressor = squareRegressionInference(T("XOR-Regressor"), PerceptionPtr());
    regressor->setStochasticLearner(learner);

    TypePtr inputType = pairClass(doubleType, doubleType);

    FeatureGeneratorPtr featureGenerator = new XorFeatureGenerator();
    featureGenerator->initialize(context, inputType);
    InferencePtr inference = preProcessInference(regressor, featureGenerator);

    // make training set
    VectorPtr trainingSet = vector(pairClass(inputType, doubleType));
    
    trainingSet->append(Variable::pair(Variable::pair(0.0, 0.0), 1.0));
    trainingSet->append(Variable::pair(Variable::pair(1.0, 0.0), 0.0));
    trainingSet->append(Variable::pair(Variable::pair(0.0, 1.0), 0.0));
    trainingSet->append(Variable::pair(Variable::pair(1.0, 1.0), 1.0));

    // create context and train
    inference->train(context, trainingSet, ContainerPtr(), T("Training"));

#endif // 0

    // evaluate
    EvaluatorPtr evaluator = regressionErrorEvaluator(T("XOR-error"));
    xorFunction->evaluate(context, trainingExamples, evaluator);
    //std::cout << "Evaluation: " << evaluator->toString() << std::endl;

    Variable myPrediction = xorFunction->compute(context, 1.0, 0.0, Variable::missingValue(doubleType));
    context.resultCallback(T("prediction"), myPrediction);

    return Variable();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_XOR_REGRESSION_H_
