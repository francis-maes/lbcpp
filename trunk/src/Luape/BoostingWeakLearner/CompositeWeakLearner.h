/*-----------------------------------------.---------------------------------.
| Filename: CompositeWeakLearner.h         | Composite Weak Learner          |
| Author  : Francis Maes                   |                                 |
| Started : 15/12/2011 12:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_WEAK_LEARNER_COMPOSITE_H_
# define LBCPP_LUAPE_WEAK_LEARNER_COMPOSITE_H_

# include <lbcpp/Luape/LuapeLearner.h>

namespace lbcpp
{

class CompositeWeakLearner : public BoostingWeakLearner
{
public:
  CompositeWeakLearner(const std::vector<BoostingWeakLearnerPtr>& weakLearners)
    : weakLearners(weakLearners) {}
  CompositeWeakLearner(BoostingWeakLearnerPtr weakLearner1, BoostingWeakLearnerPtr weakLearner2)
    : weakLearners(2) {weakLearners[0] = weakLearner1; weakLearners[1] = weakLearner2;}
  CompositeWeakLearner() {}

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
  {
    for (size_t i = 0; i < weakLearners.size(); ++i)
      if (!weakLearners[i]->initialize(context, function))
        return false;
    return true;
  }
  
  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const std::vector<size_t>& examples, double& weakObjective) const
  {
    weakObjective = -DBL_MAX;
    LuapeNodePtr res;
    for (size_t i = 0; i < weakLearners.size(); ++i)
    {
      double objective;
      LuapeNodePtr node = weakLearners[i]->learn(context, structureLearner, examples, objective);
      if (objective > weakObjective)
      {
        weakObjective = objective;
        res = node;
      }
    }
    return res;
  }

protected:
  friend class CompositeWeakLearnerClass;

  std::vector<BoostingWeakLearnerPtr> weakLearners;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_COMPOSITE_H_
