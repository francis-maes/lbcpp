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

namespace lbcpp
{

class ExecutionNotification : public Notification
{
public:
  virtual void notifyCallback(const ExecutionCallbackPtr& target) = 0;

  virtual void notify(const ObjectPtr& target)
  {
    const ExecutionCallbackPtr& callback = target.staticCast<ExecutionCallback>();
    callback->notificationCallback(refCountedPointerFromThis(this));
  }
};

typedef ReferenceCountedObjectPtr<ExecutionNotification> ExecutionNotificationPtr;

class ExecutionProgressNotification : public ExecutionNotification
{
public:
  ExecutionProgressNotification(const ProgressionStatePtr& progression)
    : progression(new ProgressionState(*progression)) {}
  ExecutionProgressNotification() {}

  virtual void notifyCallback(const ExecutionCallbackPtr& target)
    {target->progressCallback(progression);}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExecutionProgressNotificationClass;

  ProgressionStatePtr progression;
};

class ExecutionResultNotification : public ExecutionNotification
{
public:
  ExecutionResultNotification(const string& name, const ObjectPtr& value)
    : name(name), value(value) {}
  ExecutionResultNotification() {}

  virtual void notifyCallback(const ExecutionCallbackPtr& target)
    {target->resultCallback(name, value);}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExecutionResultNotificationClass;

  string name;
  ObjectPtr value;
};

class ExecutionMessageNotification : public ExecutionNotification
{
public:
  ExecutionMessageNotification(ExecutionMessageType messageType, const string& what, const string& where = string::empty)
    : messageType(messageType), what(what), where(where) {}
  ExecutionMessageNotification() : messageType(errorMessageType) {}

  virtual void notifyCallback(const ExecutionCallbackPtr& target)
  {
    switch (messageType)
    {
    case informationMessageType:  target->informationCallback(where, what); break;
    case warningMessageType:      target->warningCallback(where, what); break;
    case errorMessageType:        target->errorCallback(where, what); break;
    default: jassert(false);
    };
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExecutionMessageNotificationClass;

  ExecutionMessageType messageType;
  string what;
  string where;
};

class PreExecutionNotification : public ExecutionNotification
{
public:
  PreExecutionNotification(const ExecutionStackPtr& stack, const string& description, const WorkUnitPtr& workUnit)
    : stack(stack->cloneAndCast<ExecutionStack>()), description(description), workUnit(workUnit) {}
  PreExecutionNotification() {}

  virtual void notifyCallback(const ExecutionCallbackPtr& target)
    {target->preExecutionCallback(stack, description, workUnit);}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class PreExecutionNotificationClass;

  ExecutionStackPtr stack;
  string description;
  WorkUnitPtr workUnit;
};

class PostExecutionNotification : public ExecutionNotification
{
public:
  PostExecutionNotification(const ExecutionStackPtr& stack, const string& description, const WorkUnitPtr& workUnit, const ObjectPtr& result)
    : stack(stack->cloneAndCast<ExecutionStack>()), description(description), workUnit(workUnit), result(result) {}
  PostExecutionNotification() {}

  virtual void notifyCallback(const ExecutionCallbackPtr& target)
    {target->postExecutionCallback(stack, description, workUnit, result);}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class PostExecutionNotificationClass;

  ExecutionStackPtr stack;
  string description;
  WorkUnitPtr workUnit;
  ObjectPtr result;
};

class ThreadExecutionNotification : public ExecutionNotification
{
public:
  ThreadExecutionNotification(const ExecutionStackPtr& stack, bool isThreadEnd)
    : stack(stack->cloneAndCast<ExecutionStack>()), isThreadEnd(isThreadEnd) {}
  ThreadExecutionNotification() {}

  virtual void notifyCallback(const ExecutionCallbackPtr& target)
  {
    if (isThreadEnd)
      target->threadEndCallback(stack);
    else
      target->threadBeginCallback(stack);
  }

  bool isBeginCallback() const
    {return !isThreadEnd;}

  bool isEndCallback() const
    {return isThreadEnd;}

  const ExecutionStackPtr& getStack() const
    {return stack;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ThreadExecutionNotificationClass;
  ExecutionStackPtr stack;
  bool isThreadEnd;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXECUTION_CALLBACK_NOTIFIER_H_
