/*-----------------------------------------.---------------------------------.
| Filename: ExecutionStack.h               | Execution Stack                 |
| Author  : Francis Maes                   |                                 |
| Started : 16/04/2010 18:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_FUNCTION_STACK_H_
# define LBCPP_EXECUTION_FUNCTION_STACK_H_

# include "WorkUnit.h"

namespace lbcpp
{

class ExecutionStack : public Object
{
public:
  ExecutionStack(const ExecutionStackPtr& parentStack)
    : parentStack(parentStack) {}
  ExecutionStack() {}

  void push(const WorkUnitPtr& object);
  WorkUnitPtr pop();
  
  size_t getDepth() const;
  const WorkUnitPtr& getWorkUnit(size_t depth) const;

  lbcpp_UseDebuggingNewOperator

private:
  friend class ExecutionStackClass;

  ExecutionStackPtr parentStack;
  std::vector<WorkUnitPtr> stack;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_FUNCTION_STACK_H_
