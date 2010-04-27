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

namespace lbcpp
{

class Evaluator : public NameableObject
{
public:
  Evaluator(const String& name) : NameableObject(name) {}
  Evaluator() {}

  virtual void addPrediction(ObjectPtr predicted, ObjectPtr correct) = 0;
  virtual double getDefaultScore() const = 0;
};

typedef ReferenceCountedObjectPtr<Evaluator> EvaluatorPtr;

class ClassificationEvaluator : public Evaluator
{
public:
  ClassificationEvaluator(const String& name) : Evaluator(name), accuracy(new ScalarVariableMean()) {}
  ClassificationEvaluator() {}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    LabelPtr predicted = predictedObject.dynamicCast<Label>();
    LabelPtr correct = correctObject.dynamicCast<Label>();
    if (!predicted || !correct)
      return;
    jassert(predicted->getDictionary() == correct->getDictionary());
    accuracy->push(predicted->getIndex() == correct->getIndex() ? 1.0 : 0.0);
  }
  
  virtual String toString() const
  {
    double count = accuracy->getCount();
    if (!count)
      return String::empty;
    return getName() + T(": ") + String(accuracy->getMean() * 100.0, 2) + T("% (") + lbcpp::toString(count) + T(" examples)");
  }

  virtual double getDefaultScore() const
    {return accuracy->getMean();}

protected:
  ScalarVariableMeanPtr accuracy;
};

typedef ReferenceCountedObjectPtr<ClassificationEvaluator> ClassificationEvaluatorPtr;

class SequenceLabelingEvaluator : public Evaluator
{
public:
  SequenceLabelingEvaluator(const String& name)
    : Evaluator(name), classificationEvaluator(new ClassificationEvaluator(name)) {}

  virtual String toString() const
    {return classificationEvaluator->toString();}

  virtual double getDefaultScore() const
    {return classificationEvaluator->getDefaultScore();}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    LabelSequencePtr predicted = predictedObject.dynamicCast<LabelSequence>();
    LabelSequencePtr correct = correctObject.dynamicCast<LabelSequence>();
    if (!predicted || !correct)
      return;
    jassert(correct->getDictionary() == predicted->getDictionary());

    size_t n = predicted->size();
    jassert(correct->size() == n);
    for (size_t i = 0; i < n; ++i)
      classificationEvaluator->addPrediction(predicted->get(i), correct->get(i));
  }

protected:
  ClassificationEvaluatorPtr classificationEvaluator;
};

typedef ReferenceCountedObjectPtr<SequenceLabelingEvaluator> SequenceLabelingEvaluatorPtr;

class RegressionEvaluator : public Evaluator
{
public:
  RegressionEvaluator(const String& name) : Evaluator(name),
    absoluteError(new ScalarVariableMean()), squaredError(new ScalarVariableMean()) {}
  RegressionEvaluator() {}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    ScalarPtr predicted = predictedObject.dynamicCast<Scalar>();
    ScalarPtr correct = correctObject.dynamicCast<Scalar>();
    if (predicted && correct)
      addDelta(predicted->getValue() - correct->getValue());
  }

  virtual void addDelta(double delta)
  {
    absoluteError->push(fabs(delta));
    squaredError->push(delta * delta);
  }

  virtual String toString() const
  {
    double count = squaredError->getCount();
    if (!count)
      return String::empty;
    return getName() + T(": rmse = ") + String(getRMSE(), 4)
        + T(" abs = ") + String(absoluteError->getMean(), 4)
        + T(" (") + lbcpp::toString(count) + T(" examples)");
  }

  virtual double getDefaultScore() const
    {return -getRMSE();}

  double getRMSE() const
    {return sqrt(squaredError->getMean());}

protected:
  ScalarVariableMeanPtr absoluteError;
  ScalarVariableMeanPtr squaredError;
};

typedef ReferenceCountedObjectPtr<RegressionEvaluator> RegressionEvaluatorPtr;

class DihedralAngleRegressionEvaluator : public RegressionEvaluator
{
public:
  DihedralAngleRegressionEvaluator(const String& name) : RegressionEvaluator(name) {}

  virtual void addDelta(double delta)
    {RegressionEvaluator::addDelta(DihedralAngle::normalize(delta));}
};

class ScoreVectorSequenceRegressionEvaluator : public Evaluator
{
public:
  ScoreVectorSequenceRegressionEvaluator(const String& name)
    : Evaluator(name), regressionEvaluator(new RegressionEvaluator(name)) {}

  virtual String toString() const
    {return regressionEvaluator->toString();}

  virtual double getDefaultScore() const
    {return regressionEvaluator->getDefaultScore();}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    ScoreVectorSequencePtr predicted = predictedObject.dynamicCast<ScoreVectorSequence>();
    ScoreVectorSequencePtr correct = correctObject.dynamicCast<ScoreVectorSequence>();
    if (!predicted || !correct)
      return;

    jassert(correct->getNumScores() >= predicted->getNumScores());
    jassert(correct->getDictionary() == predicted->getDictionary());
    size_t n = predicted->size();
    size_t s = predicted->getNumScores();
    jassert(correct->size() == n);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = 0; j < s; ++j)
        regressionEvaluator->addPrediction(new Scalar(predicted->getScore(i, j)), new Scalar(correct->getScore(i, j)));
  }

protected:
  RegressionEvaluatorPtr regressionEvaluator;
};

typedef ReferenceCountedObjectPtr<ScoreVectorSequenceRegressionEvaluator> ScoreVectorSequenceRegressionEvaluatorPtr;

class ProteinBackboneBondSequenceEvaluator : public Evaluator
{
public:
  ProteinBackboneBondSequenceEvaluator(const String& name)
    : Evaluator(name),
      lengthEvaluator(new RegressionEvaluator(name + T(" length"))),
      angleEvaluator(new RegressionEvaluator(name + T(" angle"))),
      phiEvaluator(new DihedralAngleRegressionEvaluator(name + T(" phi"))),
      psiEvaluator(new DihedralAngleRegressionEvaluator(name + T(" psi"))),
      omegaEvaluator(new DihedralAngleRegressionEvaluator(name + T(" omega"))) {}


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
  RegressionEvaluatorPtr lengthEvaluator;
  RegressionEvaluatorPtr angleEvaluator;

  RegressionEvaluatorPtr phiEvaluator;
  RegressionEvaluatorPtr psiEvaluator;
  RegressionEvaluatorPtr omegaEvaluator;
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

class ProteinEvaluator : public Evaluator
{
public:
  ProteinEvaluator() :
    Evaluator(T("Protein")), numProteins(0),
    pssmEvaluator(new ScoreVectorSequenceRegressionEvaluator(T("PSSM"))),
    secondaryStructureEvaluator(new SequenceLabelingEvaluator(T("SS3"))),
    dsspSecondaryStructureEvaluator(new SequenceLabelingEvaluator(T("SS8"))),
    solventAccesibility2StateEvaluator(new SequenceLabelingEvaluator(T("SA2"))),
    disorderEvaluator(new SequenceLabelingEvaluator(T("DR"))),
    backboneBondEvaluator(new ProteinBackboneBondSequenceEvaluator(T("BBB"))),
    tertiaryStructureEvaluator(new ProteinTertiaryStructureEvaluator(T("TS"))) {}

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

    ProteinTertiaryStructurePtr tertiaryStructure = predicted->getTertiaryStructure();
    if (!tertiaryStructure)
    {
      ProteinBackboneBondSequencePtr backbone = predicted->getBackboneBondSequence();
      if (backbone)
        tertiaryStructure = ProteinTertiaryStructure::createFromBackbone(predicted->getAminoAcidSequence(), backbone);
    }
    tertiaryStructureEvaluator->addPrediction(tertiaryStructure, correct->getTertiaryStructure());
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
    return EvaluatorPtr();
  }
  
  virtual double getDefaultScore() const
    {return tertiaryStructureEvaluator->getDefaultScore();}

protected:
  size_t numProteins;

  ScoreVectorSequenceRegressionEvaluatorPtr pssmEvaluator;
  SequenceLabelingEvaluatorPtr secondaryStructureEvaluator;
  SequenceLabelingEvaluatorPtr dsspSecondaryStructureEvaluator;
  SequenceLabelingEvaluatorPtr solventAccesibility2StateEvaluator;
  SequenceLabelingEvaluatorPtr disorderEvaluator;
  EvaluatorPtr backboneBondEvaluator;
  ProteinTertiaryStructureEvaluatorPtr tertiaryStructureEvaluator;

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
