/*-----------------------------------------.---------------------------------.
| Filename: BinaryClassificationInference.h| Binary Classification Inference |
| Author  : Francis Maes                   |  classes                        |
| Started : 26/05/2010 16:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_INFERENCE_BINARY_CLASSIFICATION_H_
# define LBCPP_NUMERICAL_LEARNING_INFERENCE_BINARY_CLASSIFICATION_H_

# include <lbcpp/Inference/DecoratorInference.h>
# include <lbcpp/Inference/SequentialInference.h>
# include <lbcpp/NumericalLearning/LossFunctions.h>
# include "LinearInference.h"

namespace lbcpp
{

class BinaryClassificationInference : public StaticDecoratorInference
{
public:
  BinaryClassificationInference(const String& name, InferencePtr scoreInference, InferenceOnlineLearnerPtr onlineLearner = InferenceOnlineLearnerPtr())
    : StaticDecoratorInference(name, scoreInference)
  {
    if (onlineLearner)
    {
      scoreInference->setBatchLearner(stochasticNumericalInferenceLearner());
      scoreInference->addOnlineLearner(onlineLearner);
    }
  }
  BinaryClassificationInference() {}

  virtual ScalarFunctionPtr createLossFunction(bool isPositive) const = 0;

  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}

  virtual TypePtr getOutputType(TypePtr ) const
    {return probabilityType;}

  virtual void setName(const String& name)
  {
    DecoratorInference::setName(name); 
    decorated->setName(name + T(" score"));
  }

  virtual DecoratorInferenceStatePtr prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    ScalarFunctionPtr lossFunction;
    if (supervision.exists())
    {
      double supervisionValue = 0.0;
      if (supervision.isBoolean())
        supervisionValue = supervision.getBoolean() ? 1.0 : -1.0;
      else if (supervision.inheritsFrom(probabilityType))
        supervisionValue = supervision.getDouble() * 2.0 - 1.0;
      else
        jassert(false);

      bool isPositive = supervisionValue > 0.0;
      ScalarFunctionPtr* f = isPositive ? &positiveLoss : &negativeLoss;
      if (!*f)
        *f = createLossFunction(isPositive);
      lossFunction = *f;

      if (supervisionValue < 0)
        supervisionValue = -supervisionValue;
      if (supervisionValue != 1.0)
        lossFunction = lossFunction->multiplyByScalar(supervisionValue);
    }

    res->setSubInference(decorated, input, lossFunction);
    return res;
  }
   
  virtual Variable finalizeInference(const InferenceContextPtr& context, const DecoratorInferenceStatePtr& finalState, ReturnCode& returnCode)
  {
    static const double temperature = 1.0;
    Variable subInferenceOutput = finalState->getSubOutput();
    if (!subInferenceOutput.exists())
      return Variable();
    double score = subInferenceOutput.getDouble();
    return Variable(1.0 / (1.0 + exp(-score * temperature)), probabilityType);
  }

protected:
  ScalarFunctionPtr negativeLoss;
  ScalarFunctionPtr positiveLoss;
};

class BinaryLinearSVMInference : public BinaryClassificationInference
{
public:
  BinaryLinearSVMInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name)
    : BinaryClassificationInference(name, linearInference(name, perception), learner)
    {}
  BinaryLinearSVMInference(InferencePtr scoreInference)
    : BinaryClassificationInference(scoreInference->getName(), scoreInference)
    {}
  BinaryLinearSVMInference() {}

  virtual ScalarFunctionPtr createLossFunction(bool isPositive) const
    {return hingeLossFunction(isPositive);}
};

class BinaryLogisticRegressionInference : public BinaryClassificationInference
{
public:
  BinaryLogisticRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name)
    : BinaryClassificationInference(name, linearInference(name, perception), learner)
    {}
  BinaryLogisticRegressionInference() {}

  virtual ScalarFunctionPtr createLossFunction(bool isPositive) const
    {return logBinomialLossFunction(isPositive);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_INFERENCE_BINARY_CLASSIFICATION_H_
