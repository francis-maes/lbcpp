/*-----------------------------------------.---------------------------------.
| Filename: BlindPoseFeatureGenerator.h    | Blind Pose Feature Generator    |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 11, 2012  2:08:00 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_BLINDPOSEFEATUREGENERATOR_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_BLINDPOSEFEATUREGENERATOR_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

# include "../Pose.h"

namespace lbcpp
{

class BlindPoseFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return poseClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr resEnum = new DefaultEnumeration("Blind pose features enumeration");
    resEnum->addElement(context, "BlindF");
    return resEnum;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
    {callback.sense(0, 1.0);}

protected:
  friend class BlindPoseFeatureGeneratorClass;
};

typedef ReferenceCountedObjectPtr<BlindPoseFeatureGenerator> BlindPoseFeatureGeneratorPtr;

extern FeatureGeneratorPtr blindPoseFeatureGenerator();

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_BLINDPOSEFEATUREGENERATOR_H_
