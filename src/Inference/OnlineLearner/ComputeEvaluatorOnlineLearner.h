/*-----------------------------------------.---------------------------------.
| Filename: ComputeEvaluatorOnlineLearner.h| An online learner that computes |
| Author  : Francis Maes                   |  an evaluator                   |
| Started : 01/11/2010 13:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_ONLINE_LEARNER_COMPUTE_EVALUATOR_H_
# define LBCPP_INFERENCE_ONLINE_LEARNER_COMPUTE_EVALUATOR_H_

# include <lbcpp/Core/Pair.h>
# include <lbcpp/Core/Vector.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Inference/InferenceOnlineLearner.h>
# include <lbcpp/Inference/InferenceBatchLearner.h>

namespace lbcpp
{

class ComputeEvaluatorOnlineLearner : public InferenceOnlineLearner
{
public:
  ComputeEvaluatorOnlineLearner(EvaluatorPtr evaluator, bool computeOnValidationData)
    : evaluator(evaluator), computeOnValidationData(computeOnValidationData), lastDefaultScore(0.0) {}
  ComputeEvaluatorOnlineLearner() : computeOnValidationData(false), lastDefaultScore(0.0) {}

  virtual void passFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& batchLearnerInput)
  {
    EvaluatorPtr eval = evaluator->cloneAndCast<Evaluator>(context);
    InferenceExampleVectorPtr examples = computeOnValidationData ? batchLearnerInput->getValidationExamples() : batchLearnerInput->getTrainingExamples();
    String workUnitName(T("Evaluate on "));
    workUnitName += (computeOnValidationData ? T("validation") : T("training"));
    workUnitName += T(" data");
    inference->evaluate(context, examples, eval, workUnitName);
    lastScores.clear();
    eval->getScores(lastScores);
    lastDefaultScore = eval->getDefaultScore();
    InferenceOnlineLearner::passFinishedCallback(context, inference, batchLearnerInput);
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
  bool computeOnValidationData;

  std::vector< std::pair<String, double> > lastScores;
  double lastDefaultScore;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_ONLINE_LEARNER_COMPUTE_EVALUATOR_H_
