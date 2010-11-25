/*-----------------------------------------.---------------------------------.
| Filename: InferenceContext.h             | Inference Context               |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_H_
# define LBCPP_INFERENCE_CONTEXT_H_

# include "Inference.h"
# include "../Execution/ThreadPool.h"
# include "../Execution/ExecutionContext.h"

namespace lbcpp
{

class Evaluator;
typedef ReferenceCountedObjectPtr<Evaluator> EvaluatorPtr;

class InferenceWorkUnit : public WorkUnit
{
public:
  InferenceWorkUnit(const String& name, InferencePtr inference, const Variable& input, const Variable& supervision, Variable& output);
  InferenceWorkUnit();

  virtual String toShortString() const;
  virtual String toString() const;
  virtual bool run(ExecutionContext& context);

protected:
  friend class InferenceWorkUnitClass;

  InferencePtr inference;
  Variable input;
  Variable supervision;
  Variable& output;
};

extern WorkUnitPtr inferenceWorkUnit(const String& name, InferencePtr inference, const Variable& input, const Variable& supervision, Variable& output);

extern bool runInference(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, Variable& output);
extern bool runInference(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision);

extern bool train(ExecutionContext& context, const InferencePtr& inference, ContainerPtr trainingExamples, ContainerPtr validationExamples);
extern bool train(ExecutionContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& learnerInput);
extern bool evaluate(ExecutionContext& context, const InferencePtr& inference, ContainerPtr examples, EvaluatorPtr evaluator);
extern bool crossValidate(ExecutionContext& context, const InferencePtr& inferenceModel, ContainerPtr examples, EvaluatorPtr evaluator, size_t numFolds);
extern Variable predict(ExecutionContext& context, const InferencePtr& inference, const Variable& input);

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_H_
