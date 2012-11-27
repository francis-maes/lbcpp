/*-----------------------------------------.---------------------------------.
| Filename: LBFGSOptimizer.h               | Adaptator to the original       |
| Author  : Francis Maes                   | Fortran LBFGS converted to C    |
| Started : 08/03/2007 11:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_OPTIMIZER_LBFGS_H_
# define LBCPP_ML_OPTIMIZER_LBFGS_H_

# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/SolutionContainer.h>

extern int lbfgs(
    int* num_variables,   // in
    int* num_corrections, // in: recommended: [3..7]
    double* estimate,     // in: initial guess, out: best solution, length = num_variables
    double* f,            // in: f(estimate)
    double* g,            // in: grad(estimate) length = num_variables
    int* diagco,          // in: booleen 
    double* diag,         // in, only if diagco=true, length = num_variables
    int* iprint,          // in, iprint[0] < 0: no output, = 0: minimal output, = n: print every n iterations
                          //     iprint[1]: output type in range [0..3]
    double* eps,
    double* xtolerance,   // in: DBL_EPSILON
    double* w,            // working array of length N(2M+1)+2M
    int* iflag            // in: 0 at beggining and 1 for restart, out: error code
  );

namespace lbcpp
{

class LBFGSOptimizer : public IterativeSolver
{
public:
  LBFGSOptimizer(size_t numIterations = 0)
    : IterativeSolver(numIterations) {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    IterativeSolver::startSolver(context, problem, callback, startingSolution);
    ContinuousDomainPtr domain = problem->getDomain().staticCast<ContinuousDomain>();
    lbfgsInitialize(domain->getNumDimensions());
    parameters = startingSolution.staticCast<DenseDoubleVector>();
    if (!parameters)
      parameters = problem->getInitialGuess();
    parameters = parameters->cloneAndCast<DenseDoubleVector>();
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter) // returns false if the optimizer has converged
  {
    double value = 0.0;
    DoubleVectorPtr gradient;
    DifferentiableObjectivePtr objective = problem->getObjective(0).staticCast<DifferentiableObjective>();
    objective->evaluate(context, parameters, &value, &gradient);
    addSolution(context, parameters, value);
    DenseDoubleVectorPtr denseGradient = gradient->toDenseDoubleVector();
    int res = lbfgsStep(parameters->getValuePointer(0), value, denseGradient->getValuePointer(0));
    return (res == 1);
  }

private:
  DenseDoubleVectorPtr parameters;

  int numVariables;
  int numCorrections;
  int iflag;
  int iprint[2];
  std::vector<double> diagonal;
  std::vector<double> memory;
  
  void lbfgsInitialize(int numVariables)
  {
    this->numVariables = numVariables;
    numCorrections = 5;
    iflag = 0;
    iprint[0] = -1;
    iprint[1] = 0;
    diagonal.resize(numVariables);
    memory.resize(numVariables * (2 * numCorrections + 1) + 2 * numCorrections);    
  }
  
  int lbfgsStep(double* x, double f, const double* g)
  {
    static double tolerance = DBL_EPSILON;
    static double epsilon = 1e-9;
    static int diagco = 0;
    lbfgs(&numVariables, &numCorrections, x, &f, const_cast<double* >(g), &diagco, &diagonal[0], iprint, &epsilon, &tolerance, &memory[0], &iflag);
    return iflag;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OPTIMIZER_LBFGS_H_
