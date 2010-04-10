/*-----------------------------------------.---------------------------------.
| Filename: ProteinEvaluationCallback.h     | A callback that evaluates the  |
| Author  : Francis Maes                   |  quality of predicted proteins  |
| Started : 09/04/2010 15:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_EVALUATION_H_
# define LBCPP_PROTEIN_INFERENCE_EVALUATION_H_

# include "../InferenceCallback/InferenceCallback.h"

namespace lbcpp
{

class ProteinEvaluationCallback : public InferenceCallback
{
public:
  virtual void startInferencesCallback(size_t count)
  {
    numProteins = numUnpredictedProteins = 0;
    secondaryStructureAccuracy = new ScalarVariableMean(T("Q3"));
    dsspSecondaryStructureAccuracy = new ScalarVariableMean(T("Q8"));
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
    if (numUnpredictedProteins)
      res += T(" (") + lbcpp::toString(numUnpredictedProteins) + T(" unpredicted proteins)");
    res += "\n";
    if (numProteins)
    {
      res += T("Q3: ") + String(getQ3Score() * 100.0, 2) + T("\n");
      res += T("Q8: ") + String(getQ8Score() * 100.0, 2) + T("\n");
    }
    return res;
  }

  double getQ3Score() const
    {return secondaryStructureAccuracy->getMean();}

  double getQ8Score() const
    {return dsspSecondaryStructureAccuracy->getMean();}

  void addProtein(ProteinPtr predicted, ProteinPtr correct)
  {
    ++numProteins;
    addLabelSequence(predicted->getSecondaryStructureSequence(), correct->getSecondaryStructureSequence(), secondaryStructureAccuracy);
    addLabelSequence(predicted->getSecondaryStructureSequence(true), correct->getSecondaryStructureSequence(true), dsspSecondaryStructureAccuracy);
  }

  void addLabelSequence(LabelSequencePtr predicted, LabelSequencePtr correct, ScalarVariableMeanPtr statistics)
  {
    if (!correct)
      return;
    if (!predicted)
    {
      ++numUnpredictedProteins;
      return;
    }
    size_t n = predicted->getLength();
    jassert(correct->getLength() == n);
    for (size_t i = 0; i < n; ++i)
    {
      size_t correctLabel;
      size_t predictedLabel;
      if (correct->getVariable(i, correctLabel))
        statistics->push((predicted->getVariable(i, predictedLabel) && predictedLabel == correctLabel) ? 1.0 : 0.0);
    }
  }

protected:
  size_t numProteins;
  size_t numUnpredictedProteins;
  ScalarVariableMeanPtr secondaryStructureAccuracy;
  ScalarVariableMeanPtr dsspSecondaryStructureAccuracy;
};

typedef ReferenceCountedObjectPtr<ProteinEvaluationCallback> ProteinEvaluationCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_EVALUATION_H_
