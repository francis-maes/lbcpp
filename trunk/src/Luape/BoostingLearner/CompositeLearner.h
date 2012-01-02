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

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
  {
    bool ok = true;
    for (size_t i = 0; i < learners.size(); ++i)
      ok &= learners[i]->initialize(context, function);
    return ok;
  }

  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
  {
    bool ok = true;
    for (size_t i = 0; i < learners.size(); ++i)
      ok &= learners[i]->setExamples(context, isTrainingData, data);
    return ok;
  }

  virtual bool learn(ExecutionContext& context)
  {
    bool ok = true;
    for (size_t i = 0; i < learners.size(); ++i)
    {
      ok &= learners[i]->learn(context);
      jassert(ok);
    }
    return ok;
  }

  void addLearner(const LuapeLearnerPtr& learner)
    {learners.push_back(learner);}

protected:
  friend class CompositeLearnerClass;

  std::vector<LuapeLearnerPtr> learners;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_COMPOSITE_H_
