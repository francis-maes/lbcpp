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
  errorMessageType
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
  ** Progression
  */
  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit) {}

  void progressCallback(double normalizedProgression)
    {progressCallback(normalizedProgression * 100.0, 100.0, T("%"));}

  /*
  ** Execution
  */
  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit) {}
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit, bool result) {}

  virtual void threadBeginCallback(const ExecutionStackPtr& stack) {}
  virtual void threadEndCallback(const ExecutionStackPtr& stack) {}

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

extern ExecutionCallbackPtr consoleExecutionCallback();
extern ExecutionCallbackPtr userInterfaceExecutionCallback();
extern ExecutionCallbackPtr makeTraceExecutionCallback(ExecutionTracePtr trace);

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

  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit);

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit);
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit, bool result);
  virtual void threadBeginCallback(const ExecutionStackPtr& stack);
  virtual void threadEndCallback(const ExecutionStackPtr& stack);

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

class DispatchByThreadExecutionCallback : public CompositeExecutionCallback
{
public:
  DispatchByThreadExecutionCallback() : mainThreadID(0) {}

  virtual ExecutionCallbackPtr createCallbackForThread(const ExecutionStackPtr& stack, Thread::ThreadID threadId) = 0;

  virtual void notificationCallback(const NotificationPtr& notification);

protected:
  typedef std::map<Thread::ThreadID, std::vector<ExecutionCallbackPtr> > CallbackByThreadMap;
  CallbackByThreadMap callbacksByThread;
  Thread::ThreadID mainThreadID;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CALLBACK_H_
