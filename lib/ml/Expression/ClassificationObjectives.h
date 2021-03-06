/*-----------------------------------------.---------------------------------.
| Filename: ClassificationObjectives.h     | Classification Objectives       |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 13:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_EXPRESSION_CLASSIFICATION_OBJECTIVES_H_
# define ML_EXPRESSION_CLASSIFICATION_OBJECTIVES_H_

# include <ml/Objective.h>

namespace lbcpp
{

class AccuracyObjective : public SupervisedLearningObjective
{
public:
  AccuracyObjective(TablePtr data, VariableExpressionPtr supervision)
    {configure(data, supervision);}
  AccuracyObjective() {}

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0.0; best = 1.0;}
};

class BinaryAccuracyObjective : public AccuracyObjective
{
public:
  BinaryAccuracyObjective(TablePtr data, VariableExpressionPtr supervision)
    {configure(data, supervision);}
  BinaryAccuracyObjective() {}

  virtual double evaluatePredictions(ExecutionContext& context, DataVectorPtr predictions) const
  {
    BVectorPtr supervisions = getSupervisions().staticCast<BVector>();
    
    // compute num successes
    size_t numSuccesses = 0;
    for (DataVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      unsigned char supervision = supervisions->get(it.getIndex());
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
    {configure(data, supervision);}
  MultiClassAccuracyObjective() {}

  virtual double evaluatePredictions(ExecutionContext& context, DataVectorPtr predictions) const
  {
    IVectorPtr supervisions = getSupervisions().staticCast<IVector>();
    
    // compute num successes
    size_t numSuccesses = 0;
    if (predictions->getElementsType()->inheritsFrom(integerClass))
    {
      for (DataVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
      {
        int sup = (int)supervisions->get(it.getIndex());
        int pred = it.getRawInteger();
        if (sup == pred)
          ++numSuccesses;
      }
    }
    else if (predictions->getElementsType()->inheritsFrom(denseDoubleVectorClass()))
    {
      for (DataVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
      {
        int sup = (int)supervisions->get(it.getIndex());
        int pred = it.getRawObject().staticCast<DenseDoubleVector>()->getIndexOfMaximumValue();
        if (sup == pred)
          ++numSuccesses;
      }
    }
    else
      jassertfalse;

    return numSuccesses / (double)supervisions->getNumElements();
  }
};

}; /* namespace lbcpp */

#endif // !ML_EXPRESSION_CLASSIFICATION_OBJECTIVES_H_
