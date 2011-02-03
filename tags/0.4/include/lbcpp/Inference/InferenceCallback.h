/*-----------------------------------------.---------------------------------.
| Filename: InferenceCallback.h            | Inference Callback base class   |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 19:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_H_

# include "Inference.h"
# include "../Execution/ExecutionCallback.h"

namespace lbcpp
{

extern ExecutionCallbackPtr cacheInferenceCallback(InferenceResultCachePtr cache, InferencePtr parentStep);
extern ExecutionCallbackPtr evaluationInferenceCallback(InferencePtr inference, EvaluatorPtr evaluator);

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CALLBACK_H_
