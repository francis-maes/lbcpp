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
  typedef std::vector< std::pair<Variable, Variable> > ExampleVector;

  InferenceBatchLearnerInput(const InferencePtr& targetInference, const ContainerPtr& trainingExamples, const ContainerPtr& validationExamples);
  InferenceBatchLearnerInput(const InferencePtr& targetInference, size_t numTrainingExamples, size_t numValidationExamples);
  InferenceBatchLearnerInput(const InferencePtr& targetInference, const ExampleVector& trainingData, const ExampleVector& validationData);
  InferenceBatchLearnerInput(const InferencePtr& targetInference);
  InferenceBatchLearnerInput() {}

  /*
  ** Target Inference
  */
  const InferencePtr& getTargetInference() const
    {return targetInference;}

  /*
  ** Training Data
  */
  size_t getNumTrainingExamples() const
    {return trainingData.size();}

  const std::pair<Variable, Variable>& getTrainingExample(size_t i) const
    {jassert(i < trainingData.size()); return trainingData[i];}
  
  std::pair<Variable, Variable>& getTrainingExample(size_t i)
    {jassert(i < trainingData.size()); return trainingData[i];}

  void addTrainingExample(const Variable& input, const Variable& supervision)
    {trainingData.push_back(std::make_pair(input, supervision));}

  const ExampleVector& getTrainingExamples() const
    {return trainingData;}

  /*
  ** Validation Data
  */
  size_t getNumValidationExamples() const
    {return validationData.size();}

  const std::pair<Variable, Variable>& getValidationExample(size_t i) const
    {jassert(i < validationData.size()); return validationData[i];}

  std::pair<Variable, Variable>& getValidationExample(size_t i)
    {jassert(i < validationData.size()); return validationData[i];}

  void addValidationExample(const Variable& input, const Variable& supervision)
    {validationData.push_back(std::make_pair(input, supervision));}

  const ExampleVector& getValidationExamples() const
    {return validationData;}

  /*
  ** All Data
  */
  size_t getNumExamples() const
    {return trainingData.size() + validationData.size();}

  const std::pair<Variable, Variable>& getExample(size_t i) const
    {if (i < trainingData.size()) return trainingData[i]; i -= trainingData.size(); jassert(i < validationData.size()); return validationData[i];}

  std::pair<Variable, Variable>& getExample(size_t i)
    {if (i < trainingData.size()) return trainingData[i]; i -= trainingData.size(); jassert(i < validationData.size()); return validationData[i];}

  void setExample(size_t i, const Variable& input, const Variable& supervision)
    {std::pair<Variable, Variable>& e = getExample(i); e.first = input; e.second = supervision;}

private:
  friend class InferenceBatchLearnerInputClass;

  InferencePtr targetInference;
  ExampleVector trainingData;
  ExampleVector validationData;
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

  virtual String getDescription(const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>();
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

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_H_
