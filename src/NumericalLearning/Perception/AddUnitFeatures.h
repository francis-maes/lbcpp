/*-----------------------------------------.---------------------------------.
| Filename: AddUnitFeatures.h              | A Feature Generator that        |
| Author  : Francis Maes                   | adds to "unit" feature to its   |
| Started : 15/01/2011 18:44               | input.                          |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_PERCEPTION_ADD_UNIT_FEATURES_H_
# define LBCPP_NUMERICAL_LEARNING_PERCEPTION_ADD_UNIT_FEATURES_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class AddUnitFeatures : public Perception
{
public:
  AddUnitFeatures(TypePtr inputType) : inputType(inputType)
    {computeOutputType();}
  AddUnitFeatures() {}

  virtual String toString() const
    {return inputType->getName() + T(" + unit");}

  virtual TypePtr getInputType() const
    {return inputType;}

  virtual void computeOutputType()
  {
    addOutputVariable(T("unit"), doubleType);
    addOutputVariable(T("features"), inputType);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    callback->sense(0, 1.0);
    callback->sense(1, input);
  }

protected:
  friend class AddUnitFeaturesClass;

  TypePtr inputType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_PERCEPTION_ADD_UNIT_FEATURES_H_
