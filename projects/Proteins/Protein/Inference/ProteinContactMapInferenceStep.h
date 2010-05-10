/*-----------------------------------------.---------------------------------.
| Filename: ProteinContactMapInferenceStep.h| Protein Contact Map Inference  |
| Author  : Francis Maes                   |                                 |
| Started : 28/04/2010 11:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_
# define LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_

# include "Protein2DInferenceStep.h"

namespace lbcpp
{
/*
class AddScalarBiasDecoratorInference : public DecoratorInference
{
public:
  AddScalarBiasDecoratorInference(const String& name, InferencePtr regressionStep)
    : DecoratorInference(name, regressionStep), bias(0.0) {}
  AddScalarBiasDecoratorInference() {}
  
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ObjectPtr result = DecoratorInference::run(context, input, supervision, returnCode);
    if (result)
    {
      ScalarPtr scalarResult = result.dynamicCast<Scalar>();
      jassert(scalarResult);
      if (supervision)
      {
        ScalarFunctionPtr loss = supervision.dynamicCast<ScalarFunction>();
        jassert(loss);
        bias -= 0.001 * loss->computeDerivative(scalarResult->getValue());
        static int count = 0;
        if (++count % 10000 == 0)
          std::cout << "Bias: " << bias << std::endl;
      }
      result = new Scalar(scalarResult->getValue() + bias);
    }
    return result;
  }

protected:
  ScalarVariableMean delta;
  double bias;

  virtual bool load(InputStream& istr)
    {return DecoratorInference::load(istr) && lbcpp::read(istr, bias);}

  virtual void save(OutputStream& ostr) const
    {DecoratorInference::save(ostr); lbcpp::write(ostr, bias);}
};*/
class ProteinContactMapInferenceStep : public Protein2DInferenceStep
{
public:
  ProteinContactMapInferenceStep(const String& name, ProteinResiduePairFeaturesPtr features, const String& targetName)
    : Protein2DInferenceStep(name, InferencePtr(), features, targetName)
    {setSharedInferenceStep(/*new AddScalarBiasDecoratorInference*/(name, linearScalarInference(name + T(" Classification"))));}

  virtual void computeSubStepIndices(ProteinPtr protein, std::vector< std::pair<size_t, size_t> >& res) const
  {
    size_t n = protein->getLength();
    res.reserve(n * (n - 5) / 2);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 6; j < n; ++j)
        res.push_back(std::make_pair(i, j));
  }

  virtual ObjectPtr getSubSupervision(ObjectPtr supervisionObject, size_t firstPosition, size_t secondPosition) const
  {
    ScoreSymmetricMatrixPtr contactMap = supervisionObject.dynamicCast<ScoreSymmetricMatrix>();
    jassert(contactMap);
    if (!contactMap || !contactMap->hasScore(firstPosition, secondPosition))
      return ObjectPtr();
    return hingeLoss(contactMap->getScore(firstPosition, secondPosition) > 0.5 ? 1 : 0, 0.0);
  }

  virtual void setSubOutput(ObjectPtr output, size_t firstPosition, size_t secondPosition, ObjectPtr subOutput) const
  {
    ScoreSymmetricMatrixPtr contactMap = output.dynamicCast<ScoreSymmetricMatrix>();
    ScalarPtr score = subOutput.dynamicCast<Scalar>();
    jassert(contactMap && score);
    contactMap->setScore(firstPosition, secondPosition, 1.0 / (1.0 + exp(-score->getValue())));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_
