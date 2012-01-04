/*-----------------------------------------.---------------------------------.
| Filename: EnsembleLearner.h              | Ensemble Learner                |
| Author  : Francis Maes                   |                                 |
| Started : 02/01/2012 17:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_ENSEMBLE_H_
# define LBCPP_LUAPE_LEARNER_ENSEMBLE_H_

# include <lbcpp/Luape/LuapeLearner.h>

namespace lbcpp
{

class EnsembleLearner : public LuapeLearner
{
public:
  EnsembleLearner(const LuapeLearnerPtr& baseLearner, size_t ensembleSize)
    : baseLearner(baseLearner), ensembleSize(ensembleSize) {}
  EnsembleLearner() : ensembleSize(0) {}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    const LuapeSequenceNodePtr& sequenceNode = node.staticCast<LuapeSequenceNode>();
    sequenceNode->clearNodes();
    sequenceNode->reserveNodes(ensembleSize);
    bool ok = true;
    for (size_t i = 0; i < ensembleSize; ++i)
    {
      LuapeNodePtr baseModel = baseLearner->createInitialNode(context);
      baseModel = baseLearner->learn(context, baseModel, problem, examples);
      if (!baseModel)
        return LuapeNodePtr();
      sequenceNode->pushNode(context, baseModel); // todo: cachesToUpdate
    }
    return sequenceNode;
  }

protected:
  friend class EnsembleLearnerClass;

  LuapeLearnerPtr baseLearner;
  size_t ensembleSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ENSEMBLE_H_
