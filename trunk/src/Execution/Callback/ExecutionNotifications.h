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
# include <lbcpp/Execution/ExecutionStack.h>
# include <lbcpp/Execution/Notification.h>
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Inference/Inference.h>

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

class PreExecutionNotification : public ExecutionNotification
{
public:
  PreExecutionNotification(const ExecutionStackPtr& stack, const ObjectPtr& object, const Variable& input = Variable(), const Variable& supervision = Variable())
    : stack(stack->cloneAndCast<ExecutionStack>()), object(object), input(input), supervision(supervision) {}
  PreExecutionNotification() {}

  virtual void notify(const ExecutionCallbackPtr& target)
  {
    WorkUnitVectorPtr workUnitVector = object.dynamicCast<WorkUnitVector>();
    if (workUnitVector)
    {
      target->preExecutionCallback(stack, workUnitVector);
      return;
    }

    WorkUnitPtr workUnit = object.dynamicCast<WorkUnit>();
    if (workUnit)
    {
      target->preExecutionCallback(stack, workUnit);
      return;
    }

    InferencePtr inference = object.dynamicCast<Inference>();
    if (inference)
    {
      target->preExecutionCallback(stack, inference, input, supervision);
      return;
    }

    FunctionPtr function = object.dynamicCast<Function>();
    if (function)
    {
      target->preExecutionCallback(stack, function, input);
      return;
    }
    jassert(false);
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class PreExecutionNotificationClass;

  ExecutionStackPtr stack;
  ObjectPtr object;
  Variable input;
  Variable supervision;
};

class PostExecutionNotification : public ExecutionNotification
{
public:
  PostExecutionNotification(const ExecutionStackPtr& stack, const ObjectPtr& object, const Variable& output, const Variable& input = Variable(), const Variable& supervision = Variable())
    : stack(stack->cloneAndCast<ExecutionStack>()), object(object), input(input), supervision(supervision), output(output) {}
  PostExecutionNotification() {}

  virtual void notify(const ExecutionCallbackPtr& target)
  {
    WorkUnitVectorPtr workUnitVector = object.dynamicCast<WorkUnitVector>();
    if (workUnitVector)
    {
      target->postExecutionCallback(stack, workUnitVector, output.getBoolean());
      return;
    }

    WorkUnitPtr workUnit = object.dynamicCast<WorkUnit>();
    if (workUnit)
    {
      target->postExecutionCallback(stack, workUnit, output.getBoolean());
      return;
    }

    InferencePtr inference = object.dynamicCast<Inference>();
    if (inference)
    {
      target->postExecutionCallback(stack, inference, input, supervision, output);
      return;
    }

    FunctionPtr function = object.dynamicCast<Function>();
    if (function)
    {
      target->postExecutionCallback(stack, function, input, output);
      return;
    }
    jassert(false);
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class PostExecutionNotificationClass;

  ExecutionStackPtr stack;
  ObjectPtr object;
  Variable input;
  Variable supervision;
  Variable output;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXECUTION_CALLBACK_NOTIFIER_H_
