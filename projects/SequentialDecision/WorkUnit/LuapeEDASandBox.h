/*-----------------------------------------.---------------------------------.
| Filename: LuapeEDASandBox.h              | An EDA for Expressions           |
| Author  : Francis Maes                   |                                 |
| Started : 19/01/2012 18:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_EDA_SAND_BOX_H_
# define LBCPP_LUAPE_EDA_SAND_BOX_H_

# include <lbcpp/Luape/LuapeBatchLearner.h>
# include <lbcpp/Luape/LuapeLearner.h>
# include "../Problem/MountainCarProblem.h"
# include "../Problem/LeftOrRightControlProblem.h"

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
    
  virtual bool breakTrajectory(size_t timeStep, double reward, double cumulativeReward) const
    {return false;}

  void illustratePolicy(ExecutionContext& context, const LuapeRegressorPtr& regressor)
  {
    double discount = getDecisionProblem()->getDiscount();
    double cumulativeReward = 0.0;
    DecisionProblemStatePtr state = initialStates[0]->cloneAndCast<DecisionProblemState>();
    context.enterScope(T("Trajectory with ") + regressor->getRootNode()->toShortString());
    for (size_t t = 0; t < horizon; ++t)
    {
      context.enterScope(T("TimeStep ") + String((int)t));
      context.resultCallback(T("timeStep"), t);
      for (size_t j = 0; j < state->getNumVariables(); ++j)
      {
        Variable v = state->getVariable(j);
        if (v.isDouble())
          context.resultCallback(state->getVariableName(j), v);
      }

      ContainerPtr actions = state->getAvailableActions();
      size_t n = actions->getNumElements();
      context.resultCallback(T("numActions"), n);
      double bestScore = -DBL_MAX;
      Variable bestAction;
      for (size_t j = 0; j < n; ++j)
      {
        Variable action = actions->getElement(j);
/*          DecisionProblemStatePtr nextState = state->cloneAndCast<DecisionProblemState>();
        double reward;
        nextState->performTransition(context, action, reward, NULL);*/
        double score = regressor->compute(context, new Pair(state, action), doubleMissingValue).getDouble();
        context.resultCallback(T("score[") + String((int)j) + T("]"), score);
        if (score > bestScore)
          bestScore = score, bestAction = action;
      }
      jassert(bestAction.exists());
      double reward;
      state->performTransition(context, bestAction, reward);
      cumulativeReward += pow(discount, (double)t) * reward; 
      
      context.resultCallback(T("action"), bestAction);
      context.resultCallback(T("reward"), reward);
      context.resultCallback(T("return"), cumulativeReward);
      context.leaveScope(cumulativeReward);
      
      if (state->isFinalState() || breakTrajectory(t, reward, cumulativeReward))
        break;
    }
    context.leaveScope();
  }

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    DecisionProblemPtr problem = getDecisionProblem();

    initialStates.resize(getNumTrajectories());
    for (size_t i = 0; i < initialStates.size(); ++i)
      initialStates[i] = problem->sampleInitialState(context);
    return SimpleUnaryFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    LuapeRegressorPtr regressor = input.getObjectAndCast<LuapeRegressor>();

    double res = 0.0;
    double discount = getDecisionProblem()->getDiscount();
    for (size_t trajectory = 0; trajectory < initialStates.size(); ++trajectory)
    {
      double cumulativeReward = 0.0;
      DecisionProblemStatePtr state = initialStates[trajectory]->cloneAndCast<DecisionProblemState>();
      for (size_t t = 0; t < horizon; ++t)
      {
        ContainerPtr actions = state->getAvailableActions();
        size_t n = actions->getNumElements();
        double bestScore = -DBL_MAX;
        Variable bestAction;
        for (size_t j = 0; j < n; ++j)
        {
          Variable action = actions->getElement(j);
/*          DecisionProblemStatePtr nextState = state->cloneAndCast<DecisionProblemState>();
          double reward;
          nextState->performTransition(context, action, reward, NULL);*/
          double score = regressor->compute(context, new Pair(state, action), doubleMissingValue).getDouble();
          if (score > bestScore)
            bestScore = score, bestAction = action;
        }
        jassert(bestAction.exists());
        double reward;
        state->performTransition(context, bestAction, reward);
        cumulativeReward += pow(discount, (double)t) * reward; 
        if (state->isFinalState() || breakTrajectory(t, reward, cumulativeReward))
          break;
      }
      res += cumulativeReward;
    }
    return res / initialStates.size();
  }

protected:
  friend class EvaluateLuapePolicyFunctionClass;

  size_t horizon;
  std::vector<DecisionProblemStatePtr> initialStates;
};

typedef ReferenceCountedObjectPtr<EvaluateLuapePolicyFunction> EvaluateLuapePolicyFunctionPtr;

class EvaluateLPPLuapePolicyFunction : public EvaluateLuapePolicyFunction
{
public:
  EvaluateLPPLuapePolicyFunction(size_t horizon = 50)
    : EvaluateLuapePolicyFunction(horizon) {}

  virtual DecisionProblemPtr getDecisionProblem() const
    {return new LinearPointPhysicProblem();}

  virtual size_t getNumTrajectories() const
    {return 100;}
};

class EvaluateMountainCarLuapePolicyFunction : public EvaluateLuapePolicyFunction
{
public:
  EvaluateMountainCarLuapePolicyFunction(size_t horizon = 2500)
    : EvaluateLuapePolicyFunction(horizon) {}

  virtual DecisionProblemPtr getDecisionProblem() const
    {return new MountainCarProblem();}

  virtual size_t getNumTrajectories() const
    {return 100;}
};


class EvaluateLeftOrRightLuapePolicyFunction : public EvaluateLuapePolicyFunction
{
public:
  EvaluateLeftOrRightLuapePolicyFunction(size_t horizon = 20)
    : EvaluateLuapePolicyFunction(horizon) {}

  virtual DecisionProblemPtr getDecisionProblem() const
    {return new LeftOrRightControlProblem();}

  virtual size_t getNumTrajectories() const
    {return 100;}
};

class EvaluateHIVLuapePolicyFunction : public EvaluateLuapePolicyFunction
{
public:
  EvaluateHIVLuapePolicyFunction(size_t horizon = 300)
    : EvaluateLuapePolicyFunction(horizon) {}

  virtual DecisionProblemPtr getDecisionProblem() const
    {return new HIVDecisionProblem();}

  virtual bool breakTrajectory(size_t timeStep, double reward, double cumulativeReward) const
    {return timeStep >= 50 && reward < 5e6;}
};

class LuapeEDASandBox : public WorkUnit
{
public:
  LuapeEDASandBox() : complexity(4), populationSize(1000), numBests(100), numIterations(100) {}

  struct PopulationComparator
  {
    static size_t size(const ExpressionPtr& node)
    {
      size_t res = 1;
      for (size_t i = 0; i < node->getNumSubNodes(); ++i)
        res += size(node->getSubNode(i));
      return res;
    }

    bool operator() (const std::pair<ExpressionPtr, double>& left, const std::pair<ExpressionPtr, double>& right) const
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
    //EvaluateLuapePolicyFunctionPtr objective = new EvaluateLPPLuapePolicyFunction();
    //EvaluateLuapePolicyFunctionPtr objective = new EvaluateMountainCarLuapePolicyFunction();
    EvaluateLuapePolicyFunctionPtr objective = new EvaluateLeftOrRightLuapePolicyFunction();
    if (!objective->initialize(context, luapeRegressorClass))
      return false;

    DecisionProblemPtr decisionProblem = objective->getDecisionProblem();

    ClassPtr stateClass = decisionProblem->getStateClass();
    TypePtr actionType = decisionProblem->getActionType();

    LuapeRegressorPtr regressor = new LuapeRegressor();
    if (!regressor->initialize(context, pairClass(stateClass, actionType), doubleType))
      return false;
    regressor->addInput(stateClass, "s");
    regressor->addInput(actionType, "a");
    regressor->addFunction(andBooleanLuapeFunction());
    regressor->addFunction(equalBooleanLuapeFunction());
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
    regressor->addFunction(getDoubleVectorElementLuapeFunction());
    regressor->addFunction(new ComputeDecisionProblemSuccessorState(decisionProblem));
    regressor->addFunction(new ComputeDecisionProblemSuccessorStateFunctor(decisionProblem));

    std::vector< std::pair<ExpressionPtr, double> > population;
    std::set<ExpressionPtr> processedNodes;

    ExpressionBuilderPtr nodeBuilder = biasedRandomSequentialNodeBuilder(populationSize / 4, complexity, 1e-12); // epsilon
    for (size_t i = 0; i < numIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i));
      context.resultCallback("iteration", i);
     
      context.enterScope(T("Generating candidates"));
      std::vector<ExpressionPtr> candidates;
      for (size_t trial = 0; trial < 100 && candidates.size() < populationSize; ++trial)
      {
        std::vector<ExpressionPtr> c;
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
        ExpressionPtr candidate = candidates[j];
        regressor->setRootNode(context, candidate);
        double score = objective->compute(context, regressor).getDouble();
        population.push_back(std::make_pair(candidate, score));
        context.progressCallback(new ProgressionState(j+1, candidates.size(), T("Candidates")));
        ++numEvaluated;
      }
      regressor->setRootNode(context, ExpressionPtr());
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

      regressor->setRootNode(context, population.front().first);
      objective->illustratePolicy(context, regressor);
      regressor->setRootNode(context, ExpressionPtr());

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

