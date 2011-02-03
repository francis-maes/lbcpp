/*-----------------------------------------.---------------------------------.
 | Filename: BinaryDecisionTreeSplitter.cpp| BinaryDecisionTreeSplitter      |
 | Author  : Julien Becker                 |                                 |
 | Started : 25/11/2010 14:00              |                                 |
 `------------------------------------------/                                |
                                |                                            |
                                `-------------------------------------------*/
#include "BinaryDecisionTreeSplitter.h"
using namespace lbcpp;

double BinaryDecisionTreeSplitter::computeSplitScore(ExecutionContext& context, 
                                                     const DecisionTreeExampleVector& examples,
                                                     std::vector<size_t>& leftExamples,
                                                     std::vector<size_t>& rightExamples,
                                                     PredicatePtr predicate) const
{
  size_t n = examples.getNumExamples();
  leftExamples.reserve(n - 1);
  rightExamples.reserve(n - 1);
  const std::vector<size_t>& indices = examples.getIndices();
  for (size_t i = 0; i < n; ++i)
  {
    if (predicate->computePredicate(context, examples.getAttribute(i, variableIndex)))
      rightExamples.push_back(indices[i]);
    else
      leftExamples.push_back(indices[i]);
  }

  if (!leftExamples.size() || !rightExamples.size() || leftExamples.size() + rightExamples.size() != examples.getNumExamples())
  {
    String attributeValues;
    for (size_t i = 0; i < examples.getNumExamples(); ++i)
      attributeValues += examples.getAttribute(i, variableIndex).toShortString() + T(" ");
    context.errorCallback(T("Invalid split: ") + predicate->toString() + T(" ") + String((int)examples.getNumExamples())
      + T(" examples -> ") + String((int)leftExamples.size()) + T(" ") + String((int)rightExamples.size())
      + T("\nAttribute Values: ") + attributeValues);
    jassert(false);
    return -DBL_MAX;
  }

  return scoringFunction->compute(context, new SplitScoringInput(examples, leftExamples, rightExamples));
}

Variable DoubleBinaryDecisionTreeSplitter::sampleSplit(const DecisionTreeExampleVector& examples) const
{
  double minValue = DBL_MAX, maxValue = -DBL_MAX;
  size_t n = examples.getNumExamples();
  for (size_t i = 0; i < n; ++i)
  {
    const Variable& variable = examples.getAttribute(i, variableIndex);
    if (variable.exists())
    {
      double value = variable.getDouble();
      if (value < minValue)
        minValue = value;
      if (value > maxValue)
        maxValue = value;
    }
  }
  jassert(minValue != DBL_MAX && maxValue != -DBL_MAX);
  double res = random->sampleDouble(minValue, maxValue);
  jassert(res >= minValue && res < maxValue);// || fabs(minValue - maxValue) < 1e-5);
  return Variable(res, doubleType);
}

Variable IntegerBinaryDecisionTreeSplitter::sampleSplit(const DecisionTreeExampleVector& examples) const
{
  int minValue = 0x7FFFFFFF;
  int maxValue = -minValue;
  size_t n = examples.getNumExamples();
  for (size_t i = 0; i < n; ++i)
  {
    const Variable& variable = examples.getAttribute(i, variableIndex);
    if (variable.exists())
    {
      int value = variable.getInteger();
      if (value < minValue)
        minValue = value;
      if (value > maxValue)
        maxValue = value;
    }
  }
  jassert(minValue != 0x7FFFFFFF && maxValue != -0x7FFFFFFF);
  jassert(maxValue > minValue);
  return random->sampleInt(minValue, maxValue);
}

Variable EnumerationBinaryDecisionTreeSplitter::sampleSplit(const DecisionTreeExampleVector& examples) const
{
  EnumerationPtr enumeration = examples.getAttribute(0, variableIndex).getType().staticCast<Enumeration>();
  size_t numElements = enumeration->getNumElements();
  
  // enumerate possible values
  std::set<size_t> possibleValues;
  size_t n = examples.getNumExamples();
  for (size_t i = 0; i < n; ++i)
  {
    const Variable& value = examples.getAttribute(i, variableIndex);
    possibleValues.insert((size_t)value.getInteger());
  }
  jassert(possibleValues.size() >= 2);
  
  // convert from std::set to std::vector
  std::vector<size_t> possibleValuesVector;
  possibleValuesVector.reserve(possibleValues.size());
  for (std::set<size_t>::const_iterator it = possibleValues.begin(); it != possibleValues.end(); ++it)
    possibleValuesVector.push_back(*it);
  
  // sample selected values
  std::set<size_t> selectedValues;
  random->sampleSubset(possibleValuesVector, possibleValuesVector.size() / 2, selectedValues);
  
  // create mask
  BooleanVectorPtr mask = new BooleanVector(numElements + 1);
  for (size_t i = 0; i < mask->getNumElements(); ++i)
  {
    bool bitValue;
    if (possibleValues.find(i) == possibleValues.end())
      bitValue = random->sampleBool(); // 50% probability for values that do not appear in the training data
    else
      bitValue = (selectedValues.find(i) != selectedValues.end()); // true for selected values
    mask->set(i, bitValue);
  }

  return mask;
}

class BelongsToMaskPredicate : public Predicate
{
public:
  BelongsToMaskPredicate(BooleanVectorPtr mask)
    : mask(mask) {}
  
  virtual String toString() const
    {return T("BelongsToMask(") + mask->toString() + T(")");}
  
  virtual TypePtr getInputType() const
    {return integerType;}
  
  virtual bool computePredicate(ExecutionContext& context, const Variable& value) const
  {
    if (value.isMissingValue()) // FIXME: can be removed ?
      return mask->get(mask->getNumElements() - 1);
    size_t i = (size_t)value.getInteger();
    jassert(i < mask->getNumElements() - 1);
    return mask->get(i);
  }
  
private:
  BooleanVectorPtr mask;
};

PredicatePtr EnumerationBinaryDecisionTreeSplitter::getSplitPredicate(const Variable& splitArgument) const
  {return new BelongsToMaskPredicate(splitArgument.dynamicCast<BooleanVector>());}
