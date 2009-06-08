/*-----------------------------------------.---------------------------------.
| Filename: IterationFunction.h            | A function that depends on an   |
| Author  : Francis Maes                   |     iteration number            |
| Started : 17/03/2009 16:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ITERATION_FUNCTION_H_
# define LBCPP_ITERATION_FUNCTION_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{

class IterationFunction : public Object
{
public:
  virtual double compute(size_t iteration) const = 0;
};

extern IterationFunctionPtr constantIterationFunction(double value);
extern IterationFunctionPtr invLinearIterationFunction(double initialValue = 1.0, size_t numberIterationsToReachHalfInitialValue = 1000);

}; /* namespace lbcpp */

#endif // !LBCPP_ITERATION_FUNCTION_H_
