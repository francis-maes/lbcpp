/*-----------------------------------------.---------------------------------.
| Filename: SplittingCriterion.h           | Splitting Criterion             |
| Author  : Francis Maes                   |                                 |
| Started : 04/01/2012 20:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SPLITTING_CRITERION_H_
# define LBCPP_ML_SPLITTING_CRITERION_H_

# include "Expression.h"
# include "Objective.h"

namespace lbcpp
{

class SplittingCriterion : public SupervisedLearningObjective
{
public:
  SplittingCriterion() : upToDate(false) {}

  virtual void configure(const TablePtr& data, const VariableExpressionPtr& supervision, const DenseDoubleVectorPtr& weights, const IndexSetPtr& indices);
  virtual void setPredictions(const DataVectorPtr& predictions);

  virtual void update() = 0;
  virtual void flipPrediction(size_t index) = 0; // flip from negative prediction to positive prediction
  virtual double computeCriterion() = 0;
  virtual ObjectPtr computeVote(const IndexSetPtr& indices) = 0;

  void ensureIsUpToDate();
  void invalidate()
    {upToDate = false;}

  // Objective
  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object);

protected:
  bool upToDate;

  DataVectorPtr predictions;
};

typedef ReferenceCountedObjectPtr<SplittingCriterion> SplittingCriterionPtr;

class ClassificationSplittingCriterion : public SplittingCriterion
{
public:
  ClassificationSplittingCriterion() : numLabels(0) {}

  virtual void configure(const TablePtr& data, const VariableExpressionPtr& supervision, const DenseDoubleVectorPtr& weights, const IndexSetPtr& indices);

protected:
  IVectorPtr supervisions;
  EnumerationPtr labels;
  size_t numLabels;
};

typedef ReferenceCountedObjectPtr<ClassificationSplittingCriterion> ClassificationSplittingCriterionPtr;

extern ClassificationSplittingCriterionPtr informationGainSplittingCriterion(bool normalize);

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SPLITTING_CRITERION_H_
