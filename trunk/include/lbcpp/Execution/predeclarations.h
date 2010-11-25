/*-----------------------------------------.---------------------------------.
| Filename: predeclarations.h              | Execution predeclarations       |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 19:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_PREDECLARATIONS_H_
# define LBCPP_EXECUTION_PREDECLARATIONS_H_

# include "../Data/predeclarations.h"

namespace lbcpp
{

class ExecutionCallback;
typedef ReferenceCountedObjectPtr<ExecutionCallback> ExecutionCallbackPtr;

class ExecutionStack;
typedef ReferenceCountedObjectPtr<ExecutionStack> ExecutionStackPtr;

class ExecutionContext;
typedef ReferenceCountedObjectPtr<ExecutionContext> ExecutionContextPtr;

class WorkUnit;
typedef ReferenceCountedObjectPtr<WorkUnit> WorkUnitPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_PREDECLARATIONS_H_
