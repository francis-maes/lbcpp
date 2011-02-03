/*-----------------------------------------.---------------------------------.
| Filename: ConcatenateFeatureGenerator.h  | Concatenate Feature Generator   |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 18:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_FEATURE_GENERATOR_CONCATENATE_H_
# define LBCPP_OPERATOR_FEATURE_GENERATOR_CONCATENATE_H_

# include <lbcpp/Operator/FeatureGenerator.h>

namespace lbcpp
{

class ConcatenateFeatureGenerator : public FeatureGenerator
{
public:
  virtual VariableSignaturePtr initializeOperator(ExecutionContext& context)
  {
    size_t numInputs = getNumInputs();

    if (!numInputs)
    {
      context.errorCallback(T("No inputs"));
      return VariableSignaturePtr();
    }

    DefaultClassPtr outputType = new UnnamedDynamicClass(T("FeatureVector"));
    shifts.resize(numInputs);
    for (size_t i = 0; i < numInputs; ++i)
    {
      const VariableSignaturePtr& inputVariable = getInputVariable(i);

      shifts[i] = outputType->getNumMemberVariables();
      TypePtr subType = inputVariable->getType();
      size_t n = subType->getNumMemberVariables();
      for (size_t j = 0; j < n; ++j)
      {
        VariableSignaturePtr signature = subType->getMemberVariable(j)->cloneAndCast<VariableSignature>();
        signature->setName(inputVariable->getName() + T(".") + signature->getName());
        signature->setShortName(inputVariable->getShortName() + T(".") + signature->getShortName());
        outputType->addMemberVariable(context, signature);
      }
    }
    if (!outputType->getNumMemberVariables())
    {
      context.errorCallback(T("No member variables"));
      return VariableSignaturePtr();
    }
    return new VariableSignature(outputType, T("AllFeatures"));
  }

  struct Callback : public VariableGeneratorCallback
  {
    Callback(VariableGeneratorCallback& target)
      : target(target) {}
    size_t shift;

    virtual void sense(size_t index, double value)
      {target.sense(index + shift, value);}

    VariableGeneratorCallback& target;
  };

  virtual void computeVariables(const Variable* inputs, VariableGeneratorCallback& callback) const
  {
  /*  for (size_t i = 0; i < inputTypes.size(); ++i)
    {
      SparseDoubleObjectPtr input = inputs[i].dynamicCast<SparseDoubleObject>();
      jassert(input);
      res->appendValuesWithShift(input, shifts[i]);
    }*/
    jassert(false); 
    // FIXME: "LazyVariable"
  }

  virtual Variable computeOperator(const Variable* inputs) const
  {
    SparseDoubleObjectPtr res = new SparseDoubleObject(getOutputType());
    size_t v = 0;
    for (size_t i = 0; i < shifts.size(); ++i)
    {
      SparseDoubleObjectPtr input = inputs[i].dynamicCast<SparseDoubleObject>();
      jassert(input);
      res->appendValuesWithShift(input, shifts[i]);
    }
    return res;
  }

private:
  std::vector<size_t> shifts;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_FEATURE_GENERATOR_CONCATENATE_H_
