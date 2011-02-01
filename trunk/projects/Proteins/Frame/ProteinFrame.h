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

  const OperatorPtr& getOperation() const
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
    const OperatorPtr& operation = signature->getOperation();
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

class Frame : public Object
{
public:
  Frame(ClassPtr frameClass)
    : Object(frameClass) {}
  Frame() {}

  virtual ~Frame()
  {
    for (size_t i = 0; i < variables.size(); ++i)
      thisClass->getMemberVariableType(i)->destroy(variables[i].first);
  }
    
  std::pair<VariableValue, double>& getVariableValueReference(size_t index)
  {
    jassert(index < thisClass->getNumMemberVariables());
    if (variables.size() <= index)
    {
      size_t i = variables.size();
      variables.resize(index + 1);
      while (i < variables.size())
      {
        variables[i].first = thisClass->getMemberVariableType(i)->getMissingValue();
        variables[i].second = 0.0;
        ++i;
      }
    }
    return variables[index];
  }

  virtual Variable getVariable(size_t index) const
  {
    TypePtr type = thisClass->getMemberVariableType(index);
    if (index < variables.size())
      return Variable::copyFrom(type, variables[index].first);
    else
      return Variable::missingValue(type);
  }

  virtual void setVariable(ExecutionContext& context, size_t index, const Variable& value)
    {setVariable(index, value, Time::getMillisecondCounterHiRes());}

  void setVariable(size_t index, const Variable& value, double time)
  {
    std::pair<VariableValue, double>& v = getVariableValueReference(index);
    value.copyTo(v.first);
    v.second = time;
  }

private:
  std::vector< std::pair<VariableValue, double> > variables;
};

typedef ReferenceCountedObjectPtr<Frame> FramePtr;

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

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FRAME_H_
