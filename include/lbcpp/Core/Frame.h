/*-----------------------------------------.---------------------------------.
| Filename: Frame.h                        | Frame                           |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2011 20:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FRAME_H_
# define LBCPP_CORE_FRAME_H_

# include "DynamicObject.h"
# include "../Function/Function.h"

namespace lbcpp
{

class FrameClass;
typedef ReferenceCountedObjectPtr<FrameClass> FrameClassPtr;
class Frame;
typedef ReferenceCountedObjectPtr<Frame> FramePtr;

class FrameOperatorSignature : public VariableSignature
{
public:
  FrameOperatorSignature(FunctionPtr function, const std::vector<size_t>& inputs, const String& name, const String& shortName)
    : VariableSignature(TypePtr(), name, shortName), function(function), inputs(inputs) {}
  FrameOperatorSignature(FunctionPtr function, size_t input, const String& name, const String& shortName)
    : VariableSignature(TypePtr(), name, shortName), function(function), inputs(1, input) {}
  FrameOperatorSignature() {}

  const FunctionPtr& getFunction() const
    {return function;}

  const std::vector<size_t>& getInputs() const
    {return inputs;}

protected:
  FunctionPtr function;
  std::vector<size_t> inputs;
};

typedef ReferenceCountedObjectPtr<FrameOperatorSignature> FrameOperatorSignaturePtr;

class FrameClass : public DefaultClass
{
public:
  FrameClass(const String& name, TypePtr baseClass = objectClass);
  FrameClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass);
  FrameClass() {}

  size_t addMemberOperator(ExecutionContext& context, const FunctionPtr& operation, size_t input, const String& outputName = String::empty, const String& outputShortName = String::empty);
  size_t addMemberOperator(ExecutionContext& context, const FunctionPtr& operation, size_t input1, size_t input2, const String& outputName = String::empty, const String& outputShortName = String::empty);
  size_t addMemberOperator(ExecutionContext& context, const FunctionPtr& operation, const std::vector<size_t>& inputs, const String& outputName = String::empty, const String& outputShortName = String::empty);

  virtual bool initialize(ExecutionContext& context);

private:
  bool initializeFunction(ExecutionContext& context, const FrameOperatorSignaturePtr& signature);
};

class Frame : public DenseGenericObject
{
public:
  Frame(ClassPtr frameClass);
  Frame() {}

  bool isVariableComputed(size_t index) const;
  Variable getOrComputeVariable(size_t index);
  void ensureAllVariablesAreComputed();

  virtual Variable getVariable(size_t index) const;
};

typedef ReferenceCountedObjectPtr<Frame> FramePtr;

class FrameBasedFunction : public Function
{
public:
  FrameBasedFunction(FrameClassPtr frameClass)
    : frameClass(frameClass) {}
  FrameBasedFunction() {}

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
    {return frameClass->getMemberVariable(frameClass->getNumMemberVariables() - 1);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    FramePtr frame(new Frame(frameClass));
    for (size_t i = 0; i < getNumInputs(); ++i)
      frame->setVariable(i, inputs[i]);
    return frame->getVariable(frameClass->getNumMemberVariables() - 1);
  }

protected:
  friend class FrameBasedFunctionClass;

  FrameClassPtr frameClass;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FRAME_H_
