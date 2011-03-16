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

class ContactMapEvaluator : public CompositeEvaluator
{
public:
  ContactMapEvaluator(size_t minimumDistance)
    : minimumDistance(minimumDistance)
  {
    addEvaluator(classificationEvaluator());
    addEvaluator(rocAnalysisEvaluator());
  }
  ContactMapEvaluator() {}

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context) const
  {
    CompositeScoreObjectPtr res = CompositeEvaluator::createEmptyScoreObject(context);
    res->getScoreObject(0)->setName(T("Classification"));
    res->getScoreObject(1)->setName(T("Roc Analisis"));
    return res;
  }
  
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scoreObject, const ObjectPtr& example, const Variable& output) const
  {
    CompositeScoreObjectPtr scores = scoreObject.staticCast<CompositeScoreObject>();
    const SymmetricMatrixPtr& predicted = output.getObjectAndCast<SymmetricMatrix>(context);
    const SymmetricMatrixPtr& correct = example->getVariable(1).getObjectAndCast<SymmetricMatrix>(context);

    jassert(predicted->getDimension() == predicted->getDimension());
    
    VectorPtr supervisionContainer = vector(correct->getElementsType(), correct->getNumElements());
    VectorPtr predictedContainer = vector(predicted->getElementsType(), predicted->getNumElements());
    
    size_t n = predicted->getDimension();
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + minimumDistance; j < n; ++j)
      {
        supervisionContainer->append(correct->getElement(i, j));
        predictedContainer->append(predicted->getElement(i, j));
      }

    return evaluators[0]->updateScoreObject(context, scores->getScoreObject(0), new Pair(pairClass(anyType, anyType), supervisionContainer, supervisionContainer), predictedContainer)
           && evaluators[1]->updateScoreObject(context, scores->getScoreObject(1), new Pair(pairClass(anyType, anyType), supervisionContainer, supervisionContainer), predictedContainer);
  }

protected:
  friend class ContactMapEvaluatorClass;
  
  size_t minimumDistance;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_CONTACT_MAP_H_
