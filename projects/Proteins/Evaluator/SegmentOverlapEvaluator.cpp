/*-----------------------------------------.---------------------------------.
| Filename: SegmentOverlapEvaluator.h      | SOV                             |
| Author  : Julien Becker                  |                                 |
| Started : 29/03/2011 16:06               |                                 |
`------------------------------------------/                                 |
                             |                                             |
                             `--------------------------------------------*/

#include "SegmentOverlapEvaluator.h"

using namespace lbcpp;

void SegmentOverlapEvaluator::addPrediction(ExecutionContext& context, const Variable& prediction,
                           const Variable& supervision, const ScoreObjectPtr& result) const
{
  ContainerPtr observed = supervision.getObjectAndCast<Container>();
  ContainerPtr predicted = prediction.getObjectAndCast<Container>();
  jassert(observed);
  jassert(predicted);
  
  SegmentOverlapScoreObjectPtr sovScore = result.dynamicCast<SegmentOverlapScoreObject>();
  const size_t numStates = enumeration->getNumElements();
  jassert(numStates);

  size_t sumNormFactor = 0;
  double sumSov = 0;
  for (size_t i = 0; i < numStates; ++i)
  {
    size_t normFactor;
    double sov = computeSov(observed, predicted, i, normFactor);
    sumNormFactor += normFactor;
    sumSov += sov;
    sovScore->addSovScore(i, normFactor ? sov / (float)normFactor : 0.f);
  }
  sovScore->addGlobalSov(sumNormFactor ? sumSov / (float)sumNormFactor : 0.f);
}

double SegmentOverlapEvaluator::computeSov(const ContainerPtr& observed, const ContainerPtr& predicted, size_t index, size_t& normFactor) const
{
  std::vector< std::pair<size_t, size_t> > observedSegments;
  std::vector< std::pair<size_t, size_t> > predictedSegments;
  extractSegments(observed, index, observedSegments);
  extractSegments(predicted, index, predictedSegments);

  std::vector< std::pair<size_t, size_t> > overlappingSegments;
  std::vector<size_t> noOverlappingSegments;
  findOverlappingSegments(observedSegments, predictedSegments, overlappingSegments, noOverlappingSegments);

  normFactor = computeNormalizationFactor(observedSegments, overlappingSegments, noOverlappingSegments);

  double res = 0;
  const size_t numOverlappingSegments = overlappingSegments.size();
  for (size_t i = 0; i < numOverlappingSegments; ++i)
  {
    const std::pair<size_t, size_t> obsSeg = observedSegments[overlappingSegments[i].first];
    const std::pair<size_t, size_t> preSeg = predictedSegments[overlappingSegments[i].second];
    size_t minOverlap;
    size_t maxOverlap;
    computeMinAndMaxOverlap(obsSeg, preSeg, minOverlap, maxOverlap);
    jassert(obsSeg.second > obsSeg.first);
    jassert(preSeg.second > preSeg.first);
    size_t delta = juce::jmin(maxOverlap - minOverlap, (int)minOverlap, (obsSeg.second - obsSeg.first) / 2, (preSeg.second - preSeg.first) / 2);
    res += (minOverlap + delta) / (float)maxOverlap * (obsSeg.second - obsSeg.first);
  }
  return res;
}

void SegmentOverlapEvaluator::extractSegments(const ContainerPtr& container, size_t index, std::vector< std::pair<size_t, size_t> >& result) const
{
  const size_t n = container->getNumElements();
  size_t startIndex = (size_t)-1;
  for (size_t i = 0; i < n; ++i)
  {
    const size_t indexOfMaximumValue = (size_t)container->getElement(i).getObjectAndCast<DoubleVector>()->getIndexOfMaximumValue();
    jassert(indexOfMaximumValue != (size_t)-1);
    if (indexOfMaximumValue == index)
    {
      if (startIndex == (size_t)-1)
        startIndex = i;
    }
    else
    {
      if (startIndex == (size_t)-1)
        continue;
      result.push_back(std::make_pair(startIndex, i));
      startIndex = (size_t)-1;
    }
  }

  if (startIndex != (size_t)-1)
    result.push_back(std::make_pair(startIndex, n));
}

void SegmentOverlapEvaluator::findOverlappingSegments(const std::vector< std::pair<size_t, size_t> >& observedSegments,
                             const std::vector< std::pair<size_t, size_t> >& predictedSegments,
                             std::vector< std::pair<size_t, size_t> >& overlappingSegments,
                             std::vector<size_t>& noOverlappingSegments) const
{
  const size_t numObservedSegments = observedSegments.size();
  const size_t numPredictedSegments = predictedSegments.size();
  // FIXME: Since segments are ordered, this search can be more efficient
  for (size_t i = 0; i < numObservedSegments; ++i)
  {
    bool hasIntersection = false;
    for (size_t j = 0; j < numPredictedSegments; ++j)
      if (isIntersection(observedSegments[i], predictedSegments[j]))
      {
        overlappingSegments.push_back(std::make_pair(i, j));
        hasIntersection = true;
      }
    if (!hasIntersection)
      noOverlappingSegments.push_back(i);
  }
}

bool SegmentOverlapEvaluator::isIntersection(const std::pair<size_t, size_t>& s1, const std::pair<size_t, size_t>& s2) const
{
  std::pair<size_t, size_t> left = s1;
  std::pair<size_t, size_t> right = s2;

  if (s2.first < s1.first)
  {
    left = s2;
    right = s1;
  }

  return right.first < left.second;
}

size_t SegmentOverlapEvaluator::computeNormalizationFactor(const std::vector< std::pair<size_t, size_t> >& observedSegments,
                                  const std::vector< std::pair<size_t, size_t> >& overlappingSegments,
                                  const std::vector<size_t>& noOverlappingSegments) const
{
  size_t res = 0;
  const size_t numOverlapping = overlappingSegments.size();
  for (size_t i = 0; i < numOverlapping; ++i)
  {
    const size_t segmentIndex = overlappingSegments[i].first;
    res += observedSegments[segmentIndex].second - observedSegments[segmentIndex].first;
  }
  const size_t numNoOverlapping = noOverlappingSegments.size();
  for (size_t i = 0; i < numNoOverlapping; ++i)
  {
    const size_t segmentIndex = noOverlappingSegments[i];
    res += observedSegments[segmentIndex].second - observedSegments[segmentIndex].first;
  }
  return res;
}

inline size_t min(size_t a, size_t b)
  {return a < b ? a : b;}

inline size_t max(size_t a, size_t b)
  {return a > b ? a : b;}

void SegmentOverlapEvaluator::computeMinAndMaxOverlap(const std::pair<size_t, size_t>& s1,
                             const std::pair<size_t, size_t>& s2,
                             size_t& minOverlap, size_t& maxOverlap) const
{
  size_t minMax = min(s1.second, s2.second);
  size_t maxMin = max(s1.first, s2.first);
  
  minOverlap = minMax - maxMin;
  
  size_t minMin = min(s1.first, s2.first);
  size_t maxMax = max(s1.second, s2.second);
  
  maxOverlap = maxMax - minMin;

  jassert(maxOverlap >= minOverlap);
}
