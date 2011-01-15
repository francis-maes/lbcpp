/*-----------------------------------------.---------------------------------.
| Filename: MultiLabelClassificationEval..h| MultiLabel Classification       |
| Author  : Francis Maes                   |   Evaluator                     |
| Started : 15/01/2011 16:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_MULTI_LABEL_CLASSIFICATION_H_
# define LBCPP_FUNCTION_EVALUATOR_MULTI_LABEL_CLASSIFICATION_H_

# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class MultiLabelClassificationEvaluator : public Evaluator
{
public:
  MultiLabelClassificationEvaluator(const String& name)
    : Evaluator(name), hammingLoss(new ScalarVariableMean(T("Hamming"))), accuracy(new ScalarVariableMean(T("Accuracy"))),
      precision(new ScalarVariableMean(T("Precision"))), recall(new ScalarVariableMean(T("Recall")))
  {
  }
  MultiLabelClassificationEvaluator() {}

  virtual void addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct)
  {
    if (!predicted.exists() || !correct.exists())
      return;

    jassert(predicted.isObject() && correct.isObject());
    const ObjectPtr& predictedObject = predicted.getObject();
    const ObjectPtr& correctObject = correct.getObject();
    jassert(predictedObject->getClass() == correctObject->getClass());

    size_t n = predictedObject->getNumVariables();
    BinaryClassificationConfusionMatrix confusionMatrix;
    for (size_t i = 0; i < n; ++i)
      confusionMatrix.addPredictionIfExists(context, predictedObject->getVariable(i), correctObject->getVariable(i));
    
    hammingLoss->push(1.0 - confusionMatrix.computeAccuracy());
    double prec, rec, f1;
    confusionMatrix.computePrecisionRecallAndF1(prec, rec, f1);
    accuracy->push(f1); // todo: verifier que c'est bien la meme chose
    precision->push(prec);
    recall->push(rec);
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
  friend class MultiLabelClassificationEvaluatorClass;

  ScalarVariableMeanPtr hammingLoss;
  ScalarVariableMeanPtr accuracy;
  ScalarVariableMeanPtr precision;
  ScalarVariableMeanPtr recall;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_MULTI_LABEL_CLASSIFICATION_H_
