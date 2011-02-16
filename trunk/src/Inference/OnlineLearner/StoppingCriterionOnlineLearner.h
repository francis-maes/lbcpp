/*-----------------------------------------.---------------------------------.
| Filename: StoppingCriterionOnlineLearner.h| Adds a stopping criterion to an|
| Author  : Francis Maes                   |  online learner                 |
| Started : 26/05/2010 17:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_ONLINE_LEARNER_STOPPING_CRITERION_H_
# define LBCPP_INFERENCE_ONLINE_LEARNER_STOPPING_CRITERION_H_

# include <lbcpp/Core/Pair.h>
# include <lbcpp/Core/Container.h>
# include <lbcpp/Function/StoppingCriterion.h>
# include <lbcpp/Inference/InferenceOnlineLearner.h>
# include <lbcpp/Inference/Inference.h>

namespace lbcpp
{

class OldStoppingCriterionOnlineLearner : public UpdatableOnlineLearner
{
public:
  OldStoppingCriterionOnlineLearner(StoppingCriterionPtr criterion, bool restoreBestParametersWhenLearningStops, LearnerUpdateFrequency criterionTestFrequency)
    : UpdatableOnlineLearner(criterionTestFrequency), 
        criterion(criterion), restoreBestParametersWhenLearningStops(restoreBestParametersWhenLearningStops),
        learningStopped(false), bestDefaultScore(-DBL_MAX), iteration(0)
     {criterion->reset();}

  OldStoppingCriterionOnlineLearner() : learningStopped(false), bestDefaultScore(-DBL_MAX) {}

  virtual void startLearningCallback(ExecutionContext& context)
  {
    UpdatableOnlineLearner::startLearningCallback(context);
    learningStopped = false;
    bestParameters = ObjectPtr();
    bestDefaultScore = -DBL_MAX;
    bestScores.clear();
    iteration = 0;
  }

  virtual bool isLearningStopped() const
    {return learningStopped;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    UpdatableOnlineLearner::clone(context, target);
    if (criterion)
      target.staticCast<OldStoppingCriterionOnlineLearner>()->criterion = criterion->cloneAndCast<StoppingCriterion>(context);
  }

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
  {
    UpdatableOnlineLearner::getScores(res);
    if (restoreBestParametersWhenLearningStops)
    {
      size_t n = res.size();
      res.resize(2 * n);
      jassert(!bestScores.size() || bestScores.size() == n);
      for (size_t i = 0; i < n; ++i)
      {
        res[i + n].first = T("Best ") + res[i].first;
        res[i + n].second = bestScores.size() ? bestScores[i].second : res[i].second;
      }
    }
  }

  virtual double getDefaultScore() const
    {return bestDefaultScore;}

private:
  friend class OldStoppingCriterionOnlineLearnerClass;

  StoppingCriterionPtr criterion;
  bool restoreBestParametersWhenLearningStops;

  bool learningStopped;
  Variable bestParameters;
  double bestDefaultScore;
  std::vector< std::pair<String, double> > bestScores;
  size_t iteration;

  virtual void update(ExecutionContext& context, const InferencePtr& inference)
  {
    double defaultScore = UpdatableOnlineLearner::getDefaultScore();
    //context.informationCallback(T("StoppingCriterionOnlineLearner::update"), T("Score: ") + String(score));
    Variable parameters = inference->getParametersCopy(context);
    if (parameters.exists() && restoreBestParametersWhenLearningStops && isNumberValid(defaultScore) && defaultScore > bestDefaultScore)
    {
      bestParameters = parameters;
      bestDefaultScore = defaultScore;
      bestScores.clear();
      UpdatableOnlineLearner::getScores(bestScores);
      //context.informationCallback(T("StoppingCriterionOnlineLearner::update"), T("New best score: ") + String(bestScore));
    }
    if (!isNumberValid(defaultScore) || criterion->shouldStop(defaultScore))
    {
      context.informationCallback(T("StoppingCriterionOnlineLearner::update"), T("Stopped, last score = ") + String(defaultScore) + T(" best score = ") + String(bestDefaultScore));
      learningStopped = true;
      if (bestParameters.exists() && bestDefaultScore > defaultScore)
      {
        context.informationCallback(T("StoppingCriterionOnlineLearner::update"), T("Restoring parameters that led to score ") + String(bestDefaultScore));
        inference->setParameters(bestParameters);
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_ONLINE_LEARNER_STOPPING_CRITERION_H_
