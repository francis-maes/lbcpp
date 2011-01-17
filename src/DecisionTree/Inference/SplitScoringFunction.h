/*-----------------------------------------.---------------------------------.
 | Filename: SplitScoringFunction.h        | SplitScoringFunction            |
 | Author  : Julien Becker                 |                                 |
 | Started : 25/11/2010 13:19              |                                 |
 `------------------------------------------/                                |
                                |                                            |
                                `-------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_SPLIT_SCORING_FUNCTION_H_
# define LBCPP_DECISION_TREE_SPLIT_SCORING_FUNCTION_H_

# include <lbcpp/lbcpp.h>
# include <lbcpp/Distribution/DistributionBuilder.h>

namespace lbcpp
{

class DecisionTreeExampleVector
{
public:
  DecisionTreeExampleVector(std::vector< std::vector<Variable> >& attributes,
                            std::vector<Variable>& labels,
                            const std::vector<size_t>& indices = std::vector<size_t>())
    : attributes(attributes), labels(labels), indices(indices) {}

  DecisionTreeExampleVector subset(const std::vector<size_t>& newIndices) const
    {return DecisionTreeExampleVector(attributes, labels, newIndices);}

  size_t getNumExamples() const
    {return indices.size();}

  const Variable& getLabel(size_t index) const
    {jassert(index < indices.size()); return labels[indices[index]];}

  const Variable& getAttribute(size_t exampleIndex, size_t variableIndex) const
    {jassert(exampleIndex < indices.size()); return attributes[indices[exampleIndex]][variableIndex];}

  bool isAttributeConstant(size_t variableIndex) const
  {
    size_t n = indices.size();
    Variable constantValue;
    bool isConstantValueSet = false;
    for (size_t i = 0; i < n; ++i)
    {
      const Variable& value = getAttribute(i, variableIndex);
      jassert(!value.isNil());
      if (value.isMissingValue())
        continue;
      if (!isConstantValueSet)
      {
        constantValue = value;
        isConstantValueSet = true;
      }
      else if (constantValue != value)
        return false;
    }
    return true;
  }

  bool isLabelConstant(Variable& constantValue) const
  {
    size_t n = indices.size();
    constantValue = getLabel(0);
    jassert(constantValue.exists());
    for (size_t i = 1; i < n; ++i)
    {
      jassert(getLabel(i).exists());
      if (constantValue != getLabel(i))
        return false;
    }
    return true;
  }

  const std::vector<size_t>& getIndices() const
    {return indices;}

private:
  std::vector< std::vector<Variable> >& attributes;
  std::vector<Variable>& labels;
  std::vector<size_t> indices;
};

class SplitScoringInput : public Object
{
public:
  SplitScoringInput(const DecisionTreeExampleVector& examples,
                    std::vector<size_t>& leftIndices,
                    std::vector<size_t>& rightIndices)
      : examples(examples), leftIndices(leftIndices), rightIndices(rightIndices) {}

  SplitScoringInput() : examples(*(const DecisionTreeExampleVector* )0), 
                        leftIndices(*(std::vector<size_t>* )0),
                        rightIndices(*(std::vector<size_t>* )0) {}


  DecisionTreeExampleVector getLeftExamples() const
    {return examples.subset(leftIndices);}

  DecisionTreeExampleVector getRightExamples() const
    {return examples.subset(rightIndices);}

  const DecisionTreeExampleVector& examples;
  std::vector<size_t>& leftIndices;
  std::vector<size_t>& rightIndices;
};

typedef ReferenceCountedObjectPtr<SplitScoringInput> SplitScoringInputPtr;
extern ClassPtr splitScoringInputClass;

class SplitScoringFunction : public ObjectiveFunction
{
public:
  SplitScoringFunction(TypePtr labelsType)
    : labelsType(labelsType) {}

  virtual TypePtr getInputType() const
    {return splitScoringInputClass;}

protected:
  TypePtr labelsType;
};

class RegressionIGSplitScoringFunction : public SplitScoringFunction
{
public:
  RegressionIGSplitScoringFunction() : SplitScoringFunction(doubleType) {}

  virtual double compute(ExecutionContext& context, const Variable& input) const;

protected:
  double getLeastSquareDeviation(const DecisionTreeExampleVector& examples) const;
};
  
class ClassificationIGSplitScoringFunction : public SplitScoringFunction
{
public:
  ClassificationIGSplitScoringFunction() : SplitScoringFunction(enumerationClass) {}

  virtual double compute(ExecutionContext& context, const Variable& input) const;

protected:
  EnumerationDistributionPtr getDiscreteOutputDistribution(ExecutionContext& context, const DecisionTreeExampleVector& examples) const;
  
private:
  DistributionBuilderPtr cacheBuilder;
  DistributionBuilderPtr createProbabilityBuilder(EnumerationPtr enumeration) const;
};

class BinaryIGSplitScoringFunction : public SplitScoringFunction
{
public:
  BinaryIGSplitScoringFunction() : SplitScoringFunction(booleanType) {}
  
  virtual double compute(ExecutionContext& context, const Variable& input) const;

protected:
  BernoulliDistributionPtr getProbabilityDistribution(const DecisionTreeExampleVector& examples) const;
};

typedef ReferenceCountedObjectPtr<SplitScoringFunction> SplitScoringFunctionPtr;

};

#endif //!LBCPP_DECISION_TREE_SPLIT_SCORING_FUNCTION_H_
