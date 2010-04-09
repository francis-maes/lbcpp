/*-----------------------------------------.---------------------------------.
| Filename: ExamplesCreatorPolicy.h        | A policy that creates learning  |
| Author  : Francis Maes                   |  examples                       |
| Started : 09/04/2010 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_POLICY_EXAMPLES_CREATOR_H_
# define LBCPP_INFERENCE_POLICY_EXAMPLES_CREATOR_H_

# include "InferencePolicy.h"

namespace lbcpp
{

class ExamplesCreatorPolicy : public DefaultInferencePolicy
{
public:
  ExamplesCreatorPolicy(InferencePolicyPtr explorationPolicy)
    : explorationPolicy(explorationPolicy) {}

  virtual FeatureGeneratorPtr doClassification(ClassifierPtr classifier, FeatureGeneratorPtr input,
                                                             FeatureGeneratorPtr supervision, ReturnCode& returnCode)
  {
    if (supervision)
    {
      LabelPtr label = supervision.dynamicCast<Label>();
      jassert(label);
      addExample(classifier, new ClassificationExample(input, label->getIndex()));
    }
    return explorationPolicy->doClassification(classifier, input, supervision, returnCode);
  }

protected:
  InferencePolicyPtr explorationPolicy;

  typedef std::map<LearningMachinePtr, VectorObjectContainerPtr> ExamplesMap;
  ExamplesMap examples;

  void addExample(LearningMachinePtr learningMachine, ObjectPtr example)
  {
    VectorObjectContainerPtr& machineExamples = examples[learningMachine];
    if (!machineExamples)
      machineExamples = new VectorObjectContainer();
    machineExamples->append(example);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_POLICY_EXAMPLES_CREATOR_H_
