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
# include <lbcpp/Inference/SequentialInference.h>
# include <lbcpp/NumericalLearning/LossFunctions.h>
# include <lbcpp/NumericalLearning/NumericalSupervisedInference.h>

namespace lbcpp
{

class RegressionInference : public NumericalSupervisedInference
{
public:
  RegressionInference(const String& name, InferencePtr scoreInference)
    : NumericalSupervisedInference(name, scoreInference) {}
  RegressionInference() {}
  
  virtual TypePtr getSupervisionType() const
    {return doubleType;}

  virtual ScalarFunctionPtr getLoss(double target) const = 0;
  
  virtual void setName(const String& name)
  {
    DecoratorInference::setName(name);
    decorated->setName(name + T(" score"));
  }

  virtual DecoratorInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    res->setSubInference(decorated, input, supervision.exists() ? getLoss(supervision.getDouble()) : ScalarFunctionPtr());
    return res;
  }
};

class SquareRegressionInference : public RegressionInference
{
public:
  SquareRegressionInference(const String& name, PerceptionPtr perception)
    : RegressionInference(name, linearInference(name, perception)) {}

  SquareRegressionInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return oldSquareLossFunction(target);}
};

class DihedralAngleRegressionInference : public RegressionInference
{
public:
  DihedralAngleRegressionInference(const String& name, PerceptionPtr perception)
    : RegressionInference(name, linearInference(name, perception)) {}

  DihedralAngleRegressionInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return oldDihedralAngleSquareLossFunction(target);}
};

class AbsoluteRegressionInference : public RegressionInference
{
public:
  AbsoluteRegressionInference(const String& name, PerceptionPtr perception)
    : RegressionInference(name, linearInference(name, perception)) {}

  AbsoluteRegressionInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return oldAbsoluteLossFunction(target);}
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_INFERENCE_REGRESSION_H_
