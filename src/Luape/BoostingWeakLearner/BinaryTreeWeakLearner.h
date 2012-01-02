/*-----------------------------------------.---------------------------------.
| Filename: BinaryTreeWeakLearner.h        | Binary Tree Weak Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2011 17:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_BINARY_TREE_WEAK_H_
# define LBCPP_LUAPE_LEARNER_BINARY_TREE_WEAK_H_

# include <lbcpp/Luape/LuapeLearner.h>
# include <lbcpp/Luape/LuapeCache.h>

namespace lbcpp
{

class BinaryTreeWeakLearner : public BoostingWeakLearner
{
public:
  BinaryTreeWeakLearner(BoostingWeakLearnerPtr conditionLearner, BoostingWeakLearnerPtr subLearner)
    : conditionLearner(conditionLearner), subLearner(subLearner) {}
  BinaryTreeWeakLearner() {}

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
    {return conditionLearner->initialize(context, function) && subLearner->initialize(context, function);}
  
  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, const IndexSetPtr& examples, bool verbose, double& weakObjective)
  {
    /*
    ** Learn condition and retrieve condition values
    */
    jassert(examples->size());
    LuapeNodePtr conditionNode = conditionLearner->learn(context, structureLearner, examples, verbose, weakObjective);
    if (!conditionNode)
    {
      context.errorCallback(T("Could not learn condition with ") + String((int)examples->size()) + T(" examples"));
      return LuapeNodePtr();
    }
    LuapeTestNodePtr res = conditionNode.dynamicCast<LuapeTestNode>();
    if (!res)
      return conditionNode; // probably a constant node

    LuapeSampleVectorPtr testValues = structureLearner->getTrainingCache()->getSamples(context, res->getCondition(), examples);

    /*
    ** Dispatch examples
    */
    IndexSetPtr failureExamples, successExamples, missingExamples;
    res->dispatchIndices(testValues, failureExamples, successExamples, missingExamples);

    /*
    ** Call sub-learners on sub-examples
    */
    weakObjective = 0.0;
    LuapeNodePtr successNode = subLearn(context, structureLearner, successExamples, verbose, weakObjective);
    if (successNode)
      res->setSuccess(successNode);
    LuapeNodePtr failureNode = subLearn(context, structureLearner, failureExamples, verbose, weakObjective);
    if (failureNode)
      res->setFailure(failureNode);
    LuapeNodePtr missingNode = subLearn(context, structureLearner, missingExamples, verbose, weakObjective);
    if (missingNode)
      res->setMissing(missingNode);
    return res;
  }

protected:
  friend class BinaryTreeWeakLearnerClass;

  BoostingWeakLearnerPtr conditionLearner;
  BoostingWeakLearnerPtr subLearner;
  
  LuapeNodePtr subLearn(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, const IndexSetPtr& subExamples, bool verbose, double& weakObjective) const
  {
    if (subExamples->size() == 0)
      return LuapeNodePtr();    
    double objective;
    LuapeNodePtr node = subLearner->learn(context, structureLearner, subExamples, verbose, objective);
    if (node)
      weakObjective += objective;
    return node;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_BINARY_TREE_WEAK_H_