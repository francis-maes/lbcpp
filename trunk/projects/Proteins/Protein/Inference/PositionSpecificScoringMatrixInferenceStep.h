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
#include "ProteinResidueFeatures.h"

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

class PSSMPredictionInferenceStep : public ParallelSequenceMultiRegressionInferenceStep
{
public:
  PSSMPredictionInferenceStep(const String& name, ProteinResidueFeaturesPtr features)
    : ParallelSequenceMultiRegressionInferenceStep(name, new PSSMRowPredictionInferenceStep()), features(features) {}
  PSSMPredictionInferenceStep() {}

  virtual size_t getNumSubInferences(ObjectPtr input) const
    {return getProtein(input)->getLength();}

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return features->compute(getProtein(input), index);}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
  {
    if (!supervision)
      return ObjectPtr();
    ScoreVectorSequencePtr pssm = getProtein(supervision)->getPositionSpecificScoringMatrix();
    return ParallelSequenceMultiRegressionInferenceStep::getSubSupervision(pssm, index);
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return getProtein(input)->createEmptyObject(T("PositionSpecificScoringMatrix"));}

private:
  ProteinResidueFeaturesPtr features;

  ProteinPtr getProtein(ObjectPtr object) const
  {
    ProteinPtr protein = object.dynamicCast<Protein>();
    jassert(protein);
    return protein;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_STEP_PSSM_H_
