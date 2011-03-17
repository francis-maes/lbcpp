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
  CompositeOnlineLearner(const OnlineLearnerPtr& learner1, const OnlineLearnerPtr& learner2, const OnlineLearnerPtr& learner3, const OnlineLearnerPtr& learner4)
  {
    if (learner1) learners.push_back(learner1);
    if (learner2) learners.push_back(learner2);
    if (learner3) learners.push_back(learner3);
    if (learner4) learners.push_back(learner4);
  }
  CompositeOnlineLearner() {}

  virtual bool startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
  {
    bool ok = true;
    for (size_t i = 0; i < learners.size(); ++i)
      ok &= learners[i]->startLearning(context, function, maxIterations, trainingData, validationData);
    return ok;
  }

  virtual void startLearningIteration(size_t iteration)
  {
    for (size_t i = 0; i < learners.size(); ++i)
      learners[i]->startLearningIteration(i);
  }

  virtual void startEpisode(const ObjectPtr& inputs)
  {
    for (size_t i = 0; i < learners.size(); ++i)
      learners[i]->startEpisode(inputs);
  }

  virtual void finishEpisode(const ObjectPtr& inputs, const Variable& output)
  {
    for (size_t i = 0; i < learners.size(); ++i)
      learners[i]->finishEpisode(inputs, output);
  }

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
  {
    bool learningIsFinished = false;
    for (size_t i = 0; i < learners.size(); ++i)
      learningIsFinished |= learners[i]->finishLearningIteration(iteration, objectiveValueToMinimize);
    return learningIsFinished;
  }

  virtual void finishLearning()
  {
    for (size_t i = 0; i < learners.size(); ++i)
      learners[i]->finishLearning();
  }

  size_t getNumLearners() const
    {return learners.size();}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class CompositeOnlineLearnerClass;

  std::vector<OnlineLearnerPtr> learners;
};

typedef ReferenceCountedObjectPtr<CompositeOnlineLearner> CompositeOnlineLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_COMPOSITE_H_
