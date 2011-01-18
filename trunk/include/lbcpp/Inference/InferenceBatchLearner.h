/*-----------------------------------------.---------------------------------.
| Filename: InferenceBatchLearner.h        | Inference Batch Learners        |
| Author  : Francis Maes                   |                                 |
| Started : 01/11/2010 15:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_H_

# include "Inference.h"
# include <lbcpp/Core/Pair.h>
# include <lbcpp/Core/Vector.h>
# include <lbcpp/Optimizer/Optimizer.h>

namespace lbcpp
{

class EvaluateBatchLearnerObjectiveFunction;
typedef ReferenceCountedObjectPtr<EvaluateBatchLearnerObjectiveFunction> EvaluateBatchLearnerObjectiveFunctionPtr;

extern ClassPtr inferenceExampleVectorClass;

class InferenceExampleVector : public BuiltinVector<std::pair<Variable, Variable>, Pair>
{
public:
  typedef BuiltinVector<std::pair<Variable, Variable>, Pair> BaseClass;

  InferenceExampleVector(const std::vector<std::pair<Variable, Variable> >& examples)
    : BaseClass(inferenceExampleVectorClass, examples) {}
  InferenceExampleVector(size_t numExamples)
    : BaseClass(inferenceExampleVectorClass, numExamples, std::make_pair(Variable(), Variable())) {}
  InferenceExampleVector() {}

  virtual TypePtr getElementsType() const
    {return pairClass(variableType, variableType);}

  virtual Variable getElement(size_t index) const
    {return new Pair(BaseClass::values[index]);}
};

typedef ReferenceCountedObjectPtr<InferenceExampleVector> InferenceExampleVectorPtr;

class InferenceBatchLearnerInput : public Object
{
public:
  InferenceBatchLearnerInput(const InferencePtr& targetInference, const InferenceExampleVectorPtr& trainingExamples, const InferenceExampleVectorPtr& validationExamples);
  InferenceBatchLearnerInput(const InferencePtr& targetInference, const ContainerPtr& trainingExamples, const ContainerPtr& validationExamples);
  InferenceBatchLearnerInput(const InferencePtr& targetInference, size_t numTrainingExamples = 0, size_t numValidationExamples = 0);
  InferenceBatchLearnerInput() {}

  /*
  ** Target Inference
  */
  const InferencePtr& getTargetInference() const
    {return targetInference;}
  
  void setTargetInference(const InferencePtr& targetInference);

  /*
  ** Training Data
  */
  size_t getNumTrainingExamples() const
    {return trainingData ? trainingData->size() : 0;}

  const std::pair<Variable, Variable>& getTrainingExample(size_t i) const
    {return trainingData->get(i);}
  
  std::pair<Variable, Variable>& getTrainingExample(size_t i)
    {return trainingData->get(i);}

  void addTrainingExample(const Variable& input, const Variable& supervision)
    {trainingData->append(std::make_pair(input, supervision));}

  const InferenceExampleVectorPtr& getTrainingExamples() const
    {return trainingData;}

  /*
  ** Validation Data
  */
  size_t getNumValidationExamples() const
    {return validationData ? validationData->size() : 0;}

  const std::pair<Variable, Variable>& getValidationExample(size_t i) const
    {jassert(i < validationData->size()); return validationData->get(i);}

  std::pair<Variable, Variable>& getValidationExample(size_t i)
    {jassert(i < validationData->size()); return validationData->get(i);}

  void addValidationExample(const Variable& input, const Variable& supervision)
    {validationData->append(std::make_pair(input, supervision));}

  const InferenceExampleVectorPtr& getValidationExamples() const
    {return validationData;}

  /*
  ** All Data
  */
  size_t getNumExamples() const;
  const std::pair<Variable, Variable>& getExample(size_t i) const;
  std::pair<Variable, Variable>& getExample(size_t i);
  void setExample(size_t i, const Variable& input, const Variable& supervision);

private:
  friend class InferenceBatchLearnerInputClass;

  InferencePtr targetInference;
  InferenceExampleVectorPtr trainingData;
  InferenceExampleVectorPtr validationData;
};

extern ClassPtr inferenceBatchLearnerInputClass(TypePtr inferenceClass);
typedef ReferenceCountedObjectPtr<InferenceBatchLearnerInput> InferenceBatchLearnerInputPtr;

template<class BaseClass>
class InferenceBatchLearner : public BaseClass
{
public:
  virtual ClassPtr getTargetInferenceClass() const = 0;

  virtual TypePtr getInputType() const
    {return inferenceBatchLearnerInputClass(getTargetInferenceClass());}

  virtual TypePtr getSupervisionType() const
    {return nilType;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return nilType;}

  virtual String getDescription(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    return T("Learning ") + learnerInput->getTargetInference()->getName() + T(" with ") + 
      String((int)learnerInput->getNumTrainingExamples()) + T(" ") + learnerInput->getTargetInference()->getInputType()->getName() + T("(s)");
  }
};

/*
** Batch Learners
*/
extern InferencePtr dummyInferenceLearner();
extern InferencePtr staticSequentialInferenceLearner();
extern ParallelInferencePtr staticParallelInferenceLearner();
extern DecoratorInferencePtr sharedParallelInferenceLearner(bool filterUnsupervisedExamples = true);
extern ParallelInferencePtr parallelVoteInferenceLearner();
extern DecoratorInferencePtr decoratorInferenceLearner();
extern DecoratorInferencePtr postProcessInferenceLearner();
extern SequentialInferencePtr stochasticInferenceLearner(bool randomizeExamples = false);

extern VectorSequentialInferencePtr multiPassInferenceLearner();
extern VectorSequentialInferencePtr multiPassInferenceLearner(InferencePtr firstLearner, InferencePtr secondLearner);

extern InferencePtr initializeByCloningInferenceLearner(InferencePtr inferenceToClone);

extern InferencePtr autoTuneInferenceLearner(const OptimizerPtr& optimizer, const OptimizerInputPtr& optimizerInput);
extern InferencePtr autoTuneStochasticInferenceLearner(const OptimizerPtr& optimizer, const DistributionPtr& aprioriDistribution, const Variable& initialGuess);

class EvaluateBatchLearnerObjectiveFunction : public ObjectiveFunction
{
public:
  virtual InferencePtr createLearner(ExecutionContext& context, const Variable& parameters, const InferencePtr& targetInference) const = 0;
  virtual double getObjective(ExecutionContext& context, const InferencePtr& targetInference) const = 0;

  virtual String getDescription(const Variable& input) const;
  virtual double compute(ExecutionContext& context, const Variable& parameters) const;

  void setLearnerInput(const InferenceBatchLearnerInputPtr& learnerInput)
    {this->learnerInput = learnerInput;}

protected:
  friend class EvaluateBatchLearnerObjectiveFunctionClass;

  InferenceBatchLearnerInputPtr learnerInput;
};

typedef ReferenceCountedObjectPtr<EvaluateBatchLearnerObjectiveFunction> EvaluateBatchLearnerObjectiveFunctionPtr;

extern EvaluateBatchLearnerObjectiveFunctionPtr evaluateStochasticLearnerObjectiveFunction();

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_H_
