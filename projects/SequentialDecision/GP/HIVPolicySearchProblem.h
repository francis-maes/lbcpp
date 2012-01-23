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

class FormulaBasedHIVPolicy : public Policy
{
public:
  FormulaBasedHIVPolicy(const GPExpressionPtr& formula)
    : formula(formula) {}

  virtual Variable selectAction(ExecutionContext& context, const DecisionProblemStatePtr& state)
  {
    ContainerPtr actions = state->getAvailableActions();
  
    double bestScore = -DBL_MAX;
    Variable bestAction;
    for (size_t j = 0; j < actions->getNumElements(); ++j)
    {
      Variable action = actions->getElement(j);
      HIVDecisionProblemStatePtr nextState = state->cloneAndCast<HIVDecisionProblemState>();
      double reward;
      nextState->performTransition(context, action, reward, NULL);
      std::vector<double> st;
      nextState->getState(st);
      st.push_back(reward);
      double score = formula->compute(&st[0]);
      if (score > bestScore)
        bestScore = score, bestAction = action;
    }
    return bestAction;
  }

  virtual String toShortString() const
    {return formula->toShortString();}

protected:
  //friend class FormulaBasedHIVPolicyClass;

  GPExpressionPtr formula;
};

class HIVPolicyFormulaObjective : public SimpleUnaryFunction
{
public:
  HIVPolicyFormulaObjective() : SimpleUnaryFunction(gpExpressionClass, regretScoreObjectClass), horizon(300) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    GPExpressionPtr formula = input.getObjectAndCast<GPExpression>();
    
    RandomGeneratorPtr random = context.getRandomGenerator();
  
    std::vector<double> initialState(6);
    initialState[0] = 163573;
    initialState[1] = 5;
    initialState[2] = 11945;
    initialState[3] = 46;
    initialState[4] = 63919;
    initialState[5] = 24;
    //for (size_t i = 0; i < initialState.size(); ++i)
    //  initialState[i] *= random->sampleDouble(0.8, 1.2);

    HIVDecisionProblemStatePtr state = new HIVDecisionProblemState(initialState);
    double cumulativeReward = 0.0;
    static const double discount = 0.98;
    for (size_t t = 0; t < horizon; ++t)
    {
      ContainerPtr actions = state->getAvailableActions();
      HIVDecisionProblemStatePtr nextStates[4];
      double rewards[4];
      double bestScore = -DBL_MAX;
      HIVDecisionProblemStatePtr nextState;
      double reward = 0.0;
      for (size_t j = 0; j < 4; ++j)
      {
        nextStates[j] = state->cloneAndCast<HIVDecisionProblemState>();
        Variable action = actions->getElement(j);
        nextStates[j]->performTransition(context, action, rewards[j], NULL);
        std::vector<double> st;
        nextStates[j]->getState(st);
        st.push_back(rewards[j]);
        double score = formula->compute(&st[0]);
        if (score > bestScore)
          bestScore = score, nextState = nextStates[j], reward = rewards[j];
      }
      if (!nextState)
        return 0.0;
      state = nextState;
      cumulativeReward += pow(discount, (double)t) * reward; 
    }

    return cumulativeReward > 1.0 ? log10(cumulativeReward) / 10.0 : 0.0;
  }

protected:
  friend class HIVPolicyFormulaObjectiveClass;

  size_t horizon;
};

typedef ReferenceCountedObjectPtr<HIVPolicyFormulaObjective> HIVPolicyFormulaObjectivePtr;

extern EnumerationPtr hivStateVariablesEnumeration;

class HIVPolicySearchProblem : public FormulaSearchProblem
{
public:
  HIVPolicySearchProblem() : objective(new HIVPolicyFormulaObjective()) {}
  
  virtual FunctionPtr getObjective() const
    {return objective;}

  virtual EnumerationPtr getVariables() const
    {return hivStateVariablesEnumeration;}

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
    RandomGeneratorPtr random = context.getRandomGenerator();
    res.resize(count);
    size_t index = 0;
    for (; index < count; ++index)
    {
      std::vector<double> input(7);
      input[0] = pow(10.0, random->sampleDouble(5.0, 6.0)); // T1
      input[1] = pow(10.0, random->sampleDouble(-1.0, 3.0)); // T2
      input[2] = pow(10.0, random->sampleDouble(0.0, 6.0)); // T1star
      input[3] = pow(10.0, random->sampleDouble(0.0, 2.0)); // T2star
      input[4] = pow(10.0, random->sampleDouble(0.0, 6.0)); // V
      input[5] = pow(10.0, random->sampleDouble(0.0, 6.0)); // E
      input[6] = pow(10.0, random->sampleDouble(0.0, 6.0)); // r

      for (size_t i = 0; i < input.size(); ++i)
        input[i] = pow(10.0, random->sampleDouble(0.0, 9.0));
      res[index] = input;
    }
  }

  struct ValueComparator
  {
    bool operator() (const std::pair<size_t, double>& left, const std::pair<size_t, double>& right) const
      {return (left.second != right.second ? left.second < right.second : left.first < right.first);}
  };

  virtual BinaryKeyPtr makeBinaryKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples) const
  {
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
  friend class HIVPolicySearchProblemClass;

  HIVPolicyFormulaObjectivePtr objective;
};


}; /* namespace lbcpp */

#endif // !LBCPP_GP_SEARCH_PROBLEM_HIV_POLICY_H_
