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

  void push(const String& description, const WorkUnitPtr& object = WorkUnitPtr());
  std::pair<String, WorkUnitPtr> pop();
  
  size_t getDepth() const;
  const std::pair<String, WorkUnitPtr>& getEntry(size_t depth) const;

  const String& getDescription(size_t depth) const
    {return getEntry(depth).first;}

  const WorkUnitPtr& getWorkUnit(size_t depth) const
    {return getEntry(depth).second;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    const ExecutionStackPtr& t = target.staticCast<ExecutionStack>();
    t->parentStack = parentStack;
    t->stack = stack;
  }

  lbcpp_UseDebuggingNewOperator

private:
  friend class ExecutionStackClass;

  ExecutionStackPtr parentStack;
  std::vector< std::pair<String, WorkUnitPtr> > stack;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_FUNCTION_STACK_H_
