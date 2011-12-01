/*-----------------------------------------.---------------------------------.
| Filename: BinaryTreeWeakLearner.h        | Binary Tree Weak Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2012 17:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_BINARY_TREE_WEAK_H_
# define LBCPP_LUAPE_LEARNER_BINARY_TREE_WEAK_H_

# include "LuapeLearner.h"

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
  
  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const std::vector<size_t>& examples, double& weakObjective) const
  {
    LuapeTestNodePtr res = conditionLearner->learn(context, structureLearner, examples, weakObjective).staticCast<LuapeTestNode>();
    if (!res)
      return LuapeNodePtr();

    BooleanVectorPtr testValues = structureLearner->getTrainingSamples()->compute(context, res->getCondition()).staticCast<BooleanVector>();
    std::vector<size_t> successExamples;
    successExamples.reserve(examples.size());
    std::vector<size_t> failureExamples;
    failureExamples.reserve(examples.size());
    for (size_t i = 0; i < examples.size(); ++i)
    {
      size_t example = examples[i];
      if (testValues->get(example))
        successExamples.push_back(example);
      else
        failureExamples.push_back(example);
    }

    LuapeNodePtr successNode = subLearner->learn(context, structureLearner, successExamples, weakObjective);
    if (successNode)
      res->setSuccess(successNode);

    LuapeNodePtr failureNode = subLearner->learn(context, structureLearner, failureExamples, weakObjective);
    if (failureNode)
      res->setFailure(failureNode);
    return res;
  }

protected:
  friend class BinaryTreeWeakLearnerClass;

  BoostingWeakLearnerPtr conditionLearner;
  BoostingWeakLearnerPtr subLearner;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_BINARY_TREE_WEAK_H_
