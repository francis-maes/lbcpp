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
# include "DisulfideBondEvaluator.h"
# include "../Data/ProteinFunctions.h"

namespace lbcpp
{

class ProteinScoreObject : public ScoreObject
{
public:
  virtual double getScoreToMinimize() const
  {
    double sum = 0.0;
    for (size_t i = 0; i < scores.size(); ++i)
      sum += scores[i]->getScoreToMinimize();
    return sum / scores.size();
  }
  
  void addScoreObject(const ScoreObjectPtr& score)
    {scores.push_back(score);}
  
protected:
  friend class ProteinScoreObjectClass;

  std::vector<ScoreObjectPtr> scores;
};

class ProteinLearnerScoreObject : public ScoreObject
{
public:
  ProteinLearnerScoreObject(ScoreObjectPtr train, ScoreObjectPtr validation, ScoreObjectPtr test, size_t numFeaturesPerResidue)
    : train(train), validation(validation), test(test), numFeaturesPerResidue(numFeaturesPerResidue) {}
  ProteinLearnerScoreObject() {}
  
  virtual double getScoreToMinimize() const
    {return getValidatinScoreToMinimize();}
  
  double getTrainScoreToMinimize() const
    {return train ? train->getScoreToMinimize() : DBL_MAX;}
  
  double getValidatinScoreToMinimize() const
    {return validation ? validation->getScoreToMinimize() : DBL_MAX;}
  
  double getTestScoreToMinimize() const
    {return test ? test->getScoreToMinimize() : DBL_MAX;}
  
  size_t getNumFeaturesPerResidue() const
    {return numFeaturesPerResidue;}
  
protected:
  friend class ProteinLearnerScoreObjectClass;

  ScoreObjectPtr train;
  ScoreObjectPtr validation;
  ScoreObjectPtr test;
  size_t numFeaturesPerResidue;
};

typedef ReferenceCountedObjectPtr<ProteinLearnerScoreObject> ProteinLearnerScoreObjectPtr;

class ProteinEvaluator : public CompositeEvaluator
{
public:
  ProteinEvaluator(double oxidizedCysteineThreshold = 0.5f)
  : oxidizedCysteineThreshold(oxidizedCysteineThreshold)
  {
#if 0
/* Exemples */
//    addEvaluator(cbpTarget,  classificationEvaluator(), T("Cystein Bonding Property"));
//    addEvaluator(ss3Target,  elementContainerSupervisedEvaluator(classificationEvaluator()), T("Secondary Structure"));
//    addEvaluator(ss8Target,  elementContainerSupervisedEvaluator(classificationEvaluator()), T("DSSP Secondary Structure"));
//    addEvaluator(sa20Target, elementContainerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore)), T("Solvent Accessibility (@20)"));
//    addEvaluator(drTarget,   elementContainerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationMCCScore)), T("Disorder regions"));
//    addEvaluator(stalTarget, elementContainerSupervisedEvaluator(classificationEvaluator()), T("Structural Alphabet"));
//    addEvaluator(cma8Target, elementContainerSupervisedEvaluator(new ContactMapEvaluator(8)));
//    addEvaluator(cmb8Target, elementContainerSupervisedEvaluator(new ContactMapEvaluator(8)));
//    addEvaluator(cbsTarget,  elementContainerSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationSensitivityAndSpecificityScore, isFinalEvaluation)), T("Cystein Bonding States (Sens. & Spec.)"));
//    addEvaluator(dsbTarget,  new DisulfidePatternEvaluator(), T("Disulfide Bonds"));
//    addEvaluator(fdsbTarget,  new DisulfidePatternEvaluator(new GreedyDisulfidePatternBuilder(6)), T("Disulfide Symmetric Bonds (Greedy L=6)"));
//    addEvaluator(dsbTarget, new DoNotApplyOnDimensionGreaterThan(new DisulfidePatternEvaluator(new GreedyDisulfidePatternBuilder(6)), 10), T("Disulfide Bonds (Greedy L=6 & # <= 10)"));
//    addEvaluator(dsbTarget, new DoNotApplyOnDimensionGreaterThan(new DisulfidePatternEvaluator(new ExhaustiveDisulfidePatternBuilder()), 10), T("Disulfide Bonds (Exhaustive & # <= 10)"));
#endif // 0
/* Use ProteinEvaluator::addEvaluator outside of default constructor
    addEvaluator(dsbTarget,  symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationSensitivityAndSpecificityScore, isFinalEvaluation), 1), T("Disulfide Bonds (Sens. and Spec)"));
    addEvaluator(dsbTarget,  symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationMCCScore, isFinalEvaluation), 1), T("Disulfide Bonds (MCC)"));
    addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore), 1), T("Disulfide Bonds (Raw)"));
    addEvaluator(dsbTarget, new DisulfidePatternEvaluator(new GreedyDisulfidePatternBuilder(6, 0.0), 0.0), T("Disulfide Bonds (Greedy L=6)"));
*/
 }

  /* CompositeEvaluator */
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scoreObject, const ObjectPtr& example, const Variable& output) const
  {
    CompositeScoreObjectPtr scores = scoreObject.staticCast<CompositeScoreObject>();
    ProteinPtr supervision = example->getVariable(1).getObjectAndCast<Protein>();
    ProteinPtr predicted = output.getObjectAndCast<Protein>();

    /* Store container for fast access */
    size_t numTargets = targets.size();
    for (size_t i = 0; i < numTargets; ++i)
    {
      // noTarget = whole protein
      if (targets[i] == noTarget && !evaluators[i]->updateScoreObject(context, scoreObject, example, output))
        return false;
      else if (targets[i] == cbpTarget)
      {
        Variable supervisionValue = supervision->getTargetOrComputeIfMissing(context, (int)targets[i]);
        Variable predictedValue = predicted->getTargetOrComputeIfMissing(context, (int)targets[i]);
        
        if (!supervisionValue.exists() || !predictedValue.exists())
          continue;
  
        if (!evaluators[i]->updateScoreObject(context,
                                              scores->getScoreObject(i),
                                              new Pair(pairClass(anyType, anyType), supervisionValue, supervisionValue),
                                              predictedValue))
          return false;
        
        continue;
      }
      else
      {
        ContainerPtr supervisionContainer;
        ContainerPtr predictedContainer;
        if (targets[i] == odsbTarget)
        {
          supervisionContainer = supervision->getDisulfideBonds(context);
          predictedContainer = predicted->getOxidizedDisulfideBonds(context, oxidizedCysteineThreshold);
          /*
          std::cout << "Supervised DSB" << std::endl;
          std::cout << supervisionContainer->toString() << std::endl;
          std::cout << predicted->getCysteinBondingStates(context)->toString() << std::endl;
          std::cout << "Predicted ODSB" << std::endl;
          std::cout << predictedContainer->toString() << std::endl << std::endl;
          */
        }
        else
        {
          supervisionContainer = supervision->getTargetOrComputeIfMissing(context, (int)targets[i]).getObjectAndCast<Container>();
          predictedContainer = predicted->getTargetOrComputeIfMissing(context, (int)targets[i]).getObjectAndCast<Container>();
        }

        if (!supervisionContainer || !predictedContainer)
          continue;

        if (!evaluators[i]->updateScoreObject(context,
                                              scores->getScoreObject(i),
                                              new Pair(pairClass(anyType, anyType), supervisionContainer, supervisionContainer),
                                              predictedContainer))
          return false;
      }
    }
    return true;
  }

  ScoreObjectPtr getScoreObjectOfTarget(const CompositeScoreObjectPtr& scores, ProteinTarget target)
  {
    for (size_t i = 0; i < targets.size(); ++i)
      if (targets[i] == target)
        return scores->getScoreObject(i);
    return ScoreObjectPtr();
  }

  double getScoreToMinimize(const CompositeScoreObjectPtr& scores)
  {
    return scores->getScoreObject(scoreToMinimizeIndex)->getScoreToMinimize();
  }
  
  ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
  {
    CompositeScoreObjectPtr res = CompositeEvaluator::createEmptyScoreObject(context, function);
    res->setName(T("Protein's Scores"));
    for (size_t i = 0; i < targets.size(); ++i)
      res->getScoreObject(i)->setName(descriptions[i]);
    return res;
  }

  void addEvaluator(ProteinTarget target, EvaluatorPtr evaluator, const String& description, bool isTheScoreToMinimize = false)
  {
    if (isTheScoreToMinimize)
      scoreToMinimizeIndex = evaluators.size();
    targets.push_back(target);
    descriptions.push_back(description);
    CompositeEvaluator::addEvaluator(evaluator);
  }  

protected:
  double oxidizedCysteineThreshold;
  std::vector<ProteinTarget> targets;
  std::vector<String> descriptions;
  size_t scoreToMinimizeIndex;
};

typedef ReferenceCountedObjectPtr<ProteinEvaluator> ProteinEvaluatorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_H_
