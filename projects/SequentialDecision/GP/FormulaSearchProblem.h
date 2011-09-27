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

namespace lbcpp
{

class FormulaKey : public Object
{
public:
  FormulaKey(size_t size)
    : values(size, 0) {}
  FormulaKey() {}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const std::vector<int>& ovalues = otherObject.staticCast<FormulaKey>()->values;
    if (ovalues == values)
      return 0;
    else
      return values < ovalues ? -1 : 1;
  }

  void setValue(size_t index, int value)
    {jassert(index < values.size()); values[index] = value;}

protected:
  std::vector<int> values;
};
typedef ReferenceCountedObjectPtr<FormulaKey> FormulaKeyPtr;

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
  virtual FormulaKeyPtr makeFormulaKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples) const = 0;
};

typedef ReferenceCountedObjectPtr<FormulaSearchProblem> FormulaSearchProblemPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_GP_FORMULA_SEARCH_PROBLEM_H_
