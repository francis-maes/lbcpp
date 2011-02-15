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

class GradientDescentOnlineLearner : public OnlineLearner, public FunctionCallback
{
public:
  GradientDescentOnlineLearner(const IterationFunctionPtr& learningRate, bool normalizeLearningRate)
    : numberOfActiveFeatures(T("NumActiveFeatures"), 100), 
      learningRate(learningRate), normalizeLearningRate(normalizeLearningRate), epoch(0) {}
  GradientDescentOnlineLearner() : normalizeLearningRate(true), epoch(0) {}

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
  friend class GradientDescentOnlineLearnerClass;

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

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_GRADIENT_DESCENT_ONLINE_LEARNER_H_
