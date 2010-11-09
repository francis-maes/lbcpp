/*-----------------------------------------.---------------------------------.
| Filename: MessageCallback.cpp            | MessageCallback                 |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 17:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/lbcpp.h>

using namespace lbcpp;

/*
** ProgressCallback
*/
class ConsoleProgressCallback : public ProgressCallback
{
public:
  virtual void progressStart(const String& description)
    {std::cout << "=============== " << (const char* )description << " ===============" << std::endl;}
    
  // return false to stop the task
  virtual bool progressStep(const String& description, double iteration, double totalIterations = 0)
  {
    std::cout << "Step '" << (const char* )description << "' iteration = " << iteration;
    if (totalIterations)
      std::cout << " / " << totalIterations;
    std::cout << std::endl;
    return true;
  }
    
  virtual void progressEnd()
    {std::cout << "===========================================" << std::endl;}
};

ProgressCallback& lbcpp::consoleProgressCallback()
{
  static ConsoleProgressCallback callback;
  return callback;
}

/*
** MessageCallback
*/
class DefaultMessageCallback : public MessageCallback
{
public:
  virtual void errorMessage(const String& where, const String& what)
  {
    ScopedLock _(lock);
    std::cerr << "Error in '" << (const char* )where << "': " << (const char* )what << "." << std::endl;
    jassert(false);
  }
    
  virtual void warningMessage(const String& where, const String& what)
  {
    ScopedLock _(lock);
    std::cerr << "Warning in '" << (const char* )where << "': " << (const char* )what << "." << std::endl;
  }

  virtual void infoMessage(const String& where, const String& what)
  {
    ScopedLock _(lock);
    if (where.isNotEmpty())
      std::cout << where << ": " << what << std::endl;
    else
      std::cout << what << std::endl;
  }

private:
  CriticalSection lock;
};

static DefaultMessageCallback defaultMessageCallback;

MessageCallback* MessageCallback::instance = &defaultMessageCallback;

void MessageCallback::setInstance(MessageCallback& handler)
{
  instance = &handler;
}
