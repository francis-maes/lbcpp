/*-----------------------------------------.---------------------------------.
| Filename: GeneralFeatures.h              | GeneralFeatures                 |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Feb 28, 2012  9:50:59 AM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_GENERALFEATURES_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_GENERALFEATURES_H_

# include <lbcpp/Data/DoubleVector.h>

# include <vector>

namespace lbcpp
{

class GeneralFeatures;
typedef ReferenceCountedObjectPtr<GeneralFeatures> GeneralFeaturesPtr;

class GeneralFeatures : public Object
{
public:

  GeneralFeatures() : initialized(false) {}

  Variable initialize(ExecutionContext& context, const Variable& input)
  {
    selfInitialization(context, input);

    staticFeatures = computeStaticFeatures(context, input).getObjectAndCast<DoubleVector> ();
    dynamicFeatures = computeDynamicFeatures(context, input).getObjectAndCast<DoubleVector> ();
    numStaticFeatures = staticFeatures->getNumElements();
    numDynamicFeatures = dynamicFeatures->getNumElements();

    features = new DenseDoubleVector(numStaticFeatures + numDynamicFeatures, 0.0);

    for (size_t i = 0; i < numStaticFeatures; i++)
      features->setElement(i, staticFeatures->getElement(i));

    for (size_t i = 0; i < numDynamicFeatures; i++)
      features->setElement(numStaticFeatures + i, dynamicFeatures->getElement(i));

    initialized = true;

    return features;
  }

  Variable computeFeatures(ExecutionContext& context, const Variable& input)
  {
    jassert(initialized);

    dynamicFeatures = computeDynamicFeatures(context, input).getObjectAndCast<DoubleVector> ();

    for (size_t i = 0; i < numDynamicFeatures; i++)
      features->setElement(numStaticFeatures + i, dynamicFeatures->getElement(i));
    return features;
  }

  virtual Variable computeStaticFeatures(ExecutionContext& context, const Variable& input) = 0;
  virtual Variable computeDynamicFeatures(ExecutionContext& context, const Variable& input) = 0;

protected:
  friend class GeneralFeaturesClass;

  virtual void selfInitialization(ExecutionContext& context, const Variable& input) = 0;

  DoubleVectorPtr concatenateFeatures(const std::vector<DoubleVectorPtr>& featuresToConcatenate) const
  {
    size_t numFeatures = 0;
    for (size_t i = 0; i < featuresToConcatenate.size(); i++)
      numFeatures += featuresToConcatenate[i]->getNumElements();

    DoubleVectorPtr concatenatedFeatures = new DenseDoubleVector(numFeatures, 0.0);

    size_t j = 0;
    DoubleVectorPtr tmpFeat;
    for (size_t i = 0; i < featuresToConcatenate.size(); i++)
    {
      tmpFeat = featuresToConcatenate[i];

      for (size_t k = 0; k < tmpFeat->getNumElements(); k++)
        concatenatedFeatures->setElement(j++, tmpFeat->getElement(k));
    }
    return concatenatedFeatures;
  }

  bool initialized;
  size_t numStaticFeatures;
  size_t numDynamicFeatures;
  DoubleVectorPtr features;
  DoubleVectorPtr staticFeatures;
  DoubleVectorPtr dynamicFeatures;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_GENERALFEATURES_H_
