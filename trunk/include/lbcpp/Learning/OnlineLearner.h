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
  virtual void startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations) {}

  virtual void startLearningIteration(size_t iteration) {}

  virtual void startEpisode() {}
  virtual void finishEpisode() {}

  virtual bool finishLearningIteration(size_t iteration) {return false;} // returns true if learning is finished

  virtual void finishLearning() {}
};

extern OnlineLearnerPtr concatenateOnlineLearner(const OnlineLearnerPtr& learner1, const OnlineLearnerPtr& learner2);
extern OnlineLearnerPtr concatenateOnlineLearners(const std::vector<OnlineLearnerPtr>& learners);

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_H_
