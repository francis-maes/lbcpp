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
# include "WorkUnit.h"

namespace lbcpp
{

class ExecutionContext : public ExecutionCallback
{
public:
  ExecutionContext();

  /*
  ** ExecutionCallback
  */
  virtual void informationCallback(const String& where, const String& what);
  virtual void warningCallback(const String& where, const String& what);
  virtual void errorCallback(const String& where, const String& what);
  virtual void statusCallback(const String& status);
  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit);
  virtual void preExecutionCallback(const WorkUnitPtr& workUnit);
  virtual void postExecutionCallback(const WorkUnitPtr& workUnit, bool result);
  virtual void resultCallback(const String& name, const Variable& value);

  /*
  ** Current State
  */
  virtual bool isCanceled() const = 0;
  virtual bool isPaused() const = 0;

  /*
  ** Work Units
  */
  virtual bool run(const WorkUnitPtr& workUnit);
  virtual bool run(const std::vector<WorkUnitPtr>& workUnits) = 0;

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
