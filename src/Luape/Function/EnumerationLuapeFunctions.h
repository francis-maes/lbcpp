/*-----------------------------------------.---------------------------------.
| Filename: EnumerationLuapeFunctions.h    | Enumeration Luape Functions     |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FUNCTION_ENUMERATION_H_
# define LBCPP_LUAPE_FUNCTION_ENUMERATION_H_

# include <lbcpp/Luape/LuapeFunction.h>
# include <lbcpp/Luape/LuapeNode.h>

namespace lbcpp
{

class EqualsConstantEnumLuapeFunction : public LuapeFunction
{
public:
  EqualsConstantEnumLuapeFunction(const Variable& value = Variable())
    : value(value) {}

  virtual String toShortString() const
    {return "= " + value.toShortString();}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type.isInstanceOf<Enumeration>();}

  virtual TypePtr initialize(const std::vector<TypePtr>& )
    {return booleanType;}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + T(" == ") + value.toShortString();}
  
  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    if (inputs[0].isMissingValue())
      return Variable::missingValue(booleanType);
    return inputs[0] == value;
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    const EnumerationPtr& enumeration = inputTypes[0].staticCast<Enumeration>();
    size_t n = enumeration->getNumElements();
    VectorPtr res = vector(enumeration, n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, Variable(i, enumeration));
    return res;
  }

protected:
  friend class EqualsConstantEnumLuapeFunctionClass;

  Variable value; 
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_ENUMERATION_H_
