/*-----------------------------------------.---------------------------------.
| Filename: MultiLabelClassificationEval..h| MultiLabel Classification       |
| Author  : Francis Maes                   |   Evaluator                     |
| Started : 15/01/2011 16:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_OLD_EVALUATOR_MULTI_LABEL_CLASSIFICATION_H_
# define LBCPP_FUNCTION_OLD_EVALUATOR_MULTI_LABEL_CLASSIFICATION_H_

# include <lbcpp/Function/OldEvaluator.h>
# include "Utilities.h"

namespace lbcpp
{

class OldMultiLabelClassificationEvaluator : public OldEvaluator
{
public:
  OldMultiLabelClassificationEvaluator(const String& name)
    : OldEvaluator(name), hammingLoss(new ScalarVariableMean(T("Hamming"))), accuracy(new ScalarVariableMean(T("Accuracy"))),
      precision(new ScalarVariableMean(T("Precision"))), recall(new ScalarVariableMean(T("Recall")))
  {
  }
  OldMultiLabelClassificationEvaluator() {}

  virtual void addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct)
  {
    if (!correct.exists())
      return;

    if (predicted.exists())
    {
      jassert(predicted.isObject() && correct.isObject());
      const ObjectPtr& predictedObject = predicted.getObject();
      const ObjectPtr& correctObject = correct.getObject();
      jassert(predictedObject->getClass() == correctObject->getClass());

      size_t n = predictedObject->getNumVariables();
      BinaryClassificationConfusionMatrix confusionMatrix;
      for (size_t i = 0; i < n; ++i)
      {
        bool p = false, c = false;
        confusionMatrix.convertToBoolean(context, predictedObject->getVariable(i), p);
        confusionMatrix.convertToBoolean(context, correctObject->getVariable(i), c);
        confusionMatrix.addPrediction(p, c);
      }
      
      hammingLoss->push(1.0 - confusionMatrix.computeAccuracy());
      accuracy->push(confusionMatrix.getTruePositives() / (double)(confusionMatrix.getSampleCount() - confusionMatrix.getTrueNegatives()));
      precision->push(confusionMatrix.computePrecision());
      recall->push(confusionMatrix.computeRecall());
    }
    else
    {
      hammingLoss->push(1.0);
      accuracy->push(0.0);
      precision->push(0.0);
      recall->push(0.0);
    }
  }

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
  {
    res.push_back(std::make_pair(T("HammingLoss"), hammingLoss->getMean()));
    res.push_back(std::make_pair(T("Accuracy"), accuracy->getMean()));
    res.push_back(std::make_pair(T("Precision"), precision->getMean()));
    res.push_back(std::make_pair(T("Recall"), recall->getMean()));
  }

  virtual String toString() const
  {
    if (!hammingLoss->getCount())
      return String::empty;
  
    return getName() + T(" hammingLoss: ") + String(hammingLoss->getMean()) + 
      T(" acc: ") + String(accuracy->getMean()) + 
      T(" prec: ") + String(precision->getMean()) + 
      T(" rec: ") + String(recall->getMean());
  }

  virtual double getDefaultScore() const
    {return accuracy->getMean();}

protected:
  friend class OldMultiLabelClassificationEvaluatorClass;

  ScalarVariableMeanPtr hammingLoss;
  ScalarVariableMeanPtr accuracy;
  ScalarVariableMeanPtr precision;
  ScalarVariableMeanPtr recall;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_OLD_EVALUATOR_MULTI_LABEL_CLASSIFICATION_H_
