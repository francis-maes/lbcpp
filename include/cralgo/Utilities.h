/*-----------------------------------------.---------------------------------.
| Filename: Utilities.h                    | Miscelaneous Utilities          |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2009 18:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_UTILITIES_H_
# define CRALGO_UTILITIES_H_

# include <string>
# include <cassert>

namespace cralgo
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

class ProgressCallback
{
public:
  static ProgressCallback& getConsoleProgressCallback();

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

}; /* namespace cralgo */

#endif // !CRALGO_UTILITIES_H_
