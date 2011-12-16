/*-----------------------------------------.---------------------------------.
| Filename: MeanAndVarianceBatchLearner.h  | Mean And Variance Batch Learner |
| Author  : Julien Becker                  |                                 |
| Started : 28/11/2011 19:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_MEAN_AND_VARIANCE_BATCH_LEARNER_H_
# define LBCPP_LEARNING_NUMERICAL_MEAN_AND_VARIANCE_BATCH_LEARNER_H_

# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include "MeanAndVarianceLearnableFunction.h"

namespace lbcpp
{

class MeanAndVarianceBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return meanAndVarianceLearnableFunctionClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const MeanAndVarianceLearnableFunctionPtr& function = f.staticCast<MeanAndVarianceLearnableFunction>();

    if (!checkHasAtLeastOneExemples(trainingData))
      return false;

    EnumerationPtr enumeration = trainingData[0]->getVariable(0).getObjectAndCast<DoubleVector>()->getElementsEnumeration();
    jassert(enumeration->getNumElements() != 0);

    ComputeMeanAndVarianceFeatureGeneratorCallback callback(enumeration);
    for (size_t i = 0; i < trainingData.size(); ++i)
    {
      trainingData[i]->getVariable(0).getObjectAndCast<DoubleVector>()->computeFeatures(callback);
      callback.finalizeSense();
    }

    callback.computeMean(function->means);
    callback.computeStandardDeviation(function->standardDeviations);

    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_MEAN_AND_VARIANCE_BATCH_LEARNER_H_
