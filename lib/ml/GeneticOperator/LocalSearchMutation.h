/*-----------------------------------------.---------------------------------.
 | Filename: LocalSearchMutation.h         | Local Search Mutation Operator  |
 | Author  : Denny Verbeeck                |                                 |
 | Started : 15/01/2014 15:10              |                                 |
 `-----------------------------------------/                                 |
                                |                                            |
                                `-------------------------------------------*/

#ifndef ML_LOCAL_SEARCH_MUTATION_H_
# define ML_LOCAL_SEARCH_MUTATION_H_

# include <ml/GeneticOperator.h>
# include <ml/Domain.h>

namespace lbcpp
{
  
/**
 * This class implements a local search mutation operator
 */
class LocalSearchMutation : public Mutation
{
public:
  LocalSearchMutation(MutationPtr mutation = MutationPtr(), size_t improvementRounds = 1) : Mutation(0.0), mutation(mutation), improvementRounds(improvementRounds) {}
  
  virtual void execute(ExecutionContext& context, ProblemPtr problem, ObjectPtr object) const
  {
    if (improvementRounds == 0)
      return;
    
    DenseDoubleVectorPtr vector = object.staticCast<DenseDoubleVector>();
    FitnessPtr originalFitness = problem->evaluate(context, object);
    DenseDoubleVectorPtr mutatedSolution = new DenseDoubleVector(1, 0.0);
    
    for (size_t i = 0; i < improvementRounds; ++i)
    {
      vector->clone(context, mutatedSolution);
      mutation->execute(context, problem, mutatedSolution);
      FitnessPtr fitness = problem->evaluate(context, mutatedSolution);
      if (fitness->strictlyDominates(originalFitness))
        mutatedSolution->clone(context, object);
    }
  }
  
protected:
  friend class LocalSearchMutationClass;
  
  MutationPtr mutation;
  size_t improvementRounds;
};
  
} /* namespace lbcpp */

#endif //!ML_LOCAL_SEARCH_MUTATION_H_