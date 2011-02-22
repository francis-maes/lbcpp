/*-----------------------------------------.---------------------------------.
| Filename: ClassificationAccuracyEvalua..h| Classification Accuracy         |
| Author  : Francis Maes                   |   Evaluator                     |
| Started : 27/04/2010 16:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_OLD_EVALUATOR_CLASSIFICATION_ACCURACY_H_
# define LBCPP_FUNCTION_OLD_EVALUATOR_CLASSIFICATION_ACCURACY_H_

# include <lbcpp/Function/OldEvaluator.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>
# include <lbcpp/NumericalLearning/LossFunctions.h>
# include <lbcpp/Core/DynamicObject.h>

namespace lbcpp
{

class OldClassificationAccuracyEvaluator : public OldEvaluator
{
public:
  OldClassificationAccuracyEvaluator(const String& name) : OldEvaluator(name), accuracy(new ScalarVariableMean()) {}
  OldClassificationAccuracyEvaluator() {}

  virtual void addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct)
  {
    int correctLabel = getLabel(correct);
    if (correctLabel >= 0)
      accuracy->push(correctLabel == getLabel(predicted));
  }
  
  virtual String toString() const
  {
    double count = accuracy->getCount();
    if (!count)
      return String::empty;
    return getName() + T(": ") + String(accuracy->getMean() * 100.0, 2) + T("% (") + String((int)count) + T(" examples)");
  }

  virtual double getDefaultScore() const
    {return accuracy->getMean();}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {res.push_back(std::make_pair(T("Accuracy"), accuracy->getMean()));}
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    OldEvaluator::clone(context, target);
    target.staticCast<OldClassificationAccuracyEvaluator>()->accuracy = accuracy->cloneAndCast<ScalarVariableMean>(context);
  }

protected:
  friend class OldClassificationAccuracyEvaluatorClass;

  ScalarVariableMeanPtr accuracy;

  int getLabel(const Variable& value) const
  {
    if (!value.exists())
      return -1;
    if (value.isEnumeration())
      return value.getInteger();
    DoubleVectorPtr scores = value.dynamicCast<DoubleVector>();
    if (scores)
      return scores->getIndexOfMaximumValue();

    jassert(false);
    return -1;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_OLD_EVALUATOR_CLASSIFICATION_ACCURACY_H_
