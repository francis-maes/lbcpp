/*-----------------------------------------.---------------------------------.
| Filename: ProteinTertiaryStructureInfe..h| Tertiary Structure Inference    |
| Author  : Francis Maes                   |                                 |
| Started : 28/04/2010 19:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_TERTIARY_STRUCTURE_STEP_H_
# define LBCPP_PROTEIN_INFERENCE_TERTIARY_STRUCTURE_STEP_H_

# include "Protein1DInferenceStep.h"

namespace lbcpp
{

class ProteinResidueRefinementInferenceStep : public VectorStaticParallelInference
{
public:
  ProteinResidueRefinementInferenceStep(const String& name)
    : VectorStaticParallelInference(name)
  {
    for (size_t i = 0; i < 3; ++i)
    {
      String prefix = name + T(" ") + getBackboneAtomName(i) + T(".");
      appendStep(new RegressionInferenceStep(prefix + T("x")));
      appendStep(new RegressionInferenceStep(prefix + T("y")));
      appendStep(new RegressionInferenceStep(prefix + T("z")));
    }
  }
  ProteinResidueRefinementInferenceStep() {}

  static String getBackboneAtomName(size_t index)
  {
    jassert(index < 3);
    static const juce::tchar* backboneAtomNames[] = {T("N"), T("CA"), T("C")};
    return backboneAtomNames[index];
  }

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return input;}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index, ObjectPtr predictedObject) const
  {
    ProteinResiduePtr residue = supervision.dynamicCast<ProteinResidue>();
    if (!residue)
      return ObjectPtr();

    jassert(index < 9);
    ProteinAtomPtr atom = residue->findAtomByName(getBackboneAtomName(index / 3));
    if (!atom || !atom->getPosition().exists())
      return ObjectPtr();

    Vector3 position = atom->getPosition();
    index %= 3;
    double target = (index == 0 ? position.getX() : (index == 1 ? position.getY() : position.getZ()));
   
    // loss(prediction) = (target - prediction)^2
    return squareLoss(target);
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
  {
    ProteinResiduePtr res = new ProteinResidue();
    res->addAtom(new ProteinAtom(T("N"), T("N")));
    res->addAtom(new ProteinAtom(T("CA"), T("C")));
    res->addAtom(new ProteinAtom(T("C"), T("C")));
    return res;
  }

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
  {
    ProteinResiduePtr residue = output.dynamicCast<ProteinResidue>();
    ScalarPtr prediction = subOutput.dynamicCast<Scalar>();
    jassert(residue && prediction);
    ProteinAtomPtr atom = residue->findAtomByName(getBackboneAtomName(index / 3));
    jassert(atom);
    index %= 3;
    if (index == 0)
      atom->getPosition().setX(prediction->getValue());
    else if (index == 1)
      atom->getPosition().setY(prediction->getValue());
    else if (index == 2)
      atom->getPosition().setZ(prediction->getValue());
  }
};

class ProteinTertiaryStructureRefinementInferenceStep : public Protein1DInferenceStep
{
public:
  ProteinTertiaryStructureRefinementInferenceStep(const String& name, ProteinResidueFeaturesPtr features)
    : Protein1DInferenceStep(name, new ProteinResidueRefinementInferenceStep(name + T(" Residue")), features, T("TertiaryStructure")) {}
  
  ProteinTertiaryStructureRefinementInferenceStep() {}

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
  {
    ProteinTertiaryStructurePtr tertiaryStructure = output.dynamicCast<ProteinTertiaryStructure>();
    ProteinResiduePtr residue = subOutput.dynamicCast<ProteinResidue>();
    jassert(tertiaryStructure && residue);
    tertiaryStructure->setResidue(index, residue); 
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_TERTIARY_STRUCTURE_STEP_H_
