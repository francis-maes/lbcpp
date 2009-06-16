/*-----------------------------------------.---------------------------------.
| Filename: IterationFunction.h            | A function that depends on an   |
| Author  : Francis Maes                   |     iteration number            |
| Started : 17/03/2009 16:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   IterationFunction.h
**@author Francis MAES
**@date   Fri Jun 12 18:26:12 2009
**
**@brief  #FIXME: all
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
** @brief
*/
class IterationFunction : public Object
{
public:
  /*!
  **
  **
  ** @param iteration
  **
  ** @return
  */
  virtual double compute(size_t iteration) const = 0;
};

/*!
**
**
** @param value
**
** @return
*/
extern IterationFunctionPtr constantIterationFunction(double value);

/*!
**
**
** @param initialValue
** @param numberIterationsToReachHalfInitialValue
**
** @return
*/
extern IterationFunctionPtr invLinearIterationFunction(double initialValue = 1.0, size_t numberIterationsToReachHalfInitialValue = 1000);

}; /* namespace lbcpp */

#endif // !LBCPP_ITERATION_FUNCTION_H_
