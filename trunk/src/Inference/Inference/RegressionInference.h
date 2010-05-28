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
  
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ScalarPtr target = supervision.dynamicCast<Scalar>();
    jassert(!supervision || target);
    ScalarFunctionPtr lossFunction = target ? getLoss(target->getValue()) : ScalarFunctionPtr();
    return DecoratorInference::run(context, input, lossFunction, returnCode);
  }
};

class RegressionSquareInference : public RegressionInference
{
public:
  RegressionSquareInference(InferenceOnlineLearnerPtr learner, const String& name)
    : RegressionInference(name, linearScalarInference(name))
    {decorated->setLearner(learner);}

  RegressionSquareInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return squareLoss(target);}
};

class RegressionAbsoluteInference : public RegressionInference
{
public:
  RegressionAbsoluteInference(InferenceOnlineLearnerPtr learner, const String& name)
    : RegressionInference(name, linearScalarInference(name))
    {decorated->setLearner(learner);}

  RegressionAbsoluteInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return absoluteLoss(target);}
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_REGRESSION_H_
