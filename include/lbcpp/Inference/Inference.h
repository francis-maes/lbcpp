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

/*
** Inference
*/
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
  virtual String getDescription(ExecutionContext& context, const Variable& input, const Variable& supervision) const;

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
  const InferenceOnlineLearnerPtr& getOnlineLearner() const;
  InferenceOnlineLearnerPtr getLastOnlineLearner() const;

  void addOnlineLearner(const InferenceOnlineLearnerPtr& learner, bool insertInFront = false);
  void getInferencesThatHaveAnOnlineLearner(ExecutionContext& context, std::vector<InferencePtr>& res) const;

  /*
  ** Parameters
  */
  Variable getParameters() const;
  Variable getParametersCopy(ExecutionContext& context) const;
  void setParameters(const Variable& parameters);

  /*
  ** Object
  */
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class InferenceClass;
  friend class InferenceContext;

  virtual void parametersChangedCallback() {}

  virtual Variable computeInference(InferenceContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;

  InferenceOnlineLearnerPtr onlineLearner;
  InferencePtr batchLearner;
  juce::ReadWriteLock parametersLock;
  Variable parameters;
};

extern ClassPtr inferenceClass;

// Decorator
extern DecoratorInferencePtr postProcessInference(InferencePtr inference, FunctionPtr postProcessingFunction);

// Reductions
extern VectorParallelInferencePtr oneAgainstAllClassificationInference(ExecutionContext& context, const String& name, EnumerationPtr classes, InferencePtr binaryClassifierModel);
extern StaticDecoratorInferencePtr rankingBasedClassificationInference(const String& name, InferencePtr rankingInference, EnumerationPtr classes);
extern VectorParallelInferencePtr parallelVoteInference(ExecutionContext& context, const String& name, size_t numVoters, InferencePtr voteInferenceModel, InferencePtr voteLearner);
extern SharedParallelInferencePtr sharedParallelVectorInference(const String& name, FunctionPtr sizeFunction, InferencePtr elementInference);

// Meta
extern InferencePtr runOnSupervisedExamplesInference(InferencePtr inference, bool doInParallel);
extern SharedParallelInferencePtr crossValidationInference(const String& name, EvaluatorPtr evaluator, InferencePtr inferenceModel, size_t numFolds);
extern StaticDecoratorInferencePtr callbackBasedDecoratorInference(const String& name, InferencePtr decoratedInference, InferenceCallbackPtr callback);

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_H_
