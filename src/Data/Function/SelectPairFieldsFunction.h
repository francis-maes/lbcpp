/*-----------------------------------------.---------------------------------.
| Filename: SelectPairFieldsFunction.h     | Select Pair Fields Function     |
| Author  : Francis Maes                   |                                 |
| Started : 19/08/2010 13:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_FUNCTION_SELECT_PAIR_FIELDS_H_
# define LBCPP_DATA_FUNCTION_SELECT_PAIR_FIELDS_H_

# include <lbcpp/Data/Function.h>

namespace lbcpp
{

class SelectPairFieldsFunction : public Function
{
public:
  SelectPairFieldsFunction(int index1 = -1, int index2 = -1)
    : index1(index1), index2(index2)
    {}

  virtual TypePtr getInputType() const
    {return pairType(anyType(), anyType());}

  virtual TypePtr getOutputType(TypePtr inputType) const
  {
    return pairType(
      getOutputTypeBase(inputType->getTemplateArgument(0), index1),
      getOutputTypeBase(inputType->getTemplateArgument(1), index2));
  }

  virtual Variable computeFunction(const Variable& input, ErrorHandler& callback) const
  {
    Variable first = input[0];
    if (index1 >= 0)
      first = first[index1];
    Variable second = input[1];
    if (index2 >= 0)
      second = second[index2];
    return Variable::pair(first, second);
  }

private:
  friend class SelectPairFieldsFunctionClass;

  int index1, index2;

  static TypePtr getOutputTypeBase(TypePtr inputType, int index)
  {
    if (index >= 0)
    {
      if ((size_t)index < inputType->getNumStaticVariables())
        return inputType->getStaticVariableType((size_t)index);
      else
        return anyType();
    }
    else
      return inputType;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_FUNCTION_SELECT_PAIR_FIELDS_H_
