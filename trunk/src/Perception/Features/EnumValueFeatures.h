/*-----------------------------------------.---------------------------------.
| Filename: EnumValueFeatures.h            | A Feature Generator that        |
| Author  : Francis Maes                   | generates one feature per enum  |
| Started : 15/09/2010 19:41               | value.                          |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FEATURES_ENUM_VALUE_H_
# define LBCPP_FUNCTION_PERCEPTION_FEATURES_ENUM_VALUE_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class EnumValueFeatures : public Perception
{
public:
  EnumValueFeatures(EnumerationPtr enumeration)
    : enumeration(enumeration) {}
  EnumValueFeatures() {}

  virtual String getPreferedOutputClassName() const
    {return enumeration->getName() + T(" as features");}

  virtual bool isSparse() const
    {return true;}

  virtual TypePtr getInputType() const
    {return enumeration;}

  virtual size_t getNumOutputVariables() const
    {return enumeration->getNumElements() + 1;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return doubleType();}

  virtual String getOutputVariableName(size_t index) const
    {return Variable(index, enumeration).toString();}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input.getType()->inheritsFrom(enumeration));
    callback->sense((size_t)input.getInteger(), 1.0);
  }
  
private:
  friend class EnumValueFeaturesClass;

  EnumerationPtr enumeration;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FEATURES_ENUM_VALUE_H_
