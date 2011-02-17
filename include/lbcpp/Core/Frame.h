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

  size_t getNumInputs() const
    {return inputs.size();}

  size_t getInputIndex(size_t i) const
    {jassert(i < inputs.size()); return inputs[i];}

  const std::vector<size_t>& getInputIndices() const
    {return inputs;}

protected:
  friend class FrameOperatorSignatureClass;

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

  virtual ClassPtr getClass() const;
  virtual bool initialize(ExecutionContext& context);

private:
  bool initializeFunctionTypes(ExecutionContext& context, const FrameOperatorSignaturePtr& signature);
};

class Frame : public DenseGenericObject
{
public:
  Frame(ClassPtr frameClass);
  Frame(ClassPtr frameClass, const Variable& firstVariable);
  Frame(ClassPtr frameClass, const Variable& firstVariable, const Variable& secondVariable);
  Frame(ClassPtr frameClass, const Variable& firstVariable, const Variable& secondVariable, const Variable& thirdVariable);
  Frame() {}

  bool isVariableComputed(size_t index) const;
  Variable getOrComputeVariable(size_t index);
  void ensureAllVariablesAreComputed();

  virtual Variable getVariable(size_t index) const;
};

typedef ReferenceCountedObjectPtr<Frame> FramePtr;

class FrameBasedFunction : public CompositeFunction
{
public:
  FrameBasedFunction(FrameClassPtr frameClass = FrameClassPtr())
    : frameClass(frameClass) {}

  // CompositeFunction
  virtual size_t getNumSubFunctions() const
    {return subFunctions.size();}

  virtual const FunctionPtr& getSubFunction(size_t index) const
    {jassert(index < subFunctions.size()); return subFunctions[index];}

  // Function
  virtual size_t getMinimumNumRequiredInputs() const
    {return 0;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return anyType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  // FrameBasedFunction
  const FrameClassPtr& getFrameClass() const
    {return frameClass;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class FrameBasedFunctionClass;

  FrameClassPtr frameClass;
  std::vector<FunctionPtr> subFunctions;
};

typedef ReferenceCountedObjectPtr<FrameBasedFunction> FrameBasedFunctionPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FRAME_H_
