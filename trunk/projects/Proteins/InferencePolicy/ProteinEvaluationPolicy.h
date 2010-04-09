/*-----------------------------------------.---------------------------------.
| Filename: ProteinEvaluationPolicy.h      | A policy that evaluates the     |
| Author  : Francis Maes                   |  quality of predicted proteins  |
| Started : 09/04/2010 15:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_POLICY_PROTEIN_EVALUATION_H_
# define LBCPP_INFERENCE_POLICY_PROTEIN_EVALUATION_H_

# include "DecoratorInferencePolicy.h"

namespace lbcpp
{

class ProteinEvaluationPolicy : public DecoratorInferencePolicy
{
public:
  ProteinEvaluationPolicy(InferencePolicyPtr targetPolicy)
    : DecoratorInferencePolicy(targetPolicy) {}

  virtual ObjectContainerPtr supervisedExampleSetPreCallback(InferenceStepPtr inference, ObjectContainerPtr examples, ReturnCode& returnCode)
  {
    resetEvaluation();
    return DecoratorInferencePolicy::supervisedExampleSetPreCallback(inference, examples, returnCode);
  }

  virtual ObjectPtr supervisedExamplePostCallback(InferenceStepPtr inference, ObjectPtr input, ObjectPtr output, ObjectPtr supervision, ReturnCode& returnCode)
  {
    if (output)
    {
      ProteinPtr p1 = output.dynamicCast<Protein>();
      ProteinPtr p2 = supervision.dynamicCast<Protein>();
      jassert(p1 && p2);
      addProtein(p1, p2);
    }
    return DecoratorInferencePolicy::supervisedExamplePostCallback(inference, input, output, supervision, returnCode);
  }

  virtual String toString() const
  {
    double Q3dbg = correctDBG / (double)secondaryStructureAccuracy->getCount();
    String res;
    res += T("Q3: ") + String(getQ3Score() * 100.0, 2) + T("\n");
    //res += T("Q8: ") + String(getQ8Score() * 100.0, 2) + T("\n");
    return res;
  }

  double getQ3Score() const
    {return secondaryStructureAccuracy->getMean();}

  double getQ8Score() const
    {return dsspSecondaryStructureAccuracy->getMean();}

  void resetEvaluation()
  {
    correctDBG = 0;
    secondaryStructureAccuracy = new ScalarVariableMean(T("Q3"));
    dsspSecondaryStructureAccuracy = new ScalarVariableMean(T("Q8"));
  }

  void addProtein(ProteinPtr predicted, ProteinPtr correct)
  {
    addLabelSequence(predicted->getSecondaryStructureSequence(), correct->getSecondaryStructureSequence(), secondaryStructureAccuracy);
    addLabelSequence(predicted->getSecondaryStructureSequence(true), correct->getSecondaryStructureSequence(true), dsspSecondaryStructureAccuracy);
  }

  void addLabelSequence(LabelSequencePtr predicted, LabelSequencePtr correct, ScalarVariableMeanPtr statistics)
  {
    if (!correct)
      return;
    jassert(predicted);
    size_t n = predicted->getLength();
    jassert(correct->getLength() == n);
    for (size_t i = 0; i < n; ++i)
    {
      size_t correctLabel;
      size_t predictedLabel;
      if (correct->getVariable(i, correctLabel))
      {
        if ((predicted->getVariable(i, predictedLabel) && predictedLabel == correctLabel))
          ++correctDBG;
        statistics->push((predicted->getVariable(i, predictedLabel) && predictedLabel == correctLabel) ? 1.0 : 0.0);
      }
    }
  }

protected:
  ScalarVariableMeanPtr secondaryStructureAccuracy;
  ScalarVariableMeanPtr dsspSecondaryStructureAccuracy;
  size_t correctDBG;
};

typedef ReferenceCountedObjectPtr<ProteinEvaluationPolicy> ProteinEvaluationPolicyPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_POLICY_PROTEIN_EVALUATION_H_
