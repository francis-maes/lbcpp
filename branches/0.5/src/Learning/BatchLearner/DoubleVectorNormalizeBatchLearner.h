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
#if JUCE_DEBUG && 0
    std::cout << "DoubleVectorNormalizeBatchLearner::Means : ";
    for (size_t i = 0; i < dimension; ++i)
      std::cout << target->means[i] << ", ";
    std::cout << std::endl;

    std::cout << "DoubleVectorNormalizeBatchLearner::StdDev: ";
    for (size_t i = 0; i < dimension; ++i)
      std::cout << target->variances[i] << ", ";
    std::cout << std::endl;
#endif // !JUCE_DEBUG
    return true;
  }

protected:
  friend class DoubleVectorNormalizeBatchLearnerClass;

  bool computeVariances;
  bool computeMeans;
};

class ConcatenatedDoubleVectorNormalizeBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return concatenatedDoubleVectorNormalizeFunctionClass;}
  
  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    if (!checkHasAtLeastOneExemples(trainingData))
    {
      context.errorCallback(T("DoubleVectorNormalizeBatchLearner::train"), T("No training data !"));
      return false;
    }

    ConcatenatedDoubleVectorNormalizeFunctionPtr target = function.staticCast<ConcatenatedDoubleVectorNormalizeFunction>();
    jassert(trainingData[0]->getVariable(0).dynamicCast<DoubleVector>());
    EnumerationPtr enumeration = trainingData[0]->getVariable(0).dynamicCast<DoubleVector>()->getElementsEnumeration();
    jassert(target && enumeration);

    size_t index = 0;
    target->zFactors.resize(enumeration->getNumElements(), 1.f);
    recursivelyAssignFactors(enumeration, index, target->zFactors);

#if JUCE_DEBUG && 0
    std::cout << "ConcatenatedDoubleVectorNormalizeBatchLearner::zFactors : ";
    for (size_t i = 0; i < target->zFactors.size(); ++i)
      std::cout << target->zFactors[i] << ", ";
    std::cout << std::endl;
#endif // !JUCE_DEBUG

    return true;
  }

private:
  void recursivelyAssignFactors(const EnumerationPtr& enumeration, size_t& index, std::vector<double>& results) const
  {
    ConcatenateEnumerationPtr concatenateEnumeration = enumeration.dynamicCast<ConcatenateEnumeration>();
    if (concatenateEnumeration)
    {
      for (size_t i = 0; i < concatenateEnumeration->getNumSubEnumerations(); ++i)
        recursivelyAssignFactors(concatenateEnumeration->getSubEnumeration(i), index, results);
    }
    else
    {
      const size_t n = enumeration->getNumElements();
      for (size_t i = 0; i < n; ++i, ++index)
        results[index] = (double)n;
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_BATCH_LEARNER_DOUBLE_VECTOR_NORMALIZE_H_
