/*-----------------------------------------.---------------------------------.
| Filename: Utilities.h                    | Miscelaneous Utilities          |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2009 18:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_UTILITIES_H_
# define LBCPP_UTILITIES_H_

# include <string>
# include <cassert>
# include <sstream>
# include <vector>
# include "ReferenceCountedObject.h"

namespace lbcpp
{

class ErrorHandler
{
public:
  virtual ~ErrorHandler() {}
  virtual void errorMessage(const std::string& where, const std::string& what) = 0;
  virtual void warningMessage(const std::string& where, const std::string& what) = 0;

  static void setInstance(ErrorHandler& handler);
  static ErrorHandler& getInstance() {assert(instance); return *instance;}
  
  static void error(const std::string& where, const std::string& what)
    {getInstance().errorMessage(where, what);}

  static void warning(const std::string& where, const std::string& what)
    {getInstance().warningMessage(where, what);}
  
private:
  static ErrorHandler* instance;
};

class ProgressCallback : public ReferenceCountedObject
{
public:
  virtual ~ProgressCallback() {}
  
  virtual void progressStart(const std::string& description)
    {}
    
  // return false to stop the task
  virtual bool progressStep(const std::string& description, double iteration, double totalIterations = 0)
    {return true;}
    
  virtual void progressEnd()
    {}
};
typedef ReferenceCountedObjectPtr<ProgressCallback> ProgressCallbackPtr;

extern ProgressCallbackPtr consoleProgressCallback();

}; /* namespace lbcpp */

#endif // !LBCPP_UTILITIES_H_
