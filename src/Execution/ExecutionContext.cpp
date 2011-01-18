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

void ExecutionContext::enterScope(const String& description, const WorkUnitPtr& workUnit)
{
  preExecutionCallback(stack, description, workUnit);
  stack->push(description, workUnit);
}

void ExecutionContext::enterScope(const WorkUnitPtr& workUnit)
  {enterScope(workUnit->getName(), workUnit);}

void ExecutionContext::leaveScope(bool result)
{
  std::pair<String, WorkUnitPtr> entry = stack->pop();
  postExecutionCallback(stack, entry.first, entry.second, result);
}

bool ExecutionContext::run(const WorkUnitPtr& workUnit, bool pushIntoStack)
{
  if (pushIntoStack)
    enterScope(workUnit);
  bool res = workUnit->run(*this);
  if (pushIntoStack)
    leaveScope(res);
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
size_t ExecutionStack::getDepth() const // 0 = not running, 1 = top level
  {return (parentStack ? parentStack->getDepth() : 0) + stack.size();}

void ExecutionStack::push(const String& description, const WorkUnitPtr& workUnit)
  {stack.push_back(std::make_pair(description, workUnit));}

std::pair<String, WorkUnitPtr> ExecutionStack::pop()
{
  jassert(stack.size());
  std::pair<String, WorkUnitPtr> res = stack.back();
  stack.pop_back();
  return res;
}

const std::pair<String, WorkUnitPtr>& ExecutionStack::getEntry(size_t depth) const
{
  size_t parentDepth = parentStack ? parentStack->getDepth() : 0;
  if (depth < parentDepth)
    return parentStack->getEntry(depth);
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
