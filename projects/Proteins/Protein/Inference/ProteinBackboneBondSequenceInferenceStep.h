/*-----------------------------------------.---------------------------------.
| Filename: ProteinBackboneBondSequence...h| Prediction of the sequence of   |
| Author  : Francis Maes                   |  backbone bond sequence         |
| Started : 23/04/2010 20:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_STEP_BACKBONE_BOND_SEQUENCE_H_
# define LBCPP_PROTEIN_INFERENCE_STEP_BACKBONE_BOND_SEQUENCE_H_

# include "ProteinSequenceInferenceStep.h"

namespace lbcpp
{

// Input: Features
// Output, Supervision: BackbondBond
class ProteinBackboneBondInferenceStep : public ParallelInferenceStep
{
public:
  ProteinBackboneBondInferenceStep(const String& name)
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
    : ProteinSequenceInferenceStep(name, new ProteinBackboneBondInferenceStep(name + T("Bond")), features, T("BackboneBondSequence")) {}

  ProteinBackboneBondSequenceInferenceStep()
    {}
  // FIXME: virtual size_t getNumSubInferences(ObjectPtr input) const - 1 ?
};

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_BACKBONE_BOND_SEQUENCE_H_
