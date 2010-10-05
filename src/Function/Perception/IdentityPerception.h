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
# include <lbcpp/Function/Perception.h>

namespace lbcpp
{

class IdentityPerception : public Perception
{
public:
  IdentityPerception(TypePtr type)
    : type(type) {}
  IdentityPerception() {}

  virtual TypePtr getInputType() const
    {return type;}

  virtual String getPreferedOutputClassName() const
    {return classNameToOutputClassName(type->getName());}

  virtual TypePtr getOutputType() const
    {return type;}

  virtual size_t getNumOutputVariables() const
    {return type->getObjectNumVariables();}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return type->getObjectVariableType(index);}

  virtual String getOutputVariableName(size_t index) const
    {return type->getObjectVariableName(index);}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input.isObject());
    ObjectPtr inputObject = input.getObject();
    if (inputObject)
    {
      StreamPtr variablesStream = inputObject->getVariablesStream();
      if (variablesStream)
      {
        while (!variablesStream->isExhausted())
        {
          Variable indexAndValue = variablesStream->next();
          callback->sense((size_t)indexAndValue[0].getInteger(), indexAndValue[1]);
        }
      }
      else
      {
        size_t n = type->getObjectNumVariables();
        for (size_t i = 0; i < n; ++i)
        {
          Variable variable = inputObject->getVariable(i);
          if (!variable)
            continue;
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
