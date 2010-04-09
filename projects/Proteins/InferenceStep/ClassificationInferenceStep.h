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
# include "../InferenceCallback/InferenceContext.h"

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

  GradientBasedClassifierPtr createMaxentClassifier(FeatureDictionaryPtr labels, double regularizer = 20.0, bool useConstantLearningRate = false)
  {
    IterationFunctionPtr learningRate = useConstantLearningRate ? invLinearIterationFunction(2.0, 250000) : constantIterationFunction(1.0);
    GradientBasedLearnerPtr learner = stochasticDescentLearner(learningRate);  
    GradientBasedClassifierPtr classifier = maximumEntropyClassifier(learner, labels);
    classifier->setL2Regularizer(regularizer);
    return classifier;
  }

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    FeatureGeneratorPtr correctOutput = supervision.dynamicCast<FeatureGenerator>();

    if (!classifier)
    {
      if (!supervision)
      {
        returnCode = errorReturnCode;
        return ObjectPtr();
      }
      jassert(correctOutput);
      classifier = createMaxentClassifier(correctOutput->getDictionary());
    }
    return context->runClassification(classifier, input.dynamicCast<FeatureGenerator>(), correctOutput, returnCode);
  }

protected:
  ClassifierPtr classifier;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_CLASSIFICATION_H_
