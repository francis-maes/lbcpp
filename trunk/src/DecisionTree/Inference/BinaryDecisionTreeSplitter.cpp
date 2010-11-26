/*-----------------------------------------.---------------------------------.
 | Filename: BinaryDecisionTreeSplitter.cpp| BinaryDecisionTreeSplitter      |
 | Author  : Julien Becker                 |                                 |
 | Started : 25/11/2010 14:00              |                                 |
 `------------------------------------------/                                |
                                |                                            |
                                `-------------------------------------------*/

# include "BinaryDecisionTreeSplitter.h"

using namespace lbcpp;

static Variable getInputVariableFromExample(const Variable& example, size_t variableIndex)
{
  return example[0].getObject()->getVariable(variableIndex);
}

double BinaryDecisionTreeSplitter::computeSplitScore(ExecutionContext& context, 
                                                     ContainerPtr data, ContainerPtr& positiveExamples, ContainerPtr& negativeExamples,
                                                     PredicatePtr predicate) const
{
  VectorPtr left = vector(data->getElementsType());
  VectorPtr right = vector(data->getElementsType());
  
  for (size_t i = 0; i < data->getNumElements(); ++i)
  {
    Variable example = data->getElement(i);
    if (predicate->computePredicate(context, getInputVariableFromExample(example, variableIndex)))
      left->append(example);
    else
      right->append(example);
  }

  positiveExamples = left;
  negativeExamples = right;
  
  return scoringFunction->compute(context, Variable::pair(left, right));
}

Variable DoubleBinaryDecisionTreeSplitter::sampleSplit(ContainerPtr data) const
{
  double minValue = DBL_MAX, maxValue = -DBL_MAX;
  for (size_t i = 0; i < data->getNumElements(); ++i)
  {
    Variable variable = getInputVariableFromExample(data->getElement(i), variableIndex);
    if (variable.isNil())
      continue;
    double value = variable.getDouble();
    if (value < minValue)
      minValue = value;
    if (value > maxValue)
      maxValue = value;
  }
  jassert(minValue != DBL_MAX && maxValue != -DBL_MAX);
  double res = random->sampleDouble(minValue, maxValue);
  jassert(res >= minValue && res < maxValue);
  return Variable(res, doubleType);
}

Variable IntegereBinaryDecisionTreeSplitter::sampleSplit(ContainerPtr data) const
{
  int minValue = 0x7FFFFFFF;
  int maxValue = -minValue;
  size_t n = data->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    Variable variable = getInputVariableFromExample(data->getElement(i), variableIndex);
    if (!variable.exists())
      continue;
    int value = variable.getInteger();
    if (value < minValue)
      minValue = value;
    if (value > maxValue)
      maxValue = value;
  }
  jassert(minValue != 0x7FFFFFFF && maxValue != -0x7FFFFFFF);
  jassert(maxValue > minValue);
  return random->sampleInt(minValue, maxValue);
}

Variable EnumerationBinaryDecisionTreeSplitter::sampleSplit(ContainerPtr data) const
{
  EnumerationPtr enumeration = data->getElementsType()->getTemplateArgument(1);
  size_t n = enumeration->getNumElements();
  
  // enumerate possible values
  std::set<size_t> possibleValues;
  for (size_t i = 0; i < data->getNumElements(); ++i)
  {
    Variable value = getInputVariableFromExample(data->getElement(i), variableIndex);
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
  random->sampleSubset(possibleValuesVector, possibleValues.size() / 2, selectedValues);
  
  // create mask
  BooleanVectorPtr mask = new BooleanVector(n + 1);
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
    if (value.isMissingValue())
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
