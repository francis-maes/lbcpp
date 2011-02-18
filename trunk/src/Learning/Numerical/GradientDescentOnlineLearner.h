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
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Function/IterationFunction.h>

namespace lbcpp
{

class GradientDescentOnlineLearner;
typedef ReferenceCountedObjectPtr<GradientDescentOnlineLearner> GradientDescentOnlineLearnerPtr;

class GradientDescentOnlineLearner : public OnlineLearner, public FunctionCallback
{
public:
  GradientDescentOnlineLearner(const FunctionPtr& lossFunction, const IterationFunctionPtr& learningRate, bool normalizeLearningRate)
    : context(NULL), maxIterations(0), numberOfActiveFeatures(T("NumActiveFeatures"), 100), 
      lossFunction(lossFunction), learningRate(learningRate), normalizeLearningRate(normalizeLearningRate), epoch(0) {}
  GradientDescentOnlineLearner() : context(NULL), maxIterations(0), normalizeLearningRate(true), epoch(0) {}

  virtual void startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
  {
    numberOfActiveFeatures.clear();
    lossValue.clear();
    epoch = 0;
    this->context = &context;
    this->function = function;
    this->maxIterations = maxIterations;
  }

  virtual void learningStep(const Variable* inputs, const Variable& output) = 0;

  virtual void functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output)
  {
    jassert(function == this->function);
    learningStep(inputs, output);
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

  virtual void startLearningIteration(size_t iteration)
    {function->addPostCallback(this);}

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
  {
    function->removePostCallback(this);
    bool isLearningFinished = false;
    if (lossValue.getCount())
    {
      double mean = lossValue.getMean();
      context->resultCallback(T("Empirical Risk"), mean);
      context->resultCallback(T("Mean Active Features"), numberOfActiveFeatures.getMean());
      context->resultCallback(T("Epoch"), epoch);
      
      lossValue.clear();

      if (objectiveValueToMinimize == DBL_MAX)
        objectiveValueToMinimize = mean;

      if (mean == 0.0)
        isLearningFinished = true;
    }
    return isLearningFinished;
  }

  virtual void finishLearning()
    {function = FunctionPtr();}

  const NumericalLearnableFunctionPtr& getNumericalLearnableFunction() const
    {return function.staticCast<NumericalLearnableFunction>();}

protected:
  friend class GradientDescentOnlineLearnerClass;

  ExecutionContext* context;
  size_t maxIterations;
  FunctionPtr function;

  ScalarVariableRecentMean numberOfActiveFeatures;
  ScalarVariableMean lossValue;
  FunctionPtr lossFunction;
  IterationFunctionPtr learningRate;
  bool normalizeLearningRate;
  size_t epoch;

  void gradientDescentStep(const NumericalLearnableFunctionPtr& function, const DoubleVectorPtr& gradient, double weight = 1.0)
  {
    DoubleVectorPtr& parameters = function->getParameters();
    if (!parameters)
      parameters = new DenseDoubleVector(function->getParametersClass());
    gradient->addWeightedTo(parameters, 0, -computeLearningRate() * weight);
  }

  void computeAndAddGradient(const NumericalLearnableFunctionPtr& function, const Variable* inputs, const Variable& output, DoubleVectorPtr& target, double weight)
  {
    ++epoch;
    double exampleLossValue = 0.0;
    if (function->computeAndAddGradient(lossFunction, inputs, output, exampleLossValue, target, weight))
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

  void updateNumberOfActiveFeatures(const DoubleVectorPtr& input)
  {
    if (normalizeLearningRate && input)
    {
      // computing the l0norm() may be long, so we make more and more sparse sampling of this quantity
      if (!numberOfActiveFeatures.isMemoryFull() || (epoch % 20 == 0))
      {
        double norm = input->l0norm();
        if (norm)
          numberOfActiveFeatures.push(norm);
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_GRADIENT_DESCENT_ONLINE_LEARNER_H_
