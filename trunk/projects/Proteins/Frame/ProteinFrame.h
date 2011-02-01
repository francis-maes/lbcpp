/*-----------------------------------------.---------------------------------.
| Filename: ProteinFrame.h                 | Protein Frame                   |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 14:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_FRAME_H_
# define LBCPP_PROTEIN_FRAME_H_

# include <lbcpp/lbcpp.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class FrameClass;
typedef ReferenceCountedObjectPtr<FrameClass> FrameClassPtr;
class Frame;
typedef ReferenceCountedObjectPtr<Frame> FramePtr;

class FrameOperatorSignature : public VariableSignature
{
public:
  FrameOperatorSignature(OperatorPtr operation, const std::vector<size_t>& inputs, const String& name, const String& shortName)
    : VariableSignature(TypePtr(), name, shortName), operation(operation), inputs(inputs) {}
  FrameOperatorSignature(OperatorPtr operation, size_t input, const String& name, const String& shortName)
    : VariableSignature(TypePtr(), name, shortName), operation(operation), inputs(1, input) {}

  const OperatorPtr& getOperator() const
    {return operation;}

  const std::vector<size_t>& getInputs() const
    {return inputs;}

protected:
  OperatorPtr operation;
  std::vector<size_t> inputs;
};

typedef ReferenceCountedObjectPtr<FrameOperatorSignature> FrameOperatorSignaturePtr;

class FrameClass : public DefaultClass
{
public:
  FrameClass(const String& name, TypePtr baseClass)
    : DefaultClass(name, baseClass) {}
  FrameClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
    : DefaultClass(templateType, templateArguments, baseClass) {}
  FrameClass() {}

  virtual bool isUnnamedType() const
    {return true;}

  size_t addMemberOperator(ExecutionContext& context, const OperatorPtr& operation, size_t input, const String& outputName = String::empty, const String& outputShortName = String::empty)
    {return addMemberVariable(context, new FrameOperatorSignature(operation, input, outputName, outputShortName));}

  size_t addMemberOperator(ExecutionContext& context, const OperatorPtr& operation, const std::vector<size_t>& inputs, const String& outputName = String::empty, const String& outputShortName = String::empty)
    {return addMemberVariable(context, new FrameOperatorSignature(operation, inputs, outputName, outputShortName));}

  virtual bool initialize(ExecutionContext& context)
  {
    for (size_t i = 0; i < variables.size(); ++i)
    {
      FrameOperatorSignaturePtr signature = variables[i].dynamicCast<FrameOperatorSignature>();
      if (signature && !initializeOperator(context, signature))
        return false;
    }
    return DefaultClass::initialize(context);
  }

private:
  bool initializeOperator(ExecutionContext& context, const FrameOperatorSignaturePtr& signature)
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
};

class Frame : public DenseGenericObject
{
public:
  Frame(ClassPtr frameClass)
    : DenseGenericObject(frameClass) {}
  Frame() {}

  bool isVariableComputed(size_t index) const
  {
    return index < variableValues.size() &&
      !thisClass->getMemberVariableType(index)->isMissingValue(variableValues[index]);
  }

  Variable getOrComputeVariable(size_t index)
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
 
  virtual Variable getVariable(size_t index) const
    {return const_cast<Frame* >(this)->getOrComputeVariable(index);}
};

typedef ReferenceCountedObjectPtr<Frame> FramePtr;

FrameClassPtr defaultProteinFrameClass(ExecutionContext& context);
FramePtr createProteinFrame(ExecutionContext& context, const ProteinPtr& protein, FrameClassPtr frameClass);

#if 0
/////////////////////////////////////////////////

extern ClassPtr proteinFrameClass;

class ProteinFrame : public Object
{
public:
  ProteinFrame(const ProteinPtr& protein);
  ProteinFrame() {}

  VectorPtr getPrimaryStructure() const
    {return primaryStructure;}

protected:
  friend class ProteinFrameClass;

  VectorPtr primaryStructure;
  ContainerPtr primaryStructureAccumulator;

  VectorPtr positionSpecificScoringMatrix;
  ContainerPtr positionSpecificScoringMatrixAccumulator;

  VectorPtr secondaryStructure;
  VectorPtr secondaryStructureLabels;
  ContainerPtr secondaryStructureSegments;

  VectorPtr residueFrames;
};

typedef ReferenceCountedObjectPtr<ProteinFrame> ProteinFramePtr;

#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FRAME_H_
