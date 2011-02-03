/*-----------------------------------------.---------------------------------.
| Filename: ProteinFrame.cpp               | Protein Frame                   |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProteinFrame.h"
#include "ProteinResidueFrame.h"
using namespace lbcpp;

/*
** FrameClass
*/
FrameClass::FrameClass(const String& name, TypePtr baseClass)
  : DefaultClass(name, baseClass) {}

FrameClass::FrameClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
  : DefaultClass(templateType, templateArguments, baseClass) {}

size_t FrameClass::addMemberOperator(ExecutionContext& context, const FunctionPtr& operation, size_t input, const String& outputName, const String& outputShortName)
  {return addMemberVariable(context, new FrameOperatorSignature(operation, input, outputName, outputShortName));}

size_t FrameClass::addMemberOperator(ExecutionContext& context, const FunctionPtr& operation, size_t input1, size_t input2, const String& outputName, const String& outputShortName)
{
  std::vector<size_t> inputs(2);
  inputs[0] = input1;
  inputs[1] = input2;
  return addMemberVariable(context, new FrameOperatorSignature(operation, inputs, outputName, outputShortName));
}

size_t FrameClass::addMemberOperator(ExecutionContext& context, const FunctionPtr& operation, const std::vector<size_t>& inputs, const String& outputName, const String& outputShortName)
  {return addMemberVariable(context, new FrameOperatorSignature(operation, inputs, outputName, outputShortName));}

bool FrameClass::initialize(ExecutionContext& context)
{
  for (size_t i = 0; i < variables.size(); ++i)
  {
    FrameOperatorSignaturePtr signature = variables[i].dynamicCast<FrameOperatorSignature>();
    if (signature && !initializeFunction(context, signature))
      return false;
  }
  return DefaultClass::initialize(context);
}

bool FrameClass::initializeFunction(ExecutionContext& context, const FrameOperatorSignaturePtr& signature)
{
  const FunctionPtr& function = signature->getFunction();
  jassert(function);
  const std::vector<size_t>& inputs = signature->getInputs();

  std::vector<VariableSignaturePtr> inputVariables(inputs.size());
  for (size_t i = 0; i < inputVariables.size(); ++i)
  {
    size_t inputIndex = inputs[i];
    if (inputIndex >= variables.size())
    {
      context.errorCallback(T("Invalid index: ") + String((int)inputIndex));
      return false;
    }
    inputVariables[i] = variables[inputIndex];
  }
  if (!function->initialize(context, inputVariables))
    return false;

  FrameOperatorSignaturePtr autoSignature = function->getOutputVariable();
  signature->setType(autoSignature->getType());
  if (signature->getName().isEmpty())
    signature->setName(autoSignature->getName());
  if (signature->getShortName().isEmpty())
    signature->setShortName(autoSignature->getShortName());
  if (signature->getDescription().isEmpty())
    signature->setDescription(autoSignature->getDescription());
  return true;
}

/*
** Frame
*/
Frame::Frame(ClassPtr frameClass)
  : DenseGenericObject(frameClass) {}

bool Frame::isVariableComputed(size_t index) const
{
  return index < variableValues.size() &&
    !thisClass->getMemberVariableType(index)->isMissingValue(variableValues[index]);
}

Variable Frame::getOrComputeVariable(size_t index)
{
  VariableSignaturePtr signature = thisClass->getMemberVariable(index);
  VariableValue& variableValue = getVariableValueReference(index);
  const TypePtr& type = signature->getType();
  if (!type->isMissingValue(variableValue))
    return Variable::copyFrom(type, variableValue);

  FrameOperatorSignaturePtr operatorSignature = signature.dynamicCast<FrameOperatorSignature>();
  if (!operatorSignature)
    return Variable();
  
  const std::vector<size_t>& inputIndices = operatorSignature->getInputs();
  std::vector<Variable> inputs(inputIndices.size());
  for (size_t i = 0; i < inputs.size(); ++i)
  {
    inputs[i] = getOrComputeVariable(inputIndices[i]);
    if (!inputs[i].exists())
      return Variable();
  }
  Variable value = operatorSignature->getFunction()->computeFunction(defaultExecutionContext(), &inputs[0]);
  setVariable(index, value);
  return value;
}

Variable Frame::getVariable(size_t index) const
  {return const_cast<Frame* >(this)->getOrComputeVariable(index);}

////////////////////////////////////////////////////

class PerceptionToFeatureGeneratorWrapper : public FeatureGenerator
{
public:
  PerceptionToFeatureGeneratorWrapper(PerceptionPtr perception)
    : perception(perception) {}

  virtual EnumerationPtr getFeaturesEnumeration(ExecutionContext& context, TypePtr& elementsType)
    {return variablesEnumerationEnumeration(perception->getOutputType());}

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (!checkNumInputs(context, 1))
      return VariableSignaturePtr();
    VariableSignaturePtr inputVariable = getInputVariable(0);
    if (!checkInputType(context, 0, perception->getInputType()))
      return VariableSignaturePtr();

    return new VariableSignature(perception->getOutputType(), inputVariable->getName() + T("Perception"), inputVariable->getShortName() + T("p"));
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return perception->computeFunction(defaultExecutionContext(), inputs[0]);}

  virtual void computeVariables(const Variable* inputs, VariableGeneratorCallback& callback) const
  {
    jassert(false);
    // todo: wrapper
  }

protected:
  PerceptionPtr perception;
};

class EntropyOperator : public Function
{
public:
  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
     if (!checkNumInputs(context, 1))
      return VariableSignaturePtr();
    VariableSignaturePtr inputVariable = getInputVariable(0);
    if (!checkInputType(context, 0, distributionClass(anyType)))
      return VariableSignaturePtr();
    return new VariableSignature(negativeLogProbabilityType, inputVariable->getName() + T("Entropy"), inputVariable->getShortName() + T("e"));
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return inputs[0].getObjectAndCast<Distribution>()->computeEntropy();}
};

////////////////////////////////////////////////////

class ForEachOperator : public Function
{
public:
  ForEachOperator(FunctionPtr function, const String& loopVariableName = T("i"))
    : function(function), loopVariableName(loopVariableName) {}

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (!function->initialize(context, inputVariables))
      return VariableSignaturePtr();
    VariableSignaturePtr elementsSignature = function->getOutputVariable();
    return new VariableSignature(vectorClass(elementsSignature->getType()), elementsSignature->getName() + T("Vector"), elementsSignature->getShortName() + T("v"));
  }

  // at each iteration, replaces the first input (n) by the loop counter (i in [0,n[)
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    std::vector<Variable> subInputs(getNumInputs());
    for (size_t i = 1; i < subInputs.size(); ++i)
      subInputs[i] = inputs[i];

    size_t n = (size_t)inputs[0].getInteger();
    VectorPtr res = vector(function->getOutputType(), n);
    for (size_t i = 0; i < n; ++i)
    {
      subInputs[0] = Variable(i);
      res->setElement(i, function->computeFunction(context, &subInputs[0]));
    }
    return res;
  }

protected:
  FunctionPtr function;
  String loopVariableName;
};

////////////////////////////////////////////////////

FrameClassPtr ProteinFrameFactory::createProteinFrameClass(ExecutionContext& context)
{
  contextFreeResidueFrame = new FrameClass(T("ContextFreeResidue"), objectClass);
  createContextFreeResidueFrameClass(context, contextFreeResidueFrame);
  if (!contextFreeResidueFrame->initialize(context))
    return FrameClassPtr();

  residueFrame = new FrameClass(T("Residue"), objectClass);
  createResidueFrameClass(context, residueFrame);
  if (!residueFrame->initialize(context))
    return FrameClassPtr();

  proteinFrame = new FrameClass(T("Protein"), objectClass);
  createProteinFrameClass(context, proteinFrame);
  if (!proteinFrame->initialize(context))
    return FrameClassPtr();

  return proteinFrame;
}

void ProteinFrameFactory::createProteinFrameClass(ExecutionContext& context, const FrameClassPtr& res)
{
  // inputs
  size_t lengthIndex = res->addMemberVariable(context, positiveIntegerType, T("length"));
  size_t aaIndex = res->addMemberVariable(context, vectorClass(aminoAcidTypeEnumeration), T("primaryStructure"), T("AA"));
  size_t pssmIndex = res->addMemberVariable(context, vectorClass(enumerationDistributionClass(aminoAcidTypeEnumeration)), T("positionSpecificScoringMatrix"), T("PSSM"));
  size_t ss3Index = res->addMemberVariable(context, vectorClass(enumerationDistributionClass(secondaryStructureElementEnumeration)), T("secondaryStructure"), T("SS3"));

  // sub frames features
  std::vector<size_t> contextFreeResidueInputs;
  contextFreeResidueInputs.push_back(aaIndex);
  contextFreeResidueInputs.push_back(pssmIndex);
  contextFreeResidueInputs.push_back(ss3Index);
  size_t contextFreeResidueFeatures = res->addMemberOperator(context, applyOnContainerOperator(new FrameBasedOperator(contextFreeResidueFrame)), contextFreeResidueInputs);
  size_t contextFreeResidueFeaturesSum = res->addMemberOperator(context, accumulateOperator(), contextFreeResidueFeatures);
  
  // residues 
  std::vector<size_t> residueFrameInputs;
  residueFrameInputs.push_back(lengthIndex);
  residueFrameInputs.push_back(contextFreeResidueFeatures);
  residueFrameInputs.push_back(contextFreeResidueFeaturesSum);
  res->addMemberOperator(context, new ForEachOperator(new FrameBasedOperator(residueFrame)), residueFrameInputs, T("residueFrames"));

  // primaryStructure
  res->addMemberOperator(context, accumulateOperator(), aaIndex);

  // pssm
  res->addMemberOperator(context, accumulateOperator(), pssmIndex);

  // secondary structure
  res->addMemberOperator(context, accumulateOperator(), ss3Index);
  size_t ss3LabelsIndex = res->addMemberOperator(context, discretizeOperator(), ss3Index);
  res->addMemberOperator(context, segmentContainerOperator(), ss3LabelsIndex);
  res->addMemberOperator(context, accumulateOperator(), ss3LabelsIndex);
}

void ProteinFrameFactory::createContextFreeResidueFrameClass(ExecutionContext& context, const FrameClassPtr& res)
{
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
  res->addMemberOperator(context, concatenateFeatureGenerator(), featureIndices, T("allFeatures"));
}

void ProteinFrameFactory::createResidueFrameClass(ExecutionContext& context, const FrameClassPtr& res)
{
  size_t positionIndex = res->addMemberVariable(context, positiveIntegerType, T("position"));
  
  TypePtr contextFreeResidueFeatureClass = contextFreeResidueFrame->getLastMemberVariable()->getType();
  size_t residueFeaturesArrayIndex = res->addMemberVariable(context, containerClass(contextFreeResidueFeatureClass), T("contextFreeResidueFeatures"));
  size_t residueSumFeaturesArrayIndex = res->addMemberVariable(context, containerClass(enumBasedDoubleVectorClass(variablesEnumerationEnumeration(contextFreeResidueFeatureClass))), T("featuressum"));
  
  std::vector<size_t> featureIndices;

  size_t windowIndex = res->addMemberOperator(context, windowVariableGenerator(15), residueFeaturesArrayIndex, positionIndex);
  
   // all features
  //res->addMemberOperator(context, concatenateFeatureGenerator(), featureIndices, T("allFeatures")); 
}


/*
VectorPtr createProteinResidueFrames(const FramePtr& proteinFrame, FrameClassPtr residueFrameClass)
{
  VectorPtr primaryStructure = proteinFrame->getVariable(0).getObjectAndCast<Vector>();
  size_t n = primaryStructure->getNumElements();
  
  VectorPtr res = vector(residueFrameClass, n);
  for (size_t i = 0; i < n; ++i)
  {
    
  }
  return res;
}*/

FramePtr ProteinFrameFactory::createFrame(const ProteinPtr& protein) const
{
  FramePtr res(new Frame(proteinFrame));
  res->setVariable(0, protein->getLength());
  res->setVariable(1, protein->getPrimaryStructure());
  res->setVariable(2, protein->getPositionSpecificScoringMatrix());
  
  ContainerPtr inputSecondaryStructure = protein->getSecondaryStructure();
  if (inputSecondaryStructure)
  {
    size_t n = inputSecondaryStructure->getNumElements();
    VectorPtr secondaryStructure = vector(enumerationDistributionClass(secondaryStructureElementEnumeration), n);
    for (size_t i = 0; i < n; ++i)
    {
      EnumerationDistributionPtr distribution = new EnumerationDistribution(secondaryStructureElementEnumeration);
      distribution->setProbability((size_t)inputSecondaryStructure->getElement(i).getInteger(), 1.0);
      secondaryStructure->setElement(i, distribution);
    }
    res->setVariable(3, secondaryStructure);
  }

  //res->setVariable(3, createProteinContextFreeResidueFrames(res, contextFreeResidueFrame));
  //res->setVariable(4, createProteinResidueFrames(res, residueFrame));
  return res;
}
