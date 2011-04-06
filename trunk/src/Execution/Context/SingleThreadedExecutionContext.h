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
  SingleThreadedExecutionContext(const File& projectDirectory)
    : ExecutionContext(projectDirectory) 
  {
#ifdef DEBUG_PURE_VIRTUAL
    std::cout << "SingleThreadedExecutionContext constructor" << std::endl;
#endif
  }
  SingleThreadedExecutionContext() {}
  
#ifdef DEBUG_PURE_VIRTUAL  
  virtual ~SingleThreadedExecutionContext()
    {std::cout << "SingleThreadedExecutionContext destructor" << std::endl;}
#endif

  virtual String toString() const
    {return T("SingleThreaded");}

  virtual bool isMultiThread() const
    {return false;}

  virtual bool isCanceled() const
    {return false;}

  virtual bool isPaused() const
    {return false;}

  virtual Variable run(const CompositeWorkUnitPtr& workUnits, bool pushIntoStack = true)
    {return ExecutionContext::run((WorkUnitPtr)workUnits, pushIntoStack);}

  lbcpp_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_SINGLE_THREADED_H_
