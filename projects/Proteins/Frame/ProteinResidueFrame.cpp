/*-----------------------------------------.---------------------------------.
| Filename: ProteinResidueFrame.cpp        | Protein Residue Frame           |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 14:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinResidueFrame.h"
using namespace lbcpp;

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

class PerceptionToFeatureGeneratorWrapper : public FeatureGenerator
{
public:
  PerceptionToFeatureGeneratorWrapper(PerceptionPtr perception)
    : perception(perception) {}

  virtual VariableSignaturePtr initializeOperator(ExecutionContext& context)
  {
    if (!checkNumInputs(context, 1))
      return VariableSignaturePtr();
    VariableSignaturePtr inputVariable = getInputVariable(0);
    if (!checkInputType(context, 0, perception->getInputType()))
      return VariableSignaturePtr();

    return new VariableSignature(perception->getOutputType(), inputVariable->getName() + T("Perception"), inputVariable->getShortName() + T("p"));
  }
 
  virtual Variable computeOperator(const Variable* inputs) const
    {return perception->computeFunction(defaultExecutionContext(), inputs[0]);}

  virtual void computeVariables(const Variable* inputs, VariableGeneratorCallback& callback) const
  {
    jassert(false);
    // todo: wrapper
  }

protected:
  PerceptionPtr perception;
};

class EntropyOperator : public Operator
{
public:
  virtual VariableSignaturePtr initializeOperator(ExecutionContext& context)
  {
     if (!checkNumInputs(context, 1))
      return VariableSignaturePtr();
    VariableSignaturePtr inputVariable = getInputVariable(0);
    if (!checkInputType(context, 0, distributionClass(anyType)))
      return VariableSignaturePtr();
    return new VariableSignature(negativeLogProbabilityType, inputVariable->getName() + T("Entropy"), inputVariable->getShortName() + T("e"));
  }

  virtual Variable computeOperator(const Variable* inputs) const
    {return inputs[0].getObjectAndCast<Distribution>()->computeEntropy();}
};

FrameClassPtr lbcpp::defaultProteinSingleResidueFrameClass(ExecutionContext& context)
{
  FrameClassPtr res(new FrameClass(T("ProteinResidueFrame"), objectClass));

  // inputs
  size_t aaIndex = res->addMemberVariable(context, aminoAcidTypeEnumeration, T("aminoAcid"));
  size_t pssmIndex = res->addMemberVariable(context, enumerationDistributionClass(aminoAcidTypeEnumeration), T("pssmRow"));
  size_t ss3Index = res->addMemberVariable(context, enumerationDistributionClass(secondaryStructureElementEnumeration), T("ss3"));

  // feature generators
  std::vector<size_t> featureIndices;

  featureIndices.push_back(res->addMemberOperator(context, enumerationFeatureGenerator(), aaIndex));
  featureIndices.push_back(res->addMemberOperator(context, enumerationDistributionFeatureGenerator(), pssmIndex));
  size_t pssmEntropyIndex = res->addMemberOperator(context, new EntropyOperator(), pssmIndex);
  featureIndices.push_back(res->addMemberOperator(context, new PerceptionToFeatureGeneratorWrapper(defaultPositiveDoubleFeatures()), pssmEntropyIndex));

  featureIndices.push_back(res->addMemberOperator(context, enumerationDistributionFeatureGenerator(), ss3Index));
  size_t ss3EntropyIndex = res->addMemberOperator(context, new EntropyOperator(), ss3Index);
  featureIndices.push_back(res->addMemberOperator(context, new PerceptionToFeatureGeneratorWrapper(defaultPositiveDoubleFeatures()), ss3EntropyIndex));

  // all features
  res->addMemberOperator(context, new ConcatenateFeatureGenerator(), featureIndices, T("allFeatures"));

  return res->initialize(context) ? res : FrameClassPtr();
}

VectorPtr lbcpp::createProteinSingleResidueFrames(ExecutionContext& context, const FramePtr& proteinFrame, FrameClassPtr residueFrameClass)
{
  VectorPtr primaryStructure = proteinFrame->getVariable(0).getObjectAndCast<Vector>();
  VectorPtr pssm = proteinFrame->getVariable(1).getObjectAndCast<Vector>();
  jassert(primaryStructure && pssm);

  size_t n = primaryStructure->getNumElements();
  VectorPtr res = vector(residueFrameClass, n);

  VectorPtr ss3 = proteinFrame->getVariable(2).getObjectAndCast<Vector>();

  for (size_t i = 0; i < n; ++i)
  {
    FramePtr residueFrame(new Frame(residueFrameClass));
    residueFrame->setVariable(context, 0, primaryStructure->getElement(i));
    residueFrame->setVariable(context, 1, pssm->getElement(i));
    if (ss3)
      residueFrame->setVariable(context, 2, ss3->getElement(i));
    res->setElement(i, residueFrame);
  }
  return res;
}
