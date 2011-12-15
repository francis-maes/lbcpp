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

class ConstantWeakLearner : public BoostingWeakLearner
{
public:
  ConstantWeakLearner() : weakNode(new LuapeConstantNode(true)) {}

  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const std::vector<size_t>& examples, double& weakObjective) const
  {
    weakObjective = computeWeakObjective(context, structureLearner, weakNode, examples);
    return makeContribution(context, structureLearner, weakNode, examples);
  }

protected:
  LuapeNodePtr weakNode;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_CONSTANT_H_
