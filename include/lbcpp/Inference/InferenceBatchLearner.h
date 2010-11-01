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
# include <lbcpp/Data/Pair.h>
# include <lbcpp/Data/Container.h>

namespace lbcpp
{

class InferenceBatchLearnerInput : public Object
{
public:
  const InferencePtr& getTargetInference() const
    {return targetInference;}

  size_t getNumTrainingExamples() const
    {return trainingData.size();}

  size_t getNumValidationExamples() const
    {return validationData.size();}

private:
  friend class InferenceBatchLearnerInputClass;

  typedef std::vector< std::pair<Variable, Variable> > ExampleVector;
  InferencePtr targetInference;
  ExampleVector trainingData;
  ExampleVector validationData;
};

extern ClassPtr inferenceBatchLearnerInputClass;
typedef ReferenceCountedObjectPtr<InferenceBatchLearnerInput> InferenceBatchLearnerInputPtr;

template<class BaseClass>
class InferenceBatchLearner : public BaseClass
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

class AtomicInferenceBatchLearner : public InferenceBatchLearner<Inference>
{
public:
  virtual Variable learn(InferenceContextWeakPtr context, const InferencePtr& targetInference, const ContainerPtr& trainingData) = 0;

protected:
  virtual Variable run(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
};

/*
** Batch Learners
*/
extern AtomicInferenceBatchLearnerPtr dummyInferenceLearner();
extern InferencePtr staticSequentialInferenceLearner();
extern ParallelInferencePtr staticParallelInferenceLearner();
extern DecoratorInferencePtr sharedParallelInferenceLearner(bool filterUnsupervisedExamples = true);
extern ParallelInferencePtr parallelVoteInferenceLearner();
extern DecoratorInferencePtr decoratorInferenceLearner();
extern DecoratorInferencePtr postProcessInferenceLearner();
extern SequentialInferencePtr stochasticInferenceLearner(bool randomizeExamples = false);

extern VectorSequentialInferencePtr multiPassInferenceLearner();
extern VectorSequentialInferencePtr multiPassInferenceLearner(InferencePtr firstLearner, InferencePtr secondLearner);

extern AtomicInferenceBatchLearnerPtr initializeByCloningInferenceLearner(InferencePtr inferenceToClone);

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_H_
