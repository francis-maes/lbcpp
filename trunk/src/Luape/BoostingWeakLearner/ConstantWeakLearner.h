/*-----------------------------------------.---------------------------------.
| Filename: ConstantWeakLearner.h          | Constant Weak Learner           |
| Author  : Francis Maes                   |                                 |
| Started : 15/12/2011 12:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_WEAK_LEARNER_CONSTANT_H_
# define LBCPP_LUAPE_WEAK_LEARNER_CONSTANT_H_

# include <lbcpp/Luape/LuapeLearner.h>

namespace lbcpp
{

class ConstantWeakLearner : public FiniteBoostingWeakLearner
{
public:
  ConstantWeakLearner() : weakNode(new LuapeConstantNode(true)) {}
 
  virtual bool getCandidateWeakNodes(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& res) const
    {res.push_back(weakNode); return true;}

  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const IndexSetPtr& examples, double& weakObjective) const
  {
    weakObjective = computeWeakObjective(context, structureLearner, weakNode, examples);
    return makeContribution(context, structureLearner, weakNode, weakObjective, examples);
  }

protected:
  LuapeNodePtr weakNode;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_CONSTANT_H_
