/*-----------------------------------------.---------------------------------.
| Filename: PoseFeatures.h                 | PoseFeatures                    |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Feb 16, 2012  5:02:56 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSEFEATURES_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSEFEATURES_H_

# include <lbcpp/Data/DoubleVector.h>

# include <vector>

# include "../Pose.h"

namespace lbcpp
{

class PoseFeatures;
typedef ReferenceCountedObjectPtr<PoseFeatures> PoseFeaturesPtr;

class PoseFeatures : public Object
{
public:

  PoseFeatures() : initialized(false) {}

  void initialize(ExecutionContext& context, Variable& input)
  {
    selfInitialization(context, input);

    staticFeatures = computeStaticFeatures(context, input).getObjectAndCast<DenseDoubleVector> ();
    dynamicFeatures = computeDynamicFeatures(context, input).getObjectAndCast<DenseDoubleVector> ();
    numStaticfeatures = staticFeatures->getNumElements();
    numDynamicFeatures = dynamicFeatures->getNumElements();

    features = new DenseDoubleVector(numStaticfeatures + numDynamicFeatures, 0.0);

    for (size_t i = 0; i < numStaticfeatures; i++)
      features->setValue(i, staticFeatures->getValue(i));

    for (size_t i = 0; i < numDynamicFeatures; i++)
      features->setValue(numStaticfeatures + i, dynamicFeatures->getValue(i));

    initialized = true;
  }

  Variable computeFeatures(ExecutionContext& context, Variable& input)
  {
    jassert(initialized);

    dynamicFeatures = computeDynamicFeatures(context, input).getObjectAndCast<DenseDoubleVector> ();

    for (size_t i = 0; i < numDynamicFeatures; i++)
      features->setValue(numStaticfeatures + i, dynamicFeatures->getValue(i));
    return features;
  }

  virtual Variable computeStaticFeatures(ExecutionContext& context, Variable& input) = 0;
  virtual Variable computeDynamicFeatures(ExecutionContext& context, Variable& input) = 0;

protected:
  friend class PoseFeaturesClass;

  virtual void selfInitialization(ExecutionContext& context, Variable& input) {}

  DenseDoubleVectorPtr concatenateFeatures(const std::vector<DenseDoubleVectorPtr>& featuresToConcatenate) const
  {
    size_t numFeatures = 0;
    for (size_t i = 0; i < featuresToConcatenate.size(); i++)
      numFeatures += featuresToConcatenate[i]->getNumElements();

    DenseDoubleVectorPtr concatenatedFeatures = new DenseDoubleVector(numFeatures, 0.0);

    size_t j = 0;
    DenseDoubleVectorPtr tmpFeat;
    for (size_t i = 0; i < featuresToConcatenate.size(); i++)
    {
      tmpFeat = featuresToConcatenate[i];

      for (size_t k = 0; k < tmpFeat->getNumElements(); k++)
        staticFeatures->setValue(j++, tmpFeat->getValue(k));
    }
    return concatenatedFeatures;
  }

  bool initialized;
  size_t numStaticfeatures;
  size_t numDynamicFeatures;
  DenseDoubleVectorPtr features;
  DenseDoubleVectorPtr staticFeatures;
  DenseDoubleVectorPtr dynamicFeatures;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_POSEFEATURES_H_
