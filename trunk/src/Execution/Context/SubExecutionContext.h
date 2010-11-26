/*-----------------------------------------.---------------------------------.
| Filename: SubExecutionContext.h          | Base class for Execution Contexts|
| Author  : Francis Maes                   |   that have a parent            |
| Started : 24/11/2010 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CONTEXT_SUB_H_
# define LBCPP_EXECUTION_CONTEXT_SUB_H_

# include <lbcpp/Execution/ExecutionStack.h>
# include <lbcpp/Execution/ExecutionContext.h>

namespace lbcpp
{

class SubExecutionContext : public ExecutionContext
{
public:
  SubExecutionContext(ExecutionContext& parentContext)
    : parent(&parentContext) {}
  SubExecutionContext() {}

  virtual void informationCallback(const String& where, const String& what)
  {
    if (parent)
      parent->informationCallback(where, what);
    ExecutionContext::informationCallback(where, what);
  }

  virtual void warningCallback(const String& where, const String& what)
  {
    if (parent)
      parent->warningCallback(where, what);
    ExecutionContext::warningCallback(where, what);
  }

  virtual void errorCallback(const String& where, const String& what)
  {
    if (parent)
      parent->errorCallback(where, what);
    ExecutionContext::errorCallback(where, what);
  }

  virtual void statusCallback(const String& status)
  {
    if (parent)
      parent->statusCallback(status);
    ExecutionContext::statusCallback(status);
  }

  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit)
  {
    if (parent)
      parent->progressCallback(progression, progressionTotal, progressionUnit);
    ExecutionContext::progressCallback(progression, progressionTotal, progressionUnit);
  }

  virtual void preExecutionCallback(const WorkUnitPtr& workUnit)
  {
    if (parent)
      parent->preExecutionCallback(workUnit);
    ExecutionContext::preExecutionCallback(workUnit);
  }

  virtual void postExecutionCallback(const WorkUnitPtr& workUnit, bool result)
  {
    if (parent)
      parent->postExecutionCallback(workUnit, result);
    ExecutionContext::postExecutionCallback(workUnit, result);
  }

  virtual void resultCallback(const String& name, const Variable& value)
  {
    if (parent)
      parent->resultCallback(name, value);
    ExecutionContext::resultCallback(name, value);
  }

protected:
  friend class SubExecutionContextClass;

  ExecutionContext* parent;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_SUB_H_
