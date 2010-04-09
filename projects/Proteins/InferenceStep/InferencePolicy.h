/*-----------------------------------------.---------------------------------.
| Filename: InferencePolicy.h              | Inference policy base classes   |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_POLICY_H_
# define LBCPP_INFERENCE_POLICY_H_

# include "InferenceStep.h"

namespace lbcpp
{

class InferencePolicy : public NameableObject
{
public:
  typedef InferenceStep::ReturnCode ReturnCode;

  ReturnCode runOnSelfSupervisedExampleSet(InferenceStepPtr inference, ObjectContainerPtr examples);
  ReturnCode runOnSupervisedExampleSet(InferenceStepPtr inference, ObjectContainerPtr examples);
  ObjectPtr runOnSupervisedExample(InferenceStepPtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);

  virtual ObjectPtr doSubStep(InferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;
  virtual ObjectPtr doParallelStep(ParallelInferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;

  virtual FeatureGeneratorPtr doClassification(ClassifierPtr classifier, FeatureGeneratorPtr input, FeatureGeneratorPtr supervision, ReturnCode& returnCode) = 0;

public:
  virtual ObjectContainerPtr supervisedExampleSetPreCallback(InferenceStepPtr inference, ObjectContainerPtr examples, ReturnCode& returnCode)
    {return examples;}

  virtual void supervisedExampleSetPostCallback(InferenceStepPtr inference, ObjectContainerPtr examples, ReturnCode& returnCode)
    {}

  virtual ObjectPtr supervisedExamplePreCallback(InferenceStepPtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return input;}

  virtual ObjectPtr supervisedExamplePostCallback(InferenceStepPtr inference, ObjectPtr input, ObjectPtr output, ObjectPtr supervision, ReturnCode& returnCode)
    {return output;}
};

class DefaultInferencePolicy : public InferencePolicy
{
public:
  virtual ObjectPtr doSubStep(InferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
  virtual ObjectPtr doParallelStep(ParallelInferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);

  virtual FeatureGeneratorPtr doClassification(ClassifierPtr classifier, FeatureGeneratorPtr input, FeatureGeneratorPtr supervision, ReturnCode& returnCode);
};

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

#endif //!LBCPP_INFERENCE_POLICY_H_
