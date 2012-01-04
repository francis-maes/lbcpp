/*-----------------------------------------.---------------------------------.
| Filename: BinaryTreeWeakLearner.h        | Binary Tree Weak Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2011 17:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_BINARY_TREE_WEAK_H_
# define LBCPP_LUAPE_LEARNER_BINARY_TREE_WEAK_H_

# include <lbcpp/Luape/BoostingWeakLearner.h>
# include <lbcpp/Luape/LuapeCache.h>

namespace lbcpp
{

class BinaryTreeWeakLearner : public BoostingWeakLearner
{
public:
  BinaryTreeWeakLearner(BoostingWeakLearnerPtr conditionLearner, BoostingWeakLearnerPtr subLearner)
    : conditionLearner(conditionLearner), subLearner(subLearner) {}
  BinaryTreeWeakLearner() {}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    /*
    ** Learn condition and retrieve condition values
    */
    jassert(examples->size());
    LuapeNodePtr conditionNode = conditionLearner->learn(context, LuapeNodePtr(), problem, examples);
    if (!conditionNode)
    {
      context.errorCallback(T("Could not learn condition with ") + String((int)examples->size()) + T(" examples"));
      return LuapeNodePtr();
    }
    LuapeTestNodePtr res = conditionNode.dynamicCast<LuapeTestNode>();
    if (!res)
      return conditionNode; // probably a constant node

    LuapeSampleVectorPtr testValues = problem->getTrainingCache()->getSamples(context, res->getCondition(), examples);

    /*
    ** Dispatch examples
    */
    IndexSetPtr failureExamples, successExamples, missingExamples;
    res->dispatchIndices(testValues, failureExamples, successExamples, missingExamples);

    /*
    ** Call sub-learners on sub-examples
    */
    bestWeakObjectiveValue = 0.0;
    LuapeNodePtr successNode = subLearn(context, problem, successExamples, bestWeakObjectiveValue);
    if (successNode)
      res->setSuccess(successNode);
    LuapeNodePtr failureNode = subLearn(context, problem, failureExamples, bestWeakObjectiveValue);
    if (failureNode)
      res->setFailure(failureNode);
    LuapeNodePtr missingNode = subLearn(context, problem, missingExamples, bestWeakObjectiveValue);
    if (missingNode)
      res->setMissing(missingNode);
    return res;
  }

protected:
  friend class BinaryTreeWeakLearnerClass;

  BoostingWeakLearnerPtr conditionLearner;
  BoostingWeakLearnerPtr subLearner;
  
  LuapeNodePtr subLearn(ExecutionContext& context, const LuapeInferencePtr& problem, const IndexSetPtr& subExamples, double& weakObjective) const
  {
    if (subExamples->size() == 0)
      return LuapeNodePtr();    
    LuapeNodePtr node = subLearner->learn(context, LuapeNodePtr(), problem, subExamples);
    if (node)
      weakObjective += subLearner->getBestWeakObjectiveValue();
    return node;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_BINARY_TREE_WEAK_H_
