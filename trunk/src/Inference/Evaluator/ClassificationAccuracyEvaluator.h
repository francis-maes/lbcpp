/*-----------------------------------------.---------------------------------.
| Filename: ClassificationAccuracyEvalua..h| Classification Accuracy         |
| Author  : Francis Maes                   |   Evaluator                     |
| Started : 27/04/2010 16:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EVALUATOR_CLASSIFICATION_ACCURACY_H_
# define LBCPP_EVALUATOR_CLASSIFICATION_ACCURACY_H_

# include <lbcpp/Inference/Evaluator.h>
# include <lbcpp/FeatureGenerator/EditableFeatureGenerator.h>
# include <lbcpp/Data/ProbabilityDistribution.h>

namespace lbcpp
{

class ClassificationAccuracyEvaluator : public Evaluator
{
public:
  ClassificationAccuracyEvaluator(const String& name) : Evaluator(name), accuracy(new ScalarVariableMean()) {}
  ClassificationAccuracyEvaluator() {}

  virtual void addPrediction(const Variable& predictedObject, const Variable& correctObject)
  {
    if (!predictedObject || !correctObject)
      return;
    jassert(correctObject.isEnumeration());
    if (predictedObject.isEnumeration())
    {
      jassert(predictedObject.getType() == correctObject.getType());
      accuracy->push(predictedObject.getInteger() == correctObject.getInteger());
    }
    else
    {
      DiscreteProbabilityDistributionPtr distribution = predictedObject.getObjectAndCast<DiscreteProbabilityDistribution>();
      jassert(distribution);
      accuracy->push(distribution->compute(correctObject));
    }
  }
  
  virtual String toString() const
  {
    double count = accuracy->getCount();
    if (!count)
      return String::empty;
    return getName() + T(": ") + String(accuracy->getMean() * 100.0, 2) + T("% (") + lbcpp::toString(count) + T(" examples)");
  }

  virtual double getDefaultScore() const
    {return accuracy->getMean();}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {res.push_back(std::make_pair(T("Acc"), accuracy->getMean()));}
  
protected:
  ScalarVariableMeanPtr accuracy;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATOR_CLASSIFICATION_ACCURACY_H_
