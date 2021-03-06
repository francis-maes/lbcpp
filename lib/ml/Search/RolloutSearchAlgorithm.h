/*-----------------------------------------.---------------------------------.
| Filename: RolloutSearchAlgorithm.h       | Rollout Search Algorithm        |
| Author  : Francis Maes                   |                                 |
| Started : 10/10/2012 13:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SEARCH_ALGORITHM_ROLLOUT_H_
# define ML_SEARCH_ALGORITHM_ROLLOUT_H_

# include <ml/Search.h>

namespace lbcpp
{

class RolloutSearchAlgorithm : public SearchAlgorithm
{
public:
  RolloutSearchAlgorithm(SearchSamplerPtr sampler = SearchSamplerPtr())
    : sampler(sampler) {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    SearchAlgorithm::startSolver(context, problem, callback, startingSolution);
    sampler->initialize(context, problem->getDomain());
  }

  virtual void runSolver(ExecutionContext& context)
  {
    SearchStatePtr state = trajectory->getFinalState();
    while (!state->isFinalState())
    {
      if (callback->shouldStop())
        return;
      ObjectPtr action = sampler->sampleAction(context, trajectory);
      trajectory->append(action);
      state->performTransition(context, action);
    }
    evaluate(context, trajectory);
  }

protected:
  friend class RolloutSearchAlgorithmClass;

  SearchSamplerPtr sampler;
};

}; /* namespace lbcpp */

#endif // !ML_SEARCH_ALGORITHM_ROLLOUT_H_
