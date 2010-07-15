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

namespace lbcpp
{

extern FeatureGeneratorPtr variableFeatures(Variable variable);
extern FeatureGeneratorPtr topLevelVariableFeatures(Variable variable);

inline FeatureGeneratorPtr perceptionToFeatures(PerceptionPtr perception, const Variable& input)
  {return variableFeatures(perception->compute(input));}

class ConvertToFeaturesPerception : public CompositePerception
{
public:
  ConvertToFeaturesPerception(PerceptionPtr perception)
    : perception(perception) {}
 
  virtual TypePtr getInputType() const
    {return perception->getInputType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return Class::get(T("FeatureGenerator"));}

  virtual Variable computeFunction(const Variable& input, ErrorHandler& handler) const
    {return topLevelVariableFeatures(perception->computeFunction(input, handler));}

protected:
  PerceptionPtr perception;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_TO_FEATURES_H_
