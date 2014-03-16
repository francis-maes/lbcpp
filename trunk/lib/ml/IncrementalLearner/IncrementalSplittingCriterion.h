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
    // TODO: still incorrect, need to add right side of parent nodes as well
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

class HoeffdingBoundStdDevReductionIncrementalSplittingCriterion2 : public IncrementalSplittingCriterion
{
public:
  HoeffdingBoundStdDevReductionIncrementalSplittingCriterion2() : delta(0.0), threshold(0.0) {}
  HoeffdingBoundStdDevReductionIncrementalSplittingCriterion2(double delta, double threshold) : delta(delta), threshold(threshold) {}

  virtual Split findBestSplit(TreeNodePtr leaf) const
  {
	HoeffdingTreeNodeStatisticsPtr stats = leaf->getLearnerStatistics().staticCast<HoeffdingTreeNodeStatistics>();
    std::vector<Split> splits(stats->getEBSTs().size(), Split());
    for (size_t i = 0; i < splits.size(); ++i)
	{
	  splits[i].attribute = i;
	  PearsonCorrelationCoefficientPtr left = new PearsonCorrelationCoefficient();
      PearsonCorrelationCoefficientPtr right = new PearsonCorrelationCoefficient();
	  int total;
	  initFindSplit(stats->getEBSTs()[i], left, right, total);
      findBestSplit(i, stats->getEBSTs()[i], left, right, total, splits[i]);
	}
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
  friend class HoeffdingBoundStdDevReductionIncrementalSplittingCriterion2Class;

  double delta;
  double threshold;

private:
  void initFindSplit(ExtendedBinarySearchTreePtr ebst, PearsonCorrelationCoefficientPtr totalLeft, PearsonCorrelationCoefficientPtr totalRight, int& total) const
  {
	PearsonCorrelationCoefficientPtr left = ebst->leftCorrelation;
	PearsonCorrelationCoefficientPtr right = ebst->rightCorrelation;
	totalRight->update(left->numSamples + right->numSamples, left->sumY + right->sumY, left->sumYsquared + right->sumYsquared, 0, 0, 0);
	total = left->numSamples + right->numSamples;
  }

  void findBestSplit(size_t attribute, ExtendedBinarySearchTreePtr ebst, PearsonCorrelationCoefficientPtr totalLeft, PearsonCorrelationCoefficientPtr totalRight, int& total, Split& split) const
  {
	PearsonCorrelationCoefficientPtr left = ebst->leftCorrelation;
	PearsonCorrelationCoefficientPtr right = ebst->rightCorrelation;
	if(ebst->getLeft().exists())
	  findBestSplit(attribute, ebst->getLeft(), totalLeft, totalRight, total, split);
	//update the sums and counts for computing the SDR of the split
	totalLeft->update(0, left->sumY, left->sumYsquared, 0, 0, 0);
	totalRight->update(-(int)left->numSamples, -left->sumY, -left->sumYsquared, 0, 0, 0);
	double sdParent = std(total, left->sumY+right->sumY, left->sumYsquared+right->sumYsquared);
	double sdLeftChild = std(total - right->numSamples, left->sumY, left->sumYsquared);
	double sdRightChild = std(right->numSamples, right->sumY, right->sumYsquared);
	double _sdr = sdr(sdParent, sdLeftChild, sdRightChild, total - right->numSamples, right->numSamples);
	if(split.quality < _sdr)
	{
	  split.quality = _sdr;
	  split.value = ebst->getValue();
	}
	if(ebst->getRight().exists())
	  findBestSplit(attribute, ebst->getRight(), totalLeft, totalRight, total, split);
	//update the sums and counts for returning to the parent node
	totalLeft->update(0, -left->sumY, -left->sumYsquared, 0, 0, 0);
	totalRight->update(left->numSamples, left->sumY, left->sumYsquared, 0, 0, 0);
  }

  // incremental standard deviation
  inline double std(size_t numSamples, double sumY, double sumYsquared) const
    {return sqrt(sumYsquared/numSamples-(sumY/numSamples)*(sumY/numSamples));}

  // incremental standard reduction
  inline double sdr(double stdParent, double stdLeftChild, double stdRightChild, size_t numSamplesLeft, size_t numSamplesRight) const
    {return stdParent - (stdLeftChild * numSamplesLeft + stdRightChild * numSamplesRight) / (numSamplesLeft+numSamplesRight);}

  inline double log2(double n) const
    {return log(n) / log(2.0);}

  inline double hoeffdingBound(size_t R, size_t N, double delta) const
    {return (N==0 || delta==0) ? 1 : sqrt(R*R*log2(1/delta) / 2 / N);}
};

class MauveIncrementalSplittingCriterion : public IncrementalSplittingCriterion
{
public:
  MauveIncrementalSplittingCriterion() : delta(0.0), threshold(0.0) {}
  MauveIncrementalSplittingCriterion(double delta, double threshold) : delta(delta), threshold(threshold) {}

  virtual Split findBestSplit(TreeNodePtr leaf) const
  {
	HoeffdingTreeNodeStatisticsPtr stats = leaf->getLearnerStatistics().staticCast<HoeffdingTreeNodeStatistics>();
    std::vector<Split> splits(stats->getEBSTs().size(), Split());
    for (size_t i = 0; i < splits.size(); ++i)
	{
	  splits[i].attribute = i;
	  PearsonCorrelationCoefficientPtr left = new PearsonCorrelationCoefficient();
      PearsonCorrelationCoefficientPtr right = new PearsonCorrelationCoefficient();
	  int total;
	  initFindSplit(stats->getEBSTs()[i], left, right, total);
      findBestSplit(i, stats->getEBSTs()[i], left, right, total, splits[i]);
	}
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
  friend class MauveIncrementalSplittingCriterionClass;

  double delta;
  double threshold;

private:
  void initFindSplit(ExtendedBinarySearchTreePtr ebst, PearsonCorrelationCoefficientPtr totalLeft, PearsonCorrelationCoefficientPtr totalRight, int& total) const
  {
	PearsonCorrelationCoefficientPtr left = ebst->leftCorrelation;
	PearsonCorrelationCoefficientPtr right = ebst->rightCorrelation;
	totalRight->update(left->numSamples + right->numSamples, left->sumY + right->sumY, left->sumYsquared + right->sumYsquared,
		left->sumX + right->sumX, left->sumXsquared + right->sumXsquared, left->sumXY + right->sumXY);
	total = left->numSamples + right->numSamples;
  }

  void findBestSplit(size_t attribute, ExtendedBinarySearchTreePtr ebst, PearsonCorrelationCoefficientPtr totalLeft, PearsonCorrelationCoefficientPtr totalRight, int& total, Split& split) const
  {
	PearsonCorrelationCoefficientPtr left = ebst->leftCorrelation;
	PearsonCorrelationCoefficientPtr right = ebst->rightCorrelation;
	if(ebst->getLeft().exists())
	  findBestSplit(attribute, ebst->getLeft(), totalLeft, totalRight, total, split);
	//update the sums and counts for computing the SDR of the split
	totalLeft->update(0, left->sumY, left->sumYsquared, left->sumX, left->sumXsquared, left->sumXY);
	totalRight->update(-(int)left->numSamples, -left->sumY, -left->sumYsquared, -left->sumX, -left->sumXsquared, -left->sumXY);
	double sdParent = rstd(total, left->sumY+right->sumY, left->sumYsquared+right->sumYsquared, 
		left->sumX + right->sumX, left->sumXsquared + right->sumXsquared, left->sumXY + right->sumXY);
	double sdLeftChild = rstd(total - right->numSamples, left->sumY, left->sumYsquared, left->sumX, left->sumXsquared, left->sumXY);
	double sdRightChild = rstd(right->numSamples, right->sumY, right->sumYsquared, right->sumX, right->sumXsquared, right->sumXY);
	double _sdr = sdr(sdParent, sdLeftChild, sdRightChild, total - right->numSamples, right->numSamples);
	if(split.quality < _sdr)
	{
	  split.quality = _sdr;
	  split.value = ebst->getValue();
	}
	if(ebst->getRight().exists())
	  findBestSplit(attribute, ebst->getRight(), totalLeft, totalRight, total, split);
	//update the sums and counts for returning to the parent node
	totalLeft->update(0, -left->sumY, -left->sumYsquared, -left->sumX, -left->sumXsquared, -left->sumXY);
	totalRight->update(left->numSamples, left->sumY, left->sumYsquared, left->sumX, left->sumXsquared, left->sumXY);
  }

  // incremental residual standard deviation
  inline double rstd(size_t numSamples, double sumY, double sumYsquared, double sumX, double sumXsquared, double sumXY) const
	{
	  double div = numSamples*sumXsquared-sumX*sumX;
	  double b = div==0?0:(numSamples*sumXY-sumX*sumY)/div;
	  double a = numSamples==0?0:(sumY-b*sumX)/numSamples;
	  return sqrt(1.0/numSamples*(sumYsquared-2*a*sumY-2*b*sumXY+numSamples*a*a+2*a*b*sumX+b*b*sumXsquared));
	}

  // incremental standard reduction
  inline double sdr(double stdParent, double stdLeftChild, double stdRightChild, size_t numSamplesLeft, size_t numSamplesRight) const
    {return stdParent - (stdLeftChild * numSamplesLeft + stdRightChild * numSamplesRight) / (numSamplesLeft+numSamplesRight);}

  inline double log2(double n) const
    {return log(n) / log(2.0);}

  inline double hoeffdingBound(size_t R, size_t N, double delta) const
    {return (N==0 || delta==0) ? 1 : sqrt(R*R*log2(1/delta) / 2 / N);}
};

} /* namespace lbcpp */

#endif //!ML_INCREMENTAL_SPLITTING_CRITERION_H_
