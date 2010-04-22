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
#include "ProteinResidueFeatures.h"

namespace lbcpp
{

class ProteinSequenceLabelingInferenceStep : public ParallelSequenceLabelingInferenceStep
{
public:
  ProteinSequenceLabelingInferenceStep(const String& name, ProteinResidueFeaturesPtr features, const String& targetName = String::empty)
    : ParallelSequenceLabelingInferenceStep(name), features(features), targetName(targetName) {}
  ProteinSequenceLabelingInferenceStep() {}

  String getTargetName() const
    {return targetName;}

  void setTargetName(const String& targetName)
    {this->targetName = targetName;}

  virtual FeatureGeneratorPtr getInputFeatures(ObjectPtr input, size_t index) const
  {
    jassert(features);
    ProteinPtr protein = input.dynamicCast<Protein>();
    jassert(protein);
    return features->compute(protein, index);
  }

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
  {
    jassert(targetName.isNotEmpty());
    ProteinPtr protein = supervision.dynamicCast<Protein>();
    jassert(protein);
    ObjectPtr proteinObject = protein->getObject(targetName);
    return ParallelSequenceLabelingInferenceStep::getSubSupervision(proteinObject, index);
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
  {
    jassert(targetName.isNotEmpty());
    ProteinPtr protein = input.dynamicCast<Protein>();
    jassert(protein);
    LabelSequencePtr res = protein->createEmptyObject(targetName).dynamicCast<LabelSequence>();
    const_cast<ProteinSequenceLabelingInferenceStep* >(this)->setLabels(res->getDictionary());
    return res;
  }

  virtual size_t getNumSubInferences(ObjectPtr input) const
  {
    ProteinPtr protein = input.dynamicCast<Protein>();
    jassert(protein);
    return protein->getLength();
  }

protected:
  String targetName;
  ProteinResidueFeaturesPtr features;

  virtual bool load(InputStream& istr)
  {
    return ParallelSequenceLabelingInferenceStep::load(istr) &&
      lbcpp::read(istr, targetName) && lbcpp::read(istr, features);
  }

  virtual void save(OutputStream& ostr) const
  {
    ParallelSequenceLabelingInferenceStep::save(ostr);
    lbcpp::write(ostr, targetName);
    lbcpp::write(ostr, features);
  }
};

typedef ReferenceCountedObjectPtr<ProteinSequenceLabelingInferenceStep> ProteinSequenceLabelingInferenceStepPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_LABELING_H_
