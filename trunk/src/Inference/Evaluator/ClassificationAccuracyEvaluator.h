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

namespace lbcpp
{

class ClassificationAccuracyEvaluator : public Evaluator
{
public:
  ClassificationAccuracyEvaluator(const String& name) : Evaluator(name), accuracy(new ScalarVariableMean()) {}
  ClassificationAccuracyEvaluator() {}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    LabelPtr predicted = predictedObject.dynamicCast<Label>();
    LabelPtr correct = correctObject.dynamicCast<Label>();
    if (!predicted || !correct)
      return;
    jassert(predicted->getDictionary() == correct->getDictionary());
    accuracy->push(predicted->getIndex() == correct->getIndex() ? 1.0 : 0.0);
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
