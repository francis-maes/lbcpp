/*-----------------------------------------.---------------------------------.
| Filename: Utilities.h                    | Miscelaneous Utilities          |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2009 18:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   Utilities.h
**@author Francis MAES
**@date   Sat Jun 13 18:03:22 2009
**
**@brief Utilities, especially the ErrorHandler.
**
**
*/

#ifndef LBCPP_UTILITIES_H_
# define LBCPP_UTILITIES_H_

# include <string>
# include <cassert>
# include <sstream>
# include <vector>
# include "ReferenceCountedObject.h"

namespace lbcpp
{

/*!
** @class ErrorHandler
** @brief Error handler.
*/
class ErrorHandler
{
public:
  /**
  ** Destructor.
  */
  virtual ~ErrorHandler() {}

  /**
  ** Displays an error message.
  **
  ** @param where : where the error occurs.
  ** @param what : what's going wrong.
  */
  virtual void errorMessage(const std::string& where, const std::string& what) = 0;

  /**
  ** Displays a warning message.
  **
  ** @param where : where the problem occurs.
  ** @param what : what's going wrong.
  */
  virtual void warningMessage(const std::string& where, const std::string& what) = 0;

  /**
  ** ErrorHandler instance setter.
  **
  ** @param handler : ErrorHandler instance.
  */
  static void setInstance(ErrorHandler& handler);

  /**
  ** ErrorHandler instance getter.
  **
  ** @return the ErrorHandler instance.
  */
  static ErrorHandler& getInstance() {assert(instance); return *instance;}

  /**
  ** Displays an error message.
  **
  ** @param where : where the error occurs.
  ** @param what : what's going wrong.
  */
  static void error(const std::string& where, const std::string& what)
    {getInstance().errorMessage(where, what);}

  /**
  ** Displays a warning message.
  **
  ** @param where : where the problem occurs.
  ** @param what : what's going wrong.
  */
  static void warning(const std::string& where, const std::string& what)
    {getInstance().warningMessage(where, what);}

private:
  static ErrorHandler* instance;
};


/**
** @class ProgressCallback
** @brief
*/
class ProgressCallback : public ReferenceCountedObject
{
public:
  /**
  **
  **
  **
  ** @return
  */
  virtual ~ProgressCallback() {}

  /**
  **
  **
  ** @param description
  */
  virtual void progressStart(const std::string& description)
    {}

  // return false to stop the task
  /**
  **
  **
  ** @param description
  ** @param iteration
  ** @param totalIterations
  **
  ** @return
  */
  virtual bool progressStep(const std::string& description, double iteration, double totalIterations = 0)
    {return true;}

  /**
  **
  **
  */
  virtual void progressEnd()
    {}
};
typedef ReferenceCountedObjectPtr<ProgressCallback> ProgressCallbackPtr;

/**
**
**
**
** @return
*/
extern ProgressCallbackPtr consoleProgressCallback();

}; /* namespace lbcpp */

#endif // !LBCPP_UTILITIES_H_
