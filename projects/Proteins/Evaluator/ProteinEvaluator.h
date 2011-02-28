/*-----------------------------------------.---------------------------------.
| Filename: ProteinEvaluator.h             | Protein Evaluator               |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 23:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_EVALUATOR_H_
# define LBCPP_PROTEIN_EVALUATOR_H_

# include <lbcpp/Core/CompositeFunction.h>
# include "ContactMapEvaluator.h"
# include "TertiaryStructureEvaluator.h"

namespace lbcpp
{

class ProteinEvaluator : public CompositeEvaluator
{
public:
  ProteinEvaluator()
  {
    addEvaluator(ss3Target,  classificationEvaluator());
    addEvaluator(ss8Target,  classificationEvaluator());
    addEvaluator(sa20Target, binaryClassificationEvaluator());
    addEvaluator(drTarget,   binaryClassificationEvaluator());
    addEvaluator(stalTarget, classificationEvaluator());
  }
  
  /* Evaluator */
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? functionClass : containerClass(pairClass(proteinClass, proteinClass));}
  
  /* CompositeEvaluator */
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scoreObject, const ObjectPtr& example, const Variable& output) const
  {
    CompositeScoreObjectPtr scores = scoreObject.staticCast<CompositeScoreObject>();
    //ProteinPtr input = example->getVariable(0).getObjectAndCast<Protein>();
    ProteinPtr supervision = example->getVariable(1).getObjectAndCast<Protein>();
    ProteinPtr predicted = output.getObjectAndCast<Protein>();
    /* Strore container for fast access */
    size_t numTargets = targets.size();
    //std::vector<ContainerPtr> inputContainer(numTargets);
    std::vector<ContainerPtr> supervisionContainer(numTargets);
    std::vector<ContainerPtr> predictedContainer(numTargets);
    
    for (size_t i = 0; i < numTargets; ++i)
    {
      //inputContainer[i] = input->getTargetOrComputeIfMissing((int)targets[i]).getObjectAndCast<Container>();
      supervisionContainer[i] = supervision->getTargetOrComputeIfMissing((int)targets[i]).getObjectAndCast<Container>();
      predictedContainer[i] = predicted->getTargetOrComputeIfMissing((int)targets[i]).getObjectAndCast<Container>();
    }
    /* Call updataScoreObject for each (sub)example and each evaluator */
    size_t n = supervision->getLength();
    bool res = true;
    for (size_t i = 0; i < n; ++i)
      for (size_t j = 0; j < numTargets; ++j)
      {
        if (predictedContainer[j])
          res &= evaluators[j]->updateScoreObject(context, scores->getScoreObject(j),
                                           new Pair(pairClass(anyType, anyType), Variable(), supervisionContainer[j]->getElement(i)),
                                           predictedContainer[j]->getElement(i));
      }
    return res;
  }

protected:
  std::vector<ProteinTarget> targets;
  
  void addEvaluator(ProteinTarget target, EvaluatorPtr evaluator)
  {
    targets.push_back(target);
    CompositeEvaluator::addEvaluator(evaluator);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_H_
