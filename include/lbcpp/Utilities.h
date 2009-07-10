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
**
** The Error Handler is a singleton which receives all the
** error and warning messages produced by the library. By
** default, errors and warnings are displayed on the standard
** output. This behavior can be changed by overriding the ErrorHandler
** class and by changing the singleton.
** 
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
  ** Displays an error message using the ErrorManager singleton.
  **
  ** @param where : where the error occurs.
  ** @param what : what's going wrong.
  ** @see Object::error
  */
  static void error(const std::string& where, const std::string& what)
    {getInstance().errorMessage(where, what);}

  /**
  ** Displays a warning message using the ErrorManager singleton.
  **
  ** @param where : where the problem occurs.
  ** @param what : what's going wrong.
  ** @see Object::warning
  */
  static void warning(const std::string& where, const std::string& what)
    {getInstance().warningMessage(where, what);}

private:
  static ErrorHandler* instance;
};


/**
** @class ProgressCallback
** @brief A callback that receives information about
** the progression of a task. 
**
** This class is used to display to progression of
** a task that may eventually take a long time to complete.
** Such tasks may for example be optimization tasks or
** learning tasks.
*/
class ProgressCallback : public ReferenceCountedObject
{
public:
  /** Destructor. */
  virtual ~ProgressCallback() {}

  /** This function is called when the task begins.
  **
  ** @param description : a string that describes the task which begins.
  */
  virtual void progressStart(const std::string& description)
    {}

  /** This function is called each time the task progresses.
  **
  ** Some tasks have a fixed length, which makes it possible to compute 
  ** a percentage of progression. In this case the parameter 
  ** @a totalIterations indicates the length of the task.
  ** If the task's length is unknown in advance, the @a
  ** totalIterations parameter is equal to zero.
  **
  ** @param description : a string that describes the task.
  ** @param iteration : a progression indicator.
  ** @param totalIterations : the length of the task,
  ** <i>i.e.</i> the maximum value of @a iteration.
  **
  ** @return false to cancel the task or true to continue the task.
  */
  virtual bool progressStep(const std::string& description, double iteration, double totalIterations = 0)
    {return true;}

  /** This function is called at the end of the task. */
  virtual void progressEnd()
    {}
};
typedef ReferenceCountedObjectPtr<ProgressCallback> ProgressCallbackPtr;

/** Creates a ProgressCallback that displays progression on the standard output.
**
** @return a new ProgressCallback.
*/
extern ProgressCallbackPtr consoleProgressCallback();

}; /* namespace lbcpp */

#endif // !LBCPP_UTILITIES_H_
