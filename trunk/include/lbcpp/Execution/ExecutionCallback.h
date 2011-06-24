/*-----------------------------------------.---------------------------------.
| Filename: ExecutionCallback.h            | Execution Callback Base Class   |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 17:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CALLBACK_H_
# define LBCPP_EXECUTION_CALLBACK_H_

# include "predeclarations.h"
# include "../Core/Object.h"
# include "../Lua/Lua.h"

namespace lbcpp
{

enum ExecutionMessageType
{
  informationMessageType,
  warningMessageType,
  errorMessageType
};

extern ClassPtr executionCallbackClass;

class ExecutionCallback : public Object
{
public:
  ExecutionCallback() : context(NULL) {}

  virtual void notificationCallback(const NotificationPtr& notification);

  /*
  ** Informations, warnings and Errors
  */
  virtual void informationCallback(const String& where, const String& what) {}
  virtual void warningCallback(const String& where, const String& what) {}
  virtual void errorCallback(const String& where, const String& what) {}

  // shortcuts
  void informationCallback(const String& what)
    {informationCallback(String::empty, what);}

  void warningCallback(const String& what)
    {warningCallback(String::empty, what);}

  void errorCallback(const String& what)
    {errorCallback(String::empty, what);}

  /*
  ** Progression
  */
  virtual void progressCallback(const ProgressionStatePtr& progression) {}

  /*
  ** Execution
  */
  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit) {}
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit, const Variable& result) {}

  virtual void threadBeginCallback(const ExecutionStackPtr& stack) {}
  virtual void threadEndCallback(const ExecutionStackPtr& stack) {}

  /*
  ** Results
  */
  virtual void resultCallback(const String& name, const Variable& value) {}

  /*
  ** Context
  */
  virtual void initialize(ExecutionContext& context)
    {this->context = &context;}

  ExecutionContext& getContext()
    {jassert(context); return *context;}

  /*
  ** Lua
  */
  static int error(LuaState& state);
  static int warning(LuaState& state);
  static int information(LuaState& state);

  static int progress(LuaState& state);
  static int result(LuaState& state);

  lbcpp_UseDebuggingNewOperator

protected:
  ExecutionContext* context;

  static void getThisWhereAndWhat(LuaState& state, ExecutionCallbackPtr& pthis, String& where, String& what);
};

extern ExecutionCallbackPtr consoleExecutionCallback();
extern ExecutionCallbackPtr userInterfaceExecutionCallback();
extern ExecutionCallbackPtr makeTraceExecutionCallback(ExecutionTracePtr trace);
extern ExecutionCallbackPtr makeAndAutoSaveTraceExecutionCallback(ExecutionTracePtr trace, double autoSaveIntervalInSeconds, const File& file);

class CompositeExecutionCallback : public ExecutionCallback
{
public:
  virtual void notificationCallback(const NotificationPtr& notification);

  virtual void informationCallback(const String& where, const String& what);
  virtual void warningCallback(const String& where, const String& what);
  virtual void errorCallback(const String& where, const String& what);

  // shortcuts
  void informationCallback(const String& what)
    {informationCallback(String::empty, what);}

  void warningCallback(const String& what)
    {warningCallback(String::empty, what);}

  void errorCallback(const String& what)
    {errorCallback(String::empty, what);}

  virtual void progressCallback(const ProgressionStatePtr& progression);

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit);
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit, const Variable& result);
  virtual void threadBeginCallback(const ExecutionStackPtr& stack);
  virtual void threadEndCallback(const ExecutionStackPtr& stack);

  virtual void resultCallback(const String& name, const Variable& value);


  void appendCallback(const ExecutionCallbackPtr& callback);
  void removeCallback(const ExecutionCallbackPtr& callback);
  void clearCallbacks();

  const std::vector<ExecutionCallbackPtr>& getCallbacks() const
    {return callbacks;}

  void setCallbacks(const std::vector<ExecutionCallbackPtr>& callbacks)
    {this->callbacks = callbacks;}

  size_t getNumCallbacks() const
    {return callbacks.size();}

  ExecutionCallbackPtr getCallback(size_t index) const
    {jassert(index < callbacks.size()); return callbacks[index];}

protected:
  friend class CompositeExecutionCallbackClass;

  std::vector<ExecutionCallbackPtr> callbacks;
};

typedef ReferenceCountedObjectPtr<CompositeExecutionCallback> CompositeExecutionCallbackPtr;

class DispatchByThreadExecutionCallback : public CompositeExecutionCallback
{
public:
  DispatchByThreadExecutionCallback() : mainThreadID(0) {}

  virtual ExecutionCallbackPtr createCallbackForThread(const ExecutionStackPtr& stack, Thread::ThreadID threadId) = 0;

  virtual void notificationCallback(const NotificationPtr& notification);

protected:
  CriticalSection callbacksByThreadLock;
  typedef std::map<Thread::ThreadID, std::vector<ExecutionCallbackPtr> > CallbackByThreadMap;
  CallbackByThreadMap callbacksByThread;
  Thread::ThreadID mainThreadID;

  std::vector<ExecutionCallbackPtr>& getCallbacksByThreadId(Thread::ThreadID threadID);
};

class ProgressionState : public Object
{
public:
  ProgressionState(double value, double total, const String& unit);
  ProgressionState(size_t value, size_t total, const String& unit);
  ProgressionState(double value, const String& unit);
  ProgressionState(const ProgressionState& other);
  ProgressionState();

  double getValue() const
    {return value;}

  void setValue(double value)
    {this->value = value;}

  void setValue(size_t value)
    {this->value = (double)value;}

  double getTotal() const
    {return total;}

  void setTotal(double total)
    {this->total = total;}

  bool isBounded() const
    {return total > 0.0;}

  double getNormalizedValue() const
    {jassert(isBounded()); return juce::jlimit(0.0, 1.0, value / total);}

  const String& getUnit() const
    {return unit;}

  void setUnit(const String& unit)
    {this->unit = unit;}

  // Object
  virtual String toString() const;
  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

private:
  friend class ProgressionStateClass;

  double value;
  double total;
  String unit;
};

typedef ReferenceCountedObjectPtr<ProgressionState> ProgressionStatePtr;
extern ClassPtr progressionStateClass;

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CALLBACK_H_
