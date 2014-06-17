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

class HoeffdingBoundIncrementalSplittingCriterion : public IncrementalSplittingCriterion
{
public:
  HoeffdingBoundIncrementalSplittingCriterion() : delta(0.0), threshold(0.0) {}
  HoeffdingBoundIncrementalSplittingCriterion(double delta, double threshold) : delta(delta), threshold(threshold) {}

  virtual Split findBestSplit(TreeNodePtr leaf) const
  {
    HoeffdingTreeIncrementalLearnerStatisticsPtr stats = leaf->getLearnerStatistics().staticCast<HoeffdingTreeIncrementalLearnerStatistics>();
    std::vector<Split> splits(stats->getEBSTs().size());
    for (size_t i = 0; i < splits.size(); ++i)
    {
      splits[i] = findBestSplit(i, stats->getEBSTs()[i], new ScalarVariableMeanAndVariance(), new PearsonCorrelationCoefficient(), new ScalarVariableMeanAndVariance(), new PearsonCorrelationCoefficient());
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
    double epsilon = hoeffdingBound(1, stats->getExamplesSeen(), delta);
    if ( bestSplit.quality != 0 && ( secondBestSplit.quality/bestSplit.quality < (1 - epsilon) || epsilon < threshold))
      return bestSplit;
    else
      return Split(0, DVector::missingValue, DVector::missingValue);
  }

  virtual double splitQuality(ScalarVariableMeanAndVariancePtr leftVariance, PearsonCorrelationCoefficientPtr leftCorrelation,
    ScalarVariableMeanAndVariancePtr rightVariance, PearsonCorrelationCoefficientPtr rightCorrelation) const = 0;

protected:
  friend class HoeffdingBoundIncrementalSplittingCriterionClass;

  double delta;
  double threshold;

  Split findBestSplit(size_t attribute, ExtendedBinarySearchTreePtr ebst, ScalarVariableMeanAndVariancePtr leftVariance, PearsonCorrelationCoefficientPtr leftCorrelation,
    ScalarVariableMeanAndVariancePtr rightVariance, PearsonCorrelationCoefficientPtr rightCorrelation) const
  {
    Split bestLeft, bestRight;
    ScalarVariableMeanAndVariancePtr totalRightVariance = new ScalarVariableMeanAndVariance();
    totalRightVariance->push(*rightVariance);
    totalRightVariance->push(*(ebst->getRightStats()));
    ScalarVariableMeanAndVariancePtr totalLeftVariance = new ScalarVariableMeanAndVariance();
    PearsonCorrelationCoefficientPtr totalRightCorrelation = new PearsonCorrelationCoefficient();
    totalRightCorrelation->push(*(rightCorrelation));
    totalRightCorrelation->push(*(ebst->getRightCorrelation()->getStats(attribute)));
    totalLeftVariance->push(*leftVariance);
    totalLeftVariance->push(*(ebst->getLeftStats()));
    PearsonCorrelationCoefficientPtr totalLeftCorrelation = new PearsonCorrelationCoefficient();
    totalLeftCorrelation->push(*(leftCorrelation));
    totalLeftCorrelation->push(*(ebst->getLeftCorrelation()->getStats(attribute)));
    
    if (ebst->getLeft().exists())
      bestLeft = findBestSplit(attribute, ebst->getLeft(), leftVariance, leftCorrelation, totalRightVariance, totalRightCorrelation);
    if (ebst->getRight().exists())
      bestRight = findBestSplit(attribute, ebst->getRight(), totalLeftVariance, totalLeftCorrelation, rightVariance, rightCorrelation);
    
    Split bestChild = bestLeft.quality >= bestRight.quality ? bestLeft : bestRight;
    Split hereSplit = Split(attribute, ebst->getValue(), splitQuality(totalLeftVariance, totalLeftCorrelation, totalRightVariance, totalRightCorrelation));

    return hereSplit.quality >= bestChild.quality ? hereSplit : bestChild;
  }

  inline double hoeffdingBound(size_t R, size_t N, double delta) const
    {return (N==0 || delta==0) ? 1 : sqrt(R*R*log2(1/delta) / 2 / N);}

  inline double log2(double n) const
    {return log(n)/log(2.0);}
};

class HoeffdingBoundStdDevReductionIncrementalSplittingCriterion : public HoeffdingBoundIncrementalSplittingCriterion
{
public:
  HoeffdingBoundStdDevReductionIncrementalSplittingCriterion() : HoeffdingBoundIncrementalSplittingCriterion() {}
  HoeffdingBoundStdDevReductionIncrementalSplittingCriterion(double delta, double threshold) : HoeffdingBoundIncrementalSplittingCriterion(delta, threshold) {}

  virtual double splitQuality(ScalarVariableMeanAndVariancePtr leftVariance, PearsonCorrelationCoefficientPtr leftCorrelation,
    ScalarVariableMeanAndVariancePtr rightVariance, PearsonCorrelationCoefficientPtr rightCorrelation) const
  {
    ScalarVariableMeanAndVariancePtr totalVariance = new ScalarVariableMeanAndVariance();
    totalVariance->push(*leftVariance);
    totalVariance->push(*rightVariance);
    return totalVariance->getStandardDeviation() - leftVariance->getCount() * leftVariance->getStandardDeviation() / totalVariance->getCount()
      - rightVariance->getCount() * rightVariance->getStandardDeviation() / totalVariance->getCount();
  }
};

class HoeffdingBoundMauveIncrementalSplittingCriterion : public HoeffdingBoundIncrementalSplittingCriterion
{
public:
  HoeffdingBoundMauveIncrementalSplittingCriterion() : HoeffdingBoundIncrementalSplittingCriterion() {}
  HoeffdingBoundMauveIncrementalSplittingCriterion(double delta, double threshold) : HoeffdingBoundIncrementalSplittingCriterion(delta, threshold) {}

  virtual double splitQuality(ScalarVariableMeanAndVariancePtr leftVariance, PearsonCorrelationCoefficientPtr leftCorrelation,
    ScalarVariableMeanAndVariancePtr rightVariance, PearsonCorrelationCoefficientPtr rightCorrelation) const
  {
    PearsonCorrelationCoefficientPtr totalCorrelation = new PearsonCorrelationCoefficient();
    totalCorrelation->push(*leftCorrelation);
    totalCorrelation->push(*rightCorrelation);
    double numLeft = leftVariance->getCount();
    double numRight = rightVariance->getCount();
    return totalCorrelation->getResidualStandardDeviation() - numLeft * leftCorrelation->getResidualStandardDeviation() / (numLeft + numRight)
      - numRight * rightCorrelation->getResidualStandardDeviation() / (numLeft + numRight);
  }
};
/*
class HoeffdingBoundStdDevReductionIncrementalSplittingCriterion2 : public IncrementalSplittingCriterion
{
public:
  HoeffdingBoundStdDevReductionIncrementalSplittingCriterion2() : delta(0.0), threshold(0.0) {}
  HoeffdingBoundStdDevReductionIncrementalSplittingCriterion2(double delta, double threshold) : delta(delta), threshold(threshold) {}

  virtual Split findBestSplit(TreeNodePtr leaf) const
  {
	HoeffdingTreeIncrementalLearnerStatisticsPtr stats = leaf->getLearnerStatistics().staticCast<HoeffdingTreeIncrementalLearnerStatistics>();
    std::vector<Split> splits(stats->getEBSTs().size(), Split(0, DVector::missingValue, DVector::missingValue));
    for (size_t i = 0; i < splits.size(); ++i)
	{
	  splits[i].attribute = i;
	  PearsonCorrelationCoefficientPtr left = new PearsonCorrelationCoefficient();
      PearsonCorrelationCoefficientPtr right = new PearsonCorrelationCoefficient();
	  int total;
	  initFindSplit(stats->getEBSTs()[i], i, left, right, total);
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
    double epsilon = hoeffdingBound(1, stats->getExamplesSeen(), delta);
    stats->getSplitRatios()->push(secondBestSplit.quality/bestSplit.quality);
    if ( bestSplit.quality != 0 && ( stats->getSplitRatios()->getMean() < (1 - epsilon) || epsilon < threshold))
      return bestSplit;
    else
      return Split(0, DVector::missingValue, DVector::missingValue);
  }

protected:
  friend class HoeffdingBoundStdDevReductionIncrementalSplittingCriterion2Class;

  double delta;
  double threshold;

private:
  void initFindSplit(ExtendedBinarySearchTreePtr ebst, size_t attribute, PearsonCorrelationCoefficientPtr totalLeft, PearsonCorrelationCoefficientPtr totalRight, int& total) const
  {
    PearsonCorrelationCoefficientPtr left = ebst->getLeftCorrelation()->getStats(attribute);
    PearsonCorrelationCoefficientPtr right = ebst->getRightCorrelation()->getStats(attribute);
    totalRight->update(left->numSamples + right->numSamples, left->sumY + right->sumY, left->sumYsquared + right->sumYsquared, 0, 0, 0);
    total = left->numSamples + right->numSamples;
  }

  void findBestSplit(size_t attribute, ExtendedBinarySearchTreePtr ebst, PearsonCorrelationCoefficientPtr totalLeft, PearsonCorrelationCoefficientPtr totalRight, int& total, Split& split) const
  {
    PearsonCorrelationCoefficientPtr left = ebst->leftCorrelation->getStats(attribute);
    PearsonCorrelationCoefficientPtr right = ebst->rightCorrelation->getStats(attribute);
    if(ebst->getLeft().exists())
      findBestSplit(attribute, ebst->getLeft(), totalLeft, totalRight, total, split);
    //update the sums and counts for computing the SDR of the split
    totalLeft->update(0, left->sumY, left->sumYsquared, 0, 0, 0);
    totalRight->update(-(int)left->numSamples, -left->sumY, -left->sumYsquared, 0, 0, 0);
    double sdParent = std(total, totalLeft->sumY+totalRight->sumY, totalLeft->sumYsquared+totalRight->sumYsquared);
    double sdLeftChild = std(total - totalRight->numSamples, totalLeft->sumY, totalLeft->sumYsquared);
    double sdRightChild = std(totalRight->numSamples, totalRight->sumY, totalRight->sumYsquared);
    double _sdr = sdr(sdParent, sdLeftChild, sdRightChild, total - totalRight->numSamples, totalRight->numSamples);
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

  inline double hoeffdingBound(size_t R, size_t N, double delta) const
    {return (N==0 || delta==0) ? 1 : sqrt(R*R*log2(1/delta) / 2 / N);}

  inline double log2(double n) const
    {return log(n)/log(2.0);}
};

class MauveIncrementalSplittingCriterion : public IncrementalSplittingCriterion
{
public:
  MauveIncrementalSplittingCriterion() : delta(0.0), threshold(0.0), maximumCoefficientOfDetermination(0.0) {}
  MauveIncrementalSplittingCriterion(double delta, double threshold, double maxCoefficientOfDetermination) : delta(delta), threshold(threshold),
  maximumCoefficientOfDetermination(maxCoefficientOfDetermination) {}

  virtual Split findBestSplit(TreeNodePtr leaf) const
  {
	  HoeffdingTreeIncrementalLearnerStatisticsPtr stats = leaf->getLearnerStatistics().staticCast<HoeffdingTreeIncrementalLearnerStatistics>();
    HoeffdingTreeNodePtr hoeffdingLeaf = leaf.staticCast<HoeffdingTreeNode>();
    //MultiVariateRegressionStatisticsPtr modelStats = hoeffdingLeaf->getModel()->getLearnerStatistics().staticCast<MultiVariateRegressionStatistics>();
    std::vector<Split> splits(stats->getEBSTs().size(), Split(0, DVector::missingValue, DVector::missingValue));
    for (size_t i = 0; i < splits.size(); ++i)
	  {
	    splits[i].attribute = i;
	    PearsonCorrelationCoefficientPtr left = new PearsonCorrelationCoefficient();
      PearsonCorrelationCoefficientPtr right = new PearsonCorrelationCoefficient();
	    int total;
	    initFindSplit(stats->getEBSTs()[i], i, left, right, total);
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
    double epsilon = hoeffdingBound(1, stats->getExamplesSeen(), delta);
    //double r2 = modelStats->getCoefficientOfDetermination();
    stats->getSplitRatios()->push(secondBestSplit.quality/bestSplit.quality);
    //if ( bestSplit.quality != 0 && secondBestSplit.quality != 0 && stats->getSplitRatios()->getMean() < (1 - epsilon) && r2 < maximumCoefficientOfDetermination)
    if ( bestSplit.quality != 0 && secondBestSplit.quality != 0 && stats->getSplitRatios()->getMean() < (1 - epsilon))
      return bestSplit;
    //else if(r2 < maximumCoefficientOfDetermination && epsilon < threshold)
    else if(epsilon < threshold)
      return bestSplit;
    else
      return Split(0, DVector::missingValue, DVector::missingValue);
  }

protected:
  friend class MauveIncrementalSplittingCriterionClass;

  // 1-delta expresses how certain the algorithm is 
  // that the best split is the best of the two best splits
  double delta;
  // when the hoeffdingbound is lower than this threshold,
  // a split is made (to allow splitting when best split is equally good as 2nd best split)
  double threshold;
  // when the function to approximate has a coefficient of determination below this value
  // in the region of interest, the splitting is halted. (stop condition)
  double maximumCoefficientOfDetermination;

private:
  void initFindSplit(ExtendedBinarySearchTreePtr ebst, size_t attribute, PearsonCorrelationCoefficientPtr totalLeft, PearsonCorrelationCoefficientPtr totalRight, int& total) const
  {
    PearsonCorrelationCoefficientPtr left = ebst->leftCorrelation->getStats(attribute);
    PearsonCorrelationCoefficientPtr right = ebst->rightCorrelation->getStats(attribute);
	  totalRight->update(left->numSamples + right->numSamples, left->sumY + right->sumY, left->sumYsquared + right->sumYsquared,
		left->sumX + right->sumX, left->sumXsquared + right->sumXsquared, left->sumXY + right->sumXY);
	  total = left->numSamples + right->numSamples;
  }
  
  void findBestSplit(size_t attribute, ExtendedBinarySearchTreePtr ebst, PearsonCorrelationCoefficientPtr totalLeft, PearsonCorrelationCoefficientPtr totalRight, int& total, Split& split) const
  {
    PearsonCorrelationCoefficientPtr left = ebst->leftCorrelation->getStats(attribute);
    PearsonCorrelationCoefficientPtr right = ebst->rightCorrelation->getStats(attribute);
    if(ebst->getLeft().exists())
      findBestSplit(attribute, ebst->getLeft(), totalLeft, totalRight, total, split);
    //update the sums and counts for computing the SDR of the split
    totalLeft->update(0, left->sumY, left->sumYsquared, left->sumX, left->sumXsquared, left->sumXY);
    totalRight->update(-(int)left->numSamples, -left->sumY, -left->sumYsquared, -left->sumX, -left->sumXsquared, -left->sumXY);
    double sdParent = rstd(total, totalLeft->sumY+totalRight->sumY, totalLeft->sumYsquared+totalRight->sumYsquared, 
	    totalLeft->sumX + totalRight->sumX, totalLeft->sumXsquared + totalRight->sumXsquared, totalLeft->sumXY + totalRight->sumXY);
    double sdLeftChild = rstd(total - totalRight->numSamples, totalLeft->sumY, totalLeft->sumYsquared, totalLeft->sumX, totalLeft->sumXsquared, totalLeft->sumXY);
    double sdRightChild = rstd(totalRight->numSamples, totalRight->sumY, totalRight->sumYsquared, totalRight->sumX, totalRight->sumXsquared, totalRight->sumXY);
    double _sdr = sdr(sdParent, sdLeftChild, sdRightChild, total - totalRight->numSamples, totalRight->numSamples);
    double percNbSamplesLeftSplit = (total - totalRight->numSamples) / (double)total;
    double percNbSamplesRightSplit = totalRight->numSamples / (double)total;
    double trimPercentage = 0.05;
    if(percNbSamplesLeftSplit > trimPercentage && percNbSamplesRightSplit > trimPercentage && split.quality < _sdr)
    {
      split.quality = _sdr;
      split.value = ebst->getValue();
      //split.leftThresholdWeight = getNormalizedThresholdWeight(total - totalRight->numSamples, totalLeft->sumY, totalLeft->sumYsquared, totalLeft->sumX, totalLeft->sumXsquared, totalLeft->sumXY);
      //split.rightThresholdWeight = getNormalizedThresholdWeight(totalRight->numSamples, totalRight->sumY, totalRight->sumYsquared, totalRight->sumX, totalRight->sumXsquared, totalRight->sumXY);
      //split.leftAttributeWeight = getNormalizedAttributeWeight(total - totalRight->numSamples, totalLeft->sumY, totalLeft->sumYsquared, totalLeft->sumX, totalLeft->sumXsquared, totalLeft->sumXY);
      //split.rightAttributeWeight = getNormalizedAttributeWeight(totalRight->numSamples, totalRight->sumY, totalRight->sumYsquared, totalRight->sumX, totalRight->sumXsquared, totalRight->sumXY);
      split.rstd = std(total, totalLeft->sumY+totalRight->sumY, totalLeft->sumYsquared+totalRight->sumYsquared);
      PearsonCorrelationCoefficientPtr pCorrelation = new PearsonCorrelationCoefficient();
      pCorrelation->update(total, totalLeft->sumY+totalRight->sumY, totalLeft->sumYsquared+totalRight->sumYsquared, 
	    totalLeft->sumX + totalRight->sumX, totalLeft->sumXsquared + totalRight->sumXsquared, totalLeft->sumXY + totalRight->sumXY);
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

  // incremental standard deviation
  inline double std(size_t numSamples, double sumY, double sumYsquared) const
    {return sqrt(sumYsquared/numSamples-(sumY/numSamples)*(sumY/numSamples));}

  // incremental standard reduction
  inline double sdr(double stdParent, double stdLeftChild, double stdRightChild, size_t numSamplesLeft, size_t numSamplesRight) const
    {return stdParent - (stdLeftChild * numSamplesLeft + stdRightChild * numSamplesRight) / (numSamplesLeft+numSamplesRight);}

  inline double hoeffdingBound(size_t R, size_t N, double delta) const
    {return (N==0 || delta==0) ? 1 : sqrt(R*R*log2(1/delta) / 2 / N);}

  inline double log2(double n) const
    {return log(n)/log(2.0);}
};

class QuandtAndrewsIncrementalSplittingCriterion : public IncrementalSplittingCriterion
{
public:
  QuandtAndrewsIncrementalSplittingCriterion() : numVariables(0), threshold(0.0) {}
  QuandtAndrewsIncrementalSplittingCriterion(size_t numVariables, double threshold) : numVariables(numVariables), threshold(threshold) {}

  virtual Split findBestSplit(TreeNodePtr leaf) const
  {
	HoeffdingTreeIncrementalLearnerStatisticsPtr stats = leaf->getLearnerStatistics().staticCast<HoeffdingTreeIncrementalLearnerStatistics>();
    std::vector<Split> splits(stats->getEBSTs().size(), Split(0, DVector::missingValue, DVector::missingValue));
    for (size_t i = 0; i < splits.size(); ++i)
    {
      splits[i].attribute = i;
      PearsonCorrelationCoefficientPtr left = new PearsonCorrelationCoefficient();
      PearsonCorrelationCoefficientPtr right = new PearsonCorrelationCoefficient();
      int total;
      initFindSplit(stats->getEBSTs()[i], i, left, right, total);
      findBestSplit(i, stats->getEBSTs()[i], left, right, total, splits[i]);
    }
    Split bestSplit;
    for (size_t i = 0; i < splits.size(); ++i)
    {
      if (splits[i].quality >= bestSplit.quality)
        bestSplit = splits[i];
    }
    // stopping rule
    double delta = stoppingRuleDelta(bestSplit.rssCombined, bestSplit.rssLeft, bestSplit.rssRight, bestSplit.N, numVariables);
    if(delta > threshold)
      return bestSplit;
    else
      return Split(0, DVector::missingValue, DVector::missingValue);
  }

protected:
  friend class QuandtAndrewsIncrementalSplittingCriterionClass;

  static double criticalValues[104][10];
  size_t numVariables;
  double threshold;

private:
  void initFindSplit(ExtendedBinarySearchTreePtr ebst, size_t attribute, PearsonCorrelationCoefficientPtr totalLeft, PearsonCorrelationCoefficientPtr totalRight, int& total) const
  {
	PearsonCorrelationCoefficientPtr left = ebst->leftCorrelation->getStats(attribute);
	PearsonCorrelationCoefficientPtr right = ebst->rightCorrelation->getStats(attribute);
	totalRight->update(left->numSamples + right->numSamples, left->sumY + right->sumY, left->sumYsquared + right->sumYsquared,
		left->sumX + right->sumX, left->sumXsquared + right->sumXsquared, left->sumXY + right->sumXY);
	total = left->numSamples + right->numSamples;
  }

  void findBestSplit(size_t attribute, ExtendedBinarySearchTreePtr ebst, PearsonCorrelationCoefficientPtr totalLeft, PearsonCorrelationCoefficientPtr totalRight, int& total, Split& split) const
  {
    PearsonCorrelationCoefficientPtr left = ebst->leftCorrelation->getStats(attribute);
    PearsonCorrelationCoefficientPtr right = ebst->rightCorrelation->getStats(attribute);
	if(ebst->getLeft().exists())
	  findBestSplit(attribute, ebst->getLeft(), totalLeft, totalRight, total, split);
	//update the sums and counts for computing the SDR of the split
	totalLeft->update(0, left->sumY, left->sumYsquared, left->sumX, left->sumXsquared, left->sumXY);
	totalRight->update(-(int)left->numSamples, -left->sumY, -left->sumYsquared, -left->sumX, -left->sumXsquared, -left->sumXY);
  double rssParent = rss(total, totalLeft->sumY+totalRight->sumY, totalLeft->sumYsquared+totalRight->sumYsquared, 
		totalLeft->sumX + totalRight->sumX, totalLeft->sumXsquared + totalRight->sumXsquared, totalLeft->sumXY + totalRight->sumXY);
	double rssLeftChild = rss(total - totalRight->numSamples, totalLeft->sumY, totalLeft->sumYsquared, totalLeft->sumX, totalLeft->sumXsquared, totalLeft->sumXY);
	double rssRightChild = rss(totalRight->numSamples, totalRight->sumY, totalRight->sumYsquared, totalRight->sumX, totalRight->sumXsquared, totalRight->sumXY);
	double f = fStatistic(rssParent, rssLeftChild, rssRightChild, total, numVariables);
	if(f > getCriticalValue(numVariables, total) && split.quality < f)
	{
		split.quality = f;
		split.value = ebst->getValue();
    split.rssCombined = rssParent;
    split.rssLeft = rssLeftChild;
    split.rssRight = rssRightChild;
    split.N = total;
	}
	if(ebst->getRight().exists())
	  findBestSplit(attribute, ebst->getRight(), totalLeft, totalRight, total, split);
	//update the sums and counts for returning to the parent node
	totalLeft->update(0, -left->sumY, -left->sumYsquared, -left->sumX, -left->sumXsquared, -left->sumXY);
	totalRight->update(left->numSamples, left->sumY, left->sumYsquared, left->sumX, left->sumXsquared, left->sumXY);
  }

  inline double getCriticalValue(size_t dimensionality, size_t numSamples) const
  {
	size_t row = numSamples - 2 * dimensionality;
	if(row <= 0)
	  return 1000000;
	else if(row <= 100)
	  return criticalValues[row-1][dimensionality-1];
    else if(row <= 150)
	  return criticalValues[99][dimensionality-1];
	else if(row <= 350)
	  return criticalValues[100][dimensionality-1];
	else if(row <= 750)
	  return criticalValues[101][dimensionality-1];
	else if(row <= 1250)
	  return criticalValues[102][dimensionality-1];
	else
	  return criticalValues[103][dimensionality-1];
  }

  inline double stoppingRuleDelta(double rssCombined, double rssLeft, double rssRight, size_t N, size_t d) const
  {
    return rssCombined/(N-d) - (rssLeft+rssRight)/(N-2*d);
  }

  // F-statistic for chow test
  inline double fStatistic(double rssCombined, double rssLeft, double rssRight, size_t N, size_t d) const
  {
	double div = (rssLeft+rssRight)*d;
	return div == 0 ? 0 : (rssCombined-rssLeft-rssRight)*(N-2*d)/div;
  }

  // residual sum of squares
  inline double rss(size_t numSamples, double sumY, double sumYsquared, double sumX, double sumXsquared, double sumXY) const
  {
	double _rstd = rstd(numSamples, sumY, sumYsquared, sumX, sumXsquared, sumXY);
    return _rstd*_rstd*numSamples;
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
};

// critical values for the 0.01 significance level
double QuandtAndrewsIncrementalSplittingCriterion::criticalValues[104][10] =
  {
	{4052.19,4999.52,5403.34,5624.62,5763.65,5858.97,5928.33,5981.10,6022.50,6055.85},
	{98.502,99.000,99.166,99.249,99.300,99.333,99.356,99.374,99.388,99.399},
	{34.116,30.816,29.457,28.710,28.237,27.911,27.672,27.489,27.345,27.229},
	{21.198,18.000,16.694,15.977,15.522,15.207,14.976,14.799,14.659,14.546},
	{16.258,13.274,12.060,11.392,10.967,10.672,10.456,10.289,10.158,10.051},
	{13.745,10.925,9.780,9.148,8.746,8.466,8.260,8.102,7.976,7.874},
	{12.246,9.547,8.451,7.847,7.460,7.191,6.993,6.840,6.719,6.620},
	{11.259,8.649,7.591,7.006,6.632,6.371,6.178,6.029,5.911,5.814},
	{10.561,8.022,6.992,6.422,6.057,5.802,5.613,5.467,5.351,5.257},
	{10.044,7.559,6.552,5.994,5.636,5.386,5.200,5.057,4.942,4.849},
	{9.646,7.206,6.217,5.668,5.316,5.069,4.886,4.744,4.632,4.539},
	{9.330,6.927,5.953,5.412,5.064,4.821,4.640,4.499,4.388,4.296},
	{9.074,6.701,5.739,5.205,4.862,4.620,4.441,4.302,4.191,4.100},
	{8.862,6.515,5.564,5.035,4.695,4.456,4.278,4.140,4.030,3.939},
	{8.683,6.359,5.417,4.893,4.556,4.318,4.142,4.004,3.895,3.805},
	{8.531,6.226,5.292,4.773,4.437,4.202,4.026,3.890,3.780,3.691},
	{8.400,6.112,5.185,4.669,4.336,4.102,3.927,3.791,3.682,3.593},
	{8.285,6.013,5.092,4.579,4.248,4.015,3.841,3.705,3.597,3.508},
	{8.185,5.926,5.010,4.500,4.171,3.939,3.765,3.631,3.523,3.434},
	{8.096,5.849,4.938,4.431,4.103,3.871,3.699,3.564,3.457,3.368},
	{8.017,5.780,4.874,4.369,4.042,3.812,3.640,3.506,3.398,3.310},
	{7.945,5.719,4.817,4.313,3.988,3.758,3.587,3.453,3.346,3.258},
	{7.881,5.664,4.765,4.264,3.939,3.710,3.539,3.406,3.299,3.211},
	{7.823,5.614,4.718,4.218,3.895,3.667,3.496,3.363,3.256,3.168},
	{7.770,5.568,4.675,4.177,3.855,3.627,3.457,3.324,3.217,3.129},
	{7.721,5.526,4.637,4.140,3.818,3.591,3.421,3.288,3.182,3.094},
	{7.677,5.488,4.601,4.106,3.785,3.558,3.388,3.256,3.149,3.062},
	{7.636,5.453,4.568,4.074,3.754,3.528,3.358,3.226,3.120,3.032},
	{7.598,5.420,4.538,4.045,3.725,3.499,3.330,3.198,3.092,3.005},
	{7.562,5.390,4.510,4.018,3.699,3.473,3.305,3.173,3.067,2.979},
	{7.530,5.362,4.484,3.993,3.675,3.449,3.281,3.149,3.043,2.955},
	{7.499,5.336,4.459,3.969,3.652,3.427,3.258,3.127,3.021,2.934},
	{7.471,5.312,4.437,3.948,3.630,3.406,3.238,3.106,3.000,2.913},
	{7.444,5.289,4.416,3.927,3.611,3.386,3.218,3.087,2.981,2.894},
	{7.419,5.268,4.396,3.908,3.592,3.368,3.200,3.069,2.963,2.876},
	{7.396,5.248,4.377,3.890,3.574,3.351,3.183,3.052,2.946,2.859},
	{7.373,5.229,4.360,3.873,3.558,3.334,3.167,3.036,2.930,2.843},
	{7.353,5.211,4.343,3.858,3.542,3.319,3.152,3.021,2.915,2.828},
	{7.333,5.194,4.327,3.843,3.528,3.305,3.137,3.006,2.901,2.814},
	{7.314,5.179,4.313,3.828,3.514,3.291,3.124,2.993,2.888,2.801},
	{7.296,5.163,4.299,3.815,3.501,3.278,3.111,2.980,2.875,2.788},
	{7.280,5.149,4.285,3.802,3.488,3.266,3.099,2.968,2.863,2.776},
	{7.264,5.136,4.273,3.790,3.476,3.254,3.087,2.957,2.851,2.764},
	{7.248,5.123,4.261,3.778,3.465,3.243,3.076,2.946,2.840,2.754},
	{7.234,5.110,4.249,3.767,3.454,3.232,3.066,2.935,2.830,2.743},
	{7.220,5.099,4.238,3.757,3.444,3.222,3.056,2.925,2.820,2.733},
	{7.207,5.087,4.228,3.747,3.434,3.213,3.046,2.916,2.811,2.724},
	{7.194,5.077,4.218,3.737,3.425,3.204,3.037,2.907,2.802,2.715},
	{7.182,5.066,4.208,3.728,3.416,3.195,3.028,2.898,2.793,2.706},
	{7.171,5.057,4.199,3.720,3.408,3.186,3.020,2.890,2.785,2.698},
	{7.159,5.047,4.191,3.711,3.400,3.178,3.012,2.882,2.777,2.690},
	{7.149,5.038,4.182,3.703,3.392,3.171,3.005,2.874,2.769,2.683},
	{7.139,5.030,4.174,3.695,3.384,3.163,2.997,2.867,2.762,2.675},
	{7.129,5.021,4.167,3.688,3.377,3.156,2.990,2.860,2.755,2.668},
	{7.119,5.013,4.159,3.681,3.370,3.149,2.983,2.853,2.748,2.662},
	{7.110,5.006,4.152,3.674,3.363,3.143,2.977,2.847,2.742,2.655},
	{7.102,4.998,4.145,3.667,3.357,3.136,2.971,2.841,2.736,2.649},
	{7.093,4.991,4.138,3.661,3.351,3.130,2.965,2.835,2.730,2.643},
	{7.085,4.984,4.132,3.655,3.345,3.124,2.959,2.829,2.724,2.637},
	{7.077,4.977,4.126,3.649,3.339,3.119,2.953,2.823,2.718,2.632},
	{7.070,4.971,4.120,3.643,3.333,3.113,2.948,2.818,2.713,2.626},
	{7.062,4.965,4.114,3.638,3.328,3.108,2.942,2.813,2.708,2.621},
	{7.055,4.959,4.109,3.632,3.323,3.103,2.937,2.808,2.703,2.616},
	{7.048,4.953,4.103,3.627,3.318,3.098,2.932,2.803,2.698,2.611},
	{7.042,4.947,4.098,3.622,3.313,3.093,2.928,2.798,2.693,2.607},
	{7.035,4.942,4.093,3.618,3.308,3.088,2.923,2.793,2.689,2.602},
	{7.029,4.937,4.088,3.613,3.304,3.084,2.919,2.789,2.684,2.598},
	{7.023,4.932,4.083,3.608,3.299,3.080,2.914,2.785,2.680,2.593},
	{7.017,4.927,4.079,3.604,3.295,3.075,2.910,2.781,2.676,2.589},
	{7.011,4.922,4.074,3.600,3.291,3.071,2.906,2.777,2.672,2.585},
	{7.006,4.917,4.070,3.596,3.287,3.067,2.902,2.773,2.668,2.581},
	{7.001,4.913,4.066,3.591,3.283,3.063,2.898,2.769,2.664,2.578},
	{6.995,4.908,4.062,3.588,3.279,3.060,2.895,2.765,2.660,2.574},
	{6.990,4.904,4.058,3.584,3.275,3.056,2.891,2.762,2.657,2.570},
	{6.985,4.900,4.054,3.580,3.272,3.052,2.887,2.758,2.653,2.567},
	{6.981,4.896,4.050,3.577,3.268,3.049,2.884,2.755,2.650,2.563},
	{6.976,4.892,4.047,3.573,3.265,3.046,2.881,2.751,2.647,2.560},
	{6.971,4.888,4.043,3.570,3.261,3.042,2.877,2.748,2.644,2.557},
	{6.967,4.884,4.040,3.566,3.258,3.039,2.874,2.745,2.640,2.554},
	{6.963,4.881,4.036,3.563,3.255,3.036,2.871,2.742,2.637,2.551},
	{6.958,4.877,4.033,3.560,3.252,3.033,2.868,2.739,2.634,2.548},
	{6.954,4.874,4.030,3.557,3.249,3.030,2.865,2.736,2.632,2.545},
	{6.950,4.870,4.027,3.554,3.246,3.027,2.863,2.733,2.629,2.542},
	{6.947,4.867,4.024,3.551,3.243,3.025,2.860,2.731,2.626,2.539},
	{6.943,4.864,4.021,3.548,3.240,3.022,2.857,2.728,2.623,2.537},
	{6.939,4.861,4.018,3.545,3.238,3.019,2.854,2.725,2.621,2.534},
	{6.935,4.858,4.015,3.543,3.235,3.017,2.852,2.723,2.618,2.532},
	{6.932,4.855,4.012,3.540,3.233,3.014,2.849,2.720,2.616,2.529},
	{6.928,4.852,4.010,3.538,3.230,3.012,2.847,2.718,2.613,2.527},
	{6.925,4.849,4.007,3.535,3.228,3.009,2.845,2.715,2.611,2.524},
	{6.922,4.846,4.004,3.533,3.225,3.007,2.842,2.713,2.609,2.522},
	{6.919,4.844,4.002,3.530,3.223,3.004,2.840,2.711,2.606,2.520},
	{6.915,4.841,3.999,3.528,3.221,3.002,2.838,2.709,2.604,2.518},
	{6.912,4.838,3.997,3.525,3.218,3.000,2.835,2.706,2.602,2.515},
	{6.909,4.836,3.995,3.523,3.216,2.998,2.833,2.704,2.600,2.513},
	{6.906,4.833,3.992,3.521,3.214,2.996,2.831,2.702,2.598,2.511},
	{6.904,4.831,3.990,3.519,3.212,2.994,2.829,2.700,2.596,2.509},
	{6.901,4.829,3.988,3.517,3.210,2.992,2.827,2.698,2.594,2.507},
	{6.898,4.826,3.986,3.515,3.208,2.990,2.825,2.696,2.592,2.505},
	{6.895,4.824,3.984,3.513,3.206,2.988,2.823,2.694,2.590,2.503},
	{6.76,4.71,3.88,3.41,3.11,2.89,2.73,2.60,2.50,2.41},
	{6.69,4.65,3.82,3.36,3.05,2.84,2.68,2.55,2.44,2.36},
	{6.66,4.63,3.80,3.34,3.04,2.82,2.66,2.53,2.43,2.34},
	{1.04,4.61,3.78,3.32,3.02,2.80,2.64,2.51,2.41,2.32}
  };
  */
/** Splitting criterion that always returns no split found (i.e. no splitting)
 */
class NullIncrementalSplittingCriterion : public IncrementalSplittingCriterion
{
public:
  NullIncrementalSplittingCriterion() {}

  virtual Split findBestSplit(TreeNodePtr leaf) const
    {return Split(0, DVector::missingValue, DVector::missingValue);}
};

} /* namespace lbcpp */

#endif //!ML_INCREMENTAL_SPLITTING_CRITERION_H_
