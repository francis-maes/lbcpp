/*-----------------------------------------.---------------------------------.
| Filename: IterationFunction.h            | A function that depends on an   |
| Author  : Francis Maes                   |     iteration number            |
| Started : 17/03/2009 16:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
** @file   IterationFunction.h
** @author Francis MAES
** @date   Fri Jun 12 18:26:12 2009
**
** @brief IterationFunction is the class that represents a parameter
** that evolves <i>w.r.t.</i> iterations. Such parameters are for
** example used in online learning, in order to specify how the
** learning rate should evolve <i>w.r.t.</i> the number of
** corrections. An IterationFunction can be seen as a function from
** iteration numbers to scalars.
**
**
*/

#ifndef LBCPP_ITERATION_FUNCTION_H_
# define LBCPP_ITERATION_FUNCTION_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{

/*!
** @class IterationFunction
** @brief A function that depends on an iteration number.
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
** Creates a new IterativeFunction that decreases conversely
** proportionately.
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
** @return a new IterativeFunction.
*/
extern IterationFunctionPtr invLinearIterationFunction(double initialValue = 1.0, size_t numberIterationsToReachHalfInitialValue = 1000);

}; /* namespace lbcpp */

#endif // !LBCPP_ITERATION_FUNCTION_H_
