/*-----------------------------------------.---------------------------------.
| Filename: PositionSpecificScoringMatr...h| Inference of PSSMs with         |
| Author  : Francis Maes                   |   regression                    |
| Started : 22/04/2010 23:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_STEP_PSSM_H_
# define LBCPP_PROTEIN_INFERENCE_STEP_PSSM_H_

# include "../../InferenceStep/ParallelSharedMultiRegressionInferenceStep.h"
# include "../../InferenceStep/ParallelSequenceMultiRegressionInferenceStep.h"
# include "ProteinInferenceStepHelper.h"

namespace lbcpp
{

class PSSMRowPredictionInferenceStep : public ParallelSharedMultiRegressionInferenceStep
{
public:
  PSSMRowPredictionInferenceStep()
    : ParallelSharedMultiRegressionInferenceStep(T("PSSMRow"), AminoAcidDictionary::getInstance()) {}

  virtual size_t getNumSubInferences(ObjectPtr) const
    {return AminoAcidDictionary::numAminoAcids;}
 
  FeatureGeneratorPtr getInputFeatures(ObjectPtr input, size_t scoreIndex) const;
};

class PSSMPredictionInferenceStep : public ParallelSequenceMultiRegressionInferenceStep, public ProteinResidueRelatedInferenceStepHelper
{
public:
  PSSMPredictionInferenceStep(const String& name, ProteinResidueFeaturesPtr features)
    : ParallelSequenceMultiRegressionInferenceStep(name, new PSSMRowPredictionInferenceStep()),
      ProteinResidueRelatedInferenceStepHelper(T("PositionSpecificScoringMatrix"), features) {}
  PSSMPredictionInferenceStep() {}

  virtual size_t getNumSubInferences(ObjectPtr input) const
    {return getProteinLength(input);}

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return features->compute(getProtein(input), index);}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
  {
    if (!supervision)
      return ObjectPtr();
    ScoreVectorSequencePtr pssm = getTarget(supervision).dynamicCast<ScoreVectorSequence>();
    return ParallelSequenceMultiRegressionInferenceStep::getSubSupervision(pssm, index);
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return ProteinResidueRelatedInferenceStepHelper::createEmptyOutput(input);}

private:
  ProteinResidueFeaturesPtr features;

};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_STEP_PSSM_H_
