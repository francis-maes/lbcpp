/*-----------------------------------------.---------------------------------.
| Filename: InvLinearIterationFunction.h   | InvLinear Iteration Function    |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 22:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_ITERATION_INV_LINEAR_H_
# define LBCPP_FUNCTION_ITERATION_INV_LINEAR_H_

# include <lbcpp/Function/IterationFunction.h>

namespace lbcpp
{

class InvLinearIterationFunction : public IterationFunction
{
public:
  InvLinearIterationFunction(double initialValue = 1.0, size_t numberIterationsToReachHalfInitialValue = 1000)
    : initialValue(initialValue), numberIterationsToReachHalfInitialValue(numberIterationsToReachHalfInitialValue) {}
    
  virtual double computeIterationFunction(size_t iteration) const
    {return initialValue * numberIterationsToReachHalfInitialValue / (double)(numberIterationsToReachHalfInitialValue + iteration);}

  virtual String toString() const
    {return "InvLinearIterationFunction(" + String(initialValue) + 
       ", " + String((int)numberIterationsToReachHalfInitialValue) + ")";}

private:
  friend class InvLinearIterationFunctionClass;

  double initialValue;
  size_t numberIterationsToReachHalfInitialValue;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_ITERATION_INV_LINEAR_H_
