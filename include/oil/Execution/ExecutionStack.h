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

  void push(const string& description, const WorkUnitPtr& object = WorkUnitPtr());
  std::pair<string, WorkUnitPtr> pop();
  
  size_t getDepth() const;
  const std::pair<string, WorkUnitPtr>& getEntry(size_t depth) const;

  const string& getDescription(size_t depth) const
    {return getEntry(depth).first;}

  const WorkUnitPtr& getWorkUnit(size_t depth) const
    {return getEntry(depth).second;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    const ExecutionStackPtr& t = target.staticCast<ExecutionStack>();
    t->parentStack = parentStack;
    t->stack = stack;
  }

  bool equals(const ExecutionStackPtr& otherStack) const;

  lbcpp_UseDebuggingNewOperator

private:
  friend class ExecutionStackClass;

  ExecutionStackPtr parentStack;
  std::vector< std::pair<string, WorkUnitPtr> > stack;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_FUNCTION_STACK_H_
