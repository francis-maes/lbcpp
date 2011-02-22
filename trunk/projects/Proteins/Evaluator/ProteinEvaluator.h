/*-----------------------------------------.---------------------------------.
| Filename: ProteinEvaluator.h             | Protein Evaluator               |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 23:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_EVALUATOR_H_
# define LBCPP_PROTEIN_EVALUATOR_H_

# include "ContactMapEvaluator.h"
# include "TertiaryStructureEvaluator.h"

namespace lbcpp
{

class ProteinScoreObject : public ScoreObject
{
public:
  virtual double getScoreToMinimize() const
    {jassertfalse; return 0.0;}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
  {
    for (size_t i = 0; i < scores.size(); ++i)
    {
      std::vector<std::pair<String, double> > childScores;
      scores[i]->getScores(childScores);
      for (size_t j = 0; j < scores.size(); ++j)
        res.push_back(std::make_pair(scores[i]->getName() + T("[") + childScores[j].first + T("]"), childScores[j].second));
    }
  }

  void pushScoreObject(const ScoreObjectPtr& score)
    {scores.push_back(score);}
  
  ScoreObjectPtr getScoreObject(size_t index) const
    {return scores[index];}
  
  virtual String toString() const
  {
    String res;
    for (size_t i = 0; i < scores.size(); ++i)
    {
      String str = scores[i]->toString();
      if (str.isNotEmpty())
        res += str + T("\n");
    }
    return res;
  }
  
  lbcpp_UseDebuggingNewOperator
  
protected:
  friend class ProteinScoreObjectClass;

  std::vector<ScoreObjectPtr> scores;
};

typedef ReferenceCountedObjectPtr<ProteinScoreObject> ProteinScoreObjectPtr;

class ProteinEvaluator : public Evaluator
{
public:
  virtual TypePtr getRequiredPredictedElementsType() const
    {return proteinClass;}
  
  virtual TypePtr getRequiredSupervisionElementsType() const
    {return proteinClass;}
  
  ProteinEvaluator()
  {
    // 1D
    addEvaluator(ss3Target,  classificationAccuracyEvaluator());
    addEvaluator(ss8Target,  classificationAccuracyEvaluator());
    addEvaluator(sa20Target, binaryClassificationConfusionEvaluator());
    addEvaluator(drTarget,   binaryClassificationConfusionEvaluator());
    addEvaluator(stalTarget, classificationAccuracyEvaluator());

    // 2D
//    addEvaluator(T("contactMap8Ca"), new ContactMapEvaluator(T("RRa"), 6));
//    addEvaluator(T("contactMap8Cb"), new ContactMapEvaluator(T("RRb"), 6));
//    addEvaluator(T("disulfideBonds"), new ContactMapEvaluator(T("DSB"), 1));

    // 3D
//    addEvaluator(T("tertiaryStructure"), new TertiaryStructureEvaluator(T("TS")));
  }

protected:
  std::vector<std::pair<ProteinTarget, EvaluatorPtr> > evaluators;

  virtual ScoreObjectPtr createEmptyScoreObject() const
  {
    ProteinScoreObjectPtr res = new ProteinScoreObject();
    for (size_t i = 0; i < evaluators.size(); ++i)
      res->pushScoreObject(evaluators[i].second->createEmptyScoreObject());
    return res;
  }

  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, ScoreObjectPtr& result) const
  {
    const ProteinPtr& predicted = predictedObject.getObjectAndCast<Protein>(context);
    const ProteinPtr& correct = correctObject.getObjectAndCast<Protein>(context);
    for (size_t i = 0; i < evaluators.size(); ++i)
    {
      ProteinTarget target = evaluators[i].first;
      Variable predictedVariable = predicted->getVariable(target);
      Variable correctVariable = correct->getTargetOrComputeIfMissing(target);
      ScoreObjectPtr score = result.staticCast<ProteinScoreObject>()->getScoreObject(i);
      evaluators[i].second->addPrediction(context, predictedVariable, correctVariable, score);
    }
  }
/*
  void getScoresForTarget(ExecutionContext& context, const String& targetName, std::vector< std::pair<String, double> >& res) const
  {
    EvaluatorPtr evaluator = getEvaluatorForTarget(context, targetName);
    jassert(evaluator);
    evaluator->getScores(res);
  }
*/
  void addEvaluator(ProteinTarget target, EvaluatorPtr evaluator)
    {evaluators.push_back(std::make_pair(target, evaluator));}

  EvaluatorPtr getEvaluatorForTarget(ExecutionContext& context, ProteinTarget target) const
  {
    for (size_t i = 0; i < evaluators.size(); ++i)
      if (evaluators[i].first == target)
        return evaluators[i].second;
    
    context.errorCallback(T("ProteinEvaluator::getEvaluatorForTarget"), T("Could not find evaluator"));
    return EvaluatorPtr();
  }
};

typedef ReferenceCountedObjectPtr<ProteinEvaluator> ProteinEvaluatorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_H_
