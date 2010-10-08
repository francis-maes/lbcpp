/*-----------------------------------------.---------------------------------.
| Filename: ContactMapEvaluator.h          | The evaluator contact maps      |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 23:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_EVALUATOR_CONTACT_MAP_H_
# define LBCPP_PROTEIN_EVALUATOR_CONTACT_MAP_H_

# include <lbcpp/Function/Evaluator.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class ContactMapEvaluator : public Evaluator
{
public:
  ContactMapEvaluator(const String& name)
    : Evaluator(name),
      classificationEvaluator(binaryClassificationConfusionEvaluator(name)), 
      rocEvaluator(rocAnalysisEvaluator(name)) {}

  virtual String toString() const
  {
    String res = classificationEvaluator->toString();
    String str2 = rocEvaluator->toString();
    if (res.isNotEmpty() && str2.isNotEmpty())
      res += T("\n");
    res += str2;
    return res;
  }

  virtual double getDefaultScore() const
    {return rocEvaluator->getDefaultScore();}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
  {
    classificationEvaluator->getScores(res);
    rocEvaluator->getScores(res);
  }

  virtual void addPrediction(const Variable& predictedObject, const Variable& correctObject)
  {
    if (!correctObject || !predictedObject)
      return;

    SymmetricMatrixPtr predicted = predictedObject.getObjectAndCast<SymmetricMatrix>();
    SymmetricMatrixPtr correct = correctObject.getObjectAndCast<SymmetricMatrix>();

    jassert(predicted->getDimension() == predicted->getDimension());
    size_t n = predicted->getDimension();
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 6; j < n; ++j)
      {
        Variable predictedElement = predicted->getElement(i, j);
        Variable correctElement = correct->getElement(i, j);
        jassert(predictedElement.getType() == probabilityType && correctElement.getType() == probabilityType);
        if (predictedElement.isMissingValue() || correctElement.isMissingValue())
          continue;

        bool shouldBePositive = correctElement.getDouble() > 0.5;
        classificationEvaluator->addPrediction(predictedElement.getDouble() > 0.5, shouldBePositive);
        rocEvaluator->addPrediction(predictedElement.getDouble(), shouldBePositive);
      }
  }

protected:
  EvaluatorPtr classificationEvaluator;
  EvaluatorPtr rocEvaluator;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_CONTACT_MAP_H_
