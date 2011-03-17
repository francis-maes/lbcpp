/*-----------------------------------------.---------------------------------.
| Filename: HierarchicalOnlineLearner.h    | Hierarchical Online Learner     |
| Author  : Francis Maes                   |                                 |
| Started : 17/03/2011 11:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_ONLINE_LEARNER_HIERARCHICAL_H_
# define LBCPP_LEARNING_ONLINE_LEARNER_HIERARCHICAL_H_

# include "CompositeOnlineLearner.h"

namespace lbcpp
{

class HierarchicalOnlineLearner : public CompositeOnlineLearner
{
public:
  typedef std::pair<FunctionPtr, std::vector<size_t> >  FunctionOnlineLearnerInfo;
  typedef std::vector<FunctionOnlineLearnerInfo>  FunctionOnlineLearnerInfoVector;

  virtual bool startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
  {
    info.clear();
    getAllFunctionsThatHaveAnOnlineLearner(function, info);

    // store learners
    learners.resize(info.size());
    for (size_t i = 0; i < learners.size(); ++i)
    {
      learners[i] = info[i].first->getOnlineLearner();
      jassert(learners[i]);
    }

    // start each learner
    bool ok = true;
    std::vector<bool> learnersToRemove(info.size(), false);
    for (size_t i = 0; i < info.size(); ++i)
    {
      FunctionPtr subFunction = info[i].first;
      bool startOk;
      if (subFunction == function)
        startOk = learners[i]->startLearning(context, function, maxIterations, trainingData, validationData);
      else
        startOk = learners[i]->startLearning(context, subFunction, maxIterations, std::vector<ObjectPtr>(), std::vector<ObjectPtr>());
      if (!startOk)
      {
        learnersToRemove[i] = true;
        ok = false;
      }
    }

    // remove learners that did not start properly
    removeLearners(learnersToRemove, false);
    return ok;
  }

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
  {
    if (learners.empty())
      return true; // learning is finished

    std::vector<bool> learnersToRemove(learners.size(), false);
    ContainerPtr res = vector(doubleType, learners.size());
    for (size_t i = 0; i < learners.size(); ++i)
    {
      const OnlineLearnerPtr& learner = learners[i];
      double objectiveValue = DBL_MAX;
      if (learner->finishLearningIteration(iteration, objectiveValue))
      {
        learnersToRemove[i] = true;
        // also terminate sub learners:
        for (size_t j = 0; j < info[i].second.size(); ++j)
          learnersToRemove[info[i].second[j]] = true;
      }
      jassert(objectiveValue < DBL_MAX);
      res->setElement(i, objectiveValue);
    }
    removeLearners(learnersToRemove, true);
    lastLearningIterationResult = learners.size() > 1 ? res : res->getElement(0);
    return learners.empty();
  }

  const Variable& getLastLearningIterationResult() const
    {return lastLearningIterationResult;}

  virtual void finishLearning()
  {
    CompositeOnlineLearner::finishLearning();
    learners.clear();
    info.clear();
  }

protected:
  Variable lastLearningIterationResult;
  FunctionOnlineLearnerInfoVector info;

  static void getAllFunctionsThatHaveAnOnlineLearnerRecursively(const ObjectPtr& object, std::vector<size_t>& parents, FunctionOnlineLearnerInfoVector& res)
  {
    FunctionPtr function = object.dynamicCast<Function>();
    bool isFunctionToLearn = (function && function->hasOnlineLearner());
    if (isFunctionToLearn)
    {
      size_t index = res.size();
      res.push_back(FunctionOnlineLearnerInfo(function, std::vector<size_t>()));
      for (size_t i = 0; i < parents.size(); ++i)
        res[parents[i]].second.push_back(index);

      parents.push_back(index);
    }

    std::vector<ObjectPtr> objects;
    object->getChildObjects(objects);
    for (size_t i = 0; i < objects.size(); ++i)
      getAllFunctionsThatHaveAnOnlineLearnerRecursively(objects[i], parents, res);

    if (isFunctionToLearn)
      parents.pop_back();
  }

  static void getAllFunctionsThatHaveAnOnlineLearner(const FunctionPtr& function, FunctionOnlineLearnerInfoVector& res)
  {
    std::vector<size_t> parents;
    getAllFunctionsThatHaveAnOnlineLearnerRecursively(function, parents, res);
  }

  void removeLearners(const std::vector<bool>& learnersToRemove, bool callFinishLearning)
  {
    jassert(info.size() == learners.size());
    jassert(info.size() == learnersToRemove.size());
    for (int i = info.size() - 1; i >= 0; --i)
      if (learnersToRemove[i])
      {
        if (callFinishLearning)
          learners[i]->finishLearning();
        info.erase(info.begin() + i);
        learners.erase(learners.begin() + i);
      }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_HIERARCHICAL_H_
