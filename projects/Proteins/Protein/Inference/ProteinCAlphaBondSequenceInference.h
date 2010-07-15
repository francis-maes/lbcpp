/*-----------------------------------------.---------------------------------.
| Filename: ProteinCAlphaBondSequenceInf..h| C-alpha bond sequence inference |
| Author  : Francis Maes                   |                                 |
| Started : 29/04/2010 14:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_PROTEIN_INFERENCE_STEP_CALPHA_BOND_SEQUENCE_H_
# define LBCPP_PROTEIN_INFERENCE_STEP_CALPHA_BOND_SEQUENCE_H_

# include "Protein1DTargetInference.h"

namespace lbcpp
{

class ProteinCAlphaBondInferenceStep : public VectorParallelInference
{
public:
  ProteinCAlphaBondInferenceStep(const String& name, InferencePtr angleInference, InferencePtr dihedralInference)
    : VectorParallelInference(name)
  {
    appendInference(angleInference);
    appendInference(dihedralInference);
  }
  ProteinCAlphaBondInferenceStep() {}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    BondCoordinatesObjectPtr bond = supervision.dynamicCast<BondCoordinatesObject>();
    jassert(bond || !supervision);

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->addSubInference(getSubInference(0), input, bond && bond->getValue().hasThetaAngle()
      ? Variable(bond->getValue().getThetaAngle()) : Variable());
    res->addSubInference(getSubInference(1), input, bond && bond->getValue().hasPhiDihedralAngle()
      ? Variable(bond->getValue().getPhiDihedralAngle()) : Variable());
    return res;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    impl::BondCoordinates bond;
    bond.setLength(3.8);
    Variable theta = state->getSubOutput(0);
    Variable dihedral = state->getSubOutput(1);
    if (theta)
      bond.setThetaAngle(juce::jlimit(0.8, 2.9, theta.getDouble()));
    if (dihedral)
      bond.setPhiDihedralAngle(dihedral.getDouble());
    return ObjectPtr(new BondCoordinatesObject(bond));
  }
};

class ProteinCAlphaBondSequenceInferenceStep : public Protein1DTargetInference
{
public:
  ProteinCAlphaBondSequenceInferenceStep(const String& name, ProteinResidueFeaturesPtr features, InferencePtr angleInference, InferencePtr dihedralInference)
    : Protein1DTargetInference(name, new ProteinCAlphaBondInferenceStep(name + T(" Bond"), angleInference, dihedralInference), features, T("CAlphaBondSequence")) {}
  
  ProteinCAlphaBondSequenceInferenceStep() {}
  
  virtual size_t getNumSubInferences(ProteinObjectPtr protein) const
    {size_t n = protein->getLength(); jassert(n > 0); return n - 1;}
};

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_CALPHA_BOND_SEQUENCE_H_
