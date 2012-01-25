/*-----------------------------------------.---------------------------------.
| Filename: HIVPolicySearchProblem.h       | HIV Policy search problem       |
| Author  : Francis Maes                   |                                 |
| Started : 19/01/2012 16:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GP_SEARCH_PROBLEM_HIV_POLICY_H_
# define LBCPP_GP_SEARCH_PROBLEM_HIV_POLICY_H_

# include "FormulaSearchProblem.h"
# include "../Problem/DamienDecisionProblem.h"
# include <lbcpp/DecisionProblem/Policy.h>

namespace lbcpp
{

class GPExpressionBasedPolicy : public Policy
{
public:
  GPExpressionBasedPolicy(const GPExpressionPtr& formula, bool useNextState)
    : formula(formula), useNextState(useNextState) {}
  GPExpressionBasedPolicy() : useNextState(false) {}

  virtual Variable selectAction(ExecutionContext& context, const DecisionProblemStatePtr& state)
  {
    ContainerPtr actions = state->getAvailableActions();
   
    double bestScore = -DBL_MAX;
    std::vector<Variable> bestActions;
    for (size_t j = 0; j < actions->getNumElements(); ++j)
    {
      std::vector<double> variables; // s .. a [.. r .. s']
      fillVariables(state, variables);
      Variable action = actions->getElement(j);
      fillVariables(action, variables);
      if (useNextState)
      {
        DecisionProblemStatePtr nextState = state->cloneAndCast<DecisionProblemState>();
        double reward;
        nextState->performTransition(context, action, reward, NULL);
        variables.push_back(reward);
        fillVariables(nextState, variables);
      }

      double score = formula->compute(&variables[0]);
      if (!isNumberValid(score))
        score = doubleMissingValue;
      if (score != doubleMissingValue && score >= bestScore)
      {
        if (score > bestScore)
        {
          bestScore = score;
          bestActions.clear();
        }
        bestActions.push_back(action);
      }
    }
    RandomGeneratorPtr random = context.getRandomGenerator();
    return bestActions.size()
      ? bestActions[random->sampleSize(bestActions.size())]
      : actions->getElement(random->sampleSize(actions->getNumElements()));
  }

  virtual String toShortString() const
    {return formula->toShortString();}

  static void fillVariables(const Variable& v, std::vector<double>& res)
  {
    jassert(!v.isMissingValue());
    if (v.isConvertibleToDouble())
      res.push_back(v.toDouble());
    else
    {
      jassert(v.isObject());
      ObjectPtr object = v.getObject();
      for (size_t i = 0; i < object->getNumVariables(); ++i)
        if (object->getVariableType(i)->isConvertibleToDouble())
          res.push_back(object->getVariable(i).toDouble());
    }
  }

protected:
  friend class GPExpressionBasedPolicyClass;

  GPExpressionPtr formula;
  bool useNextState;
};

class PolicyFormulaObjective : public SimpleUnaryFunction
{
public:
  PolicyFormulaObjective(DecisionProblemPtr problem, bool useNextState, size_t horizon)
    : SimpleUnaryFunction(gpExpressionClass, doubleType), problem(problem), useNextState(useNextState), horizon(horizon) {}
  PolicyFormulaObjective() : SimpleUnaryFunction(gpExpressionClass, doubleType), useNextState(false), horizon(0) {}

  double makeTrajectory(ExecutionContext& context, const GPExpressionPtr& formula, const DecisionProblemStatePtr& state) const
  {
    size_t horizon = getHorizon();
    double discount = getDiscount();

    PolicyPtr policy = new GPExpressionBasedPolicy(formula, useNextState);
    policy->startEpisode(context, problem, state);
    double res = 0.0;
    for (size_t t = 0; t < horizon; ++t)
    {
      Variable action = policy->selectAction(context, state);
      double reward;
      state->performTransition(context, action, reward);
      res += pow(discount, (double)t) * reward; 
      if (state->isFinalState())
        break;
    }
    return res;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    GPExpressionPtr formula = input.getObjectAndCast<GPExpression>();
    DecisionProblemStatePtr state = problem->sampleInitialState(context);
    double cumulativeReward = makeTrajectory(context, formula, state);
    double maxCumulativeReward = problem->getMaxCumulativeReward();
    jassert(maxCumulativeReward);
    if (maxCumulativeReward)
    {
      if (cumulativeReward > maxCumulativeReward)
        context.informationCallback(T("Wrong maxCumulativeReward: ") + String(cumulativeReward) + T(" should be less than ") + String(maxCumulativeReward));
      return cumulativeReward / maxCumulativeReward;
    }
    else
      return cumulativeReward;
  }

  const DecisionProblemPtr& getProblem() const
    {return problem;}

  bool getUseNextState() const
    {return useNextState;}

  size_t getHorizon() const
    {return horizon ? horizon : problem->getHorizon();}
  
  double getDiscount() const
    {return problem->getDiscount();}

protected:
  friend class PolicyFormulaObjectiveClass;

  DecisionProblemPtr problem;
  bool useNextState;
  size_t horizon;
};

typedef ReferenceCountedObjectPtr<PolicyFormulaObjective> PolicyFormulaObjectivePtr;

class PolicyFormulaSearchProblem : public FormulaSearchProblem
{
public:
  PolicyFormulaSearchProblem() : numStateVariables(0) {}

  DefaultEnumerationPtr makeVariablesEnumeration(ExecutionContext& context, const DecisionProblemPtr& problem, size_t& numStateVariables) const
  {
    DefaultEnumerationPtr res = new DefaultEnumeration(T("policyVariables"));
    ClassPtr stateClass = problem->getStateClass();
    TypePtr actionType = problem->getActionType();

    // s
    for (size_t i = 0; i < stateClass->getNumMemberVariables(); ++i)
      if (stateClass->getMemberVariableType(i)->isConvertibleToDouble())
        res->addElement(context, stateClass->getMemberVariableName(i));
    numStateVariables = res->getNumElements();

    // a
    if (actionType->isConvertibleToDouble())
      res->addElement(context, "action");
    else
    {
      for (size_t i = 0; i < actionType->getNumMemberVariables(); ++i)
        if (actionType->getMemberVariableType(i)->isConvertibleToDouble())
          res->addElement(context, actionType->getMemberVariableName(i));
    }
  
    if (objective->getUseNextState())
    {
      // r
      res->addElement(context, "reward");

      // s'
      for (size_t i = 0; i < stateClass->getNumMemberVariables(); ++i)
        if (stateClass->getMemberVariableType(i)->isConvertibleToDouble())
          res->addElement(context, T("next_") + stateClass->getMemberVariableName(i));
    }
    return res;
  }

  virtual FunctionPtr getObjective() const
    {return objective;}

  double makeVerboseTrajectory(ExecutionContext& context, const DecisionProblemStatePtr& initialState, const PolicyPtr& policy) const
  {   
    double discount = objective->getDiscount();
    size_t horizon = objective->getHorizon();
    double cumulativeReward = 0.0;
    DecisionProblemStatePtr state = initialState->cloneAndCast<DecisionProblemState>();

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
      Variable action = policy->selectAction(context, state);
      jassert(action.exists());
      double reward;
      state->performTransition(context, action, reward);
      cumulativeReward += pow(discount, (double)t) * reward; 
      
      if (action.isObject())
      {
        const ObjectPtr& actionObject = action.getObject();
        for (size_t j = 0; j < actionObject->getNumVariables(); ++j)
        {
          Variable v = actionObject->getVariable(j);
          if (v.isDouble())
            context.resultCallback(actionObject->getVariableName(j), v);
        }
      }
      else
        context.resultCallback(T("action"), action);

      context.resultCallback(T("reward"), reward);
      context.resultCallback(T("return"), cumulativeReward);
      context.leaveScope(cumulativeReward);
      
      if (state->isFinalState())
        break;
    }
    return cumulativeReward;
  }

  virtual double validateFormula(ExecutionContext& context, const GPExpressionPtr& formula, bool verbose) const
  {
    if (!validationInitialStates)
    {
      PolicyFormulaSearchProblem& pthis = *const_cast<PolicyFormulaSearchProblem* >(this);
      pthis.validationInitialStates = objective->getProblem()->getValidationInitialStates(pthis.numTrajectoriesToValidate);
    }

    if (verbose)
    {
      context.enterScope(T("Example Trajectory"));
      PolicyPtr policy = new GPExpressionBasedPolicy(formula, objective->getUseNextState());
      double cumulativeReward = makeVerboseTrajectory(context, objective->getProblem()->sampleInitialState(context), policy);
      context.leaveScope(cumulativeReward);
    }

    double res = 0.0;
    size_t n = validationInitialStates->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      DecisionProblemStatePtr state = validationInitialStates->getAndCast<DecisionProblemState>(i);
      for (size_t j = 0; j < numTrajectoriesToValidate; ++j)
        res += objective->makeTrajectory(context, formula, state->cloneAndCast<DecisionProblemState>());
      if (verbose)
        context.progressCallback(new ProgressionState(i + 1, n, T("Trajectories")));
    }
    return res / (n * numTrajectoriesToValidate);
  }

  virtual EnumerationPtr getVariables() const
  {
    if (!variables)
    {
      PolicyFormulaSearchProblem& pthis = *const_cast<PolicyFormulaSearchProblem* >(this);
      pthis.variables = makeVariablesEnumeration(defaultExecutionContext(), objective->getProblem(), pthis.numStateVariables);
    }
    return variables;
  }

  virtual void getOperators(std::vector<GPPre>& unaryOperators, std::vector<GPOperator>& binaryOperators) const
  {
    for (size_t i = gpOpposite; i <= gpAbs; ++i)
      if (i != gpExp)
        unaryOperators.push_back((GPPre)i);
    for (size_t i = gpAddition; i <= gpMax; ++i)
      binaryOperators.push_back((GPOperator)i);
  }

  virtual void sampleInputs(ExecutionContext& context, size_t count, std::vector< std::vector<double> >& res) const
  {
    EnumerationPtr variables = getVariables();
    RandomGeneratorPtr random = context.getRandomGenerator();
    res.resize(count);
    for (size_t index = 0; index < count; ++index)
    {
      std::vector<double> input;
        
      DecisionProblemStatePtr state = objective->getProblem()->sampleAnyState(context);
      ContainerPtr actions = state->getAvailableActions();
      jassert(actions->getNumElements());
      Variable action = actions->getElement(random->sampleSize(actions->getNumElements()));

      // s .. a
      GPExpressionBasedPolicy::fillVariables(state, input);
      GPExpressionBasedPolicy::fillVariables(action, input);
      if (objective->getUseNextState())
      {
        // r .. s'
        double reward;
        state->performTransition(context, action, reward);
        input.push_back(reward);
        GPExpressionBasedPolicy::fillVariables(state, input);
      }

      jassert(input.size() == variables->getNumElements());
      res[index] = input;
    }
  }

  struct ValueComparator
  {
    bool operator() (const std::pair<size_t, double>& left, const std::pair<size_t, double>& right) const
    {
      double l = isNumberValid(left.second) ? left.second : -DBL_MAX;
      double r = isNumberValid(right.second) ? right.second : -DBL_MAX;
      return (l != r ? l < r : left.first < right.first);
    }
  };

  virtual BinaryKeyPtr makeBinaryKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples) const
  {
    std::map<size_t, size_t> variableUseCounts;
    expression->getVariableUseCounts(variableUseCounts);
    if (variableUseCounts.size() && variableUseCounts.rbegin()->first < numStateVariables)
      return BinaryKeyPtr(); // reject policies that only depend on the current state

    std::vector< std::pair<size_t, double> > scores(inputSamples.size());
    for (size_t i = 0; i < scores.size(); ++i)
    {
      scores[i].first = i;
      scores[i].second = expression->compute(&inputSamples[i][0]);
    }
    std::sort(scores.begin(), scores.end(), ValueComparator());

    BinaryKeyPtr res = new BinaryKey(scores.size() * 4);
    for (size_t i = 0; i < scores.size(); ++i)
      res->push32BitInteger(scores[i].first);
    return res;
  }

protected:
  friend class PolicyFormulaSearchProblemClass;

  PolicyFormulaObjectivePtr objective;

  EnumerationPtr variables;
  size_t numStateVariables;
  ObjectVectorPtr validationInitialStates;
  size_t numTrajectoriesToValidate;
};


}; /* namespace lbcpp */

#endif // !LBCPP_GP_SEARCH_PROBLEM_HIV_POLICY_H_
