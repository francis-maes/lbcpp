/*-----------------------------------------.---------------------------------.
| Filename: ProteinBackboneBondSequence...h| Prediction of the sequence of   |
| Author  : Francis Maes                   |  backbone bond sequence         |
| Started : 23/04/2010 20:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_STEP_BACKBONE_BOND_SEQUENCE_H_
# define LBCPP_PROTEIN_INFERENCE_STEP_BACKBONE_BOND_SEQUENCE_H_

# include "ProteinCAlphaBondSequenceInferenceStep.h"

namespace lbcpp
{

class ScaledSigmoidScalarFunction : public ScalarFunction
{
public:
  ScaledSigmoidScalarFunction(double minimumValue = 0.0, double maximumValue = 1.0)
    : minimumValue(minimumValue), maximumValue(maximumValue) {}
  
  virtual bool isDerivable() const
    {return false;}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    double normalizedValue = 1.0 / (1.0 + exp(-input));
    double scale = maximumValue - minimumValue;
    if (output)
      *output = normalizedValue * scale + minimumValue;
    if (derivative)
      *derivative = scale * normalizedValue * (1.0 - normalizedValue);
  }

protected:
  double minimumValue, maximumValue;

  virtual bool load(InputStream& istr)
    {return ScalarFunction::load(istr) && lbcpp::read(istr, minimumValue) && lbcpp::read(istr, maximumValue);}

  virtual void save(OutputStream& ostr) const
    {ScalarFunction::save(ostr); lbcpp::write(ostr, minimumValue); lbcpp::write(ostr, maximumValue);}
};

// Input: Features
// Output, Supervision: BackbondBond
class ProteinBackboneBondInferenceStep : public VectorStaticParallelInference
{
public:
  ProteinBackboneBondInferenceStep(const String& name)
    : VectorStaticParallelInference(name)
  {
    for (size_t i = 0; i < 3; ++i)
    {
      String namePrefix = T("Bond") + lbcpp::toString(i + 1);
      for (size_t j = 0; j < 3; ++j)
      {
        String namePostfix = (j == 0 ? T("Length") : (j == 1 ? T("Angle") : T("DihedralAngle")));
        RegressionInferenceStepPtr regressionInference = new RegressionInferenceStep(namePrefix + T(" ") + namePostfix);

      /*  if (j == 0)
        {
          static const double bondIntervals[] = {1.2, 1.8, 1.3, 2.0, 1.1, 1.9};
          appendStep(new TransferFunctionDecoratorInference(regressionInference->getName(), regressionInference,
            new ScaledSigmoidScalarFunction(bondIntervals[j*2], bondIntervals[j*2] + 1)));
        }
        else if (j == 1)
        {
          static const double angleIntervals[] = {1.5, 2.5, 1.7, 2.4, 1.4, 2.5};
          appendStep(new TransferFunctionDecoratorInference(regressionInference->getName(), regressionInference,
            new ScaledSigmoidScalarFunction(angleIntervals[j*2], angleIntervals[j*2] + 1)));
        }
        else*/
          subInferences.append(regressionInference);
      }
    }
  }

  ProteinBackboneBondInferenceStep() {}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);

    ProteinBackboneBondPtr bond = supervision.dynamicCast<ProteinBackboneBond>();
    jassert(bond || !supervision);
    
    res->reserve(subInferences.size());
    for (size_t i = 0; i < subInferences.size(); ++i)
    {
      bool targetExists = false;
      double target = 0.0;
      if (bond)
        getTarget(bond, i, targetExists);
      res->addSubInference(subInferences[i], input, targetExists ? ObjectPtr(new Scalar(target)) : ObjectPtr());
    }
    return res;
  }

  virtual ObjectPtr finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    ProteinBackboneBondPtr res = new ProteinBackboneBond();
    for (size_t i = 0; i < state->getNumSubInferences(); ++i)
    {
      ScalarPtr prediction = state->getSubOutput(i).dynamicCast<Scalar>();
      if (prediction)
      {
        bool targetExists;
        getTarget(res, i, targetExists) = prediction->getValue();
      }
    }
    return res;
  }

private:
  static double& getTarget(ProteinBackboneBondPtr bond, size_t index, bool& targetExists)
  {
    jassert(bond);
    switch (index)
    {
    case 0: targetExists = bond->getBond1().hasLength(); return bond->getBond1().getLength();
    case 1: targetExists = bond->getBond1().hasThetaAngle(); return bond->getBond1().getThetaAngle();
    case 2: targetExists = bond->getBond1().hasPhiDihedralAngle(); return bond->getBond1().getPhiDihedralAngle();

    case 3: targetExists = bond->getBond2().hasLength(); return bond->getBond2().getLength();
    case 4: targetExists = bond->getBond2().hasThetaAngle(); return bond->getBond2().getThetaAngle();
    case 5: targetExists = bond->getBond2().hasPhiDihedralAngle(); return bond->getBond2().getPhiDihedralAngle();

    case 6: targetExists = bond->getBond3().hasLength(); return bond->getBond3().getLength();
    case 7: targetExists = bond->getBond3().hasThetaAngle(); return bond->getBond3().getThetaAngle();
    case 8: targetExists = bond->getBond3().hasPhiDihedralAngle(); return bond->getBond3().getPhiDihedralAngle();

    default:
      jassert(false);
      return *(double* )0;
    };
  }
};

// Input, Supervision: Protein
// Output: BackbondBondSequence
class ProteinBackboneBondSequenceInferenceStep : public Protein1DInferenceStep
{
public:
  ProteinBackboneBondSequenceInferenceStep(const String& name, ProteinResidueFeaturesPtr features)
    : Protein1DInferenceStep(name, new ProteinBackboneBondInferenceStep(name + T("Bond")), features, T("BackboneBondSequence")) {}

  ProteinBackboneBondSequenceInferenceStep()
    {}
};

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_BACKBONE_BOND_SEQUENCE_H_
