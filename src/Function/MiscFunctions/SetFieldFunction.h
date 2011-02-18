/*-----------------------------------------.---------------------------------.
| Filename: SetFieldFunction.h             | Set Field Function              |
| Author  : Francis Maes                   |                                 |
| Started : 19/08/2010 13:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_FUNCTION_SET_FIELD_H_
# define LBCPP_DATA_FUNCTION_SET_FIELD_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{

// Input: (a,b)
// do: a[fieldIndex] = b if fieldIndex, nothing otherwise
// Output: a
class SetFieldFunction : public Function
{
public:
  SetFieldFunction(size_t fieldIndex = 0)
    : fieldIndex(fieldIndex) {}

  virtual TypePtr getInputType() const
    {return pairClass(objectClass, anyType);}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return inputType->getTemplateArgument(0);}

protected:
  friend class SetFieldFunctionClass;

  size_t fieldIndex;

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>();
    ObjectPtr object = pair->getFirst().getObject();
    if (object && pair->getSecond().exists())
      object->setVariable(fieldIndex, pair->getSecond());
    else if (!object)
      context.warningCallback(T("SetFieldFunction::computeFunction"), T("Null object"));
    return pair->getFirst();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_FUNCTION_SET_FIELD_H_
