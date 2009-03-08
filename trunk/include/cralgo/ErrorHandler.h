/*-----------------------------------------.---------------------------------.
| Filename: ErrorHandler.h                 | Error handling                  |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 17:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_ERROR_HANDLER_H_
# define CRALGO_ERROR_HANDLER_H_

# include <string>

namespace cralgo
{

class ErrorHandler
{
public:
  virtual ~ErrorHandler() {}
  virtual void errorMessage(const std::string& where, const std::string& what) = 0;

  static void setInstance(ErrorHandler& handler);
  
  static void error(const std::string& where, const std::string& what)
    {assert(instance); instance->errorMessage(where, what);}

private:
  static ErrorHandler* instance;
};

}; /* namespace cralgo */

#endif // !CRALGO_ERROR_HANDLER_H_
