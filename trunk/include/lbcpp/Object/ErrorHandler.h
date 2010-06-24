/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: ErrorHandler.h                 | Object error handler            |
| Author  : Francis Maes                   |                                 |
| Started : 05/05/2010 11:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_ERROR_HANDLER_H_
# define LBCPP_OBJECT_ERROR_HANDLER_H_

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
  virtual void errorMessage(const String& where, const String& what) = 0;

  /**
  ** Displays a warning message.
  **
  ** @param where : where the problem occurs.
  ** @param what : what's going wrong.
  */
  virtual void warningMessage(const String& where, const String& what) = 0;

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
  static ErrorHandler& getInstance() {jassert(instance); return *instance;}

  /**
  ** Displays an error message using the ErrorManager singleton.
  **
  ** @param where : where the error occurs.
  ** @param what : what's going wrong.
  ** @see Object::error
  */
  static void error(const String& where, const String& what)
    {getInstance().errorMessage(where, what);}

  /**
  ** Displays a warning message using the ErrorManager singleton.
  **
  ** @param where : where the problem occurs.
  ** @param what : what's going wrong.
  ** @see Object::warning
  */
  static void warning(const String& where, const String& what)
    {getInstance().warningMessage(where, what);}

private:
  static ErrorHandler* instance;
};

/** Checks if a cast is valid and throw an error if not.
**
** @param where : a description of the caller function
** that will be used in case of an error.
** @param object : to object to cast.
** @return false is the loading fails, true otherwise. If loading fails,
** load() is responsible for declaring an error to the ErrorManager.
*/
template<class T>
inline ReferenceCountedObjectPtr<T> checkCast(const String& where, ReferenceCountedObjectPtr<ReferenceCountedObject> object)
{
  ReferenceCountedObjectPtr<T> res;
  if (object)
  {
    res = object.dynamicCast<T>();
    if (!res)
      ErrorHandler::error(where, T("Could not cast object into '") + lbcpp::toString(typeid(*res)) + T("'"));
  }
  return res;
}


}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_ERROR_HANDLER_H_
