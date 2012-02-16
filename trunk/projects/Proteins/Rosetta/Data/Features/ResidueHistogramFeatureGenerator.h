/*-----------------------------------------.---------------------------------.
| Filename: ResidueHistogram..Generator.h  | Residue Histogram               |
| Author  : Alejandro Marcos Alvarez       | Feature Generator               |
| Started : Feb 16, 2012  3:24:43 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_RESIDUEHISTOGRAMFEATUREGENERATOR_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_RESIDUEHISTOGRAMFEATUREGENERATOR_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include "../Pose.h"
# include "../../../Data/AminoAcid.h"

namespace lbcpp
{

class ResidueHistogramFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return poseClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
    {return aminoAcidTypeEnumeration;}

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    DenseDoubleVectorPtr histogram = (inputs[0].getObjectAndCast<Pose> ())->getHistogram();
    for (size_t i = 0; i < histogram->getNumElements(); i++)
      callback.sense(i, histogram->getValue(i));
  }

protected:
  friend class ResidueHistogramFeatureGeneratorClass;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_RESIDUEHISTOGRAMFEATUREGENERATOR_H_
