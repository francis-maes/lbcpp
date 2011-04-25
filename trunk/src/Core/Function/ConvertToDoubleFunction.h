/*-----------------------------------------.---------------------------------.
| Filename: ConvertToDoubleFunction.h      | Convert To Double Function      |
| Author  : Francis Maes                   |                                 |
| Started : 25/04/2011 20:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_CONVERT_TO_DOUBLE_FUNCTION_H_
# define LBCPP_CORE_FUNCTION_CONVERT_TO_DOUBLE_FUNCTION_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class ConvertToDoubleFunction : public Function
{
public:
  ConvertToDoubleFunction(bool applyLogScale = false)
    : applyLogScale(applyLogScale) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return variableType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    if (!input.exists())
      return Variable::missingValue(getOutputType());
    if (!input.isConvertibleToDouble())
    {
      context.errorCallback(T("Not convertible to double"));
      return Variable::missingValue(getOutputType());
    }

    double res = input.toDouble();
    return applyLogScale ? log10(res) : res;
  }

protected:
  friend class ConvertToDoubleFunctionClass;

  bool applyLogScale;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_CREATE_OBJECT_H_
