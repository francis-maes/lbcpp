/*-----------------------------------------.---------------------------------.
| Filename: BooleanFeatures.h              | A Feature Generator that        |
| Author  : Francis Maes                   | converts a boolean to a feature |
| Started : 04/10/2010 10:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_PERCEPTION_BOOLEAN_H_
# define LBCPP_NUMERICAL_LEARNING_PERCEPTION_BOOLEAN_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class AddUnitFeature : public Perception
{
public:
  AddUnitFeature(TypePtr inputType) : inputType(inputType)
    {computeOutputType();}

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
  friend class AddUnitFeatureClass;

  TypePtr inputType;
};

class BooleanFeatures : public Perception
{
public:
  BooleanFeatures()
    {computeOutputType();}

  virtual String toString() const
    {return T("boolean as feature");}

  virtual TypePtr getInputType() const
    {return booleanType;}

  virtual void computeOutputType()
  {
    addOutputVariable(T("value"), doubleType);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input.isBoolean());
    if (input.exists() && input.getBoolean())
      callback->sense(0, 1.0);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_PERCEPTION_BOOLEAN_H_
