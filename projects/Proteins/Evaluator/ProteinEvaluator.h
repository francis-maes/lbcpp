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
    for (std::map<String, std::pair<size_t, EvaluatorPtr> >::const_iterator it = evaluators.begin(); it != evaluators.end(); ++it)
    {
      String str = it->second.second->toString();
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
    for (std::map<String, std::pair<size_t, EvaluatorPtr> >::iterator it = evaluators.begin(); it != evaluators.end(); ++it)
    {
      size_t variableIndex = it->second.first;
      Variable predictedVariable = predicted->getVariable(variableIndex);
      if (predictedVariable.exists())
      {
        Variable correctVariable = correct->getTargetOrComputeIfMissing(variableIndex);
        it->second.second->addPrediction(predictedVariable, correctVariable);
      }
    }
  }

  EvaluatorPtr getEvaluatorForTarget(const String& targetName) const
  {
    if (!evaluators.count(targetName))
    {
      MessageCallback::error(T("ProteinEvaluator::getEvaluatorForTarget"), T("Unknown target ") + targetName);
      return EvaluatorPtr();
    }
    return evaluators.find(targetName)->second.second;
  }
  
  virtual double getDefaultScore() const
    {return 0.0;}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
  {
    for (std::map<String, std::pair<size_t, EvaluatorPtr> >::const_iterator it = evaluators.begin(); it != evaluators.end(); ++it)
    {
      EvaluatorPtr eval = it->second.second;
      std::vector<std::pair<String, double> > scores;
      eval->getScores(scores);
      
      for (size_t i = 0; i < scores.size(); ++i)
        res.push_back(std::make_pair(it->first + T("[") + scores[i].first + T("]"), scores[i].second));
    }
  }

protected:
  size_t numProteins;

  std::map<String, std::pair<size_t, EvaluatorPtr> > evaluators;

  void addEvaluator(const String& variableName, EvaluatorPtr evaluator)
  {
    int variableIndex = proteinClass->findObjectVariable(variableName);
    jassert(variableIndex >= 0);
    evaluators[variableName] = std::make_pair(variableIndex, evaluator);
  }
};

typedef ReferenceCountedObjectPtr<ProteinEvaluator> ProteinEvaluatorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_H_
