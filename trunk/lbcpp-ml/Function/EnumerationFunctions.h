/*-----------------------------------------.---------------------------------.
| Filename: EnumerationFunctions.h         | Enumeration Functions           |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_FUNCTION_ENUMERATION_H_
# define LBCPP_ML_FUNCTION_ENUMERATION_H_

# include <lbcpp-ml/Function.h>
# include <lbcpp-ml/Expression.h>

namespace lbcpp
{

class EqualsConstantEnumFunction : public Function
{
public:
  EqualsConstantEnumFunction(EnumerationPtr enumeration = EnumerationPtr(), size_t value = 0)
    : enumeration(enumeration), value(value) {}

  virtual string toShortString() const
    {return "= " + ObjectPtr(new Integer(enumeration, value))->toShortString();}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const ClassPtr& type) const
    {return enumeration ? type == enumeration : type.isInstanceOf<Enumeration>();}

  virtual ClassPtr initialize(const ClassPtr* inputTypes)
    {return booleanClass;}

  virtual string makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return inputs[0]->toShortString() + T(" == ") + ObjectPtr(new Integer(enumeration, value))->toShortString();}
  
  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0])
      return ObjectPtr();
    return new Boolean(Integer::get(inputs[0]) == (juce::int64)value);
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<ClassPtr>& inputTypes) const
  {
    if (index == 0)
    {
      OVectorPtr res = new OVector(enumerationClass, 1);
      res->set(0, inputTypes[0]);
      return res;
    }
    else
    {
      const EnumerationPtr& enumeration = inputTypes[0].staticCast<Enumeration>();
      size_t n = enumeration->getNumElements();
      VectorPtr res = vector(enumeration, n);
      for (size_t i = 0; i < n; ++i)
        res->setElement(i, new Integer(enumeration, i));
      return res;
    }
  }

protected:
  friend class EqualsConstantEnumFunctionClass;

  EnumerationPtr enumeration;
  size_t value;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_FUNCTION_ENUMERATION_H_
