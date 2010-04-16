/*-----------------------------------------.---------------------------------.
| Filename: ClassificationInferenceStep.h  | Classification step             |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 20:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_CLASSIFICATION_H_
# define LBCPP_INFERENCE_STEP_CLASSIFICATION_H_

# include "InferenceStep.h"
# include "../InferenceContext/InferenceContext.h"

namespace lbcpp
{

// Input: FeatureGenerator
// Output: FeatureVector
// Supervision: Label
class ClassificationInferenceStep : public InferenceStep
{
public:
  ClassificationInferenceStep(const String& name)
    : InferenceStep(name) {}
 
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runClassification(ClassificationInferenceStepPtr(this), input, supervision, returnCode);}

  ClassifierPtr getClassifier() const
    {return classifier;}

  void setClassifier(ClassifierPtr classifier)
    {this->classifier = classifier;}

protected:
  ClassifierPtr classifier;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_CLASSIFICATION_H_
