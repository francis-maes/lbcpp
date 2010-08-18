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
    {return type->getNumStaticVariables();}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return type->getStaticVariableType(index);}

  virtual String getOutputVariableName(size_t index) const
    {return type->getStaticVariableName(index);}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(!input.isNil());
    if (input)
    {
      size_t n = getNumOutputVariables();
      for (size_t i = 0; i < n; ++i)
        callback->sense(i, input[i]);
    }
  }

protected:
  friend class IdentityPerceptionClass;

  TypePtr type;
};

class IdentityPerceptionClass : public DynamicClass
{
public:
  IdentityPerceptionClass() : DynamicClass(T("IdentityPerception"), perceptionClass())
  {
    addVariable(typeClass(), T("type"));
  }

  virtual VariableValue create() const
    {return new IdentityPerception();}

  LBCPP_DECLARE_VARIABLE_BEGIN(IdentityPerception)
    LBCPP_DECLARE_VARIABLE(type);
  LBCPP_DECLARE_VARIABLE_END()
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_IDENTITY_H_
