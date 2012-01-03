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

  virtual void setFunction(const LuapeInferencePtr& function)
  {
    LuapeLearner::setFunction(function);
    baseLearner->setFunction(function);
  }

  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
    {return baseLearner->setExamples(context, isTrainingData, data);}

  virtual bool initialize(ExecutionContext& context)
    {return baseLearner->initialize(context);}

  virtual bool learn(ExecutionContext& context)
  {
    const LuapeSequenceNodePtr& sequenceNode = function->getRootNode().staticCast<LuapeSequenceNode>();
    sequenceNode->clearNodes();
    sequenceNode->reserveNodes(ensembleSize);
    bool ok = true;
    for (size_t i = 0; i < ensembleSize; ++i)
    {
      // FIXME: baseLearner should return a LuapeNode
      ok &= baseLearner->initialize(context);
      ok &= baseLearner->learn(context);
      ok &= baseLearner->finalize(context);
      jassert(ok);
    }
    return ok;
  }

protected:
  friend class EnsembleLearnerClass;

  LuapeLearnerPtr baseLearner;
  size_t ensembleSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ENSEMBLE_H_
