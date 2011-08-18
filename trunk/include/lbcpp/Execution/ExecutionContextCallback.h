/*-----------------------------------------.---------------------------------.
| Filename: ExecutionContextCallback.h     | Execution Context Callback      |
| Author  : Becker Julien                  |                                 |
| Started : 11/08/2011 14:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CONTEXT_CALLBACK_H_
# define LBCPP_EXECUTION_CONTEXT_CALLBACK_H_

# include "../Core/Object.h"

namespace lbcpp
{

class ExecutionContextCallback
{
public:
  virtual ~ExecutionContextCallback() {};

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result) = 0;
};

typedef ReferenceCountedObjectPtr<ExecutionContextCallback> ExecutionContextCallbackPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_CALLBACK_H_
