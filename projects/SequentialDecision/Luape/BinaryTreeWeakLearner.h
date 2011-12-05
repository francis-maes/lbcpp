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
    jassert(examples.size());
    LuapeTestNodePtr res = conditionLearner->learn(context, structureLearner, examples, weakObjective).staticCast<LuapeTestNode>();
    if (!res)
      return LuapeNodePtr();

    const unsigned char* testValues = structureLearner->getTrainingSamples()->compute(context, res->getCondition()).staticCast<BooleanVector>()->getData();
    std::vector<size_t> successExamples;
    successExamples.reserve(examples.size() / 3);
    std::vector<size_t> failureExamples;
    failureExamples.reserve(examples.size() / 3);
    std::vector<size_t> missingExamples;
    missingExamples.reserve(examples.size() / 3);
    
    for (size_t i = 0; i < examples.size(); ++i)
    {
      size_t example = examples[i];
      switch (testValues[i])
      {
      case 0: failureExamples.push_back(example); break;
      case 1: successExamples.push_back(example); break;
      case 2: missingExamples.push_back(example); break;
      default: jassert(false);
      }
    }

    weakObjective = 0.0;
    LuapeNodePtr successNode = subLearn(context, structureLearner, successExamples, weakObjective);
    if (successNode)
      res->setSuccess(successNode);

    LuapeNodePtr failureNode = subLearn(context, structureLearner, failureExamples, weakObjective);
    if (failureNode)
      res->setFailure(failureNode);

    LuapeNodePtr missingNode = subLearn(context, structureLearner, missingExamples, weakObjective);
    if (missingNode)
      res->setMissing(missingNode);
    return res;
  }

protected:
  friend class BinaryTreeWeakLearnerClass;

  BoostingWeakLearnerPtr conditionLearner;
  BoostingWeakLearnerPtr subLearner;
  
  LuapeNodePtr subLearn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const std::vector<size_t>& subExamples, double& weakObjective) const
  {
    if (subExamples.size() == 0)
      return LuapeNodePtr();    
    double objective;
    LuapeNodePtr node = subLearner->learn(context, structureLearner, subExamples, objective);
    if (node)
      weakObjective += objective;
    return node;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_BINARY_TREE_WEAK_H_
