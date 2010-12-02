/*-----------------------------------------.---------------------------------.
| Filename: ExecutionCallback.h            | Execution Callback Base Class   |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 17:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CALLBACK_H_
# define LBCPP_EXECUTION_CALLBACK_H_

# include "predeclarations.h"
# include "../Core/Object.h"

namespace lbcpp
{

enum ExecutionMessageType
{
  informationMessageType,
  warningMessageType,
  errorMessageType,
  statusMessageType
};

class ExecutionCallback : public Object
{
public:
  ExecutionCallback() : context(NULL) {}

  virtual void notificationCallback(const NotificationPtr& notification);

  /*
  ** Informations, warnings and Errors
  */
  virtual void informationCallback(const String& where, const String& what) {}
  virtual void warningCallback(const String& where, const String& what) {}
  virtual void errorCallback(const String& where, const String& what) {}

  // shortcuts
  void informationCallback(const String& what)
    {informationCallback(String::empty, what);}

  void warningCallback(const String& what)
    {warningCallback(String::empty, what);}

  void errorCallback(const String& what)
    {errorCallback(String::empty, what);}

  /*
  ** Status and Progression
  */
  virtual void statusCallback(const String& status) {}
  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit) {}

  void progressCallback(double normalizedProgression)
    {progressCallback(normalizedProgression * 100.0, 100.0, T("%"));}

  /*
  ** Execution
  */
  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit) {}
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit, bool result) {}

  /*
  ** Results
  */
  virtual void resultCallback(const String& name, const Variable& value) {}

  /*
  ** Context
  */
  virtual void initialize(ExecutionContext& context)
    {this->context = &context;}

  ExecutionContext& getContext()
    {jassert(context); return *context;}

  lbcpp_UseDebuggingNewOperator

protected:
  ExecutionContext* context;
};

class CompositeExecutionCallback : public ExecutionCallback
{
public:
  virtual void notificationCallback(const NotificationPtr& notification);

  virtual void informationCallback(const String& where, const String& what);
  virtual void warningCallback(const String& where, const String& what);
  virtual void errorCallback(const String& where, const String& what);

  // shortcuts
  void informationCallback(const String& what)
    {informationCallback(String::empty, what);}

  void warningCallback(const String& what)
    {warningCallback(String::empty, what);}

  void errorCallback(const String& what)
    {errorCallback(String::empty, what);}

  virtual void statusCallback(const String& status);
  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit);

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit);
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit, bool result);

  virtual void resultCallback(const String& name, const Variable& value);


  void appendCallback(const ExecutionCallbackPtr& callback);
  void removeCallback(const ExecutionCallbackPtr& callback);
  void clearCallbacks();

  const std::vector<ExecutionCallbackPtr>& getCallbacks() const
    {return callbacks;}

  void setCallbacks(const std::vector<ExecutionCallbackPtr>& callbacks)
    {this->callbacks = callbacks;}

  size_t getNumCallbacks() const
    {return callbacks.size();}

  ExecutionCallbackPtr getCallback(size_t index) const
    {jassert(index < callbacks.size()); return callbacks[index];}

protected:
  friend class CompositeExecutionCallbackClass;

  std::vector<ExecutionCallbackPtr> callbacks;
};

typedef ReferenceCountedObjectPtr<CompositeExecutionCallback> CompositeExecutionCallbackPtr;

extern ExecutionCallbackPtr silentExecutionCallback();
extern ExecutionCallbackPtr consoleExecutionCallback();
extern ExecutionCallbackPtr userInterfaceExecutionCallback();

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CALLBACK_H_
