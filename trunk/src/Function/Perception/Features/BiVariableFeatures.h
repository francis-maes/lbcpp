/*-----------------------------------------.---------------------------------.
| Filename: BiVariableFeatures.h           | A Feature Generator that        |
| Author  : Julien Becker                  | merge BiVariablePerception into |
| Started : 23/09/2010 18:56               | one feature                     |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_FEATURES_BI_VARIABLE_H_
# define LBCPP_FUNCTION_PERCEPTION_FEATURES_BI_VARIABLE_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{
  
class BiVariableFeatures : public DecoratorPerception
{
public:
  BiVariableFeatures(TypePtr firstElementType, TypePtr secondElementType, PerceptionPtr subPerception)
  : DecoratorPerception(subPerception), firstElementType(firstElementType), secondElementType(secondElementType), hasBeenSwaped(false)
  {
    jassert(firstElementType->inheritsFrom(probabilityType()) || firstElementType.dynamicCast<Enumeration>());
    jassert(secondElementType->inheritsFrom(probabilityType()) || secondElementType.dynamicCast<Enumeration>());

    if (firstElementType->inheritsFrom(probabilityType())
        && secondElementType.dynamicCast<Enumeration>())
    {
      this->firstElementType = secondElementType;
      this->secondElementType = firstElementType;
      this->hasBeenSwaped = true;
    }
  }

  BiVariableFeatures() {}

  virtual TypePtr getInputType() const
    {return Type::get("BiVariablePerception");}

  virtual size_t getNumOutputVariables() const
  {
    if (firstElementType->inheritsFrom(probabilityType()))
      return 1;

    EnumerationPtr firstEnumeration = firstElementType.dynamicCast<Enumeration>();
    size_t res = firstEnumeration->getNumElements() + 1;
    
    EnumerationPtr secondEnumeration = secondElementType.dynamicCast<Enumeration>();
    if (secondEnumeration)
      res *= secondEnumeration->getNumElements() + 1;

    return res;
  }

  virtual TypePtr getOutputVariableType(size_t index) const
    {return doubleType();}

  virtual String getOutputVariableName(size_t index) const
  {
    if (firstElementType->inheritsFrom(probabilityType()))
      return T("Probability");
    
    EnumerationPtr firstEnumeration = firstElementType.dynamicCast<Enumeration>();
    if (index < firstEnumeration->getNumElements() + 1)
      return Variable(index, firstEnumeration).toString();
    EnumerationPtr secondEnumeration = secondElementType.dynamicCast<Enumeration>();
    return Variable(index - firstEnumeration->getNumElements(), secondEnumeration).toString();
  }

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    if (firstElementType->inheritsFrom(probabilityType()))
    {
      callback->sense(0, decorated, input[0]);
      return;
    }
    
    Variable first = hasBeenSwaped ? input[1] : input[0];
    Variable second = hasBeenSwaped ? input[0] : input[1];
    
    size_t index = first.getInteger();
    if (secondElementType->inheritsFrom(probabilityType()))
    {
      callback->sense(index, decorated, second);
      return;
    }
    callback->sense(index + second.getInteger(), 1.0);
  }
  
private:
  friend class BiVariableFeaturesClass;
  
  TypePtr firstElementType;
  TypePtr secondElementType;
  bool hasBeenSwaped;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_FEATURES_BI_VARIABLE_H_
