/*-----------------------------------------.---------------------------------.
| Filename: ExecutionStack.h               | Execution Stack                 |
| Author  : Francis Maes                   |                                 |
| Started : 16/04/2010 18:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_FUNCTION_STACK_H_
# define LBCPP_EXECUTION_FUNCTION_STACK_H_

# include "predeclarations.h"
# include "../Core/Variable.h"

namespace lbcpp
{

class ExecutionStack : public Object
{
public:
  ExecutionStack(const ExecutionStackPtr& parentStack)
    : parentStack(parentStack) {}
  ExecutionStack() {}

  void push(const ObjectPtr& object); // WorkUnitVector, WorkUnit, Function, Inference
  void pop();
  
  size_t getDepth() const;
  const ObjectPtr& getElement(size_t depth) const;

  FunctionPtr findParentFunction() const;

  lbcpp_UseDebuggingNewOperator

private:
  friend class ExecutionStackClass;

  ExecutionStackPtr parentStack;
  std::vector<ObjectPtr> stack;
  static ObjectPtr nullObject;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_FUNCTION_STACK_H_
