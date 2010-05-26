/*-----------------------------------------.---------------------------------.
| Filename: InferencePredeclarations.h     | Inference predeclarations       |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 14:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_PREDECLARATIONS_H_
# define LBCPP_INFERENCE_PREDECLARATIONS_H_

# include "../Object/Object.h"

namespace lbcpp
{

/*
** Inference
*/
class Inference;
typedef ReferenceCountedObjectPtr<Inference> InferencePtr;

class SequentialInference;
typedef ReferenceCountedObjectPtr<SequentialInference> SequentialInferencePtr;

class ParallelInference;
typedef ReferenceCountedObjectPtr<ParallelInference> ParallelInferencePtr;

class StaticParallelInference;
typedef ReferenceCountedObjectPtr<StaticParallelInference> StaticParallelInferencePtr;

class SharedParallelInference;
typedef ReferenceCountedObjectPtr<SharedParallelInference> SharedParallelInferencePtr;

class ClassificationInferenceStep;
typedef ReferenceCountedObjectPtr<ClassificationInferenceStep> ClassificationInferenceStepPtr;

class RegressionInferenceStep;
typedef ReferenceCountedObjectPtr<RegressionInferenceStep> RegressionInferenceStepPtr;

class ParameterizedInference;
typedef ReferenceCountedObjectPtr<ParameterizedInference> ParameterizedInferencePtr;

class DecoratorInference;
typedef ReferenceCountedObjectPtr<DecoratorInference> DecoratorInferencePtr;

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

class InferenceResultCache;
typedef ReferenceCountedObjectPtr<InferenceResultCache> InferenceResultCachePtr;

/*
** InferenceLearner
*/
class InferenceLearnerCallback;
typedef ReferenceCountedObjectPtr<InferenceLearnerCallback> InferenceLearnerCallbackPtr;

class InferenceLearner;
typedef ReferenceCountedObjectPtr<InferenceLearner> InferenceLearnerPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_PREDECLARATIONS_H_
