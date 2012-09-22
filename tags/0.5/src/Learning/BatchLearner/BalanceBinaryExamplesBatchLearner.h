/*-----------------------------------------.---------------------------------.
| Filename: BalanceBinaryExamplesBatch...h | Balance Binary Examples         |
| Author  : Julien Becker                  |                Batch Learners   |
| Started : 28/10/2011 11:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_BATCH_LEARNER_BALANCE_BINARY_EXAMPLES_H_
# define LBCPP_LEARNING_BATCH_LEARNER_BALANCE_BINARY_EXAMPLES_H_

# include <lbcpp/Learning/BatchLearner.h>

namespace lbcpp
{

class BalanceBinaryExamplesBatchLearner : public DecoratorBatchLearner
{
public:
  BalanceBinaryExamplesBatchLearner(BatchLearnerPtr decorated)
    : DecoratorBatchLearner(decorated) {}

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    std::vector<ObjectPtr> balancedTrainingData;
    balanceBinaryExamples(context, trainingData, balancedTrainingData);
    randomizeExamples(context, balancedTrainingData);
    context.informationCallback(T("Num. duplicated examples: ") + String((int)(balancedTrainingData.size() - trainingData.size())));

    return decorated->train(context, function, balancedTrainingData, validationData);
  }

protected:
  friend class BalanceBinaryExamplesBatchLearnerClass;

  BalanceBinaryExamplesBatchLearner() {}

  void balanceBinaryExamples(ExecutionContext& context, const std::vector<ObjectPtr>& fromData, std::vector<ObjectPtr>& toData) const
  {
    if (fromData.size() == 0)
      return;
    // Counts numbers of + and -
    std::vector<size_t> trueIndices;
    std::vector<size_t> falseIndices;

    size_t supervisionIndex = fromData[0]->getNumVariables() - 1;
    for (size_t i = 0; i < fromData.size(); ++i)
    {
      const Variable output = fromData[i]->getVariable(supervisionIndex);
      jassert(output.inheritsFrom(sumType(booleanType, probabilityType)));
      if (output.isBoolean() && output.getBoolean()
          || output.isDouble() && output.getDouble() > 0.5f)
        trueIndices.push_back(i);
      else
        falseIndices.push_back(i);
    }
    // Select the minority
    std::vector<size_t>& minority = (trueIndices.size() > falseIndices.size()) ? falseIndices : trueIndices;
    if (minority.size() == 0)
    {
      toData = fromData;
      return;
    }

    const size_t n = (fromData.size() - minority.size()) * 2;
    toData.resize(n);
    for (size_t i = 0; i < fromData.size(); ++i)
      toData[i] = fromData[i];

    const RandomGeneratorPtr rand = context.getRandomGenerator();
    for (size_t i = fromData.size(); i < n; ++i)
      toData[i] = fromData[minority[rand->sampleSize(minority.size())]];
  }

  void randomizeExamples(ExecutionContext& context, std::vector<ObjectPtr>& data) const
  {
    const RandomGeneratorPtr rand = context.getRandomGenerator();
    for (size_t i = 1; i < data.size(); ++i)
    {
      const size_t newIndex = rand->sampleSize(i + 1);
      ObjectPtr tmp = data[newIndex];
      data[newIndex] = data[i];
      data[i] = tmp;
    }
  }
};

};

#endif // !LBCPP_LEARNING_BATCH_LEARNER_BALANCE_BINARY_EXAMPLES_H_
