/*-----------------------------------------.---------------------------------.
| Filename: BiVariablePerception.h         | Bi Variable Perception         |
| Author  : Julien Becker                  |                                 |
| Started : 23/09/2010 10:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_BI_VARIABLE_H_
# define LBCPP_DATA_PERCEPTION_BI_VARIABLE_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{

class BiVariablePerception : public Perception
{
public:
  BiVariablePerception(TypePtr firstElementType, TypePtr secondElementType)
  : firstElementType(firstElementType), secondElementType(secondElementType) {}
  
  BiVariablePerception() {}
  
  virtual TypePtr getInputType() const
    {return pairType(firstElementType, secondElementType);}

  virtual size_t getNumOutputVariables() const
  {
    if (firstElementType->inheritsFrom(probabilityType())
        && secondElementType->inheritsFrom(probabilityType()))
      return 1;
    return 2;
  }

  virtual TypePtr getOutputVariableType(size_t index) const
  {
    if (firstElementType->inheritsFrom(probabilityType())
        && secondElementType->inheritsFrom(probabilityType()))
      return doubleType();
    return index ? secondElementType : firstElementType;
  }

  virtual String getOutputVariableName(size_t index) const
  {
    TypePtr type = index ? secondElementType : firstElementType;
    
    EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
    if (enumeration)
      return enumeration->getName();
    if (type->inheritsFrom(type))
      return T("Probability");
    jassert(false);
    return T("???");
  }

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    if (firstElementType->inheritsFrom(probabilityType())
        && secondElementType->inheritsFrom(probabilityType()))
    {
      callback->sense(0, Variable(input[0].getDouble() * input[1].getDouble(), probabilityType()));
      return;
    }

    callback->sense(0, input[0]);
    callback->sense(1, input[1]);    
  }

protected:
  friend class BiVariablePerceptionClass;

  TypePtr firstElementType;
  TypePtr secondElementType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_BI_CONTAINER_H_
