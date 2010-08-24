/*-----------------------------------------.---------------------------------.
| Filename: IdentityPerception.h           | Identity Perception             |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 16:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_IDENTITY_H_
# define LBCPP_DATA_PERCEPTION_IDENTITY_H_

# include <lbcpp/Data/Perception.h>

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
      size_t n = type->getObjectNumVariables();
      for (size_t i = 0; i < n; ++i)
      {
        Variable variable = inputObject->getVariable(i);
        if (variable)
          callback->sense(i, variable);
      }
    }
  }

protected:
  friend class IdentityPerceptionClass;

  TypePtr type;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_IDENTITY_H_
