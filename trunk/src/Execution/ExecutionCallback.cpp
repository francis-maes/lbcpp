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

void ExecutionCallback::preExecutionCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision)
  {preExecutionCallback(inference, input);}

void ExecutionCallback::postExecutionCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& output)
  {postExecutionCallback(inference, input, output);}

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

void CompositeExecutionCallback::preExecutionCallback(const WorkUnitVectorPtr& workUnits)
  {notificationCallback(new WorkUnitsExecutionWorkUnitNotification(workUnits, false));}

void CompositeExecutionCallback::postExecutionCallback(const WorkUnitVectorPtr& workUnits, bool result)
  {notificationCallback(new WorkUnitsExecutionWorkUnitNotification(workUnits, true, result));}

void CompositeExecutionCallback::preExecutionCallback(const WorkUnitPtr& workUnit)
  {notificationCallback(new WorkUnitExecutionWorkUnitNotification(workUnit, false));}

void CompositeExecutionCallback::postExecutionCallback(const WorkUnitPtr& workUnit, bool result)
  {notificationCallback(new WorkUnitExecutionWorkUnitNotification(workUnit, true, result));}

void CompositeExecutionCallback::preExecutionCallback(const FunctionPtr& function, const Variable& input)
  {notificationCallback(new FunctionExecutionWorkUnitNotification(function, input, false));}

void CompositeExecutionCallback::postExecutionCallback(const FunctionPtr& function, const Variable& input, const Variable& output)
  {notificationCallback(new FunctionExecutionWorkUnitNotification(function, input, true, output));}

void CompositeExecutionCallback::preExecutionCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision)
  {notificationCallback(new InferenceExecutionWorkUnitNotification(inference, input, supervision, false));}

void CompositeExecutionCallback::postExecutionCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& output)
  {notificationCallback(new InferenceExecutionWorkUnitNotification(inference, input, supervision, true, output));}

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
