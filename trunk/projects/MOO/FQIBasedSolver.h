/*-----------------------------------------.---------------------------------.
| Filename: FQIBasedSolver.h               | FQI Based Solver                |
| Author  : Francis Maes                   |                                 |
| Started : 30/11/2012 11:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SOLVER_FQI_BASED_H_
# define LBCPP_ML_SOLVER_FQI_BASED_H_

# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/Search.h>
# include <lbcpp-ml/Expression.h>
# include <lbcpp-ml/ExpressionDomain.h>
# include <lbcpp-ml/SplittingCriterion.h>
# include <lbcpp-ml/ExpressionSampler.h>

namespace lbcpp
{

//////////////// GENERIC ////////////////

class FQIBasedSolver : public PopulationBasedSolver
{
public:
  FQIBasedSolver(size_t populationSize = 100, size_t numGenerations = 0)
    : PopulationBasedSolver(populationSize, numGenerations) {}

  virtual SearchStatePtr createInitialState(DomainPtr domain) = 0;
  virtual std::pair<ObjectPtr, double> optimizeActionWrtRegressor(ExecutionContext& context, ExpressionPtr regressor, SearchStatePtr state) = 0;
  virtual ExpressionDomainPtr createRegressionDomain(ExecutionContext& context, size_t stepNumber) = 0;
  virtual std::vector<ObjectPtr> makeRegressionInput(SearchStatePtr state, ObjectPtr action) = 0;

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    DomainPtr domain = problem->getDomain();
  
    // create sampler
    SearchSamplerPtr sampler;
    if (iter == 0)
      sampler = randomSearchSampler();
    else
    {
      std::vector<ExpressionPtr> randomForests = learnFQI(context, examples);
      sampler = new RandomForestsBasedSampler(this, randomForests);
    }
    
    // initialize sampler
    SearchDomainPtr searchDomain = new SearchDomain(createInitialState(domain));
    sampler->initialize(context, searchDomain);
    
    // sample trajectories and evaluate constructed solution
    examples.reserve(examples.size() + populationSize);
    ObjectPtr bestSolution;
    FitnessPtr bestFitness;
    for (size_t i = 0; i < populationSize; ++i)
    {
      if (verbosity >= verbosityDetailed)
        context.enterScope("Sample solution");
        
      SearchTrajectoryPtr trajectory = sampler->sample(context).staticCast<SearchTrajectory>();
      trajectory->ensureStatesAreComputed(context, searchDomain->getInitialState());
      ObjectPtr solution = trajectory->getFinalState()->getConstructedObject();
      FitnessPtr fitness = evaluate(context, solution);
      examples.push_back(std::make_pair(trajectory, fitness));
      
      if (verbosity >= verbosityDetailed)
      {
        if (!bestFitness || fitness->strictlyDominates(bestFitness))
          bestFitness = fitness, bestSolution = solution;
      
        context.resultCallback("trajectory", trajectory);
        context.resultCallback("solution", solution);
        context.resultCallback("fitness", fitness);
        context.leaveScope();
      }
    }
    if (verbosity >= verbosityDetailed)
    {
      context.resultCallback("bestSolution", bestSolution);
      context.resultCallback("bestFitness", bestFitness);      
    }
    return true;
  }
  
protected:
  typedef std::vector< std::pair<SearchTrajectoryPtr, FitnessPtr> > ExampleVector;
  ExampleVector examples;

  std::vector<ExpressionPtr> learnFQI(ExecutionContext& context, const ExampleVector& examples)
  {
    // find trajectory maximum length
    size_t trajectoryMaxLength = 0;
    for (size_t i = 0; i < examples.size(); ++i)
    {
      size_t length = examples[i].first->getLength();
      if (length > trajectoryMaxLength)
        trajectoryMaxLength = length;
    }
    
    // learn regression models
    std::vector<ExpressionPtr> res(trajectoryMaxLength);
    ExpressionPtr nextModel;
    for (int i = (int)trajectoryMaxLength - 1; i >= 0; --i)
    {
      if (verbosity >= verbosityDetailed)
        context.enterScope("Learn step " + string(i));
      res[i] = learnFQIStep(context, examples, (size_t)i, nextModel);
      if (verbosity >= verbosityDetailed)
        context.leaveScope();
      nextModel = res[i];
    }
    
    return res;
  }
  
  ExpressionPtr learnFQIStep(ExecutionContext& context, const ExampleVector& examples, size_t stepNumber, ExpressionPtr nextRegressor)
  {
    // make regression domain
    ExpressionDomainPtr regressionDomain = createRegressionDomain(context, stepNumber);
  
    // make dataset
    if (verbosity >= verbosityDetailed)
      context.enterScope("Make dataset");
    TablePtr data = regressionDomain->createTable(0);
    for (size_t i = 0; i < examples.size(); ++i)
    {
      SearchTrajectoryPtr trajectory = examples[i].first;
      if (stepNumber < trajectory->getLength())
      {
        std::vector<ObjectPtr> input = makeRegressionInput(trajectory->getState(stepNumber), trajectory->getAction(stepNumber));
        double y;
        if (stepNumber < trajectory->getLength() - 1)
        {
          jassert(nextRegressor);
          // Q^t(s,a) = max_{a'} Q^{t+1}(s',a') with s'=f(s,a)
          std::pair<ObjectPtr, double> bestActionAndScore = optimizeActionWrtRegressor(context, nextRegressor, trajectory->getState(stepNumber + 1));
          y = bestActionAndScore.second;
        }
        else
          y = examples[i].second->getValue(0);  // last step: fitness value        
        
        input.push_back(new Double(y));
        data->addRow(input);
      }
    }
    
    if (verbosity >= verbosityDetailed)
    {
      context.resultCallback("data", data);
      context.leaveScope();
      context.enterScope("Learn regressor");
    }
    
    // solve regression problem
    ExpressionPtr res;
    ProblemPtr learningProblem = new Problem(regressionDomain, rmseRegressionObjective(data, regressionDomain->getSupervision()));
    SolverPtr regressor = createRegressor();
    regressor->solve(context, learningProblem, storeBestSolutionSolverCallback(*(ObjectPtr* )&res));
    
    if (verbosity >= verbosityDetailed)
      context.leaveScope();
      
    return res;
  }

  SolverPtr createRegressor()
  {
    SamplerPtr expressionVectorSampler = scalarExpressionVectorSampler();
    SolverPtr conditionLearner = randomSplitConditionLearner(expressionVectorSampler);
    //conditionLearner->setVerbosity((SolverVerbosity)verbosity);
    SolverPtr learner = treeLearner(stddevReductionSplittingCriterion(), conditionLearner, 5); 
    //learner->setVerbosity((SolverVerbosity)verbosity);
    learner = simpleEnsembleLearner(learner, 10);
    learner->setVerbosity(verbosityDetailed);
    return learner;
  }
  
  struct RandomForestsBasedSampler : public SearchSampler
  {
    RandomForestsBasedSampler(FQIBasedSolver* owner, const std::vector<ExpressionPtr>& randomForests)
      : owner(owner), randomForests(randomForests) {}

    virtual ObjectPtr sampleAction(ExecutionContext& context, SearchTrajectoryPtr trajectory) const
    {
      SearchStatePtr state = trajectory->getFinalState();
      DomainPtr actionDomain = state->getActionDomain();
      size_t stepNumber = trajectory->getLength();
      jassert(stepNumber < randomForests.size());
      AggregatorExpressionPtr randomForest = randomForests[stepNumber].staticCast<AggregatorExpression>();
      size_t modelNumber = context.getRandomGenerator()->sampleSize(randomForest->getNumSubNodes());
      ExpressionPtr regressor = randomForest->getSubNode(modelNumber);
      //ExpressionPtr regressor = randomForests[stepNumber];
      return owner->optimizeActionWrtRegressor(context, regressor, state).first;
    }
    
  protected:
    FQIBasedSolver* owner;
    std::vector<ExpressionPtr> randomForests;
  };  
};

//////////////// ScalarVector ////////////////

class ScalarVectorSearchState : public SearchState
{
public:
  ScalarVectorSearchState(ScalarVectorDomainPtr vectorDomain)
    : vectorDomain(vectorDomain), vector(new DenseDoubleVector(0, 0.0)) {}
  ScalarVectorSearchState() {}

  virtual DomainPtr getActionDomain() const
    {return vectorDomain->getScalarDomain(vector->getNumValues());}
    
  virtual void performTransition(ExecutionContext& context, const ObjectPtr& action, ObjectPtr* stateBackup = NULL)
    {vector->appendValue(Double::get(action));}
  
  virtual void undoTransition(ExecutionContext& context, const ObjectPtr& stateBackup)
    {vector->removeLastValue();}

  virtual bool isFinalState() const
    {return vector->getNumValues() == vectorDomain->getNumDimensions();}

  virtual ObjectPtr getConstructedObject() const
    {return vector;}
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    const ReferenceCountedObjectPtr<ScalarVectorSearchState>& t = target.staticCast<ScalarVectorSearchState>();
    t->vectorDomain = vectorDomain;
    t->vector = vector ? vector->cloneAndCast<DenseDoubleVector>() : DenseDoubleVectorPtr();
  }
        
protected:
  friend class ScalarVectorSearchStateClass;
  
  ScalarVectorDomainPtr vectorDomain;
  DenseDoubleVectorPtr vector;
};

class ScalarVectorFQIBasedSolver : public FQIBasedSolver
{
public:
  ScalarVectorFQIBasedSolver(size_t populationSize = 100, size_t numGenerations = 0)
    : FQIBasedSolver(populationSize, numGenerations) {}

  virtual SearchStatePtr createInitialState(DomainPtr domain)
    {return new ScalarVectorSearchState(domain.staticCast<ScalarVectorDomain>());}

  virtual std::pair<ObjectPtr, double> optimizeActionWrtRegressor(ExecutionContext& context, ExpressionPtr regressor, SearchStatePtr state)
  {
    ScalarDomainPtr domain = state->getActionDomain().staticCast<ScalarDomain>();
    
    double worst, best;
    problem->getObjective(0)->getObjectiveRange(worst, best);
    std::vector<ObjectPtr> bestActions;
    double bestScore = best < worst ? DBL_MAX : -DBL_MAX;
    context.enterScope("Optimize regressor");
    context.resultCallback("state", state);
    context.resultCallback("regressor", regressor);
    for (size_t i = 0; i < 100; ++i)
    {
      double x = domain->getLowerLimit() + i * (domain->getUpperLimit() - domain->getLowerLimit()) / 99.0;
      ObjectPtr action = new Double(x);
      std::vector<ObjectPtr> input = makeRegressionInput(state, action);
      double y = Double::get(regressor->compute(context, &input[0]));
      context.enterScope(string((int)i));
      context.resultCallback("x", x);
      context.resultCallback("y", y);
      context.leaveScope();
      
      if ((best < worst && y <= bestScore) ||
          (best > worst && y >= bestScore))
      {
        if (y != bestScore)
        {
          bestActions.clear();
          bestScore = y;
        }
        bestActions.push_back(action);
      }
    }
    context.leaveScope();
    
    return std::make_pair(bestActions[context.getRandomGenerator()->sampleSize(bestActions.size())], bestScore);
  }
  
  virtual ExpressionDomainPtr createRegressionDomain(ExecutionContext& context, size_t stepNumber)
  {
    ExpressionDomainPtr res = new ExpressionDomain();
    ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
    for (size_t i = 0; i <= stepNumber; ++i)
      res->addInput(doubleClass, "x" + string((int)i+1));
    res->createSupervision(doubleClass, "y");
    return res;
  }
  
  virtual std::vector<ObjectPtr> makeRegressionInput(SearchStatePtr state, ObjectPtr action)
  {
    DenseDoubleVectorPtr v = state->getConstructedObject().staticCast<DenseDoubleVector>();
    std::vector<ObjectPtr> res(v->getNumValues() + 1);
    for (size_t i = 0; i < v->getNumValues(); ++i)
      res[i] = new Double(v->getValue(i));
    res.back() = action;
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SOLVER_FQI_BASED_H_ 
