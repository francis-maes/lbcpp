/*-----------------------------------------.---------------------------------.
| Filename: LuapeNodeBuilder.h             | Node Builder base classes       |
| Author  : Francis Maes                   |                                 |
| Started : 03/01/2012 17:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_H_
# define LBCPP_LUAPE_NODE_BUILDER_H_

# include "LuapeNode.h"
# include "LuapeUniverse.h"
# include <lbcpp/DecisionProblem/Policy.h>

namespace lbcpp
{

class LuapeRPNSequence;
typedef ReferenceCountedObjectPtr<LuapeRPNSequence> LuapeRPNSequencePtr;

class LuapeRPNSequence : public Object
{
public:
  LuapeRPNSequence(const std::vector<ObjectPtr>& sequence);
  LuapeRPNSequence() {}

  static LuapeRPNSequencePtr fromNode(const LuapeNodePtr& node);

  void appendNode(const LuapeNodePtr& node);
  void append(const ObjectPtr& action)
    {sequence.push_back(action);}

  size_t getLength() const
    {return sequence.size();}

  const ObjectPtr& getElement(size_t index) const
    {jassert(index < sequence.size()); return sequence[index];}

  bool startsWith(const LuapeRPNSequencePtr& start) const;

  virtual String toShortString() const;

  std::vector<TypePtr> computeTypeState(const std::vector<TypePtr>& initialState = std::vector<TypePtr>()) const;

private:
  friend class LuapeRPNSequenceClass;

  std::vector<ObjectPtr> sequence;
};

class LuapeNodeBuilder : public Object
{
public:
  virtual void buildNodes(ExecutionContext& context, const LuapeInferencePtr& function, size_t maxCount, std::vector<LuapeNodePtr>& res) = 0;
};

typedef ReferenceCountedObjectPtr<LuapeNodeBuilder> LuapeNodeBuilderPtr;

extern LuapeNodeBuilderPtr inputsNodeBuilder();
extern LuapeNodeBuilderPtr singletonNodeBuilder(const LuapeNodePtr& node);
extern LuapeNodeBuilderPtr compositeNodeBuilder(const std::vector<LuapeNodeBuilderPtr>& builders);
extern LuapeNodeBuilderPtr compositeNodeBuilder(LuapeNodeBuilderPtr builder1, LuapeNodeBuilderPtr builder2);
extern LuapeNodeBuilderPtr exhaustiveSequentialNodeBuilder(size_t complexity);

class StochasticNodeBuilder : public LuapeNodeBuilder
{
public:
  StochasticNodeBuilder(size_t numNodes = 0);

  virtual LuapeNodePtr sampleNode(ExecutionContext& context, const LuapeInferencePtr& function) = 0;

  virtual void buildNodes(ExecutionContext& context, const LuapeInferencePtr& function, size_t maxCount, std::vector<LuapeNodePtr>& res);

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

  virtual bool sampleAction(ExecutionContext& context, const LuapeInferencePtr& problem, LuapeGraphBuilderTypeStatePtr typeState, ObjectPtr& res) const = 0;

  virtual LuapeNodePtr sampleNode(ExecutionContext& context, const LuapeInferencePtr& problem);

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class SequentialNodeBuilderClass;

  virtual void samplingDone(ExecutionContext& context, size_t numSamplingFailures, size_t numFailuresAllowed) {}

  size_t complexity;

  LuapeUniversePtr universe;
  LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace;

  static bool isActionAvailable(ObjectPtr action, const std::vector<LuapeNodePtr>& stack);
  LuapeGraphBuilderTypeStatePtr getTypeState(size_t stepNumber, const std::vector<LuapeNodePtr>& stack) const;
  void executeAction(std::vector<LuapeNodePtr>& stack, const ObjectPtr& action) const;
};


}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_H_
