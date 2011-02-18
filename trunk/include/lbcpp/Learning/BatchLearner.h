/*-----------------------------------------.---------------------------------.
| Filename: BatchLearner.h                 | Batch Learners                  |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 19:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_BATCH_LEARNER_H_
# define LBCPP_LEARNING_BATCH_LEARNER_H_

# include <lbcpp/Function/Function.h>

namespace lbcpp
{

// Function, Container[VariableVector], optional Container[VariableVector] -> Function
class BatchLearner : public Function
{
public:
  BatchLearner()
    {numInputs = 3;} // tmp: necessaire tant qu'on ne sait pas trop d'ou appeler le Function::initialize

  virtual TypePtr getRequiredFunctionType() const
    {return functionClass;}

  virtual TypePtr getRequiredExamplesType() const
    {return objectClass;}
  
  virtual bool checkHasAtLeastOneExemples(const std::vector<ObjectPtr>& data) const
    {return data.size() != 0;}

  // Function
  virtual size_t getMinimumNumRequiredInputs() const
    {return 2;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return 3;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? getRequiredFunctionType() : (TypePtr)objectVectorClass(getRequiredExamplesType());}

  virtual String getOutputPostFix() const
    {return T("Learned");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return functionClass;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const FunctionPtr& function = inputs[0].getObjectAndCast<Function>();
    const ObjectVectorPtr& trainingData = inputs[1].getObjectAndCast<ObjectVector>();
    ObjectVectorPtr validationData = (getNumInputs() == 3 ? inputs[2].getObjectAndCast<ObjectVector>() : ObjectVectorPtr());
    return train(context, function, trainingData->getObjects(), validationData ? validationData->getObjects() : std::vector<ObjectPtr>());
  }

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const = 0;

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<BatchLearner> BatchLearnerPtr;

class BatchLearnerDecorator : public BatchLearner
{
public:
  BatchLearnerDecorator(BatchLearnerPtr decorated)
    : decorated(decorated) {}

  BatchLearnerDecorator() {}
  
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
    {return decorated->computeFunction(context, &input);} // FIXME: !!!

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
  friend class BatchLearnerDecoratorClass;
  
  BatchLearnerPtr decorated;
};

typedef ReferenceCountedObjectPtr<BatchLearnerDecorator> BatchLearnerDecoratorPtr;

extern BatchLearnerPtr proxyFunctionBatchLearner();
extern BatchLearnerPtr frameBasedFunctionBatchLearner();
extern BatchLearnerPtr stochasticBatchLearner(size_t maxIterations = 100, bool randomizeExamples = true);
extern BatchLearnerPtr stochasticBatchLearner(const std::vector<FunctionPtr>& functionsToLearn, size_t maxIterations = 100, bool randomizeExamples = true);

extern BatchLearnerDecoratorPtr supervisedExamplesBatchLearner(BatchLearnerPtr decorated);

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_H_
