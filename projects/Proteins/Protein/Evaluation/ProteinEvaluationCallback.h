/*-----------------------------------------.---------------------------------.
| Filename: ProteinEvaluationCallback.h     | A callback that evaluates the  |
| Author  : Francis Maes                   |  quality of predicted proteins  |
| Started : 09/04/2010 15:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_EVALUATION_H_
# define LBCPP_PROTEIN_INFERENCE_EVALUATION_H_

# include "../Protein.h"
# include "../../InferenceContext/InferenceCallback.h"
# include "../../InferenceContext/InferenceStack.h"

# include "../../Evaluator/RegressionErrorEvaluator.h"

namespace lbcpp
{

class ProteinBackboneBondSequenceEvaluator : public Evaluator
{
public:
  ProteinBackboneBondSequenceEvaluator(const String& name)
    : Evaluator(name),
      lengthEvaluator(new RegressionErrorEvaluator(name + T(" length"))),
      angleEvaluator(new RegressionErrorEvaluator(name + T(" angle"))),
      phiEvaluator(new DihedralAngleRegressionErrorEvaluator(name + T(" phi"))),
      psiEvaluator(new DihedralAngleRegressionErrorEvaluator(name + T(" psi"))),
      omegaEvaluator(new DihedralAngleRegressionErrorEvaluator(name + T(" omega"))) {}


  virtual String toString() const
  {
    if (lengthEvaluator->getRMSE() == 0.0)
      return String::empty;
    return T("Backbone length: ") + String(lengthEvaluator->getRMSE(), 4)
      + T(" angle: ") + String(angleEvaluator->getRMSE(), 4)
      + T(" phi: ") + String(phiEvaluator->getRMSE(), 4)
      + T(" psi: ") + String(psiEvaluator->getRMSE(), 4)
      + T(" omega: ") + String(omegaEvaluator->getRMSE(), 4);
  }

  virtual double getDefaultScore() const
    {return lengthEvaluator->getDefaultScore() + angleEvaluator->getDefaultScore() +
      ((phiEvaluator->getDefaultScore() + psiEvaluator->getDefaultScore() + omegaEvaluator->getDefaultScore()) / 3.0);}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    ProteinBackboneBondSequencePtr predicted = predictedObject.dynamicCast<ProteinBackboneBondSequence>();
    ProteinBackboneBondSequencePtr correct = correctObject.dynamicCast<ProteinBackboneBondSequence>();
    if (!predicted || !correct)
      return;

    size_t n = predicted->size();
    jassert(n == correct->size());
    for (size_t i = 0; i < n; ++i)
    {
      ProteinBackboneBondPtr p = predicted->getBond(i);
      ProteinBackboneBondPtr c = correct->getBond(i);
      if (p && c)
      {
        addBondPrediction(p->getBond1(), c->getBond1());
        addBondPrediction(p->getBond2(), c->getBond2());
        addBondPrediction(p->getBond3(), c->getBond3());
      }

      if (p->getPhiAngle().exists() && c->getPhiAngle().exists())
      {
        double delta = (double)p->getPhiAngle() - (double)c->getPhiAngle();
        phiEvaluator->addDelta(delta);
      }
      if (p->getPsiAngle().exists() && c->getPsiAngle().exists())
      {
        double delta = (double)p->getPsiAngle() - (double)c->getPsiAngle();
        psiEvaluator->addDelta(delta);
      }
      if (p->getOmegaAngle().exists() && c->getOmegaAngle().exists())
      {
        double delta = (double)p->getOmegaAngle() - (double)c->getOmegaAngle();
        omegaEvaluator->addDelta(delta);
      }
    }
  }

  void addBondPrediction(const BondCoordinates& predicted, const BondCoordinates& correct)
  {
    if (predicted.hasLength() && correct.hasLength())
      lengthEvaluator->addDelta(predicted.getLength() - correct.getLength());
    if (predicted.hasThetaAngle() && correct.hasThetaAngle())
      angleEvaluator->addDelta((double)predicted.getThetaAngle() - (double)correct.getThetaAngle());
  }

private:
  RegressionErrorEvaluatorPtr lengthEvaluator;
  RegressionErrorEvaluatorPtr angleEvaluator;

  RegressionErrorEvaluatorPtr phiEvaluator;
  RegressionErrorEvaluatorPtr psiEvaluator;
  RegressionErrorEvaluatorPtr omegaEvaluator;
};

class ProteinTertiaryStructureEvaluator : public Evaluator
{
public:
  ProteinTertiaryStructureEvaluator(const String& name) : Evaluator(name), calphaRMSE(new ScalarVariableMean()) {}

  virtual String toString() const
  {
    double count = calphaRMSE->getCount();
    if (!count)
      return String::empty;
    return T("C-Alpha RMSE = ") + String(calphaRMSE->getMean(), 4);
  }

  virtual double getDefaultScore() const
    {return -calphaRMSE->getMean();}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    ProteinTertiaryStructurePtr predicted = predictedObject.dynamicCast<ProteinTertiaryStructure>();
    ProteinTertiaryStructurePtr correct = correctObject.dynamicCast<ProteinTertiaryStructure>();
    if (!correct || !predicted)
      return;
    jassert(correct->size() == predicted->size());
    calphaRMSE->push(predicted->computeCAlphaAtomsRMSE(correct));
  }

protected:
  ScalarVariableMeanPtr calphaRMSE;
};

typedef ReferenceCountedObjectPtr<ProteinTertiaryStructureEvaluator> ProteinTertiaryStructureEvaluatorPtr;

class ProteinContactMapEvaluator : public Evaluator
{
public:
  ProteinContactMapEvaluator(const String& name)
    : Evaluator(name), classificationEvaluator(binaryClassificationConfusionEvaluator(name)) {}

  virtual String toString() const
    {return classificationEvaluator->toString();}

  virtual double getDefaultScore() const
    {return classificationEvaluator->getDefaultScore();}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    ScoreSymmetricMatrixPtr predicted = predictedObject.dynamicCast<ScoreSymmetricMatrix>();
    ScoreSymmetricMatrixPtr correct = correctObject.dynamicCast<ScoreSymmetricMatrix>();
    if (!predicted || !correct)
      return;

    jassert(predicted->getDimension() == predicted->getDimension());
    size_t n = predicted->getDimension();
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i; j < n; ++j)
      {
        if (correct->hasScore(i, j) && predicted->hasScore(i, j))
          classificationEvaluator->addPrediction(
            new Label(BinaryClassificationDictionary::getInstance(), predicted->getScore(i, j) > 0.5),
            new Label(BinaryClassificationDictionary::getInstance(), correct->getScore(i, j) > 0.5));
      }
  }

protected:
  EvaluatorPtr classificationEvaluator;
};


class ProteinEvaluator : public Evaluator
{
public:
  ProteinEvaluator() :
    Evaluator(T("Protein")), numProteins(0),
    pssmEvaluator(scoreVectorSequenceRegressionErrorEvaluator(T("PSSM"))),
    secondaryStructureEvaluator(sequenceLabelingAccuracyEvaluator(T("SS3"))),
    dsspSecondaryStructureEvaluator(sequenceLabelingAccuracyEvaluator(T("SS8"))),
    solventAccesibility2StateEvaluator(sequenceLabelingAccuracyEvaluator(T("SA2"))),
    disorderEvaluator(binarySequenceLabelingConfusionEvaluator(T("DR"))),
    backboneBondEvaluator(new ProteinBackboneBondSequenceEvaluator(T("BBB"))),
    tertiaryStructureEvaluator(new ProteinTertiaryStructureEvaluator(T("TS"))),
    contactMapEvaluator(new ProteinContactMapEvaluator(T("RR")))    
    {}

  virtual String toString() const
  {
    String res;
    res += lbcpp::toString(numProteins) + T(" proteins");
    res += "\n";
    if (pssmEvaluator->getDefaultScore())
      evaluatorToString(res, pssmEvaluator);
    evaluatorToString(res, secondaryStructureEvaluator);
    evaluatorToString(res, dsspSecondaryStructureEvaluator);
    evaluatorToString(res, solventAccesibility2StateEvaluator);
    evaluatorToString(res, disorderEvaluator);
    evaluatorToString(res, backboneBondEvaluator);
    evaluatorToString(res, tertiaryStructureEvaluator);
    evaluatorToString(res, contactMapEvaluator);
    return res;
  }

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    ProteinPtr predicted = predictedObject.dynamicCast<Protein>();
    ProteinPtr correct = correctObject.dynamicCast<Protein>();
    if (!predicted || !correct)
      return;

    ++numProteins;
    predicted->computeMissingFields();

    pssmEvaluator->addPrediction(predicted->getPositionSpecificScoringMatrix(), correct->getPositionSpecificScoringMatrix());
    secondaryStructureEvaluator->addPrediction(predicted->getSecondaryStructureSequence(), correct->getSecondaryStructureSequence());
    dsspSecondaryStructureEvaluator->addPrediction(predicted->getDSSPSecondaryStructureSequence(), correct->getDSSPSecondaryStructureSequence());
    solventAccesibility2StateEvaluator->addPrediction(predicted->getSolventAccessibilityThreshold20(), correct->getSolventAccessibilityThreshold20());
    disorderEvaluator->addPrediction(predicted->getDisorderSequence(), correct->getDisorderSequence());
    backboneBondEvaluator->addPrediction(predicted->getBackboneBondSequence(), correct->getBackboneBondSequence());
    tertiaryStructureEvaluator->addPrediction(predicted->getTertiaryStructure(), correct->getTertiaryStructure());
    contactMapEvaluator->addPrediction(predicted->getResidueResidueContactMatrix8Cb(), correct->getResidueResidueContactMatrix8Cb());
  }

  EvaluatorPtr getEvaluatorForTarget(const String& targetName)
  {
    if (targetName == T("PositionSpecificScoringMatrix"))
      return pssmEvaluator;
    if (targetName == T("SecondaryStructureSequence"))
      return secondaryStructureEvaluator;
    if (targetName == T("DSSPSecondaryStructureSequence"))
      return dsspSecondaryStructureEvaluator;
    if (targetName == T("SolventAccessibilityThreshold20"))
      return solventAccesibility2StateEvaluator;
    if (targetName == T("DisorderSequence") || targetName == T("DisorderProbabilitySequence"))
      return disorderEvaluator;
    if (targetName ==  T("BackboneBondSequence"))
      return backboneBondEvaluator;
    if (targetName == T("TertiaryStructure"))
      return tertiaryStructureEvaluator;
    if (targetName == T("ResidueResidueContactMatrix8Cb"))
      return contactMapEvaluator;
    return EvaluatorPtr();
  }
  
  virtual double getDefaultScore() const
    {return tertiaryStructureEvaluator->getDefaultScore();}

protected:
  size_t numProteins;

  EvaluatorPtr pssmEvaluator;
  EvaluatorPtr secondaryStructureEvaluator;
  EvaluatorPtr dsspSecondaryStructureEvaluator;
  EvaluatorPtr solventAccesibility2StateEvaluator;
  EvaluatorPtr disorderEvaluator;
  EvaluatorPtr backboneBondEvaluator;
  EvaluatorPtr tertiaryStructureEvaluator;
  EvaluatorPtr contactMapEvaluator;

  static void evaluatorToString(String& res, EvaluatorPtr evaluator)
  {
    String str = evaluator->toString();
    if (str.isNotEmpty())
      res += str + T("\n");
  }
};

typedef ReferenceCountedObjectPtr<ProteinEvaluator> ProteinEvaluatorPtr;

class ProteinEvaluationCallback : public InferenceCallback
{
public:
  virtual void startInferencesCallback(size_t count)
    {evaluator = new ProteinEvaluator();}

  virtual String toString() const
    {return evaluator->toString();}

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
    {
      // top-level inference is finished
      jassert(output.dynamicCast<Protein>() && supervision.dynamicCast<Protein>());
      evaluator->addPrediction(output, supervision);
    }
  }

  double getDefaultScoreForTarget(const String& targetName)
  {
    EvaluatorPtr targetEvaluator = evaluator->getEvaluatorForTarget(targetName);
    return targetEvaluator ? targetEvaluator->getDefaultScore() : 0.0;
  }

protected:
  ProteinEvaluatorPtr evaluator;
};

typedef ReferenceCountedObjectPtr<ProteinEvaluationCallback> ProteinEvaluationCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_EVALUATION_H_
