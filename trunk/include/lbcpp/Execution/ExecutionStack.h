/*-----------------------------------------.---------------------------------.
| Filename: ExecutionStack.h                | Function Stack                  |
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
  const FunctionPtr& getTopLevelInference() const
    {jassert(stack.size()); return stack[0];}

  const FunctionPtr& getCurrentInference() const
    {jassert(stack.size()); return stack.back();}

  static FunctionPtr nullFunction;

  const FunctionPtr& getParentInference() const
    {return stack.size() <= 1 ? nullFunction : stack[stack.size() - 2];}

  FunctionPtr getGrandParentInference() const
    {return stack.size() <= 2 ? nullFunction : stack[stack.size() - 3];}

  FunctionPtr getGrandGrandParentInference() const
    {return stack.size() <= 3 ? nullFunction : stack[stack.size() - 4];}

  void push(const FunctionPtr& function)
    {jassert(function); stack.push_back(function);}

  void pop()
    {jassert(stack.size()); stack.pop_back();}

  size_t getDepth() const // 0 = not running, 1 = top level
    {return stack.size();}

  bool isInferenceRunning(const FunctionPtr& inference, int* index = NULL)
  {
    for (size_t i = 0; i < stack.size(); ++i)
      if (stack[i] == inference)
      {
        if (index)
          *index = (int)i;
        return true;
      }
    if (index)
      *index = -1;
    return false;
  }

  const FunctionPtr& getInference(int index) const
    {return index >= 0 && index < (int)stack.size() ? stack[index] : nullFunction;}

private:
  friend class ExecutionStackClass;

  std::vector<FunctionPtr> stack;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_FUNCTION_STACK_H_
