/*-----------------------------------------.---------------------------------.
 | Filename: RegressionInference.h          | Regression Inference classes    |
 | Author  : Julien Becker                  |                                 |
 | Started : 28/05/2010 05:11               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_INFERENCE_REGRESSION_H_
# define LBCPP_NUMERICAL_LEARNING_INFERENCE_REGRESSION_H_

# include <lbcpp/Inference/DecoratorInference.h>
# include <lbcpp/Function/ScalarFunction.h>
# include <lbcpp/NumericalLearning/NumericalLearning.h>

namespace lbcpp
{

class RegressionInference : public StaticDecoratorInference
{
public:
  RegressionInference(const String& name, InferencePtr scoreInference, InferenceOnlineLearnerPtr onlineLearner)
    : StaticDecoratorInference(name, scoreInference)
  {
    if (onlineLearner)
    {
      setBatchLearner(onlineToBatchInferenceLearner());
      scoreInference->addOnlineLearner(onlineLearner);
    }
  }
  RegressionInference() {}
  
  virtual TypePtr getSupervisionType() const
    {return doubleType;}

  virtual ScalarFunctionPtr getLoss(double target) const = 0;
  
  virtual void setName(const String& name)
  {
    DecoratorInference::setName(name);
    decorated->setName(name + T(" score"));
  }

  virtual DecoratorInferenceStatePtr prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    res->setSubInference(decorated, input, supervision.exists() ? getLoss(supervision.getDouble()) : ScalarFunctionPtr());
    return res;
  }
};

class SquareRegressionInference : public RegressionInference
{
public:
  SquareRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name)
    : RegressionInference(name, linearInference(name, perception), learner) {}

  SquareRegressionInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return squareLossFunction(target);}
};

class DihedralAngleRegressionInference : public RegressionInference
{
public:
  DihedralAngleRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name)
    : RegressionInference(name, linearInference(name, perception), learner) {}

  DihedralAngleRegressionInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return dihedralAngleSquareLossFunction(target);}
};

class AbsoluteRegressionInference : public RegressionInference
{
public:
  AbsoluteRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name)
    : RegressionInference(name, linearInference(name, perception), learner) {}

  AbsoluteRegressionInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return absoluteLossFunction(target);}
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_INFERENCE_REGRESSION_H_
