/*-----------------------------------------.---------------------------------.
| Filename: ExpressionBuilder.h            | Node Builder base classes       |
| Author  : Francis Maes                   |                                 |
| Started : 03/01/2012 17:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_H_
# define LBCPP_LUAPE_NODE_BUILDER_H_

# include "predeclarations.h"
# include <lbcpp-ml/Expression.h>
# include <lbcpp-ml/ExpressionUniverse.h>
# include <lbcpp-ml/ExpressionRPN.h>
# include <lbcpp/DecisionProblem/Policy.h>
# include "LuapeInference.h"

namespace lbcpp
{

class ExpressionBuilder : public Object
{
public:
  virtual void buildNodes(ExecutionContext& context, const LuapeInferencePtr& domain, size_t maxCount, std::vector<ExpressionPtr>& res) = 0;
};

typedef ReferenceCountedObjectPtr<ExpressionBuilder> ExpressionBuilderPtr;

extern ExpressionBuilderPtr inputsNodeBuilder();
extern ExpressionBuilderPtr singletonNodeBuilder(const ExpressionPtr& node);
extern ExpressionBuilderPtr compositeNodeBuilder(const std::vector<ExpressionBuilderPtr>& builders);
extern ExpressionBuilderPtr compositeNodeBuilder(ExpressionBuilderPtr builder1, ExpressionBuilderPtr builder2);
extern ExpressionBuilderPtr exhaustiveSequentialNodeBuilder(size_t complexity);

class StochasticNodeBuilder : public ExpressionBuilder
{
public:
  StochasticNodeBuilder(size_t numNodes = 0);

  virtual ExpressionPtr sampleNode(ExecutionContext& context, const ExpressionDomainPtr& domain) = 0;

  virtual void buildNodes(ExecutionContext& context, const LuapeInferencePtr& domain, size_t maxCount, std::vector<ExpressionPtr>& res);

protected:
  friend class StochasticNodeBuilderClass;

  size_t numNodes;
};

typedef ReferenceCountedObjectPtr<StochasticNodeBuilder> StochasticNodeBuilderPtr;

extern PolicyPtr treeBasedRandomPolicy();
extern StochasticNodeBuilderPtr policyBasedNodeBuilder(const PolicyPtr& policy, size_t numNodes, size_t complexity);
extern StochasticNodeBuilderPtr adaptativeSamplingNodeBuilder(size_t numNodes, size_t complexity, bool useVariableRelevancies, bool useExtendedVariables);
extern StochasticNodeBuilderPtr randomSequentialNodeBuilder(size_t numNodes, size_t complexity);
extern StochasticNodeBuilderPtr biasedRandomSequentialNodeBuilder(size_t numNodes, size_t complexity, double initialImportance);

class SequentialNodeBuilder : public StochasticNodeBuilder
{
public:
  SequentialNodeBuilder(size_t numNodes, size_t complexity);
  SequentialNodeBuilder() {}

  virtual bool sampleAction(ExecutionContext& context, const ExpressionDomainPtr& problem, ExpressionRPNTypeStatePtr typeState, ObjectPtr& res) const = 0;

  virtual ExpressionPtr sampleNode(ExecutionContext& context, const ExpressionDomainPtr& problem);

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class SequentialNodeBuilderClass;

  virtual void samplingDone(ExecutionContext& context, size_t numSamplingFailures, size_t numFailuresAllowed) {}

  size_t complexity;

  ExpressionUniversePtr universe;
  ExpressionRPNTypeSpacePtr typeSearchSpace;

  static bool isActionAvailable(ObjectPtr action, const std::vector<ExpressionPtr>& stack);
  ExpressionRPNTypeStatePtr getTypeState(size_t stepNumber, const std::vector<ExpressionPtr>& stack) const;
  void executeAction(std::vector<ExpressionPtr>& stack, const ObjectPtr& action) const;
};


}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_H_
