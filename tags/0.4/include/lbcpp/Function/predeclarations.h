/*-----------------------------------------.---------------------------------.
| Filename: predeclarations.h              | Function predeclarations        |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 20:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PREDECLARATIONS_H_
# define LBCPP_FUNCTION_PREDECLARATIONS_H_

# include "../Data/predeclarations.h"

namespace lbcpp
{

class Function;
typedef ReferenceCountedObjectPtr<Function> FunctionPtr;

class IterationFunction;
typedef ReferenceCountedObjectPtr<IterationFunction> IterationFunctionPtr;

class StoppingCriterion;
typedef ReferenceCountedObjectPtr<StoppingCriterion> StoppingCriterionPtr;

class ScalarFunction;
typedef ReferenceCountedObjectPtr<ScalarFunction> ScalarFunctionPtr;

class ScalarObjectFunction;
typedef ReferenceCountedObjectPtr<ScalarObjectFunction> ScalarObjectFunctionPtr;

class Evaluator;
typedef ReferenceCountedObjectPtr<Evaluator> EvaluatorPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_PREDECLARATIONS_H_