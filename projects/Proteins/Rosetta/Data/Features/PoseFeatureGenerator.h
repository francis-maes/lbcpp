/*-----------------------------------------.---------------------------------.
| Filename: PoseFeatureGenerator.h         | Pose Feature Generator          |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 10, 2012  5:45:46 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSEFEATUREGENERATOR_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSEFEATUREGENERATOR_H_

# include "PoseStaticFeatureGenerator.h"
# include "PoseDynamicFeatureGenerator.h"

namespace lbcpp
{

class PoseFeatureGenerator : public CompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    // inputs and variables
    size_t input = builder.addInput(poseClass);

    // select features
    builder.startSelection();

    // features
    builder.addFunction(new PoseStaticFeatureGenerator(), input);
    builder.addFunction(new PoseDynamicFeatureGenerator(), input);

    // end selection
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

protected:
  friend class PoseFeatureGeneratorClass;
};

typedef ReferenceCountedObjectPtr<PoseFeatureGenerator> PoseFeatureGeneratorPtr;

extern CompositeFunctionPtr poseFeatureGenerator();

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSEFEATUREGENERATOR_H_
