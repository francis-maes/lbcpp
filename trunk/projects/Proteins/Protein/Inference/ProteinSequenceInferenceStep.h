/*-----------------------------------------.---------------------------------.
| Filename: ProteinSequenceLabelingInfe...h| Protein Sequence Labeling       |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 21:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_LABELING_H_
# define LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_LABELING_H_

# include "../../InferenceStep/ClassificationInferenceStep.h"
# include "../../InferenceStep/ParallelInferenceStep.h"
# include "../../InferenceStep/ParallelSharedMultiRegressionInferenceStep.h"
# include "ProteinInferenceStepHelper.h"

namespace lbcpp
{

// Inference:
//   Input, Supervision: Protein
//   Output: a class inherited from Sequence
//
// SubInference:
//   Input: Features
//   Output, Supervision: Sequence elements
class ProteinSequenceInferenceStep : public SharedParallelInferenceStep, public ProteinResidueRelatedInferenceStepHelper
{
public:
  ProteinSequenceInferenceStep(const String& name, InferenceStepPtr subInference, ProteinResidueFeaturesPtr features, const String& targetName)
    : SharedParallelInferenceStep(name, subInference), ProteinResidueRelatedInferenceStepHelper(targetName, features) {}
  
  ProteinSequenceInferenceStep() {}
  
  virtual size_t getNumSubInferences(ObjectPtr input) const
    {return getProteinLength(input);}

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return features->compute(getProtein(input), index);}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
  {
    SequencePtr sequence = getTarget(supervision).dynamicCast<Sequence>();
    jassert(sequence);
    return sequence->get(index);
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return ProteinResidueRelatedInferenceStepHelper::createEmptyOutput(input);}

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
    {output.dynamicCast<ObjectContainer>()->set(index, subOutput);}

protected:
  virtual bool load(InputStream& istr)
  {
    return SharedParallelInferenceStep::load(istr) &&
      ProteinResidueRelatedInferenceStepHelper::load(istr);
  }

  virtual void save(OutputStream& ostr) const
  {
    SharedParallelInferenceStep::save(ostr);
    ProteinResidueRelatedInferenceStepHelper::save(ostr);
  }
};
  
typedef ReferenceCountedObjectPtr<ProteinSequenceInferenceStep> ProteinSequenceInferenceStepPtr;

class ProteinSequenceLabelingInferenceStep : public ProteinSequenceInferenceStep
{
public:
  ProteinSequenceLabelingInferenceStep(const String& name, ProteinResidueFeaturesPtr features, const String& targetName)
    : ProteinSequenceInferenceStep(name, new ClassificationInferenceStep(name + T("Classification")), features, targetName) {}
  ProteinSequenceLabelingInferenceStep() {}

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
  {
    LabelSequencePtr res = ProteinSequenceInferenceStep::createEmptyOutput(input).dynamicCast<LabelSequence>();
    getSharedInferenceStep().dynamicCast<ClassificationInferenceStep>()->setLabels(res->getDictionary());
    return res;
  }
};


// Input: Features
// Output, Supervision: BackbondBond
class BackboneBondInferenceStep : public ParallelInferenceStep
{
public:
  BackboneBondInferenceStep(const String& name)
    : ParallelInferenceStep(name) {}

  virtual size_t getNumSubInferences(ObjectPtr input) const
    {return 0;}

  virtual InferenceStepPtr getSubInference(ObjectPtr input, size_t index) const
    {return InferenceStepPtr();}

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return input;}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
    {return supervision;}

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return ObjectPtr();}

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
    {}

  // phi
  // psi
  // omega
  // L1
  // L2
  // L3
};


// Input, Supervision: Protein
// Output: BackbondBondSequence
class ProteinBackboneBondSequenceInferenceStep : public ProteinSequenceInferenceStep
{
public:
  ProteinBackboneBondSequenceInferenceStep(const String& name, ProteinResidueFeaturesPtr features)
    : ProteinSequenceInferenceStep(name, new BackboneBondInferenceStep(name + T("Bond")), features, T("BackboneBondSequence")) {}

  ProteinBackboneBondSequenceInferenceStep()
    {}
  // FIXME: virtual size_t getNumSubInferences(ObjectPtr input) const - 1 ?
};

class PSSMRowPredictionInferenceStep : public ParallelSharedMultiRegressionInferenceStep
{
public:
  PSSMRowPredictionInferenceStep()
    : ParallelSharedMultiRegressionInferenceStep(T("PSSMRow"), AminoAcidDictionary::getInstance()) {}

  virtual size_t getNumSubInferences(ObjectPtr) const
    {return AminoAcidDictionary::numAminoAcids;}
 
  FeatureGeneratorPtr getInputFeatures(ObjectPtr input, size_t scoreIndex) const;
};

class PSSMPredictionInferenceStep : public ProteinSequenceInferenceStep
{
public:
  PSSMPredictionInferenceStep(const String& name, ProteinResidueFeaturesPtr features)
    : ProteinSequenceInferenceStep(name, new PSSMRowPredictionInferenceStep(), features, T("PositionSpecificScoringMatrix")) {}
  PSSMPredictionInferenceStep() {}
};

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_LABELING_H_
