/*-----------------------------------------.---------------------------------.
| Filename: OnlineLearner.h                | Online Learner Base Class       |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_ONLINE_LEARNER_H_
# define LBCPP_LEARNING_ONLINE_LEARNER_H_

# include "../Core/Function.h"
# include "../Function/StoppingCriterion.h"

namespace lbcpp
{

class OnlineLearner : public Object
{
public:
  virtual bool startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) = 0;

  virtual void startLearningIteration(size_t iteration) {}

  virtual void startEpisode(const ObjectPtr& inputs) {}
  virtual void finishEpisode(const ObjectPtr& inputs, const Variable& output) {}

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize) {return false;} // returns true if learning is finished

  virtual void finishLearning() {}

  lbcpp_UseDebuggingNewOperator
};

extern OnlineLearnerPtr stoppingCriterionOnlineLearner(StoppingCriterionPtr stoppingCriterion);
extern OnlineLearnerPtr restoreBestParametersOnlineLearner();
extern OnlineLearnerPtr evaluatorOnlineLearner(bool evaluateOnTrainingData = true, bool evaluateOnValidationData = true);

extern OnlineLearnerPtr compositeOnlineLearner(const std::vector<OnlineLearnerPtr>& learners);
extern OnlineLearnerPtr compositeOnlineLearner(const OnlineLearnerPtr& learner1,
                                        const OnlineLearnerPtr& learner2,
                                        const OnlineLearnerPtr& learner3 = OnlineLearnerPtr(),
                                        const OnlineLearnerPtr& learner4 = OnlineLearnerPtr());

extern OnlineLearnerPtr hierarchicalOnlineLearner();

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_H_