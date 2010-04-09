/*-----------------------------------------.---------------------------------.
| Filename: DecoratorInferencePolicy.h     | Base class for decorator policies|
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 15:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_POLICY_DECORATOR_H_
# define LBCPP_INFERENCE_POLICY_DECORATOR_H_

# include "InferencePolicy.h"

namespace lbcpp
{

class DecoratorInferencePolicy : public DefaultInferencePolicy
{
public:
  DecoratorInferencePolicy(InferencePolicyPtr targetPolicy)
    : targetPolicy(targetPolicy) {}

  // commands
  virtual ObjectPtr doSubStep(InferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return targetPolicy ? targetPolicy->doSubStep(step, input, supervision, returnCode)
                      : DefaultInferencePolicy::doSubStep(step, input, supervision, returnCode);}

  virtual ObjectPtr doParallelStep(ParallelInferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return targetPolicy ? targetPolicy->doParallelStep(step, input, supervision, returnCode)
                       : DefaultInferencePolicy::doParallelStep(step, input, supervision, returnCode);}

  virtual FeatureGeneratorPtr doClassification(ClassifierPtr classifier, FeatureGeneratorPtr input, FeatureGeneratorPtr supervision, ReturnCode& returnCode)
    {return targetPolicy ? targetPolicy->doClassification(classifier, input, supervision, returnCode)
                       : DefaultInferencePolicy::doClassification(classifier, input, supervision, returnCode);}

  // callbacks
  virtual ObjectContainerPtr supervisedExampleSetPreCallback(InferenceStepPtr inference, ObjectContainerPtr examples, ReturnCode& returnCode)
    {return targetPolicy ? targetPolicy->supervisedExampleSetPreCallback(inference, examples, returnCode) : examples;}
  
  virtual void supervisedExampleSetPostCallback(InferenceStepPtr inference, ObjectContainerPtr examples, ReturnCode& returnCode)
    {if (targetPolicy) targetPolicy->supervisedExampleSetPostCallback(inference, examples, returnCode);}

  virtual ObjectPtr supervisedExamplePreCallback(InferenceStepPtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return targetPolicy ? targetPolicy->supervisedExamplePreCallback(inference, input, supervision, returnCode) : input;}

  virtual ObjectPtr supervisedExamplePostCallback(InferenceStepPtr inference, ObjectPtr input, ObjectPtr output, ObjectPtr supervision, ReturnCode& returnCode)
    {return targetPolicy ? targetPolicy->supervisedExamplePostCallback(inference, input, output, supervision, returnCode) : output;}

protected:
  InferencePolicyPtr targetPolicy;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_POLICY_DECORATOR_H_
