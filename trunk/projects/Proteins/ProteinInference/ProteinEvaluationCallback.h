/*-----------------------------------------.---------------------------------.
| Filename: ProteinEvaluationCallback.h     | A callback that evaluates the  |
| Author  : Francis Maes                   |  quality of predicted proteins  |
| Started : 09/04/2010 15:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_EVALUATION_H_
# define LBCPP_PROTEIN_INFERENCE_EVALUATION_H_

# include "Protein.h"
# include "../InferenceCallback/InferenceCallback.h"

namespace lbcpp
{

class ProteinEvaluationCallback : public InferenceCallback
{
public:
  virtual void startInferencesCallback(size_t count)
  {
    numProteins = 0;
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

  void addProtein(ProteinPtr predicted, ProteinPtr correct)
  {
    ++numProteins;
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

protected:
  size_t numProteins;
  ScalarVariableMeanPtr secondaryStructureAccuracy;
  ScalarVariableMeanPtr dsspSecondaryStructureAccuracy;
  ScalarVariableMeanPtr solventAccesibility2StateAccuracy;
};

typedef ReferenceCountedObjectPtr<ProteinEvaluationCallback> ProteinEvaluationCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_EVALUATION_H_
