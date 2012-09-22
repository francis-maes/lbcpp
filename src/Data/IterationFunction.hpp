/*-----------------------------------------.---------------------------------.
| Filename: IterationFunction.hpp          | Iteration Functions             |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 19:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_ITERATION_CONSTANT_HPP
# define LBCPP_DATA_ITERATION_CONSTANT_HPP

# include <lbcpp/Data/IterationFunction.h>

namespace lbcpp
{

class ConstantIterationFunction : public IterationFunction
{
public:
  ConstantIterationFunction(double value = 0.0) : value(value) {}
  
  virtual double computeIterationFunction(size_t iteration) const
    {return value;}
    
private:
  friend class ConstantIterationFunctionClass;

  double value;
};

class InvLinearIterationFunction : public IterationFunction
{
public:
  InvLinearIterationFunction(double initialValue = 1.0, size_t numberIterationsToReachHalfInitialValue = 1000)
    : initialValue(initialValue), numberIterationsToReachHalfInitialValue(numberIterationsToReachHalfInitialValue) {}
    
  virtual double computeIterationFunction(size_t iteration) const
    {return initialValue * numberIterationsToReachHalfInitialValue / (double)(numberIterationsToReachHalfInitialValue + iteration);}

private:
  friend class InvLinearIterationFunctionClass;

  double initialValue;
  size_t numberIterationsToReachHalfInitialValue;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_ITERATION_CONSTANT_HPP
