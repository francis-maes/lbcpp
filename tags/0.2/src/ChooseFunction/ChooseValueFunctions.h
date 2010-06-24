/*-----------------------------------------.---------------------------------.
| Filename: ChooseValueFunction.h          | Value functions given to the    |
| Author  : Francis Maes                   |   choose call.                  |
| Started : 22/06/2009 19:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_VALUE_FUNCTION_CHOOSE_H_
# define LBCPP_CORE_VALUE_FUNCTION_CHOOSE_H_

# include <lbcpp/CRAlgorithm/ChooseFunction.h>

namespace lbcpp
{

class ChooseStateValueFunction : public StateValueFunction
{
public:
  virtual String toString() const
    {return "chooseStateValues()";}
    
  virtual void setChoose(ChoosePtr choose)
    {function = choose->getStateValueFunction();}

  virtual double compute() const
    {return function->compute();}
    
private:
  StateValueFunctionPtr function;     
};

class ChooseActionValueFunction : public ActionValueFunction
{
public:
  virtual String toString() const
    {return "chooseActionValues()";}
    
  virtual void setChoose(ChoosePtr choose)
    {function = choose->getActionValueFunction();}

  virtual double compute(VariablePtr choice) const
    {return function->compute(choice);}
    
private:
  ActionValueFunctionPtr function;     
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_VALUE_FUNCTION_CHOOSE_H_
