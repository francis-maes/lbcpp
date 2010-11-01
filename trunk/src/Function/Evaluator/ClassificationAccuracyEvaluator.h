/*-----------------------------------------.---------------------------------.
| Filename: ClassificationAccuracyEvalua..h| Classification Accuracy         |
| Author  : Francis Maes                   |   Evaluator                     |
| Started : 27/04/2010 16:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_CLASSIFICATION_ACCURACY_H_
# define LBCPP_FUNCTION_EVALUATOR_CLASSIFICATION_ACCURACY_H_

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Data/ProbabilityDistribution.h>
# include <lbcpp/NumericalLearning/LossFunctions.h>
# include "../../Data/Object/DenseDoubleObject.h"

namespace lbcpp
{

class ClassificationAccuracyEvaluator : public Evaluator
{
public:
  ClassificationAccuracyEvaluator(const String& name) : Evaluator(name), accuracy(new ScalarVariableMean()) {}
  ClassificationAccuracyEvaluator() {}

  virtual void addPrediction(const Variable& predicted, const Variable& correct)
  {
    if (!predicted.exists() || !correct.exists())
      return;

    int correctLabel = getCorrectLabel(correct);
    if (correctLabel < 0)
      return;

    if (predicted.isEnumeration())
    {
      accuracy->push(predicted.getInteger() == correctLabel);
      return;
    }
    
    if (predicted.isObject())
    {
      DenseDoubleObjectPtr scores = predicted.dynamicCast<DenseDoubleObject>();
      if (scores)
      {
        const std::vector<double>& scoreValues = scores->getValues();
        double bestScore = -DBL_MAX;
        int bestIndex = -1;
        for (size_t i = 0; i < scoreValues.size(); ++i)
          if (scoreValues[i] > bestScore)
            bestScore = scoreValues[i], bestIndex = (int)i;
        accuracy->push(bestIndex == correctLabel);
        return;
      }

      DiscreteProbabilityDistributionPtr distribution = predicted.dynamicCast<DiscreteProbabilityDistribution>();
      if (distribution)
      {
        accuracy->push(distribution->compute(correctLabel));
        return;
      }
    }

    jassert(false);
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
    {res.push_back(std::make_pair(getName(), accuracy->getMean()));}
  
  virtual void clone(const ObjectPtr& target) const
    {Evaluator::clone(target); target.staticCast<ClassificationAccuracyEvaluator>()->accuracy = accuracy->cloneAndCast<ScalarVariableMean>();}

protected:
  friend class ClassificationAccuracyEvaluatorClass;

  ScalarVariableMeanPtr accuracy;

  int getCorrectLabel(const Variable& correct) const
  {
    if (!correct.exists())
      return -1;
    if (correct.isEnumeration())
      return correct.getInteger();

    MultiClassLossFunctionPtr lossFunction = correct.dynamicCast<MultiClassLossFunction>();
    if (lossFunction)
      return lossFunction->getCorrectClass();

    jassert(false);
    return -1;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_CLASSIFICATION_ACCURACY_H_
