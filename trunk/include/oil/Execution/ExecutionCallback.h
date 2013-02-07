/*-----------------------------------------.---------------------------------.
| Filename: ExecutionCallback.h            | Execution Callback Base Class   |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 17:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef OIL_EXECUTION_CALLBACK_H_
# define OIL_EXECUTION_CALLBACK_H_

# include "predeclarations.h"
# include "../Core/Object.h"

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
  virtual void informationCallback(const string& where, const string& what) {}
  virtual void warningCallback(const string& where, const string& what) {}
  virtual void errorCallback(const string& where, const string& what) {}

  // shortcuts
  void informationCallback(const string& what)
    {informationCallback(string::empty, what);}

  void warningCallback(const string& what)
    {warningCallback(string::empty, what);}

  void errorCallback(const string& what)
    {errorCallback(string::empty, what);}

  /*
  ** Progression
  */
  virtual void progressCallback(const ProgressionStatePtr& progression) {}

  /*
  ** Execution
  */
  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const string& description, const WorkUnitPtr& workUnit) {}
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const string& description, const WorkUnitPtr& workUnit, const ObjectPtr& result) {}

  virtual void threadBeginCallback(const ExecutionStackPtr& stack) {}
  virtual void threadEndCallback(const ExecutionStackPtr& stack) {}

  /*
  ** Results
  */
  virtual void resultCallback(const string& name, const ObjectPtr& value) {}
  
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

  static void getThisWhereAndWhat(LuaState& state, ExecutionCallbackPtr& pthis, string& where, string& what);
};

extern ExecutionCallbackPtr consoleExecutionCallback();
extern ExecutionCallbackPtr makeTraceExecutionCallback(ExecutionTracePtr trace);
extern ExecutionCallbackPtr makeAndAutoSaveTraceExecutionCallback(ExecutionTracePtr trace, double autoSaveIntervalInSeconds, const juce::File& file);

class CompositeExecutionCallback : public ExecutionCallback
{
public:
  virtual void notificationCallback(const NotificationPtr& notification);

  virtual void informationCallback(const string& where, const string& what);
  virtual void warningCallback(const string& where, const string& what);
  virtual void errorCallback(const string& where, const string& what);

  // shortcuts
  void informationCallback(const string& what)
    {informationCallback(string::empty, what);}

  void warningCallback(const string& what)
    {warningCallback(string::empty, what);}

  void errorCallback(const string& what)
    {errorCallback(string::empty, what);}

  virtual void progressCallback(const ProgressionStatePtr& progression);

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const string& description, const WorkUnitPtr& workUnit);
  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const string& description, const WorkUnitPtr& workUnit, const ObjectPtr& result);
  virtual void threadBeginCallback(const ExecutionStackPtr& stack);
  virtual void threadEndCallback(const ExecutionStackPtr& stack);

  virtual void resultCallback(const string& name, const ObjectPtr& value);

  void resultCallback(const string& name, bool value);
  void resultCallback(const string& name, juce::int64 value);
  void resultCallback(const string& name, size_t value);
  void resultCallback(const string& name, double value);
  void resultCallback(const string& name, const string& value);

  template<class T>
  void resultCallback(const string& name, const ReferenceCountedObjectPtr<T>& value)
    {resultCallback(name, ObjectPtr(value));}

  template<class T>
  void resultCallback(const string& name, const T* value)
    {resultCallback(name, ObjectPtr(value));}
  
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
  ProgressionState(double value, double total, const string& unit);
  ProgressionState(size_t value, size_t total, const string& unit);
  ProgressionState(double value, const string& unit);
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

  const string& getUnit() const
    {return unit;}

  void setUnit(const string& unit)
    {this->unit = unit;}

  // Object
  virtual string toString() const;
  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

private:
  friend class ProgressionStateClass;

  double value;
  double total;
  string unit;
};

typedef ReferenceCountedObjectPtr<ProgressionState> ProgressionStatePtr;
extern ClassPtr progressionStateClass;

}; /* namespace lbcpp */

#endif //!OIL_EXECUTION_CALLBACK_H_
