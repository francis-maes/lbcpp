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
    : values(size, 0), position(0) {}
  FormulaKey() {}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const std::vector<unsigned char>& ovalues = otherObject.staticCast<FormulaKey>()->values;
    if (ovalues == values)
      return 0;
    else
      return values < ovalues ? -1 : 1;
  }

  void pushByte(unsigned char c)
    {jassert(position < values.size()); values[position++] = c;}

  void pushInteger(juce::int64 value)
  {
    memcpy(&values[position], &value, sizeof (juce::int64));
    position += sizeof (juce::int64);
  }

  size_t computeHashValue() const
  {
    size_t res = 0;
    const unsigned char* ptr = &values[0];
    const unsigned char* lim = ptr + values.size();
    while (ptr < lim)
      res = 31 * res + *ptr++;
    return res;
  }

protected:
  std::vector<unsigned char> values;
  size_t position;
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
