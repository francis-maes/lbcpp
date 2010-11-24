/*-----------------------------------------.---------------------------------.
| Filename: ExecutionContext.cpp           | Execution Context Base Class    |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 18:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Execution/ExecutionContext.h>
using namespace lbcpp;

ExecutionContext::ExecutionContext()
{
  context = this;
}

void ExecutionContext::informationCallback(const String& where, const String& what)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->informationCallback(where, what);
}

void ExecutionContext::warningCallback(const String& where, const String& what)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->warningCallback(where, what);
}

void ExecutionContext::errorCallback(const String& where, const String& what)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->errorCallback(where, what);
}

void ExecutionContext::statusCallback(const String& status)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->statusCallback(status);
}

void ExecutionContext::progressCallback(double progression, double progressionTotal, const String& progressionUnit)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->progressCallback(progression, progressionTotal, progressionUnit);
}

void ExecutionContext::preExecutionCallback(const WorkUnitPtr& workUnit)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->preExecutionCallback(workUnit);
}

void ExecutionContext::postExecutionCallback(const WorkUnitPtr& workUnit, bool result)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->postExecutionCallback(workUnit, result);
}

void ExecutionContext::resultCallback(const String& name, const Variable& value)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->resultCallback(name, value);
}

void ExecutionContext::appendCallback(const ExecutionCallbackPtr& callback)
{
  jassert(callback);
  callback->setContext(*this);
  callbacks.push_back(callback);
}

void ExecutionContext::removeCallback(const ExecutionCallbackPtr& callback)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    if (callbacks[i] == callback)
    {
      callbacks.erase(callbacks.begin() + i);
      break;
    }
}

void ExecutionContext::clearCallbacks()
  {callbacks.clear();}

bool ExecutionContext::run(const WorkUnitPtr& workUnit)
{
  preExecutionCallback(workUnit);
  bool res = workUnit->run(*this);
  postExecutionCallback(workUnit, res);
  return res;
}
