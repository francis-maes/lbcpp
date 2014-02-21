/*-----------------------------------------.---------------------------------.
| Filename: AbYSSOptimizer.h               | AbYSS Optimizer                 |
| Author  : Denny Verbeeck                 |                                 |
| Started : 08/01/2014 21:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/


#ifndef ML_OPTIMIZER_ABYSS_H_
# define ML_OPTIMIZER_ABYSS_H_

# include <ml/Problem.h>
# include <ml/Sampler.h>
# include <ml/Solver.h>
# include <ml/SolutionContainer.h>
# include <ml/SolutionComparator.h>
# include <ml/GeneticOperator.h>

namespace lbcpp
{

/**
 * This class implements the AbYSS algorithm. This algorithm is an adaptation
 * of the single-objective scatter search template defined by F. Glover in:
 * F. Glover. "A template for scatter search and path relinking", Lecture Notes 
 * in Computer Science, Springer Verlag, 1997. AbYSS is described in: 
 *   A.J. Nebro, F. Luna, E. Alba, B. Dorronsoro, J.J. Durillo, A. Beham 
 *   "AbYSS: Adapting Scatter Search to Multiobjective Optimization." 
 *   IEEE Transactions on Evolutionary Computation. Vol. 12, 
 *   No. 4 (August 2008), pp. 439-457
 *
 * Ported from the jMetal implementation (http://jmetal.sourceforge.net/)
 */
class AbYSSOptimizer : public PopulationBasedSolver
{
public:
  AbYSSOptimizer(size_t populationSize = 20, size_t archiveSize = 100, size_t refSet1Size = 10, size_t refSet2Size = 10, size_t numIterations = 0)
    : PopulationBasedSolver(populationSize, numIterations), archiveSize_(archiveSize), refSet1Size_(refSet1Size), refSet2Size_(refSet2Size) {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution = ObjectPtr())
  {
    PopulationBasedSolver::startSolver(context, problem, callback, startingSolution);
    init(context);
  }

  virtual void stopSolver(ExecutionContext& context)
  {
    PopulationBasedSolver::stopSolver(context);
    cleanUp();
  }

  ~AbYSSOptimizer()
    {}

  virtual bool iterateSolver(ExecutionContext& context, size_t iter);

protected:
  friend class AbYSSOptimizerClass;

  SolutionVectorPtr archive;
  SolutionVectorPtr solutionSet;
  SolutionVectorPtr refSet1;
  SolutionVectorPtr refSet2;
  SolutionVectorPtr subSet;
  std::map<ObjectPtr, double> distancesToSolutionSet;
  std::map<ObjectPtr, bool> marked;

  /**
   * These variables are used in the diversification method.
   */
  int* sumOfFrequencyValues_;
  int* sumOfReverseFrequencyValues_;
  int** frequency_;
  int** reverseFrequency_;

  size_t numberOfSubranges_ ;   //!< Stores the number of subranges in which each variable is divided. Used in the diversification method. By default it takes the value 4 
  size_t archiveSize_;          //!< Maximum size of the external archive
  size_t refSet1Size_;          //!< Maximum size of the reference set one
  size_t refSet2Size_;          //!< Maximum size of the reference set two
  size_t subSetSize_;           //!< Maximum size of the subSet

  MutationPtr improvementOperator;
  CrossoverPtr crossoverOperator;

  void init(ExecutionContext& context);
  void cleanUp();
  DenseDoubleVectorPtr diversificationGeneration(ExecutionContext& context);

  /** Update the reference set
   *  \param build If true, indicates that the reference has to be built for
   *               the first time. Otherwise the reference set will be updated
   *               with new solutions.
   */
  void referenceSetUpdate(ExecutionContext& context, bool build);
  size_t subSetGeneration(ExecutionContext& context);
  bool refSet1Test(const ObjectPtr& solution, const FitnessPtr& fitness);
  bool refSet2Test(const ObjectPtr& solution, const FitnessPtr& fitness);
  
  double distanceToSolutionSet(const DenseDoubleVectorPtr& individual, const SolutionVectorPtr& set) const
  {
    double distance = DBL_MAX;
    double aux = 0.0;
    for (size_t i = 0; i < set->getNumSolutions(); ++i)
    {
      aux = individual->distanceTo(set->getSolution(i).staticCast<DenseDoubleVector>());
      if (aux < distance)
        distance = aux;
    }
    return distance;
  }
  
  DenseDoubleVectorPtr cloneVector(ExecutionContext& context, DenseDoubleVectorPtr source) const
  {
    DenseDoubleVectorPtr o1 = new DenseDoubleVector(1, 0.0);
    source->clone(context, o1);
    return o1;
  }
};

} /* namespace lbcpp */

#endif //!ML_OPTIMIZER_ABYSS_H_
