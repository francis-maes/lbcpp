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

size_t FrameClass::addMemberOperator(ExecutionContext& context, const OperatorPtr& operation, size_t input, const String& outputName, const String& outputShortName)
  {return addMemberVariable(context, new FrameOperatorSignature(operation, input, outputName, outputShortName));}

size_t FrameClass::addMemberOperator(ExecutionContext& context, const OperatorPtr& operation, const std::vector<size_t>& inputs, const String& outputName, const String& outputShortName)
  {return addMemberVariable(context, new FrameOperatorSignature(operation, inputs, outputName, outputShortName));}

bool FrameClass::initialize(ExecutionContext& context)
{
  for (size_t i = 0; i < variables.size(); ++i)
  {
    FrameOperatorSignaturePtr signature = variables[i].dynamicCast<FrameOperatorSignature>();
    if (signature && !initializeOperator(context, signature))
      return false;
  }
  return DefaultClass::initialize(context);
}

bool FrameClass::initializeOperator(ExecutionContext& context, const FrameOperatorSignaturePtr& signature)
{
  const OperatorPtr& operation = signature->getOperator();
  jassert(operation);
  const std::vector<size_t>& inputs = signature->getInputs();

  std::vector<TypePtr> inputTypes(inputs.size());
  for (size_t i = 0; i < inputTypes.size(); ++i)
    inputTypes[i] = variables[inputs[i]]->getType();
  if (!operation->initialize(context, inputTypes))
    return false;
  signature->setType(operation->getOutputType());
  //if (signature->getName().isEmpty())
  //  signature->setName(operation->getOutputName(...));
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
  Variable value = operatorSignature->getOperator()->computeOperator(&inputs[0]);
  setVariable(defaultExecutionContext(), index, value);
  return value;
}

Variable Frame::getVariable(size_t index) const
  {return const_cast<Frame* >(this)->getOrComputeVariable(index);}

////////////////////////////////////////////////////

/*
** ProteinFrame
*/
FrameClassPtr lbcpp::defaultProteinFrameClass(ExecutionContext& context)
{
  FrameClassPtr res(new FrameClass(T("ProteinFrame"), objectClass));

  // inputs
  size_t aaIndex = res->addMemberVariable(context, vectorClass(aminoAcidTypeEnumeration), T("primaryStructure"), T("AA"));
  size_t pssmIndex = res->addMemberVariable(context, vectorClass(enumerationDistributionClass(aminoAcidTypeEnumeration)), T("positionSpecificScoringMatrix"), T("PSSM"));
  size_t ss3Index = res->addMemberVariable(context, vectorClass(enumerationDistributionClass(secondaryStructureElementEnumeration)), T("secondaryStructure"), T("SS3"));

  size_t singleResidueFramesIndex = res->addMemberVariable(context, vectorClass(objectClass), T("singleResidueFrames"));

  // primaryStructure
  res->addMemberOperator(context, accumulateOperator(), aaIndex, T("primaryStructureAccumulator"), T("AAc"));

  // pssm
  res->addMemberOperator(context, accumulateOperator(), pssmIndex, T("positionSpecificScoringMatrixAccumulator"), T("PSSMc"));

  // secondary structure
  res->addMemberOperator(context, accumulateOperator(), ss3Index, T("secondaryStructureAccumulator"), T("SS3c"));
  size_t ss3LabelsIndex = res->addMemberOperator(context, discretizeOperator(), ss3Index, T("secondaryStructureLabels"), T("SS3d"));
  res->addMemberOperator(context, segmentContainerOperator(), ss3LabelsIndex, T("secondaryStructureSegments"), T("SS3ds"));
  res->addMemberOperator(context, accumulateOperator(), ss3LabelsIndex, T("secondaryStructureLabelsAccumulator"), T("SS3dc"));
  
  return res->initialize(context) ? res : FrameClassPtr();
}

FramePtr lbcpp::createProteinFrame(ExecutionContext& context, const ProteinPtr& protein, FrameClassPtr frameClass)
{
  FramePtr res(new Frame(frameClass));
  res->setVariable(context, 0, protein->getPrimaryStructure());
  res->setVariable(context, 1, protein->getPositionSpecificScoringMatrix());
  
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
    res->setVariable(context, 2, secondaryStructure);
  }

  res->setVariable(context, 3, createProteinSingleResidueFrames(context, res, defaultProteinSingleResidueFrameClass(context)));
  return res;
}
