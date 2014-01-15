/*-----------------------------------------.---------------------------------.
 | Filename: GeneticOperators.h            | Genetic Operators               |
 | Author  : Denny Verbeeck                |                                 |
 | Started : 10/01/2014 14:09              |                                 |
 `-----------------------------------------/                                 |
                                |                                            |
                                `-------------------------------------------*/

#ifndef ML_GENETIC_OPERATORS_H_
# define ML_GENETIC_OPERATORS_H_

# include "predeclarations.h"
# include <ml/Problem.h>

namespace lbcpp
{

/** Base class for mutation operators **/
class Mutation : public Object
{
public:
  Mutation(double probability = 0.0) : probability(probability) {}

  /** Perform this mutation on a given object.
   *  \param context The current Execution Context
   *  \param problem The problem to which the given object belongs
   *  \param object  The object to be mutated
   */
  virtual void execute(ExecutionContext& context, ProblemPtr problem, ObjectPtr object) const = 0;

protected:
  friend class MutationClass;

  double probability;
};

typedef ReferenceCountedObjectPtr<Mutation> MutationPtr;

extern MutationPtr uniformMutation(double probability, double perturbation);
extern MutationPtr nonUniformMutation(double probability, double perturbation, size_t currentIteration, size_t maxIterations);
extern MutationPtr polynomialMutation(double probability, double distributionIndex = 20.0, double eta_m = 20.0);
extern MutationPtr localSearchMutation(MutationPtr mutation, size_t improvementRounds = 1);

  
/** Base class for crossover operators **/
class Crossover : public Object
{
public:
  Crossover(double probability = 0.0) : probability(probability) {}
  
  /** Perform this crossover on a vector of parent objects.
   *  \param context The current Execution Context
   *  \param problem The problem to which the given objects belong
   *  \param parents The parents for the crossover
   */
  virtual std::vector<ObjectPtr> execute(ExecutionContext& context, ProblemPtr problem, const std::vector<ObjectPtr>& parents) const = 0;
  
protected:
  friend class CrossoverClass;
  
  double probability;
};
  
typedef ReferenceCountedObjectPtr<Crossover> CrossoverPtr;
  
extern CrossoverPtr sbxCrossover(double probability, double distributionIndex = 20.0);

} /* namespace lbcpp */

#endif //!ML_GENETIC_OPERATORS_H_