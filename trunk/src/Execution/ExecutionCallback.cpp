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

void ExecutionCallback::preExecutionCallback(const ExecutionStackPtr& stack, const InferencePtr& inference, const Variable& input, const Variable& supervision)
  {preExecutionCallback(stack, inference, input);}

void ExecutionCallback::postExecutionCallback(const ExecutionStackPtr& stack, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& output)
  {postExecutionCallback(stack, inference, input, output);}

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

void CompositeExecutionCallback::preExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitVectorPtr& workUnits)
  {notificationCallback(new PreExecutionNotification(stack, workUnits));}

void CompositeExecutionCallback::postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitVectorPtr& workUnits, bool result)
  {notificationCallback(new PostExecutionNotification(stack, workUnits, result));}

void CompositeExecutionCallback::preExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit)
  {notificationCallback(new PreExecutionNotification(stack, workUnit));}

void CompositeExecutionCallback::postExecutionCallback(const ExecutionStackPtr& stack, const WorkUnitPtr& workUnit, bool result)
  {notificationCallback(new PostExecutionNotification(stack, workUnit, result));}

void CompositeExecutionCallback::preExecutionCallback(const ExecutionStackPtr& stack, const FunctionPtr& function, const Variable& input)
  {notificationCallback(new PreExecutionNotification(stack, function, input));}

void CompositeExecutionCallback::postExecutionCallback(const ExecutionStackPtr& stack, const FunctionPtr& function, const Variable& input, const Variable& output)
  {notificationCallback(new PostExecutionNotification(stack, function, output, input));}

void CompositeExecutionCallback::preExecutionCallback(const ExecutionStackPtr& stack, const InferencePtr& inference, const Variable& input, const Variable& supervision)
  {notificationCallback(new PreExecutionNotification(stack, inference, input, supervision));}

void CompositeExecutionCallback::postExecutionCallback(const ExecutionStackPtr& stack, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& output)
  {notificationCallback(new PostExecutionNotification(stack, inference, output, input, supervision));}

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
