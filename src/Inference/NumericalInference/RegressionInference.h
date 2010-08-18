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

class RegressionInference : public StaticDecoratorInference
{
public:
  RegressionInference(const String& name, InferencePtr scoreInference)
    : StaticDecoratorInference(name, scoreInference)
    {setBatchLearner(onlineToBatchInferenceLearner());}
  RegressionInference() {}
  
  virtual TypePtr getSupervisionType() const
    {return doubleType();}

  virtual ScalarFunctionPtr getLoss(double target) const = 0;
  
  virtual void setName(const String& name)
  {
    DecoratorInference::setName(name);
    decorated->setName(name + T(" score"));
  }

  virtual DecoratorInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    res->setSubInference(decorated, input, supervision ? getLoss(supervision.getDouble()) : ScalarFunctionPtr());
    return res;
  }
};

class SquareRegressionInference : public RegressionInference
{
public:
  SquareRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
    : RegressionInference(name, linearInference(name))
    {decorated->setOnlineLearner(learner);}

  SquareRegressionInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return squareLoss(target);}
};

class DihedralAngleRegressionInference : public RegressionInference
{
public:
  DihedralAngleRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
    : RegressionInference(name, linearInference(name))
    {decorated->setOnlineLearner(learner);}

  DihedralAngleRegressionInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return dihedralAngleSquareLoss(target);}
};

class AbsoluteRegressionInference : public RegressionInference
{
public:
  AbsoluteRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
    : RegressionInference(name, linearInference(name))
    {decorated->setOnlineLearner(learner);}

  AbsoluteRegressionInference() {}
  
  virtual ScalarFunctionPtr getLoss(double target) const
    {return absoluteLoss(target);}
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_REGRESSION_H_
