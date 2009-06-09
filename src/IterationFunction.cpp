/*-----------------------------------------.---------------------------------.
| Filename: IterationFunction.cpp          | Iteration Functions             |
| Author  : Francis Maes                   |                                 |
| Started : 09/06/2009 16:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/IterationFunction.h>
using namespace lbcpp;

class ConstantIterationFunction : public IterationFunction
{
public:
  ConstantIterationFunction(double value = 0.0) : value(value) {}
  
  virtual double compute(size_t iteration) const
    {return value;}
    
  virtual void save(std::ostream& ostr) const
    {write(ostr, value);}
    
  virtual bool load(std::istream& istr)
    {return read(istr, value);}

  virtual std::string toString() const
    {return "ConstantIterationFunction(" + lbcpp::toString(value) + ")";}
    
private:
  double value;
};

IterationFunctionPtr lbcpp::constantIterationFunction(double value)
  {return new ConstantIterationFunction(value);}

class InvLinearIterationFunction : public IterationFunction
{
public:
  InvLinearIterationFunction(double initialValue = 1.0, size_t numberIterationsToReachHalfInitialValue = 1000)
    : initialValue(initialValue), numberIterationsToReachHalfInitialValue(numberIterationsToReachHalfInitialValue) {}
    
  virtual double compute(size_t iteration) const
    {return initialValue * numberIterationsToReachHalfInitialValue / (double)(numberIterationsToReachHalfInitialValue + iteration);}

  virtual void save(std::ostream& ostr) const
    {write(ostr, initialValue); write(ostr, numberIterationsToReachHalfInitialValue);}
    
  virtual bool load(std::istream& istr)
    {return read(istr, initialValue) && read(istr, numberIterationsToReachHalfInitialValue);}

  virtual std::string toString() const
    {return "InvLinearIterationFunction(" + lbcpp::toString(initialValue) + 
       ", " + lbcpp::toString(numberIterationsToReachHalfInitialValue) + ")";}

private:
  double initialValue;
  size_t numberIterationsToReachHalfInitialValue;
};

IterationFunctionPtr lbcpp::invLinearIterationFunction(double initialValue, size_t numberIterationsToReachHalfInitialValue)
  {return new InvLinearIterationFunction(initialValue, numberIterationsToReachHalfInitialValue);}


/*
** Serializable classes declaration
*/
void declareIterationFunctions()
{
  LBCPP_DECLARE_CLASS(ConstantIterationFunction);
  LBCPP_DECLARE_CLASS(InvLinearIterationFunction);
}
