/*-----------------------------------------.---------------------------------.
| Filename: ScoreVectorSequenceRegressio..h| Score Vector Sequence           |
| Author  : Francis Maes                   |   Regression Error Evaluator    |
| Started : 27/04/2010 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EVALUATOR_SCORE_VECTOR_SEQUENCE_REGRESSION_ERROR_H_
# define LBCPP_EVALUATOR_SCORE_VECTOR_SEQUENCE_REGRESSION_ERROR_H_

# include "../../InferenceData/ScoreVectorSequence.h"

namespace lbcpp
{

class ScoreVectorSequenceRegressionEvaluator : public Evaluator
{
public:
  ScoreVectorSequenceRegressionEvaluator(const String& name)
    : Evaluator(name), regressionEvaluator(new RegressionErrorEvaluator(name)) {}

  virtual String toString() const
    {return regressionEvaluator->toString();}

  virtual double getDefaultScore() const
    {return regressionEvaluator->getDefaultScore();}

  virtual void addPrediction(const Variable& predictedObject, const Variable& correctObject)
  {
    ScoreVectorSequencePtr predicted = predictedObject.dynamicCast<ScoreVectorSequence>();
    ScoreVectorSequencePtr correct = correctObject.dynamicCast<ScoreVectorSequence>();
    if (!predicted || !correct)
      return;

    jassert(correct->getNumScores() >= predicted->getNumScores());
    jassert(correct->getDictionary() == predicted->getDictionary());
    size_t n = predicted->size();
    size_t s = predicted->getNumScores();
    jassert(correct->size() == n);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = 0; j < s; ++j)
        regressionEvaluator->addPrediction(predicted->getScore(i, j), correct->getScore(i, j));
  }

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {regressionEvaluator->getScores(res);}

protected:
  RegressionErrorEvaluatorPtr regressionEvaluator;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATOR_SCORE_VECTOR_SEQUENCE_REGRESSION_ERROR_H_
