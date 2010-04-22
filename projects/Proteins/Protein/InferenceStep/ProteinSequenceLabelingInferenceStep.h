/*-----------------------------------------.---------------------------------.
| Filename: ProteinSequenceLabelingInfe...h| Protein Sequence Labeling       |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 21:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_LABELING_H_
# define LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_LABELING_H_

# include "../../InferenceStep/ParallelSequenceLabelingInferenceStep.h"

namespace lbcpp
{

class ProteinSequenceLabelingInferenceStep : public ParallelSequenceLabelingInferenceStep
{
public:
  ProteinSequenceLabelingInferenceStep(const String& name)
    : ParallelSequenceLabelingInferenceStep(name) {}
  ProteinSequenceLabelingInferenceStep() {}

  virtual String getTargetName() const = 0;
  virtual FeatureDictionaryPtr getTargetDictionary() const = 0;

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
  {
    ProteinPtr protein = supervision.dynamicCast<Protein>();
    jassert(protein);
    ObjectPtr proteinObject = protein->getObject(getTargetName());
    return ParallelSequenceLabelingInferenceStep::getSubSupervision(proteinObject, index);
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return new LabelSequence(getTargetName(), getTargetDictionary(), getNumSubInferences(input));}

  virtual size_t getNumSubInferences(ObjectPtr input) const
  {
    ProteinPtr protein = input.dynamicCast<Protein>();
    jassert(protein);
    return protein->getLength();
  }

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    setLabels(getTargetDictionary());
    return ParallelSequenceLabelingInferenceStep::run(context, input, supervision, returnCode);
  }
};

typedef ReferenceCountedObjectPtr<ProteinSequenceLabelingInferenceStep> ProteinSequenceLabelingInferenceStepPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_LABELING_H_
