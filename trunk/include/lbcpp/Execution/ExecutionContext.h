/*-----------------------------------------.---------------------------------.
| Filename: ExecutionContext.h             | Execution Context Base Class    |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 17:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CONTEXT_H_
# define LBCPP_EXECUTION_CONTEXT_H_

# include "ExecutionCallback.h"
# include "WorkUnit.h"

namespace lbcpp
{

class ExecutionContext : public ExecutionCallback
{
public:
  ExecutionContext();

  virtual bool isMultiThread() const = 0;

  /*
  ** ExecutionCallback
  */
  virtual void informationCallback(const String& where, const String& what);
  virtual void warningCallback(const String& where, const String& what);
  virtual void errorCallback(const String& where, const String& what);
  virtual void statusCallback(const String& status);
  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit);
  virtual void preExecutionCallback(const WorkUnitPtr& workUnit);
  virtual void postExecutionCallback(const WorkUnitPtr& workUnit, bool result);
  virtual void resultCallback(const String& name, const Variable& value);

  // shortcuts
  void informationCallback(const String& what)
    {informationCallback(String::empty, what);}

  void warningCallback(const String& what)
    {warningCallback(String::empty, what);}

  void errorCallback(const String& what)
    {errorCallback(String::empty, what);}

  /*
  ** Checks
  */
#ifdef JUCE_DEBUG
  bool checkInheritance(TypePtr type, TypePtr baseType);
  bool checkInheritance(const Variable& variable, TypePtr baseType);
#else
  inline bool checkInheritance(TypePtr type, TypePtr baseType) {return true;}
  inline bool checkInheritance(const Variable& variable, TypePtr baseType) {return true;}
#endif // JUCE_DEBUG

  /*
  ** Utilities
  */
  ObjectPtr createObject(ClassPtr objectClass);
  Variable createVariable(TypePtr type);

  void declareType(TypePtr typeInstance);
  void declareTemplateType(TemplateTypePtr templateTypeInstance);
  void finishTypeDeclarations();

  bool doTypeExists(const String& typeName);

  TypePtr getType(const String& typeName);
  TypePtr getType(const String& name, const std::vector<TypePtr>& arguments);
  EnumerationPtr getEnumeration(const String& className);

  /*
  ** Current State
  */
  virtual bool isCanceled() const = 0;
  virtual bool isPaused() const = 0;

  /*
  ** Work Units
  */
  virtual bool run(const WorkUnitPtr& workUnit);
  virtual bool run(const std::vector<WorkUnitPtr>& workUnits) = 0;

  /*
  ** Callbacks
  */
  void appendCallback(const ExecutionCallbackPtr& callback);
  void removeCallback(const ExecutionCallbackPtr& callback);
  void clearCallbacks();

  const std::vector<ExecutionCallbackPtr>& getCallbacks() const
    {return callbacks;}

  void setCallbacks(const std::vector<ExecutionCallbackPtr>& callbacks)
    {this->callbacks = callbacks;}

protected:
  friend class ExecutionContextClass;

  std::vector<ExecutionCallbackPtr> callbacks;
};

extern ExecutionContextPtr silentExecutionContext;

extern ExecutionContextPtr singleThreadedExecutionContext();
extern ExecutionContextPtr multiThreadedExecutionContext(size_t numThreads);

extern ExecutionContextPtr defaultConsoleExecutionContext();

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_H_
