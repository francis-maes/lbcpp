/*-----------------------------------------.---------------------------------.
| Filename: PoseStaticFeatures.h           | Pose Static Features            |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 10, 2012  2:23:23 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSESTATICFEATUREGENERATOR_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSESTATICFEATUREGENERATOR_H_

# include <lbcpp/Core/CompositeFunction.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

# include "../Pose.h"

namespace lbcpp
{

/*
 * Residue Histogram features
 */
class PoseResidueHistogramStaticFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return poseClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr resEnum = new DefaultEnumeration("Residue histogram features enumeration");
    for (size_t i = 0; i < aminoAcidTypeEnumeration->getNumElements(); ++i)
      resEnum->addElement(context, "RHStatF:" + aminoAcidTypeEnumeration->getElementName(i));
    return resEnum;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    DenseDoubleVectorPtr histogram = (inputs[0].getObjectAndCast<Pose> ())->getAminoAcidHistogram();
    for (size_t i = 0; i < histogram->getNumValues(); ++i)
      callback.sense(i, histogram->getValue(i));
  }

protected:
  friend class PoseResidueHistogramStaticFeatureGeneratorClass;
};

/*
 * Number residues features
 */
class PoseNumberResiduesStaticFeatureGenerator : public FeatureGenerator
{
public:
  PoseNumberResiduesStaticFeatureGenerator()
  {
    numResiduesFeatureGenerator = softDiscretizedNumberFeatureGenerator(0.0, 1000, 10, true);
    numResiduesFeatureGenerator->initialize(defaultExecutionContext(), doubleType);
  }

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return poseClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    EnumerationPtr numResiduesFeatEnumeration = numResiduesFeatureGenerator->getFeaturesEnumeration();
    DefaultEnumerationPtr resEnum = new DefaultEnumeration("Number residue features enumeration");
    for (size_t i = 0; i < numResiduesFeatEnumeration->getNumElements(); ++i)
      resEnum->addElement(context, "NRStatF:" + numResiduesFeatEnumeration->getElementName(i));
    return resEnum;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    Variable length((double)(inputs[0].getObjectAndCast<Pose> ())->getLength());
    callback.sense(0, numResiduesFeatureGenerator, &length, 1.0);
  }

protected:
  friend class PoseNumberResiduesStaticFeatureGeneratorClass;

  FeatureGeneratorPtr numResiduesFeatureGenerator;
};

/*
 * Static features
 */
class PoseStaticFeatureGenerator : public CompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    // inputs and variables
    size_t input = builder.addInput(poseClass);

    // select features
    builder.startSelection();

    // features
    builder.addFunction(new PoseNumberResiduesStaticFeatureGenerator(), input);
    builder.addFunction(new PoseResidueHistogramStaticFeatureGenerator(), input);

    // end selection
    builder.finishSelectionWithFunction(concatenateFeatureGenerator());
  }

protected:
  friend class PoseStaticFeatureGeneratorClass;
};

typedef ReferenceCountedObjectPtr<PoseStaticFeatureGenerator> PoseStaticFeatureGeneratorPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSESTATICFEATUREGENERATOR_H_
