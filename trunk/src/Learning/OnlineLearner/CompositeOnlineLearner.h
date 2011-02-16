/*-----------------------------------------.---------------------------------.
| Filename: CompositeOnlineLearner.h       | Composite Online Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 16/02/2011 13:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_ONLINE_LEARNER_COMPOSITE_H_
# define LBCPP_LEARNING_ONLINE_LEARNER_COMPOSITE_H_

# include <lbcpp/Learning/OnlineLearner.h>

namespace lbcpp
{

class CompositeOnlineLearner : public OnlineLearner
{
public:
  CompositeOnlineLearner(const std::vector<OnlineLearnerPtr>& learners)
    : learners(learners) {}
  CompositeOnlineLearner(const OnlineLearnerPtr& learner1, const OnlineLearnerPtr& learner2)
    : learners(2) {learners[0] = learner1; learners[1] = learner2;}
  CompositeOnlineLearner() {}

  virtual void startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations)
  {
    for (size_t i = 0; i < learners.size(); ++i)
      learners[i]->startLearning(context, function, maxIterations);
  }

  virtual void startLearningIteration(size_t iteration)
  {
    for (size_t i = 0; i < learners.size(); ++i)
      learners[i]->startLearningIteration(i);
  }

  virtual void startEpisode()
  {
    for (size_t i = 0; i < learners.size(); ++i)
      learners[i]->startEpisode();
  }

  virtual void finishEpisode()
  {
    for (int i = learners.size() - 1; i >= 0; --i)
      learners[i]->finishEpisode();
  }

  virtual bool finishLearningIteration(size_t iteration)
  {
    bool learningIsFinished = false;
    for (int i = learners.size() - 1; i >= 0; --i)
      learningIsFinished |= learners[i]->finishLearningIteration(i);
    return learningIsFinished;
  }

  virtual void finishLearning()
  {
    for (int i = learners.size() - 1; i >= 0; --i)
      learners[i]->finishLearning();
  }

  size_t getNumLearners() const
    {return learners.size();}

  void startLearningAndAddLearner(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations)
  {
    const OnlineLearnerPtr& onlineLearner = function->getOnlineLearner();
    jassert(onlineLearner);
    learners.push_back(onlineLearner);
    onlineLearner->startLearning(context, function, maxIterations);
  }

  void finishLearningIterationAndRemoveFinishedLearners(size_t iteration)
  {
    for (int i = learners.size() - 1; i >= 0; --i)
      if (learners[i]->finishLearningIteration(i))
        learners.erase(learners.begin() + i);
  }

protected:
  friend class CompositeOnlineLearnerClass;

  std::vector<OnlineLearnerPtr> learners;
};

typedef ReferenceCountedObjectPtr<CompositeOnlineLearner> CompositeOnlineLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_COMPOSITE_H_
