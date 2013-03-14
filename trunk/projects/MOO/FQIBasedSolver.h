/*-----------------------------------------.---------------------------------.
| Filename: FQIBasedSolver.h               | FQI Based Solver                |
| Author  : Francis Maes                   |                                 |
| Started : 30/11/2012 11:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SOLVER_FQI_BASED_H_
# define ML_SOLVER_FQI_BASED_H_

# include <ml/Solver.h>
# include <ml/Search.h>
# include <ml/Expression.h>
# include <ml/ExpressionDomain.h>
# include <ml/SplittingCriterion.h>
# include <ml/ExpressionSampler.h>
# include <ml/RandomVariable.h>

namespace lbcpp
{

//////////////// GENERIC ////////////////

class FQIBasedSolver : public PopulationBasedSolver
{
public:
  FQIBasedSolver(size_t populationSize = 100, size_t numGenerations = 0)
    : PopulationBasedSolver(populationSize, numGenerations) {}

  virtual SearchStatePtr createInitialState(DomainPtr domain) = 0;
  virtual std::pair<ObjectPtr, double> optimizeActionWrtRegressor(ExecutionContext& context, ExpressionPtr regressor, SearchStatePtr state, bool approximate) = 0;
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
          std::pair<ObjectPtr, double> bestActionAndScore = optimizeActionWrtRegressor(context, nextRegressor, trajectory->getState(stepNumber + 1), false);
          y = bestActionAndScore.second;
        }
        else
        {
          y = examples[i].second->getValue(0);  // last step: fitness value
          if (!this->problem->getFitnessLimits()->shouldObjectiveBeMaximised(0))
            y = -y;
          // in the tree, we treat the problem as maximization
        }
        
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
    SolverPtr learner = treeLearner(stddevReductionSplittingCriterion(), conditionLearner); 
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
      /*AggregatorExpressionPtr randomForest = randomForests[stepNumber].staticCast<AggregatorExpression>();
      size_t modelNumber = context.getRandomGenerator()->sampleSize(randomForest->getNumSubNodes());
      ExpressionPtr regressor = randomForest->getSubNode(modelNumber);*/
      ExpressionPtr regressor = randomForests[stepNumber];
      return owner->optimizeActionWrtRegressor(context, regressor, state, true).first;
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

#if 0
  struct DTNode
  {
    DTNode() : value(0.0), threshold(0.0), left(NULL), right(NULL) {}
    ~DTNode()
      {if (left) delete left; if (right) delete right;}
    
    // leaf and internal nodes
    double value;
    
    // internal node
    double threshold;
    DTNode* left;
    DTNode* right;
    
    bool isInternal() const
      {return left && right;}
  };

  DTNode* projectTree(ExecutionContext& context, ExpressionPtr tree, const DenseDoubleVectorPtr& fixedValues)
  {
    TestExpressionPtr testNode = tree.dynamicCast<TestExpression>();
    if (testNode)
    {
      FunctionExpressionPtr testCondition = testNode->getCondition().staticCast<FunctionExpression>();
      double threshold = Double::get(testCondition->getFunction()->getVariable(0));
      VariableExpressionPtr testVariable = testCondition->getSubNode(0).staticCast<VariableExpression>();
      if (testVariable->getInputIndex() < fixedValues->getNumValues())
      {
        bool success = (fixedValues->getValue(testVariable->getInputIndex()) >= threshold);
        return projectTree(context, success ? testNode->getSuccess() : testNode->getFailure(), fixedValues);
      }
      else
      {
        jassert(testVariable->getInputIndex() == fixedValues->getNumValues());
        DTNode* res = new DTNode();
        res->threshold = threshold;
        res->left = projectTree(context, testNode->getFailure(), fixedValues);
        res->right = projectTree(context, testNode->getSuccess(), fixedValues);
        res->value = juce::jmax(res->left->value, res->right->value);
        return res;
      }
    }
    else
    {
      ConstantExpressionPtr constantNode = tree.staticCast<ConstantExpression>();
      DTNode* res = new DTNode();
      res->value = Double::get(constantNode->getValue());
      return res;
    }
  }
#endif // 0

  virtual std::pair<ObjectPtr, double> optimizeActionWrtRegressor(ExecutionContext& context, ExpressionPtr regressor, SearchStatePtr state, bool approximate)
  {
    ScalarDomainPtr domain = state->getActionDomain().staticCast<ScalarDomain>();

    double score;
    std::vector<ObjectPtr> bestActions;
    /*
    if (approximate)
    {
      AggregatorExpressionPtr randomForest = regressor.staticCast<AggregatorExpression>();
      ExpressionPtr decisionTree = randomForest->getSubNode(context.getRandomGenerator()->sampleSize(randomForest->getNumSubNodes()));
      DenseDoubleVectorPtr fixedValues = state->getConstructedObject().staticCast<DenseDoubleVector>();
      DTNode* node = projectTree(context, decisionTree, fixedValues);
      
      DTNode* ptr = node;
      double lowerLimit = domain->getLowerLimit();
      double upperLimit = domain->getUpperLimit();
      while (ptr->isInternal())
      {
        DTNode* next;
        //if (context.getRandomGenerator()->sampleBool(0.9))
          next = (ptr->left->value > ptr->right->value ? ptr->left : ptr->right);
        //else
        //  next = (ptr->left->value > ptr->right->value ? ptr->right : ptr->left);
        if (next == ptr->left)
          upperLimit = ptr->threshold;
        else
          lowerLimit = ptr->threshold;
        ptr = next;
      }
      score = ptr->value;
      action = (lowerLimit + upperLimit) / 2.0;    
      delete node;
    }
    */
    //if (!approximate)
      score = -DBL_MAX;
    context.enterScope("Optimize regressor");
    context.resultCallback("state", state);
    for (size_t i = 0; i < 100; ++i)
    {
      double x = domain->getLowerLimit() + i * (domain->getUpperLimit() - domain->getLowerLimit()) / 99.0;
      //double x = context.getRandomGenerator()->sampleDouble(domain->getLowerLimit(), domain->getUpperLimit());
      std::vector<ObjectPtr> input = makeRegressionInput(state, new Double(x));
      double y = Double::get(regressor->compute(context, input));
      ScalarVariableStatisticsPtr yup = computeUpperBound(context, input, regressor.staticCast<AggregatorExpression>());
      context.enterScope(string((int)i));
      context.resultCallback("x", x);
      context.resultCallback("y", - y);
      context.resultCallback("up-mean", - yup->getMean());
      context.resultCallback("up-min", - yup->getMinimum());
      context.resultCallback("up-max", - yup->getMaximum());
      context.leaveScope();
      y = yup->getMinimum();
      if (y >= score)
      {
        if (y > score)
        {
          score = y;
          bestActions.clear();
        }
        bestActions.push_back(new Double(x));
      }
    }
    context.leaveScope();
    
    return std::make_pair(bestActions[context.getRandomGenerator()->sampleSize(bestActions.size())], score);
  }
  
  ScalarVariableStatisticsPtr computeUpperBound(ExecutionContext& context, const std::vector<ObjectPtr>& input, AggregatorExpressionPtr expression)
  {
    ScalarVariableStatisticsPtr res = new ScalarVariableStatistics("res");
    for (size_t i = 0; i < expression->getNumSubNodes(); ++i)
    {
      ExpressionPtr tree = expression->getSubNode(i);
      ScalarVectorDomainPtr leafDomain = computeLeafDomain(context, input, tree);
      double radius = getDomainRadius(leafDomain);
      res->push(Double::get(tree->compute(context, input)) + radius);
    }
    return res;
  }

  double getDomainRadius(ScalarVectorDomainPtr domain)
  {
    double res = 0.0;
    for (size_t i = 0; i < domain->getNumDimensions(); ++i)
    {
      double l = domain->getUpperLimit(i) - domain->getLowerLimit(i);
      if (l > res)
        res = l;
    }
    return res;
  }

  ScalarVectorDomainPtr computeLeafDomain(ExecutionContext& context, const std::vector<ObjectPtr>& input, ExpressionPtr tree)
  {
    std::vector<std::pair<double, double> > limits = this->problem->getDomain().staticCast<ScalarVectorDomain>()->getLimits();
    while (!tree.isInstanceOf<ConstantExpression>())
    {
      TestExpressionPtr test = tree.staticCast<TestExpression>();
      FunctionExpressionPtr testCondition = test->getCondition().staticCast<FunctionExpression>();
      double threshold = Double::get(testCondition->getFunction()->getVariable(0));
      VariableExpressionPtr testVariable = testCondition->getSubNode(0).staticCast<VariableExpression>();
      size_t index = testVariable->getInputIndex();
      bool success = (Double::get(input[index]) >= threshold);
      if (success)
      {
        limits[index].first = threshold;
        tree = test->getSuccess();
      }
      else
      {
        limits[index].second = threshold;
        tree = test->getFailure();
      }
    }
    return new ScalarVectorDomain(limits);
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

#endif // !ML_SOLVER_FQI_BASED_H_ 
