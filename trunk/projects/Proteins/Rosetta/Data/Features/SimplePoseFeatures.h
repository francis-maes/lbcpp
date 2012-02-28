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

# include "../Pose.h"

namespace lbcpp
{

class SimplePoseFeatures : public GeneralFeatures
{
public:

  virtual Variable computeStaticFeatures(ExecutionContext& context, const Variable& input)
  {
    std::vector<DoubleVectorPtr> tempFeatures;

    PosePtr pose = input.getObjectAndCast<Pose> ();

    // histogram
    DoubleVectorPtr histogram = pose->getHistogram();
    tempFeatures.push_back(histogram);

    // num residues
    Variable res((double)pose->getLength());
    DoubleVectorPtr numResidues = numResiduesFeatureGenerator->computeFunction(context, &res).getObjectAndCast<DoubleVector> ();

    tempFeatures.push_back(numResidues);

    // gathering features
    staticFeatures = concatenateFeatures(tempFeatures);

    return staticFeatures;
  }

  virtual Variable computeDynamicFeatures(ExecutionContext& context, const Variable& input)
  {
    std::vector<DoubleVectorPtr> tempFeatures;

    PosePtr pose = input.getObjectAndCast<Pose> ();

    // distances
    size_t cutoff = 20;
    double shortDistance = 0;
    double longDistance = 0;
    pose->computeMeanDistances(cutoff, &shortDistance, &longDistance);

    double maxShortDist = 5.4 * cutoff / 3.6;
    double maxLongDist = 5.4 * pose->getLength() / 3.6;

    Variable inputShortDistance(shortDistance / maxShortDist);
    Variable inputLongDistance(longDistance / maxLongDist);
    DoubleVectorPtr sdFeatures = shortDistanceFeatureGenerator->computeFunction(context, &inputShortDistance).getObjectAndCast<
        DoubleVector> ();
    tempFeatures.push_back(sdFeatures);
    DoubleVectorPtr ldFeatures = longDistanceFeatureGenerator->computeFunction(context, &inputLongDistance).getObjectAndCast<
        DoubleVector> ();
    tempFeatures.push_back(ldFeatures);

    // energy
    double energy = pose->getEnergy();
    double k = 0.001;
    energy = 1.0 / (1.0 + std::exp(-k * energy));
    Variable inputEnergy(energy);
    DoubleVectorPtr enFeatures = energyFeatureGenerator->computeFunction(context, &inputEnergy).getObjectAndCast<DoubleVector> ();
    tempFeatures.push_back(enFeatures);

    // gathering features
    dynamicFeatures = concatenateFeatures(tempFeatures);

    return dynamicFeatures;
  }

protected:
  friend class SimplePoseFeaturesClass;

  virtual void selfInitialization(ExecutionContext& context, const Variable& input)
  {
    // num residues feature generator
    numResiduesFeatureGenerator = softDiscretizedNumberFeatureGenerator(0.0, 1000, 10, true);
    numResiduesFeatureGenerator->initialize(context, doubleType);

    // short distance feature generator
    shortDistanceFeatureGenerator = softDiscretizedNumberFeatureGenerator(0.0, 1, 10, true);
    shortDistanceFeatureGenerator->initialize(context, doubleType);

    // long distance feature generator
    longDistanceFeatureGenerator = softDiscretizedNumberFeatureGenerator(0.0, 1, 10, true);
    longDistanceFeatureGenerator->initialize(context, doubleType);

    // energy feature generator
    energyFeatureGenerator = softDiscretizedNumberFeatureGenerator(0.0, 1, 10, true);
    energyFeatureGenerator->initialize(context, doubleType);

  }

  FeatureGeneratorPtr numResiduesFeatureGenerator;
  FeatureGeneratorPtr shortDistanceFeatureGenerator;
  FeatureGeneratorPtr longDistanceFeatureGenerator;
  FeatureGeneratorPtr energyFeatureGenerator;
};

typedef ReferenceCountedObjectPtr<SimplePoseFeatures> SimplePoseFeaturesPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_SIMPLEPOSEFEATURES_H_

