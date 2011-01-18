/*-----------------------------------------.---------------------------------.
| Filename: SingleThreadedExecutionContext.h| Single-Threaded Execution      |
| Author  : Francis Maes                   | Context                         |
| Started : 24/11/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CONTEXT_SINGLE_THREADED_H_
# define LBCPP_EXECUTION_CONTEXT_SINGLE_THREADED_H_

# include <lbcpp/Execution/ExecutionContext.h>
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class SingleThreadedExecutionContext : public ExecutionContext
{
public:
  virtual String toString() const
    {return T("SingleThreaded");}

  virtual bool isMultiThread() const
    {return false;}

  virtual bool isCanceled() const
    {return false;}

  virtual bool isPaused() const
    {return false;}

  virtual bool run(const CompositeWorkUnitPtr& workUnits)
    {return ExecutionContext::run((WorkUnitPtr)workUnits);}
    
  lbcpp_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_SINGLE_THREADED_H_
