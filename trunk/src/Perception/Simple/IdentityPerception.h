/*-----------------------------------------.---------------------------------.
| Filename: IdentityPerception.h           | Identity Perception             |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 16:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_IDENTITY_H_
# define LBCPP_FUNCTION_PERCEPTION_IDENTITY_H_

# include <lbcpp/Data/Stream.h>
# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class IdentityPerception : public Perception
{
public:
  IdentityPerception(TypePtr type)
    : type(type) {computeOutputVariables();}
  IdentityPerception() {}

  virtual TypePtr getInputType() const
    {return type;}

  virtual String toString() const
    {return classNameToOutputClassName(type->getName());}

  virtual TypePtr getOutputType() const
    {return type;}

  virtual void computeOutputVariables()
  {
    size_t n = type->getObjectNumVariables();
    reserveOutputVariables(n);
    for (size_t i = 0; i < n; ++i)
      addOutputVariable(type->getObjectVariableName(i), type->getObjectVariableType(i));
  }

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input.isObject());
    ObjectPtr inputObject = input.getObject();
    if (inputObject)
    {
      VariableIterator* iterator = inputObject->createVariablesIterator();
      if (iterator)
      {
        for ( ; iterator->exists(); iterator->next())
        {
          size_t index;
          Variable value = iterator->getCurrentVariable(index);
          callback->sense(index, value);
        }
        delete iterator;
      }
      else
      {
        size_t n = type->getObjectNumVariables();
        for (size_t i = 0; i < n; ++i)
        {
          Variable variable = inputObject->getVariable(i);
          if (variable)
            callback->sense(i, variable);
        }
      }
    }
  }

  juce_UseDebuggingNewOperator

protected:
  friend class IdentityPerceptionClass;

  TypePtr type;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_IDENTITY_H_
