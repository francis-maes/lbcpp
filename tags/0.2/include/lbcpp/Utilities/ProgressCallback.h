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
| Filename: ProgressCallback.h             | Progression Callback            |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2009 18:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_UTILITIES_PROGRESS_CALLBACK_H_
# define LBCPP_UTILITIES_PROGRESS_CALLBACK_H_

# include "../common.h"
# include "../Object/ReferenceCountedObject.h"

namespace lbcpp
{

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
typedef ReferenceCountedObjectPtr<ProgressCallback> ProgressCallbackPtr;

/** Creates a ProgressCallback that displays progression on the standard output.
**
** @return a new ProgressCallback.
*/
extern ProgressCallbackPtr consoleProgressCallback();

}; /* namespace lbcpp */

#endif // !LBCPP_UTILITIES_PROGRESS_CALLBACK_H_
