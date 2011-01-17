/*-----------------------------------------.---------------------------------.
| Filename: ExecutionCallback.cpp          | Execution Callback Base Classes |
| Author  : Francis Maes                   |                                 |
| Started : 30/11/2010 18:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Execution/ExecutionCallback.h>
#include <lbcpp/Inference/Inference.h>
#include "Callback/ExecutionNotifications.h"
using namespace lbcpp;


/*
** ExecutionCallback
*/
void ExecutionCallback::notificationCallback(const NotificationPtr& notification)
{
  ExecutionNotificationPtr executionNotification = notification.staticCast<ExecutionNotification>();
  executionNotification->notifyCallback(refCountedPointerFromThis(this));
}


/*
** CompositeExecutionCallback
*/
void CompositeExecutionCallback::notificationCallback(const NotificationPtr& notification)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->notificationCallback(notification);
}

void CompositeExecutionCallback::informationCallback(const String& where, const String& what)
  {notificationCallback(new ExecutionMessageNotification(informationMessageType, what, where));}

void CompositeExecutionCallback::warningCallback(const String& where, const String& what)
  {notificationCallback(new ExecutionMessageNotification(warningMessageType, what, where));}

void CompositeExecutionCallback::errorCallback(const String& where, const String& what)
  {notificationCallback(new ExecutionMessageNotification(errorMessageType, what, where));}

void CompositeExecutionCallback::progressCallback(const ProgressionStatePtr& progression)
  {notificationCallback(new ExecutionProgressNotification(progression));}

void CompositeExecutionCallback::resultCallback(const String& name, const Variable& value)
  {notificationCallback(new ExecutionResultNotification(name, value));}

void CompositeExecutionCallback::preExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit)
  {notificationCallback(new PreExecutionNotification(stack, description, workUnit));}

void CompositeExecutionCallback::postExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit, bool result)
  {notificationCallback(new PostExecutionNotification(stack, description, workUnit, result));}

void CompositeExecutionCallback::threadBeginCallback(const ExecutionStackPtr& stack)
  {notificationCallback(new ThreadExecutionNotification(stack, false));}

void CompositeExecutionCallback::threadEndCallback(const ExecutionStackPtr& stack)
  {notificationCallback(new ThreadExecutionNotification(stack, true));}

void CompositeExecutionCallback::appendCallback(const ExecutionCallbackPtr& callback)
{
  jassert(callback);
  callback->initialize(*context);
  callbacks.push_back(callback);
}

void CompositeExecutionCallback::removeCallback(const ExecutionCallbackPtr& callback)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    if (callbacks[i] == callback)
    {
      callbacks.erase(callbacks.begin() + i);
      break;
    }
}

void CompositeExecutionCallback::clearCallbacks()
  {callbacks.clear();}

/*
** DispatchByThreadExecutionCallback
*/
void DispatchByThreadExecutionCallback::notificationCallback(const NotificationPtr& notification)
{
  Thread::ThreadID threadId = notification->getSourceThreadId();
  std::vector<ExecutionCallbackPtr>& callbacks = callbacksByThread[threadId];
  
  ReferenceCountedObjectPtr<ThreadExecutionNotification> threadNotification = notification.dynamicCast<ThreadExecutionNotification>();
  if (threadNotification)
  {
    if (threadNotification->isBeginCallback())
      callbacks.push_back(createCallbackForThread(threadNotification->getStack(), threadId));
    else
    {
      jassert(callbacks.size());
      callbacks.pop_back();
    }
  }
  else
  {
    if (callbacks.empty() && mainThreadID == 0)
    {
      mainThreadID = threadId;
      callbacks.push_back(createCallbackForThread(new ExecutionStack(), threadId));
    }
    jassert(callbacks.size());
    callbacks.back()->notificationCallback(notification);
  }
}
