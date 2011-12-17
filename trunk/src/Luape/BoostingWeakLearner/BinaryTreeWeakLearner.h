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
  
  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const IndexSetPtr& examples, double& weakObjective) const
  {
    /*
    ** Learn condition and retrieve condition values
    */
    jassert(examples->size());
    LuapeNodePtr conditionNode = conditionLearner->learn(context, structureLearner, examples, weakObjective);
    LuapeTestNodePtr res = conditionNode.dynamicCast<LuapeTestNode>();
    if (!res)
      return conditionNode; // probably a constant node

    LuapeSampleVectorPtr testValues = structureLearner->getTrainingSamples()->getSamples(context, res->getCondition(), examples);

    /*
    ** Dispatch examples
    */
    IndexSetPtr successExamples = new IndexSet();
    IndexSetPtr failureExamples = new IndexSet();
    IndexSetPtr missingExamples = new IndexSet();
    for (LuapeSampleVector::const_iterator it = testValues->begin(); it != testValues->end(); ++it)
    {
      switch (it.getRawBoolean())
      {
      case 0: failureExamples->append(it.getIndex()); break;
      case 1: successExamples->append(it.getIndex()); break;
      case 2: missingExamples->append(it.getIndex()); break;
      default: jassert(false);
      }
    }

    /*
    ** Call sub-learners on sub-examples
    */
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
  
  LuapeNodePtr subLearn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const IndexSetPtr& subExamples, double& weakObjective) const
  {
    if (subExamples->size() == 0)
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
