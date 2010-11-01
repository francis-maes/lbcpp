/*-----------------------------------------.---------------------------------.
| Filename: ComputeEvaluatorOnlineLearner.h| An online learner that computes |
| Author  : Francis Maes                   |  an evaluator                   |
| Started : 01/11/2010 13:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_ONLINE_LEARNER_COMPUTE_EVALUATOR_H_
# define LBCPP_INFERENCE_ONLINE_LEARNER_COMPUTE_EVALUATOR_H_

# include <lbcpp/Data/Pair.h>
# include <lbcpp/Data/Vector.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Inference/InferenceOnlineLearner.h>
# include <lbcpp/Inference/InferenceContext.h>

namespace lbcpp
{

class ComputeEvaluatorOnlineLearner : public UpdatableOnlineLearner
{
public:
  ComputeEvaluatorOnlineLearner(EvaluatorPtr evaluator, ContainerPtr examples, LearnerUpdateFrequency computeFrequency)
    : UpdatableOnlineLearner(computeFrequency), evaluator(evaluator), examples(examples), lastDefaultScore(0.0)
    {}
  ComputeEvaluatorOnlineLearner() : lastDefaultScore(0.0) {}

  virtual void update(InferenceContextWeakPtr context, const InferencePtr& inference)
  {
    EvaluatorPtr eval = evaluator->cloneAndCast<Evaluator>();
    context->evaluate(inference, examples, eval);
    lastScores.clear();
    eval->getScores(lastScores);
    lastDefaultScore = eval->getDefaultScore();
  }

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
  {
    if (previousLearner)
      previousLearner->getScores(res);
    res.reserve(res.size() + lastScores.size());
    for (size_t i = 0; i < lastScores.size(); ++i)
      res.push_back(lastScores[i]);
  }

  virtual double getDefaultScore() const
    {return lastDefaultScore;}

protected:
  friend class ComputeEvaluatorOnlineLearnerClass;

  EvaluatorPtr evaluator;
  ContainerPtr examples;

  std::vector< std::pair<String, double> > lastScores;
  double lastDefaultScore;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_ONLINE_LEARNER_COMPUTE_EVALUATOR_H_
