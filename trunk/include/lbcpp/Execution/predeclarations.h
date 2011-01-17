/*-----------------------------------------.---------------------------------.
| Filename: predeclarations.h              | Execution predeclarations       |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 19:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_PREDECLARATIONS_H_
# define LBCPP_EXECUTION_PREDECLARATIONS_H_

# include "../Core/predeclarations.h"

namespace lbcpp
{

class Function;
typedef ReferenceCountedObjectPtr<Function> FunctionPtr;

class Inference;
typedef ReferenceCountedObjectPtr<Inference> InferencePtr;

class ExecutionCallback;
typedef ReferenceCountedObjectPtr<ExecutionCallback> ExecutionCallbackPtr;

class ExecutionStack;
typedef ReferenceCountedObjectPtr<ExecutionStack> ExecutionStackPtr;

class ExecutionTraceItem;
typedef ReferenceCountedObjectPtr<ExecutionTraceItem> ExecutionTraceItemPtr;

class CompositeExecutionTraceItem;
typedef ReferenceCountedObjectPtr<CompositeExecutionTraceItem> CompositeExecutionTraceItemPtr;

class ExecutionTrace;
typedef ReferenceCountedObjectPtr<ExecutionTrace> ExecutionTracePtr;

class WorkUnit;
typedef ReferenceCountedObjectPtr<WorkUnit> WorkUnitPtr;

class CompositeWorkUnit;
typedef ReferenceCountedObjectPtr<CompositeWorkUnit> CompositeWorkUnitPtr;

class TestUnit;
typedef ReferenceCountedObjectPtr<TestUnit> TestUnitPtr;

class Notification;
typedef ReferenceCountedObjectPtr<Notification> NotificationPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_PREDECLARATIONS_H_
