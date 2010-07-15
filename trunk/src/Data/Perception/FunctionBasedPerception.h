/*-----------------------------------------.---------------------------------.
| Filename: FunctionBasedPerception.h      | Transforms a Function into      |
| Author  : Francis Maes                   |  a single-output Perception     |
| Started : 14/07/2010 19:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_FUNCTION_BASED_H_
# define LBCPP_DATA_PERCEPTION_FUNCTION_BASED_H_

namespace lbcpp
{

class FunctionBasedPerception : public Perception
{
public:
  FunctionBasedPerception(FunctionPtr function = FunctionPtr())
    : function(function) {}

  virtual TypePtr getInputType() const
    {return function->getInputType();}

  virtual size_t getNumOutputVariables() const
    {return 1;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return function->getOutputType(getInputType());}

  virtual String getOutputVariableName(size_t index) const
    {return T("proteinLength");}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
    {callback->sense(0, function->compute(input));}

protected:
  FunctionPtr function;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_FUNCTION_BASED_H_
