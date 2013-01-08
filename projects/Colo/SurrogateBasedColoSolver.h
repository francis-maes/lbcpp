/*-----------------------------------------.---------------------------------.
| Filename: SurrogateBasedColoSolver.h     | Surrogate Based Colo Solver     |
| Author  : Francis Maes                   |                                 |
| Started : 08/01/2013 16:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef COLO_SOLVER_SURROGATE_BASED_H_
# define COLO_SOLVER_SURROGATE_BASED_H_

# include <ml/Solver.h>
# include <ml/Sampler.h>
# include <ml/ExpressionDomain.h>
# include "ColoProblem.h"
# include "../../lib/ml/Solver/SurrogateBasedSolver.h"

namespace lbcpp
{

class SurrogateBasedMOSolver : public SurrogateBasedSolver
{
public:
  SurrogateBasedMOSolver(SamplerPtr initialSampler, size_t numInitialSamples, SolverPtr surrogateLearner, SolverPtr surrogateSolver, size_t numIterations)
    : SurrogateBasedSolver(initialSampler, numInitialSamples, surrogateLearner, surrogateSolver, numIterations) {}
  SurrogateBasedMOSolver() {}

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    ObjectPtr object;
    if (iter < numInitialSamples)
    {
      // make random sample
      object = initialSampler->sample(context);
    }
    else
    {
      // learn surrogate
      if (verbosity >= verbosityDetailed)
        context.enterScope("Learn surrogate");
      ExpressionPtr surrogateModel = learnSurrogateModel(context, surrogateLearningProblem);
      if (verbosity >= verbosityDetailed)
      {
        context.resultCallback("surrogateModel", surrogateModel);
        context.leaveScope();
      }
      
      // optimize surrogate
      if (verbosity >= verbosityDetailed)
        context.enterScope("Optimize surrogate");
      object = optimizeSurrogate(context, surrogateModel);
      if (verbosity >= verbosityDetailed)
      {
        context.resultCallback("object", object);
        context.leaveScope();
      }        
    }
    
    // evaluate point and add to training data
    FitnessPtr fitness = evaluate(context, object);
    if (verbosity >= verbosityDetailed)
      context.resultCallback("fitness", fitness);
    addSurrogateData(context, object, fitness, surrogateData);      
    return true;
  }
};

class ColoSurrogateBasedMOSolver : public SurrogateBasedMOSolver
{
public:
  ColoSurrogateBasedMOSolver(SamplerPtr initialSampler, size_t numInitialSamples, SolverPtr surrogateLearner, SolverPtr surrogateSolver, size_t numIterations)
    : SurrogateBasedMOSolver(initialSampler, numInitialSamples, surrogateLearner, surrogateSolver, numIterations) {}
  ColoSurrogateBasedMOSolver() {}

  virtual void createEncodingVariables(ExecutionContext& context, DomainPtr domain, ExpressionDomainPtr res)
  {
    ColoDomainPtr coloDomain = domain.staticCast<ColoDomain>();
    size_t numFlags = coloDomain->getNumFlags();
    for (size_t i = 0; i < numFlags; ++i)
      res->addInput(positiveIntegerClass, "flag" + string((int)i+1));
    for (size_t i = 0; i < numFlags; ++i)
      for (size_t j = 0; j < numFlags; ++j)
        res->addInput(positiveIntegerClass, "flag" + string((int)i+1) + "before" + string((int)j+1));
  }

  virtual void encodeIntoVariables(ExecutionContext& context, ObjectPtr solution, std::vector<ObjectPtr>& res)
  {
    ColoDomainPtr coloDomain = problem->getDomain().staticCast<ColoDomain>();
    size_t numFlags = coloDomain->getNumFlags();

    ColoObjectPtr coloObject = solution.staticCast<ColoObject>();
    std::vector<size_t> counts(numFlags + numFlags * (numFlags - 1) / 2, 0);
    for (size_t j = 0; j < coloObject->getLength(); ++j)
    {
      size_t flag = coloObject->getFlag(j);
      counts[flag]++;
      for (size_t i = 0; i < j; ++i)
      {
        size_t previousFlag = coloObject->getFlag(i);
        counts[numFlags + previousFlag * numFlags + flag]++;
      }
    }
    res.resize(counts.size());
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = new PositiveInteger(counts[i]);
  }
};

}; /* namespace lbcpp */

#endif // !COLO_SOLVER_SURROGATE_BASED_H_
