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
    addEvaluator(ss3Target,  containerSupervisedEvaluator(classificationEvaluator()));
    addEvaluator(ss8Target,  containerSupervisedEvaluator(classificationEvaluator()));
    addEvaluator(sa20Target, containerSupervisedEvaluator(binaryClassificationEvaluator()));
    addEvaluator(drTarget,   containerSupervisedEvaluator(binaryClassificationEvaluator()));
    addEvaluator(stalTarget, containerSupervisedEvaluator(classificationEvaluator()));
  }
  
  /* Evaluator */
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? functionClass : containerClass(pairClass(proteinClass, proteinClass));}
  
  /* CompositeEvaluator */
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scoreObject, const ObjectPtr& example, const Variable& output) const
  {
    CompositeScoreObjectPtr scores = scoreObject.staticCast<CompositeScoreObject>();
    ProteinPtr supervision = example->getVariable(1).getObjectAndCast<Protein>();
    ProteinPtr predicted = output.getObjectAndCast<Protein>();

    /* Strore container for fast access */
    size_t numTargets = targets.size();
    for (size_t i = 0; i < numTargets; ++i)
    {
      ContainerPtr supervisionContainer = supervision->getTargetOrComputeIfMissing(context, (int)targets[i]).getObjectAndCast<Container>();
      ContainerPtr predictedContainer = predicted->getTargetOrComputeIfMissing(context, (int)targets[i]).getObjectAndCast<Container>();

      if (!supervisionContainer || !predictedContainer)
        continue;

      if (!evaluators[i]->updateScoreObject(context,
                                            scores->getScoreObject(i),
                                            new Pair(pairClass(anyType, anyType), supervisionContainer, supervisionContainer),
                                            predictedContainer))
        return false;
    }
    return true;
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
