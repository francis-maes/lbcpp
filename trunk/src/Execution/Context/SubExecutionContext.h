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

  virtual void preExecutionCallback(const WorkUnitVectorPtr& workUnits)
  {
    if (parent)
      parent->preExecutionCallback(workUnits);
    ExecutionContext::preExecutionCallback(workUnits);
  }

  virtual void postExecutionCallback(const WorkUnitVectorPtr& workUnits, bool result)
  {
    if (parent)
      parent->postExecutionCallback(workUnits, result);
    ExecutionContext::postExecutionCallback(workUnits, result);
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

  virtual void preExecutionCallback(const FunctionPtr& function, const Variable& input)
  {
    if (parent)
      parent->preExecutionCallback(function, input);
    preExecutionCallback(function, input);
  }

  virtual void postExecutionCallback(const FunctionPtr& function, const Variable& input, const Variable& output)
  {
    if (parent)
      parent->postExecutionCallback(function, input, output);
    postExecutionCallback(function, input, output);
  }


  virtual void preExecutionCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision)
  {
    if (parent)
      parent->preExecutionCallback(inference, input, supervision);
    preExecutionCallback(inference, input, supervision);
  }

  virtual void postExecutionCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& output)
  {
    if (parent)
      parent->postExecutionCallback(inference, input, supervision, output);
    postExecutionCallback(inference, input, supervision, output);
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
