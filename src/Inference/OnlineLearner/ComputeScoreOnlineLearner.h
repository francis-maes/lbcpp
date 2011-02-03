/*-----------------------------------------.---------------------------------.
| Filename: ComputeScoreOnlineLearner.h    | An online learner that computes |
| Author  : Francis Maes                   |  the current score              |
| Started : 01/11/2010 12:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_ONLINE_LEARNER_COMPUTE_SCORE_H_
# define LBCPP_INFERENCE_ONLINE_LEARNER_COMPUTE_SCORE_H_

# include <lbcpp/Function/Function.h>
# include <lbcpp/Inference/InferenceOnlineLearner.h>

namespace lbcpp
{

class ComputeScoreOnlineLearner : public UpdatableOnlineLearner
{
public:
  ComputeScoreOnlineLearner(const String& scoreName, FunctionPtr scoreFunction, LearnerUpdateFrequency computeFrequency)
    : UpdatableOnlineLearner(computeFrequency), scoreName(scoreName), scoreFunction(scoreFunction), lastScoreValue(0.0)
  {
    checkInheritance(scoreFunction->getInputType(), inferenceClass);
    checkInheritance(scoreFunction->getOutputType(inferenceClass), doubleType);
  }
  ComputeScoreOnlineLearner() : lastScoreValue(0.0) {}

  virtual void update(ExecutionContext& context, const InferencePtr& inference)
    {lastScoreValue = scoreFunction->computeFunction(context, inference).getDouble();}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
  {
    if (previousLearner)
      previousLearner->getScores(res);
    res.push_back(std::make_pair(scoreName, lastScoreValue));
  }

  virtual double getDefaultScore() const
    {return lastScoreValue;}

protected:
  friend class ComputeScoreOnlineLearnerClass;

  String scoreName;
  FunctionPtr scoreFunction;
  double lastScoreValue;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_ONLINE_LEARNER_COMPUTE_SCORE_H_
