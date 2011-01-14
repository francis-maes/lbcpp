/*-----------------------------------------.---------------------------------.
| Filename: ParallelVoteInference.h        | A base class for vote-based     |
| Author  : Francis Maes                   |   inferences                    |
| Started : 28/06/2010 15:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_REDUCTION_PARALLEL_VOTE_H_
# define LBCPP_INFERENCE_REDUCTION_PARALLEL_VOTE_H_

# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Inference/InferenceBatchLearner.h>
# include <lbcpp/Distribution/Distribution.h>
# include <lbcpp/Distribution/DistributionBuilder.h>
# include <lbcpp/Data/RandomGenerator.h>

namespace lbcpp 
{

class ParallelVoteInference : public VectorParallelInference
{
public:
  ParallelVoteInference(const String& name, size_t numVoters, InferencePtr voteInferenceModel, InferencePtr voterLearner, DistributionBuilderPtr probabilityBuilderModel)
    : VectorParallelInference(name), voteInferenceModel(voteInferenceModel), probabilityBuilderModel(probabilityBuilderModel)
  {
    jassert(numVoters);
    subInferences.resize(numVoters);
    for (size_t i = 0; i < numVoters; ++i)
    {
      InferencePtr voteInference = voteInferenceModel->cloneAndCast<Inference>();
      voteInference->setBatchLearner(voterLearner->cloneAndCast<Inference>());
      subInferences[i] = voteInference;
    }
    setBatchLearner(parallelVoteInferenceLearner());
  }

  ParallelVoteInference() {}
  
  virtual bool useMultiThreading() const
    {return false;}

  virtual TypePtr getInputType() const
    {return voteInferenceModel->getInputType();}

  virtual TypePtr getSupervisionType() const
    {return voteInferenceModel->getSupervisionType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return voteInferenceModel->getOutputType(inputType);}

  virtual ParallelInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    ParallelInferenceStatePtr state = new ParallelInferenceState(input, supervision);
    state->reserve(subInferences.size());
    for (size_t i = 0; i < subInferences.size(); ++i)
      state->addSubInference(subInferences[i], input, supervision);
    return state;
  }

  virtual Variable finalizeInference(ExecutionContext& context, ParallelInferenceStatePtr state) const
  {
    size_t n = state->getNumSubInferences();
    if (!n)
      return Variable();
    
    DistributionBuilderPtr probabilityBuilder = probabilityBuilderModel->cloneAndCast<DistributionBuilder>(context);
    for (size_t i = 0; i < n; ++i)
    {
      DistributionPtr distribution = state->getSubOutput(i).getObjectAndCast<Distribution>();
      jassert(distribution);
      probabilityBuilder->addDistribution(distribution);
    }
    return probabilityBuilder->build(context);
  }

protected:
  friend class ParallelVoteInferenceClass;

  InferencePtr voteInferenceModel;
  DistributionBuilderPtr probabilityBuilderModel;
};

typedef ReferenceCountedObjectPtr<ParallelVoteInference> ParallelVoteInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_REDUCTION_PARALLEL_VOTE_H_
