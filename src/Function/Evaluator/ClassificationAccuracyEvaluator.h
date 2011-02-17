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
# include <lbcpp/Distribution/DiscreteDistribution.h>
# include <lbcpp/NumericalLearning/LossFunctions.h>
# include <lbcpp/Core/DynamicObject.h>

namespace lbcpp
{

class ClassificationAccuracyEvaluator : public Evaluator
{
public:
  ClassificationAccuracyEvaluator(const String& name) : Evaluator(name), accuracy(new ScalarVariableMean()) {}
  ClassificationAccuracyEvaluator() {}

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
    Evaluator::clone(context, target);
    target.staticCast<ClassificationAccuracyEvaluator>()->accuracy = accuracy->cloneAndCast<ScalarVariableMean>(context);
  }

protected:
  friend class ClassificationAccuracyEvaluatorClass;

  ScalarVariableMeanPtr accuracy;

  int getLabel(const Variable& value) const
  {
    if (!value.exists())
      return -1;
    if (value.isEnumeration())
      return value.getInteger();

    if (value.isObject())
    {
      DenseDoubleVectorPtr scores = value.dynamicCast<DenseDoubleVector>();
      if (scores)
      {
        const std::vector<double>& scoreValues = scores->getValues();
        double bestScore = -DBL_MAX;
        int bestIndex = -1;
        for (size_t i = 0; i < scoreValues.size(); ++i)
          if (scoreValues[i] > bestScore)
            bestScore = scoreValues[i], bestIndex = (int)i;
        return bestIndex;
      }

      EnumerationDistributionPtr distribution = value.dynamicCast<EnumerationDistribution>();
      if (distribution)
      {
        size_t n = distribution->getEnumeration()->getNumElements();
        double bestProb = 0.0;
        int bestIndex = -1;
        for (size_t i = 0; i < n; ++i)
        {
          double prob = distribution->getProbability(i);
          if (prob > bestProb)
            bestProb = prob, bestIndex = (int)i;
        }
        return bestIndex;
      }    
    }

    jassert(false);
    return -1;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_CLASSIFICATION_ACCURACY_H_
