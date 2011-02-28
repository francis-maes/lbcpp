/*-----------------------------------------.---------------------------------.
| Filename: BatchLearner.h                 | Batch Learners                  |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 19:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_BATCH_LEARNER_H_
# define LBCPP_LEARNING_BATCH_LEARNER_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{

// Function, Container[VariableVector], optional Container[VariableVector] -> Boolean
class BatchLearner : public Function
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return functionClass;}

  virtual TypePtr getRequiredExamplesType() const
    {return objectClass;}
  
  bool checkHasAtLeastOneExemples(const std::vector<ObjectPtr>& data) const
    {return data.size() != 0;}

  // Function
  virtual size_t getMinimumNumRequiredInputs() const
    {return 2;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return 3;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? getRequiredFunctionType() : (TypePtr)containerClass(getRequiredExamplesType());}

  virtual String getOutputPostFix() const
    {return T("Learned");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return booleanType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {jassertfalse; return Variable();}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const = 0;

  lbcpp_UseDebuggingNewOperator

protected:
  static ObjectVectorPtr makeObjectVector(const ContainerPtr& container);
  static bool trainSubFunction(ExecutionContext& context, const FunctionPtr& subFunction, const ContainerPtr& subTrainingData, const ContainerPtr& subValidationData);
};

typedef ReferenceCountedObjectPtr<BatchLearner> BatchLearnerPtr;

class DecoratorBatchLearner : public BatchLearner
{
public:
  DecoratorBatchLearner(BatchLearnerPtr decorated)
    : decorated(decorated) {}

  DecoratorBatchLearner() {}
  
  /* Batch Learner */
  virtual TypePtr getRequiredFunctionType() const
    {return decorated->getRequiredFunctionType();}
  
  virtual TypePtr getRequiredExamplesType() const
    {return decorated->getRequiredExamplesType();}
  
  virtual bool checkHasAtLeastOneExemples(const std::vector<ObjectPtr>& data) const
    {return decorated->checkHasAtLeastOneExemples(data);}
  
  /* Function - Type checking */
  virtual size_t getNumRequiredInputs() const
    {return decorated->getNumRequiredInputs();}

  virtual size_t getMinimumNumRequiredInputs() const
    {return decorated->getMinimumNumRequiredInputs();}
  
  virtual size_t getMaximumNumRequiredInputs() const
    {return decorated->getMaximumNumRequiredInputs();}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return decorated->getRequiredInputType(index, numInputs);}

  /* Function - Static computation */
  virtual String getOutputPostFix() const
    {return decorated->getOutputPostFix();}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return decorated->initializeFunction(context, inputVariables, outputName, outputShortName);}

  /* Function - Dynamic computation */
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return decorated->computeFunction(context, input);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return decorated->computeFunction(context, inputs);}
  
  virtual String getDescription(ExecutionContext& context, const Variable* inputs) const
    {return decorated->getDescription(context, inputs);}
  
  /* Object */
  virtual String toString() const
    {return decorated->toString();}

  virtual String toShortString() const
    {return decorated->toShortString();}

  virtual String getName() const
    {return decorated->getName();}
  
protected:
  friend class DecoratorBatchLearnerClass;
  
  BatchLearnerPtr decorated;
};

typedef ReferenceCountedObjectPtr<DecoratorBatchLearner> DecoratorBatchLearnerPtr;

extern BatchLearnerPtr proxyFunctionBatchLearner();
extern BatchLearnerPtr compositeFunctionBatchLearner();
extern BatchLearnerPtr stochasticBatchLearner(size_t maxIterations = 100, bool randomizeExamples = true);
extern BatchLearnerPtr stochasticBatchLearner(const std::vector<FunctionPtr>& functionsToLearn, size_t maxIterations = 100, bool randomizeExamples = true);
extern BatchLearnerPtr mapContainerFunctionBatchLearner();

extern DecoratorBatchLearnerPtr filterUnsupervisedExamplesBatchLearner(BatchLearnerPtr decorated);

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_H_
