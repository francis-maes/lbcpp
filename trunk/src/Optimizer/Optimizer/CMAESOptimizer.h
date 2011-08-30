/*-----------------------------------------.---------------------------------.
| Filename: CMAESOptimizer.h               | CMA/ES Optimizer                |
| Author  : Francis Maes                   |                                 |
| Started : 29/08/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_CMA_ES_H_
#define LBCPP_OPTIMIZER_CMA_ES_H_

# include <lbcpp/Optimizer/Optimizer.h>

# undef T
# include "../../shark/include/EALib/CMA.h"
# define T JUCE_T

namespace lbcpp
{

class LbcppToSharkObjectiveFunction : public ObjectiveFunctionVS<double> 
{
public:
  LbcppToSharkObjectiveFunction(ExecutionContext& context, FunctionPtr objective, DenseDoubleVectorPtr initialGuess)
    : context(context), objective(objective), initialGuess(initialGuess)
  {
  	m_name = (const char* )objective->toShortString();
    m_dimension = initialGuess->getNumValues();
  }

  virtual unsigned int objectives() const
    {return 1;}

  virtual void result(double* const& point, std::vector<double>& value)
  {
    DenseDoubleVectorPtr vector = new DenseDoubleVector(m_dimension, 0.0);
    memcpy(vector->getValuePointer(0), point, sizeof (double) * m_dimension);
	  value.resize(1);
	  value[0] = objective->compute(context, vector).toDouble();
	  m_timesCalled++;
  }
  
  virtual bool ProposeStartingPoint(double*& point) const
  {
    memcpy(point, initialGuess->getValuePointer(0), sizeof (double) * m_dimension);
    return true;
  }

protected:
  ExecutionContext& context;
  FunctionPtr objective;
  DenseDoubleVectorPtr initialGuess;
};

class CMAOptimizerState : public OptimizerState
{
public:
  CMAOptimizerState() : fitness(NULL) {}
  virtual ~CMAOptimizerState()
  {
    if (fitness)
      delete fitness;
  }

  void initialize(ExecutionContext& context, const FunctionPtr& objective, const DenseDoubleVectorPtr& initialGuess)
  {
    this->initialGuess = initialGuess;
    jassert(!fitness);
    fitness = new LbcppToSharkObjectiveFunction(context, objective, initialGuess);
    size_t n = initialGuess->getNumElements();
    Array<double> start(n);
    memcpy(start.elemvec(), initialGuess->getValuePointer(0), sizeof (double) * n);
    cma.init(*fitness, start, 1.0);
  }

  bool isInitialized() const
    {return fitness != NULL;}

  double doIteration(size_t iteration, ExecutionContext& context)
  {
    jassert(isInitialized());
    cma.run();

    DenseDoubleVectorPtr bestSolution = initialGuess->cloneAndCast<DenseDoubleVector>();
    memcpy(bestSolution->getValuePointer(0), cma.bestSolution(), sizeof (double) * initialGuess->getNumElements());
    setBestParameters(bestSolution);
    setBestScore(cma.bestSolutionFitness());

    context.resultCallback("iteration", iteration);
    context.resultCallback("bestScore", bestScore);
    context.resultCallback("bestSolution", bestSolution);
    context.resultCallback("timesCalled", (size_t)fitness->timesCalled());
    return bestScore;
  }

protected:
  CMASearch cma;
  ObjectiveFunctionVS<double>* fitness;
  DenseDoubleVectorPtr initialGuess;
};

typedef ReferenceCountedObjectPtr<CMAOptimizerState> CMAOptimizerStatePtr;

class CMAESOptimizer : public Optimizer
{
public:
  CMAESOptimizer(size_t numIterations)
    : numIterations(numIterations) {}
  CMAESOptimizer() : numIterations(0) {}

  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const
  {
    FunctionPtr objectiveFunction = problem->getObjective();
    DenseDoubleVectorPtr initialGuess = problem->getInitialGuess().getObjectAndCast<DenseDoubleVector>();
    
    CMAOptimizerStatePtr cmaState = optimizerState.staticCast<CMAOptimizerState>();
    
    if (!cmaState->isInitialized())
      cmaState->initialize(context, objectiveFunction, initialGuess);
   
    for (size_t i = 0; i < numIterations; ++i)
    {
      context.enterScope("Iteration " + String((int)i+1));
      double res = cmaState->doIteration(i, context);
      context.leaveScope(res);
    }

    return cmaState;
  }

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context) const
    {return new CMAOptimizerState();}

protected:
  friend class CMAESOptimizerClass;

  size_t numIterations;
};

};/* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_CMA_ES_H_
