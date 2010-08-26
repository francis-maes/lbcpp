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
| Filename: Utilities.h                    | Misc Utilities                  |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 21:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_UTILITIES_H_
# define LBCPP_DATA_UTILITIES_H_

# include "../common.h"
# include <cmath>
# include <cfloat>

namespace lbcpp
{

inline std::ostream& operator <<(std::ostream& ostr, const String& value)
  {return ostr << (const char* )value;}

template<bool> struct StaticAssert;
template<> struct StaticAssert<true> {};

inline bool isNumberValid(double number)
{
#ifdef JUCE_WIN32
    return (number == number) && (number != DBL_MAX) && (number != -DBL_MAX);
#else
    return !std::isnan(number) && !std::isinf(number);
#endif
}

inline bool isNumberNearlyNull(double value, double epsilon = 0.00001)
  {return fabs(value) < epsilon;}

inline double normalizeAngle(double angle)
{
  double res = fmod(angle, M_2_TIMES_PI);
  if (res < -M_PI)
    res += M_2_TIMES_PI;
  else if (res > M_PI)
    res -= M_2_TIMES_PI;
  jassert(res >= -M_PI && res < M_PI);
  return res;
}

#ifdef JUCE_WIN32
inline double log2(double x)
{
  static const double oneOverLog2 = 1.0/log(2.0);
  return log(x) * oneOverLog2;
}
#endif // JUCE_WIN32


extern String getTypeName(const std::type_info& info);

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
class ProgressCallback
{
public:
  /** Destructor. */
  virtual ~ProgressCallback() {}

  /** This function is called when the task begins.
  **
  ** @param description : a string that describes the task which begins.
  */
  virtual void progressStart(const String& description)
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
  virtual bool progressStep(const String& description, double iteration, double totalIterations = 0)
    {return true;}

  /** This function is called at the end of the task. */
  virtual void progressEnd()
    {}
};

/** Creates a ProgressCallback that displays progression on the standard output.
**
** @return a new ProgressCallback.
*/
extern ProgressCallback& consoleProgressCallback();

/*!
** @class MessageCallback
** @brief Message Callback.
**
** The Message Callback is a singleton which receives all the
** error and warning messages produced by the library. By
** default, errors and warnings are displayed on the standard
** output. This behavior can be changed by overriding the MessageCallback
** class and by changing the singleton.
**
*/
class MessageCallback
{
public:
  /**
  ** Destructor.
  */
  virtual ~MessageCallback() {}

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
  ** MessageCallback instance setter.
  **
  ** @param handler : MessageCallback instance.
  */
  static void setInstance(MessageCallback& handler);

  /**
  ** MessageCallback instance getter.
  **
  ** @return the MessageCallback instance.
  */
  static MessageCallback& getInstance() {jassert(instance); return *instance;}

  /**
  ** Displays an error message using the MessageCallback singleton.
  **
  ** @param where : where the error occurs.
  ** @param what : what's going wrong.
  */
  static void error(const String& where, const String& what)
    {getInstance().errorMessage(where, what);}

  /**
  ** Displays a warning message using the MessageCallback singleton.
  **
  ** @param where : where the problem occurs.
  ** @param what : what's going wrong.
  */
  static void warning(const String& where, const String& what)
    {getInstance().warningMessage(where, what);}

private:
  static MessageCallback* instance;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_UTILITIES_H_

