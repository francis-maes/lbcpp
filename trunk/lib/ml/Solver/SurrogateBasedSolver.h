/*-----------------------------------------.---------------------------------.
| Filename: SurrogateBasedSolver.h         | Surrogate Based Solver          |
| Author  : Francis Maes                   |                                 |
| Started : 27/11/2012 14:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SOLVER_SURROGATE_BASED_H_
# define ML_SOLVER_SURROGATE_BASED_H_

# include <ml/Solver.h>
# include <ml/Sampler.h>
# include <ml/ExpressionDomain.h>

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
    
    std::pair<ProblemPtr, TablePtr> p = createSurrogateLearningProblem(context, problem);
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
    addSurrogateData(context, object, fitness, surrogateData);      
    return true;
  }
  
protected:
  virtual void createEncodingVariables(ExecutionContext& context, DomainPtr domain, ExpressionDomainPtr res) = 0;
  virtual void encodeIntoVariables(ExecutionContext& context, ObjectPtr object, std::vector<ObjectPtr>& res) = 0;

protected:
  friend class SurrogateBasedSolverClass;

  SamplerPtr initialSampler;
  size_t numInitialSamples;
  SolverPtr surrogateLearner;
  SolverPtr surrogateSolver;
  
  ProblemPtr surrogateLearningProblem;

  TablePtr surrogateData;
  ClassPtr getSurrogateSupervisionClass() const
    {return surrogateLearningProblem->getDomain().staticCast<ExpressionDomain>()->getSupervision()->getType();}

  std::pair<ProblemPtr, TablePtr> createSurrogateLearningProblem(ExecutionContext& context, ProblemPtr problem)
  {
    ExpressionDomainPtr surrogateDomain = createSurrogateDomain(context, problem);
    TablePtr data = surrogateDomain->createTable(0);
    ProblemPtr res = new Problem();
    res->setDomain(surrogateDomain);
    if (problem->getNumObjectives() == 1)
      res->addObjective(normalizedRMSERegressionObjective(data, surrogateDomain->getSupervision()));
    else
      res->addObjective(mseMultiRegressionObjective(data, surrogateDomain->getSupervision()));
    return std::make_pair(res, data);
  }
  
  ExpressionDomainPtr createSurrogateDomain(ExecutionContext& context, ProblemPtr problem)
  {
    ExpressionDomainPtr res = new ExpressionDomain();
    
    DomainPtr domain = problem->getDomain();
    createEncodingVariables(context, domain, res);
      
    if (problem->getNumObjectives() == 1)
      res->createSupervision(doubleClass, "y");
    else
    {
      DefaultEnumerationPtr objectivesEnumeration = new DefaultEnumeration("objectives");
      for (size_t i = 0; i < problem->getNumObjectives(); ++i)
        objectivesEnumeration->addElement(context, problem->getObjective(i)->toShortString());
      ClassPtr dvClass = denseDoubleVectorClass(objectivesEnumeration, doubleClass);
      res->createSupervision(dvClass, "y");
    }
    return res;
  }

  void addSurrogateData(ExecutionContext& context, ObjectPtr object, FitnessPtr fitness, TablePtr data)
  {
    std::vector<ObjectPtr> row;
    encodeIntoVariables(context, object, row);
      
    if (fitness->getNumValues() == 1)
      row.push_back(new Double(fitness->getValue(0)));
    else
    {
      ClassPtr dvClass = getSurrogateSupervisionClass();
      DenseDoubleVectorPtr fitnessVector(new DenseDoubleVector(dvClass));
      for (size_t i = 0; i < fitness->getNumValues(); ++i)
        fitnessVector->setValue(i, fitness->getValue(i));
      row.push_back(fitnessVector);
    }
    surrogateLearningProblem->getObjective(0).staticCast<LearningObjective>()->getIndices()->append(data->getNumRows());
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
    SurrogateObjective(SurrogateBasedSolver* owner, size_t objectiveNumber, ExpressionPtr surrogateModel)
      : owner(owner), objectiveNumber(objectiveNumber), surrogateModel(surrogateModel) {}

    // FIXME: add an exploration term

    virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
    {
      std::vector<ObjectPtr> row;
      owner->encodeIntoVariables(context, object, row);
      ObjectPtr prediction = surrogateModel->compute(context, &row[0]);
      if (prediction.isInstanceOf<Double>())
      {
        jassert(objectiveNumber == 0);
        return Double::get(prediction);
      }
      else
      {
        DenseDoubleVectorPtr pred = prediction.staticCast<DenseDoubleVector>();
        jassert(objectiveNumber < pred->getNumValues());
        return pred->getValue(objectiveNumber);
      }
    }

    virtual void getObjectiveRange(double& worst, double& best) const
      {return owner->problem->getObjective(objectiveNumber)->getObjectiveRange(worst, best);}

  private:
    SurrogateBasedSolver* owner;
    size_t objectiveNumber;
    ExpressionPtr surrogateModel;
  };
  
  ProblemPtr createSurrogateOptimizationProblem(ExpressionPtr surrogateModel)
  {
    ProblemPtr res = new Problem();
    res->setDomain(problem->getDomain());
    for (size_t i = 0; i < problem->getNumObjectives(); ++i)
    {
      res->addObjective(new SurrogateObjective(this, i, surrogateModel));
      res->addValidationObjective(problem->getObjective(i));
    }
    return res;
  }

  ObjectPtr optimizeSurrogate(ExecutionContext& context, ExpressionPtr surrogateModel)
  {
    ProblemPtr surrogateProblem = createSurrogateOptimizationProblem(surrogateModel);
    ObjectPtr res;
    FitnessPtr bestFitness;
    surrogateSolver->solve(context, surrogateProblem, storeBestSolverCallback(res, bestFitness));
    if (verbosity >= verbosityDetailed)
      context.resultCallback("surrogateObjectiveValue", bestFitness);
    return res;
  }
};

class ContinuousSurrogateBasedSolver : public SurrogateBasedSolver
{
public:
  ContinuousSurrogateBasedSolver(SamplerPtr initialSampler, size_t numInitialSamples, SolverPtr surrogateLearner, SolverPtr surrogateSolver, size_t numIterations)
    : SurrogateBasedSolver(initialSampler, numInitialSamples, surrogateLearner, surrogateSolver, numIterations) {}
  ContinuousSurrogateBasedSolver() {}

  virtual void createEncodingVariables(ExecutionContext& context, DomainPtr domain, ExpressionDomainPtr res)
  {
    ScalarVectorDomainPtr continuousDomain = domain.dynamicCast<ScalarVectorDomain>();
    jassert(continuousDomain);
    for (size_t i = 0; i < continuousDomain->getNumDimensions(); ++i)
      res->addInput(doubleClass, "x" + string((int)i+1));
  }

  virtual void encodeIntoVariables(ExecutionContext& context, ObjectPtr object, std::vector<ObjectPtr>& res)
  {
    DenseDoubleVectorPtr vector = object.dynamicCast<DenseDoubleVector>();
    jassert(vector);
    for (size_t i = 0; i < vector->getNumValues(); ++i)
      res.push_back(new Double(vector->getValue(i)));
  }
};

}; /* namespace lbcpp */

#endif // !ML_SOLVER_SURROGATE_BASED_H_
