/*-----------------------------------------.---------------------------------.
| Filename: SupervisedExamplesBatchLear...h| Supervised Examples             |
| Author  : Julien Becker                  |                Batch Learners   |
| Started : 18/02/2011 10:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_BATCH_LEARNER_SUPERVISED_EXAMPLES_H_
# define LBCPP_LEARNING_BATCH_LEARNER_SUPERVISED_EXAMPLES_H_

# include <lbcpp/Learning/BatchLearner.h>

namespace lbcpp
{

class FilterUnsupervisedExamplesBatchLearner : public DecoratorBatchLearner
{
public:
  FilterUnsupervisedExamplesBatchLearner(BatchLearnerPtr decorated, bool randomizeOrder)
    : DecoratorBatchLearner(decorated), randomizeOrder(randomizeOrder) {}
  FilterUnsupervisedExamplesBatchLearner() {}
  
  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    std::vector<ObjectPtr> supervisedTrainingData;
    keepSupervisedExamplesOnly(context, trainingData, supervisedTrainingData);
    std::vector<ObjectPtr> supervisedValidationData;
    keepSupervisedExamplesOnly(context, validationData, supervisedValidationData);
    return decorated->train(context, function, supervisedTrainingData, supervisedValidationData);
  }

protected:
  friend class FilterUnsupervisedExamplesBatchLearnerClass;

  bool randomizeOrder;

  void keepSupervisedExamplesOnly(ExecutionContext& context, const std::vector<ObjectPtr>& fromData, std::vector<ObjectPtr>& toData) const
  {
    size_t n = fromData.size();
    if (n == 0)
      return;
    size_t supervisionIndex = fromData[0]->getNumVariables() - 1;
    toData.reserve(n);
    
    std::vector<size_t> order;
    if (randomizeOrder)
      context.getRandomGenerator()->sampleOrder(0, n, order);
    for (size_t i = 0; i < n; ++i)
    {
      size_t index = randomizeOrder ? order[i] : i;
      if (fromData[index]->getVariable(supervisionIndex).exists())
        toData.push_back(fromData[index]);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_SUPERVISED_EXAMPLES_H_
