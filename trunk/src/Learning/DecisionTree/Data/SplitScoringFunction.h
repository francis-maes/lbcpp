/*-----------------------------------------.---------------------------------.
| Filename: SplitScoringFunction.h         | Split Scoring Function          |
| Author  : Julien Becker                  |                                 |
| Started : 25/11/2010 13:19               |                                 |
`------------------------------------------/                                 |
                                |                                            |
                                `-------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_SPLIT_SCORING_FUNCTION_H_
# define LBCPP_DECISION_TREE_SPLIT_SCORING_FUNCTION_H_

//# include <lbcpp/Optimizer/ObjectiveFunction.h>   //TODO arnaud : ObjectiveFunction -> Function : not tested
# include <lbcpp/Core/Function.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>
# include <lbcpp/Distribution/DistributionBuilder.h>
# include "DecisionTreeExampleVector.h"

namespace lbcpp
{

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

class SplitScoringFunction : public Function
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
