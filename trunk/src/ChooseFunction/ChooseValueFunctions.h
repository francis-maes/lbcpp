/*-----------------------------------------.---------------------------------.
| Filename: ChooseValueFunction.h          | Value functions given to the    |
| Author  : Francis Maes                   |   choose call.                  |
| Started : 22/06/2009 19:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_VALUE_FUNCTION_CHOOSE_H_
# define LBCPP_CORE_VALUE_FUNCTION_CHOOSE_H_

# include <lbcpp/ChooseFunction.h>

namespace lbcpp
{

class ChooseActionValueFunction : public ActionValueFunction
{
public:
  virtual std::string toString() const
    {return "chooseActionValue()";}
    
  virtual void setChoose(ChoosePtr choose)
    {function = choose->getActionValueFunction();}

  virtual double compute(VariablePtr choice) const
    {return function->compute(choice);}
    
private:
  ActionValueFunctionPtr function;     
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_VALUE_FUNCTION_CHOOSE_H_
