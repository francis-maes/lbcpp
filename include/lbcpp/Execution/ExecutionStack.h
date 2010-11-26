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
# include "../Function/Function.h"

namespace lbcpp
{

class ExecutionStack : public Object
{
public:
  ExecutionStack(const ExecutionStackPtr& parentStack)
    : parentStack(parentStack) {}
  ExecutionStack() {}

  void push(const FunctionPtr& function)
    {jassert(function); stack.push_back(function);}

  void pop()
    {jassert(stack.size()); stack.pop_back();}

  size_t getDepth() const;
  const FunctionPtr& getFunction(int index) const;
  const FunctionPtr& getCurrentFunction() const;
  const FunctionPtr& getParentFunction() const;

  juce_UseDebuggingNewOperator

private:
  friend class ExecutionStackClass;

  ExecutionStackPtr parentStack;
  std::vector<FunctionPtr> stack;
  static FunctionPtr nullFunction;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_FUNCTION_STACK_H_
