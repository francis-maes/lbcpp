/*-----------------------------------------.---------------------------------.
| Filename: InferencePredeclarations.h     | Inference predeclarations       |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 14:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_PREDECLARATIONS_H_
# define LBCPP_INFERENCE_PREDECLARATIONS_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

/*
** InferenceStep
*/
class InferenceStep;
typedef ReferenceCountedObjectPtr<InferenceStep> InferenceStepPtr;

class SequentialInferenceStep;
typedef ReferenceCountedObjectPtr<SequentialInferenceStep> SequentialInferenceStepPtr;

class ParallelInferenceStep;
typedef ReferenceCountedObjectPtr<ParallelInferenceStep> ParallelInferenceStepPtr;

class SharedParallelInferenceStep;
typedef ReferenceCountedObjectPtr<SharedParallelInferenceStep> SharedParallelInferenceStepPtr;

class ClassificationInferenceStep;
typedef ReferenceCountedObjectPtr<ClassificationInferenceStep> ClassificationInferenceStepPtr;

class RegressionInferenceStep;
typedef ReferenceCountedObjectPtr<RegressionInferenceStep> RegressionInferenceStepPtr;

/*
** InferenceVisitor
*/
class InferenceVisitor;
typedef ReferenceCountedObjectPtr<InferenceVisitor> InferenceVisitorPtr;

/*
** InferenceContext
*/
class InferenceCallback;
typedef ReferenceCountedObjectPtr<InferenceCallback> InferenceCallbackPtr;

class InferenceStack;
typedef ReferenceCountedObjectPtr<InferenceStack> InferenceStackPtr;

class InferenceContext;
typedef ReferenceCountedObjectPtr<InferenceContext> InferenceContextPtr;

/*
** InferenceLearner
*/
class InferenceLearnerCallback;
typedef ReferenceCountedObjectPtr<InferenceLearnerCallback> InferenceLearnerCallbackPtr;

class InferenceLearner;
typedef ReferenceCountedObjectPtr<InferenceLearner> InferenceLearnerPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_PREDECLARATIONS_H_
