/*-----------------------------------------.---------------------------------.
| Filename: SimplePoseFeatures.h           | SimplePoseFeatures              |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Feb 24, 2012  3:23:37 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_SIMPLEPOSEFEATURES_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_SIMPLEPOSEFEATURES_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

# include "GeneralFeatures.h"

namespace lbcpp
{

class SimplePoseFeatures : public GeneralFeatures
{
public:

  virtual Variable computeStaticFeatures(ExecutionContext& context, const Variable& input);
  virtual Variable computeDynamicFeatures(ExecutionContext& context, const Variable& input);

protected:
  friend class SimplePoseFeaturesClass;

  virtual void selfInitialization(ExecutionContext& context, const Variable& input);

  FeatureGeneratorPtr numResiduesFeatureGenerator;
  FeatureGeneratorPtr shortDistanceFeatureGenerator;
  FeatureGeneratorPtr longDistanceFeatureGenerator;
  FeatureGeneratorPtr energyFeatureGenerator;
};

typedef ReferenceCountedObjectPtr<SimplePoseFeatures> SimplePoseFeaturesPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_SIMPLEPOSEFEATURES_H_

