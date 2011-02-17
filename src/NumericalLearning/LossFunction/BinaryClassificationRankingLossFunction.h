/*-----------------------------------------.---------------------------------.
| Filename: BinaryClassificationRanking...h| Binary Classification Ranking   |
| Author  : Francis Maes                   |  Loss                           |
| Started : 26/10/2010 14:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_BINARY_CLASSIFICATION_RANKING_H_
# define LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_BINARY_CLASSIFICATION_RANKING_H_

# include "AdditiveRankingLossFunction.h"
# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class BinaryClassificationRankingLossFunction : public AdditiveRankingLossFunction
{
public:
  BinaryClassificationRankingLossFunction(DiscriminativeLossFunctionPtr baseLoss, const std::vector<double>& costs)
    : AdditiveRankingLossFunction(baseLoss, costs) {}
  BinaryClassificationRankingLossFunction() {}

  virtual double computeScore(const BinaryClassificationConfusionMatrix& matrix) const = 0;

  double computeScoreDifference(BinaryClassificationConfusionMatrix& matrix, double refScore, bool type1Error) const
  {
    bool b1 = !type1Error;
    bool b2 = type1Error;
    double score1 = refScore, score2 = refScore;

    size_t count = 0;
    if (matrix.getCount(b2, b1)) // if there is an error
    {
      // remove an error
      matrix.removePrediction(b2, b1);
      matrix.addPrediction(b1, b1);
      score1 = computeScore(matrix);
      matrix.removePrediction(b1, b1);
      matrix.addPrediction(b2, b1);
      ++count;
    }
    if (matrix.getCount(b1, b1)) // if there is one correct
    {
      // add an error
      matrix.removePrediction(b1, b1);
      matrix.addPrediction(b2, b1);
      score2 = computeScore(matrix);
      matrix.removePrediction(b2, b1);
      matrix.addPrediction(b1, b1);
      ++count;
    }
    
    jassert(score1 >= refScore && refScore >= score2);
    return count ? (score1 - score2) / (double)count : 0.0;
  }

  virtual void computeRankingLoss(ExecutionContext& context, const std::vector<double>& scores, const std::vector<double>& costs, double* output, std::vector<double>* gradient) const
  {
    BinaryClassificationConfusionMatrix matrix;
    size_t n = scores.size();
    jassert(n && n == costs.size());
    for (size_t i = 0; i < n; ++i)
      matrix.addPrediction(scores[i] > 0, costs[i] == 0.0);

#ifdef JUCE_DEBUG
    BinaryClassificationConfusionMatrix dbg(matrix);
    //double refScore = computeScore(matrix);
    //double diff1 = computeScoreDifference(matrix, refScore, true);
    jassert(matrix == dbg);
    //double diff2 = computeScoreDifference(matrix, refScore, false);
    jassert(matrix == dbg);
    //context.informationCallback(T("F1-RankingLoss: d1=") + String(diff1) + T(" d2=") + String(diff2));
#endif // JUCE_DEBUG

    for (size_t i = 0; i < n; ++i)
    {  
      bool isPositive = (costs[i] == 0.0);
      double sign = (isPositive ? 1.0 : -1.0);
      double score = scores[i] * sign;
      double diff = 1.0;//juce::jmax(0.01, isPositive ? diff2 : diff1);
      jassert(diff >= 0.0);

      double baseOutput, baseDerivative;
      baseLoss->computeDiscriminativeLoss(score, output ? &baseOutput : NULL, gradient ? &baseDerivative : NULL);
      if (output)
        *output += baseOutput * diff;
      if (gradient)
        (*gradient)[i] += baseDerivative * diff * sign;
    }
    if (n)
      multiplyOutputAndGradient(output, gradient, 1.0 / (double)n);
  }
};

class F1ScoreRankingLossFunction : public BinaryClassificationRankingLossFunction
{
public:
  F1ScoreRankingLossFunction(DiscriminativeLossFunctionPtr baseLoss, const std::vector<double>& costs)
    : BinaryClassificationRankingLossFunction(baseLoss, costs) {}

  F1ScoreRankingLossFunction() {}

  virtual double computeScore(const BinaryClassificationConfusionMatrix& matrix) const
    {return matrix.computeF1Score();}
};

class MCCRankingLossFunction : public BinaryClassificationRankingLossFunction
{
public:
  MCCRankingLossFunction(DiscriminativeLossFunctionPtr baseLoss, const std::vector<double>& costs)
    : BinaryClassificationRankingLossFunction(baseLoss, costs) {}

  MCCRankingLossFunction() {}

  virtual double computeScore(const BinaryClassificationConfusionMatrix& matrix) const
    {return matrix.computeMatthewsCorrelation();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_BINARY_CLASSIFICATION_RANKING_H_
