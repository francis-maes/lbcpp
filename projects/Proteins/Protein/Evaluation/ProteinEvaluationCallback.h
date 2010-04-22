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

namespace lbcpp
{

class ProteinEvaluationCallback : public InferenceCallback
{
public:
  virtual void startInferencesCallback(size_t count)
  {
    numProteins = 0;
    pssmAbsoluteError = new ScalarVariableMean(T("PSSM"));
    pssmSquaredError = new ScalarVariableMean(T("PSSM^2"));
    secondaryStructureAccuracy = new ScalarVariableMean(T("SS3"));
    dsspSecondaryStructureAccuracy = new ScalarVariableMean(T("SS8"));
    solventAccesibility2StateAccuracy = new ScalarVariableMean(T("SA2"));
  }

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
    {
      // top-level inference is finished
      ProteinPtr p1 = output.dynamicCast<Protein>();
      ProteinPtr p2 = supervision.dynamicCast<Protein>();
      jassert(p1 && p2);
      addProtein(p1, p2);
    }
  }

  virtual String toString() const
  {
    String res;
    res += lbcpp::toString(numProteins) + T(" proteins");
    res += "\n";
    if (numProteins)
    {
      if (pssmSquaredError->getMean())
        res += T("PSSM: mean abs. error = ") + String(pssmAbsoluteError->getMean()) +
               T(" rmse = ") + String(sqrt(pssmSquaredError->getMean())) + T("\n");
      res += scoreToString(secondaryStructureAccuracy);
      res += scoreToString(dsspSecondaryStructureAccuracy);
      res += scoreToString(solventAccesibility2StateAccuracy);
    }
    return res;
  }

  static String scoreToString(ScalarVariableMeanPtr score)
  {
    String name = score->getName();
    double count = score->getCount();
    if (count)
      return name + T(": ") + String(score->getMean() * 100.0, 2) + T(" (") + lbcpp::toString(count) + T(" elements)\n");
    else
      return name + T(": N/A\n");
  }

  double getQ3Score() const
    {return secondaryStructureAccuracy->getMean();}

  double getQ8Score() const
    {return dsspSecondaryStructureAccuracy->getMean();}

  double getSA2Score() const
    {return solventAccesibility2StateAccuracy->getMean();}

  double getPSSMRootMeanSquareError() const
    {return sqrt(pssmSquaredError->getMean());}

  void addProtein(ProteinPtr predicted, ProteinPtr correct)
  {
    ++numProteins;
    addScoreVectorSequence(predicted->getPositionSpecificScoringMatrix(), correct->getPositionSpecificScoringMatrix(), pssmAbsoluteError, pssmSquaredError);
    addLabelSequence(predicted->getSecondaryStructureSequence(), correct->getSecondaryStructureSequence(), secondaryStructureAccuracy);
    addLabelSequence(predicted->getDSSPSecondaryStructureSequence(), correct->getDSSPSecondaryStructureSequence(), dsspSecondaryStructureAccuracy);
    addLabelSequence(predicted->getSolventAccessibilitySequence(), correct->getSolventAccessibilitySequence(), solventAccesibility2StateAccuracy);
  }

  void addLabelSequence(LabelSequencePtr predicted, LabelSequencePtr correct, ScalarVariableMeanPtr statistics)
  {
    if (!correct || !predicted)
      return;
    jassert(correct->getDictionary() == predicted->getDictionary());

    size_t n = predicted->size();
    jassert(correct->size() == n);
    for (size_t i = 0; i < n; ++i)
    {
      LabelPtr correctLabel = correct->get(i);
      LabelPtr predictedLabel = predicted->get(i);
      if (correctLabel)
        statistics->push(predictedLabel && correctLabel->getIndex() == predictedLabel->getIndex() ? 1.0 : 0.0);
    }
  }

  void addScoreVectorSequence(ScoreVectorSequencePtr predicted, ScoreVectorSequencePtr correct, ScalarVariableMeanPtr absoluteError, ScalarVariableMeanPtr squaredError)
  {
    if (!correct || !predicted)
      return;

    jassert(correct->getNumScores() >= predicted->getNumScores());
    jassert(correct->getDictionary() == predicted->getDictionary());

    size_t n = predicted->size();
    size_t s = predicted->getNumScores();
    jassert(correct->size() == n);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = 0; j < s; ++j)
      {
        double delta = predicted->getScore(i, j) - correct->getScore(i, j);
        //std::cout << "Predicted = " << predicted->getScore(i, j) << " Correct = " << correct->getScore(i, j) << " Delta = " << delta << std::endl;
        absoluteError->push(fabs(delta));
        squaredError->push(delta * delta);
      }
  }

protected:
  size_t numProteins;
  ScalarVariableMeanPtr pssmAbsoluteError;
  ScalarVariableMeanPtr pssmSquaredError;

  ScalarVariableMeanPtr secondaryStructureAccuracy;
  ScalarVariableMeanPtr dsspSecondaryStructureAccuracy;
  ScalarVariableMeanPtr solventAccesibility2StateAccuracy;
};

typedef ReferenceCountedObjectPtr<ProteinEvaluationCallback> ProteinEvaluationCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_EVALUATION_H_
