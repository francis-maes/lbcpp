/*-----------------------------------------.---------------------------------.
| Filename: SmallMDPFormulaSearchProblem.h | SmallMDP Formula Objective      |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2011 11:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GP_SEARCH_PROBLEM_SMALL_MDP_FORMULA_H_
# define LBCPP_GP_SEARCH_PROBLEM_SMALL_MDP_FORMULA_H_

# include "FormulaSearchProblem.h"
# include "../SmallMDP/ParameterizedSmallMDPPolicy.h"
# include "../SmallMDP/SmallMDPSandBox.h"
# include <algorithm>

namespace lbcpp
{

class SmallMDPFormulaObjective : public SimpleUnaryFunction
{
public:
  SmallMDPFormulaObjective(const SamplerPtr& mdpSampler = SamplerPtr(), bool useModel = false)
    : SimpleUnaryFunction(gpExpressionClass, doubleType), mdpSampler(mdpSampler), useModel(useModel) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    GPExpressionPtr formula = input.getObjectAndCast<GPExpression>();
    SmallMDPPolicyPtr policy = new GPExpressionSmallMDPPolicy(formula, useModel);
    SmallMDPPtr mdp = mdpSampler->sample(context, context.getRandomGenerator()).getObjectAndCast<SmallMDP>();
    WorkUnitPtr workUnit = new EvaluateSmallMDPPolicy(policy, mdp);
    return context.run(workUnit, false).getDouble() / 273.0;
  }
 
protected:
  friend class SmallMDPFormulaObjectiveClass;

  SamplerPtr mdpSampler;
  bool useModel;
};

typedef ReferenceCountedObjectPtr<SmallMDPFormulaObjective> SmallMDPFormulaObjectivePtr;

extern EnumerationPtr smallMDPFormulaVariablesEnumeration;

class SmallMDPFormulaSearchProblem : public FormulaSearchProblem
{
public:
  virtual FunctionPtr getObjective() const
    {return objective;}

  virtual EnumerationPtr getVariables() const
    {return smallMDPFormulaVariablesEnumeration;}

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
    for (size_t i = 0; i < res.size(); ++i)
    {
      std::vector<double> input(4);
      input[0] = juce::jlimit(0.0, 100.0, random->sampleDouble(-1, 101));
      input[1] = juce::jlimit(0.0, 1.0, random->sampleDouble(-0.1, 1.1));
      input[2] = juce::jlimit(0.0, 100.0, random->sampleDouble(-1, 101));
      input[3] = juce::jlimit(1.0, 100.0, random->sampleDouble(0, 101));
      res[i] = input;
    }
  }

  virtual BinaryKeyPtr makeBinaryKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples) const
  {
    std::map<size_t, size_t> variableUseCounts;
    expression->getVariableUseCounts(variableUseCounts);
    if (variableUseCounts[1] == 0 || variableUseCounts[2] == 0) // at least the reward and the next state value must be used
      return BinaryKeyPtr();

    BinaryKeyPtr res = new BinaryKey(inputSamples.size() * sizeof (juce::int64));
    for (size_t i = 0; i < inputSamples.size(); ++i)
    {
      double value = expression->compute(&inputSamples[i][0]);
      if (!isNumberValid(value))
        return BinaryKeyPtr();
      res->pushInteger((juce::int64)(value * 100000));
    }
    return res;
  }

protected:
  friend class SmallMDPFormulaSearchProblemClass;

  FunctionPtr objective;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GP_SEARCH_PROBLEM_SMALL_MDP_FORMULA_H_
