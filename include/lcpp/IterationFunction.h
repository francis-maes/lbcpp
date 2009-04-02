/*-----------------------------------------.---------------------------------.
| Filename: IterationFunction.h            | A function that depends on an   |
| Author  : Francis Maes                   |     iteration number            |
| Started : 17/03/2009 16:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_ITERATION_FUNCTION_H_
# define LCPP_ITERATION_FUNCTION_H_

# include "ObjectPredeclarations.h"

namespace lcpp
{

class IterationFunction : public Object
{
public:
  static IterationFunctionPtr createConstant(double value);
  static IterationFunctionPtr createInvLinear(double initialValue = 1.0, size_t numberIterationsToReachHalfInitialValue = 1000);

  virtual double compute(size_t iteration) const = 0;
};

}; /* namespace lcpp */

#endif // !LCPP_ITERATION_FUNCTION_H_
