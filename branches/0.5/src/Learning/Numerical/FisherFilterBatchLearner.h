/*-----------------------------------------.---------------------------------.
| Filename: FisherFilterBatchLearner.h     | Fisher Filter Batch Learner     |
| Author  : Julien Becker                  |                                 |
| Started : 16/11/2011 22:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_FISHER_FILTER_BATCH_LEARNER_H_
# define LBCPP_LEARNING_NUMERICAL_FISHER_FILTER_BATCH_LEARNER_H_

# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Data/RandomVariable.h>
# include "FisherFilterLearnableFunction.h"

namespace lbcpp
{

class FisherFilterBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return fisherFilterLearnableFunctionClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const FisherFilterLearnableFunctionPtr& function = f.staticCast<FisherFilterLearnableFunction>();

    if (!checkHasAtLeastOneExemples(trainingData))
      return false;
    EnumerationPtr enumeration = trainingData[0]->getVariable(0).getObjectAndCast<DoubleVector>()->getElementsEnumeration();
    const size_t numFeatures = enumeration->getNumElements();
    jassert(numFeatures != 0);

    std::vector<ScalarVariableMeanPtr> means(numFeatures);
    for (size_t i = 0; i < numFeatures; ++i)
      means[i] = new ScalarVariableMean();

    const size_t numClasses = getNumClasses(trainingData[0]->getVariable(1));

    std::vector<std::vector<ScalarVariableMeanAndVariancePtr> > statByClasses(numFeatures);
    for (size_t i = 0; i < numFeatures; ++i)
    {
      std::vector<ScalarVariableMeanAndVariancePtr> stats(numClasses);
      for (size_t j = 0; j < numClasses; ++j)
        stats[j] = new ScalarVariableMeanAndVariance();
      statByClasses[i] = stats;
    }

    for (size_t i = 0; i < trainingData.size(); ++i)
    {
      DoubleVectorPtr input = trainingData[i]->getVariable(0).getObjectAndCast<DoubleVector>();
      for (size_t j = 0; j < numFeatures; ++j)
      {
        const double value = input->getElement(j).getDouble();
        means[j]->push(value);
        statByClasses[j][getSupervisionClass(trainingData[i]->getVariable(1))]->push(value);
      }
    }

    std::vector<std::pair<double, size_t> > fisherScores(numFeatures);
    for (size_t i = 0; i < numFeatures; ++i)
    {
      double numerator = 0.f;
      double denominator = 0.f;
      for (size_t j = 0; j < numClasses; ++j)
      {
        const double count = statByClasses[i][j]->getCount();
        const double diffMeans = statByClasses[i][j]->getMean() - means[i]->getMean();
        numerator += count * diffMeans * diffMeans;
        const double variance = statByClasses[i][j]->getVariance();
        denominator += count * variance * variance;
      }
      fisherScores[i] = std::make_pair(denominator < 1e-6 ? 0.f : numerator / denominator, i);
    }

    sort(fisherScores.rbegin(), fisherScores.rend());

    function->featuresMap.resize(numFeatures);
    for (size_t i = 0; i < numFeatures; ++i)
      function->featuresMap[i] = fisherScores[i].second;
    for (size_t i = 0; i < function->numSelectedFeatures; ++i)
      function->outputEnumeration->getElement(i)->setName(enumeration->getElementName(fisherScores[i].second));

    for (size_t i = 0; i < numFeatures; ++i)
    {
      context.enterScope(T("Feature ") + String((int)i));
      context.resultCallback(T("Rank"), i);
      //context.resultCallback(T("Id"), fisherScores[i].second);
      context.resultCallback(T("Name"), enumeration->getElementName(fisherScores[i].second));
      context.resultCallback(T("Fisher Score"), fisherScores[i].first);
      context.leaveScope(fisherScores[i].first);
    }

    return true;
  }

protected:
  friend class FisherFilterBatchLearnerClass;

  size_t getNumClasses(const Variable& v) const
  {
    if (v.inheritsFrom(booleanType))
      return 2;
    if (v.inheritsFrom(probabilityType))
      return 2;
    if (v.inheritsFrom(doubleVectorClass(anyType, doubleType)))
      return v.getObjectAndCast<DoubleVector>()->getElementsEnumeration()->getNumElements();
    jassertfalse;
    return 0;
  }

  size_t getSupervisionClass(const Variable& v) const
  {
    if (v.inheritsFrom(booleanType))
      return v.getBoolean() ? 1 : 0;
    if (v.inheritsFrom(probabilityType))
      return v.getDouble() > 0.5 ? 1 : 0;
    if (v.inheritsFrom(doubleVectorClass(anyType, doubleType)))
      return v.getObjectAndCast<DoubleVector>()->getIndexOfMaximumValue();
    jassertfalse;
    return 0;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_FISHER_FILTER_BATCH_LEARNER_H_
