/*-----------------------------------------.---------------------------------.
| Filename: ClassificationObjectives.h     | Classification Objectives       |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 13:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_CLASSIFICATION_OBJECTIVES_H_
# define LBCPP_ML_EXPRESSION_CLASSIFICATION_OBJECTIVES_H_

# include <lbcpp-ml/Objective.h>

namespace lbcpp
{

class AccuracyObjective : public SupervisedLearningObjective
{
public:
  AccuracyObjective(TablePtr data, VariableExpressionPtr supervision)
    : SupervisedLearningObjective(data, supervision) {}
  AccuracyObjective() {}

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0.0; best = 1.0;}
};

class BinaryAccuracyObjective : public AccuracyObjective
{
public:
  BinaryAccuracyObjective(TablePtr data, VariableExpressionPtr supervision)
    : AccuracyObjective(data, supervision) {}
  BinaryAccuracyObjective() {}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    // retrieve predictions and supervisions
    ExpressionPtr expression = object.staticCast<Expression>();
    DataVectorPtr predictions = computePredictions(context, expression);
    BooleanVectorPtr supervisions = getSupervisions().staticCast<BooleanVector>();
    
    // compute num successes
    size_t numSuccesses = 0;
    for (DataVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      unsigned char supervision = supervisions->getData()[it.getIndex()];
      unsigned char prediction = it.getRawBoolean();
      if (supervision == prediction)
        ++numSuccesses;
    }

    return numSuccesses / (double)supervisions->getNumElements();
  }
};

class MultiClassAccuracyObjective : public AccuracyObjective
{
public:
  MultiClassAccuracyObjective(TablePtr data, VariableExpressionPtr supervision)
    : AccuracyObjective(data, supervision) {}
  MultiClassAccuracyObjective() {}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    // retrieve predictions and supervisions
    ExpressionPtr expression = object.staticCast<Expression>();
    DataVectorPtr predictions = computePredictions(context, expression);
    VectorPtr supervisions = getSupervisions();
    
    // compute num successes
    size_t numSuccesses = 0;
    for (DataVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      int sup = supervisions->getElement(it.getIndex()).getInteger();
      int pred = it.getRawInteger();
      if (sup == pred)
        ++numSuccesses;
    }

    return numSuccesses / (double)supervisions->getNumElements();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_CLASSIFICATION_OBJECTIVES_H_
