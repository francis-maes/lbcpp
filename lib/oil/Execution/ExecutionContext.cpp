/*-----------------------------------------.---------------------------------.
| Filename: ExecutionContext.cpp           | Execution Context Base Class    |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 18:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <oil/Execution/ExecutionContext.h>
#include <oil/Execution/ExecutionStack.h>
#include <oil/Core.h>
#include <oil/Core/RandomGenerator.h>
#include <oil/Lua/Lua.h>
#include <oil/library.h>
using namespace lbcpp;

/*
** ExecutionContext
*/
ExecutionContext::ExecutionContext(const juce::File& projectDirectory)
  : stack(new ExecutionStack()), projectDirectory(projectDirectory), randomGenerator(new RandomGenerator())
{
  initialize(*this);
}

ExecutionContext::~ExecutionContext() {}

void ExecutionContext::enterScope(const string& description, const WorkUnitPtr& workUnit)
{
  preExecutionCallback(stack, description, workUnit);
  stack->push(description, workUnit);
}

void ExecutionContext::enterScope(const WorkUnitPtr& workUnit)
  {enterScope(workUnit->toShortString(), workUnit);}

void ExecutionContext::leaveScope(const ObjectPtr& result)
{
  std::pair<string, WorkUnitPtr> entry = stack->pop();
  postExecutionCallback(stack, entry.first, entry.second, result);
}

void ExecutionContext::leaveScope(bool result)
  {leaveScope(ObjectPtr(new Boolean(result)));}

void ExecutionContext::leaveScope(size_t result)
  {leaveScope(ObjectPtr(new PositiveInteger(result)));}

void ExecutionContext::leaveScope(double result)
  {leaveScope(ObjectPtr(new Double(result)));}

void ExecutionContext::leaveScope(const string& result)
  {leaveScope(ObjectPtr(new String(result)));}

ObjectPtr ExecutionContext::run(const WorkUnitPtr& workUnit, bool pushIntoStack)
{
  if (pushIntoStack)
    enterScope(workUnit);
  ObjectPtr res = workUnit->run(*this);
  if (pushIntoStack)
    leaveScope(res);
  return res;
}

bool ExecutionContext::checkInheritance(ClassPtr type, ClassPtr baseType)
{
  jassert(baseType);
  if (!type || !type->inheritsFrom(baseType))
  {
    errorCallback(T("checkInheritance"), T("Invalid type, Expected ") + baseType->getName().quoted() + T(" found ") + (type ? type->getName().quoted() : T("Nil")));
    return false;
  }
  return true;
}

bool ExecutionContext::checkInheritance(const ObjectPtr& object, ClassPtr baseType)
{
  jassert(baseType);
  return !object || checkInheritance((ClassPtr)object->getClass(), baseType);
}

static bool checkSharedPointerCyclesRecursively(ExecutionContext& context, const ObjectPtr& object, std::vector<ObjectPtr>& currentStack)
{
  for (size_t i = 0; i < currentStack.size(); ++i)
    if (currentStack[i] == object)
    {
      string cycle;
      for (size_t j = i; j < currentStack.size(); ++j)
        cycle += currentStack[j]->getClassName() + T(" -> ");
      cycle += object->getClassName();
      context.errorCallback(T("Found a shared pointer cycle: ") + cycle);
      return false;
    }

  currentStack.push_back(object);
  size_t n = object->getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    ObjectPtr v = object->getVariable(i);
    if (v)
    {
      if (!checkSharedPointerCyclesRecursively(context, v, currentStack))
        return false;
    }
  }
  currentStack.pop_back();
  return true;
}

bool ExecutionContext::checkSharedPointerCycles(const ObjectPtr& object)
{
  std::vector<ObjectPtr> currentStack;
  return checkSharedPointerCyclesRecursively(*this, object, currentStack);
}

juce::File ExecutionContext::getFile(const string& path)
{
  if (path.isEmpty())
    return juce::File::nonexistent;
  if (juce::File::isAbsolutePath(path))
    return juce::File(path);
  juce::File dir = getProjectDirectory();
  if (!dir.exists())
  {
    errorCallback(T("Project directory is not specified. Could not find path ") + path);
    return juce::File::nonexistent;
  }
  return dir.getChildFile(path);
}

string ExecutionContext::getFilePath(const juce::File& file) const
{
  juce::File dir = getProjectDirectory();
  if (dir.exists() && file.isAChildOf(dir))
    return file.getRelativePathFrom(dir).replaceCharacter('\\', '/');
  else
    return file.getFullPathName();
}

int ExecutionContext::enter(LuaState& state)
{
  ExecutionContextPtr pthis = state.checkObject(1, executionContextClass);
  string name = state.checkString(2);
  pthis->enterScope(name);
  return 0;
}

int ExecutionContext::leave(LuaState& state)
{
  ExecutionContextPtr pthis = state.checkObject(1, executionContextClass);
  ObjectPtr res = new Boolean(true);
  if (state.getTop() >= 2)
    res = state.checkObject(2);
  pthis->leaveScope(res);
  return 0;
}

// forwarder to Context.call(...)
int ExecutionContext::call(LuaState& state)
{
  int numArguments = state.getTop();
  state.getGlobal("Context", "call");
  state.insert(1); // move function to top of stack
  state.call(numArguments, 1);  // no support for multiple returns yet
  return 1;
}

// WorkUnit -> Variable
int ExecutionContext::run(LuaState& state)
{
  ExecutionContextPtr pthis = state.checkObject(1, executionContextClass);
  WorkUnitPtr workUnit = state.checkObject(2, workUnitClass);
  ObjectPtr res = pthis->run(workUnit, true);
  if (!res)
    return 0;
  state.pushObject(res);
  return 1;
}

class LuaExecutionContextCallback : public ExecutionContextCallback
{
public:
  LuaExecutionContextCallback(LuaState& state, int functionReference)
    : state((lua_State* )state), functionReference(functionReference) {}

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const ObjectPtr& result, const ExecutionTracePtr& trace)
  {
    {
      ScopedLock _(state.lock);
      state.pushReference(functionReference);
      state.pushObject(workUnit);
      state.pushObject(result);
      state.call(2, 0);
      state.freeReference(functionReference);
    }
    delete this;
  }

protected:
  LuaState state;
  int functionReference;
};

int ExecutionContext::push(LuaState& state)
{
  ExecutionContextPtr pthis = state.checkObject(1, executionContextClass);
  WorkUnitPtr workUnit = state.checkObject(2, workUnitClass);
  ExecutionContextCallbackPtr callback = new LuaExecutionContextCallback(state, state.toReference(3));
  pthis->pushWorkUnit(workUnit, callback);
  return 0;
}

int ExecutionContext::sleep(LuaState& state)
{
  ExecutionContextPtr pthis = state.checkObject(1, executionContextClass);
  double lengthInSeconds = state.checkNumber(2);
  Thread::sleep((int)(lengthInSeconds * 1000));
  pthis->flushCallbacks();
  return 0;
}

// forwarder to Context.random(...)
int ExecutionContext::random(LuaState& state)
{
  int numArguments = state.getTop();
  state.getGlobal("Context", "random");
  state.insert(1); // move function to top of stack
  state.call(numArguments, 1);  // no support for multiple returns yet
  return 1;
}

int ExecutionContext::waitUntilAllWorkUnitsAreDone(LuaState& state)
{
  ExecutionContextPtr pthis = state.checkObject(1, executionContextClass);
  pthis->waitUntilAllWorkUnitsAreDone();
  return 0;
}

/*
** ExecutionStack
*/
size_t ExecutionStack::getDepth() const // 0 = not running, 1 = top level
  {return (parentStack ? parentStack->getDepth() : 0) + stack.size();}

void ExecutionStack::push(const string& description, const WorkUnitPtr& workUnit)
  {stack.push_back(std::make_pair(description, workUnit));}

std::pair<string, WorkUnitPtr> ExecutionStack::pop()
{
  jassert(stack.size());
  std::pair<string, WorkUnitPtr> res = stack.back();
  stack.pop_back();
  return res;
}

const std::pair<string, WorkUnitPtr>& ExecutionStack::getEntry(size_t depth) const
{
  size_t parentDepth = parentStack ? parentStack->getDepth() : 0;
  if (depth < parentDepth)
    return parentStack->getEntry(depth);
  depth -= parentDepth;
  jassert(depth < stack.size());
  return stack[depth];
}

bool ExecutionStack::equals(const ExecutionStackPtr& otherStack) const
{
  if (stack != otherStack->stack)
    return false;
  return (parentStack == otherStack->parentStack) || (parentStack && parentStack->equals(otherStack->parentStack));
}

/*
** TimedScope
*/
TimedScope::TimedScope(ExecutionContext& context, const string& name, bool enable)
  : context(context), startTime(0.0)
{
  if (enable)
  {
    this->name = name;
    startTime = Time::getHighResolutionCounter();
  }
}

TimedScope::~TimedScope()
{
  if (name.isNotEmpty())
    context.resultCallback(name + T(" time"), Time::getHighResolutionCounter() - startTime);
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
