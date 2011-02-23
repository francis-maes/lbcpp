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

  virtual void startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
  {
    for (size_t i = 0; i < learners.size(); ++i)
      learners[i]->startLearning(context, function, maxIterations, trainingData, validationData);
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

  void startLearningAndAddLearner(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
  {
    const OnlineLearnerPtr& onlineLearner = function->getOnlineLearner();
    jassert(onlineLearner);
    learners.push_back(onlineLearner);
    onlineLearner->startLearning(context, function, maxIterations, trainingData, validationData);
  }

  Variable finishLearningIterationAndRemoveFinishedLearners(size_t iteration)
  {
    if (learners.empty())
      return Variable();

    std::vector<OnlineLearnerPtr> remainingLearners;
    remainingLearners.reserve(learners.size());
    
    ContainerPtr res = vector(doubleType, learners.size());
    for (size_t i = 0; i < learners.size(); ++i)
    {
      const OnlineLearnerPtr& learner = learners[i];
      double objectiveValue = DBL_MAX;
      if (learner->finishLearningIteration(iteration, objectiveValue))
        learner->finishLearning();
      else
        remainingLearners.push_back(learner);
      jassert(objectiveValue < DBL_MAX);
      res->setElement(i, objectiveValue);
    }
    learners.swap(remainingLearners);
    return learners.size() > 1 ? res : res->getElement(0);
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class CompositeOnlineLearnerClass;

  std::vector<OnlineLearnerPtr> learners;
};

typedef ReferenceCountedObjectPtr<CompositeOnlineLearner> CompositeOnlineLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_COMPOSITE_H_
