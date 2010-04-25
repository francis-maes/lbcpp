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
class ProteinBackboneBondInferenceStep : public VectorParallelInferenceStep
{
public:
  ProteinBackboneBondInferenceStep(const String& name)
    : VectorParallelInferenceStep(name)
  {
    for (size_t i = 0; i < 3; ++i)
    {
      String namePrefix = T("Bond") + lbcpp::toString(i + 1);
      for (size_t j = 0; j < 3; ++j)
      {
        String namePostfix = (j == 0 ? T("Length") : (j == 1 ? T("Angle") : T("DihedralAngle")));
        appendStep(new RegressionInferenceStep(namePrefix + T(" ") + namePostfix));
      }
    }
  }

  ProteinBackboneBondInferenceStep() {}

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return input;}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
  {
    ProteinBackboneBondPtr bond = supervision.dynamicCast<ProteinBackboneBond>();
    return bond ? ObjectPtr(new Scalar(getTarget(bond, index))) : ObjectPtr();
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return new ProteinBackboneBond();}

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
  {
    ProteinBackboneBondPtr bond = output.dynamicCast<ProteinBackboneBond>();
    ScalarPtr prediction = subOutput.dynamicCast<Scalar>();
    jassert(bond && prediction);
    getTarget(bond, index) = prediction->getValue();
  }

private:
  static double& getTarget(ProteinBackboneBondPtr bond, size_t index)
  {
    jassert(bond);
    switch (index)
    {
    case 0: return bond->getBond1().getLength();
    case 1: return bond->getBond1().getThetaAngle();
    case 2: return bond->getBond1().getPhiDihedralAngle();

    case 3: return bond->getBond2().getLength();
    case 4: return bond->getBond2().getThetaAngle();
    case 5: return bond->getBond2().getPhiDihedralAngle();

    case 6: return bond->getBond3().getLength();
    case 7: return bond->getBond3().getThetaAngle();
    case 8: return bond->getBond3().getPhiDihedralAngle();

    default:
      jassert(false);
      return *(double* )0;
    };
  }
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
};

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_BACKBONE_BOND_SEQUENCE_H_
