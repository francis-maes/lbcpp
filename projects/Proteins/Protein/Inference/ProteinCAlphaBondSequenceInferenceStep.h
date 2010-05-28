/*-----------------------------------------.---------------------------------.
| Filename: ProteinCAlphaBondSequenceInf..h| C-alpha bond sequence inference |
| Author  : Francis Maes                   |                                 |
| Started : 29/04/2010 14:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_PROTEIN_INFERENCE_STEP_CALPHA_BOND_SEQUENCE_H_
# define LBCPP_PROTEIN_INFERENCE_STEP_CALPHA_BOND_SEQUENCE_H_

# include "Protein1DInferenceStep.h"

namespace lbcpp
{

class ProteinCAlphaBondInferenceStep : public VectorStaticParallelInference
{
public:
  ProteinCAlphaBondInferenceStep(const String& name, InferencePtr angleInference, InferencePtr dihedralInference)
    : VectorStaticParallelInference(name)
  {
    subInferences.append(angleInference);
    subInferences.append(dihedralInference);
  }
  ProteinCAlphaBondInferenceStep() {}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    BondCoordinatesObjectPtr bond = supervision.dynamicCast<BondCoordinatesObject>();
    jassert(bond || !supervision);

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->addSubInference(subInferences.get(0), input, bond && bond->getValue().hasThetaAngle()
      ? ObjectPtr(new Scalar(bond->getValue().getThetaAngle())) : ObjectPtr());
    res->addSubInference(subInferences.get(1), input, bond && bond->getValue().hasPhiDihedralAngle()
      ? ObjectPtr(new Scalar(bond->getValue().getPhiDihedralAngle())) : ObjectPtr());
    return res;
  }

  virtual ObjectPtr finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    BondCoordinates bond;
    bond.setLength(3.8);
    ScalarPtr theta = state->getSubOutput(0).dynamicCast<Scalar>();
    ScalarPtr dihedral = state->getSubOutput(1).dynamicCast<Scalar>();
    if (theta)
      bond.setThetaAngle(juce::jlimit(0.8, 2.9, theta->getValue()));
    if (dihedral)
      bond.setPhiDihedralAngle(dihedral->getValue());
    return new BondCoordinatesObject(bond);
  }
};

class ProteinCAlphaBondSequenceInferenceStep : public Protein1DInferenceStep
{
public:
  ProteinCAlphaBondSequenceInferenceStep(const String& name, ProteinResidueFeaturesPtr features, InferencePtr angleInference, InferencePtr dihedralInference)
    : Protein1DInferenceStep(name, new ProteinCAlphaBondInferenceStep(name + T(" Bond"), angleInference, dihedralInference), features, T("CAlphaBondSequence")) {}
  
  ProteinCAlphaBondSequenceInferenceStep() {}
  
  virtual size_t getNumSubInferences(ProteinPtr protein) const
    {size_t n = protein->getLength(); jassert(n > 0); return n - 1;}
};

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_CALPHA_BOND_SEQUENCE_H_
