/*-----------------------------------------.---------------------------------.
| Filename: SearchHeuristic.h              | Search Heuristic                |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 16:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_HEURISTIC_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_SEARCH_HEURISTIC_H_

# include "SearchTree.h"
# include <lbcpp/Learning/Numerical.h>

namespace lbcpp
{

class GreedySearchHeuristic : public SimpleSearchHeuristic
{
public:
  GreedySearchHeuristic(double discount = 1.0)
    : discount(discount) {}

  virtual double computeHeuristic(const SearchTreeNodePtr& node) const
    {return node->getReward() * pow(discount, (double)node->getDepth() - 1.0);}

  virtual String toShortString() const
    {return T("greedy(") + String(discount) + T(")");}

protected:
  friend class GreedySearchHeuristicClass;

  double discount;
};

class MaxReturnSearchHeuristic : public SimpleSearchHeuristic
{
public:
  virtual double computeHeuristic(const SearchTreeNodePtr& node) const
    {return node->getCurrentReturn();}
};

class MinDepthSearchHeuristic : public SimpleSearchHeuristic
{
public:
  virtual double computeHeuristic(const SearchTreeNodePtr& node) const
    {return -(double)node->getDepth();}
};

class OptimisticPlanningSearchHeuristic : public SimpleSearchHeuristic
{
public:
  OptimisticPlanningSearchHeuristic(double discount = 0.9, double maxReward = 1.0)
      : discount(discount), maxReward(maxReward)
    {jassert(discount > 0.0 && discount < 1.0 && maxReward != 0.0);}

  virtual String toShortString() const
    {return T("optimistic(") + String(discount) + T(", ") + String(maxReward) + T(")");}

  virtual double computeHeuristic(const SearchTreeNodePtr& node) const
    {return node->getCurrentReturn() / maxReward + pow(discount, (double)node->getDepth()) / (1.0 - discount);}

private:
  friend class OptimisticPlanningSearchHeuristicClass;

  double discount;
  double maxReward;
};

class LinearInterpolatedSearchHeuristic : public SimpleSearchHeuristic
{
public:
  LinearInterpolatedSearchHeuristic(FunctionPtr heuristic1, FunctionPtr heuristic2, double k)
    : heuristic1(heuristic1), heuristic2(heuristic2), k(k) {}
  LinearInterpolatedSearchHeuristic() : k(0.0) {}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    if (!heuristic1->initialize(context, (TypePtr)searchTreeNodeClass) ||
        !heuristic2->initialize(context, (TypePtr)searchTreeNodeClass))
      return TypePtr();
    return SimpleSearchHeuristic::initializeFunction(context, inputVariables, outputName, outputShortName);
  }

  virtual double computeHeuristic(const SearchTreeNodePtr& node) const
  {
    double h1 = heuristic1->compute(defaultExecutionContext(), node).getDouble();
    double h2 = heuristic2->compute(defaultExecutionContext(), node).getDouble();
    return h1 + k * (h2 - h1);
  }

protected:
  friend class LinearInterpolatedSearchHeuristicClass;

  FunctionPtr heuristic1;
  FunctionPtr heuristic2;
  double k;
};

// SearchNode -> Scalar
class LearnableSearchHeuristic : public CompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t node = builder.addInput(searchTreeNodeClass, T("node"));
    size_t perception = builder.addFunction(createPerceptionFunction(), node);
    size_t supervision = builder.addConstant(Variable());
    builder.addFunction(createScoringFunction(), perception, supervision);
  }

  const FunctionPtr& getPerceptionFunction() const
    {return functions[0];}

  const FunctionPtr& getScoringFunction() const
    {return functions[1];}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    Variable res = CompositeFunction::computeFunction(context, inputs);
    return res.exists() ? res.getDouble() : 0.0;
  }

protected:
  virtual FunctionPtr createPerceptionFunction() const = 0; // SearchNode -> Features
  virtual FunctionPtr createScoringFunction() const = 0;    // Features -> Score
};

typedef ReferenceCountedObjectPtr<LearnableSearchHeuristic> LearnableSearchHeuristicPtr;


}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
