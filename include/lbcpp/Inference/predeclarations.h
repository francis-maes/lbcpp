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
class Inference;
typedef ReferenceCountedObjectPtr<Inference> InferencePtr;

class SequentialInferenceState;
typedef ReferenceCountedObjectPtr<SequentialInferenceState> SequentialInferenceStatePtr;

class SequentialInference;
typedef ReferenceCountedObjectPtr<SequentialInference> SequentialInferencePtr;

class VectorSequentialInference;
typedef ReferenceCountedObjectPtr<VectorSequentialInference> VectorSequentialInferencePtr;

class ParallelInference;
typedef ReferenceCountedObjectPtr<ParallelInference> ParallelInferencePtr;

class StaticParallelInference;
typedef ReferenceCountedObjectPtr<StaticParallelInference> StaticParallelInferencePtr;

class VectorParallelInference;
typedef ReferenceCountedObjectPtr<VectorParallelInference> VectorParallelInferencePtr;

class SharedParallelInference;
typedef ReferenceCountedObjectPtr<SharedParallelInference> SharedParallelInferencePtr;

class NumericalInference;
typedef ReferenceCountedObjectPtr<NumericalInference> NumericalInferencePtr;

class DecoratorInference;
typedef ReferenceCountedObjectPtr<DecoratorInference> DecoratorInferencePtr;

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

class InferenceResultCache;
typedef ReferenceCountedObjectPtr<InferenceResultCache> InferenceResultCachePtr;

/*
** Inference learners
*/
class InferenceOnlineLearner;
typedef ReferenceCountedObjectPtr<InferenceOnlineLearner> InferenceOnlineLearnerPtr;

/*
** InferenceLearner -- old
*/
class InferenceLearnerCallback;
typedef ReferenceCountedObjectPtr<InferenceLearnerCallback> InferenceLearnerCallbackPtr;

class InferenceLearner;
typedef ReferenceCountedObjectPtr<InferenceLearner> InferenceLearnerPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_PREDECLARATIONS_H_
