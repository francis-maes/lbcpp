/*-----------------------------------------.---------------------------------.
| Filename: PoseDynamicFeatures.h          | Pose Dynamic Feature Generator  |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 10, 2012  5:42:57 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSEDYNAMICFEATUREGENERATOR_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSEDYNAMICFEATUREGENERATOR_H_

# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

# include "../Pose.h"

namespace lbcpp
{

/*
 * Short And Long Distances
 */
class PoseShortAndLongDistancesDynamicFeatureGenerator : public FeatureGenerator
{
public:
  PoseShortAndLongDistancesDynamicFeatureGenerator() : shortDistanceNumFeatures(0), longDistanceNumFeatures(0)
  {
    shortDistanceFeatureGenerator = softDiscretizedNumberFeatureGenerator(0.0, 1, 10, true);
    shortDistanceFeatureGenerator->initialize(defaultExecutionContext(), doubleType);

    longDistanceFeatureGenerator = softDiscretizedNumberFeatureGenerator(0.0, 1, 10, true);
    longDistanceFeatureGenerator->initialize(defaultExecutionContext(), doubleType);
  }

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return poseClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    EnumerationPtr shortDistancesFeatEnumeration = shortDistanceFeatureGenerator->getFeaturesEnumeration();
    shortDistanceNumFeatures = shortDistancesFeatEnumeration->getNumElements();
    EnumerationPtr longDistancesFeatEnumeration = longDistanceFeatureGenerator->getFeaturesEnumeration();
    longDistanceNumFeatures = longDistancesFeatEnumeration->getNumElements();
    DefaultEnumerationPtr resEnum = new DefaultEnumeration("Short and Long Distances features enumeration");

    for (size_t i = 0; i < shortDistanceNumFeatures; ++i)
      resEnum->addElement(context, "SLDDynF:SD:" + shortDistancesFeatEnumeration->getElementName(i));
    for (size_t i = 0; i < longDistanceNumFeatures; ++i)
      resEnum->addElement(context, "SLDynF:LD:" + longDistancesFeatEnumeration->getElementName(i));

    return resEnum;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    PosePtr pose = inputs[0].getObjectAndCast<Pose> ();

    // distances
    size_t cutoff = 20;
    double shortDistance = 0;
    double longDistance = 0;
    pose->computeMeanDistances(cutoff, &shortDistance, &longDistance);

    double maxShortDist = 5.4 * cutoff / 3.6;
    double maxLongDist = 5.4 * pose->getLength() / 3.6;

    Variable inputShortDistance(shortDistance / maxShortDist);
    Variable inputLongDistance(longDistance / maxLongDist);

    callback.sense(0, shortDistanceFeatureGenerator, &inputShortDistance, 1.0);
    callback.sense(shortDistanceNumFeatures, longDistanceFeatureGenerator, &inputLongDistance, 1.0);
  }

protected:
  friend class PoseShortAndLongDistancesDynamicFeatureGeneratorClass;

  FeatureGeneratorPtr shortDistanceFeatureGenerator;
  FeatureGeneratorPtr longDistanceFeatureGenerator;
  size_t shortDistanceNumFeatures;
  size_t longDistanceNumFeatures;
};

/*
 * Energy
 */
class PoseEnergyDynamicFeatureGenerator : public FeatureGenerator
{
public:
  PoseEnergyDynamicFeatureGenerator()
  {
    energyFeatureGenerator = softDiscretizedNumberFeatureGenerator(0.0, 1, 10, true);
    energyFeatureGenerator->initialize(defaultExecutionContext(), doubleType);
  }

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return poseClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    EnumerationPtr energyFeatEnumeration = energyFeatureGenerator->getFeaturesEnumeration();
    DefaultEnumerationPtr resEnum = new DefaultEnumeration("Energy features enumeration");

    for (size_t i = 0; i < energyFeatEnumeration->getNumElements(); ++i)
      resEnum->addElement(context, "EDynF:" + energyFeatEnumeration->getElementName(i));

    return resEnum;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    double energy = (inputs[0].getObjectAndCast<Pose> ())->getEnergy();
    double k = 0.001;
    double normalized = 1.0 / (1.0 + std::exp(-k * energy));

    Variable inputEnergy(normalized);

    callback.sense(0, energyFeatureGenerator, &inputEnergy, 1.0);
  }

protected:
  friend class PoseEnergyDynamicFeatureGeneratorClass;

  FeatureGeneratorPtr energyFeatureGenerator;
};

/*
 * Dynamic features
 */
class PoseDynamicFeatureGenerator : public CompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    // inputs and variables
    size_t input = builder.addInput(poseClass);

    // select features
    builder.startSelection();

    // features
    builder.addFunction(new PoseShortAndLongDistancesDynamicFeatureGenerator(), input);
    builder.addFunction(new PoseEnergyDynamicFeatureGenerator(), input);

    // end selection
    builder.finishSelectionWithFunction(concatenateFeatureGenerator());
  }

protected:
  friend class PoseDynamicFeatureGeneratorClass;
};

typedef ReferenceCountedObjectPtr<PoseDynamicFeatureGenerator> PoseDynamicFeatureGeneratorPtr;


}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSEDYNAMICFEATUREGENERATOR_H_
