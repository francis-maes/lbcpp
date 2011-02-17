/*-----------------------------------------.---------------------------------.
| Filename: ConstantIterationFunction.h    | Constant Iteration Function     |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 22:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_ITERATION_CONSTANT_H_
# define LBCPP_FUNCTION_ITERATION_CONSTANT_H_

# include <lbcpp/Function/IterationFunction.h>

namespace lbcpp
{

class ConstantIterationFunction : public IterationFunction
{
public:
  ConstantIterationFunction(double value = 0.0) : value(value) {}
  
  virtual double computeIterationFunction(size_t iteration) const
    {return value;}
    
  virtual String toString() const
    {return "ConstantIterationFunction(" + String(value) + ")";}
  
private:
  friend class ConstantIterationFunctionClass;

  double value;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_ITERATION_CONSTANT_H_
