/*-----------------------------------------.---------------------------------.
| Filename: Function.h                     | Base class for Functions        |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_FUNCTION_H_
# define LBCPP_FUNCTION_FUNCTION_H_

# include "../Core/Variable.h"
# include "../Core/Pair.h"
# include "../Core/Vector.h"
# include "predeclarations.h"

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

class Function : public Object
{
public:
  Function() : numInputs(0), pushIntoStack(false) {}

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
  bool initialize(ExecutionContext& context, VariableSignaturePtr inputVariable);
  bool initialize(ExecutionContext& context, const std::vector<TypePtr>& inputTypes);
  bool initialize(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables);

  /*
  ** Static Prototype
  */
  size_t getNumInputs() const
    {return numInputs;}

  const DynamicClassPtr& getInputsClass() const
    {return inputsClass;}

  const VariableSignaturePtr& getOutputVariable() const
    {return outputVariable;}

  const TypePtr& getOutputType() const
    {return outputVariable->getType();}

  /*
  ** Dynamic computation
  */
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {jassert(getNumInputs() == 1); return computeFunction(context, &input);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {jassert(getNumInputs() == 1); return computeFunction(context, *inputs);}
 
  virtual String getDescription(ExecutionContext& context, const Variable* inputs) const
    {return T("FIXME");}

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

  void removePostCallback(const FunctionCallbackPtr& callback)
  {
    for (size_t i = 0; i < postCallbacks.size(); ++i)
      if (postCallbacks[i] == callback)
      {
        postCallbacks.erase(postCallbacks.begin() + i);
        return;
      }
  }

  /*
  ** Push into stack flag
  */
  void setPushIntoStackFlag(bool value)
    {pushIntoStack = value;}

  bool hasPushIntoStackFlag() const
    {return pushIntoStack;}

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
  ** High level learning operations
  */
  bool train(ExecutionContext& context, const ContainerPtr& trainingData, const ContainerPtr& validationData = ContainerPtr());
  bool evaluate(ExecutionContext& context, const ContainerPtr& examples, const EvaluatorPtr& evaluator) const;
  bool evaluate(ExecutionContext& context, const std::vector<ObjectPtr>& examples, const EvaluatorPtr& evaluator) const;

  /*
  ** High level dynamic computation (calls callbacks and push into stack if requested)
  */
  Variable compute(ExecutionContext& context, const std::vector<Variable>& inputs) const;
  Variable compute(ExecutionContext& context, const Variable* inputs) const;
  Variable compute(ExecutionContext& context, const Variable& input) const;
  Variable compute(ExecutionContext& context, const Variable& input1, const Variable& input2) const;
  Variable compute(ExecutionContext& context, const Variable& input1, const Variable& input2, const Variable& input3) const;
  Variable computeWithInputsObject(ExecutionContext& context, const ObjectPtr& inputsObject) const;

  /////////////////////////////////////////////////////////////
  // old
  virtual TypePtr getInputType() const
    {return anyType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return anyType;}

  virtual String getDescription(const Variable& input) const
    {return getClassName() + T("(") + input.toShortString() + T(")");}
  // -
  
  lbcpp_UseDebuggingNewOperator

protected:
  friend class FunctionClass;
  
  size_t numInputs;
  VariableSignaturePtr outputVariable;

  std::vector<FunctionCallbackPtr> preCallbacks;
  std::vector<FunctionCallbackPtr> postCallbacks;
  FunctionPtr batchLearner;
  OnlineLearnerPtr onlineLearner;
  bool pushIntoStack;

  DynamicClassPtr inputsClass;
};

extern ClassPtr functionClass;

// new
extern FunctionPtr getVariableFunction(size_t variableIndex);
extern FunctionPtr getVariableFunction(const String& variableName);
extern FunctionPtr getElementFunction();

// old
extern FunctionPtr identityFunction(TypePtr type);
extern FunctionPtr composeFunction(const FunctionPtr& f, const FunctionPtr& g);
extern FunctionPtr multiplyDoubleFunction();

extern FunctionPtr loadFromFileFunction(TypePtr expectedType = objectClass); // File -> Object
extern FunctionPtr loadFromFilePairFunction(TypePtr expectedType1 = objectClass, TypePtr expectedType2 = objectClass);

extern FunctionPtr setFieldFunction(size_t fieldIndex); // (Object,Any) Pair -> Object
extern FunctionPtr selectVariableFunction(int index);
extern FunctionPtr selectPairVariablesFunction(int index1 = -1, int index2 = -1, TypePtr inputPairClass = pairClass(anyType, anyType));
// -

class ProxyFunction : public Function
{
protected:
  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const = 0;
 
  virtual size_t getMinimumNumRequiredInputs() const
    {return 0;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    implementation = createImplementation(inputVariables);
    if (!implementation)
    {
      context.errorCallback(T("Could not create implementation in proxy operator"));
      return TypePtr();
    }
    if (!implementation->initialize(context, inputVariables))
      return TypePtr();

    const VariableSignaturePtr& v = implementation->getOutputVariable();
    outputName = v->getName();
    outputShortName = v->getShortName();
    return v->getType();
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {jassert(implementation); return implementation->computeFunction(context, input);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {jassert(implementation); return implementation->computeFunction(context, inputs);}

  FunctionPtr implementation;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_H_
