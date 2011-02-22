/*-----------------------------------------.---------------------------------.
| Filename: ContactMapEvaluator.h          | The evaluator contact maps      |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 23:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_EVALUATOR_CONTACT_MAP_H_
# define LBCPP_PROTEIN_EVALUATOR_CONTACT_MAP_H_

# include <lbcpp/Function/OldEvaluator.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class ContactMapEvaluator : public OldEvaluator
{
public:
  ContactMapEvaluator(const String& name, size_t minimumDistance)
    : OldEvaluator(name),
      classificationEvaluator(oldBinaryClassificationConfusionEvaluator(name)), 
      rocEvaluator(oldROCAnalysisEvaluator(name)),
      minimumDistance(minimumDistance) {}

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

  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject)
  {
    if (!correctObject.exists() || !predictedObject.exists())
      return;

    const SymmetricMatrixPtr& predicted = predictedObject.getObjectAndCast<SymmetricMatrix>(context);
    const SymmetricMatrixPtr& correct = correctObject.getObjectAndCast<SymmetricMatrix>(context);

    jassert(predicted->getDimension() == predicted->getDimension());
    size_t n = predicted->getDimension();
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + minimumDistance; j < n; ++j)
      {
        Variable predictedElement = predicted->getElement(i, j);
        Variable correctElement = correct->getElement(i, j);
        jassert(predictedElement.getType() == probabilityType && correctElement.getType() == probabilityType);
        if (predictedElement.isMissingValue() || correctElement.isMissingValue())
          continue;

        bool shouldBePositive = correctElement.getDouble() > 0.5;
        classificationEvaluator->addPrediction(context, predictedElement.getDouble() > 0.5, shouldBePositive);
        rocEvaluator->addPrediction(context, predictedElement.getDouble(), shouldBePositive);
      }
  }

protected:
  OldEvaluatorPtr classificationEvaluator;
  OldEvaluatorPtr rocEvaluator;
  size_t minimumDistance;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_CONTACT_MAP_H_
