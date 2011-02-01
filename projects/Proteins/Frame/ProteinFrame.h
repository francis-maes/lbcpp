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
  FrameClass(const String& name, TypePtr baseClass);
  FrameClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass);
  FrameClass() {}

  virtual bool isUnnamedType() const
    {return true;}

  size_t addMemberOperator(ExecutionContext& context, const OperatorPtr& operation, size_t input, const String& outputName = String::empty, const String& outputShortName = String::empty);
  size_t addMemberOperator(ExecutionContext& context, const OperatorPtr& operation, const std::vector<size_t>& inputs, const String& outputName = String::empty, const String& outputShortName = String::empty);

  virtual bool initialize(ExecutionContext& context);

private:
  bool initializeOperator(ExecutionContext& context, const FrameOperatorSignaturePtr& signature);
};

class Frame : public DenseGenericObject
{
public:
  Frame(ClassPtr frameClass);
  Frame() {}

  bool isVariableComputed(size_t index) const;
  Variable getOrComputeVariable(size_t index);

  virtual Variable getVariable(size_t index) const;
};

typedef ReferenceCountedObjectPtr<Frame> FramePtr;

// Proteins ...
FrameClassPtr defaultProteinFrameClass(ExecutionContext& context);
FramePtr createProteinFrame(ExecutionContext& context, const ProteinPtr& protein, FrameClassPtr frameClass);

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FRAME_H_
