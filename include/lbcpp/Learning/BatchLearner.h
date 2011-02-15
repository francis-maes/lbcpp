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

  // Function
  virtual size_t getMinimumNumRequiredInputs() const
    {return 2;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return 3;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? functionClass : objectVectorClass(getRequiredExamplesType());}

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

  virtual FunctionPtr train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const = 0;
};

typedef ReferenceCountedObjectPtr<BatchLearner> BatchLearnerPtr;


BatchLearnerPtr frameBasedFunctionBatchLearner();
BatchLearnerPtr stochasticBatchLearner(const std::vector<FunctionPtr>& functionsToLearn, EvaluatorPtr evaluator,
                        size_t maxIterations = 100,
                        StoppingCriterionPtr stoppingCriterion = StoppingCriterionPtr(),
                        bool randomizeExamples = true,
                        bool restoreBestParametersWhenDone = true);

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_H_
