/*-----------------------------------------.---------------------------------.
| Filename: Inference.h                    | Inference base class            |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 19:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_H_
# define LBCPP_INFERENCE_H_

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
  virtual ~Inference();

  /*
  ** Types
  */
  virtual TypePtr getInputType() const
    {return anyType;}

  virtual TypePtr getSupervisionType() const
    {return anyType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return anyType;}

  virtual TypePtr getParametersType() const
    {return nilType;}

  // description
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

  void addOnlineLearner(const InferenceOnlineLearnerPtr& learner, bool insertInFront = false);
  void getInferencesThatHaveAnOnlineLearner(std::vector<InferencePtr>& res) const;

  /*
  ** Parameters
  */
  Variable getParameters() const;
  Variable getParametersCopy() const;
  void setParameters(const Variable& parameters);

  /*
  ** Object
  */
  virtual void clone(const ObjectPtr& target) const;

  juce_UseDebuggingNewOperator

protected:
  friend class InferenceClass;
  friend class InferenceContext;

  virtual void parametersChangedCallback() {}

  virtual Variable run(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;

  InferenceOnlineLearnerPtr onlineLearner;
  InferencePtr batchLearner;
  juce::ReadWriteLock parametersLock;
  Variable parameters;
};

extern ClassPtr inferenceClass;

/*
** Decorator inference
*/
extern DecoratorInferencePtr postProcessInference(InferencePtr inference, FunctionPtr postProcessingFunction);

/*
** Reductions
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

#endif //!LBCPP_INFERENCE_H_
