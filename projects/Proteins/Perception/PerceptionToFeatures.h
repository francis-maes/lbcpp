/*-----------------------------------------.---------------------------------.
| Filename: PerceptionToFeatures.h         | Transform a Perception into a   |
| Author  : Francis Maes                   |   Feature Generator             |
| Started : 14/07/2010 19:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_TO_FEATURES_H_
# define LBCPP_PROTEIN_PERCEPTION_TO_FEATURES_H_

# include <lbcpp/Data/Perception.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

extern FeatureGeneratorPtr variableFeatures(Variable variable);
extern FeatureGeneratorPtr topLevelVariableFeatures(Variable variable);

inline FeatureGeneratorPtr perceptionToFeatures(PerceptionPtr perception, const Variable& input)
  {return variableFeatures(perception->compute(input));}

class ConvertToFeaturesPerception : public Perception
{
public:
  ConvertToFeaturesPerception(PerceptionPtr perception = PerceptionPtr())
    : perception(perception) {}
 
  virtual TypePtr getInputType() const
    {return perception->getInputType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return Class::get(T("FeatureGenerator"));}

  virtual size_t getNumOutputVariables() const
    {return 1;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return Class::get(T("FeatureGenerator"));}

  virtual String getOutputVariableName(size_t index) const
    {return perception->getName();}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
    {jassert(false);}

  virtual Variable computeFunction(const Variable& input, ErrorHandler& handler) const
    {return topLevelVariableFeatures(perception->computeFunction(input, handler));}

protected:
  friend class ConvertToFeaturesPerceptionClass;

  PerceptionPtr perception;
};

class ConvertToFeaturesPerceptionClass : public DynamicClass
{
public:
  ConvertToFeaturesPerceptionClass() 
    : DynamicClass(T("ConvertToFeaturesPerception"), perceptionClass())
  {
    addVariable(perceptionClass(), T("perception"));
  }

  virtual VariableValue create() const
    {return new ConvertToFeaturesPerception();}

  LBCPP_DECLARE_VARIABLE_BEGIN(ConvertToFeaturesPerception)
    LBCPP_DECLARE_VARIABLE(perception);
  LBCPP_DECLARE_VARIABLE_END()
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_TO_FEATURES_H_
