/*-----------------------------------------.---------------------------------.
| Filename: OnlineLearner.h                | Online Learner Base Class       |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_ONLINE_LEARNER_H_
# define LBCPP_LEARNING_ONLINE_LEARNER_H_

# include <lbcpp/Function/Function.h>

namespace lbcpp
{

class OnlineLearner : public Object
{
public:
  virtual void startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) {}

  virtual void startLearningIteration(size_t iteration) {}

  virtual void startEpisode() {}
  virtual void finishEpisode() {}

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize) {return false;} // returns true if learning is finished

  virtual void finishLearning() {}
};

OnlineLearnerPtr stoppingCriterionOnlineLearner(StoppingCriterionPtr stoppingCriterion);
OnlineLearnerPtr restoreBestParametersOnlineLearner();
OnlineLearnerPtr evaluatorOnlineLearner(EvaluatorPtr evaluator);

OnlineLearnerPtr compositeOnlineLearner(const std::vector<OnlineLearnerPtr>& learners);
OnlineLearnerPtr compositeOnlineLearner(const OnlineLearnerPtr& learner1,
                                        const OnlineLearnerPtr& learner2,
                                        const OnlineLearnerPtr& learner3 = OnlineLearnerPtr(),
                                        const OnlineLearnerPtr& learner4 = OnlineLearnerPtr());

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_H_
