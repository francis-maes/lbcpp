/*-----------------------------------------.---------------------------------.
| Filename: PerceptionToFeatures.h         | A decorator to make a Feature   |
| Author  : Francis Maes                   | Generator Perception            |
| Started : 20/08/2010 20:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_TO_FEATURES_H_
# define LBCPP_DATA_PERCEPTION_TO_FEATURES_H_

# include <lbcpp/Data/Perception.h>

namespace lbcpp
{

class EnumValueToFeaturesPerception : public Perception
{
public:
  EnumValueToFeaturesPerception(EnumerationPtr enumeration)
    : enumeration(enumeration) {}

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
  EnumerationPtr enumeration;
};

class IntegerToFeaturesPerception : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return integerType();}

  virtual size_t getNumOutputVariables() const
    {return 0;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return doubleType();}

  virtual String getOutputVariableName(size_t index) const
    {return String::empty;}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    //jassert(input.getType()->inheritsFrom(enumeration));
    //callback->sense((size_t)input.getInteger(), 1.0);
  }
};

class PerceptionToFeatures : public ModifierPerception
{
public:
  PerceptionToFeatures(PerceptionPtr decorated)
    : ModifierPerception(decorated) {}
  PerceptionToFeatures() {}

  virtual PerceptionPtr getModifiedPerception(size_t index, TypePtr valueType) const
  {
    if (valueType->inheritsFrom(doubleType()))
      return PerceptionPtr();
    if (valueType->inheritsFrom(enumValueType()))
      return PerceptionPtr(new EnumValueToFeaturesPerception(valueType));
    if (valueType->inheritsFrom(integerType()))
      return PerceptionPtr(new IntegerToFeaturesPerception());

    // to be continued
    return PerceptionPtr();
  }

  virtual ObjectPtr clone() const
    {return new PerceptionToFeatures(decorated);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_TO_FEATURES_H_
