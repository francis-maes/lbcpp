/*-----------------------------------------.---------------------------------.
| Filename: LuapeEDASandBox.h              | An EDA for LuapeNodes           |
| Author  : Francis Maes                   |                                 |
| Started : 19/01/2012 18:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_EDA_SAND_BOX_H_
# define LBCPP_LUAPE_EDA_SAND_BOX_H_

# include <lbcpp/Luape/LuapeBatchLearner.h>
# include <lbcpp/Luape/LuapeLearner.h>

namespace lbcpp
{

class EvaluateLuapePolicyFunction : public SimpleUnaryFunction
{
public:
  EvaluateLuapePolicyFunction(size_t horizon = 300)
    : SimpleUnaryFunction(luapeRegressorClass, doubleType), horizon(horizon) {}

  virtual DecisionProblemPtr getDecisionProblem() const = 0;
  virtual size_t getNumTrajectories() const
    {return 1;}
  virtual DecisionProblemStatePtr sampleInitialState(RandomGeneratorPtr random) const = 0;
  virtual bool breakTrajectory(size_t timeStep, double reward, double cumulativeReward) const
    {return false;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    LuapeRegressorPtr regressor = input.getObjectAndCast<LuapeRegressor>();
    RandomGeneratorPtr random = context.getRandomGenerator();

    size_t numTrajectories = getNumTrajectories();
    double res = 0.0;
    double discount = getDecisionProblem()->getDiscount();
    for (size_t trajectory = 0; trajectory < numTrajectories; ++trajectory)
    {
      double cumulativeReward = 0.0;
      DecisionProblemStatePtr state = sampleInitialState(random);
      for (size_t t = 0; t < horizon; ++t)
      {
        ContainerPtr actions = state->getAvailableActions();
        size_t n = actions->getNumElements();
        double bestScore = -DBL_MAX;
        DecisionProblemStatePtr bestNextState;
        double bestReward = 0.0;
        for (size_t j = 0; j < n; ++j)
        {
          DecisionProblemStatePtr nextState = state->cloneAndCast<DecisionProblemState>();
          Variable action = actions->getElement(j);
          double reward;
          nextState->performTransition(context, action, reward, NULL);
          double score = regressor->compute(context, nextState, doubleMissingValue).getDouble();
          if (score > bestScore)
            bestScore = score, bestNextState = nextState, bestReward = reward;
        }
        jassert(bestNextState);
        state = bestNextState;
        cumulativeReward += pow(discount, (double)t) * bestReward; 
        if (breakTrajectory(t, bestReward, cumulativeReward))
          break;
      }
      res += cumulativeReward;
    }
    return res / numTrajectories;
  }

protected:
  friend class EvaluateLuapePolicyFunctionClass;

  size_t horizon;
};

typedef ReferenceCountedObjectPtr<EvaluateLuapePolicyFunction> EvaluateLuapePolicyFunctionPtr;

class EvaluateLPPLuapePolicyFunction : public EvaluateLuapePolicyFunction
{
public:
  EvaluateLPPLuapePolicyFunction(size_t horizon = 50)
    : EvaluateLuapePolicyFunction(50) {}

  virtual DecisionProblemPtr getDecisionProblem() const
    {return new LinearPointPhysicProblem();}

  virtual size_t getNumTrajectories() const
    {return 10;}

  virtual DecisionProblemStatePtr sampleInitialState(RandomGeneratorPtr random) const
    {return new LinearPointPhysicState(random->sampleDouble(-1.0, 1.0), random->sampleDouble(-2.0, 2.0));}
};

class EvaluateHIVLuapePolicyFunction : public EvaluateLuapePolicyFunction
{
public:
  EvaluateHIVLuapePolicyFunction(size_t horizon = 300)
    : EvaluateLuapePolicyFunction(horizon) {}

  virtual DecisionProblemPtr getDecisionProblem() const
    {return new HIVDecisionProblem();}

  virtual DecisionProblemStatePtr sampleInitialState(RandomGeneratorPtr random) const
  {
    std::vector<double> initialState(6);
    initialState[0] = 163573;
    initialState[1] = 5;
    initialState[2] = 11945;
    initialState[3] = 46;
    initialState[4] = 63919;
    initialState[5] = 24;
    //for (size_t i = 0; i < initialState.size(); ++i)
    //  initialState[i] *= random->sampleDouble(0.8, 1.2);
    return new HIVDecisionProblemState(initialState);
  }

  virtual bool breakTrajectory(size_t timeStep, double reward, double cumulativeReward) const
    {return timeStep >= 50 && reward < 5e6;}
};

class LuapeEDASandBox : public WorkUnit
{
public:
  LuapeEDASandBox() : complexity(4), populationSize(1000), numBests(100), numIterations(100) {}

  struct PopulationComparator
  {
    static size_t size(const LuapeNodePtr& node)
    {
      size_t res = 1;
      for (size_t i = 0; i < node->getNumSubNodes(); ++i)
        res += size(node->getSubNode(i));
      return res;
    }

    bool operator() (const std::pair<LuapeNodePtr, double>& left, const std::pair<LuapeNodePtr, double>& right) const
    {
      if (left.second != right.second)
        return left.second > right.second;
      size_t s1 = size(left.first);
      size_t s2 = size(right.first);
      if (s1 != s2)
        return s1 < s2;
      return left.first < right.first;
    }
  };

  virtual Variable run(ExecutionContext& context)
  {
    //EvaluateLuapePolicyFunctionPtr objective = new EvaluateHIVLuapePolicyFunction(300);
    EvaluateLuapePolicyFunctionPtr objective = new EvaluateLPPLuapePolicyFunction();

    DecisionProblemPtr decisionProblem = objective->getDecisionProblem();

    ClassPtr stateClass = decisionProblem->getStateClass();

    LuapeRegressorPtr regressor = new LuapeRegressor();
    if (!regressor->initialize(context, stateClass, doubleType))
      return false;
    regressor->addInput(stateClass, "s");
    //regressor->addFunction(andBooleanLuapeFunction());
    //regressor->addFunction(equalBooleanLuapeFunction());
    regressor->addFunction(logDoubleLuapeFunction());
    regressor->addFunction(sqrtDoubleLuapeFunction());
    regressor->addFunction(addDoubleLuapeFunction());
    regressor->addFunction(subDoubleLuapeFunction());
    regressor->addFunction(mulDoubleLuapeFunction());
    regressor->addFunction(divDoubleLuapeFunction());
    regressor->addFunction(minDoubleLuapeFunction());
    regressor->addFunction(maxDoubleLuapeFunction());
    //regressor->addFunction(greaterThanDoubleLuapeFunction());
    regressor->addFunction(getVariableLuapeFunction());
    regressor->addFunction(new GetDecisionProblemSuccessorState(decisionProblem->getFixedNumberOfActions()));

    std::vector< std::pair<LuapeNodePtr, double> > population;
    std::set<LuapeNodePtr> processedNodes;

    LuapeNodeBuilderPtr nodeBuilder = biasedRandomSequentialNodeBuilder(populationSize / 4, complexity, 0.0);
    for (size_t i = 0; i < numIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i));
      context.resultCallback("iteration", i);
     
      context.enterScope(T("Generating candidates"));
      std::vector<LuapeNodePtr> candidates;
      for (size_t trial = 0; trial < 100 && candidates.size() < populationSize; ++trial)
      {
        std::vector<LuapeNodePtr> c;
        nodeBuilder->buildNodes(context, regressor, 0, c);
        for (size_t i = 0; i < c.size(); ++i)
          if (processedNodes.find(c[i]) == processedNodes.end())
          {
            context.informationCallback(T("Candidate: ") + c[i]->toShortString());
            processedNodes.insert(c[i]);
            candidates.push_back(c[i]);
          }
      }
      if (candidates.size() > populationSize)
        candidates.erase(candidates.begin() + populationSize, candidates.end());
      context.leaveScope(candidates.size());

      context.enterScope(T("Evaluating candidates"));
      population.reserve(population.size() + candidates.size());
      size_t numEvaluated = 0;
      for (size_t j = 0; j < candidates.size(); ++j)
      {
        LuapeNodePtr candidate = candidates[j];
        regressor->setRootNode(context, candidate);
        double score = objective->compute(context, regressor).getDouble();
        population.push_back(std::make_pair(candidate, score));
        context.progressCallback(new ProgressionState(j+1, candidates.size(), T("Candidates")));
        ++numEvaluated;
      }
      regressor->setRootNode(context, LuapeNodePtr());
      context.leaveScope(numEvaluated);

      context.enterScope(T("Updating population"));
      std::sort(population.begin(), population.end(), PopulationComparator());
      if (population.size() > populationSize)
        population.erase(population.begin() + populationSize, population.end());
      
      double currentScore = doubleMissingValue;
      size_t currentIndex = 0;
      for (size_t j = 0; j < population.size(); ++j)
      {
        if (population[j].second != currentScore)
        {
          currentScore = population[j].second;
          currentIndex = j;
        }
        population[j].first->addImportance(1.0 / (currentIndex + 1.0));
      }

      context.leaveScope(population.front().second);

      context.resultCallback(T("bestScore"), population.front().second);
      context.resultCallback(T("worstScore"), population.back().second);

      for (size_t i = 0; i < 10; ++i)
      {
        if (i >= population.size())
          break;
        context.informationCallback(T("Top ") + String((int)i + 1) + T(": ") + population[i].first->toShortString() + T(" [") + Variable(population[i].second).toShortString() + T("]"));
      }

      context.leaveScope(population.front().second);
    }

    return true;
  }

protected:
  friend class LuapeEDASandBoxClass;

  size_t complexity;
  size_t populationSize;
  size_t numBests;
  size_t numIterations;
};


}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_EDA_SAND_BOX_H_

