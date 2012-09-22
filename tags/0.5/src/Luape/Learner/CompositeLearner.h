/*-----------------------------------------.---------------------------------.
| Filename: CompositeLearner.h             | Composite Learner               |
| Author  : Francis Maes                   |                                 |
| Started : 02/01/2012 17:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_COMPOSITE_H_
# define LBCPP_LUAPE_LEARNER_COMPOSITE_H_

# include <lbcpp/Luape/LuapeLearner.h>

namespace lbcpp
{

class CompositeLearner : public LuapeLearner
{
public:
  CompositeLearner(const std::vector<LuapeLearnerPtr>& learners)
    : learners(learners) {}
  CompositeLearner(const LuapeLearnerPtr& learner1, const LuapeLearnerPtr& learner2)
    : learners(2) {learners[0] = learner1; learners[1] = learner2;}
  CompositeLearner() {}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    // default behavior is sequential
    LuapeNodePtr res = node;
    //bool ok = true;
    for (size_t i = 0; i < learners.size(); ++i)
    {
      res = learners[i]->learn(context, res, problem, examples);
      if (!res)
        break;
    }
    return res;
  }

  void addLearner(const LuapeLearnerPtr& learner)
    {learners.push_back(learner);}

protected:
  friend class CompositeLearnerClass;

  std::vector<LuapeLearnerPtr> learners;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_COMPOSITE_H_
