/*-----------------------------------------.---------------------------------.
| Filename: ExecutionContext.cpp           | Execution Context Base Class    |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 18:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Execution/ExecutionContext.h>
#include <lbcpp/Execution/ExecutionStack.h>
#include <lbcpp/Core/TypeManager.h>
#include <lbcpp/Core/Variable.h>
#include <lbcpp/Core/Function.h>
#include <lbcpp/Inference/Inference.h>
using namespace lbcpp;

/*
** ExecutionContext
*/
ExecutionContext::ExecutionContext()
  : stack(new ExecutionStack())
{
  initialize(*this);
}

bool ExecutionContext::run(const WorkUnitPtr& workUnit)
{
  preExecutionCallback(stack, workUnit);
  stack->push(workUnit);
  bool res = workUnit->run(*this);
  stack->pop();
  postExecutionCallback(stack, workUnit, res);
  return res;
}

#ifdef JUCE_DEBUG
bool ExecutionContext::checkInheritance(TypePtr type, TypePtr baseType)
{
  jassert(baseType);
  if (!type || !type->inheritsFrom(baseType))
  {
    errorCallback(T("checkInheritance"), T("Invalid type, Expected ") + baseType->getName().quoted() + T(" found ") + (type ? type->getName().quoted() : T("Nil")));
    return false;
  }
  return true;
}

bool ExecutionContext::checkInheritance(const Variable& variable, TypePtr baseType)
{
  jassert(baseType);
  return variable.isNil() || checkInheritance(variable.getType(), baseType);
}
#endif // JUCE_DEBUG

/*
** ExecutionStack
*/
//ObjectPtr ExecutionStack::nullObject;

size_t ExecutionStack::getDepth() const // 0 = not running, 1 = top level
  {return (parentStack ? parentStack->getDepth() : 0) + stack.size();}

void ExecutionStack::push(const WorkUnitPtr& workUnit)
{
  jassert(workUnit);
  stack.push_back(workUnit);
}

void ExecutionStack::pop()
{
  jassert(stack.size());
  stack.pop_back();
}
/*
FunctionPtr ExecutionStack::findParentFunction() const
{
  for (int i = (int)stack.size() - 1; i >= 0; --i)
  {
    FunctionPtr res = stack[i].dynamicCast<Function>();
    if (res)
      return res;
  }
  return FunctionPtr();
}*/

const WorkUnitPtr& ExecutionStack::getWorkUnit(size_t depth) const
{
  size_t parentDepth = parentStack ? parentStack->getDepth() : 0;
  if (depth < parentDepth)
    return parentStack->getWorkUnit(depth);
  depth -= parentDepth;
  jassert(depth < stack.size());
  return stack[depth];
}

/*
** Execution Context constructor functions
*/
ExecutionContextPtr lbcpp::defaultConsoleExecutionContext(bool noMultiThreading)
{
  int numCpus = juce::SystemStats::getNumCpus();
  ExecutionContextPtr res = (numCpus > 1 && !noMultiThreading ? multiThreadedExecutionContext(numCpus) : singleThreadedExecutionContext());
  res->appendCallback(consoleExecutionCallback());
  return res;
}
