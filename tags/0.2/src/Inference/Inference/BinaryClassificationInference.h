/*-----------------------------------------.---------------------------------.
| Filename: BinaryClassificationInference.h| Binary Classification Inference |
| Author  : Francis Maes                   |  classes                        |
| Started : 26/05/2010 16:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BINARY_CLASSIFICATION_H_
# define LBCPP_INFERENCE_BINARY_CLASSIFICATION_H_

# include <lbcpp/Inference/DecoratorInference.h>

namespace lbcpp
{

class BinaryClassificationInference : public DecoratorInference
{
public:
  BinaryClassificationInference(const String& name, InferencePtr scoreInference)
    : DecoratorInference(name, scoreInference) {}
  BinaryClassificationInference() {}

  virtual ScalarFunctionPtr getLoss(size_t correctLabel) const = 0;

  virtual void setName(const String& name)
  {
    DecoratorInference::setName(name); 
    decorated->setName(name + T(" score"));
  }

  virtual std::pair<ObjectPtr, ObjectPtr> prepareSubInference(ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    LabelPtr correctLabel = supervision.dynamicCast<Label>();
    jassert(!supervision || (correctLabel && correctLabel->getDictionary() == BinaryClassificationDictionary::getInstance()));
    ScalarFunctionPtr lossFunction;
    if (correctLabel)
    {
      ScalarFunctionPtr* f = correctLabel->getIndex() ? &positiveLoss : &negativeLoss;
      if (!*f)
        *f = getLoss(correctLabel->getIndex());
      lossFunction = *f;
    }
    return std::make_pair(input, lossFunction);
  }
    
  virtual ObjectPtr finalizeSubInference(ObjectPtr input, ObjectPtr supervision, ObjectPtr subInferenceOutput, ReturnCode& returnCode) const
  {
    if (!subInferenceOutput)
      return ObjectPtr();
    ScalarPtr scalar = subInferenceOutput.dynamicCast<Scalar>();
    jassert(scalar);
    double value = scalar->getValue();
    return new Label(BinaryClassificationDictionary::getInstance(), value > 0 ? 1 : 0, fabs(value));
  }

protected:
  ScalarFunctionPtr negativeLoss;
  ScalarFunctionPtr positiveLoss;
};

class BinaryLinearSVMInference : public BinaryClassificationInference
{
public:
  BinaryLinearSVMInference(InferenceOnlineLearnerPtr learner, const String& name)
    : BinaryClassificationInference(name, linearScalarInference(name))
    {decorated->setOnlineLearner(learner);}
  BinaryLinearSVMInference() {}

  virtual ScalarFunctionPtr getLoss(size_t correctLabel) const
    {return hingeLoss(correctLabel);}
};

class BinaryLogisticRegressionInference : public BinaryClassificationInference
{
public:
  BinaryLogisticRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
    : BinaryClassificationInference(name, linearScalarInference(name))
    {decorated->setOnlineLearner(learner);}
  BinaryLogisticRegressionInference() {}

  virtual ScalarFunctionPtr getLoss(size_t correctLabel) const
    {return logBinomialLoss(correctLabel);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BINARY_CLASSIFICATION_H_
