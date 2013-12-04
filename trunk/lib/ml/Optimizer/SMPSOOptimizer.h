/*-----------------------------------------.---------------------------------.
| Filename: SMPSOOptimizer.h               | SMPSO Optimizer                 |
| Author  : Denny Verbeeck                 |                                 |
| Started : 29/11/2013 14:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_OPTIMIZER_SMPSO_H_
# define ML_OPTIMIZER_SMPSO_H_

# include <ml/Problem.h>
# include <ml/Sampler.h>
# include <ml/Solver.h>
# include <ml/SolutionContainer.h>
# include <ml/SolutionComparator.h>

namespace lbcpp
{

/**
 *This class implements the SMPSO algorithm described in:
 * A.J. Nebro, J.J. Durillo, J. Garcia-Nieto, C.A. Coello Coello, F. Luna and E. Alba
 * "SMPSO: A New PSO-based Metaheuristic for Multi-objective Optimization". 
 * IEEE Symposium on Computational Intelligence in Multicriteria Decision-Making 
 * (MCDM 2009), pp: 66-73. March 2009
 *
 * Ported from the jMetal implementation (http://jmetal.sourceforge.net/)
 */
class SMPSOOptimizer : public PopulationBasedSolver
{
public:
  SMPSOOptimizer(size_t swarmSize = 100, size_t archiveSize = 100, size_t numIterations = 0, SamplerPtr initialVectorSampler = SamplerPtr())
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

  ~SMPSOOptimizer()
    {cleanUp();}

  virtual bool iterateSolver(ExecutionContext& context, size_t iter);

protected:
  friend class SMPSOOptimizerClass;

  void init(ExecutionContext& context);
  void cleanUp();
  void computeSpeed(ExecutionContext& context, size_t iter);
  void computeNewPositions();
  void mopsoMutation(ExecutionContext& context, size_t iter);
  void doMutation(ExecutionContext& context, DenseDoubleVectorPtr particle);
  double constrictionCoefficient(double c1, double c2);
  double velocityConstriction(double v, double* deltaMax, double* deltaMin, int variableIndex, int particleIndex);
  double inertiaWeight(int iter, int miter, double wma, double wmin);
  DenseDoubleVectorPtr cloneVector(ExecutionContext& context, DenseDoubleVectorPtr source)
  {
    DenseDoubleVectorPtr o1 = new DenseDoubleVector();
    source->clone(context, o1);
    return o1;
  }

  SamplerPtr initialVectorSampler;

  /* parameters */
  size_t archiveSize;
  double r1Max_;
  double r1Min_;
  double r2Max_;
  double r2Min_;
  double C1Max_;
  double C1Min_;
  double C2Max_;
  double C2Min_;
  double WMax_;
  double WMin_;
  double ChVel1_;
  double ChVel2_;
  double* deltaMax_;
  double* deltaMin_;

  /* particles */
  SolutionVectorPtr particles;

  /* particle memory */
  SolutionVectorPtr best;

  /* archive */
  CrowdingArchivePtr leaders;

  /* speed vectors */
  double** speed;

  /* mutation operator parameters */
  double mutationProbability;
  double distributionIndex;
  double eta_m;
};

} /* namespace lbcpp */

#endif // !ML_OPTIMIZER_SMPSO_H_
