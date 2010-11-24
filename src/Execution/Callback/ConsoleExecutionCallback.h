/*-----------------------------------------.---------------------------------.
| Filename: ConsoleExecutionCallback.h     | Console Execution Callback      |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 17:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CALLBACK_CONSOLE_H_
# define LBCPP_EXECUTION_CALLBACK_CONSOLE_H_

# include <lbcpp/Execution/ExecutionCallback.h>

namespace lbcpp
{

class ConsoleExecutionCallback : public ExecutionCallback
{
public:
  virtual void informationCallback(const String& where, const String& what)
  {
    ScopedLock _(lock);
    if (where.isNotEmpty())
      std::cout << where << ": " << what << std::endl;
    else
      std::cout << what << std::endl;
  }

  virtual void warningCallback(const String& where, const String& what)
  {
    ScopedLock _(lock);
    std::cerr << "Warning in '" << (const char* )where << "': " << (const char* )what << "." << std::endl;
  }

  virtual void errorCallback(const String& where, const String& what)
  {
    ScopedLock _(lock);
    std::cerr << "Error in '" << (const char* )where << "': " << (const char* )what << "." << std::endl;
    jassert(false);
  }
    
  virtual void statusCallback(const String& status)
  {
    ScopedLock _(lock);
    std::cout << "Status: " << status << std::endl;
  }

  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit)
  {
    ScopedLock _(lock);
    std::cout << "Progression: " << progression;
    if (progressionTotal)
      std::cout << " / " << progressionTotal;
    std::cout << " " << progressionUnit << std::endl;
  }

private:
  CriticalSection lock;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CALLBACK_CONSOLE_H_
