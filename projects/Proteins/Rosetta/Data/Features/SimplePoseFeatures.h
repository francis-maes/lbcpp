/*-----------------------------------------.---------------------------------.
| Filename: SimplePoseFeatures.h           | SimplePoseFeatures              |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Feb 24, 2012  3:23:37 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_SIMPLEPOSEFEATURES_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_SIMPLEPOSEFEATURES_H_

# include "PoseFeatures.h"

namespace lbcpp
{

class SimplePoseFeatures : public PoseFeatures
{
public:

  virtual Variable computeStaticFeatures(ExecutionContext& context, Variable& input)
  {
    std::vector<DenseDoubleVectorPtr> tempFeatures;

    PosePtr pose = input.getObjectAndCast<Pose> ();

    // histogram
    DenseDoubleVectorPtr histogram = pose->getHistogram();
    tempFeatures.push_back(histogram);

    // num residues
    DenseDoubleVectorPtr numResidues = numResiduesFeatureGenerator->computeFunction(context, &input).getObjectAndCast<
        DenseDoubleVector> ();
    tempFeatures.push_back(numResidues);

    // gathering features
    staticFeatures = concatenateFeatures(tempFeatures);

    return staticFeatures;
  }

  virtual Variable computeDynamicFeatures(ExecutionContext& context, Variable& input)
  {

    return Variable();
  }

protected:
  friend class SimplePoseFeaturesClass;

  virtual void selfInitialization(ExecutionContext& context, Variable& input)
  {
    // num residues feature generator
    numResiduesFeatureGenerator = softDiscretizedNumberFeatureGenerator(0.0, 1000, 100, true);
    numResiduesFeatureGenerator->initialize(context, doubleType);

  }

  FeatureGeneratorPtr numResiduesFeatureGenerator;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_SIMPLEPOSEFEATURES_H_


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

