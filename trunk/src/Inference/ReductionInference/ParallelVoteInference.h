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

namespace lbcpp 
{

class ParallelVoteInference : public VectorParallelInference
{
public:
  ParallelVoteInference(const String& name, size_t numVoters, InferencePtr voteInferenceModel, InferencePtr voterLearner)
    : VectorParallelInference(name), voteInferenceModel(voteInferenceModel)
  {
    jassert(numVoters);
    subInferences->resize(numVoters);
    for (size_t i = 0; i < numVoters; ++i)
    {
      InferencePtr voteInference = voteInferenceModel->cloneAndCast<Inference>();
      voteInference->setBatchLearner(voterLearner);
      setSubInference(i, voteInference);
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

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr state = new ParallelInferenceState(input, supervision);
    for (size_t i = 0; i < subInferences->getNumElements(); ++i)
      state->addSubInference(getSubInference(i), input, supervision);
    return state;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
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
  InferencePtr voteInferenceModel;
};

typedef ReferenceCountedObjectPtr<ParallelVoteInference> ParallelVoteInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_REDUCTION_PARALLEL_VOTE_H_
