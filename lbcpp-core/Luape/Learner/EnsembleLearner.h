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

  virtual ExpressionPtr createInitialNode(ExecutionContext& context, const LuapeInferencePtr& problem)
  {
    if (problem.isInstanceOf<LuapeClassifier>())
      return new VectorSumExpression(problem.staticCast<LuapeClassifier>()->getLabels(), false);
    else if (problem.isInstanceOf<LuapeBinaryClassifier>())
      return new ScalarSumExpression(true, true);
    else
    {
      jassert(false); // not implemented yet
      return ExpressionPtr();
    }
  }

  virtual IndexSetPtr getSubSamples(ExecutionContext& context, size_t modelIndex, const IndexSetPtr& examples) const
    {return examples;}

  virtual ExpressionPtr learn(ExecutionContext& context, const ExpressionPtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    const SequenceExpressionPtr& sequenceNode = node.staticCast<SequenceExpression>();
    sequenceNode->clearNodes();
    sequenceNode->reserveNodes(ensembleSize);
    //bool ok = true;

    for (size_t i = 0; i < ensembleSize; ++i)
    {
      //Object::displayObjectAllocationInfo(std::cout);
      if (verbose)
      {
        context.enterScope(T("Iteration ") + String((int)i));
        context.resultCallback(T("iteration"), i);
      }
      ExpressionPtr baseModel = subLearn(context, baseLearner, ExpressionPtr(), problem, getSubSamples(context, i, examples));
      if (baseModel)
      {
        sequenceNode->pushNode(context, baseModel, problem->getSamplesCaches());
        //sequenceNode->setNode(i, ExpressionPtr()); // TMP: save memory !
      }
      else
        return ExpressionPtr();

      if (verbose)
      {
        double trainingScore, validationScore;
        evaluatePredictions(context, problem, trainingScore, validationScore);
        context.leaveScope();
        context.progressCallback(new ProgressionState(i + 1, ensembleSize, T(" base models")));
      }
    }
    return sequenceNode;
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    LuapeLearner::clone(context, target);
    if (baseLearner)
      target.staticCast<EnsembleLearner>()->baseLearner = baseLearner->cloneAndCast<LuapeLearner>(context);
  }

protected:
  friend class EnsembleLearnerClass;

  LuapeLearnerPtr baseLearner;
  size_t ensembleSize;
};

class BaggingLearner : public EnsembleLearner
{
public:
  BaggingLearner(const LuapeLearnerPtr& baseLearner, size_t ensembleSize)
    : EnsembleLearner(baseLearner, ensembleSize) {}
  BaggingLearner() {}

  virtual IndexSetPtr getSubSamples(ExecutionContext& context, size_t modelIndex, const IndexSetPtr& examples) const
    {return examples->sampleBootStrap(context.getRandomGenerator());}
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ENSEMBLE_H_
