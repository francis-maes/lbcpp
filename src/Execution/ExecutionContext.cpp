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

namespace lbcpp
{

class ExecutionContext;
class WorkUnit : public NameableObject
{
public:
  WorkUnit(const String& name) : NameableObject(name) {}
  WorkUnit() {}

protected:
  virtual bool run(ExecutionContext& context) = 0;
};

typedef ReferenceCountedObjectPtr<WorkUnit> WorkUnitPtr;

class ExecutionContext : public ExecutionCallback
{
public:
  /*
  ** ExecutionCallback
  */
  virtual void informationCallback(const String& where, const String& what);
  virtual void warningCallback(const String& where, const String& what);
  virtual void errorCallback(const String& where, const String& what);
  virtual void statusCallback(const String& status);
  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit);

  /*
  ** Current State
  */
  virtual bool isCanceled() const = 0;
  virtual bool isPaused() const = 0;

  /*
  ** Work Units
  */
  virtual void run(WorkUnitPtr workUnit) = 0;
  virtual void run(const std::vector<WorkUnit>& workUnits) = 0;

  /*
  ** Callbacks
  */
  void appendCallback(const ExecutionCallbackPtr& callback);
  void removeCallback(const ExecutionCallbackPtr& callback);
  void clearCallbacks();

  const std::vector<ExecutionCallbackPtr>& getCallbacks() const
    {return callbacks;}

  void setCallbacks(const std::vector<ExecutionCallbackPtr>& callbacks)
    {this->callbacks = callbacks;}

private:
  friend class ExecutionContextClass;

  std::vector<ExecutionCallbackPtr> callbacks;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_H_
