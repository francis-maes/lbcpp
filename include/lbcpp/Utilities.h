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
**@brief  #FIXME: all
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
** @brief
*/
class ErrorHandler
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual ~ErrorHandler() {}

  /*!
  **
  **
  ** @param where
  ** @param what
  */
  virtual void errorMessage(const std::string& where, const std::string& what) = 0;

  /*!
  **
  **
  ** @param where
  ** @param what
  */
  virtual void warningMessage(const std::string& where, const std::string& what) = 0;

  /*!
  **
  **
  ** @param handler
  */
  static void setInstance(ErrorHandler& handler);

  /*!
  **
  **
  **
  ** @return
  */
  static ErrorHandler& getInstance() {assert(instance); return *instance;}

  /*!
  **
  **
  ** @param where
  ** @param what
  */
  static void error(const std::string& where, const std::string& what)
    {getInstance().errorMessage(where, what);}

  /*!
  **
  **
  ** @param where
  ** @param what
  */
  static void warning(const std::string& where, const std::string& what)
    {getInstance().warningMessage(where, what);}

private:
  static ErrorHandler* instance; /*!< */
};


/*!
** @class ProgressCallback
** @brief
*/
class ProgressCallback : public ReferenceCountedObject
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual ~ProgressCallback() {}

  /*!
  **
  **
  ** @param description
  */
  virtual void progressStart(const std::string& description)
    {}

  // return false to stop the task
  /*!
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

  /*!
  **
  **
  */
  virtual void progressEnd()
    {}
};
typedef ReferenceCountedObjectPtr<ProgressCallback> ProgressCallbackPtr;

/*!
**
**
**
** @return
*/
extern ProgressCallbackPtr consoleProgressCallback();

}; /* namespace lbcpp */

#endif // !LBCPP_UTILITIES_H_
