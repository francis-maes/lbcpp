/*-----------------------------------------.---------------------------------.
| Filename: IterationFunction.h            | A function that depends on an   |
| Author  : Francis Maes                   |     iteration number            |
| Started : 17/03/2009 16:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_ITERATION_FUNCTION_H_
# define CRALGO_ITERATION_FUNCTION_H_

# include "Object.h"

namespace cralgo
{

class IterationFunction : public Object
{
public:
  static IterationFunctionPtr createConstant(double value);
  static IterationFunctionPtr createInvLinear(double initialValue = 1.0, size_t numberIterationsToReachHalfInitialValue = 1000);

  virtual double compute(size_t iteration) const = 0;
};

}; /* namespace cralgo */

#endif // !CRALGO_ITERATION_FUNCTION_H_
