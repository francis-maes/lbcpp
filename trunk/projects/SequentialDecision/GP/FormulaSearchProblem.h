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

namespace lbcpp
{

class FormulaSearchProblem : public Object
{
public:
  virtual FunctionPtr getObjective() const = 0;

  virtual EnumerationPtr getVariables() const = 0;
  virtual void getOperators(std::vector<GPPre>& unaryOperators, std::vector<GPOperator>& binaryOperators) const = 0;

  virtual void sampleInputs(ExecutionContext& context, size_t count, std::vector< std::vector<double> >& res) const = 0;
  virtual bool makeFormulaKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples, std::vector<int>& res) const = 0;
};

typedef ReferenceCountedObjectPtr<FormulaSearchProblem> FormulaSearchProblemPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_GP_FORMULA_SEARCH_PROBLEM_H_
