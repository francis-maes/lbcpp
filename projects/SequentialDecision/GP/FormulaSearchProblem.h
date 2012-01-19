/*-----------------------------------------.---------------------------------.
| Filename: FormulaSearchProblem.h         | Formula Search Problem          |
| Author  : Francis Maes                   |                                 |
| Started : 25/09/2011 21:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_GP_FORMULA_SEARCH_PROBLEM_H_
# define LBCPP_SEQUENTIAL_DECISION_GP_FORMULA_SEARCH_PROBLEM_H_

# include "GPExpression.h"
# include "GPExpressionBuilder.h"
# include <lbcpp/Data/BinaryKey.h>

namespace lbcpp
{

class RegretScoreObject : public ScoreObject
{
public:
  RegretScoreObject(double regret = 1.0, double referenceRegret = 1.0)
    : regret(regret), reward(juce::jlimit(0.0, 1.0, (referenceRegret - regret) / referenceRegret)) {}

  virtual double getScoreToMinimize() const
    {return regret;}

  double getRegret() const
    {return regret;}

  double getReward() const
    {return reward;}

protected:
  friend class RegretScoreObjectClass;

  double regret;
  double reward;
};

typedef ReferenceCountedObjectPtr<RegretScoreObject> RegretScoreObjectPtr;
extern ClassPtr regretScoreObjectClass;

class FormulaSearchProblem : public Object
{
public:
  virtual FunctionPtr getObjective() const = 0;

  virtual EnumerationPtr getVariables() const = 0;
  virtual void getOperators(std::vector<GPPre>& unaryOperators, std::vector<GPOperator>& binaryOperators) const = 0;

  GPExpressionBuilderStatePtr makeGPBuilderState(size_t maxSize) const
  {
    std::vector<GPPre> unaryOperators;
    std::vector<GPOperator> binaryOperators;
    getOperators(unaryOperators, binaryOperators);
    return new RPNGPExpressionBuilderState("Coucou", getVariables(), FunctionPtr(), maxSize, unaryOperators, binaryOperators);
  }
  
  virtual void sampleInputs(ExecutionContext& context, size_t count, std::vector< std::vector<double> >& res) const = 0;
  virtual BinaryKeyPtr makeBinaryKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples) const = 0;
};

typedef ReferenceCountedObjectPtr<FormulaSearchProblem> FormulaSearchProblemPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_GP_FORMULA_SEARCH_PROBLEM_H_
