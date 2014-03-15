/*------------------------------------------.---------------------------------.
| Filename: IncrementalSplittingCriterion.h | Classes implementing splitting  |
| Author  : Denny Verbeeck                  | criteria for incremental tree   |
| Started : 14/03/2014 10:48                | learners                        |
`-------------------------------------------/                                 |
                               |                                              |
                               `---------------------------------------------*/

#ifndef ML_INCREMENTAL_SPLITTING_CRITERION_H_
# define ML_INCREMENTAL_SPLITTING_CRITERION_H_

# include <ml/predeclarations.h>
# include <ml/Expression.h>
# include <ml/BinarySearchTree.h>
# include <ml/IncrementalLearner.h>
# include "HoeffdingTreeIncrementalLearner.h"

namespace lbcpp
{

class HoeffdingBoundStdDevReductionIncrementalSplittingCriterion : public IncrementalSplittingCriterion
{
public:
  HoeffdingBoundStdDevReductionIncrementalSplittingCriterion() : delta(0.0), threshold(0.0) {}
  HoeffdingBoundStdDevReductionIncrementalSplittingCriterion(double delta, double threshold) : delta(delta), threshold(threshold) {}

  virtual Split findBestSplit(TreeNodePtr leaf) const
  {
    HoeffdingTreeNodeStatisticsPtr stats = leaf->getLearnerStatistics().staticCast<HoeffdingTreeNodeStatistics>();
    std::vector<Split> splits(stats->getEBSTs().size());
    for (size_t i = 0; i < splits.size(); ++i)
      splits[i] = findBestSplit(i, stats->getEBSTs()[i]);
    Split bestSplit, secondBestSplit;
    for (size_t i = 0; i < splits.size(); ++i)
    {
      if (splits[i].quality > bestSplit.quality)
      {
        secondBestSplit = bestSplit;
        bestSplit = splits[i];
      }
      else if (splits[i].quality > secondBestSplit.quality)
        secondBestSplit = splits[i];
    }
    double epsilon = hoeffdingBound(1, stats->getNumExamplesSeen(), delta);
    if ( bestSplit.quality != 0 && ( secondBestSplit.quality/bestSplit.quality < (1 - epsilon) || epsilon < threshold))
      return bestSplit;
    else
      return Split(0, DVector::missingValue, DVector::missingValue);
  }

protected:
  friend class HoeffdingBoundStdDevReductionIncrementalSplittingCriterionClass;

  double delta;
  double threshold;

private:
  Split findBestSplit(size_t attribute, ExtendedBinarySearchTreePtr ebst) const
  {
    Split bestLeft, bestRight;
    if (ebst->getLeft().exists())
      bestLeft = findBestSplit(attribute, ebst->getLeft().staticCast<ExtendedBinarySearchTree>());
    if (ebst->getRight().exists())
      bestRight = findBestSplit(attribute, ebst->getRight().staticCast<ExtendedBinarySearchTree>());
    ScalarVariableMeanAndVariancePtr leftStats = ebst->getLeftStats();
    ScalarVariableMeanAndVariancePtr rightStats = ebst->getRightStats();
    double totalCount = leftStats->getCount() + rightStats->getCount();
    double totalStdDev = sqrt((leftStats->getSumOfSquares() + rightStats->getSumOfSquares()) / totalCount - 
      ((leftStats->getSum() + rightStats->getSum()) / totalCount) * ((leftStats->getSum() + rightStats->getSum()) / totalCount));
    double sdr = totalStdDev - 
                 leftStats->getStandardDeviation() * leftStats->getCount() / totalCount -
                 rightStats->getStandardDeviation() * rightStats->getCount() / totalCount;
    Split bestChild = bestLeft.quality >= bestRight.quality ? bestLeft : bestRight;
    Split hereSplit = Split(attribute, ebst->getValue(), sdr);
    return hereSplit.quality >= bestChild.quality ? hereSplit : bestChild;
  }

  inline double log2(double n) const
    {return log(n) / log(2.0);}

  inline double hoeffdingBound(size_t R, size_t N, double delta) const
    {return (N==0 || delta==0) ? 1 : sqrt(R*R*log2(1/delta) / 2 / N);}
};

} /* namespace lbcpp */

#endif //!ML_INCREMENTAL_SPLITTING_CRITERION_H_
