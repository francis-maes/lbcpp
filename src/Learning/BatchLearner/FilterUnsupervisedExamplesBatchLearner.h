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
 FilterUnsupervisedExamplesBatchLearner(BatchLearnerPtr decorated)
    : DecoratorBatchLearner(decorated) {}

 FilterUnsupervisedExamplesBatchLearner() {}
  
  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    std::vector<ObjectPtr> supervisedTrainingData;
    keepSupervisedExamplesOnly(trainingData, supervisedTrainingData);
    std::vector<ObjectPtr> supervisedValidationData;
    keepSupervisedExamplesOnly(validationData, supervisedValidationData);
    return decorated->train(context, function, supervisedTrainingData, supervisedValidationData);
  }

protected:
  void keepSupervisedExamplesOnly(const std::vector<ObjectPtr>& fromData, std::vector<ObjectPtr>& toData) const
  {
    size_t n = fromData.size();
    if (n == 0)
      return;
    size_t supervisionIndex = fromData[0]->getNumVariables() - 1;
    toData.reserve(n);
    for (size_t i = 0; i < n; ++i)
      if (fromData[i]->getVariable(supervisionIndex).exists())
        toData.push_back(fromData[i]);
  }
};

};

#endif // !LBCPP_LEARNING_BATCH_LEARNER_SUPERVISED_EXAMPLES_H_
