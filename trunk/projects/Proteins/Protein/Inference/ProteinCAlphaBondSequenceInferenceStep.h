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
# include "../../InferenceStep/DecoratorInferenceStep.h"

namespace lbcpp
{

class AngleDifferenceScalarFunction : public ScalarFunction
{
public:
  AngleDifferenceScalarFunction(double referenceAngle = 0.0)
    : referenceAngle(referenceAngle) {}

  virtual bool isDerivable() const
    {return false;}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (output)
      *output = DihedralAngle::normalize(input - referenceAngle);
    if (derivative)
      *derivative = 1.0;
  }

protected:
  double referenceAngle;

  virtual bool load(InputStream& istr)
    {return ScalarFunction::load(istr) && lbcpp::read(istr, referenceAngle);}

  virtual void save(OutputStream& ostr) const
    {ScalarFunction::save(ostr); lbcpp::write(ostr, referenceAngle);}
};

class ProteinCAlphaBondInferenceStep : public VectorParallelInferenceStep
{
public:
  ProteinCAlphaBondInferenceStep(const String& name)
    : VectorParallelInferenceStep(name)
  {
    appendStep(new RegressionInferenceStep(getName() + T(" angle")));
    appendStep(new RegressionInferenceStep(getName() + T(" dihedral")));
  }
  ProteinCAlphaBondInferenceStep() {}

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
  {
    BondCoordinates bond;
    bond.setLength(3.8);
    return new BondCoordinatesObject(bond);
  }
 
  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return input;}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index, ObjectPtr predictedObject) const
  {
    BondCoordinatesObjectPtr bond = supervision.dynamicCast<BondCoordinatesObject>();
    jassert(bond);
    BondCoordinates& c = bond->getValue();
    if (index == 0)
      return c.hasThetaAngle() ? ObjectPtr(squareFunction(addConstantScalarFunction(-c.getThetaAngle()))) : ObjectPtr();
    else
      return c.hasPhiDihedralAngle() ? ObjectPtr(squareFunction(new AngleDifferenceScalarFunction(c.getPhiDihedralAngle()))) : ObjectPtr();
  }

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
  {
    BondCoordinatesObjectPtr bond = output.dynamicCast<BondCoordinatesObject>();
    ScalarPtr value = subOutput.dynamicCast<Scalar>();
    jassert(bond && value);
    if (index == 0)
      bond->getValue().setThetaAngle(juce::jlimit(0.8, 2.9, value->getValue()));
    else
      bond->getValue().setPhiDihedralAngle(value->getValue());
  }
};

class ProteinCAlphaBondSequenceInferenceStep : public Protein1DInferenceStep
{
public:
  ProteinCAlphaBondSequenceInferenceStep(const String& name, ProteinResidueFeaturesPtr features)
    : Protein1DInferenceStep(name, new ProteinCAlphaBondInferenceStep(name + T(" Bond")), features, T("CAlphaBondSequence")) {}
  
  ProteinCAlphaBondSequenceInferenceStep() {}
  
  virtual size_t getNumSubInferences(ObjectPtr input) const
    {size_t n = getProteinLength(input); jassert(n > 0); return n - 1;}
};

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_CALPHA_BOND_SEQUENCE_H_
