/*-----------------------------------------.---------------------------------.
 | Filename: ParEGOOptimizer.h             | Wrapper for ParEGO               |
 | Author  : Denny Verbeeck                |                                  |
 | Started : 19/06/2013 13:41              |                                  |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef ML_OPTIMIZER_PAREGO_H_
# define ML_OPTIMIZER_PAREGO_H_

# include <ml/Solver.h>
# include <LinAlg/VecMat.h>
# include <ctime>
# include "Solver/SurrogateBasedSolverInformation.h"
# define MAX_K 5

namespace lbcpp
{
  
class ParEGOOptimizer : public IterativeSolver
{
public:
  ParEGOOptimizer(size_t numIterations = 0) : IterativeSolver(numIterations) {}
  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    IterativeSolver::startSolver(context, problem, callback, startingSolution);
    initialSamples = init_ParEGO(context).staticCast<OVector>();
  }
  
  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    ObjectPtr object;
    FitnessPtr fitness;
    
    if (iter < initialSamples->getNumElements())
      object = initialSamples->get(iter);
    else
      object = iterate_ParEGO(context, iter);
    
    fitness = evaluate(context, object);
    if (verbosity >= verbosityDetailed)
    {
      // fitnesses
      context.resultCallback("object", object);
      for (size_t i = 0; i < fitness->getNumValues(); ++i)
        context.resultCallback("fitness" + string((int) i), fitness->getValue(i));

      // SurrogateBasedSolverInformation
      SurrogateBasedSolverInformationPtr information(new SurrogateBasedSolverInformation(iter + 1));
      information->setProblem(problem);
      if (lastInformation)
        information->setSolutions(lastInformation->getSolutions());
      else
        information->setSolutions(new SolutionVector(problem->getFitnessLimits()));
      information->getSolutions()->insertSolution(object, fitness);
      information->setSurrogateModel(ExpressionPtr());
      context.resultCallback("information", information);
      lastInformation = information;
    }
    return true;
  }
  
  virtual void stopSolver(ExecutionContext& context)
  {
    IterativeSolver::stopSolver(context);
    cleanup();
  }
  
  static int pcomp(const void *i, const void *j);
  
protected:
  friend class ParEGOSolverClass;
  SurrogateBasedSolverInformationPtr lastInformation;
  
private:
  OVectorPtr initialSamples;


  // ParEGO variables:
  typedef struct sor
  {
    double y;
    int indx;
  }SOR;
  
  int MAX_ITERS;
  int nobjs;
  int **wv;      // first dimension needs to be #weight vectors^nobjs
  double **dwv;  // first dimension needs to be #weight vectors^nobjs
  double **normwv; // first dimension needs to be N+k-1 choose k-1 ($^{N+k-1}C_{k-1}$) , where N is a parameter and k=nobjs
  int N; // parameter relating to the number of divisions
  int nwvs;
  
  double alph; // power in the DTLZ2 and DTLZ4 test functions
  
  int *improvements;  
  int dim;  // dimension of the actual search space, X
  int pdim; // number of parameters in the DACE model
  int titer; // global giving the size of the current correlation matrix R being used
  double **ff; // two-dimensional array of all multiobjective cost vectors
  double **ax;  // two-dimensional array of all x vectors
  double ***pax; // a pointer to the above, or the below
  double **tmpax; // two dimensional array of selected x vectors
  double *xmin; // upper and lower constraints on the search space
  double *xmax;
  double *ay; // array of true/measured y fitnesses
  double *tmpay;
  double **pay;
  
  double minss;  // a minimum standard error of the predictor to force exploration
  
  double glik;
  double altlik;
  bool DEBUG;
  
  double global_min;
  double gmu;
  double gsigma;
  double *gtheta;
  double *gp;
  double ymin;
  double gymin;
  ::Matrix *pgR;
  ::Matrix *pInvR;
  ::Vector *pgy;
  int best_ever;
  double niche;
  
  double *gz; // an array holding the z values for the gaussian distribution
  
  double *absmax;
  double *absmin;
  double *gideal;
  double *gwv;
  
  clock_t start, end;
  double cpu_time_used;
  
  // ParEGO functions:
  ObjectPtr init_ParEGO(lbcpp::ExecutionContext& context);
  ObjectPtr iterate_ParEGO(lbcpp::ExecutionContext& context, int iter);
  
  void cleanup();
  void snake_wv(int s, int k);
  void reverse(int k, int n, int s);
  void init_gz();
  void cwr(int **target, int k, int n);
  ::Matrix identM(int n);
  void pr_sq_mat(const ::Matrix& m, int dim);
  void pr_vec(const ::Vector& v, int dim);
  double mu_hat(const ::Matrix& InvR, const ::Vector& y, int n);
  double sigma_squared_hat(const ::Matrix& InvR, const ::Vector& y, double mu_hat, int n);
  double likelihood(double *param);
  double weighted_distance(double *xi, double *xj, double *theta, double *p, int dim);
  double correlation(double *xi, double *xj, double *theta, double *p, int dim);
  void build_R(double **ax, double *theta, double *p, int dim, int n, ::Matrix &R);
  void build_y(double *ay, int n, ::Vector &y);
  void init_arrays(double ***ax, double **ay, int n, int dim);
  double myabs(double v);
  int mypow(int x, int expon);
  double predict_y(double **ax, const ::Matrix& R, const ::Vector& y, double mu_hat, double *theta, double *p, int n, int dim);
  void get_params(double **param, double *theta, double *p, double *sigma, double *mu);
  double myfit(lbcpp::ExecutionContext &context, double *x, double *ff);
  double posdet(Matrix& R, int n);
  double s2(double **ax, double *theta, double *p, double sigma, int dim, int n, const Matrix& InvR);
  double standard_density(double z);
  double standard_distribution(double z);
  double expected_improvement(double yhat, double ymin, double s);
  double wrap_ei(double *x); // returns -(expected improvement), given the solution x
  void latin_hyp(double **ax, int iter);
  void compute_n(int n, int *dig, int base);
  double Tcheby(double *wv, double *vec, double *ideal);
  int tourn_select(double *xsel, double **ax, double *ay, int iter, int t_size);
  void mutate(double *x);
  void cross(double *child, double *par1, double *par2);
  void mysort(int *idx, double *val, int num);
};
  
} /* namespace lbcpp */

#endif //!ML_OPTIMIZER_PAREGO_H_
