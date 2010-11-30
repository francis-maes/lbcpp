/*-----------------------------------------.---------------------------------.
| Filename: NotifierExecutionCallback.h    | Notifier Execution Callback     |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 19:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CALLBACK_NOTIFIER_H_
# define LBCPP_EXECUTION_CALLBACK_NOTIFIER_H_

# include <lbcpp/Execution/ExecutionCallback.h>
# include <lbcpp/Execution/Notification.h>

namespace lbcpp
{

class ExecutionNotification : public Notification
{
public:
  ExecutionNotification(ExecutionCallbackPtr target)
    : target(target) {}
  ExecutionNotification() {}

  virtual void notify() {}

protected:
  friend class ExecutionNotificationClass;

  ExecutionCallbackPtr target;
};

typedef ReferenceCountedObjectPtr<ExecutionNotification> ExecutionNotificationPtr;

class ExecutionProgressNotification : public ExecutionNotification
{
public:
  ExecutionProgressNotification(ExecutionCallbackPtr target, double progression, double progressionTotal, const String& progressionUnit)
    : ExecutionNotification(target), progression(progression), progressionTotal(progressionTotal), progressionUnit(progressionUnit) {}
  ExecutionProgressNotification() : progression(0.0), progressionTotal(0.0) {}

  virtual void notify()
    {target->progressCallback(progression, progressionTotal, progressionUnit);}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExecutionProgressNotificationClass;

  double progression;
  double progressionTotal;
  String progressionUnit;
};

class ExecutionResultNotification : public ExecutionNotification
{
public:
  ExecutionResultNotification(ExecutionCallbackPtr target, const String& name, const Variable& value)
    : ExecutionNotification(target), name(name), value(value) {}
  ExecutionResultNotification() {}

  virtual void notify()
    {target->resultCallback(name, value);}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExecutionResultNotificationClass;

  String name;
  Variable value;
};

enum ExecutionMessageType
{
  informationMessageType,
  warningMessageType,
  errorMessageType,
  statusMessageType
};

class ExecutionMessageNotification : public ExecutionNotification
{
public:
  ExecutionMessageNotification(ExecutionCallbackPtr target, ExecutionMessageType messageType, const String& what, const String& where = String::empty)
    : ExecutionNotification(target), messageType(messageType), what(what), where(where) {}
  ExecutionMessageNotification() : messageType(errorMessageType) {}

  virtual void notify()
  {
    switch (messageType)
    {
    case informationMessageType:  target->informationCallback(where, what); break;
    case warningMessageType:      target->warningCallback(where, what); break;
    case errorMessageType:        target->errorCallback(where, what); break;
    case statusMessageType:       target->statusCallback(what); break;
    default: jassert(false);
    };
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExecutionMessageNotificationClass;

  ExecutionMessageType messageType;
  String what;
  String where;
};

class WorkUnitsExecutionWorkUnitNotification : public ExecutionNotification
{
public:
  WorkUnitsExecutionWorkUnitNotification(ExecutionCallbackPtr target, const WorkUnitVectorPtr& workUnits, bool isPostExecution)
    : ExecutionNotification(target), workUnits(workUnits), isPostExecution(isPostExecution) {}
  WorkUnitsExecutionWorkUnitNotification() {}

  virtual void notify()
  {
    if (isPostExecution)
      target->postExecutionCallback(workUnits);
    else
      target->preExecutionCallback(workUnits);
  }

protected:
  friend class WorkUnitsExecutionWorkUnitNotificationClass;

  WorkUnitVectorPtr workUnits;
  bool isPostExecution;
};

class WorkUnitExecutionWorkUnitNotification : public ExecutionNotification
{
public:
  WorkUnitExecutionWorkUnitNotification(ExecutionCallbackPtr target, const WorkUnitPtr& workUnit, bool isPostExecution, bool executionResult = false)
    : ExecutionNotification(target), workUnit(workUnit), isPostExecution(isPostExecution), executionResult(executionResult) {}
  WorkUnitExecutionWorkUnitNotification() {}

  virtual void notify()
  {
    if (isPostExecution)
      target->postExecutionCallback(workUnit, executionResult);
    else
      target->preExecutionCallback(workUnit);
  }

protected:
  friend class WorkUnitExecutionWorkUnitNotificationClass;

  WorkUnitPtr workUnit;
  bool isPostExecution;
  bool executionResult;
};

class FunctionExecutionWorkUnitNotification : public ExecutionNotification
{
public:
  FunctionExecutionWorkUnitNotification(ExecutionCallbackPtr target, const FunctionPtr& function, const Variable& input, bool isPostExecution, const Variable& output = Variable())
    : ExecutionNotification(target), function(function), input(input), isPostExecution(isPostExecution), output(output) {}
  FunctionExecutionWorkUnitNotification() {}

  virtual void notify()
  {
    if (isPostExecution)
      target->postExecutionCallback(function, input, output);
    else
      target->preExecutionCallback(function, input);
  }

protected:
  friend class FunctionExecutionWorkUnitNotificationClass;

  FunctionPtr function;
  Variable input;
  bool isPostExecution;
  Variable output;
};

class NotifierExecutionCallback : public ExecutionCallback
{
public:
  NotifierExecutionCallback(ExecutionContext& context, ConsumerPtr notificationsConsumer, ExecutionCallbackPtr target)
    : context(context), notificationsConsumer(notificationsConsumer), target(target) {}
  NotifierExecutionCallback() : context(*silentExecutionContext) {}

  virtual void informationCallback(const String& where, const String& what)
    {notify(new ExecutionMessageNotification(target, informationMessageType, what, where));}

  virtual void warningCallback(const String& where, const String& what)
    {notify(new ExecutionMessageNotification(target, warningMessageType, what, where));}

  virtual void errorCallback(const String& where, const String& what)
    {notify(new ExecutionMessageNotification(target, errorMessageType, what, where));}

  virtual void statusCallback(const String& status)
    {notify(new ExecutionMessageNotification(target, statusMessageType, status));}

  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit)
    {notify(new ExecutionProgressNotification(target, progression, progressionTotal, progressionUnit));}

  virtual void resultCallback(const String& name, const Variable& value)
    {notify(new ExecutionResultNotification(target, name, value));}

  virtual void preExecutionCallback(const WorkUnitVectorPtr& workUnits)
    {notify(new WorkUnitsExecutionWorkUnitNotification(target, workUnits, false));}

  virtual void postExecutionCallback(const WorkUnitVectorPtr& workUnits)
    {notify(new WorkUnitsExecutionWorkUnitNotification(target, workUnits, true));}

  virtual void preExecutionCallback(const WorkUnitPtr& workUnit)
    {notify(new WorkUnitExecutionWorkUnitNotification(target, workUnit, false));}

  virtual void postExecutionCallback(const WorkUnitPtr& workUnit, bool result)
    {notify(new WorkUnitExecutionWorkUnitNotification(target, workUnit, true, result));}

  virtual void preExecutionCallback(const FunctionPtr& function, const Variable& input)
    {notify(new FunctionExecutionWorkUnitNotification(target, function, input, false));}

  virtual void postExecutionCallback(const FunctionPtr& function, const Variable& input, const Variable& output)
    {notify(new FunctionExecutionWorkUnitNotification(target, function, input, true, output));}

private:
  friend class NotifierExecutionCallbackClass;

  ExecutionContext& context;
  ConsumerPtr notificationsConsumer;
  ExecutionCallbackPtr target;

  void notify(NotificationPtr notification)
    {notificationsConsumer->consume(context, notification);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXECUTION_CALLBACK_NOTIFIER_H_
