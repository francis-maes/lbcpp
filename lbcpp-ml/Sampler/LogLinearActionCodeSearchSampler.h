/*-----------------------------------------.---------------------------------.
| Filename: LogLinearActionCodeSearchSampler.h | Search Sampler based on     |
| Author  : Francis Maes                   | action codes                    |
| Started : 05/10/2012 11:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SAMPLER_SEARCH_LOG_LINEAR_ACTION_CODE_H_
# define LBCPP_ML_SAMPLER_SEARCH_LOG_LINEAR_ACTION_CODE_H_

# include <lbcpp-ml/Search.h>
# include <lbcpp-ml/Optimizer.h>
# include <lbcpp-ml/Problem.h>
# include <lbcpp-ml/SolutionSet.h>

namespace lbcpp
{
  
class LogLinearActionCodeLearningProblem : public ContinuousDerivableProblem
{
public:
  struct Example
  {
    std::vector<size_t> availableActions;
    std::map<size_t, size_t> countsPerAction;
  };

  LogLinearActionCodeLearningProblem(const std::vector<Example>& examples, double regularizer, const DenseDoubleVectorPtr& initialParameters)
    : examples(examples), regularizer(regularizer), initialParameters(initialParameters)
  {
    domain = new ContinuousDomain(std::vector<std::pair<double, double> >(initialParameters->getNumValues(), std::make_pair(-DBL_MAX, DBL_MAX)));
    limits = new FitnessLimits(std::vector<std::pair<double, double> >(1, std::make_pair(DBL_MAX, 0.0))); // minimization problem
  }

  // minimize sum_examples (-sum_selectedActions (actionSelectedCount * theta[selectedAction])
  //                        +sum_selectedActions (actionSelectedCount) * log sum_i exp(theta[example.availableAction[i])) / num_examples
  //                + lambda * sumOfSquares(theta) / 2
  virtual void evaluate(ExecutionContext& context, const DenseDoubleVectorPtr& parameters, size_t objectiveNumber, double* value, DoubleVectorPtr* gradient)
  {
    jassert(objectiveNumber == 0);
    jassert(!value || *value == 0.0);

    DenseDoubleVectorPtr denseGradient;
    if (gradient)
    {
      denseGradient = new DenseDoubleVector(parameters->getClass());
      denseGradient->ensureSize(parameters->getNumValues());
      *gradient = denseGradient;
    }

    size_t totalNumExamples = 0;
    for (size_t i = 0; i < examples.size(); ++i)
    {
      const Example& example = examples[i];

      size_t totalCount = 0;
      for (std::map<size_t, size_t>::const_iterator it = example.countsPerAction.begin(); it != example.countsPerAction.end(); ++it)
      {
        if (value)
          *value -= getParameter(parameters, it->first) * it->second;
        if (denseGradient)
          denseGradient->decrementValue(it->first, it->second);
        totalCount += it->second;
      }
      totalNumExamples += totalCount;

      DenseDoubleVectorPtr activations = new DenseDoubleVector(example.availableActions.size(), 0.0);
      for (size_t j = 0; j < example.availableActions.size(); ++j)
        activations->setValue(j, getParameter(parameters, example.availableActions[j]));
      double logSumExp = activations->computeLogSumOfExponentials();

      if (value)
        *value += totalCount * logSumExp;
      if (denseGradient)
      {
        for (size_t j = 0; j < example.availableActions.size(); ++j)
          denseGradient->incrementValue(example.availableActions[j], totalCount * exp(activations->getValue(j) - logSumExp));
      }
    }

    if (value)
    {
      *value /= (double)totalNumExamples;
      *value += regularizer * parameters->sumOfSquares() / 2.0;
    }
    if (denseGradient)
    {
      denseGradient->multiplyByScalar(1.0 / (double)totalNumExamples);
      if (regularizer)
        parameters->addWeightedTo(denseGradient, 0, regularizer);
    }
  }

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
    {return initialParameters;}

protected:
  std::vector<Example> examples;
  double regularizer;
  DenseDoubleVectorPtr initialParameters;

  static double getParameter(const DenseDoubleVectorPtr& parameters, size_t index)
    {return parameters && index < parameters->getNumValues() ? parameters->getValue(index) : 0.0;}
};

class LogLinearActionCodeSearchSampler : public SearchSampler
{
public:
  LogLinearActionCodeSearchSampler(double regularizer = 0.1, double learningRate = 1.0)
    : regularizer(regularizer), learningRate(learningRate), deterministic(false) {}

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    SearchTrajectoryPtr res(new SearchTrajectory());
    SearchStatePtr state = domain->createInitialState();
    bool det = true;
    while (!state->isFinalState())
    {
      ObjectPtr action = sampleAction(context, state, det);
      res->append(state->cloneAndCast<SearchState>(), action);
      state->performTransition(context, action);
    }
    res->setFinalState(state);
    const_cast<LogLinearActionCodeSearchSampler* >(this)->deterministic = det;
    return res;
  }

  virtual bool isDeterministic() const // returns true if the sampler has became deterministic
    {return deterministic;}

  typedef LogLinearActionCodeLearningProblem::Example Example;

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects)
  {
    std::vector<size_t> indices(objects.size());
    for (size_t i = 0; i < indices.size(); ++i)
      indices[i] = i;
    
    std::vector<Example> dataset;
    size_t highestActionCode = 0;
    makeDatasetRecursively(*(const std::vector<SearchTrajectoryPtr>* )&objects, indices, 0, dataset, highestActionCode);

    if (!parameters)
      parameters = new DenseDoubleVector(highestActionCode + 1, 0.0);
    else
      parameters->ensureSize(highestActionCode + 1);

    ContinuousDerivableProblemPtr learningProblem = new LogLinearActionCodeLearningProblem(dataset, regularizer, parameters);
    OptimizerPtr optimizer = lbfgsOptimizer();
    context.enterScope("LBFGS");
    ParetoFrontPtr front = optimizer->optimize(context, learningProblem, Optimizer::verbosityAll);
    context.leaveScope();
    if (front->getNumSolutions())
      parameters = front->getSolution(0)->getObject().staticCast<DenseDoubleVector>();
  }

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object)
  {
    SearchTrajectoryPtr trajectory = object.staticCast<SearchTrajectory>();
    
    SparseDoubleVectorPtr gradient = new SparseDoubleVector();
    size_t n = trajectory->getLength();
    for (size_t i = 0; i < n; ++i)
    {
      SearchStatePtr state = trajectory->getState(i);
      ObjectPtr action = trajectory->getAction(i);
      DiscreteDomainPtr actionDomain = state->getActionDomain();

      size_t code = domain->getActionCode(state, action);
      gradient->incrementValue(code, 1.0);

      size_t numActions = actionDomain->getNumElements();
      std::vector< std::pair<size_t, double> > codesAndProbabilities(numActions);
      double Z = 0.0;
      for (size_t j = 0; j < numActions; ++j)
      {
        size_t code = domain->getActionCode(state, actionDomain->getElement(j));
        double probability = exp(getParameter(code));
        codesAndProbabilities[j] = std::make_pair(code, probability);
        Z += probability;
      }
      for (size_t j = 0; j < numActions; ++j)
        gradient->incrementValue(codesAndProbabilities[j].first, -codesAndProbabilities[j].second / Z);
    }

    if (!parameters)
      parameters = new DenseDoubleVector(0, 0.0);
    gradient->addWeightedTo(parameters, 0, learningRate);
  }
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const ReferenceCountedObjectPtr<LogLinearActionCodeSearchSampler>& target = t.staticCast<LogLinearActionCodeSearchSampler>();
    target->domain = domain;
    target->learningRate = learningRate;
    target->parameters = parameters ? parameters->cloneAndCast<DenseDoubleVector>() : DenseDoubleVectorPtr();
  }

protected:
  friend class LogLinearActionCodeSearchSamplerClass;

  double regularizer;
  double learningRate;

  DenseDoubleVectorPtr parameters;
  bool deterministic;

  double getParameter(size_t index) const
    {return parameters && parameters->getNumValues() > index ? parameters->getValue(index) : 0.0;}

  ObjectPtr sampleAction(ExecutionContext& context, SearchStatePtr state, bool& isDeterministic) const
  {
    DiscreteDomainPtr actionDomain = state->getActionDomain().staticCast<DiscreteDomain>();
    size_t n = actionDomain->getNumElements();
    
    std::vector<double> probabilities(n, 0.0);
    double Z = 0.0;
    double highestProbability = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      size_t code = domain->getActionCode(state, actionDomain->getElement(i));
      double p = exp(getParameter(code));
      probabilities[i] = p;
      highestProbability = juce::jmax(p, highestProbability);
      Z += p;
    }

    if (highestProbability < Z * (1 - 1e-9))
      isDeterministic = false;

    size_t index = context.getRandomGenerator()->sampleWithProbabilities(probabilities, Z);
    return actionDomain->getElement(index);
  }
  
  void makeDatasetRecursively(const std::vector<SearchTrajectoryPtr>& trajectories, const std::vector<size_t>& indices, size_t step, std::vector<Example>& res, size_t& highestActionCode)
  {
    if (step == trajectories[indices[0]]->getLength())
      return;

    jassert(indices.size());
    SearchStatePtr state = trajectories[indices[0]]->getState(step);
    DiscreteDomainPtr actionDomain = state->getActionDomain().staticCast<DiscreteDomain>();

    typedef std::map<size_t, std::vector<size_t> > DispatchMap; // action code -> trajectory indices
    
    Example example;
    example.availableActions.resize(actionDomain->getNumElements());
    for (size_t i = 0; i < example.availableActions.size(); ++i)
    {
      size_t code = domain->getActionCode(state, actionDomain->getElement(i));
      if (code > highestActionCode)
        highestActionCode = code;
      example.availableActions[i] = code;
    }

    DispatchMap m;
    for (size_t i = 0; i < indices.size(); ++i)
    {
      ObjectPtr action = trajectories[indices[i]]->getAction(step);
      size_t actionCode = domain->getActionCode(state, action);
      m[actionCode].push_back(indices[i]);
    }
    for (DispatchMap::const_iterator it = m.begin(); it != m.end(); ++it)
    {
      example.countsPerAction[it->first] = it->second.size();
      makeDatasetRecursively(trajectories, it->second, step + 1, res, highestActionCode);
    }
    res.push_back(example);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SAMPLER_SEARCH_LOG_LINEAR_ACTION_CODE_H_
