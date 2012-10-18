/*-----------------------------------------.---------------------------------.
| Filename: RolloutSearchAlgorithm.h       | Rollout Search Algorithm        |
| Author  : Francis Maes                   |                                 |
| Started : 10/10/2012 13:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SEARCH_ALGORITHM_ROLLOUT_H_
# define LBCPP_ML_SEARCH_ALGORITHM_ROLLOUT_H_

# include <lbcpp-ml/Search.h>

namespace lbcpp
{

class RolloutSearchAlgorithm : public SearchAlgorithm
{
public:
  RolloutSearchAlgorithm(SearchSamplerPtr sampler = SearchSamplerPtr())
    : sampler(sampler) {}

  virtual void configure(ExecutionContext& context, ProblemPtr problem, SolutionContainerPtr solutions, ObjectPtr initialSolution, Verbosity verbosity)
  {
    SearchAlgorithm::configure(context, problem, solutions, initialSolution, verbosity);
    sampler->initialize(context, problem->getDomain());
  }

  virtual void optimize(ExecutionContext& context)
  {
    SearchStatePtr state = trajectory->getFinalState();
    while (!state->isFinalState())
    {
      if (problem->shouldStop())
        return;
      ObjectPtr action = sampler->sampleAction(context, state);
      trajectory->append(action);
      state->performTransition(context, action);
    }
    trajectory->setFinalState(state);
    evaluate(context, trajectory);
  }

protected:
  friend class RolloutSearchAlgorithmClass;

  SearchSamplerPtr sampler;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SEARCH_ALGORITHM_ROLLOUT_H_
