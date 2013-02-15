/*-----------------------------------------.---------------------------------.
| Filename: SegmentOverlapEvaluator.h      | SOV                             |
| Author  : Julien Becker                  |                                 |
| Started : 29/03/2011 16:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*
 * The Segment Overlap algorithm is based on the paper:
 * "A Modified Definition of Sov, a Segment-Based Measure for Protein
 *  Secodary Structure Prediction Assessement", Zemla et al. 1999
 */

#ifndef LBCPP_PROTEIN_EVALUATOR_SEGMENT_OVERLAP_H_
# define LBCPP_PROTEIN_EVALUATOR_SEGMENT_OVERLAP_H_

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Data/RandomVariable.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class SegmentOverlapScoreObject : public ScoreObject
{
public:
  SegmentOverlapScoreObject(const EnumerationPtr& enumeration)
  : globalSovMean(0.f), globalSov(new ScalarVariableMean())
  {
    const size_t numElements = enumeration->getNumElements();
    sovVectors.resize(numElements);
    for (size_t i = 0; i < numElements; ++i)
      sovVectors[i] = new ScalarVariableMean();
  }

  virtual double getScoreToMinimize() const
    {return 1.0 - globalSovMean;}

  void addSovScore(size_t index, double value)
    {jassert(index < sovVectors.size()); sovVectors[index]->push(value);}

  void addGlobalSov(double value)
    {globalSov->push(value);}

  void finalize()
    {globalSovMean = globalSov->getMean();}

protected:
  friend class SegmentOverlapScoreObjectClass;

  SegmentOverlapScoreObject() {}

  double globalSovMean;
  std::vector<ScalarVariableMeanPtr> sovVectors;
  ScalarVariableMeanPtr globalSov;
};

typedef ReferenceCountedObjectPtr<SegmentOverlapScoreObject> SegmentOverlapScoreObjectPtr;

class SegmentOverlapEvaluator : public SupervisedEvaluator
{
public:
  SegmentOverlapEvaluator(const EnumerationPtr& enumeration)
    : enumeration(enumeration) {}

  /* SupervisedEvaluator */
  virtual TypePtr getRequiredPredictionType() const
    {return containerClass(doubleVectorClass(enumeration, probabilityType));}

  virtual TypePtr getRequiredSupervisionType() const
    {return containerClass(doubleVectorClass(enumeration, probabilityType));}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& prediction,
                             const Variable& supervision, const ScoreObjectPtr& result) const;

  double computeSov(const ContainerPtr& observed, const ContainerPtr& predicted, size_t index, size_t& normFactor) const;

  void extractSegments(const ContainerPtr& container, size_t index, std::vector< std::pair<size_t, size_t> >& result) const;

  void findOverlappingSegments(const std::vector< std::pair<size_t, size_t> >& observedSegments,
                               const std::vector< std::pair<size_t, size_t> >& predictedSegments,
                               std::vector< std::pair<size_t, size_t> >& overlappingSegments,
                               std::vector<size_t>& noOverlappingSegments) const;

  bool isIntersection(const std::pair<size_t, size_t>& s1, const std::pair<size_t, size_t>& s2) const;

  size_t computeNormalizationFactor(const std::vector< std::pair<size_t, size_t> >& observedSegments,
                                    const std::vector< std::pair<size_t, size_t> >& overlappingSegments,
                                    const std::vector<size_t>& noOverlappingSegments) const;

  void computeMinAndMaxOverlap(const std::pair<size_t, size_t>& s1,
                               const std::pair<size_t, size_t>& s2,
                               size_t& minOverlap, size_t& maxOverlap) const;
  
  /* Evaluator */
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new SegmentOverlapScoreObject(enumeration);}
  
  virtual void finalizeScoreObject(const ScoreObjectPtr& score, const FunctionPtr& function) const
    {score.dynamicCast<SegmentOverlapScoreObject>()->finalize();}

protected:
  friend class SegmentOverlapEvaluatorClass;

  EnumerationPtr enumeration;

  SegmentOverlapEvaluator() {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_SEGMENT_OVERLAP_H_
