/*-----------------------------------------.---------------------------------.
| Filename: CompositeFunction.h            | Composite Function              |
| Author  : Francis Maes                   |                                 |
| Started : 18/02/2011 19:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_COMPOSITE_FUNCTION_H_
# define LBCPP_CORE_COMPOSITE_FUNCTION_H_

# include "Function.h"
# include "DynamicObject.h"

namespace lbcpp
{

/*
** CompositeFunction
*/
class CompositeFunctionBuilder;
class CompositeFunction : public Function
{
public:
  CompositeFunction();

  virtual void buildFunction(CompositeFunctionBuilder& builder) = 0;

  // Function
  virtual size_t getMinimumNumRequiredInputs() const
    {return 0;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return anyType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);

  /*
  ** Sub Functions
  */
  size_t getNumSubFunctions() const
    {return functions.size();}

  const FunctionPtr& getSubFunction(size_t index) const
    {jassert(index < functions.size()); return functions[index];}

  const std::vector<size_t>& getSubFunctionInputs(size_t index) const
    {jassert(index < functionInputs.size()); return functionInputs[index];}

  /*
  ** Constants
  */
  size_t getNumConstants() const
    {return constants.size();}

  const Variable& getConstant(size_t index) const
    {jassert(index < constants.size()); return constants[index];}

  /*
  ** Steps
  */
  enum StepType
  {
    inputStep,
    constantStep,
    functionStep
  };
  
  size_t getNumSteps() const
    {return steps.size();}

  StepType getStepType(size_t index) const
    {jassert(index < steps.size()); return steps[index].first;}

  size_t getStepArgument(size_t index) const
    {jassert(index < steps.size()); return steps[index].second;}

  /*
  ** State
  */
  const DynamicClassPtr& getStateClass() const
    {return stateClass;}

  ObjectPtr makeInitialState(const ObjectPtr& inputsObject) const;
  ObjectPtr makeSubInputsObject(size_t stepNumber, const ObjectPtr& state) const;
  void updateState(ExecutionContext& context, size_t stepNumber, ObjectPtr& state) const;

  /*
  ** Object
  */
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  lbcpp_UseDebuggingNewOperator

protected:
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

protected:
  friend class CompositeFunctionClass;
  friend class CompositeFunctionBuilder;

  DynamicClassPtr stateClass;
  
  std::vector<Variable> constants;
  std::vector<FunctionPtr> functions;
  std::vector<std::vector<size_t> > functionInputs;
  size_t maxNumFunctionInputs;

  std::vector< std::pair<StepType, size_t> > steps; // signature and argument index
};

typedef ReferenceCountedObjectPtr<CompositeFunction> CompositeFunctionPtr;
extern ClassPtr compositeFunctionClass;

/*
** CompositeFunctionBuilder
*/
class CompositeFunctionBuilder
{
public:
  CompositeFunctionBuilder(ExecutionContext& context, CompositeFunctionPtr function, const std::vector<VariableSignaturePtr>& inputVariables);

  size_t getNumProvidedInputs() const
    {return inputVariables.size();}

  VariableSignaturePtr getProvidedInput(size_t index) const
    {jassert(index < inputVariables.size()); return inputVariables[index];}

  const std::vector<VariableSignaturePtr>& getProvidedInputs() const
    {return inputVariables;}

  size_t invalidIndex() const
    {return (size_t)-1;}

  size_t addInput(TypePtr type, const String& optionalName = String::empty, const String& optionalShortName = String::empty);

  size_t addConstant(const Variable& value, const String& optionalName = String::empty, const String& optionalShortName = String::empty);

  size_t addFunction(const FunctionPtr& function, size_t input, const String& optionalName = String::empty, const String& optionalShortName = String::empty);
  size_t addFunction(const FunctionPtr& function, size_t input1, size_t input2, const String& optionalName = String::empty, const String& optionalShortName = String::empty);
  size_t addFunction(const FunctionPtr& function, const std::vector<size_t>& inputs, const String& optionalName = String::empty, const String& optionalShortName = String::empty);

  void startSelection();
  void addInSelection(size_t index) {currentSelection.push_back(index);}
  const std::vector<size_t>& finishSelection();
  size_t finishSelectionWithFunction(const FunctionPtr& function, const String& optionalName = String::empty, const String& optionalShortName = String::empty);

  TypePtr getOutputType() const;

  ExecutionContext& getContext()
    {return context;}

  bool hasFailed() const
    {return failed;}

private:
  ExecutionContext& context;
  CompositeFunctionPtr function;
  std::vector<VariableSignaturePtr> inputVariables;
  size_t numInputs;
  bool failed;

  std::vector<size_t> currentSelection;

  size_t addVariable(TypePtr type, const String& name, const String& shortName, CompositeFunction::StepType stepType, size_t stepArgument);
  size_t returnError()
    {failed = true; return invalidIndex();}
};

/*
** MethodBasedCompositeFunction
*/
typedef void (Object::*FunctionBuildFunction)(CompositeFunctionBuilder& builder) const; 

class MethodBasedCompositeFunction : public CompositeFunction
{
public:
  MethodBasedCompositeFunction(ObjectPtr factory, FunctionBuildFunction buildFunctionFunction)
    : factory(factory), buildFunctionFunction(buildFunctionFunction) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    Object& object = *factory;
    (object.*buildFunctionFunction)(builder);
    factory = ObjectPtr(); // free reference to factory
  }

protected:
  ObjectPtr factory;
  FunctionBuildFunction buildFunctionFunction;
};

inline void variableToNative(ExecutionContext& context, CompositeFunction::StepType& dest, const Variable& source)
  {jassert(source.isInteger()); dest = (CompositeFunction::StepType)source.getInteger();}

template<class TT>
inline void variableToNative(ExecutionContext& context, std::pair<CompositeFunction::StepType, TT>& dest, const Variable& source)
{
  jassert(source.isObject());
  const PairPtr& sourcePair = source.getObjectAndCast<Pair>(context);
  if (sourcePair)
  {
    lbcpp::variableToNative(context, dest.first, sourcePair->getFirst());
    lbcpp::variableToNative(context, dest.second, sourcePair->getSecond());
  }
  else
  {
    dest.first = CompositeFunction::inputStep;
    dest.second = TT();
  }
}

template<class TT>
inline void variableToNative(ExecutionContext& context, std::vector< std::pair<CompositeFunction::StepType, TT> >& dest, const Variable& source)
{
  jassert(source.isObject());
  const VectorPtr& sourceVector = source.getObjectAndCast<Vector>(context);
  if (sourceVector)
  {
    dest.resize(sourceVector->getNumElements());
    for (size_t i = 0; i < dest.size(); ++i)
      lbcpp::variableToNative(context, dest[i], sourceVector->getElement(i));
  }
  else
    dest.clear();
}

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_COMPOSITE_FUNCTION_H_
