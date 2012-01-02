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
 
  virtual bool getCandidateWeakNodes(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& res) const
    {res.push_back(weakNode); return true;}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, const IndexSetPtr& examples, bool verbose, double& weakObjective)
  {
    weakObjective = computeWeakObjective(context, structureLearner, weakNode, examples);
    return makeContribution(context, structureLearner, weakNode, weakObjective, examples);
  }

protected:
  LuapeNodePtr weakNode;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_CONSTANT_H_