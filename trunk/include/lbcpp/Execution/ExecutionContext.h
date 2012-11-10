/*-----------------------------------------.---------------------------------.
| Filename: ExecutionContext.h             | Execution Context Base Class    |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 17:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CONTEXT_H_
# define LBCPP_EXECUTION_CONTEXT_H_

# include "ExecutionCallback.h"
# include "ExecutionContextCallback.h"
# include "../Data/RandomGenerator.h"

namespace lbcpp
{

class RandomGenerator;
typedef ReferenceCountedObjectPtr<RandomGenerator> RandomGeneratorPtr;

class ExecutionContext : public CompositeExecutionCallback
{
public:
  ExecutionContext(const File& projectDirectory = File::nonexistent);
  virtual ~ExecutionContext();
  
  virtual bool isMultiThread() const = 0;

  /*
  ** Checks
  */
  bool checkInheritance(TypePtr type, TypePtr baseType);
  bool checkInheritance(const ObjectPtr& object, TypePtr baseType);
  bool checkSharedPointerCycles(const ObjectPtr& object);

  /*
  ** Current State
  */
  virtual bool isCanceled() const = 0;
  virtual bool isPaused() const = 0;

  ExecutionStackPtr getStack() const
    {return stack;}

  void setStack(const ExecutionStackPtr& stack)
    {this->stack = stack;}

  void enterScope(const String& description, const WorkUnitPtr& workUnit = WorkUnitPtr());
  void enterScope(const WorkUnitPtr& workUnit);
  void leaveScope(const ObjectPtr& result);
  
  void leaveScope(bool result = true);
  void leaveScope(size_t result);
  void leaveScope(double result);
  void leaveScope(const String& result);

  template<class T>
  void leaveScope(const ReferenceCountedObjectPtr<T>& value)
    {leaveScope(ObjectPtr(value));}

  /*
  ** Work Units
  */
  virtual ObjectPtr run(const WorkUnitPtr& workUnit, bool pushIntoStack = true);
  virtual ObjectPtr run(const CompositeWorkUnitPtr& workUnits, bool pushIntoStack = true) = 0;

  // multi-thread
  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, ExecutionContextCallbackPtr callback = ExecutionContextCallbackPtr(), bool pushIntoStack = true)
    {pushWorkUnit(workUnit, (int*)NULL);}
  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, int* counterToDecrementWhenDone, bool pushIntoStack = true)
    {jassert(isMultiThread());}
  virtual void waitUntilAllWorkUnitsAreDone(size_t timeOutInMilliseconds = 0)
    {}
  virtual void flushCallbacks()
    {}

  /*
  ** Access to files
  */
  File getFile(const String& path);
  String getFilePath(const File& file) const;

  virtual File getProjectDirectory() const
    {return projectDirectory;}

  void setProjectDirectory(const File& projectDirectory)
    {this->projectDirectory = projectDirectory;}

  /*
  ** Random generator
  */
  const RandomGeneratorPtr& getRandomGenerator() const
    {return randomGenerator;}
    
  void setRandomGenerator(const RandomGeneratorPtr& random)
    {randomGenerator = random;}

  /*
  ** Lua
  */
  static int enter(LuaState& state);
  static int leave(LuaState& state);
  static int call(LuaState& state);

  static int run(LuaState& state);
  static int push(LuaState& state);

  static int sleep(LuaState& state);
  static int random(LuaState& state);

  static int waitUntilAllWorkUnitsAreDone(LuaState& state);

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExecutionContextClass;

  ExecutionStackPtr stack;
  File projectDirectory;
  RandomGeneratorPtr randomGenerator;
};

extern ClassPtr executionContextClass;

extern ExecutionContext& defaultExecutionContext();
extern void setDefaultExecutionContext(ExecutionContextPtr defaultContext);

extern ExecutionContextPtr singleThreadedExecutionContext(const File& projectDirectory = File::nonexistent);
extern ExecutionContextPtr multiThreadedExecutionContext(size_t numThreads, const File& projectDirectory = File::nonexistent);

extern ExecutionContextPtr defaultConsoleExecutionContext(bool noMultiThreading = false);

class TimedScope
{
public:
  TimedScope(ExecutionContext& context, const String& name, bool enable = true);
  ~TimedScope();
  
private:
  ExecutionContext& context;
  String name;
  double startTime;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_H_
