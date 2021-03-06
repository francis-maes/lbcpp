/*-----------------------------------------.---------------------------------.
| Filename: MakeTraceExecutionCallback.h   | Make Trace Execution Callback   |
| Author  : Francis Maes                   |                                 |
| Started : 17/01/2011 12:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef OIL_EXECUTION_CALLBACK_MAKE_TRACE_H_
# define OIL_EXECUTION_CALLBACK_MAKE_TRACE_H_

# include <oil/Execution/ExecutionCallback.h>
# include <oil/Execution/ExecutionStack.h>
# include <oil/Execution/ExecutionTrace.h>

namespace lbcpp
{

class MakeTraceThreadExecutionCallback : public ExecutionCallback
{
public:
  MakeTraceThreadExecutionCallback(ExecutionTraceNodePtr parentItem, const juce::Time& startTime)
    : stack(1, parentItem), startTime(startTime), currentNotificationTime(0.0) {}
  MakeTraceThreadExecutionCallback() : currentNotificationTime(0.0) {}

  virtual void notificationCallback(const NotificationPtr& notification)
  {
    juce::Time notificationTime = notification->getConstructionTime();
    double time = (notificationTime.toMilliseconds() - startTime.toMilliseconds()) / 1000.0;
    if (time > currentNotificationTime)
      currentNotificationTime = time;
    ExecutionCallback::notificationCallback(notification);
  }

  virtual void informationCallback(const string& where, const string& what)
    {appendTraceItem(new MessageExecutionTraceItem(currentNotificationTime, informationMessageType, what, where));}

  virtual void warningCallback(const string& where, const string& what)
    {appendTraceItem(new MessageExecutionTraceItem(currentNotificationTime, warningMessageType, what, where));}

  virtual void errorCallback(const string& where, const string& what)
    {appendTraceItem(new MessageExecutionTraceItem(currentNotificationTime, errorMessageType, what, where));}

  virtual void progressCallback(const ProgressionStatePtr& progression)
  {
    ExecutionTraceNodePtr node = getCurrentNode();
    node->setProgression(progression);
    node->setEndTime(currentNotificationTime);
  }

  virtual void resultCallback(const string& name, const ObjectPtr& value)
    {getCurrentNode()->setResult(name, value);}

  virtual void preExecutionCallback(const ExecutionStackPtr& , const string& description, const WorkUnitPtr& workUnit)
  {
    jassert(stack.size());
    ExecutionTraceNodePtr newNode(new ExecutionTraceNode(description, workUnit, currentNotificationTime));
    appendTraceItem(newNode);
    stack.push_back(newNode);
  }

  virtual void postExecutionCallback(const ExecutionStackPtr& , const string& description, const WorkUnitPtr& workUnit, const ObjectPtr& result)
  {
    ExecutionTraceNodePtr finishedNode = getCurrentNode();
    finishedNode->setEndTime(currentNotificationTime);
    finishedNode->removeWorkUnit();
    finishedNode->setReturnValue(result);
    jassert(stack.size());
    stack.pop_back();
    jassert(stack.size());
  }

protected:
  std::vector<ExecutionTraceNodePtr> stack;
  juce::Time startTime;

  double currentNotificationTime;

  ExecutionTraceNodePtr getCurrentNode() const
    {jassert(stack.size()); return stack.back();}

  virtual void appendTraceItem(ExecutionTraceItemPtr item)
    {getCurrentNode()->appendSubItem(item);}
};

class MakeTraceExecutionCallback : public DispatchByThreadExecutionCallback
{
public:
  MakeTraceExecutionCallback(ExecutionTracePtr trace = ExecutionTracePtr())
    : trace(trace) {}

  virtual ExecutionCallbackPtr createCallbackForThread(const ExecutionStackPtr& stack, Thread::ThreadID threadId)
  {
    ExecutionTraceNodePtr traceNode = trace->findNode(stack);
    if (!traceNode)
      traceNode = trace->getRootNode();
    jassert(traceNode);
    return new MakeTraceThreadExecutionCallback(traceNode, trace->getStartTime());
  }

protected:
  friend class MakeTraceExecutionCallbackClass;

  ExecutionTracePtr trace;
};

}; /* namespace lbcpp */

#endif //!OIL_EXECUTION_CALLBACK_CONSOLE_H_
