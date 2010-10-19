/*-----------------------------------------.---------------------------------.
| Filename: Inference.h                    | Inference base class            |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 19:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_H_
# define LBCPP_INFERENCE_STEP_H_

# include "predeclarations.h"
# include "../Data/Variable.h"
# include "../Data/Container.h"
# include "../Data/RandomVariable.h"

namespace lbcpp
{

class Inference : public NameableObject
{
public:
  Inference(const String& name = T("Unnamed"))
    : NameableObject(name) {}

  virtual TypePtr getInputType() const
    {return anyType;}

  virtual TypePtr getSupervisionType() const
    {return anyType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return anyType;}

  virtual String getDescription(const Variable& input, const Variable& supervision) const;

  enum ReturnCode
  {
    finishedReturnCode = 0,
    canceledReturnCode,
    errorReturnCode,
  };

  // Used in SharedParallelInference before and after a block of many run() calls
  virtual void beginRunSession() {}
  virtual void endRunSession() {}

  /*
  ** Batch Learner
  */
  const InferencePtr& getBatchLearner() const
    {return batchLearner;}
    
  void setBatchLearner(InferencePtr batchLearner);

  /*
  ** Online Learner
  */
  const InferenceOnlineLearnerPtr& getOnlineLearner() const
    {return onlineLearner;}

  void setOnlineLearner(InferenceOnlineLearnerPtr learner)
    {this->onlineLearner = learner;}

  void getInferencesThatHaveAnOnlineLearner(std::vector<InferencePtr>& res) const;

  /*
  ** Object
  */
  virtual void clone(ObjectPtr target) const;

  juce_UseDebuggingNewOperator

protected:
  friend class InferenceClass;
  friend class InferenceContext;

  virtual Variable run(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;

  InferenceOnlineLearnerPtr onlineLearner;
  InferencePtr batchLearner;
};

extern ClassPtr inferenceClass;

/*
** Decorator inference
*/
extern DecoratorInferencePtr postProcessInference(InferencePtr inference, FunctionPtr postProcessingFunction);

/*
** Numerical Inference
*/

// Atomic
extern InferencePtr linearInference(const String& name, PerceptionPtr perception);
extern InferencePtr multiLinearInference(const String& name, PerceptionPtr perception, ClassPtr outputClass);
extern InferencePtr transferFunctionDecoratorInference(const String& name, InferencePtr decoratedInference, ScalarFunctionPtr transferFunction);

// Binary Classification
extern InferencePtr binaryLinearSVMInference(InferencePtr scoreInference);
extern InferencePtr binaryLinearSVMInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern InferencePtr binaryLogisticRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));

// Regression
extern InferencePtr squareRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern InferencePtr absoluteRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern InferencePtr dihedralAngleRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));

// MultiClass Classification
extern StaticDecoratorInferencePtr multiClassLinearSVMInference(PerceptionPtr perception, EnumerationPtr classes, InferenceOnlineLearnerPtr learner, bool updateOnlyMostViolatedClasses = false, const String& name = T("unnamed"));
extern StaticDecoratorInferencePtr multiClassMaxentInference(PerceptionPtr perception, EnumerationPtr classes, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));

/*
** Decision Tree Inference
*/
extern InferencePtr regressionExtraTreeInference(const String& name, PerceptionPtr perception, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);
extern InferencePtr binaryClassificationExtraTreeInference(const String& name, PerceptionPtr perception, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);
extern InferencePtr classificationExtraTreeInference(const String& name, PerceptionPtr perception, EnumerationPtr classes, size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);

/*
** Reduction
*/
extern VectorParallelInferencePtr oneAgainstAllClassificationInference(const String& name, EnumerationPtr classes, InferencePtr binaryClassifierModel);
extern VectorParallelInferencePtr parallelVoteInference(const String& name, size_t numVoters, InferencePtr voteInferenceModel, InferencePtr voteLearner);
extern SharedParallelInferencePtr sharedParallelVectorInference(const String& name, FunctionPtr sizeFunction, InferencePtr elementInference);

/*
** Batch Learners
*/
// Input: (Inference, trainingData) pair; trainingData = container of (input, supervision) pairs
// Supervision: None
// Output: None (side-effect on input Inference)
extern InferencePtr dummyInferenceLearner();
extern InferencePtr staticSequentialInferenceLearner();
extern ParallelInferencePtr staticParallelInferenceLearner();
extern DecoratorInferencePtr sharedParallelInferenceLearner(bool filterUnsupervisedExamples = true);
extern ParallelInferencePtr parallelVoteInferenceLearner();
extern SequentialInferencePtr onlineToBatchInferenceLearner();
extern DecoratorInferencePtr decoratorInferenceLearner();
extern DecoratorInferencePtr postProcessInferenceLearner();

extern VectorSequentialInferencePtr multiPassInferenceLearner();
extern VectorSequentialInferencePtr multiPassInferenceLearner(InferencePtr firstLearner, InferencePtr secondLearner);

extern InferencePtr initializeByCloningInferenceLearner(InferencePtr inferenceToClone);

// Meta
extern InferencePtr runOnSupervisedExamplesInference(InferencePtr inference, bool doInParallel);
extern SharedParallelInferencePtr crossValidationInference(const String& name, EvaluatorPtr evaluator, InferencePtr inferenceModel, size_t numFolds);
extern StaticDecoratorInferencePtr callbackBasedDecoratorInference(const String& name, InferencePtr decoratedInference, InferenceCallbackPtr callback);

/*
** InferenceState
*/
class InferenceState : public Object
{
public:
  InferenceState(const Variable& input, const Variable& supervision)
    : input(input), supervision(supervision) {}

  const Variable& getInput() const
    {return input;}

  const Variable& getSupervision() const
    {return supervision;}

protected:
  Variable input;
  Variable supervision;
};

typedef ReferenceCountedObjectPtr<InferenceState> InferenceStatePtr;

template<class BaseClass>
class InferenceLearner : public BaseClass
{
public:
  virtual ClassPtr getTargetInferenceClass() const = 0;

  virtual TypePtr getInputType() const
    {return pairClass(getTargetInferenceClass(), containerClass(pairClass(anyType, anyType)));}

  virtual TypePtr getSupervisionType() const
    {return nilType;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return nilType;}

  virtual String getDescription(const Variable& input, const Variable& supervision) const
  {
    InferencePtr targetInference = getInference(input);
    ContainerPtr trainingData = getTrainingData(input);
    return T("Learning ") + targetInference->getName() + T(" with ") + 
      String((int)trainingData->getNumElements()) + T(" ") + trainingData->getElementsType()->getTemplateArgument(0)->getName() + T("(s)");
  }

protected:
  InferencePtr getInference(const Variable& input) const
    {return input[0].getObjectAndCast<Inference>();}

  template<class T>
  ReferenceCountedObjectPtr<T> getInferenceAndCast(const Variable& input) const
    {return input[0].getObjectAndCast<T>();}

  ContainerPtr getTrainingData(const Variable& input) const
    {return input[1].getObjectAndCast<Container>();}
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_H_
