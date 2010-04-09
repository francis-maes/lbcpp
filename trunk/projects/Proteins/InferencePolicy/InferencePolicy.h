/*-----------------------------------------.---------------------------------.
| Filename: InferencePolicy.h              | Inference policy base classes   |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_POLICY_H_
# define LBCPP_INFERENCE_POLICY_H_

# include "../InferenceStep/InferenceStep.h"

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

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_POLICY_H_
