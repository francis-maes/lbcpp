/*-----------------------------------------.---------------------------------.
| Filename: PoseFeatures.h                 | PoseFeatures                    |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Feb 16, 2012  5:02:56 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSEFEATURES_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSEFEATURES_H_

# include <lbcpp/Core/CompositeFunction.h>
# include "../Pose.h"

namespace lbcpp
{

class PoseFeatures : public CompositeFunction
{
public:
  PoseFeatures() {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
//    // inputs and variables
//    size_t input = builder.addInput(rosettaProteinClass);
//    size_t numResidues = builder.addFunction(getVariableFunction(T("numResidues")), input);
//    size_t energy = builder.addFunction(getVariableFunction(T("normalizedScore")), input);
//    //size_t histogram = builder.addFunction(getVariableFunction(T("histogram")), input);
//    size_t shortDistance = builder.addFunction(getVariableFunction(T("shortDistance")), input);
//    size_t longDistance = builder.addFunction(getVariableFunction(T("longDistance")), input);
//
//    // select features
//    builder.startSelection();
//    size_t residueFeatures;
//    size_t energyFeatures;
//    size_t histogramFeatures;
//    size_t shortDistancesFeatures;
//    size_t longDistancesFeatures;
//    if (selectResiduesFeatures)
//      residueFeatures = builder.addFunction(softDiscretizedLogNumberFeatureGenerator(1, std::log10(100.0), 10, true), numResidues);
//    if (selectEnergyFeatures)
//      energyFeatures = builder.addFunction(defaultProbabilityFeatureGenerator(10), energy);
//    if (selectHistogramFeatures)
//      histogramFeatures = builder.addFunction(new RosettaProteinResidueHistogramFeatureGenerator(), input);
//    if (selectDistanceFeatures)
//    {
//      shortDistancesFeatures = builder.addFunction(defaultProbabilityFeatureGenerator(10), shortDistance);
//      longDistancesFeatures = builder.addFunction(defaultProbabilityFeatureGenerator(10), longDistance);
//    }
//
//    // end selection
//    builder.finishSelectionWithFunction(concatenateFeatureGenerator());
  }

protected:
  friend class PoseFeaturesClass;

};

typedef ReferenceCountedObjectPtr<PoseFeatures> PoseFeaturesPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSEFEATURES_H_
