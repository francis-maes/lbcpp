/*-----------------------------------------.---------------------------------.
| Filename: PerceptionToFeatures.h         | A decorator to make a Feature   |
| Author  : Francis Maes                   | Generator Perception            |
| Started : 20/08/2010 20:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_TO_FEATURES_H_
# define LBCPP_DATA_PERCEPTION_TO_FEATURES_H_

# include <lbcpp/Function/Perception.h>

namespace lbcpp
{
/*
class PerceptionToFeatures : public ModifierPerception
{
public:
  PerceptionToFeatures(PerceptionPtr decorated)
    : ModifierPerception(decorated) {}
  PerceptionToFeatures() {}

  virtual String getPreferedOutputClassName() const
    {return decorated->getPreferedOutputClassName() + T(" as features");}

  virtual PerceptionPtr getModifiedPerception(size_t index, TypePtr valueType) const
  {
    PerceptionPtr decoratedSubPerception = decorated->getOutputVariableGenerator(index);
    if (decoratedSubPerception)
      return PerceptionPtr(new PerceptionToFeatures(decoratedSubPerception));

    if (valueType->inheritsFrom(doubleType()))
      return PerceptionPtr(); // identity
    if (valueType->inheritsFrom(enumValueType()))
      return enumValueFeatures(valueType);
    if (valueType->inheritsFrom(integerType()))
      return nullPerception();

    // to be continued
    return PerceptionPtr(); // identity
  }

  virtual ObjectPtr clone() const
    {return new PerceptionToFeatures(decorated);}
};
*/
}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_TO_FEATURES_H_
