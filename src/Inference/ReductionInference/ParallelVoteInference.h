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
# include <lbcpp/Data/ProbabilityDistribution.h>
# include <lbcpp/Data/RandomGenerator.h>

namespace lbcpp 
{

class ParallelVoteInference : public VectorParallelInference
{
public:
  ParallelVoteInference(const String& name, size_t numVoters, InferencePtr voteInferenceModel, InferencePtr voterLearner)
    : VectorParallelInference(name), voteInferenceModel(voteInferenceModel)
  {
    jassert(numVoters);
    subInferences.resize(numVoters);
    for (size_t i = 0; i < numVoters; ++i)
    {
      InferencePtr voteInference = voteInferenceModel->cloneAndCast<Inference>();
      voteInference->setBatchLearner(voterLearner);
      subInferences[i] = voteInference;
    }
    setBatchLearner(parallelVoteInferenceLearner());
  }

  ParallelVoteInference() {}
 
  virtual TypePtr getInputType() const
    {return voteInferenceModel->getInputType();}

  virtual TypePtr getSupervisionType() const
    {return voteInferenceModel->getSupervisionType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return voteInferenceModel->getOutputType(inputType);}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr state = new ParallelInferenceState(input, supervision);
    state->reserve(subInferences.size());
    for (size_t i = 0; i < subInferences.size(); ++i)
      state->addSubInference(subInferences[i], input, supervision);
    return state;
  }

  virtual Variable finalizeInference(InferenceContextWeakPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    size_t n = state->getNumSubInferences();
    if (!n)
      return Variable();
    jassert(false); // FIXME
    /*
    Variable res;
    double weight = 1.0 / (double)n;
    for (size_t i = 0; i < n; ++i)
      res.addWeighted(state->getSubOutput(i), weight);*/
    return Variable();
  }

protected:
  friend class ParallelVoteInferenceClass;

  InferencePtr voteInferenceModel;
};

typedef ReferenceCountedObjectPtr<ParallelVoteInference> ParallelVoteInferencePtr;

class MeanScalarParallelVoteInference : public ParallelVoteInference
{
public:
  MeanScalarParallelVoteInference(const String& name, size_t numVotes, InferencePtr voteInferenceModel, InferencePtr voterLearner)
    : ParallelVoteInference(name, numVotes, voteInferenceModel, voterLearner)
    {jassert(voteInferenceModel->getOutputType(voteInferenceModel->getInputType())->inheritsFrom(doubleType));}
  MeanScalarParallelVoteInference() {}

  virtual Variable finalizeInference(InferenceContextWeakPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    size_t n = state->getNumSubInferences();
    double sum = 0.0;
    size_t count = 0;
    for (size_t i = 0; i < n; ++i)
    {
      Variable vote = state->getSubOutput(i);
      jassert(vote.isDouble());
      if (vote.exists())
      {
        ++count;
        sum += vote.getDouble();
      }
    }
    TypePtr type = getOutputType(getInputType());
    return count ? Variable(sum / (double)count, type) : Variable::missingValue(type);
  }
};

class MajorityClassParallelVoteInference : public ParallelVoteInference
{
public:
  MajorityClassParallelVoteInference(const String& name, size_t numVotes, InferencePtr voteInferenceModel, InferencePtr voterLearner)
    : ParallelVoteInference(name, numVotes, voteInferenceModel, voterLearner)
    {jassert(voteInferenceModel->getOutputType(voteInferenceModel->getInputType()).dynamicCast<Enumeration>());}
  MajorityClassParallelVoteInference() {}

  virtual Variable finalizeInference(InferenceContextWeakPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    TypePtr enumType = getOutputType(getInputType());
    size_t n = state->getNumSubInferences();
    DiscreteProbabilityDistributionPtr distribution = new DiscreteProbabilityDistribution(enumType); 
    for (size_t i = 0; i < n; ++i)
    {
      Variable vote = state->getSubOutput(i);
      jassert(vote.getType()->inheritsFrom(enumType));
      if (vote.exists())
        distribution->increment(vote);
    }
    return distribution->sample(RandomGenerator::getInstance()); // FIXME: replace by a sampling of argmaxs 
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_REDUCTION_PARALLEL_VOTE_H_
