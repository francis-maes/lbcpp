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
  {notification->notify(refCountedPointerFromThis(this));}


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

void CompositeExecutionCallback::statusCallback(const String& status)
  {notificationCallback(new ExecutionMessageNotification(statusMessageType, status));}

void CompositeExecutionCallback::progressCallback(double progression, double progressionTotal, const String& progressionUnit)
  {notificationCallback(new ExecutionProgressNotification(progression, progressionTotal, progressionUnit));}

void CompositeExecutionCallback::resultCallback(const String& name, const Variable& value)
  {notificationCallback(new ExecutionResultNotification(name, value));}

void CompositeExecutionCallback::preExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit)
  {notificationCallback(new PreExecutionNotification(stack, workUnit));}

void CompositeExecutionCallback::postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit, bool result)
  {notificationCallback(new PostExecutionNotification(stack, workUnit, result));}

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
