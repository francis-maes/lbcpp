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

namespace lbcpp
{

class LuapeNodeBuilder : public Object
{
public:
  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
    {return true;}

  virtual void buildNodes(ExecutionContext& context, size_t maxCount, std::vector<LuapeNodePtr>& res) = 0;
};

typedef ReferenceCountedObjectPtr<LuapeNodeBuilder> LuapeNodeBuilderPtr;

class StochasticNodeBuilder : public LuapeNodeBuilder
{
public:
  StochasticNodeBuilder(size_t numNodes = 0);

  virtual LuapeNodePtr sampleNode(ExecutionContext& context) = 0;

  virtual void buildNodes(ExecutionContext& context, size_t maxCount, std::vector<LuapeNodePtr>& res);

protected:
  friend class StochasticNodeBuilderClass;

  size_t numNodes;
};

class SequentialNodeBuilder : public StochasticNodeBuilder
{
public:
  SequentialNodeBuilder(size_t numNodes, size_t complexity);
  SequentialNodeBuilder() {}

  virtual bool sampleAction(ExecutionContext& context, LuapeGraphBuilderTypeStatePtr typeState, ObjectPtr& res) const = 0;

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function);
  virtual LuapeNodePtr sampleNode(ExecutionContext& context);

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

class SingletonNodeBuilder : public LuapeNodeBuilder
{
public:
  SingletonNodeBuilder(const LuapeNodePtr& node = LuapeNodePtr())
    : node(node) {}

  virtual void buildNodes(ExecutionContext& context, size_t maxCount, std::vector<LuapeNodePtr>& res)
    {res.push_back(node);}

protected:
  friend class SingletonNodeBuilderClass;

  LuapeNodePtr node;
};

class CompositeNodeBuilder : public LuapeNodeBuilder
{
public:
  CompositeNodeBuilder(const std::vector<LuapeNodeBuilderPtr>& builders)
    : builders(builders) {}
  CompositeNodeBuilder(LuapeNodeBuilderPtr builder1, LuapeNodeBuilderPtr builder2)
    : builders(2) {builders[0] = builder1; builders[1] = builder2;}
  CompositeNodeBuilder() {}
 
  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
  {
    bool ok = true;
    for (size_t i = 0; i < builders.size(); ++i)
      ok &= builders[i]->initialize(context, function);
    return ok;
  }

  virtual void buildNodes(ExecutionContext& context, size_t maxCount, std::vector<LuapeNodePtr>& res)
  {
    for (size_t i = 0; i < builders.size(); ++i)
    {
      builders[i]->buildNodes(context, maxCount ? maxCount - res.size() : 0, res);
      if (maxCount && res.size() >= maxCount)
        break;
    }
  }

protected:
  friend class CompositeNodeBuilderClass;

  std::vector<LuapeNodeBuilderPtr> builders;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_H_
