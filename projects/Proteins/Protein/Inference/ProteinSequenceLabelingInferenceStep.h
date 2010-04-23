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
# include "ProteinInferenceStepHelper.h"

namespace lbcpp
{

class ProteinSequenceLabelingInferenceStep : public ParallelSequenceLabelingInferenceStep, public ProteinResidueRelatedInferenceStepHelper
{
public:
  ProteinSequenceLabelingInferenceStep(const String& name, ProteinResidueFeaturesPtr features, const String& targetName = String::empty)
    : ParallelSequenceLabelingInferenceStep(name), ProteinResidueRelatedInferenceStepHelper(targetName, features) {}
  ProteinSequenceLabelingInferenceStep() {}

  virtual FeatureGeneratorPtr getInputFeatures(ObjectPtr input, size_t index) const
    {return features->compute(getProtein(input), index);}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
    {return ParallelSequenceLabelingInferenceStep::getSubSupervision(getTarget(supervision), index);}

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
  {
    LabelSequencePtr res = ProteinResidueRelatedInferenceStepHelper::createEmptyOutput(input).dynamicCast<LabelSequence>();
    const_cast<ProteinSequenceLabelingInferenceStep* >(this)->setLabels(res->getDictionary());
    return res;
  }

  virtual size_t getNumSubInferences(ObjectPtr input) const
    {return getProteinLength(input);}
    
protected:
  virtual bool load(InputStream& istr)
  {
    return ParallelSequenceLabelingInferenceStep::load(istr) &&
      ProteinResidueRelatedInferenceStepHelper::load(istr);
  }

  virtual void save(OutputStream& ostr) const
  {
    ParallelSequenceLabelingInferenceStep::save(ostr);
    ProteinResidueRelatedInferenceStepHelper::save(ostr);
  }
};

typedef ReferenceCountedObjectPtr<ProteinSequenceLabelingInferenceStep> ProteinSequenceLabelingInferenceStepPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_LABELING_H_
