/*-----------------------------------------.---------------------------------.
| Filename: ProteinSequenceLabelingInfe...h| Protein Sequence Labeling       |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 21:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_H_
# define LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_H_

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
  ProteinSequenceInferenceStep(const String& name, InferenceStepPtr subInference, ProteinResidueFeaturesPtr features, const String& targetName, const String& supervisionName = String::empty)
    : SharedParallelInferenceStep(name, subInference), ProteinResidueRelatedInferenceStepHelper(targetName, features, supervisionName) {}
  
  ProteinSequenceInferenceStep() {}
  
  virtual size_t getNumSubInferences(ObjectPtr input) const
    {return getProteinLength(input);}

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return features->compute(getProtein(input), index);}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
  {
    SequencePtr sequence = getSupervision(supervision).dynamicCast<Sequence>();
    return sequence ? sequence->get(index) : ObjectPtr();
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
  ProteinSequenceLabelingInferenceStep(const String& name, ProteinResidueFeaturesPtr features, const String& targetName, const String& supervisionName = String::empty)
    : ProteinSequenceInferenceStep(name, new ClassificationInferenceStep(name + T("Classification")), features, targetName, supervisionName) {}
  ProteinSequenceLabelingInferenceStep() {}

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
  {
    SequencePtr res = ProteinSequenceInferenceStep::createEmptyOutput(input).dynamicCast<Sequence>();
    ClassificationInferenceStepPtr step = getSharedInferenceStep().dynamicCast<ClassificationInferenceStep>();
    LabelSequencePtr ls = res.dynamicCast<LabelSequence>();
    if (ls)
      step->setLabels(ls->getDictionary());
    else
    {
      ScoreVectorSequencePtr svs = res.dynamicCast<ScoreVectorSequence>();
      if (svs)
        step->setLabels(svs->getDictionary());
      else
        jassert(false);
    }
    return res;
  }
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

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_H_
