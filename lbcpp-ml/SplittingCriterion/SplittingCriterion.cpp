/*-----------------------------------------.---------------------------------.
| Filename: SplittingCriterion.cpp         | Splitting Criterion             |
| Author  : Francis Maes                   |   base classes                  |
| Started : 22/12/2011 14:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/SplittingCriterion.h>
#include <lbcpp-ml/Function.h>
using namespace lbcpp;

/*
** SplittingCriterion
*/
void SplittingCriterion::configure(const TablePtr& data, const VariableExpressionPtr& supervision, const DenseDoubleVectorPtr& weights, const IndexSetPtr& indices)
{
  SupervisedLearningObjective::configure(data, supervision, weights, indices);
  invalidate();
}
  
void SplittingCriterion::setPredictions(const DataVectorPtr& predictions)
  {this->predictions = predictions; invalidate();}

void SplittingCriterion::ensureIsUpToDate()
{
  if (!upToDate)
  {
    update();
    upToDate = true;
  }
}

double SplittingCriterion::evaluate(ExecutionContext& context, const ObjectPtr& object)
{
  setPredictions(computePredictions(context, object.staticCast<Expression>()));
  return computeCriterion();
}

/*
** ClassificationSplittingCriterion
*/
void ClassificationSplittingCriterion::configure(const TablePtr& data, const VariableExpressionPtr& supervision, const DenseDoubleVectorPtr& weights, const IndexSetPtr& indices)
{
  SplittingCriterion::configure(data, supervision, weights, indices);
  labels = supervision->getType().staticCast<Enumeration>();
  numLabels = labels->getNumElements();
  supervisions = getSupervisions().staticCast<IVector>();
}
