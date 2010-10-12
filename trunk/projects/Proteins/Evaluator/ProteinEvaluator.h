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
  ProteinEvaluator() :
    Evaluator(T("Protein")), numProteins(0),
    //pssmEvaluator(scoreVectorSequenceRegressionErrorEvaluator(T("PSSM"))),
    secondaryStructureEvaluator(sequenceLabelingAccuracyEvaluator(T("SS3"))),
    dsspSecondaryStructureEvaluator(sequenceLabelingAccuracyEvaluator(T("SS8"))),
    solventAccessibility2StateEvaluator(binarySequenceLabelingConfusionEvaluator(T("SA2"))),
    disorderEvaluator(binarySequenceLabelingConfusionEvaluator(T("DR"))),
    //calphaBondsEvaluator(new BondCoordinatesSequenceEvaluator(T("CAB"))),
    //backboneBondEvaluator(new ProteinBackboneBondSequenceEvaluator(T("BBB"))),
    tertiaryStructureEvaluator(new TertiaryStructureEvaluator(T("TS"))),
    calphaContactMapEvaluator(new ContactMapEvaluator(T("RRa"))),
    cbetaContactMapEvaluator(new ContactMapEvaluator(T("RRb"))),
    structuralAlphabetEvaluator(sequenceLabelingAccuracyEvaluator(T("StAl")))
    {}

  virtual String toString() const
  {
    String res;
    res += String((int)numProteins) + T(" proteins");
    res += "\n";
    //if (pssmEvaluator->getDefaultScore())
    //  evaluatorToString(res, pssmEvaluator);
    evaluatorToString(res, secondaryStructureEvaluator);
    evaluatorToString(res, dsspSecondaryStructureEvaluator);
    evaluatorToString(res, solventAccessibility2StateEvaluator);
    evaluatorToString(res, disorderEvaluator);
    //evaluatorToString(res, calphaBondsEvaluator);
    //evaluatorToString(res, backboneBondEvaluator);
    evaluatorToString(res, tertiaryStructureEvaluator);
    evaluatorToString(res, calphaContactMapEvaluator);
    evaluatorToString(res, cbetaContactMapEvaluator);
    evaluatorToString(res, structuralAlphabetEvaluator);
    return res;
  }

  virtual void addPrediction(const Variable& predictedObject, const Variable& correctObject)
  {
    if (!correctObject.exists() || !predictedObject.exists())
      return;

    const ProteinPtr& predicted = predictedObject.getObjectAndCast<Protein>();
    const ProteinPtr& correct = correctObject.getObjectAndCast<Protein>();

    ++numProteins;
    predicted->computeMissingVariables();

    //pssmEvaluator->addPrediction(predicted->getPositionSpecificScoringMatrix(), correct->getPositionSpecificScoringMatrix());
    secondaryStructureEvaluator->addPrediction(predicted->getSecondaryStructure(), correct->getSecondaryStructure());
    dsspSecondaryStructureEvaluator->addPrediction(predicted->getDSSPSecondaryStructure(), correct->getDSSPSecondaryStructure());
    solventAccessibility2StateEvaluator->addPrediction(predicted->getSolventAccessibilityAt20p(), correct->getSolventAccessibilityAt20p());
    disorderEvaluator->addPrediction(predicted->getDisorderRegions(), correct->getDisorderRegions());
    //calphaBondsEvaluator->addPrediction(predicted->getCAlphaBondSequence(), correct->getCAlphaBondSequence());
    //backboneBondEvaluator->addPrediction(predicted->getBackboneBondSequence(), correct->getBackboneBondSequence());
    tertiaryStructureEvaluator->addPrediction(predicted->getTertiaryStructure(), correct->getTertiaryStructure());
    calphaContactMapEvaluator->addPrediction(predicted->getContactMap(8, false), correct->getContactMap(8, false));
    cbetaContactMapEvaluator->addPrediction(predicted->getContactMap(8, true), correct->getContactMap(8, true));
    structuralAlphabetEvaluator->addPrediction(predicted->getStructuralAlphabetSequence(), correct->getStructuralAlphabetSequence());
  }

  EvaluatorPtr getEvaluatorForTarget(const String& targetName)
  {
    //if (targetName == T("positionSpecificScoringMatrix"))
    //  return pssmEvaluator;
    if (targetName == T("secondaryStructure"))
      return secondaryStructureEvaluator;
    if (targetName == T("dsspSecondaryStructure"))
      return dsspSecondaryStructureEvaluator;
    if (targetName == T("solventAccessibilityAt20p"))
      return solventAccessibility2StateEvaluator;
    if (targetName == T("disorderRegions"))
      return disorderEvaluator;
 /*   if (targetName == T("CAlphaBondSequence"))
      return calphaBondsEvaluator;
    if (targetName ==  T("BackboneBondSequence"))
      return backboneBondEvaluator;*/
    if (targetName == T("tertiaryStructure"))
      return tertiaryStructureEvaluator;
    if (targetName == T("contactMap8Ca"))
      return calphaContactMapEvaluator;
    if (targetName == T("contactMap8Cb"))
      return cbetaContactMapEvaluator;
    if (targetName == T("structuralAlphabetSequence"))
      return structuralAlphabetEvaluator;
    jassert(false);
    std::cerr << "ProteinEvaluator::getEvaluatorForTarget(" + targetName + ")" << std::endl;
    return EvaluatorPtr();
  }
  
  virtual double getDefaultScore() const
    {return tertiaryStructureEvaluator->getDefaultScore();}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {} // FIXME

protected:
  size_t numProteins;

  //EvaluatorPtr pssmEvaluator;
  EvaluatorPtr secondaryStructureEvaluator;
  EvaluatorPtr dsspSecondaryStructureEvaluator;
  EvaluatorPtr solventAccessibility2StateEvaluator;
  EvaluatorPtr disorderEvaluator;
  //EvaluatorPtr calphaBondsEvaluator;
  //EvaluatorPtr backboneBondEvaluator;
  EvaluatorPtr tertiaryStructureEvaluator;
  EvaluatorPtr calphaContactMapEvaluator;
  EvaluatorPtr cbetaContactMapEvaluator;
  EvaluatorPtr structuralAlphabetEvaluator;

  static void evaluatorToString(String& res, EvaluatorPtr evaluator)
  {
    String str = evaluator->toString();
    if (str.isNotEmpty())
      res += str + T("\n");
  }
};

typedef ReferenceCountedObjectPtr<ProteinEvaluator> ProteinEvaluatorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EVALUATOR_H_
