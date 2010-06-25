/*-----------------------------------------.---------------------------------.
 | Filename: RegressionInference.h          | Regression Inference classes    |
 | Author  : Julien Becker                  |                                 |
 | Started : 28/05/2010 05:11               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_REGRESSION_H_
# define LBCPP_INFERENCE_REGRESSION_H_

# include <lbcpp/Inference/DecoratorInference.h>

namespace lbcpp
{

class RegressionInference : public DecoratorInference
{
public:
  RegressionInference(const String& name, InferencePtr scoreInference)
    : DecoratorInference(name, scoreInference) {}
  RegressionInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const = 0;
  
  virtual void setName(const String& name)
  {
    DecoratorInference::setName(name);
    decorated->setName(name + T(" score"));
  }

  virtual std::pair<Variable, Variable> prepareSubInference(const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return std::make_pair(input, supervision ? getLoss(supervision.getDouble()) : ScalarFunctionPtr());}
};

class SquareRegressionInference : public RegressionInference
{
public:
  SquareRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
    : RegressionInference(name, linearScalarInference(name))
    {decorated->setOnlineLearner(learner);}

  SquareRegressionInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return squareLoss(target);}
};

class AngleRegressionInference : public RegressionInference
{
public:
  AngleRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
    : RegressionInference(name, linearScalarInference(name))
    {decorated->setOnlineLearner(learner);}

  AngleRegressionInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return dihedralAngleSquareLoss(target);}
};

class AbsoluteRegressionInference : public RegressionInference
{
public:
  AbsoluteRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
    : RegressionInference(name, linearScalarInference(name))
    {decorated->setOnlineLearner(learner);}

  AbsoluteRegressionInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return absoluteLoss(target);}
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_REGRESSION_H_
