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

class SplitScoringFunction : public ObjectiveFunction
{
public:
  SplitScoringFunction(TypePtr elementType = anyType)
    : elementType(elementType), pairInputType(pairClass(containerClass(elementType), containerClass(elementType))) {}

  virtual TypePtr getInputType() const
    {return pairInputType;}

protected:
  TypePtr elementType;

private:
  TypePtr pairInputType; // avoid re-calculation
};

class RegressionIGSplitScoringFunction : public SplitScoringFunction
{
public:
  RegressionIGSplitScoringFunction() : SplitScoringFunction(pairClass(anyType, doubleType)) {}

  virtual double compute(ExecutionContext& context, const Variable& input) const;

protected:
  double getLeastSquareDeviation(ContainerPtr data) const;
};
  
class ClassificationIGSplitScoringFunction : public SplitScoringFunction
{
public:
  ClassificationIGSplitScoringFunction() : SplitScoringFunction(pairClass(anyType, enumerationClass)) {}

  virtual double compute(ExecutionContext& context, const Variable& input) const;

protected:
  EnumerationDistributionPtr getDiscreteOutputDistribution(ExecutionContext& context, ContainerPtr data) const;
  
private:
  DistributionBuilderPtr cacheBuilder;
  DistributionBuilderPtr createProbabilityBuilder(EnumerationPtr enumeration) const;
};

typedef ReferenceCountedObjectPtr<SplitScoringFunction> SplitScoringFunctionPtr;

};

#endif //!LBCPP_DECISION_TREE_SPLIT_SCORING_FUNCTION_H_
