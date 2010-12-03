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

namespace lbcpp
{

class ExecutionContext : public CompositeExecutionCallback
{
public:
  ExecutionContext();

  virtual bool isMultiThread() const = 0;

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

  ExecutionStackPtr getStack() const
    {return stack;}

  void setStack(const ExecutionStackPtr& stack)
    {this->stack = stack;}

  /*
  ** Work Units
  */
  virtual bool run(const WorkUnitPtr& workUnit);
  virtual bool run(const CompositeWorkUnitPtr& workUnits) = 0;

  // multi-thread
  virtual void pushWorkUnit(const WorkUnitPtr& workUnit)
    {jassert(isMultiThread());}
  virtual void waitUntilAllWorkUnitsAreDone()
    {jassert(isMultiThread());}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExecutionContextClass;

  ExecutionStackPtr stack;
};

extern ExecutionContextPtr singleThreadedExecutionContext();
extern ExecutionContextPtr multiThreadedExecutionContext(size_t numThreads);

extern ExecutionContextPtr defaultExecutionContext(bool noMultiThreading = false);
extern ExecutionContextPtr defaultConsoleExecutionContext(bool noMultiThreading = false);

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_H_
