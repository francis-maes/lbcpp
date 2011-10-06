/*-----------------------------------------.---------------------------------.
| Filename: DoubleVectorNormalizeBatc...h  | Function related to             |
| Author  : Julien Becker                  |                    DoubleVector |
| Started : 06/10/2011 10:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_BATCH_LEARNER_DOUBLE_VECTOR_NORMALIZE_H_
# define LBCPP_CORE_BATCH_LEARNER_DOUBLE_VECTOR_NORMALIZE_H_

# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Data/RandomVariable.h>
# include "../../Core/Function/DoubleVectorFunction.h"

namespace lbcpp
{

class DoubleVectorNormalizeBatchLearner : public BatchLearner
{
public:
  DoubleVectorNormalizeBatchLearner(bool computeVariances = true, bool computeMeans = false)
    : computeVariances(computeVariances), computeMeans(computeMeans) {}

  virtual TypePtr getRequiredFunctionType() const
    {return doubleVectorNormalizeFunctionClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    if (!checkHasAtLeastOneExemples(trainingData))
    {
      context.errorCallback(T("DoubleVectorNormalizeBatchLearner::train"), T("No training data !"));
      return false;
    }

    DoubleVectorNormalizeFunctionPtr target = function.staticCast<DoubleVectorNormalizeFunction>();
    jassert(trainingData[0]->getVariable(0).dynamicCast<DoubleVector>());
    EnumerationPtr enumeration = trainingData[0]->getVariable(0).dynamicCast<DoubleVector>()->getElementsEnumeration();
    jassert(target && enumeration);
    // Initialize
    const size_t dimension = enumeration->getNumElements();
    target->variances.resize(dimension, 1.f);
    target->means.resize(dimension, 0.f);

    std::vector<ScalarVariableMeanAndVariancePtr> meansAndVariances(dimension);
    for (size_t i = 0; i < dimension; ++i)
      meansAndVariances[i] = new ScalarVariableMeanAndVariance();
    // Learn
    const size_t n = trainingData.size();
    for (size_t i = 0; i < n; ++i)
    {
      DoubleVectorPtr data =  trainingData[i]->getVariable(0).dynamicCast<DoubleVector>();
      jassert(data);
      const size_t numValues = data->getNumElements();
      for (size_t i = 0; i < numValues; ++i)
      {
        Variable v = data->getElement(i);
        meansAndVariances[i]->push(v.exists() ? v.getDouble() : 0.f);
      }
    }

    if (computeMeans)
      for (size_t i = 0; i < dimension; ++i)
        target->means[i] = meansAndVariances[i]->getMean();

    if (computeVariances)
      for (size_t i = 0; i < dimension; ++i)
      {
        target->variances[i] = meansAndVariances[i]->getStandardDeviation();
        if (target->variances[i] < 1e-6) // Numerical unstability
          target->variances[i] = 1.f;
      }

    return true;
  }

protected:
  friend class DoubleVectorNormalizeBatchLearnerClass;

  bool computeVariances;
  bool computeMeans;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_BATCH_LEARNER_DOUBLE_VECTOR_NORMALIZE_H_
