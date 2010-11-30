/*-----------------------------------------.---------------------------------.
| Filename: ExecutionNotifications.h       | Execution Notifications         |
| Author  : Francis Maes                   |                                 |
| Started : 30/11/2010 18:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CALLBACK_NOTIFICATIONS_H_
# define LBCPP_EXECUTION_CALLBACK_NOTIFICATIONS_H_

# include <lbcpp/Execution/ExecutionCallback.h>
# include <lbcpp/Execution/Notification.h>

namespace lbcpp
{

class ExecutionNotification : public Notification
{
public:
  virtual void notify(const ExecutionCallbackPtr& target) = 0;

  virtual void notify(const ObjectPtr& target)
    {notify(target.staticCast<ExecutionCallback>());}
};

typedef ReferenceCountedObjectPtr<ExecutionNotification> ExecutionNotificationPtr;

class ExecutionProgressNotification : public ExecutionNotification
{
public:
  ExecutionProgressNotification(double progression, double progressionTotal, const String& progressionUnit)
    : progression(progression), progressionTotal(progressionTotal), progressionUnit(progressionUnit) {}
  ExecutionProgressNotification() : progression(0.0), progressionTotal(0.0) {}

  virtual void notify(const ExecutionCallbackPtr& target)
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
  ExecutionResultNotification(const String& name, const Variable& value)
    : name(name), value(value) {}
  ExecutionResultNotification() {}

  virtual void notify(const ExecutionCallbackPtr& target)
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
  ExecutionMessageNotification(ExecutionMessageType messageType, const String& what, const String& where = String::empty)
    : messageType(messageType), what(what), where(where) {}
  ExecutionMessageNotification() : messageType(errorMessageType) {}

  virtual void notify(const ExecutionCallbackPtr& target)
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

class WorkUnitExecutionWorkUnitNotification : public ExecutionNotification
{
public:
  WorkUnitExecutionWorkUnitNotification(const WorkUnitPtr& workUnit, bool isPostExecution, bool executionResult = false)
    : workUnit(workUnit), isPostExecution(isPostExecution), executionResult(executionResult) {}
  WorkUnitExecutionWorkUnitNotification() {}

  virtual void notify(const ExecutionCallbackPtr& target)
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

class WorkUnitsExecutionWorkUnitNotification : public ExecutionNotification
{
public:
  WorkUnitsExecutionWorkUnitNotification(const WorkUnitVectorPtr& workUnits, bool isPostExecution, bool executionResult = false)
    : workUnits(workUnits), isPostExecution(isPostExecution), executionResult(executionResult) {}
  WorkUnitsExecutionWorkUnitNotification() {}

  virtual void notify(const ExecutionCallbackPtr& target)
  {
    if (isPostExecution)
      target->postExecutionCallback(workUnits, executionResult);
    else
      target->preExecutionCallback(workUnits);
  }

protected:
  friend class WorkUnitsExecutionWorkUnitNotificationClass;

  WorkUnitVectorPtr workUnits;
  bool isPostExecution;
  bool executionResult;
};

class FunctionExecutionWorkUnitNotification : public ExecutionNotification
{
public:
  FunctionExecutionWorkUnitNotification(const FunctionPtr& function, const Variable& input, bool isPostExecution, const Variable& output = Variable())
    : function(function), input(input), isPostExecution(isPostExecution), output(output) {}
  FunctionExecutionWorkUnitNotification() {}

  virtual void notify(const ExecutionCallbackPtr& target)
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

class InferenceExecutionWorkUnitNotification : public ExecutionNotification
{
public:
  InferenceExecutionWorkUnitNotification(const InferencePtr& inference, const Variable& input, const Variable& supervision, bool isPostExecution, const Variable& output = Variable())
    : inference(inference), input(input), supervision(supervision), isPostExecution(isPostExecution), output(output) {}
  InferenceExecutionWorkUnitNotification () {}

  virtual void notify(const ExecutionCallbackPtr& target)
  {
    if (isPostExecution)
      target->postExecutionCallback(inference, input, supervision, output);
    else
      target->preExecutionCallback(inference, input, supervision);
  }

protected:
  friend class InferenceExecutionWorkUnitNotificationClass;

  InferencePtr inference;
  Variable input;
  Variable supervision;
  bool isPostExecution;
  Variable output;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXECUTION_CALLBACK_NOTIFIER_H_
