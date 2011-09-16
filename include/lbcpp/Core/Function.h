/*-----------------------------------------.---------------------------------.
| Filename: Function.h                     | Base class for Functions        |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_H_
# define LBCPP_CORE_FUNCTION_H_

# include "Variable.h"
# include "Pair.h"
# include "Vector.h"

namespace lbcpp
{
  
class FunctionCallback
{
public:
  virtual ~FunctionCallback() {}

  virtual void functionCalled(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs) {}
  virtual void functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output) {}
};

typedef FunctionCallback* FunctionCallbackPtr;

class OnlineLearner;
typedef ReferenceCountedObjectPtr<OnlineLearner> OnlineLearnerPtr;

class Evaluator;
typedef ReferenceCountedObjectPtr<Evaluator> EvaluatorPtr;

class ScoreObject;
typedef ReferenceCountedObjectPtr<ScoreObject> ScoreObjectPtr;

class Function : public Object
{
public:
  Function() : learning(false) {}

  /*
  ** Type checking
  */
  virtual size_t getNumRequiredInputs() const
    {jassert(false); return 0;}

  virtual size_t getMinimumNumRequiredInputs() const
    {return getNumRequiredInputs();}

  virtual size_t getMaximumNumRequiredInputs() const
    {return getNumRequiredInputs();}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {jassert(false); return anyType;}

  /*
  ** Static computation
  */
  virtual String getOutputPostFix() const
    {return T("Processed");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {jassert(false); return TypePtr();} // should be =0

  bool initialize(ExecutionContext& context, TypePtr inputType);
  bool initialize(ExecutionContext& context, ClassPtr inputClass);
  bool initialize(ExecutionContext& context, TypePtr inputType1, TypePtr inputType2);
  bool initialize(ExecutionContext& context, VariableSignaturePtr inputVariable);
  bool initialize(ExecutionContext& context, const std::vector<TypePtr>& inputTypes);
  bool initialize(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables);
  bool initializeWithInputsObjectClass(ExecutionContext& context, ClassPtr inputsObjectClass);

  bool isInitialized() const
    {return outputVariable;}

  /*
  ** Static Prototype
  */
  size_t getNumInputs() const
    {return inputVariables.size();}

  const VariableSignaturePtr& getInputVariable(size_t index) const
    {jassert(index < inputVariables.size()); return inputVariables[index];}

  const DynamicClassPtr& getInputsClass() const
    {return inputsClass;}

  const VariableSignaturePtr& getOutputVariable() const
    {return outputVariable;}

  TypePtr getOutputType() const
    {return outputVariable ? outputVariable->getType() : TypePtr();}

  /*
  ** Description
  */
  virtual String getDescription(ExecutionContext& context, const Variable* inputs) const
    {return T("Executing Function");}

  /*
  ** Callbacks
  */
  size_t getNumPreCallbacks() const
    {return preCallbacks.size();}
  
  void addPreCallback(const FunctionCallbackPtr& callback)
    {preCallbacks.push_back(callback);}

  size_t getNumPostCallbacks() const
    {return postCallbacks.size();}

  void addPostCallback(const FunctionCallbackPtr& callback)
    {postCallbacks.push_back(callback);}

  void removePostCallback(const FunctionCallbackPtr& callback);

  /*
  ** Learner
  */
  bool hasBatchLearner() const
    {return batchLearner;}

  const FunctionPtr& getBatchLearner() const
    {return batchLearner;}

  void setBatchLearner(const FunctionPtr& batchLearner);

  bool hasOnlineLearner() const
    {return onlineLearner;}

  const OnlineLearnerPtr& getOnlineLearner() const
    {return onlineLearner;}

  void setOnlineLearner(const OnlineLearnerPtr& onlineLearner)
    {this->onlineLearner = onlineLearner;}

  /*
  ** Evaluator
  */
  bool hasEvaluator() const
    {return evaluator;}

  const EvaluatorPtr& getEvaluator() const
    {return evaluator;}

  void setEvaluator(const EvaluatorPtr& evaluator)
    {this->evaluator = evaluator;}

  /*
  ** High level learning operations
  */
  bool checkIsInitialized(ExecutionContext& context) const;

  ScoreObjectPtr train(ExecutionContext& context, const ContainerPtr& trainingData, const ContainerPtr& validationData = ContainerPtr(), const String& scopeName = String::empty, bool returnLearnedFunction = false);
  ScoreObjectPtr train(ExecutionContext& context, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData = std::vector<ObjectPtr>(), const String& scopeName = String::empty, bool returnLearnedFunction = false);

  ScoreObjectPtr evaluate(ExecutionContext& context, const ContainerPtr& examples, const EvaluatorPtr& evaluator = EvaluatorPtr(), const String& scopeName = String::empty) const;
  ScoreObjectPtr evaluate(ExecutionContext& context, const std::vector<ObjectPtr>& examples, const EvaluatorPtr& evaluator = EvaluatorPtr(), const String& scopeName = String::empty) const;

  bool isCurrentlyLearning() const
    {return learning;}

  /*
  ** High level dynamic computation (calls callbacks and push into stack if requested)
  */
  Variable compute(ExecutionContext& context, const std::vector<Variable>& inputs) const;
  Variable compute(ExecutionContext& context, const Variable* inputs, size_t numInputs) const;
  Variable compute(ExecutionContext& context, const Variable& input) const;
  Variable compute(ExecutionContext& context, const Variable& input1, const Variable& input2) const;
  Variable compute(ExecutionContext& context, const Variable& input1, const Variable& input2, const Variable& input3) const;
  Variable compute(ExecutionContext& context, const Variable& input1, const Variable& input2, const Variable& input3, const Variable& input4) const;
  Variable computeWithInputsObject(ExecutionContext& context, const ObjectPtr& inputsObject) const;

  /*
  ** Object
  */
  virtual String toShortString() const;
  
  virtual ObjectPtr clone(ExecutionContext& context) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  virtual int __call(LuaState& state);

  lbcpp_UseDebuggingNewOperator

protected:
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {jassert(getNumInputs() == 1); return computeFunction(context, &input);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {jassert(getNumInputs() == 1); return computeFunction(context, *inputs);}

protected:
  friend class FunctionClass;
  friend class BatchLearner;
  
  std::vector<VariableSignaturePtr> inputVariables;
  VariableSignaturePtr outputVariable;

  std::vector<FunctionCallbackPtr> preCallbacks;
  std::vector<FunctionCallbackPtr> postCallbacks;

  FunctionPtr batchLearner;
  OnlineLearnerPtr onlineLearner;
  EvaluatorPtr evaluator;

  DynamicClassPtr inputsClass;
  bool learning;
};

extern ClassPtr functionClass;

/*
** Core functions
*/
extern FunctionPtr getVariableFunction(size_t variableIndex);
extern FunctionPtr getVariableFunction(const String& variableName);
extern FunctionPtr getElementFunction();
extern FunctionPtr getElementInVariableFunction(const String& variableName);

extern FunctionPtr setVariableFunction(size_t variableIndex);
extern FunctionPtr setVariableFunction(const String& variableName);

extern FunctionPtr loadFromFileFunction(TypePtr expectedType = objectClass); // File -> Object
extern FunctionPtr loadFromFilePairFunction(TypePtr expectedType1 = objectClass, TypePtr expectedType2 = objectClass);

extern FunctionPtr createObjectFunction(ClassPtr objectClass);
extern FunctionPtr createVectorFunction(FunctionPtr elementGeneratorFunction, bool transmitIndexToGeneratorFunction = true);
extern FunctionPtr createSymmetricMatrixFunction(FunctionPtr elementGeneratorFunction);

extern FunctionPtr convertToDoubleFunction(bool applyLogScale = false);

extern FunctionPtr composeFunction(const FunctionPtr& f, const FunctionPtr& g);

extern FunctionPtr signedScalarToProbabilityFunction();

extern FunctionPtr concatenateScoreObjectFunction();
extern FunctionPtr concatenateContainerFunction();

/*
** LuaFunction
*/
class LuaFunction : public Function
{
public:
  LuaFunction(lua_State* L, int functionReference, const std::vector<TypePtr>& inputTypes, TypePtr outputType);
  LuaFunction() : L(NULL) {}
  virtual ~LuaFunction();

  static int create(LuaState& state);

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  lbcpp_UseDebuggingNewOperator

protected:
  lua_State* L;
  int functionReference;
  std::vector<TypePtr> inputTypes;
  TypePtr outputType;
};

/*
** SimpleFunction base classes
*/
class SimpleFunction : public Function
{
public:
  SimpleFunction(TypePtr outputType, const String& outputPostFix)
    : outputType(outputType), outputPostFix(outputPostFix) {}

  virtual String getOutputPostFix() const
    {return outputPostFix;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return outputType;}

  lbcpp_UseDebuggingNewOperator

protected:
  TypePtr outputType;
  String outputPostFix;
};

/*
** Simple Unary Function
*/
class SimpleUnaryFunction : public SimpleFunction
{
public:
  SimpleUnaryFunction(TypePtr inputType, TypePtr outputType, const String& outputPostFix = String::empty)
    : SimpleFunction(outputType, outputPostFix), inputType(inputType) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return inputType;}

  lbcpp_UseDebuggingNewOperator

protected:
  TypePtr inputType;
};

typedef Variable (Object::*UnaryMemberFunctionPointer)(ExecutionContext&, const Variable& ) const; 

class MethodBasedUnaryFunction : public SimpleUnaryFunction
{
public:
  MethodBasedUnaryFunction(Object& pthis, UnaryMemberFunctionPointer impl,
                                TypePtr inputType, TypePtr outputType)
    : SimpleUnaryFunction(inputType, outputType), pthis(pthis), impl(impl) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return (pthis.*impl)(context, input);}

  lbcpp_UseDebuggingNewOperator

protected:
  Object& pthis;
  UnaryMemberFunctionPointer impl;
};

# define lbcppMemberUnaryFunction(ThisClass, Fun, InputType, OutputType) \
    FunctionPtr(new MethodBasedUnaryFunction(*this, \
                (UnaryMemberFunctionPointer)(&ThisClass::Fun), InputType, OutputType))

/*
** Simple Binary Function
*/
class SimpleBinaryFunction : public SimpleFunction
{
public:
  SimpleBinaryFunction(TypePtr inputType1, TypePtr inputType2, TypePtr outputType, const String& outputPostFix = String::empty)
    : SimpleFunction(outputType, outputPostFix), inputType1(inputType1), inputType2(inputType2) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? inputType2 : inputType1;}

  lbcpp_UseDebuggingNewOperator

protected:
  TypePtr inputType1;
  TypePtr inputType2;
};

typedef Variable (Object::*BinaryMemberFunctionPointer)(ExecutionContext&, const Variable& , const Variable& ) const; 

class MethodBasedBinaryFunction : public SimpleBinaryFunction
{
public:
  MethodBasedBinaryFunction(Object& pthis, BinaryMemberFunctionPointer impl,
                                TypePtr inputType1, TypePtr inputType2, TypePtr outputType)
    : SimpleBinaryFunction(inputType1, inputType2, outputType), pthis(pthis), impl(impl) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return (pthis.*impl)(context, inputs[0], inputs[1]);}

  lbcpp_UseDebuggingNewOperator

protected:
  Object& pthis;
  BinaryMemberFunctionPointer impl;
};

# define lbcppMemberBinaryFunction(ThisClass, Fun, InputType1, InputType2, OutputType) \
    FunctionPtr(new MethodBasedBinaryFunction(*this, \
                (BinaryMemberFunctionPointer)(&ThisClass::Fun), InputType1, InputType2, OutputType))

class ProxyFunction : public Function
{
public:
  const FunctionPtr& getImplementation() const
    {return implementation;}

  // Function
  virtual size_t getMinimumNumRequiredInputs() const
    {return 0;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const;
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ProxyFunctionClass;

  FunctionPtr implementation;

  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const = 0;
};

typedef ReferenceCountedObjectPtr<ProxyFunction> ProxyFunctionPtr;

class UnaryHigherOrderFunction : public Function
{
public:
  UnaryHigherOrderFunction(FunctionPtr baseFunction);
  UnaryHigherOrderFunction() {}

  virtual size_t getNumSubInputs(const ObjectPtr& inputsObject) const = 0;
  virtual void appendSubInputs(const ObjectPtr& inputsObject, std::vector<ObjectPtr>& res, size_t& index) const = 0;

  virtual String getOutputPostFix() const
    {return baseFunction->getOutputPostFix();}

  const FunctionPtr& getBaseFunction() const
    {return baseFunction;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class UnaryHigherOrderFunctionClass;

  FunctionPtr baseFunction;
};

extern ClassPtr unaryHigherOrderFunctionClass;
typedef ReferenceCountedObjectPtr<UnaryHigherOrderFunction> UnaryHigherOrderFunctionPtr;

extern UnaryHigherOrderFunctionPtr mapContainerFunction(const FunctionPtr& mapFunction);
extern UnaryHigherOrderFunctionPtr mapNContainerFunction(const FunctionPtr& mapFunction);

extern UnaryHigherOrderFunctionPtr mapMatrixFunction(const FunctionPtr& mapFunction);
extern UnaryHigherOrderFunctionPtr mapNMatrixFunction(const FunctionPtr& mapFunction);

extern UnaryHigherOrderFunctionPtr mapSymmetricMatrixFunction(const FunctionPtr& mapFunction, size_t minimumDistanceFromDiagonal = 0);
extern UnaryHigherOrderFunctionPtr mapNSymmetricMatrixFunction(const FunctionPtr& mapFunction, size_t minimumDistanceFromDiagonal = 0);

extern ProxyFunctionPtr mapNFunction(const FunctionPtr& mapFunction);

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_H_
