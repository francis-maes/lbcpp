/*-----------------------------------------.---------------------------------.
| Filename: EnumValueFeatures.h            | A Feature Generator that        |
| Author  : Francis Maes                   | generates one feature per enum  |
| Started : 15/09/2010 19:41               | value.                          |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_PERCEPTION_ENUM_VALUE_H_
# define LBCPP_NUMERICAL_LEARNING_PERCEPTION_ENUM_VALUE_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class EnumValueFeatures : public Perception
{
public:
  EnumValueFeatures(EnumerationPtr enumeration)
    : enumeration(enumeration) {computeOutputType();}
  EnumValueFeatures() {}

  virtual String toString() const
    {return enumeration->getName() + T(" as features");}

  virtual bool isSparse() const
    {return true;}

  virtual TypePtr getInputType() const
    {return enumeration;}
  
  virtual void computeOutputType()
  {
    size_t n = enumeration->getNumElements();
    reserveOutputVariables(n);
    for (size_t i = 0; i < n; ++i)
      addOutputVariable(enumeration->getElementName(i), doubleType);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input.getType()->inheritsFrom(enumeration));
    if (input.exists())
      callback->sense((size_t)input.getInteger(), 1.0);
  }
  
private:
  friend class EnumValueFeaturesClass;

  EnumerationPtr enumeration;
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_PERCEPTION_ENUM_VALUE_H_