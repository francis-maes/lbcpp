/*-----------------------------------------.---------------------------------.
| Filename: SharedParallelVectorInference.h| Infer a Vector by inferring each|
| Author  : Francis Maes                   |  element parallely with a       |
| Started : 15/07/2010 14:41               |  shared elements sub-inference  |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_REDUCTION_SHARED_PARALLEL_VECTOR_H_
# define LBCPP_INFERENCE_REDUCTION_SHARED_PARALLEL_VECTOR_H_

# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Data/Vector.h>
# include <lbcpp/Function/Perception.h>
# include <lbcpp/Data/ProbabilityDistribution.h>
# include <lbcpp/Data/RandomGenerator.h>

namespace lbcpp
{

class SharedParallelVectorInference : public SharedParallelInference
{
public:
  SharedParallelVectorInference(const String& name, FunctionPtr sizeFunction, InferencePtr elementInference)
    : SharedParallelInference(name, elementInference), sizeFunction(sizeFunction) {}
  SharedParallelVectorInference() {}

  virtual TypePtr getInputType() const
    {return anyType();}

  virtual TypePtr getSupervisionType() const
    {return vectorClass(subInference->getSupervisionType());}

  TypePtr getOutputElementsType(TypePtr inputType) const
    {return subInference->getOutputType(pairClass(inputType, positiveIntegerType()));}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return vectorClass(getOutputElementsType(inputType));}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    size_t n = (size_t)sizeFunction->compute(input).getInteger();
    
    VectorPtr supervisionVector = supervision ? supervision.getObjectAndCast<Vector>() : VectorPtr();

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable elementSupervision;
      if (supervisionVector)
        elementSupervision = supervisionVector->getElement(i);
      res->addSubInference(subInference, Variable::pair(input, i), elementSupervision);
    }
    return res;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    size_t n = state->getNumSubInferences();
    VectorPtr res = vector(getOutputElementsType(state->getInput().getType()), n);
    bool atLeastOnePrediction = false;
    for (size_t i = 0; i < n; ++i)
    {
      Variable result = state->getSubOutput(i);
      if (result)
      {
        if (result.isObject())
        {
          DiscreteProbabilityDistributionPtr distribution = result.dynamicCast<DiscreteProbabilityDistribution>();
          if (distribution)
            result = distribution->sample(RandomGenerator::getInstance());
        }
        atLeastOnePrediction = true;
        res->setElement(i, result);
      }
    }
    return atLeastOnePrediction ? res : Variable::missingValue(res->getClass());
  }

protected:
  friend class SharedParallelVectorInferenceClass;

  FunctionPtr sizeFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_REDUCTION_SHARED_PARALLEL_VECTOR_H_
