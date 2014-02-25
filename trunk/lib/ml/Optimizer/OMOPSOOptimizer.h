/*-----------------------------------------.---------------------------------.
| Filename: OMOPSOOptimizer.h              | OMOPSO Optimizer                |
| Author  : Denny Verbeeck                 |                                 |
| Started : 03/12/2013 23:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_OPTIMIZER_OMOPSO_H_
# define ML_OPTIMIZER_OMOPSO_H_

# include <ml/Solver.h>
# include <ml/Sampler.h>
# include <ml/GeneticOperator.h>

namespace lbcpp
{

/**
 * Ported from the jMetal implementation (http://jmetal.sourceforge.net/)
 */
class OMOPSOOptimizer : public PopulationBasedSolver
{
public:
  OMOPSOOptimizer(size_t swarmSize = 100, size_t archiveSize = 100, size_t numIterations = 0, SamplerPtr initialVectorSampler = SamplerPtr())
    : PopulationBasedSolver(swarmSize, numIterations), archiveSize(archiveSize), initialVectorSampler(initialVectorSampler) {}

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

  virtual bool iterateSolver(ExecutionContext& context, size_t iter);
  ~OMOPSOOptimizer() {}

protected:
  friend class OMOPSOOptimizerClass;
  
  size_t archiveSize;
  double perturbation;
  SolutionVectorPtr particles;
  SolutionVectorPtr best;
  CrowdingArchivePtr leaders;
  double** speed;
  double eta;

  SamplerPtr initialVectorSampler;

  double uniformMutationProbability;
  double uniformMutationPerturbationIndex;
  MutationPtr uMutation;
  double nonUniformMutationProbability;
  double nonUniformMutationPerturbationIndex;

  void init(ExecutionContext& context);
  void cleanUp();
  void computeSpeed(ExecutionContext& context, size_t iter);
  void computeNewPositions();
  void mopsoMutation(ExecutionContext& context, size_t iter);
  DenseDoubleVectorPtr cloneVector(ExecutionContext& context, DenseDoubleVectorPtr source)
  {
    DenseDoubleVectorPtr o1 = new DenseDoubleVector(1, 0.0);
    source->clone(context, o1);
    return o1;
  }
};

} /* namespace lbcpp */

#endif // !ML_OPTIMIZER_OMOPSO_H_
