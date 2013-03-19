/*-----------------------------------------.---------------------------------.
| Filename: ExecutionContextCallback.h     | Execution Context Callback      |
| Author  : Becker Julien                  |                                 |
| Started : 11/08/2011 14:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef OIL_EXECUTION_CONTEXT_CALLBACK_H_
# define OIL_EXECUTION_CONTEXT_CALLBACK_H_

# include "../Core/Object.h"

namespace lbcpp
{

class ExecutionContextCallback
{
public:
  virtual ~ExecutionContextCallback() {}

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const ObjectPtr& result, const ExecutionTracePtr& trace) = 0;
};

typedef ExecutionContextCallback* ExecutionContextCallbackPtr;

}; /* namespace lbcpp */

#endif //!OIL_EXECUTION_CONTEXT_CALLBACK_H_
