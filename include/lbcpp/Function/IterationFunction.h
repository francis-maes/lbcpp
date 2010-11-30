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
| Filename: IterationFunction.h            | A function that depends on an   |
| Author  : Francis Maes                   |     iteration number            |
| Started : 17/03/2009 16:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ITERATION_FUNCTION_H_
# define LBCPP_ITERATION_FUNCTION_H_

# include "../Core/Variable.h"
# include "predeclarations.h"

namespace lbcpp
{

/*!
** @class IterationFunction
** @brief A function that depends on an iteration number.
**
** IterationFunction is the class that represents a parameter
** that evolves <i>w.r.t.</i> iterations. Such parameters are for
** example used in online learning, in order to specify how the
** learning rate should evolve <i>w.r.t.</i> the number of
** corrections. An IterationFunction can be seen as a function from
** iteration numbers to scalars.
**
*/
class IterationFunction : public Object
{
public:
  /*!
  ** Computes @a iteration iterations.
  **
  ** @param iteration : iteration number.
  **
  ** @return the result of @a iteration iterations.
  */
  virtual double compute(size_t iteration) const = 0;
};

/*!
** Creates a constant IterationFunction.
**
** This function creates a new IterationFunction that always returns
** the constant value @a value.
**
** @param value : constant value to return.
**
** @returns a new IterationFunction.
*/
extern IterationFunctionPtr constantIterationFunction(double value);

/*!
** Creates a new IterativeFunction that decreases
** with an inversly linear function.
*
** Defined by :
** - f(0) = @a initialValue
** - f(@a numberIterationsToReachHalfInitialValue) = @a initialValue/2
** - f(i) = @a initialValue * @a numberIterationsToReachHalfInitialValue /
** (double)(@a numberIterationsToReachHalfInitialValue + i);
**
**
** @param initialValue : initial value.
** @param numberIterationsToReachHalfInitialValue : number of
** iteration to reach half initial value.
**
** @return a new IterationFunction.
*/
extern IterationFunctionPtr invLinearIterationFunction(double initialValue = 1.0, size_t numberIterationsToReachHalfInitialValue = 1000);

}; /* namespace lbcpp */

#endif // !LBCPP_ITERATION_FUNCTION_H_
