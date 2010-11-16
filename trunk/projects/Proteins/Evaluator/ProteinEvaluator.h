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

class ProteinEvaluator : public Evaluator
{
public:
  ProteinEvaluator() : Evaluator(T("Protein")), numProteins(0)
  {
    // 1D
    addEvaluator(T("secondaryStructure"), sequenceLabelingAccuracyEvaluator(T("SS3")));
    addEvaluator(T("dsspSecondaryStructure"), sequenceLabelingAccuracyEvaluator(T("SS8")));
    addEvaluator(T("solventAccessibilityAt20p"), binarySequenceLabelingConfusionEvaluator(T("SA2")));
    addEvaluator(T("disorderRegions"), binarySequenceLabelingConfusionEvaluator(T("DR")));
    addEvaluator(T("structuralAlphabetSequence"), sequenceLabelingAccuracyEvaluator(T("StAl")));

    // 2D
    addEvaluator(T("contactMap8Ca"), new ContactMapEvaluator(T("RRa"), 6));
    addEvaluator(T("contactMap8Cb"), new ContactMapEvaluator(T("RRb"), 6));
    addEvaluator(T("disulfideBonds"), new ContactMapEvaluator(T("DSB"), 1));

    // 3D
    addEvaluator(T("tertiaryStructure"), new TertiaryStructureEvaluator(T("TS")));
  }

  virtual String toString() const
  {
    String res;
    res += String((int)numProteins) + T(" proteins");
    res += "\n";
    for (size_t i = 0; i < evaluators.size(); ++i)
    {
      String str = evaluators[i].second->toString();
      if (str.isNotEmpty())
        res += str + T("\n");
    }
    return res;
  }

  virtual void addPrediction(const Variable& predictedObject, const Variable& correctObject)
  {
    if (!correctObject.exists() || !predictedObject.exists())
      return;

    const ProteinPtr& predicted = predictedObject.getObjectAndCast<Protein>();
    const ProteinPtr& correct = correctObject.getObjectAndCast<Protein>();

    ++numProteins;
    for (size_t i = 0; i < evaluators.size(); ++i)
    {
      size_t variableIndex = evaluators[i].first;
      Variable predictedVariable = predicted->getVariable(variableIndex);
      if (predictedVariable.exists())
      {
        Variable correctVariable = correct->getTargetOrComputeIfMissing(variableIndex);
        evaluators[i].second->addPrediction(predictedVariable, correctVariable);
      }
    }
  }

  EvaluatorPtr getEvaluatorForTarget(const String& targetName) const
  {
    int variableIndex = proteinClass->findObjectVariable(targetName);
    if (variableIndex < 0)
    {
      MessageCallback::error(T("ProteinEvaluator::getEvaluatorForTarget"), T("Unknown target ") + targetName);
      return EvaluatorPtr();
    }

    for (size_t i = 0; i < evaluators.size(); ++i)
      if (evaluators[i].first == (size_t)variableIndex)
        return evaluators[i].second;
    
    MessageCallback::error(T("ProteinEvaluator::getEvaluatorForTarget"), T("Could not find evaluator for target ") + targetName);
    return EvaluatorPtr();
  }
  
  virtual double getDefaultScore() const
    {return 0.0;}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
  {
    for (size_t i = 0; i < evaluators.size(); ++i)
    {
      EvaluatorPtr evaluator = evaluators[i].second;
      std::vector<std::pair<String, double> > scores;
      evaluator->getScores(scores);
      
      for (size_t j = 0; j < scores.size(); ++j)
        res.push_back(std::make_pair(evaluator->getName() + T("[") + scores[j].first + T("]"), scores[j].second));
    }
  }
  
  void getScoresForTarget(const String& targetName, std::vector< std::pair<String, double> >& res) const
  {
    EvaluatorPtr evaluator = getEvaluatorForTarget(targetName);
    jassert(evaluator);
    evaluator->getScores(res);
  }

protected:
  size_t numProteins;

  std::vector<std::pair<size_t, EvaluatorPtr> > evaluators;

  void addEvaluator(const String& variableName, EvaluatorPtr evaluator)
  {
    int variableIndex = proteinClass->findObjectVariable(variableName);
    jassert(variableIndex >= 0);
    evaluators.push_back(std::make_pair(variableIndex, evaluator));
  }
};

typedef ReferenceCountedObjectPtr<ProteinEvaluator> ProteinEvaluatorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_H_
