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
    jassertfalse;
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
      //evaluators[i].second->addPrediction(context, predictedVariable, correctVariable, score);
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

class MergeContainerFunction : public SimpleUnaryFunction
{
public:
  MergeContainerFunction()
    : SimpleUnaryFunction(containerClass(containerClass(anyType)), containerClass(anyType))
    {}
  
protected:
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ContainerPtr inputContainer = input.getObjectAndCast<Container>();
    VectorPtr res = vector(inputContainer->getElementsType()->getTemplateArgument(0));
    size_t n = inputContainer->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ContainerPtr values = inputContainer->getElement(i).getObjectAndCast<Container>();
      const size_t numElements = values->getNumElements();
      const size_t totalNumElements = res->getNumElements();
      res->resize(totalNumElements + numElements);
      for (size_t i = 0; i < numElements; ++i)
        res->setElement(totalNumElements + i, values->getElement(i));
    }
    return res;
  }
};

class ProteinEvaluatorCompositeFunction : public CompositeFunction
{
public:
  ProteinEvaluatorCompositeFunction()
    {numInputs = 2;}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t predicted = builder.addInput(containerClass(proteinClass), T("predicted"));
    size_t supervision = builder.addInput(containerClass(proteinClass), T("supervision"));
    
    size_t ss3 = builder.addFunction(mapContainerFunction(getVariableFunction(ss3Target)), predicted, T("ss3"));
    ss3 = builder.addFunction(new MergeContainerFunction(), ss3, T("ss3"));
    
    size_t ss3Supervision = builder.addFunction(mapContainerFunction(getVariableFunction(ss3Target)), supervision, T("ss3Supervision"));
    ss3Supervision = builder.addFunction(new MergeContainerFunction(), ss3Supervision, T("ss3Supervision"));
    
    builder.startSelection();

      builder.addFunction(classificationAccuracyEvaluator(), ss3, ss3Supervision, T("ss3"));

    builder.finishSelectionWithFunction(concatenateScoreObjectFunction());
  }

};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_H_
