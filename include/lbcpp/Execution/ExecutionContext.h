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
  bool checkInheritance(const Variable& variable, TypePtr baseType);
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
  void leaveScope(const Variable& result);
  void leaveScope();

  /*
  ** Work Units
  */
  virtual Variable run(const WorkUnitPtr& workUnit, bool pushIntoStack = true);
  virtual Variable run(const CompositeWorkUnitPtr& workUnits, bool pushIntoStack = true) = 0;

  // multi-thread
  virtual void pushWorkUnit(const WorkUnitPtr& workUnit, int* counterToDecrementWhenDone = NULL, bool pushIntoStack = true)
    {jassert(isMultiThread());}
  virtual void waitUntilAllWorkUnitsAreDone()
    {jassert(isMultiThread());}

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

  static int random(LuaState& state);

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

extern ExecutionContextPtr distributedExecutionContext(ExecutionContext& parentContext, const String& remoteHostName, size_t remotePort);

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_H_
