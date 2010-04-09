/*-----------------------------------------.---------------------------------.
| Filename: SharedParallelInferenceStep.h  | Reduction from a problem        |
| Author  : Francis Maes                   |   to a simpler problem          |
| Started : 08/04/2010 18:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_REDUCTION_PREDICTION_PROBLEM_H_
# define LBCPP_REDUCTION_PREDICTION_PROBLEM_H_

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

class ParallelInferenceStep : public InferenceStep
{
public:
  ParallelInferenceStep(const String& name) : InferenceStep(name) {}

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(ParallelInferenceStepPtr(this));}
  
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runParallelInferences(ParallelInferenceStepPtr(this), input, supervision, returnCode);}

  virtual size_t getNumSubInferences(ObjectPtr input) const = 0;
  virtual InferenceStepPtr getSubInference(ObjectPtr input, size_t index) const = 0;
  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const = 0;
  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const = 0;

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const = 0;
  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const = 0;
};

class SharedParallelInferenceStep : public ParallelInferenceStep
{
public:
  SharedParallelInferenceStep(const String& name, InferenceStepPtr subInference)
    : ParallelInferenceStep(name), subInference(subInference) {}

  virtual InferenceStepPtr getSubInference(ObjectPtr input, size_t index) const
    {return subInference;}

protected:
  InferenceStepPtr subInference;
};

}; /* namespace lbcpp */

#endif //!LBCPP_REDUCTION_PREDICTION_PROBLEM_H_
