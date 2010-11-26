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
# include "../Core/Variable.h"
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
class Inference : public Function
{
public:
  Inference(const String& name = T("Unnamed"))
    : name(name) {}
  virtual ~Inference();

  /*
  ** Types
  */
  virtual TypePtr getSupervisionType() const
    {return anyType;}

  virtual TypePtr getParametersType() const
    {return nilType;}

  // description
  virtual String getDescription(ExecutionContext& context, const Variable& input, const Variable& supervision) const;

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
  virtual String getName() const
    {return name;}

  virtual void setName(const String& name)
    {this->name = name;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return computeInference(context, input, Variable());}

    // todo: move this in protected
  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const = 0;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class InferenceClass;

  virtual void parametersChangedCallback() {}

  String name;
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
