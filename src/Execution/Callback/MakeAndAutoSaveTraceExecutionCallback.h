/*-----------------------------------------.---------------------------------.
| Filename: MakeAndAutoSaveTraceExecuti...h| Make a Trace And Auto Save it   |
| Author  : Francis Maes                   |                                 |
| Started : 25/03/2011 15:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CALLBACK_MAKE_AND_AUTO_SAVE_TRACE_H_
# define LBCPP_EXECUTION_CALLBACK_MAKE_AND_AUTO_SAVE_TRACE_H_

# include "MakeTraceExecutionCallback.h"

namespace lbcpp
{

class MakeAndAutoSaveTraceExecutionCallback : public MakeTraceExecutionCallback
{
public:
  MakeAndAutoSaveTraceExecutionCallback(ExecutionTracePtr trace, double autoSaveIntervalInSeconds, const File& file)
    : MakeTraceExecutionCallback(trace), saveInterval(autoSaveIntervalInSeconds), file(file), lastSaveTime(0.0) {}
  MakeAndAutoSaveTraceExecutionCallback() : saveInterval(0.0), lastSaveTime(0.0) {}

  virtual ExecutionCallbackPtr createCallbackForThread(const ExecutionStackPtr& stack, Thread::ThreadID threadId)
  {
    ExecutionTraceNodePtr traceNode = trace->findNode(stack);
    jassert(traceNode);
    return new MakeTraceThreadExecutionCallback(traceNode, trace->getStartTime());
  }

  virtual void notificationCallback(const NotificationPtr& notification)
  {
    MakeTraceExecutionCallback::notificationCallback(notification);
    {
      ScopedLock _(autoSaveLock);
      double time = Time::getMillisecondCounter() / 1000.0;
      if (saveInterval > 0.0 && (time - lastSaveTime > saveInterval))
      {
        lastSaveTime = time;
        autoSave();
      }
    }
  }
 
protected:
  friend class MakeAndAutoSaveTraceExecutionCallbackClass;

  CriticalSection autoSaveLock;
  double saveInterval;
  File file;

  double lastSaveTime;

  void autoSave()
  {
    ExecutionContext& context = getContext();
    std::cerr << "Saving execution trace into " << file.getFullPathName() << std::endl;
    trace->saveToFile(context, file);
  }
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CALLBACK_MAKE_AND_AUTO_SAVE_TRACE_H_
