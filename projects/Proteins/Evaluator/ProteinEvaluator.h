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

class ProteinEvaluator : public CompositeEvaluator
{
public:
  ProteinEvaluator(bool isFinalEvaluation = false)
  {
    addEvaluator(ss3Target,  containerSupervisedEvaluator(classificationEvaluator()));
    addEvaluator(ss8Target,  containerSupervisedEvaluator(classificationEvaluator()));
    addEvaluator(sa20Target, containerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore)));
    addEvaluator(drTarget,   containerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationMCCScore)));
    addEvaluator(stalTarget, containerSupervisedEvaluator(classificationEvaluator()));
//    addEvaluator(cma8Target, containerSupervisedEvaluator(new ContactMapEvaluator(8)));
//    addEvaluator(cmb8Target, containerSupervisedEvaluator(new ContactMapEvaluator(8)));
    addEvaluator(dsbTarget,  symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationAccuracyScore, isFinalEvaluation), 1));
    addEvaluator(dsbTarget,  symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationMCCScore, isFinalEvaluation), 1));
    addEvaluator(dsbTarget,  new DisulfidePatternEvaluator());
    addEvaluator(cbsTarget,  containerSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationSensitivityAndSpecificityScore, isFinalEvaluation)));
  }

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

  ScoreObjectPtr getScoreObjectOfTarget(const CompositeScoreObjectPtr& scores, ProteinTarget target)
  {
    for (size_t i = 0; i < targets.size(); ++i)
      if (targets[i] == target)
        return scores->getScoreObject(i);
    return ScoreObjectPtr();
  }
  
  ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
  {
    CompositeScoreObjectPtr res = CompositeEvaluator::createEmptyScoreObject(context, function);
    res->setName(T("Protein's Scores"));
    for (size_t i = 0; i < targets.size(); ++i)
      res->getScoreObject(i)->setName(proteinClass->getMemberVariableName((int)targets[i]));
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
