/*-----------------------------------------.---------------------------------.
| Filename: ExecutionContext.cpp           | Execution Context Base Class    |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 18:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Execution/ExecutionContext.h>
#include <lbcpp/Execution/ExecutionStack.h>
#include <lbcpp/Core/Variable.h>
#include <lbcpp/Core/Function.h>
#include <lbcpp/Inference/Inference.h>
#include <lbcpp/lbcpp.h>
using namespace lbcpp;

/*
** ExecutionStack
*/
FunctionPtr ExecutionStack::nullFunction;

size_t ExecutionStack::getDepth() const // 0 = not running, 1 = top level
  {return (parentStack ? parentStack->getDepth() : 0) + stack.size();}

void ExecutionStack::push(const FunctionPtr& function)
{
  jassert(function);
  stack.push_back(function);
}

void ExecutionStack::pop()
{
  jassert(stack.size());
  stack.pop_back();
}

const FunctionPtr& ExecutionStack::getFunction(int index) const
{
  if (index < 0)
    return nullFunction;
  if (parentStack)
  {
    size_t pd = parentStack->getDepth();
    if (index < (int)pd)
      return parentStack->getFunction(index);
    index -= (int)pd;
  }
  return index < (int)stack.size() ? stack[index] : nullFunction;
}

const FunctionPtr& ExecutionStack::getCurrentFunction() const
{
  if (stack.size())
    return stack.back();
  else if (parentStack)
    return parentStack->getCurrentFunction();
  else
    return nullFunction;
}

const FunctionPtr& ExecutionStack::getParentFunction() const
  {return getFunction((int)getDepth() - 2);}


/*
** CompositeExecutionCallback
*/
void ExecutionCallback::preExecutionCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision)
  {preExecutionCallback(inference, input);}

void ExecutionCallback::postExecutionCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& output)
  {postExecutionCallback(inference, input, output);}

/*
** CompositeExecutionCallback
*/
void CompositeExecutionCallback::informationCallback(const String& where, const String& what)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->informationCallback(where, what);
}

void CompositeExecutionCallback::warningCallback(const String& where, const String& what)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->warningCallback(where, what);
}

void CompositeExecutionCallback::errorCallback(const String& where, const String& what)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->errorCallback(where, what);
}

void CompositeExecutionCallback::statusCallback(const String& status)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->statusCallback(status);
}

void CompositeExecutionCallback::progressCallback(double progression, double progressionTotal, const String& progressionUnit)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->progressCallback(progression, progressionTotal, progressionUnit);
}

void CompositeExecutionCallback::preExecutionCallback(const WorkUnitVectorPtr& workUnits)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->preExecutionCallback(workUnits);
}

void CompositeExecutionCallback::postExecutionCallback(const WorkUnitVectorPtr& workUnits, bool result)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->postExecutionCallback(workUnits, result);
}

void CompositeExecutionCallback::preExecutionCallback(const WorkUnitPtr& workUnit)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->preExecutionCallback(workUnit);
}

void CompositeExecutionCallback::postExecutionCallback(const WorkUnitPtr& workUnit, bool result)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->postExecutionCallback(workUnit, result);
}

void CompositeExecutionCallback::preExecutionCallback(const FunctionPtr& function, const Variable& input)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->preExecutionCallback(function, input);
}

void CompositeExecutionCallback::postExecutionCallback(const FunctionPtr& function, const Variable& input, const Variable& output)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->postExecutionCallback(function, input, output);
}

void CompositeExecutionCallback::preExecutionCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->preExecutionCallback(inference, input, supervision);
}

void CompositeExecutionCallback::postExecutionCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& output)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->postExecutionCallback(inference, input, supervision, output);
}

void CompositeExecutionCallback::resultCallback(const String& name, const Variable& value)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->resultCallback(name, value);
}

void CompositeExecutionCallback::appendCallback(const ExecutionCallbackPtr& callback)
{
  jassert(callback);
  callback->initialize(*context);
  callbacks.push_back(callback);
}

void CompositeExecutionCallback::removeCallback(const ExecutionCallbackPtr& callback)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    if (callbacks[i] == callback)
    {
      callbacks.erase(callbacks.begin() + i);
      break;
    }
}

void CompositeExecutionCallback::clearCallbacks()
  {callbacks.clear();}


/*
** ExecutionContext
*/
ExecutionContext::ExecutionContext()
  : stack(new ExecutionStack())
{
  initialize(*this);
}

void ExecutionContext::finishTypeDeclarations()
  {typeManager().finishDeclarations(*this);}

void ExecutionContext::declareType(TypePtr typeInstance)
  {typeManager().declare(*this, typeInstance);}

void ExecutionContext::declareTemplateType(TemplateTypePtr templateTypeInstance)
  {typeManager().declare(*this, templateTypeInstance);}

TypePtr ExecutionContext::getType(const String& typeName)
  {return typeManager().getType(*this, typeName);}

TypePtr ExecutionContext::getType(const String& name, const std::vector<TypePtr>& arguments)
  {return typeManager().getType(*this, name, arguments);}

bool ExecutionContext::doTypeExists(const String& typeName)
  {return typeManager().doTypeExists(typeName);}

size_t ExecutionContext::getStackDepth() const
  {return stack->getDepth();}

const FunctionPtr& ExecutionContext::getCurrentFunction() const
  {return stack->getCurrentFunction();}

const FunctionPtr& ExecutionContext::getParentFunction() const
  {return stack->getParentFunction();}

bool ExecutionContext::run(const WorkUnitPtr& workUnit)
{
  preExecutionCallback(workUnit);
  bool res = workUnit->run(*this);
  postExecutionCallback(workUnit, res);
  return res;
}

ObjectPtr ExecutionContext::createObject(ClassPtr objectClass)
{
  ObjectPtr res = objectClass->create(*this).getObject();
  jassert(res);
  jassert(res->getReferenceCount() == 2);
  res->decrementReferenceCounter();
  return res;
}

Variable ExecutionContext::createVariable(TypePtr type)
{
  jassert(type && type->isInitialized());
  return Variable(type, type->create(*this));
}

EnumerationPtr ExecutionContext::getEnumeration(const String& className)
  {return (EnumerationPtr)getType(className);}

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
** Execution Context constructor functions
*/
ExecutionContextPtr lbcpp::defaultConsoleExecutionContext(bool noMultiThreading)
{
  int numCpus = juce::SystemStats::getNumCpus();
  ExecutionContextPtr res = numCpus > 1 && !noMultiThreading ? multiThreadedExecutionContext(numCpus) : singleThreadedExecutionContext();
  res->appendCallback(consoleExecutionCallback());
  return res;
}

ExecutionContextPtr lbcpp::silentExecutionContext = singleThreadedExecutionContext();
