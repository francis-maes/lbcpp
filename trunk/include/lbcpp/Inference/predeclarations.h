/*-----------------------------------------.---------------------------------.
| Filename: InferencePredeclarations.h     | Inference predeclarations       |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 14:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_PREDECLARATIONS_H_
# define LBCPP_INFERENCE_PREDECLARATIONS_H_

# include "../Data/predeclarations.h"

namespace lbcpp
{

/*
** Inference
*/
class InferenceState;
typedef ReferenceCountedObjectPtr<InferenceState> InferenceStatePtr;

class Inference;
typedef ReferenceCountedObjectPtr<Inference> InferencePtr;

// sequential inference
class SequentialInferenceState;
typedef ReferenceCountedObjectPtr<SequentialInferenceState> SequentialInferenceStatePtr;

class SequentialInference;
typedef ReferenceCountedObjectPtr<SequentialInference> SequentialInferencePtr;
typedef SequentialInference* SequentialInferenceWeakPtr;

class VectorSequentialInference;
typedef ReferenceCountedObjectPtr<VectorSequentialInference> VectorSequentialInferencePtr;

// parallel inference
class ParallelInference;
typedef ReferenceCountedObjectPtr<ParallelInference> ParallelInferencePtr;
typedef ParallelInference* ParallelInferenceWeakPtr;

class StaticParallelInference;
typedef ReferenceCountedObjectPtr<StaticParallelInference> StaticParallelInferencePtr;

class VectorParallelInference;
typedef ReferenceCountedObjectPtr<VectorParallelInference> VectorParallelInferencePtr;

class SharedParallelInference;
typedef ReferenceCountedObjectPtr<SharedParallelInference> SharedParallelInferencePtr;

// decorator inference
class DecoratorInference;
typedef ReferenceCountedObjectPtr<DecoratorInference> DecoratorInferencePtr;
typedef DecoratorInference* DecoratorInferenceWeakPtr;

class StaticDecoratorInference;
typedef ReferenceCountedObjectPtr<StaticDecoratorInference> StaticDecoratorInferencePtr;

/*
** InferenceContext
*/
class InferenceCallback;
typedef ReferenceCountedObjectPtr<InferenceCallback> InferenceCallbackPtr;

class InferenceStack;
typedef ReferenceCountedObjectPtr<InferenceStack> InferenceStackPtr;

class InferenceContext;
typedef ReferenceCountedObjectPtr<InferenceContext> InferenceContextPtr;
typedef InferenceContext* InferenceContextWeakPtr;

class InferenceResultCache;
typedef ReferenceCountedObjectPtr<InferenceResultCache> InferenceResultCachePtr;

/*
** Learners
*/
class InferenceBatchLearnerInput;
typedef ReferenceCountedObjectPtr<InferenceBatchLearnerInput> InferenceBatchLearnerInputPtr;

class InferenceOnlineLearner;
typedef ReferenceCountedObjectPtr<InferenceOnlineLearner> InferenceOnlineLearnerPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_PREDECLARATIONS_H_
