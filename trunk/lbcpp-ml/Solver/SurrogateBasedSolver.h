/*-----------------------------------------.---------------------------------.
| Filename: SurrogateBasedSolver.h         | Surrogate Based Solver          |
| Author  : Francis Maes                   |                                 |
| Started : 27/11/2012 14:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SOLVER_SURROGATE_BASED_H_
# define LBCPP_ML_SOLVER_SURROGATE_BASED_H_

# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/Sampler.h>
# include <lbcpp-ml/ExpressionDomain.h>

namespace lbcpp
{

class SurrogateBasedSolver : public IterativeSolver
{
public:
  SurrogateBasedSolver(SamplerPtr initialSampler, size_t numInitialSamples, SolverPtr surrogateLearner, SolverPtr surrogateSolver, size_t numIterations)
    : IterativeSolver(numIterations), initialSampler(initialSampler), numInitialSamples(numInitialSamples),
      surrogateLearner(surrogateLearner), surrogateSolver(surrogateSolver) {}
  SurrogateBasedSolver() : numInitialSamples(0) {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    IterativeSolver::startSolver(context, problem, callback, startingSolution);
    initialSampler->initialize(context, problem->getDomain());
    
    std::pair<ProblemPtr, TablePtr> p = createSurrogateLearningProblem(problem);
    surrogateLearningProblem = p.first;
    surrogateData = p.second;
  }

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
    addSurrogateData(object, fitness, surrogateData);      
    return true;
  }
  
protected:
  friend class SurrogateBasedSolverClass;

  SamplerPtr initialSampler;
  size_t numInitialSamples;
  SolverPtr surrogateLearner;
  SolverPtr surrogateSolver;
  
  ProblemPtr surrogateLearningProblem;
  TablePtr surrogateData;

  std::pair<ProblemPtr, TablePtr> createSurrogateLearningProblem(ProblemPtr problem)
  {
    ExpressionDomainPtr surrogateDomain = createSurrogateDomain(problem);
    TablePtr data = surrogateDomain->createTable(0);
    ProblemPtr res = new Problem();
    res->setDomain(surrogateDomain);
    res->addObjective(normalizedRMSERegressionObjective(data, surrogateDomain->getSupervision()));
    return std::make_pair(res, data);
  }
  
  ExpressionDomainPtr createSurrogateDomain(ProblemPtr problem)
  {
    ExpressionDomainPtr res = new ExpressionDomain();
    
    DomainPtr domain = problem->getDomain();
    ScalarVectorDomainPtr continuousDomain = domain.dynamicCast<ScalarVectorDomain>();
    if (continuousDomain)
    {
      for (size_t i = 0; i < continuousDomain->getNumDimensions(); ++i)
        res->addInput(doubleClass, "x" + string((int)i+1));
    }
    else
      jassertfalse;
      
    if (problem->getNumObjectives() == 1)
      res->createSupervision(doubleClass, "y");
    else
      jassertfalse;

    return res;
  }

  void addSurrogateData(ObjectPtr object, FitnessPtr fitness, TablePtr data)
  {
    std::vector<ObjectPtr> row;
    DenseDoubleVectorPtr vector = object.dynamicCast<DenseDoubleVector>();
    if (vector)
    {
      for (size_t i = 0; i < vector->getNumValues(); ++i)
        row.push_back(new Double(vector->getValue(i)));
    }
    else
      jassertfalse;
      
    if (fitness->getNumValues() == 1)
      row.push_back(new Double(fitness->getValue(0)));
    else
      jassertfalse;
      
    data->addRow(row);
  }

  ExpressionPtr learnSurrogateModel(ExecutionContext& context, ProblemPtr surrogateLearningProblem)
  {
    ExpressionPtr res;
    surrogateLearner->solve(context, surrogateLearningProblem, storeBestSolutionSolverCallback(*(ObjectPtr* )&res));
    return res;
  }
  
  struct SurrogateObjective : public Objective
  {
    SurrogateObjective(ObjectivePtr originalObjective, ExpressionPtr surrogateModel)
      : originalObjective(originalObjective), surrogateModel(surrogateModel) {}

    virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
    {
      std::vector<ObjectPtr> row;
      DenseDoubleVectorPtr vector = object.dynamicCast<DenseDoubleVector>();
      if (vector)
      {
        for (size_t i = 0; i < vector->getNumValues(); ++i)
          row.push_back(new Double(vector->getValue(i)));
      }
      else
        jassertfalse;
      return Double::get(surrogateModel->compute(context, &row[0])); // FIXME: add an exploration term
    }

    virtual void getObjectiveRange(double& worst, double& best) const
      {return originalObjective->getObjectiveRange(worst, best);}

  private:
    ObjectivePtr originalObjective;
    ExpressionPtr surrogateModel;
  };
  
  ObjectPtr optimizeSurrogate(ExecutionContext& context, ExpressionPtr surrogateModel)
  {
    ProblemPtr surrogateProblem = new Problem();
    surrogateProblem->setDomain(problem->getDomain());
    surrogateProblem->addObjective(new SurrogateObjective(problem->getObjective(0), surrogateModel));
    surrogateProblem->addValidationObjective(problem->getObjective(0));
    
    ObjectPtr res;
    FitnessPtr bestFitness;
    surrogateSolver->solve(context, surrogateProblem, storeBestSolverCallback(res, bestFitness));
    if (verbosity >= verbosityDetailed)
      context.resultCallback("surrogateObjectiveValue", bestFitness);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SOLVER_SURROGATE_BASED_H_
