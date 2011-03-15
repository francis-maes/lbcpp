/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentOnlineLearner.h | Gradient Descent Online Learner |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 19:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_GRADIENT_DESCENT_ONLINE_LEARNER_H_
# define LBCPP_LEARNING_NUMERICAL_GRADIENT_DESCENT_ONLINE_LEARNER_H_

# include <lbcpp/Learning/OnlineLearner.h>
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/Learning/LossFunction.h>
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Function/IterationFunction.h>
# include "../../Core/Function/MapContainerFunction.h"

namespace lbcpp
{

class GradientDescentOnlineLearner;
typedef ReferenceCountedObjectPtr<GradientDescentOnlineLearner> GradientDescentOnlineLearnerPtr;

class GradientDescentOnlineLearner : public OnlineLearner, public FunctionCallback
{
public:
  GradientDescentOnlineLearner(const FunctionPtr& lossFunction, const IterationFunctionPtr& learningRate, bool normalizeLearningRate)
    : context(NULL), maxIterations(0), numberOfActiveFeatures(T("NumActiveFeatures"), 100), 
      lossFunction(lossFunction), learningRate(learningRate), normalizeLearningRate(normalizeLearningRate), epoch(0), failure(false) {}
  GradientDescentOnlineLearner() : context(NULL), maxIterations(0), normalizeLearningRate(true), epoch(0), failure(false) {}

  virtual bool startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
  {
    numberOfActiveFeatures.clear();
    lossValue.clear();
    epoch = 0;
    this->context = &context;
    this->function = function;
    this->maxIterations = maxIterations;
    failure = false;

    jassert(function->getNumInputs() >= 2);
    TypePtr supervisionType = function->getInputVariable(1)->getType();
    if (!lossFunction)
    {
      // create default loss function
      if (supervisionType == booleanType || supervisionType == probabilityType)
        lossFunction = hingeDiscriminativeLossFunction();
      else if (supervisionType == doubleType)
        lossFunction = squareRegressionLossFunction();
      else if (supervisionType->inheritsFrom(enumValueType) || supervisionType->inheritsFrom(doubleVectorClass(enumValueType, probabilityType)))
        lossFunction = oneAgainstAllMultiClassLossFunction(hingeDiscriminativeLossFunction());
      else if (supervisionType->inheritsFrom(denseDoubleVectorClass(positiveIntegerEnumerationEnumeration)))
        lossFunction = allPairsRankingLossFunction(hingeDiscriminativeLossFunction());
      else
      {
        context.errorCallback(T("Could not create default loss function for type ") + supervisionType->getName());
        return false;
      }
    }
    return lossFunction->initialize(context, function->getOutputType(), supervisionType);
  }

  virtual void learningStep(const Variable* inputs, const Variable& output) = 0;

  virtual void functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output)
  {
    if (!inputs[1].exists())
      return; // no supervision

    jassert(function == this->function);
    learningStep(inputs, output);
    updateMeanInputsSize(inputs);
  }

  virtual void startLearningIteration(size_t iteration)
    {function->addPostCallback(this);}

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
  {
    function->removePostCallback(this);
    bool isLearningFinished = false;
    if (!lossValue.getCount())
    {
      objectiveValueToMinimize = 0.0;
      // no examples
      //context->errorCallback(T("Learning failed: no examples"));
      //failure = true;
      isLearningFinished = true;
      return true;
    }

    DoubleVectorPtr parameters = function->getParameters();
    double l2norm = (parameters ? parameters->l2norm() : 0.0);

    double mean = lossValue.getMean();
    context->resultCallback(T("Empirical Risk"), mean);
    context->resultCallback(T("Mean Active Features"), numberOfActiveFeatures.getMean());
    context->resultCallback(T("Num Params"), (parameters ? parameters->l0norm() : 0));
    context->resultCallback(T("Params Norm"), l2norm);
    context->resultCallback(T("Epoch"), epoch);
    lossValue.clear();

    if (!isNumberValid(l2norm))
    {
      context->errorCallback(T("Learning failed: parameters have diverged"));
      failure = true;
      return true;
    }

    if (objectiveValueToMinimize == DBL_MAX)
      objectiveValueToMinimize = mean;

    if (mean == 0.0)
      isLearningFinished = true;

    return failure || isLearningFinished;
  }

  virtual void finishLearning()
    {function = NumericalLearnableFunctionPtr();}

/*
  void addComputedGradient(const NumericalLearnableFunctionPtr& function, const DoubleVectorPtr& gradient, double lossValue)
  {
    double weight = -computeLearningRate();
    jassert(gradient);
    function->addGradient(Variable(), gradient, function->getParameters(), weight);
    ++epoch;
    updateNumberOfActiveFeatures(gradient);
    this->lossValue.push(lossValue);
  }*/

protected:
  friend class GradientDescentOnlineLearnerClass;

  ExecutionContext* context;
  size_t maxIterations;
  NumericalLearnableFunctionPtr function;

  ScalarVariableRecentMean numberOfActiveFeatures;
  ScalarVariableMean lossValue;
  FunctionPtr lossFunction;
  IterationFunctionPtr learningRate;
  bool normalizeLearningRate;
  size_t epoch;
  bool failure;

  void gradientDescentStep(const DoubleVectorPtr& gradient, double weight = 1.0)
  {
    DoubleVectorPtr parameters = function->getOrCreateParameters();
    gradient->addWeightedTo(parameters, 0, -computeLearningRate() * weight);
  }

  void computeAndAddGradient(const Variable* inputs, const Variable& prediction, DoubleVectorPtr& target, double weight)
  {
    if (failure || !inputs[1].exists())
      return; // failed or no supervision

    double exampleLossValue = 0.0;

    Variable lossGradient;
    if (!function->computeLoss(lossFunction, inputs, prediction, exampleLossValue, lossGradient))
    {
      context->errorCallback(T("Learning failed: could not compute loss gradient"));
      failure = true;
      return;
    }
    if (!target)
      target = function->createParameters();
    function->addGradient(lossGradient, inputs, target, weight);
    ++epoch;
    lossValue.push(exampleLossValue);
  }

  double computeLearningRate() const
  {
    double res = 1.0;
    if (learningRate)
      res *= learningRate->computeIterationFunction(epoch);
    if (normalizeLearningRate && numberOfActiveFeatures.getMean())
      res /= numberOfActiveFeatures.getMean();
    jassert(isNumberValid(res));
    return res;
  }

  void updateMeanInputsSize(const Variable* inputs)
  {
    if (normalizeLearningRate)
    {
      // computing the inputsSize may be long, so we make more and more sparse sampling of this quantity
      if (!numberOfActiveFeatures.isMemoryFull() || (epoch % 20 == 0))
      {
        double norm = function->getInputsSize(inputs);
        if (norm)
          numberOfActiveFeatures.push(norm);
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_GRADIENT_DESCENT_ONLINE_LEARNER_H_
